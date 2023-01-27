// -----------------------------------------------------------------------------
// File: spring_booster.ss
// Description: spring booster script
// Author: Alexandre Martins <http://opensurge2d.org>
// License: MIT
// -----------------------------------------------------------------------------
using SurgeEngine.Actor;
using SurgeEngine.Brick;
using SurgeEngine.Player;
using SurgeEngine.Vector2;
using SurgeEngine.Audio.Sound;
using SurgeEngine.Collisions.CollisionBox;

object "Spring Booster Right" is "entity", "basic"
{
    booster = spawn("Spring Booster").setDirection(1);

    state "main"
    {
    }

    fun onReset()
    {
        // "Spring Booster" is not spawned via the editor;
        // delegate responsibility
        booster.onReset();
    }
}

object "Spring Booster Left" is "entity", "basic"
{
    booster = spawn("Spring Booster").setDirection(-1);

    state "main"
    {
    }

    fun onReset()
    {
        booster.onReset();
    }
}

object "Spring Booster" is "private", "entity"
{
    boostSpeed = 960; // pixels per second
    actor = Actor("Spring Booster");
    brick = Brick("Spring Booster Block");
    sensorCollider = CollisionBox(96, 40);
    springCollider = CollisionBox(16, 32);
    boostSfx = Sound("samples/spring.wav");
    appearSfx = Sound("samples/trotada.wav");
    direction = 1; // 1: right, -1: left

    state "main"
    {
        springCollider.setAnchor(1 - (1 + direction) / 2, 1);
        brick.offset = Vector2(-16 * direction, 0);
        brick.enabled = false;
        changeAnimation(3);
        state = "invisible";
    }

    state "invisible"
    {
    }

    state "appearing"
    {
        if(actor.animation.finished) {
            changeAnimation(0);
            state = "visible";
        }
    }

    state "visible"
    {
    }

    state "boosting"
    {
        if(actor.animation.finished) {
            changeAnimation(0);
            state = "visible";
        }
    }

    fun setDirection(dir)
    {
        direction = Math.sign(dir);
        return this;
    }

    fun constructor()
    {
        brick.type = "cloud";
        sensorCollider.setAnchor(0.5, 1);
        springCollider.setAnchor(0, 1);
        //sensorCollider.visible = true;
        //springCollider.visible = true;
    }

    fun appear()
    {
        brick.enabled = true;
        appearSfx.play();
        changeAnimation(1);
        state = "appearing";
    }

    fun boost(player)
    {
        // change speed
        oldSpeed = Math.abs(player.speed);
        if(boostSpeed > oldSpeed)
            player.speed = direction * boostSpeed;

        // prevent braking
        player.hlock(0.27);

        // misc
        boostSfx.play();
        changeAnimation(2);
        state = "boosting";
    }

    fun changeAnimation(baseID)
    {
        actor.anim = baseID + ((1 - direction) / 2) * 5;
    }

    fun onCollision(otherCollider)
    {
        if(otherCollider.entity.hasTag("player")) {
            if(otherCollider.collidesWith(springCollider)) {
                if(state != "invisible")
                    boost(otherCollider.entity);
            }
            else if(state == "invisible")
                appear();
        }
    }

    fun onReset()
    {
        state = "main";
    }
}

object "Spring Booster Up" is "entity", "basic"
{
    boostSpeed = 960; // pixels per second
    actor = Actor("Spring Booster Up");
    springCollider = CollisionBox(32, 16).setAnchor(0.5, 2);
    sensorCollider = CollisionBox(32, 64).setAnchor(0.5, 0.5);
    boostSfx = Sound("samples/spring.wav");
    appearSfx = Sound("samples/trotada.wav");

    state "main"
    {
        actor.anim = 3;
        //springCollider.visible = true;
        //sensorCollider.visible = true;
        state = "invisible";
    }

    state "invisible"
    {
        actor.anim = 3;
    }

    state "appearing"
    {
        actor.anim = 1;
        if(actor.animation.finished)
            state = "visible";
    }

    state "visible"
    {
        actor.anim = 0;
    }

    state "boosting"
    {
        actor.anim = 2;
        if(actor.animation.finished)
            state = "visible";
    }

    fun appear()
    {
        appearSfx.play();
        state = "appearing";
    }

    fun boost(player)
    {
        // nothing to do
        if(state == "boosting")
            return;

        // check if we need to activate the spring
        if(!player.hit && !player.dying) {
            slope = Math.deg2rad(player.slope);
            if(Math.abs(Math.cos(slope)) < 0.1) {
                // we're running on a wall
                speed = boostSpeed * Math.sign(Math.sin(slope));
                if((speed > 0 && speed > player.gsp) || (speed < 0 && speed < player.gsp))
                    player.gsp = speed; // change gsp only if the spring increases the speed
                boostSfx.play();
                state = "boosting";
            }
            else if(player.midair && player.ysp > 0) {
                // we're midair
                player.ysp = -boostSpeed;
                player.springify();
                boostSfx.play();
                state = "boosting";
            }
        }
    }

    fun onCollision(otherCollider)
    {
        if(otherCollider.entity.hasTag("player")) {
            if(otherCollider.collidesWith(springCollider) && state != "invisible") {
                if(state != "invisible")
                    boost(otherCollider.entity);
            }
            else if(state == "invisible")
                appear();
        }
    }

    fun onReset()
    {
        state = "main";
    }
}