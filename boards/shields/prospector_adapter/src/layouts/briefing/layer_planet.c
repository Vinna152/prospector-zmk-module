#include "layer_planet.h"
#include "connection.h"
#include "battery_bars.h"

#include <ctype.h>
#include <math.h>
#include <stdio.h>
#include <zephyr/kernel.h>
#include <zmk/display.h>
#include <zmk/event_manager.h>
#include <zmk/events/layer_state_changed.h>
#include <zmk/keymap.h>

#include <fonts.h>
#include "colors.h"

static sys_slist_t widgets = SYS_SLIST_STATIC_INIT(&widgets);

#define PLANET_CX 196
#define PLANET_CY 60
#define PLANET_R  44
#define MOON_R    7
#define ORBIT_R   60
#define ORBIT_PERIOD_MS 30000

static struct k_work_delayable orbit_work;
static int64_t start_ms = 0;

static void position_circle(lv_obj_t *o, int cx, int cy, int r) {
    lv_obj_set_pos(o, cx - r, cy - r);
}

static void update_planet_phase(struct zmk_widget_layer_planet *w) {
    uint8_t avg = briefing_battery_avg();
    uint8_t conn_count = briefing_connected_count();

    if (conn_count == 0) {
        lv_obj_set_style_bg_color(w->planet, lv_color_hex(BRF_COLOR_ACCENT_DARK), LV_PART_MAIN);
    } else {
        int dim = (100 - avg) * 80 / 100;
        uint8_t r = 0xe3 - (uint8_t)(0xe3 * dim / 200);
        uint8_t g = 0x6a - (uint8_t)(0x6a * dim / 200);
        uint8_t b = 0x8c - (uint8_t)(0x8c * dim / 200);
        lv_obj_set_style_bg_color(w->planet, lv_color_make(r, g, b), LV_PART_MAIN);
    }

    int shadow_offset = PLANET_R - (avg * PLANET_R / 100);
    position_circle(w->shadow, PLANET_CX + shadow_offset, PLANET_CY, PLANET_R);
}

static void update_orbits(struct zmk_widget_layer_planet *w) {
    int64_t now = k_uptime_get();
    float t = (float)((now - start_ms) % ORBIT_PERIOD_MS) / (float)ORBIT_PERIOD_MS;
    float angle_l = t * 2.0f * 3.14159265f;
    float angle_r = angle_l + 3.14159265f;

    bool l_conn = briefing_connection_is_connected(0);
    bool r_conn = briefing_connection_is_connected(1);

    if (l_conn) {
        int x = PLANET_CX + (int)(cosf(angle_l) * ORBIT_R);
        int y = PLANET_CY + (int)(sinf(angle_l) * ORBIT_R * 0.55f);
        position_circle(w->moon_left, x, y, MOON_R);
        lv_obj_clear_flag(w->moon_left, LV_OBJ_FLAG_HIDDEN);
    } else {
        lv_obj_add_flag(w->moon_left, LV_OBJ_FLAG_HIDDEN);
    }

    if (r_conn) {
        int x = PLANET_CX + (int)(cosf(angle_r) * ORBIT_R);
        int y = PLANET_CY + (int)(sinf(angle_r) * ORBIT_R * 0.55f);
        position_circle(w->moon_right, x, y, MOON_R);
        lv_obj_clear_flag(w->moon_right, LV_OBJ_FLAG_HIDDEN);
    } else {
        lv_obj_add_flag(w->moon_right, LV_OBJ_FLAG_HIDDEN);
    }
}

static void orbit_work_handler(struct k_work *work) {
    struct zmk_widget_layer_planet *w;
    SYS_SLIST_FOR_EACH_CONTAINER(&widgets, w, node) {
        update_orbits(w);
        update_planet_phase(w);
    }
    k_work_schedule(&orbit_work, K_MSEC(80));
}

struct layer_state {
    uint8_t index;
};

static void layer_update_cb(struct layer_state state) {
    const char *name = zmk_keymap_layer_name(zmk_keymap_layer_index_to_id(state.index));
    char buf[16];
    if (name && *name) {
        snprintf(buf, sizeof(buf), "%s", name);
    } else {
        snprintf(buf, sizeof(buf), "%d", state.index);
    }
    for (int i = 0; buf[i]; i++) buf[i] = toupper((unsigned char)buf[i]);

    struct zmk_widget_layer_planet *w;
    SYS_SLIST_FOR_EACH_CONTAINER(&widgets, w, node) {
        lv_label_set_text(w->layer_label, buf);
    }
}

static struct layer_state layer_get_state(const zmk_event_t *eh) {
    return (struct layer_state){.index = zmk_keymap_highest_layer_active()};
}

ZMK_DISPLAY_WIDGET_LISTENER(widget_layer_planet, struct layer_state, layer_update_cb, layer_get_state)
ZMK_SUBSCRIPTION(widget_layer_planet, zmk_layer_state_changed);

int zmk_widget_layer_planet_init(struct zmk_widget_layer_planet *widget, lv_obj_t *parent) {
    widget->obj = lv_obj_create(parent);
    lv_obj_set_size(widget->obj, 280, 110);
    lv_obj_set_style_bg_opa(widget->obj, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_border_width(widget->obj, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_all(widget->obj, 0, LV_PART_MAIN);
    lv_obj_clear_flag(widget->obj, LV_OBJ_FLAG_SCROLLABLE);

    widget->planet = lv_obj_create(widget->obj);
    lv_obj_set_size(widget->planet, PLANET_R * 2, PLANET_R * 2);
    position_circle(widget->planet, PLANET_CX, PLANET_CY, PLANET_R);
    lv_obj_set_style_radius(widget->planet, LV_RADIUS_CIRCLE, LV_PART_MAIN);
    lv_obj_set_style_bg_color(widget->planet, lv_color_hex(BRF_COLOR_ACCENT), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(widget->planet, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_border_width(widget->planet, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_all(widget->planet, 0, LV_PART_MAIN);
    lv_obj_set_style_clip_corner(widget->planet, true, LV_PART_MAIN);
    lv_obj_clear_flag(widget->planet, LV_OBJ_FLAG_SCROLLABLE);

    widget->shadow = lv_obj_create(widget->planet);
    lv_obj_set_size(widget->shadow, PLANET_R * 2, PLANET_R * 2);
    lv_obj_set_pos(widget->shadow, PLANET_R, 0);
    lv_obj_set_style_radius(widget->shadow, LV_RADIUS_CIRCLE, LV_PART_MAIN);
    lv_obj_set_style_bg_color(widget->shadow, lv_color_hex(BRF_COLOR_ACCENT_DARK), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(widget->shadow, LV_OPA_70, LV_PART_MAIN);
    lv_obj_set_style_border_width(widget->shadow, 0, LV_PART_MAIN);
    lv_obj_clear_flag(widget->shadow, LV_OBJ_FLAG_SCROLLABLE);

    widget->moon_left = lv_obj_create(widget->obj);
    lv_obj_set_size(widget->moon_left, MOON_R * 2, MOON_R * 2);
    lv_obj_set_style_radius(widget->moon_left, LV_RADIUS_CIRCLE, LV_PART_MAIN);
    lv_obj_set_style_bg_color(widget->moon_left, lv_color_hex(BRF_COLOR_ACCENT), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(widget->moon_left, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_border_width(widget->moon_left, 0, LV_PART_MAIN);
    lv_obj_add_flag(widget->moon_left, LV_OBJ_FLAG_HIDDEN);

    widget->moon_right = lv_obj_create(widget->obj);
    lv_obj_set_size(widget->moon_right, MOON_R * 2, MOON_R * 2);
    lv_obj_set_style_radius(widget->moon_right, LV_RADIUS_CIRCLE, LV_PART_MAIN);
    lv_obj_set_style_bg_color(widget->moon_right, lv_color_hex(BRF_COLOR_ACCENT), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(widget->moon_right, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_border_width(widget->moon_right, 0, LV_PART_MAIN);
    lv_obj_add_flag(widget->moon_right, LV_OBJ_FLAG_HIDDEN);

    widget->layer_label = lv_label_create(widget->obj);
    lv_label_set_text(widget->layer_label, "BASE");
    lv_obj_set_style_text_font(widget->layer_label, &FR_Regular_48, LV_PART_MAIN);
    lv_obj_set_style_text_color(widget->layer_label, lv_color_hex(BRF_COLOR_TEXT), LV_PART_MAIN);
    lv_obj_set_style_text_letter_space(widget->layer_label, -1, LV_PART_MAIN);
    lv_obj_set_pos(widget->layer_label, 20, 14);

    widget->subtitle = lv_label_create(widget->obj);
    lv_label_set_text(widget->subtitle, "KEY-LAYER  CONFIG ALPHA");
    lv_obj_set_style_text_font(widget->subtitle, &FG_Medium_20, LV_PART_MAIN);
    lv_obj_set_style_text_color(widget->subtitle, lv_color_hex(BRF_COLOR_TEXT_DIM), LV_PART_MAIN);
    lv_obj_set_style_text_letter_space(widget->subtitle, 2, LV_PART_MAIN);
    lv_obj_set_pos(widget->subtitle, 20, 80);

    sys_slist_append(&widgets, &widget->node);
    widget_layer_planet_init();

    start_ms = k_uptime_get();
    k_work_init_delayable(&orbit_work, orbit_work_handler);
    k_work_schedule(&orbit_work, K_MSEC(80));

    update_planet_phase(widget);
    return 0;
}

lv_obj_t *zmk_widget_layer_planet_obj(struct zmk_widget_layer_planet *widget) {
    return widget->obj;
}
