#ifndef MCODE_EMU_HW_LCD_S95513_H
#define MCODE_EMU_HW_LCD_S95513_H

#include "hw-lcd-s95513.h"

#include <gdk/gdk.h>

void emu_hw_lcd_s95513_init (void);
void emu_hw_lcd_s95513_deinit (void);

void emu_hw_lcd_s95513_turn_on (void);
void emu_hw_lcd_s95513_turn_off (void);
void emu_hw_lcd_s95513_set_scroll_start (uint16_t start);

/*
 * GTK part: this defines a GTK window
 * that emulates the LCD screen based on S95513 driver
 */
#include <gtk/gtkwidget.h>

/*
GTK => MCODE
DIAL => LCD_SCREEN
*/

#define MCODE_LCD_SCREEN(obj)          GTK_CHECK_CAST (obj, mcode_lcd_screen_get_type (), MCodeLcdScreen)
#define MCODE_LCD_SCREEN_CLASS(klass)  GTK_CHECK_CLASS_CAST (klass, mcode_lcd_screen_get_type (), MCodeLcdScreenClass)
#define MCODE_IS_LCD_SCREEN(obj)       GTK_CHECK_TYPE (obj, mcode_lcd_screen_get_type ())

typedef struct _MCodeLcdScreen        MCodeLcdScreen;
typedef struct _MCodeLcdScreenClass   MCodeLcdScreenClass;

struct _MCodeLcdScreen
{
  GtkWidget parent;

  GdkPixmap *m_pPixmap;
};

struct _MCodeLcdScreenClass
{
  GtkWidgetClass parent_class;
};

GtkWidget* mcode_lcd_screen_new (void);
guint mcode_lcd_screen_get_type (void);

#endif /* MCODE_EMU_HW_LCD_S95513_H */
