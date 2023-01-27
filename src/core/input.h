/*
 * Open Surge Engine
 * input.h - input management
 * Copyright (C) 2008-2022  Alexandre Martins <alemartf@gmail.com>
 * http://opensurge2d.org
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _INPUT_H
#define _INPUT_H

#include <stdbool.h>
#include "v2d.h"

/* forward declarations */
typedef enum inputbutton_t inputbutton_t;
typedef struct input_t input_t; /* input_t is the base class */
typedef struct inputmouse_t inputmouse_t; /* the following are derived from input_t */
typedef struct inputcomputer_t inputcomputer_t;
typedef struct inputuserdefined_t inputuserdefined_t;

/* constants */
#define MAX_JOYSTICK_BUTTONS 32 /* bit vector uint32_t */

/* available buttons */
enum inputbutton_t {
    IB_UP,      /* up */
    IB_DOWN,    /* down */
    IB_RIGHT,   /* right */
    IB_LEFT,    /* left */
    IB_FIRE1,   /* jump */
    IB_FIRE2,   /* switch character */
    IB_FIRE3,   /* pause */
    IB_FIRE4,   /* quit */
    IB_FIRE5,
    IB_FIRE6,
    IB_FIRE7,
    IB_FIRE8,
    /* --- */
    IB_MAX      /* number of buttons */
};

/* public methods */
void input_init();
void input_update();
void input_release();

bool input_is_joystick_available(); /* a joystick is available, but not necessarily enabled */
bool input_is_joystick_enabled(); /* a joystick is available and enabled (i.e., not ignored) */
bool input_is_joystick_ignored();
void input_ignore_joystick(bool ignore); /* ignores the input received from joysticks (if they're available) */
int input_number_of_joysticks();

input_t *input_create_user(const char* inputmap_name); /* user's custom input device (set inputmap_name to NULL to use a default mapping) */
input_t *input_create_computer(); /* computer-controlled "input": will return an inputcomputer_t*, which is also an input_t* */
input_t *input_create_mouse(); /* mouse */
void input_destroy(input_t *in);

bool input_button_down(input_t *in, inputbutton_t button);
bool input_button_pressed(input_t *in, inputbutton_t button);
bool input_button_up(input_t *in, inputbutton_t button);
void input_simulate_button_down(input_t *in, inputbutton_t button);
void input_simulate_button_up(input_t *in, inputbutton_t button);
void input_reset(input_t *in);
void input_ignore(input_t *in);
void input_restore(input_t *in);
bool input_is_ignored(input_t *in);

/* these will only work for a mouse input device */
v2d_t input_get_xy(inputmouse_t *in);

/* the following will only work for a user customized input device */
void input_change_mapping(inputuserdefined_t *in, const char* inputmap_name); /* set inputmap_name to NULL to use a default mapping */
const char* input_get_mapping_name(inputuserdefined_t *in);

#endif