#pragma once

#include <lvgl.h>
#include <zephyr/sys/slist.h>

struct zmk_widget_top_bar {
    sys_snode_t node;
    lv_obj_t *obj;
    lv_obj_t *briefing_label;
    lv_obj_t *briefing_diamond;
    lv_obj_t *coords_label;
};

int zmk_widget_top_bar_init(struct zmk_widget_top_bar *widget, lv_obj_t *parent);
lv_obj_t *zmk_widget_top_bar_obj(struct zmk_widget_top_bar *widget);
