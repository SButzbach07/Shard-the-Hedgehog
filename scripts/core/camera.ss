// -----------------------------------------------------------------------------
// File: camera.ss
// Description: the default camera for Open Surge
// Author: Alexandre Martins <http://opensurge2d.org>
// License: MIT
// -----------------------------------------------------------------------------
using SurgeEngine.Transform;
using SurgeEngine.Player;
using SurgeEngine.Camera;
using SurgeEngine.Actor;
using SurgeEngine.Level;
using SurgeEngine.Vector2;
using SurgeEngine.Video.Screen;

object "Default Camera" is "entity", "awake", "private"
{
    public offset = Vector2.zero; // an adjustable offset
    public enabled = true; // is this camera enabled?

    transform = Transform();
    actor = Actor("Default Camera");
    upDown = spawn("Default Camera - Up/Down Logic");
    player = Player.active;

    state "main"
    {
        // compute the delta
        delta = player.transform.position.minus(transform.position);

        // move within a box
        if(delta.x > 8)
            transform.translateBy(Math.min(16, delta.x - 8), 0);
        else if(delta.x < -8)
            transform.translateBy(Math.max(-16, delta.x + 8), 0);
        if(player.midair || player.frozen) {
            if(delta.y > 32)
                transform.translateBy(0, Math.min(16, delta.y - 32));
            else if(delta.y < -32)
                transform.translateBy(0, Math.max(-16, delta.y + 32));
        }
        else {
            dy = Math.abs(player.gsp) >= 360 ? 16 : 6;
            if(delta.y >= 1)
                transform.translateBy(0, Math.min(dy, delta.y - 1));
            else if(delta.y <= -1)
                transform.translateBy(0, Math.max(-dy, delta.y + 1));
        }

        // switched player?
        if(Player.active != player) {
            player = Player.active;
            centerCamera(player.transform.position);
        }

        // camera too far away? (e.g., player teleported)
        if(delta.length > 2 * Screen.width)
            centerCamera(player.transform.position);

        // freeze camera
        if(player.dying || Level.cleared)
            state = "frozen";

        // update camera
        refresh();
    }

    state "frozen"
    {
        // update camera
        refresh();
    }

    fun constructor()
    {
        centerCamera(player.transform.position);
        actor.zindex = 9999;
        actor.visible = false;
    }

    fun centerCamera(position)
    {
        transform.position = position;
    }

    fun showGizmo()
    {
        actor.visible = true;
    }

    fun hideGizmo()
    {
        actor.visible = false;
    }

    fun translate(offset)
    {
        transform.translate(offset);
    }

    fun refresh()
    {
        if(!enabled)
            return;

        if(state == "frozen") {
            Camera.position = transform.position.plus(offset);
            return;
        }

        Camera.position = transform.position.translatedBy(offset.x, offset.y + upDown.offset);
    }
}

object "Default Camera - Up/Down Logic"
{
    public readonly offset = 0;
    player = Player.active;
    speed = 120; // move at 120 px/s

    state "main"
    {
        player = Player.active;
        if(player.lookingUp)
            state = "wait up";
        else if(player.crouchingDown)
            state = "wait down";
        else
            moveBackToZero();
    }

    state "wait up"
    {
        if(timeout(2.0))
            state = "move up";
        else if(Player.active != player || !player.lookingUp)
            state = "main";
        else
            moveBackToZero();
    }

    state "wait down"
    {
        if(timeout(2.0))
            state = "move down";
        else if(Player.active != player || !player.crouchingDown)
            state = "main";
        else
            moveBackToZero();
    }

    state "move up"
    {
        offset = Math.max(-96, offset - speed * Time.delta);
        if(Player.active != player || !player.lookingUp)
            state = "main";
    }

    state "move down"
    {
        offset = Math.min(92, offset + speed * Time.delta);
        if(Player.active != player || !player.crouchingDown)
            state = "main";
    }

    fun moveBackToZero()
    {
        if(offset > 0)
            offset = Math.max(0, offset - speed * Time.delta);
        else
            offset = Math.min(0, offset + speed * Time.delta);
    }
}
