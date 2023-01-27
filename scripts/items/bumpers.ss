// -----------------------------------------------------------------------------
// File: bumpers.ss
// Description: bumper script
// Author: Alexandre Martins <http://opensurge2d.org>
// License: MIT
// -----------------------------------------------------------------------------
using SurgeEngine.Actor;
using SurgeEngine.Level;
using SurgeEngine.Transform;
using SurgeEngine.Audio.Sound;
using SurgeEngine.Collisions.CollisionBall;

object "Bumper" is "entity", "basic"
{
    sfx = Sound("samples/bumper.wav");
    actor = Actor("Bumper");
    collider = CollisionBall(16);
    transform = Transform();
    bumpSpeed = 420;
    score = 100;

    state "main"
    {
        state = "idle";
    }

    state "idle"
    {
        actor.anim = 0;
    }

    state "active"
    {
        actor.anim = 1;
        if(actor.animation.finished)
            state = "idle";
    }

    fun constructor()
    {
        //collider.visible = true;
    }

    fun onCollision(otherCollider)
    {
        if(otherCollider.entity.hasTag("player"))
            bump(otherCollider.entity);
    }

    fun bump(player)
    {
        if(!player.dying) {
            // compute the bump vector
            v = transform.position
                .directionTo(player.transform.position)
                .scaledBy(bumpSpeed);

            // bump player
            player.speed = v.x;
            player.ysp = v.y;

            // play sound
            sfx.play();

            // add to score
            player.score += score;
            Level.spawnEntity("Score Text", collider.center).setText(score);

            // change state
            state = "active";
        }
    }
}