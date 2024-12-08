#if 1 /*Set it to "1" to enable content*/

#ifndef LV_CONF_H
#define LV_CONF_H

#include <stdint.h>

/*====================
   COLOR SETTINGS
 *====================*/

#define LV_COLOR_DEPTH     16
#define LV_COLOR_16_SWAP   0

/*====================
 * MEMORY SETTINGS
 *====================*/

#define LV_MEM_CUSTOM      1
#if LV_MEM_CUSTOM == 0
    #define LV_MEM_SIZE    (32U * 1024U)
#else
    #define LV_MEM_CUSTOM_INCLUDE   <stdlib.h>
    #define LV_MEM_CUSTOM_ALLOC     malloc
    #define LV_MEM_CUSTOM_FREE      free
#endif

/*====================
 * HAL SETTINGS
 *====================*/

#define LV_TICK_CUSTOM     1
#if LV_TICK_CUSTOM == 1
    #define LV_TICK_CUSTOM_INCLUDE  "esp_timer.h"
    #define LV_TICK_CUSTOM_SYS_TIME_EXPR (esp_timer_get_time() / 1000)
#endif

/*====================
 * FEATURE CONFIGURATION
 *====================*/

/*1: Enable the Animations */
#define LV_USE_ANIMATION        1

/*1: Enable shadow drawing*/
#define LV_USE_SHADOW           1

/*1: Enable object groups (for keyboard/encoder navigation) */
#define LV_USE_GROUP            1

/*1: Enable GPU interface*/
#define LV_USE_GPU              0

/*1: Enable file system (might be required for images */
#define LV_USE_FILESYSTEM       1

/*1: Enable drawing complex multi-line TrueType-like fonts*/
#define LV_USE_FREETYPE         0

/*================
 * THEME USAGE
 *================*/

/*1: Enable all the themes*/
#define LV_USE_THEME_DEFAULT    1
#define LV_THEME_DEFAULT_INIT               lv_theme_default_init
#define LV_THEME_DEFAULT_COLOR_PRIMARY      lv_color_hex(0x01a2b1)
#define LV_THEME_DEFAULT_COLOR_SECONDARY    lv_color_hex(0x44d1b6)

/*==================
 * WIDGET USAGE
 *==================*/

/*Documentation of the widgets: https://docs.lvgl.io/latest/en/html/widgets/index.html*/

#define LV_USE_ARC          1

#define LV_USE_BAR          1

#define LV_USE_BTN          1

#define LV_USE_BTNMATRIX    1

#define LV_USE_CANVAS       1

#define LV_USE_CHECKBOX     1

#define LV_USE_DROPDOWN     1

#define LV_USE_IMG          1

#define LV_USE_LABEL        1
#if LV_USE_LABEL
    #define LV_LABEL_TEXT_SELECTION         1
    #define LV_LABEL_LONG_TXT_HINT          1
#endif

#define LV_USE_LINE         1

#define LV_USE_ROLLER       1

#define LV_USE_SLIDER       1

#define LV_USE_SWITCH       1

#define LV_USE_TEXTAREA     1
#if LV_USE_TEXTAREA != 0
    #define LV_TEXTAREA_DEF_PWD_SHOW_TIME   1500
#endif

#define LV_USE_TABLE        1

/*==================
 * EXTRA COMPONENTS
 *==================*/

/*-----------
 * Widgets
 *----------*/
#define LV_USE_CALENDAR     1
#if LV_USE_CALENDAR
    #define LV_CALENDAR_WEEK_STARTS_MONDAY 0
#endif

#define LV_USE_CHART        1

#define LV_USE_COLORWHEEL   1

#define LV_USE_IMGBTN       1

#define LV_USE_KEYBOARD     1

#define LV_USE_LED          1

#define LV_USE_LIST         1

#define LV_USE_METER        1

#define LV_USE_SPINBOX      1

#define LV_USE_SPINNER      1

#define LV_USE_TABVIEW      1

#define LV_USE_TILEVIEW     1

#define LV_USE_WIN          1

/*-----------
 * Themes
 *----------*/
#define LV_USE_THEME_BASIC  1

/*-----------
 * Layouts
 *----------*/
#define LV_USE_FLEX         1
#define LV_USE_GRID         1

#endif /*LV_CONF_H*/

#endif /*End of "Content enable"*/