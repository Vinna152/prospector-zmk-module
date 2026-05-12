#pragma once

#include <lvgl.h>
#include <zephyr/sys/slist.h>

struct zmk_widget_cpm {
    sys_snode_t node;
    lv_obj_t *obj;
    lv_obj_t *header;
    lv_obj_t *value;
    lv_obj_t *unit;
};

int zmk_widget_cpm_init(struct zmk_widget_cpm *widget, lv_obj_t *parent);
lv_obj_t *zmk_widget_cpm_obj(struct zmk_widget_cpm *widget);
