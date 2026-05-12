#pragma once

#include <lvgl.h>
#include <zephyr/sys/slist.h>

struct zmk_widget_battery_bars {
    sys_snode_t node;
    lv_obj_t *obj;
    lv_obj_t *header;
    lv_obj_t *value;
    lv_obj_t *bar_left;
    lv_obj_t *bar_right;
};

int zmk_widget_battery_bars_init(struct zmk_widget_battery_bars *widget, lv_obj_t *parent);
lv_obj_t *zmk_widget_battery_bars_obj(struct zmk_widget_battery_bars *widget);
uint8_t briefing_battery_get(uint8_t source);
uint8_t briefing_battery_avg(void);
