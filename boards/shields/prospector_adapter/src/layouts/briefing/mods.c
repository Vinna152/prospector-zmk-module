#include "mods.h"

#include <zmk/display.h>
#include <zmk/event_manager.h>
#include <zmk/events/keycode_state_changed.h>
#include <zmk/hid.h>

#include <fonts.h>
#include "colors.h"

static sys_slist_t widgets = SYS_SLIST_STATIC_INIT(&widgets);

static const char *const MOD_TEXT[4] = {"CTL", "ALT", "SFT", "GUI"};

struct mods_state {
    bool active[4];
};

static void mods_update_cb(struct mods_state state) {
    struct zmk_widget_mods *w;
    SYS_SLIST_FOR_EACH_CONTAINER(&widgets, w, node) {
        for (int i = 0; i < 4; i++) {
            lv_color_t c = state.active[i] ? lv_color_hex(BRF_COLOR_ACCENT)
                                           : lv_color_hex(BRF_COLOR_TEXT_DIM);
            lv_obj_set_style_text_color(w->labels[i], c, LV_PART_MAIN);
        }
    }
}

static struct mods_state mods_get_state(const zmk_event_t *eh) {
    zmk_mod_flags_t mods = zmk_hid_get_explicit_mods();
    struct mods_state s = {0};
    s.active[0] = (mods & (MOD_LCTL | MOD_RCTL)) != 0;
    s.active[1] = (mods & (MOD_LALT | MOD_RALT)) != 0;
    s.active[2] = (mods & (MOD_LSFT | MOD_RSFT)) != 0;
    s.active[3] = (mods & (MOD_LGUI | MOD_RGUI)) != 0;
    return s;
}

ZMK_DISPLAY_WIDGET_LISTENER(widget_mods, struct mods_state, mods_update_cb, mods_get_state)
ZMK_SUBSCRIPTION(widget_mods, zmk_keycode_state_changed);

int zmk_widget_mods_init(struct zmk_widget_mods *widget, lv_obj_t *parent) {
    widget->obj = lv_obj_create(parent);
    lv_obj_set_size(widget->obj, 240, 20);
    lv_obj_set_style_bg_opa(widget->obj, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_border_width(widget->obj, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_all(widget->obj, 0, LV_PART_MAIN);

    widget->header = lv_label_create(widget->obj);
    lv_label_set_text(widget->header, "MODS");
    lv_obj_set_style_text_font(widget->header, &FG_Medium_20, LV_PART_MAIN);
    lv_obj_set_style_text_color(widget->header, lv_color_hex(BRF_COLOR_TEXT_DIM), LV_PART_MAIN);
    lv_obj_set_style_text_letter_space(widget->header, 2, LV_PART_MAIN);
    lv_obj_set_pos(widget->header, 0, 0);

    int x = 60;
    for (int i = 0; i < 4; i++) {
        widget->labels[i] = lv_label_create(widget->obj);
        lv_label_set_text(widget->labels[i], MOD_TEXT[i]);
        lv_obj_set_style_text_font(widget->labels[i], &FG_Medium_20, LV_PART_MAIN);
        lv_obj_set_style_text_color(widget->labels[i], lv_color_hex(BRF_COLOR_TEXT_DIM), LV_PART_MAIN);
        lv_obj_set_style_text_letter_space(widget->labels[i], 2, LV_PART_MAIN);
        lv_obj_set_pos(widget->labels[i], x, 0);
        x += 46;
    }

    sys_slist_append(&widgets, &widget->node);
    widget_mods_init();
    return 0;
}

lv_obj_t *zmk_widget_mods_obj(struct zmk_widget_mods *widget) {
    return widget->obj;
}
