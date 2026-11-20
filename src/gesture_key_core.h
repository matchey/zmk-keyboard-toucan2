#ifndef TOUCAN_GESTURE_KEY_CORE_H_
#define TOUCAN_GESTURE_KEY_CORE_H_

#include <stdbool.h>
#include <stdint.h>

enum gesture_key_direction {
    GESTURE_KEY_DIR_NONE = 0,
    GESTURE_KEY_DIR_NEG,
    GESTURE_KEY_DIR_POS,
};

struct gesture_key_config {
    int32_t distance;
    bool fire_once;
    int32_t idle_reset_ms;
};

struct gesture_key_state {
    int32_t accum;
    bool fired;
    bool has_last_event;
    int64_t last_event_ms;
};

struct gesture_key_result {
    struct gesture_key_state next_state;
    enum gesture_key_direction direction;
    uint32_t count;
};

static inline struct gesture_key_state gesture_key_state_init(void) {
    struct gesture_key_state st = {
        .accum = 0,
        .fired = false,
        .has_last_event = false,
        .last_event_ms = 0,
    };
    return st;
}

static inline struct gesture_key_state gesture_key_reset(void) {
    return gesture_key_state_init();
}

static inline struct gesture_key_result gesture_key_step(struct gesture_key_state state,
                                                         struct gesture_key_config cfg,
                                                         int32_t delta, int64_t now_ms) {
    struct gesture_key_result res = {
        .next_state = state,
        .direction = GESTURE_KEY_DIR_NONE,
        .count = 0,
    };

    if (cfg.distance <= 0 || delta == 0) {
        return res;
    }

    if (cfg.idle_reset_ms > 0 && state.has_last_event &&
        (now_ms - state.last_event_ms) >= cfg.idle_reset_ms) {
        state.accum = 0;
        state.fired = false;
    }

    /* Refresh timestamp even while fired so sustained dragging does not time out mid-gesture. */
    state.has_last_event = true;
    state.last_event_ms = now_ms;

    if (cfg.fire_once && state.fired) {
        res.next_state = state;
        return res;
    }

    int64_t sum = (int64_t)state.accum + (int64_t)delta;

    if (cfg.fire_once) {
        if (sum >= cfg.distance) {
            state.accum = 0;
            state.fired = true;
            res.next_state = state;
            res.direction = GESTURE_KEY_DIR_POS;
            res.count = 1;
            return res;
        }
        if (sum <= -(int64_t)cfg.distance) {
            state.accum = 0;
            state.fired = true;
            res.next_state = state;
            res.direction = GESTURE_KEY_DIR_NEG;
            res.count = 1;
            return res;
        }
        state.accum = (int32_t)sum;
        res.next_state = state;
        return res;
    }

    int64_t steps = sum / cfg.distance;
    state.accum = (int32_t)(sum % cfg.distance);
    res.next_state = state;

    if (steps > 0) {
        res.direction = GESTURE_KEY_DIR_POS;
        res.count = (uint32_t)steps;
    } else if (steps < 0) {
        res.direction = GESTURE_KEY_DIR_NEG;
        res.count = (uint32_t)(-steps);
    }

    return res;
}

#endif /* TOUCAN_GESTURE_KEY_CORE_H_ */
