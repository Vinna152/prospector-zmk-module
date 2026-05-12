#include "connection.h"

#include <zmk/ble.h>
#include <zmk/display.h>
#include <zmk/event_manager.h>
#include <zmk/events/split_central_status_changed.h>

#include <fonts.h>
#include "colors.h"

static sys_slist_t widgets = SYS_SLIST_STATIC_INIT(&widgets);

#ifndef PERIPHERAL_COUNT
#define PERIPHERAL_COUNT ZMK_SPLIT_BLE_PERIPHERAL_COUNT
#endif

static bool g_connected[PERIPHERAL_COUNT] = {false};

void briefing_connection_set(uint8_t source, bool connected) {
    if (source < PERIPHERAL_COUNT) {
        g_connected[source] = connected;
    }
}

bool briefing_connection_is_connected(uint8_t source) {
    if (source >= PERIPHERAL_COUNT) return false;
    return g_connected[source];
}

uint8_t briefing_connected_count(void) {
    uint8_t n = 0;
    for (int i = 0; i < PERIPHERAL_COUNT; i++) {
        if (g_connected[i]) n++;
    }
    return n;
}

static void render(struct zmk_widget_connection *w) {
    uint8_t n = briefing_connected_count();
    const char *text;
    uint32_t color;
    if (n >= PERIPHERAL_COUNT) {
        text = "ACTIVE";
        color = BRF_COLOR_ACCENT;
    } else if (n == 0) {
        text = "OFFLINE";
        color = BRF_COLOR_TEXT_DIM;
    } else {
        text = "INTERFERENCE";
        color = BRF_COLOR_WARN;
    }
    lv_label_set_text(w->value, text);
    lv_obj_set_style_text_color(w->value, lv_color_hex(color), LV_PART_MAIN);
}

static int conn_listener(const zmk_event_t *eh) {
    const struct zmk_split_central_status_changed *ev = as_zmk_split_central_status_changed(eh);
    if (ev) {
        briefing_connection_set(ev->slot, ev->connected);
        struct zmk_widget_connection *w;
        SYS_SLIST_FOR_EACH_CONTAINER(&widgets, w, node) {
            render(w);
        }
    }
    return ZMK_EV_EVENT_BUBBLE;
}

ZMK_LISTENER(widget_connection, conn_listener);
ZMK_SUBSCRIPTION(widget_connection, zmk_split_central_status_changed);

int zmk_widget_connection_init(struct zmk_widget_connection *widget, lv_obj_t *parent) {
    widget->obj = lv_obj_create(parent);
    lv_obj_set_size(widget->obj, 100, 40);
    lv_obj_set_style_bg_opa(widget->obj, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_border_width(widget->obj, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_all(widget->obj, 0, LV_PART_MAIN);

    widget->header = lv_label_create(widget->obj);
    lv_label_set_text(widget->header, "CONNECTION");
    lv_obj_set_style_text_font(widget->header, &FG_Medium_20, LV_PART_MAIN);
    lv_obj_set_style_text_color(widget->header, lv_color_hex(BRF_COLOR_TEXT_DIM), LV_PART_MAIN);
    lv_obj_set_style_text_letter_space(widget->header, 2, LV_PART_MAIN);
    lv_obj_set_pos(widget->header, 0, 0);

    widget->value = lv_label_create(widget->obj);
    lv_obj_set_style_text_font(widget->value, &DINish_Medium_24, LV_PART_MAIN);
    lv_obj_set_style_text_letter_space(widget->value, 2, LV_PART_MAIN);
    lv_obj_set_pos(widget->value, 0, 14);

    sys_slist_append(&widgets, &widget->node);
    render(widget);
    return 0;
}

lv_obj_t *zmk_widget_connection_obj(struct zmk_widget_connection *widget) {
    return widget->obj;
}
