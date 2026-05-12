#pragma once

#include <lvgl.h>
#include <zephyr/sys/slist.h>

struct zmk_widget_mods {
    sys_snode_t node;
    lv_obj_t *obj;
    lv_obj_t *header;
    lv_obj_t *labels[4];
};

int zmk_widget_mods_init(struct zmk_widget_mods *widget, lv_obj_t *parent);
lv_obj_t *zmk_widget_mods_obj(struct zmk_widget_mods *widget);
