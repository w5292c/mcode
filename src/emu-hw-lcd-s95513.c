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

/*static gboolean emu_hw_lcd_s95513_expose_cb (GtkWidget *widget, cairo_t *cr, gpointer data);*/
/*static gint emu_hw_lcd_s95513_expose_cb (GtkWidget *widget, GdkEventExpose *event, gpointer data);*/
static gboolean expose_event_callback (GtkWidget *widget, GdkEventExpose *event, gpointer data);

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

static GtkWidget *TheLcdScreen = NULL;
static GtkWidget *TheLcdWindow = NULL;
void emu_hw_lcd_s95513_init (void)
{
  if (!TheLcdWindow)
  {
    TheLcdWindow = gtk_window_new (GTK_WINDOW_TOPLEVEL);
    TheLcdScreen = gtk_drawing_area_new ();
/*gtk_label_new ("Hello");*/
/*gtk_drawing_area_new ();*/
    gtk_widget_set_size_request (TheLcdScreen, 320, 480);
    g_signal_connect (G_OBJECT (TheLcdScreen), "expose_event",
                    G_CALLBACK (expose_event_callback), NULL);
/*mcode_lcd_screen_new ();*/
    gtk_container_add (GTK_CONTAINER (TheLcdWindow), TheLcdScreen);

    gtk_widget_show_all (TheLcdScreen);
    gtk_widget_show_all (TheLcdWindow);
  }
}

void emu_hw_lcd_s95513_deinit (void)
{
/*  g_object_unref (TheLcdWindow);
  TheLcdWindow = NULL;*/
}

void emu_hw_lcd_s95513_read (uint8_t cmd, uint8_t length)
{
}

void emu_hw_lcd_s95513_write (uint8_t cmd, uint8_t length, const uint8_t *data)
{
}

void emu_hw_lcd_s95513_reset (void)
{
}

/*#include <gtk/gtkmain.h>
#include <gtk/gtksignal.h>*/

/*#define SCROLL_DELAY_LENGTH  300
#define DIAL_DEFAULT_SIZE 100*/
gboolean expose_event_callback (GtkWidget *widget, GdkEventExpose *event, gpointer data)
{
printf ("expose_event_callback\r\n");
  gdk_draw_arc (widget->window,
                widget->style->fg_gc[gtk_widget_get_state (widget)],
                TRUE,
                0, 0, widget->allocation.width, widget->allocation.height,
                0, 64 * 360);
  return TRUE;
}


/* Forward declarations */
static void mcode_lcd_screen_realize (GtkWidget *widget);
static void mcode_lcd_screen_destroy (GtkObject *pObject);
static void mcode_lcd_screen_init (MCodeLcdScreen *pObject);
static void mcode_lcd_screen_class_init (MCodeLcdScreenClass *klass);
static gint mcode_lcd_screen_expose (GtkWidget *widget, GdkEventExpose *event);
static void mcode_lcd_screen_size_allocate (GtkWidget *widget, GtkAllocation *allocation);
static void mcode_lcd_screen_size_request (GtkWidget *widget, GtkRequisition *requisition);

/* Local data */
static GtkWidgetClass *parent_class = NULL;

guint mcode_lcd_screen_get_type (void)
{
  static guint lcd_screen_type = 0;

  if (!lcd_screen_type)
    {
      GtkTypeInfo lcd_screen_info =
      {
        "MCodeLcdScreen",
        sizeof (MCodeLcdScreen),
        sizeof (MCodeLcdScreenClass),
        (GtkClassInitFunc) mcode_lcd_screen_class_init,
        (GtkObjectInitFunc) mcode_lcd_screen_init,
        NULL,
        NULL,
      };

      lcd_screen_type = gtk_type_unique (gtk_widget_get_type (), &lcd_screen_info);
    }

  return lcd_screen_type;
}

static void
mcode_lcd_screen_class_init (MCodeLcdScreenClass *klass)
{
  GtkObjectClass *object_class;
  GtkWidgetClass *widget_class;

  object_class = (GtkObjectClass*) klass;
  widget_class = (GtkWidgetClass*) klass;

  parent_class = gtk_type_class (gtk_widget_get_type ());

  object_class->destroy = mcode_lcd_screen_destroy;

  widget_class->realize = mcode_lcd_screen_realize;
  widget_class->expose_event = mcode_lcd_screen_expose;
  widget_class->size_request = mcode_lcd_screen_size_request;
  widget_class->size_allocate = mcode_lcd_screen_size_allocate;
/*  widget_class->button_press_event = mcode_lcd_screen_button_press;
  widget_class->button_release_event = mcode_lcd_screen_button_release;
  widget_class->motion_notify_event = mcode_lcd_screen_motion_notify;*/
}

void mcode_lcd_screen_init (MCodeLcdScreen *pSelf)
{
  GdkColor color;
  color.red = 0;
  color.green = 65535;
  color.blue = 65535;

  pSelf->m_pPixmap = gdk_pixmap_new (NULL, 320, 480, 24);
  GdkGC *const pGC = gdk_gc_new (pSelf->m_pPixmap);
  gdk_gc_set_foreground (pGC, &color);
  gdk_draw_rectangle (pSelf->m_pPixmap, pGC, TRUE, 0, 0, 320, 480);
  printf ("Created pixmap: %p\r\n", pSelf->m_pPixmap);
  g_object_unref (pGC);
}

GtkWidget*
mcode_lcd_screen_new (void)
{
  return gtk_type_new (mcode_lcd_screen_get_type ());
}

static void
mcode_lcd_screen_destroy (GtkObject *pObject)
{
  MCodeLcdScreen *pSelf;
  g_return_if_fail (pObject);
  g_return_if_fail (MCODE_IS_LCD_SCREEN (pObject));

  pSelf = MCODE_LCD_SCREEN (pObject);
  g_object_unref (pSelf->m_pPixmap);
  printf ("Released pixmap: %p\r\n", pSelf->m_pPixmap);
  pSelf->m_pPixmap = NULL;

  if (GTK_OBJECT_CLASS (parent_class)->destroy)
  {
    (* GTK_OBJECT_CLASS (parent_class)->destroy) (pObject);
  }
}

void mcode_lcd_screen_realize (GtkWidget *widget)
{
  printf ("mcode_lcd_screen_realize\r\n");
#if 1
  MCodeLcdScreen *dial;
  GdkWindowAttr attributes;
  gint attributes_mask;

  g_return_if_fail (widget != NULL);
  g_return_if_fail (MCODE_IS_LCD_SCREEN (widget));

  gtk_widget_set_realized (widget, TRUE);
  /*GTK_WIDGET_SET_FLAGS (widget, GTK_REALIZED);*/
  dial = MCODE_LCD_SCREEN (widget);

  attributes.x = widget->allocation.x;
  attributes.y = widget->allocation.y;
  attributes.width = widget->allocation.width;
  attributes.height = widget->allocation.height;
  attributes.wclass = GDK_INPUT_OUTPUT;
  attributes.window_type = GDK_WINDOW_CHILD;
  attributes.event_mask = gtk_widget_get_events (widget) |
    GDK_EXPOSURE_MASK | GDK_BUTTON_PRESS_MASK |
    GDK_BUTTON_RELEASE_MASK | GDK_POINTER_MOTION_MASK |
    GDK_POINTER_MOTION_HINT_MASK;
  attributes.visual = gtk_widget_get_visual (widget);
  attributes.colormap = gtk_widget_get_colormap (widget);

  attributes_mask = GDK_WA_X | GDK_WA_Y | GDK_WA_VISUAL | GDK_WA_COLORMAP;
  widget->window = gdk_window_new (widget->parent->window, &attributes, attributes_mask);

  widget->style = gtk_style_attach (widget->style, widget->window);

  gdk_window_set_user_data (widget->window, widget);

/*  gtk_style_set_background (widget->style, widget->window, GTK_STATE_ACTIVE);*/
#endif
}

gint mcode_lcd_screen_expose (GtkWidget *pObject, GdkEventExpose *event)
{
printf ("mcode_lcd_screen_expose\r\n");
  MCodeLcdScreen *pSelf;
  g_return_if_fail (pObject);
  g_return_if_fail (MCODE_IS_LCD_SCREEN (pObject));

  pSelf = MCODE_LCD_SCREEN (pObject);

  GdkWindow *const pWindow = gtk_widget_get_root_window (pObject);
  GdkGC *const pGC = gdk_gc_new (pSelf->m_pPixmap);

  gdk_draw_drawable (pWindow, pGC, pSelf->m_pPixmap, 0, 0, 0, 0, 320, 480);
  g_object_unref (pGC);

#if 0
  GtkDial *dial;
  GdkPoint points[3];
  gdouble s,c;
  gdouble theta;
  gint xc, yc;
  gint tick_length;
  gint i;

  g_return_val_if_fail (widget != NULL, FALSE);
  g_return_val_if_fail (GTK_IS_DIAL (widget), FALSE);
  g_return_val_if_fail (event != NULL, FALSE);
#endif
  return FALSE;
}

void mcode_lcd_screen_size_request (GtkWidget *widget, GtkRequisition *requisition)
{
  requisition->width = 320;
  requisition->height = 480;
}

void mcode_lcd_screen_size_allocate (GtkWidget *widget, GtkAllocation *allocation)
{
  printf ("mcode_lcd_screen_size_allocate: %d, %d, %d, %d\r\n", allocation->x, allocation->y, allocation->width, allocation->height);
#if 0
  MCodeLcdScreen *dial;

  g_return_if_fail (widget != NULL);
  g_return_if_fail (GTK_IS_DIAL (widget));
  g_return_if_fail (allocation != NULL);

  widget->allocation = *allocation;
  if (GTK_WIDGET_REALIZED (widget))
    {
      dial = GTK_DIAL (widget);

      gdk_window_move_resize (widget->window,
                              allocation->x, allocation->y,
                              allocation->width, allocation->height);

      dial->radius = MAX(allocation->width,allocation->height) * 0.45;
      dial->pointer_width = dial->radius / 5;
    }
#endif
}
