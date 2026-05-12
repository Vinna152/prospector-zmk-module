#include "top_bar.h"

#include <stdio.h>
#include <zmk/display.h>
#include <zmk/ble.h>
#include <zmk/event_manager.h>
#include <zmk/events/ble_active_profile_changed.h>

#include <fonts.h>
#include "colors.h"

static sys_slist_t widgets = SYS_SLIST_STATIC_INIT(&widgets);

static uint8_t active_profile_index = 0;

static void render_coords(struct zmk_widget_top_bar *widget) {
    char buf[24];
    snprintf(buf, sizeof(buf), "TAU CETI IV / %02d", active_profile_index + 1);
    lv_label_set_text(widget->coords_label, buf);
}

static int profile_listener(const zmk_event_t *eh) {
    const struct zmk_ble_active_profile_changed *ev = as_zmk_ble_active_profile_changed(eh);
    if (ev) {
        active_profile_index = ev->index;
        struct zmk_widget_top_bar *w;
        SYS_SLIST_FOR_EACH_CONTAINER(&widgets, w, node) {
            render_coords(w);
        }
    }
    return ZMK_EV_EVENT_BUBBLE;
}

ZMK_LISTENER(widget_top_bar, profile_listener);
ZMK_SUBSCRIPTION(widget_top_bar, zmk_ble_active_profile_changed);

int zmk_widget_top_bar_init(struct zmk_widget_top_bar *widget, lv_obj_t *parent) {
    widget->obj = lv_obj_create(parent);
    lv_obj_set_size(widget->obj, 280, 18);
    lv_obj_set_style_bg_opa(widget->obj, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_border_width(widget->obj, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_all(widget->obj, 0, LV_PART_MAIN);

    widget->briefing_diamond = lv_label_create(widget->obj);
    lv_label_set_text(widget->briefing_diamond, "*");
    lv_obj_set_style_text_font(widget->briefing_diamond, &FG_Medium_20, LV_PART_MAIN);
    lv_obj_set_style_text_color(widget->briefing_diamond, lv_color_hex(BRF_COLOR_ACCENT), LV_PART_MAIN);
    lv_obj_set_pos(widget->briefing_diamond, 20, 0);

    widget->briefing_label = lv_label_create(widget->obj);
    lv_label_set_text(widget->briefing_label, "BRIEFING");
    lv_obj_set_style_text_font(widget->briefing_label, &FG_Medium_20, LV_PART_MAIN);
    lv_obj_set_style_text_color(widget->briefing_label, lv_color_hex(BRF_COLOR_ACCENT), LV_PART_MAIN);
    lv_obj_set_style_text_letter_space(widget->briefing_label, 2, LV_PART_MAIN);
    lv_obj_set_pos(widget->briefing_label, 36, 0);

    widget->coords_label = lv_label_create(widget->obj);
    lv_obj_set_style_text_font(widget->coords_label, &FG_Medium_20, LV_PART_MAIN);
    lv_obj_set_style_text_color(widget->coords_label, lv_color_hex(BRF_COLOR_TEXT_DIM), LV_PART_MAIN);
    lv_obj_set_style_text_letter_space(widget->coords_label, 2, LV_PART_MAIN);
    lv_obj_align(widget->coords_label, LV_ALIGN_TOP_RIGHT, -20, 0);

    active_profile_index = zmk_ble_active_profile_index();
    render_coords(widget);

    sys_slist_append(&widgets, &widget->node);
    return 0;
}

lv_obj_t *zmk_widget_top_bar_obj(struct zmk_widget_top_bar *widget) {
    return widget->obj;
}
