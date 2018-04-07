#include "button_listener.h"

#include "board.h"

// debouncing
#define MIN_PRESS_DURATION 10

#define BUTTON_HOLD_DURATION 350

void init_button_state(button_state_t *state) {
    state->is_pressed = false;
    state->lockout    = false;
    state->action     = BUTTON_ACTION_NONE;
}

void update_button_state(button_state_t *state, bool newval) {
    state->action = BUTTON_ACTION_NONE;

    int duration = msTicks - state->press_start_ms;

    // on button down
    if (!state->is_pressed && newval) {
        state->is_pressed = true;
        state->press_start_ms = msTicks;
        state->lockout = false;
        duration = 0;

    // on button up
    } else if (state->is_pressed && !newval) {
        state->is_pressed = false;

        if (!state->lockout && duration > MIN_PRESS_DURATION) {
           state->action = BUTTON_ACTION_TAP;
        }

    }
    if (state->is_pressed && duration > BUTTON_HOLD_DURATION && !state->lockout) {
        state->lockout = true;
        state->action = BUTTON_ACTION_HOLD;
    }
}