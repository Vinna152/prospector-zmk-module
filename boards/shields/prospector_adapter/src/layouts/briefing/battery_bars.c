#include "battery_bars.h"
#include "connection.h"

#include <stdio.h>
#include <zmk/ble.h>
#include <zmk/display.h>
#include <zmk/event_manager.h>
#include <zmk/events/battery_state_changed.h>
#include <zmk/events/split_central_status_changed.h>

#include <fonts.h>
#include "colors.h"

static sys_slist_t widgets = SYS_SLIST_STATIC_INIT(&widgets);

#ifndef PERIPHERAL_COUNT
#define PERIPHERAL_COUNT ZMK_SPLIT_BLE_PERIPHERAL_COUNT
#endif

#define BAR_WIDTH 88
#define BAR_HEIGHT 4
#define BAR_GAP 4

static uint8_t g_battery[PERIPHERAL_COUNT] = {0};

uint8_t briefing_battery_get(uint8_t source) {
    if (source >= PERIPHERAL_COUNT) return 0;
    return g_battery[source];
}

uint8_t briefing_battery_avg(void) {
    uint16_t sum = 0;
    uint8_t n = 0;
    for (int i = 0; i < PERIPHERAL_COUNT; i++) {
        if (briefing_connection_is_connected(i)) {
            sum += g_battery[i];
            n++;
        }
    }
    return n ? (uint8_t)(sum / n) : 0;
}

static void render(struct zmk_widget_battery_bars *w) {
    uint8_t l = PERIPHERAL_COUNT > 0 ? g_battery[0] : 0;
    uint8_t r = PERIPHERAL_COUNT > 1 ? g_battery[1] : 0;
    bool l_conn = briefing_connection_is_connected(0);
    bool r_conn = (PERIPHERAL_COUNT > 1) && briefing_connection_is_connected(1);

    char buf[12];
    snprintf(buf, sizeof(buf), "%03d/%03d", l_conn ? l : 0, r_conn ? r : 0);
    lv_label_set_text(w->value, buf);

    lv_bar_set_value(w->bar_left, l_conn ? l : 0, LV_ANIM_ON);
    lv_bar_set_value(w->bar_right, r_conn ? r : 0, LV_ANIM_ON);

    uint32_t l_color = l_conn ? BRF_COLOR_ACCENT : BRF_COLOR_RULE;
    uint32_t r_color = r_conn ? BRF_COLOR_ACCENT : BRF_COLOR_RULE;
    lv_obj_set_style_bg_color(w->bar_left, lv_color_hex(l_color), LV_PART_INDICATOR);
    lv_obj_set_style_bg_color(w->bar_right, lv_color_hex(r_color), LV_PART_INDICATOR);
}

struct batt_state {
    uint8_t source;
    uint8_t level;
};

static void batt_update_cb(struct batt_state s) {
    if (s.source < PERIPHERAL_COUNT) {
        g_battery[s.source] = s.level;
    }
    struct zmk_widget_battery_bars *w;
    SYS_SLIST_FOR_EACH_CONTAINER(&widgets, w, node) {
        render(w);
    }
}

static struct batt_state batt_get_state(const zmk_event_t *eh) {
    if (!eh) return (struct batt_state){0};
    const struct zmk_peripheral_battery_state_changed *ev =
        as_zmk_peripheral_battery_state_changed(eh);
    if (!ev) return (struct batt_state){0};
    return (struct batt_state){.source = ev->source, .level = ev->state_of_charge};
}

ZMK_DISPLAY_WIDGET_LISTENER(widget_battery_bars, struct batt_state, batt_update_cb, batt_get_state)
ZMK_SUBSCRIPTION(widget_battery_bars, zmk_peripheral_battery_state_changed);

static int conn_relay(const zmk_event_t *eh) {
    struct zmk_widget_battery_bars *w;
    SYS_SLIST_FOR_EACH_CONTAINER(&widgets, w, node) {
        render(w);
    }
    return ZMK_EV_EVENT_BUBBLE;
}
ZMK_LISTENER(widget_battery_bars_conn, conn_relay);
ZMK_SUBSCRIPTION(widget_battery_bars_conn, zmk_split_central_status_changed);

static lv_obj_t *make_bar(lv_obj_t *parent, int x, int y) {
    lv_obj_t *bar = lv_bar_create(parent);
    lv_obj_set_size(bar, BAR_WIDTH, BAR_HEIGHT);
    lv_obj_set_pos(bar, x, y);
    lv_bar_set_range(bar, 0, 100);
    lv_bar_set_value(bar, 0, LV_ANIM_OFF);
    lv_obj_set_style_radius(bar, 1, LV_PART_MAIN);
    lv_obj_set_style_radius(bar, 1, LV_PART_INDICATOR);
    lv_obj_set_style_bg_color(bar, lv_color_hex(BRF_COLOR_RULE), LV_PART_MAIN);
    lv_obj_set_style_bg_color(bar, lv_color_hex(BRF_COLOR_ACCENT), LV_PART_INDICATOR);
    lv_obj_set_style_bg_opa(bar, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(bar, LV_OPA_COVER, LV_PART_INDICATOR);
    return bar;
}

int zmk_widget_battery_bars_init(struct zmk_widget_battery_bars *widget, lv_obj_t *parent) {
    widget->obj = lv_obj_create(parent);
    lv_obj_set_size(widget->obj, 100, 52);
    lv_obj_set_style_bg_opa(widget->obj, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_border_width(widget->obj, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_all(widget->obj, 0, LV_PART_MAIN);

    widget->header = lv_label_create(widget->obj);
    lv_label_set_text(widget->header, "BATT");
    lv_obj_set_style_text_font(widget->header, &FG_Medium_20, LV_PART_MAIN);
    lv_obj_set_style_text_color(widget->header, lv_color_hex(BRF_COLOR_TEXT_DIM), LV_PART_MAIN);
    lv_obj_set_style_text_letter_space(widget->header, 2, LV_PART_MAIN);
    lv_obj_set_pos(widget->header, 0, 0);

    widget->value = lv_label_create(widget->obj);
    lv_label_set_text(widget->value, "000/000");
    lv_obj_set_style_text_font(widget->value, &DINish_Medium_24, LV_PART_MAIN);
    lv_obj_set_style_text_color(widget->value, lv_color_hex(BRF_COLOR_TEXT), LV_PART_MAIN);
    lv_obj_set_pos(widget->value, 0, 14);

    widget->bar_left = make_bar(widget->obj, 0, 40);
    widget->bar_right = make_bar(widget->obj, 0, 40 + BAR_HEIGHT + BAR_GAP);

    sys_slist_append(&widgets, &widget->node);
    widget_battery_bars_init();
    render(widget);
    return 0;
}

lv_obj_t *zmk_widget_battery_bars_obj(struct zmk_widget_battery_bars *widget) {
    return widget->obj;
}
