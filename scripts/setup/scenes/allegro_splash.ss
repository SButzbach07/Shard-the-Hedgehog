// -----------------------------------------------------------------------------
// File: allegro_splash.ss
// Description: script for the Allegro Splash Screen
// Author: Alexandre Martins <http://opensurge2d.org>
// License: MIT
// -----------------------------------------------------------------------------

//
// The art is from an Allegro 4 example game ("shooter"). Allegro 4 is licensed
// as Giftware. The animation was extracted using Allegro's grabber utility and
// later converted to a spritesheet.
//
// ps: this animation was probably created in the early 2000s ;)
//

using SurgeEngine.Audio.Music;
using SurgeEngine.Input;
using SurgeEngine.Level;

object "Allegro Splash Screen" is "setup"
{
    jingle = Music("musics/allegro.ogg");
    fader = spawn("Fader").setFadeTime(0.5);
    input = Input("default");
    secondsToWait = 5.0;

    state "main"
    {
        jingle.play();
        state = "waiting";
    }

    state "waiting"
    {
        if(timeout(secondsToWait) || shouldSkip()) {
            fader.fadeOut();
            state = "fading out";
        }
    }

    state "fading out"
    {
        jingle.volume -= Time.delta / fader.fadeTime;
        if(timeout(fader.fadeTime)) {
            jingle.stop();
            state = "finished";
        }
    }

    state "finished"
    {
        Level.loadNext();
    }

    fun shouldSkip()
    {
        return (
            input.buttonPressed("fire1") ||
            input.buttonPressed("fire3") ||
            input.buttonPressed("fire4")
        );
    }
}