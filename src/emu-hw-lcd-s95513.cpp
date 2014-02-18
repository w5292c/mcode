#include "emu-hw-lcd-s95513.h"

#include "hw-uart.h"

#ifdef __AVR__
#include <avr/pgmspace.h>
#else /* __AVR__ */
#include "emu-common.h"

#include <stdlib.h>
#include <gtk/gtk.h>
#include <math.h>
#include <gtk/gtk.h>
#endif /* __AVR__ */

static GdkPixmap *ThePixmap = NULL;
static GtkWidget *TheLcdScreen = NULL;
static GtkWidget *TheLcdWindow = NULL;
static hw_i80_read_callback TheReadCallback = NULL;

static void emu_hw_lcd_s95513_schedule_update (void);
static uint32_t emu_hw_lcd_s95513_to_color (uint32_t data);
static gboolean emu_hw_lcd_s95513_update (gpointer data_pointer);
static void emu_hw_lcd_s95513_set_pixel (gint x, gint y, guint32 color);
static gboolean expose_event_callback (GtkWidget *widget, GdkEventExpose *event, gpointer data);

/**
 * Commands
 */
static void emu_hw_lcd_s95513_handle_write_start (uint32_t length, const uint8_t *data);
static void emu_hw_lcd_s95513_handle_write_continue (uint32_t length, const uint8_t *data);
static void emu_hw_lcd_s95513_handle_set_page_addr (uint32_t length, const uint8_t *data);
static void emu_hw_lcd_s95513_handle_set_column_addr (uint32_t length, const uint8_t *data);

void emu_hw_lcd_s95513_turn_on (void)
{
  hw_uart_write_string_P (PSTR ("EMU: emu_hw_lcd_s95513_turn_on\r\n"));
}

void emu_hw_lcd_s95513_turn_off (void)
{
  hw_uart_write_string_P (PSTR ("EMU: emu_hw_lcd_s95513_turn_off\r\n"));
}

void emu_hw_lcd_s95513_set_scroll_start (uint16_t start)
{
  hw_uart_write_string_P (PSTR ("EMU: emu_hw_lcd_s95513_set_scroll_start ("));
  hw_uart_write_uint (start);
  hw_uart_write_string_P (PSTR (")\r\n"));
}

void emu_hw_lcd_s95513_init (void)
{
  if (!TheLcdWindow)
  {
    /* initialize the offscreen pixmap */
    ThePixmap = gdk_pixmap_new (NULL, 320, 480, 24);
    GdkGC *pGC = gdk_gc_new (ThePixmap);

    /* emulates the initial color on LCD */
    const GdkColor color = { 0xff008080u, 0, 0, 0 };
    gdk_gc_set_foreground (pGC, &color);
    gdk_draw_rectangle (GDK_DRAWABLE (ThePixmap), pGC, TRUE, 0, 0, 320, 480);

    TheLcdWindow = gtk_window_new (GTK_WINDOW_TOPLEVEL);
    TheLcdScreen = gtk_drawing_area_new ();
    gtk_widget_set_size_request (TheLcdScreen, 320, 480);
    g_signal_connect (G_OBJECT (TheLcdScreen), "expose_event", G_CALLBACK (expose_event_callback), NULL);
    gtk_container_add (GTK_CONTAINER (TheLcdWindow), TheLcdScreen);

    gtk_widget_show_all (TheLcdScreen);
    gtk_widget_show_all (TheLcdWindow);

    g_object_unref (pGC);
  }
}

void emu_hw_lcd_s95513_deinit (void)
{
/*  g_object_unref (ThePixmap);
  ThePixmap = NULL;*/
/*  g_object_unref (TheLcdWindow);
  TheLcdWindow = NULL;*/
}

void emu_hw_lcd_s95513_set_read_callback (hw_i80_read_callback aCallback)
{
  TheReadCallback = aCallback;
}

void emu_hw_lcd_s95513_read (uint8_t cmd, uint8_t length)
{
  if (TheReadCallback)
  {
    /**@todo Implement reading, return empty data for now */
    (*TheReadCallback) (0, NULL);
  }
}

void emu_hw_lcd_s95513_write (uint8_t cmd, uint8_t length, const uint8_t *data)
{
  switch (cmd)
  {
  case LCD_S95513_WR_RAM_START:
    emu_hw_lcd_s95513_handle_write_start (length, data);
    break;
  case LCD_S95513_WR_RAM_CONT:
    emu_hw_lcd_s95513_handle_write_continue (length, data);
    break;
  case LCD_S95513_SET_COLUMN_ADDR:
    emu_hw_lcd_s95513_handle_set_column_addr (length, data);
    break;
  case LCD_S95513_SET_PAGE_ADDR:
    emu_hw_lcd_s95513_handle_set_page_addr (length, data);
    break;
  default:
    hw_uart_write_string_P (PSTR ("EMU: emu_hw_lcd_s95513_write (cmd: "));
    hw_uart_write_uint (cmd);
    hw_uart_write_string_P (PSTR (", data length: "));
    hw_uart_write_uint (length);
    hw_uart_write_string_P (PSTR (")\r\n"));
    break;
  }
}

void emu_hw_lcd_s95513_reset (void)
{
  GdkGC *const pGC = gdk_gc_new (ThePixmap);
  const GdkColor color = { 0xff808080u, 0, 0, 0 };
  gdk_gc_set_foreground (pGC, &color);
  gdk_draw_rectangle (GDK_DRAWABLE (ThePixmap), pGC, TRUE, 0, 0, 320, 480);
  g_object_unref (pGC);

  emu_hw_lcd_s95513_schedule_update ();
}

gboolean expose_event_callback (GtkWidget *widget, GdkEventExpose *event, gpointer data)
{
  GdkGC *const pGC = gdk_gc_new (ThePixmap);
  gdk_draw_drawable (GDK_DRAWABLE (widget->window),
    pGC, /* GdkGC *gc */
    ThePixmap, 0, 0, /* GdkDrawable *src, gint xsrc, gint ysrc, */
    0, 0, widget->allocation.width, widget->allocation.height);
  g_object_unref (pGC);

  return TRUE;
}

void emu_hw_lcd_s95513_set_pixel (gint x, gint y, guint32 color)
{
  if (!(x >= 0 && x < 320))
  {
    printf ("emu_hw_lcd_s95513_set_pixel: wrong x: %d, %d\r\n", x, y);
  }
  if (!(y >= 0 && y < 480))
  {
    printf ("emu_hw_lcd_s95513_set_pixel: wrong y: %d, %d\r\n", x, y);
  }
  g_assert (x >= 0 && x < 320);
  g_assert (y >= 0 && y < 480);
  g_assert (GDK_IS_DRAWABLE (ThePixmap));

  GdkGC *pGC = gdk_gc_new (GDK_DRAWABLE (ThePixmap));
  GdkColor colorStruct = { 0, 0, 0, 0 };
  colorStruct.pixel = color;
/*  gdk_gc_set_foreground (pGC, &colorStruct);
  gdk_draw_point (GDK_DRAWABLE (ThePixmap), pGC, x, y);*/
  g_object_unref (pGC);
  emu_hw_lcd_s95513_schedule_update ();
}

static uint32_t TheCache = 0;
static uint16_t ThePageEnd = 0;
static uint16_t TheNextPage = 0;
static uint16_t ThePageStart = 0;
static uint16_t TheColumnEnd = 0;
static uint16_t TheColumnStart = 0;
static uint16_t TheNextColumn = 0;
static uint32_t TheWriteIndex = 0;

static int8_t TheState = 0;
static void emu_hw_lcd_s95513_put_byte (uint8_t data)
{
  if (TheState < 0)
  {
    /* ignore-state */
    return;
  }

  TheCache = (TheCache << 8);
  TheCache = (TheCache | data);
  ++TheWriteIndex;
  if (0 == (TheWriteIndex%2))
  {
    /* got next color sample, put it to the surface */
    const uint32_t color = emu_hw_lcd_s95513_to_color (TheCache);
    emu_hw_lcd_s95513_set_pixel (TheNextColumn, TheNextPage, color);

    TheNextColumn++;
    if (TheNextColumn > TheColumnEnd)
    {
      TheNextColumn = TheColumnStart;
      TheNextPage++;
      if (TheNextPage > ThePageEnd)
      {
        TheState = -1;
      }
    }

    /* clean-up before the next sample */
    TheCache = 0;
  }
}

void emu_hw_lcd_s95513_handle_write_start (uint32_t length, const uint8_t *data)
{
  printf ("emu_hw_lcd_s95513_handle_write_start: %u\r\n", length);

  TheState = 0;
  TheCache = 0;
  TheWriteIndex = 0;
  TheNextPage = ThePageStart;
  TheNextColumn = TheColumnStart;

  while (length--)
  {
    emu_hw_lcd_s95513_put_byte (*data++);
  }
}

void emu_hw_lcd_s95513_handle_write_continue (uint32_t length, const uint8_t *data)
{
  while (length--)
  {
    emu_hw_lcd_s95513_put_byte (*data++);
  }
}

void emu_hw_lcd_s95513_handle_set_page_addr (uint32_t length, const uint8_t *data)
{
  /* do not support general case for now, to be implemented */
  g_assert (4 == length);

  ThePageStart = ((*data++)<<8) | (*data++);
  ThePageEnd = ((*data++)<<8) | (*data++);
  printf ("emu_hw_lcd_s95513_handle_set_page_addr: [%u, %u]\r\n", ThePageStart, ThePageEnd);
}

void emu_hw_lcd_s95513_handle_set_column_addr (uint32_t length, const uint8_t *data)
{
  /* do not support general case for now, to be implemented */
  g_assert (4 == length);

  TheColumnStart = ((*data++)<<8) | (*data++);
  TheColumnEnd = ((*data++)<<8) | (*data++);
  printf ("emu_hw_lcd_s95513_handle_set_column_addr: [%u, %u]\r\n", TheColumnStart, TheColumnEnd);
}

uint32_t emu_hw_lcd_s95513_to_color (uint32_t data)
{
  const uint8_t red =   (0xffu&((0xf80U&data)>>8));
  const uint8_t green = (0xffu&((0x7E0U&data)>>3));
  const uint8_t blue =  (0xffu&((0x01FU&data)<<3));
  return (red << 16)|(green << 8)| blue;
}

static guint TheUpdateSourceId = 0;
void emu_hw_lcd_s95513_schedule_update (void)
{
  if (TheUpdateSourceId)
  {
    g_source_remove (TheUpdateSourceId);
    TheUpdateSourceId = 0;
  }

  TheUpdateSourceId = g_timeout_add (100, emu_hw_lcd_s95513_update, NULL);
}

gboolean emu_hw_lcd_s95513_update (gpointer data_pointer)
{
/*  gtk_widget_queue_draw (TheLcdScreen);*/
  printf ("update-event\r\n");
  return FALSE;
}
