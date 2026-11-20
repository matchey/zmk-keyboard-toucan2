#define DT_DRV_COMPAT zmk_input_processor_gesture_key

#include <zephyr/device.h>
#include <zephyr/dt-bindings/input/input-event-codes.h>
#include <zephyr/input/input.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include <drivers/input_processor.h>
#include <zmk/behavior.h>
#include <zmk/keymap.h>

#include "gesture_key_core.h"

LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

struct zip_gesture_key_config {
    uint16_t event_code;
    struct gesture_key_config core;
    struct zmk_behavior_binding neg_binding;
    struct zmk_behavior_binding pos_binding;
};

struct zip_gesture_key_data {
    struct gesture_key_state state;
};

static int gesture_key_handle_event(const struct device *dev, struct input_event *event,
                                    uint32_t param1, uint32_t param2,
                                    struct zmk_input_processor_state *state) {
    ARG_UNUSED(param1);
    ARG_UNUSED(param2);
    ARG_UNUSED(state);

    const struct zip_gesture_key_config *cfg = dev->config;
    struct zip_gesture_key_data *data = dev->data;

    /* Reset on finger lift without consuming the event so downstream processors still see it. */
    if (event->type == INPUT_EV_KEY && event->code == INPUT_BTN_TOUCH && event->value == 0) {
        data->state = gesture_key_reset();
        return 0;
    }

    if (event->type != INPUT_EV_REL || event->code != cfg->event_code) {
        return 0;
    }

    int64_t now = k_uptime_get();
    struct gesture_key_result res =
        gesture_key_step(data->state, cfg->core, (int32_t)event->value, now);
    data->state = res.next_state;

    if (res.count > 0 && res.direction != GESTURE_KEY_DIR_NONE) {
        const struct zmk_behavior_binding *binding =
            (res.direction == GESTURE_KEY_DIR_POS) ? &cfg->pos_binding : &cfg->neg_binding;

        for (uint32_t i = 0; i < res.count; i++) {
            struct zmk_behavior_binding_event bev = {
                .position = INT32_MAX,
                .timestamp = now,
#if IS_ENABLED(CONFIG_ZMK_SPLIT)
                .source = ZMK_POSITION_STATE_CHANGE_SOURCE_LOCAL,
#endif
            };
            zmk_behavior_invoke_binding(binding, bev, true);
            zmk_behavior_invoke_binding(binding, bev, false);
        }
    }

    /* Zero and stop so converted relative motion never leaks to pointer or scroll HID reports. */
    event->value = 0;
    return ZMK_INPUT_PROC_STOP;
}

static const struct zmk_input_processor_driver_api gesture_key_driver_api = {
    .handle_event = gesture_key_handle_event,
};

#define ZIP_GESTURE_KEY_INST(n)                                                                    \
    BUILD_ASSERT(DT_INST_PROP_LEN(n, bindings) == 2,                                               \
                 "zmk,input-processor-gesture-key requires exactly 2 bindings (neg, pos)");        \
    BUILD_ASSERT(DT_INST_PROP(n, distance) > 0, "distance must be greater than 0");                \
    static const struct zip_gesture_key_config zip_gesture_key_config_##n = {                      \
        .event_code = DT_INST_PROP(n, event_code),                                                 \
        .core =                                                                                    \
            {                                                                                      \
                .distance = DT_INST_PROP(n, distance),                                             \
                .fire_once = DT_INST_PROP(n, fire_once),                                           \
                .idle_reset_ms = DT_INST_PROP(n, idle_reset_ms),                                   \
            },                                                                                     \
        .neg_binding = ZMK_KEYMAP_EXTRACT_BINDING(0, DT_DRV_INST(n)),                              \
        .pos_binding = ZMK_KEYMAP_EXTRACT_BINDING(1, DT_DRV_INST(n)),                              \
    };                                                                                             \
    static struct zip_gesture_key_data zip_gesture_key_data_##n;                                   \
    DEVICE_DT_INST_DEFINE(n, NULL, NULL, &zip_gesture_key_data_##n, &zip_gesture_key_config_##n,   \
                          POST_KERNEL, CONFIG_KERNEL_INIT_PRIORITY_DEFAULT,                        \
                          &gesture_key_driver_api);

DT_INST_FOREACH_STATUS_OKAY(ZIP_GESTURE_KEY_INST)
