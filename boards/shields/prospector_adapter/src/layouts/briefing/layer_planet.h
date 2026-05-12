#pragma once

#include <lvgl.h>
#include <zephyr/sys/slist.h>

struct zmk_widget_layer_planet {
    sys_snode_t node;
    lv_obj_t *obj;
    lv_obj_t *layer_label;
    lv_obj_t *subtitle;
    lv_obj_t *planet;
    lv_obj_t *shadow;
    lv_obj_t *moon_left;
    lv_obj_t *moon_right;
};

int zmk_widget_layer_planet_init(struct zmk_widget_layer_planet *widget, lv_obj_t *parent);
lv_obj_t *zmk_widget_layer_planet_obj(struct zmk_widget_layer_planet *widget);
