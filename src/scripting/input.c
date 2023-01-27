/*
 * Open Surge Engine
 * input.c - scripting system: input object
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

#include <surgescript.h>
#include <stdint.h>
#include "../core/input.h"

/* input API */
static surgescript_var_t* fun_constructor(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_destructor(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_main(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_init(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_buttondown(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_buttonpressed(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_buttonreleased(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_simulatebutton(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_getenabled(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_setenabled(surgescript_object_t* object, const surgescript_var_t** param, int num_params);

/* button hashes: "up", "down", "left", "right", "fire1", "fire2", ..., "fire8" */
#define BUTTON_UP             UINT64_C(0x5979CA)        /* hash("up") */
#define BUTTON_DOWN           UINT64_C(0x17C95CD5D)     /* hash("down") */
#define BUTTON_LEFT           UINT64_C(0x17C9A03B0)     /* and so on... */
#define BUTTON_RIGHT          UINT64_C(0x3110494163)
#define BUTTON_FIRE1          UINT64_C(0x310F70497C)
#define BUTTON_FIRE2          UINT64_C(0x310F70497D)
#define BUTTON_FIRE3          UINT64_C(0x310F70497E)
#define BUTTON_FIRE4          UINT64_C(0x310F70497F)
#define BUTTON_FIRE5          UINT64_C(0x310F704980)
#define BUTTON_FIRE6          UINT64_C(0x310F704981)
#define BUTTON_FIRE7          UINT64_C(0x310F704982)
#define BUTTON_FIRE8          UINT64_C(0x310F704983)

/* misc */
static uint64_t hash(const char *str);
static const surgescript_heapptr_t IS_OWN_INPUT_POINTER = 0;


/*
 * scripting_register_input()
 * Register the object
 */
void scripting_register_input(surgescript_vm_t* vm)
{
    surgescript_vm_bind(vm, "Input", "state:main", fun_main, 0);
    surgescript_vm_bind(vm, "Input", "constructor", fun_constructor, 0);
    surgescript_vm_bind(vm, "Input", "destructor", fun_destructor, 0);
    surgescript_vm_bind(vm, "Input", "buttonDown", fun_buttondown, 1);
    surgescript_vm_bind(vm, "Input", "buttonPressed", fun_buttonpressed, 1);
    surgescript_vm_bind(vm, "Input", "buttonReleased", fun_buttonreleased, 1);
    surgescript_vm_bind(vm, "Input", "simulateButton", fun_simulatebutton, 2);
    surgescript_vm_bind(vm, "Input", "get_enabled", fun_getenabled, 0);
    surgescript_vm_bind(vm, "Input", "set_enabled", fun_setenabled, 1);
    surgescript_vm_bind(vm, "Input", "__init", fun_init, 1);
}

/* Console routines */

/* main state */
surgescript_var_t* fun_main(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    return NULL;
}

/* constructor */
surgescript_var_t* fun_constructor(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    surgescript_heap_t* heap = surgescript_object_heap(object);

    ssassert(IS_OWN_INPUT_POINTER == surgescript_heap_malloc(heap));
    surgescript_var_set_bool(surgescript_heap_at(heap, IS_OWN_INPUT_POINTER), false);

    /* We may accept an external an input_t* as this object's userdata */
    if(surgescript_object_userdata(object) == NULL) {
        input_t* input = input_create_user(NULL);
        surgescript_object_set_userdata(object, input);
        surgescript_var_set_bool(surgescript_heap_at(heap, IS_OWN_INPUT_POINTER), true);
    }

    return NULL;
}

/* destructor */
surgescript_var_t* fun_destructor(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    surgescript_heap_t* heap = surgescript_object_heap(object);

    /* Will destroy the input_t* only if we created it ourselves */
    if(surgescript_var_get_bool(surgescript_heap_at(heap, IS_OWN_INPUT_POINTER))) {
        input_t* input = (input_t*)surgescript_object_userdata(object);
        input_destroy(input);
    }

    return NULL;
}

/* __init(inputMap): set an input map */
surgescript_var_t* fun_init(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    surgescript_heap_t* heap = surgescript_object_heap(object);

    if(!surgescript_var_is_null(param[0])) {
        if(surgescript_var_get_bool(surgescript_heap_at(heap, IS_OWN_INPUT_POINTER))) {
            input_t* input = (input_t*)surgescript_object_userdata(object);
            const char* inputmap = surgescript_var_fast_get_string(param[0]);
            if(*inputmap != '\0')
                input_change_mapping((inputuserdefined_t*)input, inputmap);
        }
    }

    return NULL;
}

/* buttonDown(button): is the given button being held down?
 * valid buttons are: "up", "down", "left", "right", "fire1", "fire2", ..., "fire8"
 * for optimization reasons, it's mandatory: button must be of the string type */
surgescript_var_t* fun_buttondown(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    input_t* input = (input_t*)surgescript_object_userdata(object);
    const char* button = surgescript_var_fast_get_string(param[0]);
    switch(hash(button)) {
        case BUTTON_UP:     return surgescript_var_set_bool(surgescript_var_create(), input_button_down(input, IB_UP));
        case BUTTON_DOWN:   return surgescript_var_set_bool(surgescript_var_create(), input_button_down(input, IB_DOWN));
        case BUTTON_LEFT:   return surgescript_var_set_bool(surgescript_var_create(), input_button_down(input, IB_LEFT));
        case BUTTON_RIGHT:  return surgescript_var_set_bool(surgescript_var_create(), input_button_down(input, IB_RIGHT));
        case BUTTON_FIRE1:  return surgescript_var_set_bool(surgescript_var_create(), input_button_down(input, IB_FIRE1));
        case BUTTON_FIRE2:  return surgescript_var_set_bool(surgescript_var_create(), input_button_down(input, IB_FIRE2));
        case BUTTON_FIRE3:  return surgescript_var_set_bool(surgescript_var_create(), input_button_down(input, IB_FIRE3));
        case BUTTON_FIRE4:  return surgescript_var_set_bool(surgescript_var_create(), input_button_down(input, IB_FIRE4));
        case BUTTON_FIRE5:  return surgescript_var_set_bool(surgescript_var_create(), input_button_down(input, IB_FIRE5));
        case BUTTON_FIRE6:  return surgescript_var_set_bool(surgescript_var_create(), input_button_down(input, IB_FIRE6));
        case BUTTON_FIRE7:  return surgescript_var_set_bool(surgescript_var_create(), input_button_down(input, IB_FIRE7));
        case BUTTON_FIRE8:  return surgescript_var_set_bool(surgescript_var_create(), input_button_down(input, IB_FIRE8));
        default:            return surgescript_var_set_bool(surgescript_var_create(), false);
    }
}

/* buttonPressed(button): has the given button just been pressed?
 * valid buttons are: "up", "down", "left", "right", "fire1", "fire2", ..., "fire8"
 * for optimization reasons, it's mandatory: button must be of the string type */
surgescript_var_t* fun_buttonpressed(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    input_t* input = (input_t*)surgescript_object_userdata(object);
    const char* button = surgescript_var_fast_get_string(param[0]);
    switch(hash(button)) {
        case BUTTON_UP:     return surgescript_var_set_bool(surgescript_var_create(), input_button_pressed(input, IB_UP));
        case BUTTON_DOWN:   return surgescript_var_set_bool(surgescript_var_create(), input_button_pressed(input, IB_DOWN));
        case BUTTON_LEFT:   return surgescript_var_set_bool(surgescript_var_create(), input_button_pressed(input, IB_LEFT));
        case BUTTON_RIGHT:  return surgescript_var_set_bool(surgescript_var_create(), input_button_pressed(input, IB_RIGHT));
        case BUTTON_FIRE1:  return surgescript_var_set_bool(surgescript_var_create(), input_button_pressed(input, IB_FIRE1));
        case BUTTON_FIRE2:  return surgescript_var_set_bool(surgescript_var_create(), input_button_pressed(input, IB_FIRE2));
        case BUTTON_FIRE3:  return surgescript_var_set_bool(surgescript_var_create(), input_button_pressed(input, IB_FIRE3));
        case BUTTON_FIRE4:  return surgescript_var_set_bool(surgescript_var_create(), input_button_pressed(input, IB_FIRE4));
        case BUTTON_FIRE5:  return surgescript_var_set_bool(surgescript_var_create(), input_button_pressed(input, IB_FIRE5));
        case BUTTON_FIRE6:  return surgescript_var_set_bool(surgescript_var_create(), input_button_pressed(input, IB_FIRE6));
        case BUTTON_FIRE7:  return surgescript_var_set_bool(surgescript_var_create(), input_button_pressed(input, IB_FIRE7));
        case BUTTON_FIRE8:  return surgescript_var_set_bool(surgescript_var_create(), input_button_pressed(input, IB_FIRE8));
        default:            return surgescript_var_set_bool(surgescript_var_create(), false);
    }
}

/* buttonReleased(button): has the given button just been released?
 * valid buttons are: "up", "down", "left", "right", "fire1", "fire2", ..., "fire8"
 * for optimization reasons, it's mandatory: button must be of the string type */
surgescript_var_t* fun_buttonreleased(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    input_t* input = (input_t*)surgescript_object_userdata(object);
    const char* button = surgescript_var_fast_get_string(param[0]);
    switch(hash(button)) {
        case BUTTON_UP:     return surgescript_var_set_bool(surgescript_var_create(), input_button_up(input, IB_UP));
        case BUTTON_DOWN:   return surgescript_var_set_bool(surgescript_var_create(), input_button_up(input, IB_DOWN));
        case BUTTON_LEFT:   return surgescript_var_set_bool(surgescript_var_create(), input_button_up(input, IB_LEFT));
        case BUTTON_RIGHT:  return surgescript_var_set_bool(surgescript_var_create(), input_button_up(input, IB_RIGHT));
        case BUTTON_FIRE1:  return surgescript_var_set_bool(surgescript_var_create(), input_button_up(input, IB_FIRE1));
        case BUTTON_FIRE2:  return surgescript_var_set_bool(surgescript_var_create(), input_button_up(input, IB_FIRE2));
        case BUTTON_FIRE3:  return surgescript_var_set_bool(surgescript_var_create(), input_button_up(input, IB_FIRE3));
        case BUTTON_FIRE4:  return surgescript_var_set_bool(surgescript_var_create(), input_button_up(input, IB_FIRE4));
        case BUTTON_FIRE5:  return surgescript_var_set_bool(surgescript_var_create(), input_button_up(input, IB_FIRE5));
        case BUTTON_FIRE6:  return surgescript_var_set_bool(surgescript_var_create(), input_button_up(input, IB_FIRE6));
        case BUTTON_FIRE7:  return surgescript_var_set_bool(surgescript_var_create(), input_button_up(input, IB_FIRE7));
        case BUTTON_FIRE8:  return surgescript_var_set_bool(surgescript_var_create(), input_button_up(input, IB_FIRE8));
        default:            return surgescript_var_set_bool(surgescript_var_create(), false);
    }
}

/* simulateButton(button, down): simulates that a button is being held down/not down
 * valid buttons are: "up", "down", "left", "right", "fire1", "fire2", ..., "fire8"
 * for optimization reasons, it's mandatory: button must be of the string type */
surgescript_var_t* fun_simulatebutton(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    input_t* input = (input_t*)surgescript_object_userdata(object);
    const char* button = surgescript_var_fast_get_string(param[0]);
    bool down = surgescript_var_get_bool(param[1]);
    void (*simulate_button)(input_t*,inputbutton_t) = down ? input_simulate_button_down : input_simulate_button_up;

    switch(hash(button)) {
        case BUTTON_UP:     simulate_button(input, IB_UP); break;
        case BUTTON_DOWN:   simulate_button(input, IB_DOWN); break;
        case BUTTON_LEFT:   simulate_button(input, IB_LEFT); break;
        case BUTTON_RIGHT:  simulate_button(input, IB_RIGHT); break;
        case BUTTON_FIRE1:  simulate_button(input, IB_FIRE1); break;
        case BUTTON_FIRE2:  simulate_button(input, IB_FIRE2); break;
        case BUTTON_FIRE3:  simulate_button(input, IB_FIRE3); break;
        case BUTTON_FIRE4:  simulate_button(input, IB_FIRE4); break;
        case BUTTON_FIRE5:  simulate_button(input, IB_FIRE5); break;
        case BUTTON_FIRE6:  simulate_button(input, IB_FIRE6); break;
        case BUTTON_FIRE7:  simulate_button(input, IB_FIRE7); break;
        case BUTTON_FIRE8:  simulate_button(input, IB_FIRE8); break;
    }

    return NULL;
}

/* is the input object enabled? */
surgescript_var_t* fun_getenabled(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    input_t* input = (input_t*)surgescript_object_userdata(object);
    return surgescript_var_set_bool(surgescript_var_create(), !input_is_ignored(input));
}

/* enable or disable the input object */
surgescript_var_t* fun_setenabled(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    input_t* input = (input_t*)surgescript_object_userdata(object);
    bool enabled = surgescript_var_get_bool(param[0]);

    if(enabled) {
        if(input_is_ignored(input))
            input_restore(input);
    }
    else {
        if(!input_is_ignored(input))
            input_ignore(input);
    }

    return NULL;
}

/* -- private -- */

/* djb2 hash function */
uint64_t hash(const char *str)
{
    int c; uint64_t hash = 5381;

    while((c = *((unsigned char*)(str++))))
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */

    return hash;
}