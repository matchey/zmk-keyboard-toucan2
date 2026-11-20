#include <assert.h>
#include <stdio.h>

#include "../src/gesture_key_core.h"

static void test_repeat_accumulates_and_keeps_remainder(void) {
    struct gesture_key_config cfg = {
        .distance = 10,
        .fire_once = false,
        .idle_reset_ms = 0,
    };
    struct gesture_key_state st = gesture_key_state_init();

    struct gesture_key_result r = gesture_key_step(st, cfg, 4, 100);
    assert(r.direction == GESTURE_KEY_DIR_NONE);
    assert(r.count == 0);
    assert(r.next_state.accum == 4);

    r = gesture_key_step(r.next_state, cfg, 8, 120);
    assert(r.direction == GESTURE_KEY_DIR_POS);
    assert(r.count == 1);
    assert(r.next_state.accum == 2);

    r = gesture_key_step(r.next_state, cfg, 25, 140);
    assert(r.direction == GESTURE_KEY_DIR_POS);
    assert(r.count == 2);
    assert(r.next_state.accum == 7);
}

static void test_repeat_negative_direction(void) {
    struct gesture_key_config cfg = {
        .distance = 10,
        .fire_once = false,
        .idle_reset_ms = 0,
    };
    struct gesture_key_state st = gesture_key_state_init();

    struct gesture_key_result r = gesture_key_step(st, cfg, -15, 100);
    assert(r.direction == GESTURE_KEY_DIR_NEG);
    assert(r.count == 1);
    assert(r.next_state.accum == -5);

    r = gesture_key_step(r.next_state, cfg, -5, 110);
    assert(r.direction == GESTURE_KEY_DIR_NEG);
    assert(r.count == 1);
    assert(r.next_state.accum == 0);
}

static void test_zero_delta_does_not_fire(void) {
    struct gesture_key_config cfg = {
        .distance = 10,
        .fire_once = false,
        .idle_reset_ms = 500,
    };
    struct gesture_key_state st = gesture_key_state_init();

    struct gesture_key_result r = gesture_key_step(st, cfg, 5, 100);
    r = gesture_key_step(r.next_state, cfg, 0, 150);
    assert(r.direction == GESTURE_KEY_DIR_NONE);
    assert(r.count == 0);
    assert(r.next_state.accum == 5);
}

static void test_fire_once_triggers_only_once_per_gesture(void) {
    struct gesture_key_config cfg = {
        .distance = 10,
        .fire_once = true,
        .idle_reset_ms = 500,
    };
    struct gesture_key_state st = gesture_key_state_init();

    struct gesture_key_result r = gesture_key_step(st, cfg, 6, 100);
    assert(r.direction == GESTURE_KEY_DIR_NONE);
    assert(r.count == 0);

    r = gesture_key_step(r.next_state, cfg, 30, 120);
    assert(r.direction == GESTURE_KEY_DIR_POS);
    assert(r.count == 1);
    assert(r.next_state.fired);

    /* Continued movement within the same gesture must not re-fire. */
    r = gesture_key_step(r.next_state, cfg, 50, 300);
    assert(r.direction == GESTURE_KEY_DIR_NONE);
    assert(r.count == 0);

    /* Keep dragging past original timestamp + idle_reset_ms; still active. */
    r = gesture_key_step(r.next_state, cfg, -50, 700);
    assert(r.direction == GESTURE_KEY_DIR_NONE);
    assert(r.count == 0);
}

static void test_fire_once_negative_direction(void) {
    struct gesture_key_config cfg = {
        .distance = 10,
        .fire_once = true,
        .idle_reset_ms = 0,
    };
    struct gesture_key_state st = gesture_key_state_init();

    struct gesture_key_result r = gesture_key_step(st, cfg, -25, 100);
    assert(r.direction == GESTURE_KEY_DIR_NEG);
    assert(r.count == 1);
    assert(r.next_state.fired);
}

static void test_reset_rearms_once_and_clears_remainder(void) {
    struct gesture_key_config once_cfg = {
        .distance = 10,
        .fire_once = true,
        .idle_reset_ms = 0,
    };
    struct gesture_key_state st = gesture_key_state_init();

    struct gesture_key_result r = gesture_key_step(st, once_cfg, 10, 100);
    assert(r.count == 1);

    st = gesture_key_reset();
    r = gesture_key_step(st, once_cfg, -10, 110);
    assert(r.direction == GESTURE_KEY_DIR_NEG);
    assert(r.count == 1);

    struct gesture_key_config rep_cfg = {
        .distance = 10,
        .fire_once = false,
        .idle_reset_ms = 0,
    };
    r = gesture_key_step(gesture_key_state_init(), rep_cfg, 9, 200);
    assert(r.count == 0);
    st = gesture_key_reset();
    r = gesture_key_step(st, rep_cfg, 5, 210);
    assert(r.count == 0);
    assert(r.next_state.accum == 5);
}

static void test_idle_reset_clears_stale_state(void) {
    struct gesture_key_config cfg = {
        .distance = 10,
        .fire_once = true,
        .idle_reset_ms = 500,
    };
    struct gesture_key_state st = gesture_key_state_init();

    struct gesture_key_result r = gesture_key_step(st, cfg, 12, 1000);
    assert(r.count == 1);

    /* Below idle_reset_ms: remains fired. */
    r = gesture_key_step(r.next_state, cfg, 12, 1499);
    assert(r.count == 0);

    /* At or above idle_reset_ms from last event (1499): resets and fires. */
    r = gesture_key_step(r.next_state, cfg, -12, 1999);
    assert(r.direction == GESTURE_KEY_DIR_NEG);
    assert(r.count == 1);
}

static void test_invalid_distance_is_safe(void) {
    struct gesture_key_config cfg = {
        .distance = 0,
        .fire_once = false,
        .idle_reset_ms = 0,
    };
    struct gesture_key_result r = gesture_key_step(gesture_key_state_init(), cfg, 100, 100);
    assert(r.direction == GESTURE_KEY_DIR_NONE);
    assert(r.count == 0);
}

int main(void) {
    test_repeat_accumulates_and_keeps_remainder();
    test_repeat_negative_direction();
    test_zero_delta_does_not_fire();
    test_fire_once_triggers_only_once_per_gesture();
    test_fire_once_negative_direction();
    test_reset_rearms_once_and_clears_remainder();
    test_idle_reset_clears_stale_state();
    test_invalid_distance_is_safe();
    puts("gesture_key_core_test: all passed");
    return 0;
}
