// -----------------------------------------------------------------------------
// File: platformer.ss
// Description: platform movement behavior
// Author: Alexandre Martins <http://opensurge2d.org>
// License: MIT
// -----------------------------------------------------------------------------

/*
 * This is a SIMPLE behavior for a Platform Movement,
 * intended to be used mostly by NPCs and baddies.
 * It's supposed to be lightweight for the CPU,
 * and it doesn't support 360-physics.
 *
 * NOTE:
 *   For best results, the hot spot of the
 *   entity should be placed around its feet.
 *
 * Example:
 *

using SurgeEngine.Actor;
using SurgeEngine.Behaviors.Enemy;
using SurgeEngine.Behaviors.Platformer;

object "My Baddie" is "entity", "enemy"
{
    actor = Actor("My Baddie"); // handles the graphics
    enemy = Enemy(); // make it behave like a baddie
    platformer = Platformer().walk(); // make it walk

    state "main"
    {
        platformer.speed = 60; // 60 pixels per second
    }
}

 *
 * Have fun!
 *
 */

// -----------------------------------------------------------------------------
// The following code makes the Platform Movement work.
// Think twice before modifying this code!
// -----------------------------------------------------------------------------
using SurgeEngine.Collisions.Sensor;
using SurgeEngine.Vector2;
using SurgeEngine.Player;
using SurgeEngine.Level;

object "Platformer" is "behavior"
{
    public readonly entity = parent; // the entity associated with this behavior
    speed = 120; // speed in px/s
    public jumpSpeed = 400; // px/s
    grvmul = 1.0; // gravity multiplier
    xsp = 0;
    ysp = 0;
    topxsp = 360;
    topysp = 720;
    topjsp = 800;
    direction = 1;
    oldx = 0;
    ceiling = false;
    midair = false;
    wall = false;
    rwall = false;
    lwall = false;
    lledge = false;
    rledge = false;
    sensors = null;
    autowalk = null;
    transform = null;
    actor = null;

    state "main"
    {
        // setup
        if(transform == null)
            transform = entity.child("Transform") || entity.spawn("Transform");
        if(actor == null)
            actor = entity.child("Actor");
        if(sensors == null) {
            if(actor != null) // autodetect collision box
                setSensorBox(actor.width * 0.8, actor.height);
            else
                return; // no actor, no sensors
        }

        // warming up...
        dt = Time.delta;
        oldx = transform.position.x;

        // gravity
        if(sensors.midair) {
            grv = Level.gravity * grvmul;
            ysp += grv * dt;
            if(ysp > topysp)
                ysp = topysp;
        }

        // move
        transform.translateBy(xsp * dt, ysp * dt);
        sensors.update();

        // wall collision
        if(xsp != 0 && sensors.wall) {
            lwall = (xsp < 0);
            rwall = (xsp > 0);
            wall = true;
            xsp = 0;
            dx = oldx - transform.position.x;
            transform.translateBy(dx, 0);
            sensors.update();
            while(sensors.wall) {
                transform.translateBy(-direction, 0);
                sensors.update();
            }
        }
        else
            wall = lwall = rwall = false;

        // ceiling collision
        if(sensors.ceiling) {
            ceiling = true;
            ysp = Math.abs(ysp) * -0.25;
            while(sensors.ceiling) {
                transform.translateBy(0, 1);
                sensors.update();
            }
        }
        else
            ceiling = false;

        // ground collision
        if(!sensors.midair && (!sensors.cloud || ysp > 0)) {
            ysp = 0;
            midair = false;
            lledge = sensors.lledge;
            rledge = sensors.rledge;
            while(!sensors.midair) {
                transform.translateBy(0, -1);
                sensors.update();
            }
            transform.translateBy(0, 2);
        }
        else {
            midair = true;
            lledge = rledge = false;
        }

        // movement direction
        if(xsp != 0) {
            direction = (xsp > 0) ? 1 : -1;
            if(actor != null)
                actor.hflip = (xsp < 0);
        }
    }

    state "disabled"
    {
        // no movement
    }

    // constructor
    fun constructor()
    {
        // behavior validation
        if(!entity.hasTag("entity"))
            Application.crash("Object \"" + entity.__name + "\" must be tagged \"entity\" to use " + this.__name + ".");
    }

    // --- MODIFIERS ----

    // walk to the right
    fun walkRight()
    {
        xsp = Math.min(Math.abs(speed), topxsp);
        return this;
    }

    // walk to the left
    fun walkLeft()
    {
        xsp = Math.max(-Math.abs(speed), -topxsp);
        return this;
    }

    // enable automatic walking
    fun walk()
    {
        if(autowalk == null)
            autowalk = spawn("PlatformerAutoWalker");
        return this;
    }

    // stop walking
    fun stop()
    {
        if(autowalk != null) {
            autowalk.destroy();
            autowalk = null;
        }
        xsp = 0;
        return this;
    }

    // jump
    fun jump()
    {
        if(!midair && !ceiling)
            ysp = Math.max(-Math.abs(jumpSpeed), -topjsp);
        return this;
    }

    // force jump (useful for double jump, for example)
    fun forceJump(speed)
    {
        if(!ceiling)
            ysp = Math.max(-Math.abs(speed), -topjsp);
        return this;
    }

    // set the size of the sensor box
    fun setSensorBox(width, height)
    {
        sensors = spawn("PlatformerSensors").setSensorBox(width, height);
        //sensors.visible = true; // debug
        return this;
    }

    // show the internal sensors
    fun showSensors()
    {
        if(sensors != null)
            sensors.visible = true;
        return this;
    }

    // hide the internal sensors
    fun hideSensors()
    {
        if(sensors != null)
            sensors.visible = true;
        return this;
    }

    // --- PROPERTIES ---

    // enable/disable movement
    fun get_enabled()
    {
        return state != "disabled";
    }

    fun set_enabled(enabled)
    {
        state = enabled ? "main" : "disabled";
    }

    // get/set speed
    fun get_speed()
    {
        return speed;
    }

    fun set_speed(newSpeed)
    {
        speed = newSpeed;

        // update xsp if walking
        if(xsp > 0)
            walkRight();
        else if(xsp < 0)
            walkLeft();
    }

    // get/set gravity multiplier
    fun get_gravityMultiplier()
    {
        return grvmul;
    }

    fun set_gravityMultiplier(multiplier)
    {
        grvmul = Math.max(0, multiplier);
    }

    // direction is +1 if the platformer is facing
    // right, -1 if facing left
    fun get_direction()
    {
        return direction;
    }

    // am I walking?
    fun get_walking()
    {
        return xsp != 0;
    }

    // am I walking right?
    fun get_walkingRight()
    {
        return xsp > 0;
    }

    // am I walking left?
    fun get_walkingLeft()
    {
        return xsp < 0;
    }

    // am I midair?
    fun get_midair()
    {
        return midair;
    }

    // am I touching a wall?
    fun get_wall()
    {
        return wall;
    }

    // am I touching a wall to my left?
    fun get_leftWall()
    {
        return lwall;
    }

    // am I touching a wall to my right?
    fun get_rightWall()
    {
        return rwall;
    }

    // am I standing on a ledge to my left?
    fun get_leftLedge()
    {
        return lledge;
    }

    // am I standing on a ledge to my right?
    fun get_rightLedge()
    {
        return rledge;
    }

    // am I falling down?
    fun get_falling()
    {
        return midair && ysp > 0;
    }

    /*
    // __sensors
    fun get___sensors()
    {
        return sensors;
    }
    */
}

object "PlatformerSensors"
{
    // sensors
    public readonly left = null;
    public readonly right = null;
    public readonly mid = null;
    public readonly top = null;

    // status
    public readonly midair = false;
    public readonly wall = false;
    public readonly ceiling = false;
    public readonly lledge = false;
    public readonly rledge = false;
    public readonly ground = false;
    public readonly cloud = false;

    state "main"
    {
        assert(left != null && right != null && mid != null && top != null);
        updateStatus();
    }

    fun updateStatus()
    {
        l = left.status;
        r = right.status;
        m = mid.status;
        t = top.status;

        rledge = (l != null && r == null);
        lledge = (l == null && r != null);
        wall = (m == "solid");
        ceiling = (t == "solid");
        cloud = (l == "cloud" || r == "cloud");
        ground = (l == "solid" || r == "solid");
        midair = (!l && !r) || (cloud && m == "cloud");
    }

    fun update()
    {
        left.onTransformChange();
        right.onTransformChange();
        mid.onTransformChange();
        top.onTransformChange();
        updateStatus();
    }

    fun get_visible()
    {
        return mid != null && mid.visible;
    }

    fun set_visible(visible)
    {
        if(left != null && right != null && mid != null && top != null) {
            left.visible = visible;
            right.visible = visible;
            mid.visible = visible;
            top.visible = visible;
        }
    }

    fun setSensorBox(w, h)
    {
        left = Sensor(-w * 0.4, -h * 0.5, 1, h * 0.5 + 2);
        right = Sensor(w * 0.4, -h * 0.5, 1, h * 0.5 + 2);
        mid = Sensor(-w * 0.4 - 1, -h * 0.4, w * 0.8 + 3, 1);
        top = Sensor(-w * 0.4, -h, w * 0.8 + 1, 1);
        return this;
    }
}

object "PlatformerAutoWalker"
{
    platformer = parent;

    state "main"
    {
        if(platformer.direction >= 0)
            platformer.walkRight();
        else
            platformer.walkLeft();
        state = "autoWalk";
    }

    state "autoWalk"
    {
        rightWall = platformer.rightWall;
        leftWall = platformer.leftWall;

        if((!rightWall && leftWall) || (platformer.leftLedge && platformer.direction < 0))
            platformer.walkRight();
        else if((rightWall && !leftWall) || (platformer.rightLedge && platformer.direction > 0))
            platformer.walkLeft();
        else if(rightWall && leftWall)
            platformer.stop();
    }
}
