#include "cpm.h"

#include <stdio.h>
#include <zephyr/kernel.h>
#include <zmk/display.h>
#include <zmk/event_manager.h>
#include <zmk/events/wpm_state_changed.h>
#include <zmk/wpm.h>

#include <fonts.h>
#include "colors.h"

static sys_slist_t widgets = SYS_SLIST_STATIC_INIT(&widgets);

static struct k_work_delayable smooth_work;
static float displayed = 0.0f;
static float target = 0.0f;
static const float K_UP = 0.30f;
static const float K_DOWN = 0.05f;

struct cpm_state {
    uint8_t wpm;
};

static void render(int cpm) {
    char buf[5];
    snprintf(buf, sizeof(buf), "%03d", cpm > 999 ? 999 : cpm);
    struct zmk_widget_cpm *w;
    SYS_SLIST_FOR_EACH_CONTAINER(&widgets, w, node) {
        lv_label_set_text(w->value, buf);
    }
}

static void smooth_work_handler(struct k_work *work) {
    float diff = target - displayed;
    bool at_target = diff > -0.5f && diff < 0.5f;

    int old_int = (int)(displayed + 0.5f);
    if (at_target) {
        displayed = target;
    } else {
        displayed += diff * (diff > 0 ? K_UP : K_DOWN);
    }
    int new_int = (int)(displayed + 0.5f);

    if (new_int != old_int) {
        render(new_int * 5);
    }

    if (!at_target) {
        k_work_schedule(&smooth_work, K_MSEC(50));
    }
}

static void cpm_update_cb(struct cpm_state state) {
    target = (float)state.wpm;
    k_work_schedule(&smooth_work, K_NO_WAIT);
}

static struct cpm_state cpm_get_state(const zmk_event_t *eh) {
    return (struct cpm_state){.wpm = zmk_wpm_get_state()};
}

ZMK_DISPLAY_WIDGET_LISTENER(widget_cpm, struct cpm_state, cpm_update_cb, cpm_get_state)
ZMK_SUBSCRIPTION(widget_cpm, zmk_wpm_state_changed);

int zmk_widget_cpm_init(struct zmk_widget_cpm *widget, lv_obj_t *parent) {
    widget->obj = lv_obj_create(parent);
    lv_obj_set_size(widget->obj, 80, 40);
    lv_obj_set_style_bg_opa(widget->obj, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_border_width(widget->obj, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_all(widget->obj, 0, LV_PART_MAIN);

    widget->header = lv_label_create(widget->obj);
    lv_label_set_text(widget->header, "OUTPUT");
    lv_obj_set_style_text_font(widget->header, &FG_Medium_20, LV_PART_MAIN);
    lv_obj_set_style_text_color(widget->header, lv_color_hex(BRF_COLOR_TEXT_DIM), LV_PART_MAIN);
    lv_obj_set_style_text_letter_space(widget->header, 2, LV_PART_MAIN);
    lv_obj_set_pos(widget->header, 0, 0);

    widget->value = lv_label_create(widget->obj);
    lv_label_set_text(widget->value, "000");
    lv_obj_set_style_text_font(widget->value, &DINish_Medium_24, LV_PART_MAIN);
    lv_obj_set_style_text_color(widget->value, lv_color_hex(BRF_COLOR_TEXT), LV_PART_MAIN);
    lv_obj_set_pos(widget->value, 0, 14);

    widget->unit = lv_label_create(widget->obj);
    lv_label_set_text(widget->unit, "CPM");
    lv_obj_set_style_text_font(widget->unit, &FG_Medium_20, LV_PART_MAIN);
    lv_obj_set_style_text_color(widget->unit, lv_color_hex(BRF_COLOR_TEXT_DIM), LV_PART_MAIN);
    lv_obj_set_style_text_letter_space(widget->unit, 2, LV_PART_MAIN);
    lv_obj_set_pos(widget->unit, 50, 22);

    sys_slist_append(&widgets, &widget->node);
    widget_cpm_init();
    k_work_init_delayable(&smooth_work, smooth_work_handler);
    return 0;
}

lv_obj_t *zmk_widget_cpm_obj(struct zmk_widget_cpm *widget) {
    return widget->obj;
}
