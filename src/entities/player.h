/*
 * Open Surge Engine
 * player.h - player module
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
 *
 * Edits by Dalton Sterritt (copyright given to Alexandre):
 * player_enable_roll, player_disable_roll
 */

#ifndef _PLAYER_H
#define _PLAYER_H

#include "../core/v2d.h"
#include "../core/darray.h"
#include "brick.h"

/* constants */
extern const int PLAYER_INITIAL_LIVES;       /* initial lives */

/* shield types */
enum playershield_t {
    SH_NONE,            /* no shield */
    SH_SHIELD,          /* regular shield */
    SH_FIRESHIELD,      /* fire shield */
    SH_THUNDERSHIELD,   /* thunder shield */
    SH_WATERSHIELD,     /* water shield */
    SH_ACIDSHIELD,      /* acid shield */
    SH_WINDSHIELD       /* wind shield */
};

/* types & structs */
struct actor_t;
struct animation_t;
struct brick_list_t;
struct item_list_t;
struct enemy_list_t;
struct physicsactor_t;
struct obstaclemap_t;
struct character_t;
struct surgescript_object_t;
typedef struct player_t player_t;
typedef enum playershield_t playershield_t;

/* player structure */
struct player_t {
    /* general */
    char *name;
    struct actor_t *actor;
    int disable_movement;
    int disable_roll;
    int on_movable_platform;
    int disable_collectible_loss;
    int disable_animation_control;
    int aggressive;
    int got_glasses;

    /* shields */
    playershield_t shield_type;
    struct actor_t *shield;

    /* invincibility */
    int invincible;
    float invincibility_timer;
    struct actor_t** star;

    /* turbo */
    int turbo;
    float turbo_timer;

    /* loop system */
    bricklayer_t layer;

    /* private */
    struct physicsactor_t *pa;
    int pa_old_state;
    int underwater;
    float underwater_timer;
    float breath_time;
    int blinking;
    float blink_timer;
    float blink_visibility_timer;
    int thrown_while_rolling;
    int visible;
    const struct character_t* character;
    struct obstaclemap_t* obstaclemap;
    DARRAY(struct obstacle_t*, mock_obstacles);
};

/* public functions */
player_t* player_create(const char *character_name);
player_t* player_destroy(player_t *player);
void player_update(player_t *player, struct player_t **team, int team_size, struct brick_list_t *brick_list, struct item_list_t *item_list, struct enemy_list_t *enemy_list, struct surgescript_object_t* (*get_bricklike_object)(int));
void player_render(player_t *player, v2d_t camera_position);

void player_hit(player_t *player, float direction);
void player_hit_ex(player_t *player, const struct actor_t *hazard);
int player_bounce(player_t *player, float direction, int is_heavy_object);
int player_bounce_ex(player_t *player, const struct actor_t *hazard, int is_heavy_object);
void player_detach_from_ground(player_t *player);
void player_kill(player_t *player);
void player_spring(player_t *player);
void player_roll(player_t *player);
void player_enable_roll(player_t *player);
void player_disable_roll(player_t *player);
void player_lock_horizontally_for(player_t *player, float seconds);
int player_collision(const player_t *player, const struct actor_t *actor);
int player_overlaps(const player_t *player, int x, int y, int width, int height);
int player_senses_layer(const player_t* player, bricklayer_t layer);

void player_enter_water(player_t *player);
void player_leave_water(player_t *player);
void player_breathe(player_t *player);
void player_reset_underwater_timer(player_t *player);
void player_drown(player_t *player);
int player_is_underwater(const player_t *player);
float player_seconds_remaining_to_drown(const player_t *player);
void player_set_breath_time(player_t* player, float seconds);
float player_breath_time(const player_t* player);

int player_is_midair(const player_t *player);
int player_is_attacking(const player_t *player);
int player_is_blinking(const player_t *player);
void player_set_blinking(player_t* player, int blink);
int player_is_turbocharged(const player_t *player);
void player_set_turbo(player_t* player, int turbo);
int player_is_invincible(const player_t *player);
void player_set_invincible(player_t* player, int invincible);
playershield_t player_shield_type(const player_t* player);
void player_grant_shield(player_t* player, playershield_t shield_type);
int player_is_frozen(const player_t* player);
void player_set_frozen(player_t* player, int frozen);
bricklayer_t player_layer(const player_t* player);
void player_set_layer(player_t* player, bricklayer_t layer);
int player_is_visible(const player_t* player);
void player_set_visible(player_t* player, int visible);
int player_is_aggressive(const player_t* player);
void player_set_aggressive(player_t* player, int aggressive);

int player_is_stopped(const player_t *player);
int player_is_walking(const player_t *player);
int player_is_running(const player_t *player);
int player_is_jumping(const player_t *player);
int player_is_springing(const player_t *player);
int player_is_rolling(const player_t *player);
int player_is_charging(const player_t *player);
int player_is_pushing(const player_t *player);
int player_is_getting_hit(const player_t *player);
int player_is_dying(const player_t *player);
int player_is_braking(const player_t *player);
int player_is_at_ledge(const player_t *player);
int player_is_drowning(const player_t *player);
int player_is_breathing(const player_t *player);
int player_is_ducking(const player_t *player);
int player_is_looking_up(const player_t *player);
int player_is_waiting(const player_t *player);
int player_is_winning(const player_t *player);

const char* player_name(const player_t* player);
const struct animation_t* player_animation(const player_t* player);
void player_override_animation(player_t* player, const struct animation_t* animation);
const char* player_sprite_name(const player_t* player);
const char* player_companion_name(const player_t* player, int index);

int player_get_collectibles();
void player_set_collectibles(int c);
int player_get_lives();
void player_set_lives(int l);
int player_get_score();
void player_set_score(int s);

#endif
