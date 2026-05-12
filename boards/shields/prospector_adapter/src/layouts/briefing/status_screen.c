#include <lvgl.h>

#include "top_bar.h"
#include "layer_planet.h"
#include "connection.h"
#include "battery_bars.h"
#include "cpm.h"
#include "mods.h"

#include "colors.h"

static struct zmk_widget_top_bar      top_bar_widget;
static struct zmk_widget_layer_planet layer_planet_widget;
static struct zmk_widget_connection   connection_widget;
static struct zmk_widget_battery_bars battery_bars_widget;
static struct zmk_widget_cpm          cpm_widget;
static struct zmk_widget_mods         mods_widget;

static void draw_corner_marks(lv_obj_t *parent) {
    static const lv_point_t tl_h[] = {{6, 8}, {14, 8}};
    static const lv_point_t tl_v[] = {{10, 4}, {10, 12}};
    static const lv_point_t tr_h[] = {{266, 8}, {274, 8}};
    static const lv_point_t tr_v[] = {{270, 4}, {270, 12}};
    static const lv_point_t bl_h[] = {{6, 232}, {14, 232}};
    static const lv_point_t bl_v[] = {{10, 228}, {10, 236}};
    static const lv_point_t br_h[] = {{266, 232}, {274, 232}};
    static const lv_point_t br_v[] = {{270, 228}, {270, 236}};

    static const lv_point_t *pairs[] = {tl_h, tl_v, tr_h, tr_v, bl_h, bl_v, br_h, br_v};
    for (int i = 0; i < 8; i++) {
        lv_obj_t *line = lv_line_create(parent);
        lv_line_set_points(line, pairs[i], 2);
        lv_obj_set_style_line_color(line, lv_color_hex(BRF_COLOR_TEXT_DIM), LV_PART_MAIN);
        lv_obj_set_style_line_width(line, 1, LV_PART_MAIN);
    }
}

static void draw_horizontal_rule(lv_obj_t *parent) {
    static const lv_point_t pts[] = {{20, 152}, {260, 152}};
    lv_obj_t *line = lv_line_create(parent);
    lv_line_set_points(line, pts, 2);
    lv_obj_set_style_line_color(line, lv_color_hex(BRF_COLOR_RULE), LV_PART_MAIN);
    lv_obj_set_style_line_width(line, 1, LV_PART_MAIN);
}

lv_obj_t *zmk_display_status_screen() {
    lv_obj_t *screen = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(screen, lv_color_hex(BRF_COLOR_BG), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(screen, LV_OPA_COVER, LV_PART_MAIN);

    draw_corner_marks(screen);
    draw_horizontal_rule(screen);

    zmk_widget_top_bar_init(&top_bar_widget, screen);
    lv_obj_set_pos(zmk_widget_top_bar_obj(&top_bar_widget), 0, 18);

    zmk_widget_layer_planet_init(&layer_planet_widget, screen);
    lv_obj_set_pos(zmk_widget_layer_planet_obj(&layer_planet_widget), 0, 42);

    zmk_widget_battery_bars_init(&battery_bars_widget, screen);
    lv_obj_set_pos(zmk_widget_battery_bars_obj(&battery_bars_widget), 16, 160);

    zmk_widget_connection_init(&connection_widget, screen);
    lv_obj_set_pos(zmk_widget_connection_obj(&connection_widget), 120, 160);

    zmk_widget_cpm_init(&cpm_widget, screen);
    lv_obj_set_pos(zmk_widget_cpm_obj(&cpm_widget), 200, 160);

    zmk_widget_mods_init(&mods_widget, screen);
    lv_obj_set_pos(zmk_widget_mods_obj(&mods_widget), 20, 216);

    return screen;
}
