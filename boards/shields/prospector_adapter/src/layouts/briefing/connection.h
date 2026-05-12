#pragma once

#include <lvgl.h>
#include <zephyr/sys/slist.h>

struct zmk_widget_connection {
    sys_snode_t node;
    lv_obj_t *obj;
    lv_obj_t *header;
    lv_obj_t *value;
};

int zmk_widget_connection_init(struct zmk_widget_connection *widget, lv_obj_t *parent);
lv_obj_t *zmk_widget_connection_obj(struct zmk_widget_connection *widget);
void briefing_connection_set(uint8_t source, bool connected);
bool briefing_connection_is_connected(uint8_t source);
uint8_t briefing_connected_count(void);
