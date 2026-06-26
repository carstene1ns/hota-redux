# Heart of The Alien Redux

2004-2005 (c) Gil Megidish (gil at megidish dot net)

Thank you for downloading Heart of The Alien Redux! 
This rewrite brings the wonderful game to modern computers and consoles. 

For more information, please visit the original project homepage at
https://hota.sf.net/

## NOTE

You are required to obtain a copy of Heart of The Alien. In order to 
play the game, you may use ISO and MP3s prefixed "Heart Of The Alien (U)"
and put them in the same directory where you unpacked this distribution.

## USAGE

Running HOTA with no custom settings will follow the game flow as
it plays the introduction, and then pops up the code entry screen.

Running `alien -h` will display a list of acceptable parameters. 
Here are some interesting settings:

* `--scale` `x`      scale window by x
* `--filter`         use bilinear filter
* `--fullscreen`     start in fullscreen
* `--iso` [`prefix`] use ISO file for game data
                     (default: "Heart Of The Alien (U)")
* `--music` `prefix` use this prefix for wav/mp3/ogg/opus/flac music
                     (default: iso prefix)
* `--room` `n`       start from script n
* `--record`         record your key pressed
* `--replay`         replay recorded sequence
* `--fastest`        play game as fast as possible

These are developer only:
* `--debug`          write a lot of debug messages
* `--intro-test`     play all animations
* `--sprite-test`    show sprites in the current script

## KEYS

To control your character, you may use either set of cursor keys.
Moreover, here are extra keys available:

- <kbd>Z</kbd>/<kbd>A</kbd>  shoot, keep down in order to run
- <kbd>X</kbd>/<kbd>S</kbd>  use your whip
- <kbd>C</kbd>/<kbd>D</kbd>  jump
- <kbd>F5</kbd>              quicksave (1 slot)
- <kbd>F7</kbd>              quickload
- <kbd>ESC</kbd>             leave game, or skip animation

## BUILDING

CMake is required: `cmake -B hota`, `cmake --build hota`

## BUGS

The final version will be announced when no more bug reports have
been received, and no request for features posted. You are more 
than welcome to report a bug on the SourceForge project 
page (see links below.) When submitting a bug, if possible, please
record your keys and attach ``recorded-keys'', or attach a
``quicksave'' file to help reproduce the problem.

## FAQ

- Q: Why can't I play Another World (or Out of This World.)
  A: This project is for Heart of The Alien, Another World was
     attached to HOTA as bonus (or to increase sales.) You may 
     still play it on modern computers using 
     dosbox (http://dosbox.sourceforge.net) or 
     bochs (http://bochs.sourceforge.net)

- Q: I think I found a bug!
  A: Some of the bugs you may find also appear in the original game.
     There are plenty suspected. Only one has been checked and found
     faulty in the Sega CD version as well:
     * First screen, run left, quickly run right (away from the beast,)
     and run left again. Beast is gone.

- Q: I want to request a feature.
  A: Sure! Don't hesitate, see Contacts below.

## LINKS

* Homepage: https://hota.sourceforge.net
* Bugs and Requests: http://www.sourceforge.net/projects/hota/
* My wiki entry: http://www.megidish.net/wiki/index.php?title=Heart_of_The_Alien
* MobyGames entry: http://segacd.mobygames.com/game/sheet/gameId,8140/
* Underdogs entry: http://www.the-underdogs.org/game.php?id=3180
* Walkthrough: http://www.gamefaqs.com/console/segacd/game/8569.html

* CMake: https://cmake.org/
* SDL3: https://libsdl.org/
* SDL3_Mixer: https://libsdl.org/projects/SDL_mixer/

## CONTACT

The entire work was mostly done by a single person. Opinions, praises
and flames are mostly welcome.

I can be contacted at gil at megidish net

## CHANGELOG

### 2.0.0
  - Ported to SDL3
  - Fixed: no more intro animation bugs

### 1.2.4
  - Added: joystick support
  - Fixed: was polling one event at a time
  - Fixed: action keys break animation (instead of esc)
  - Fixed: better timing
  - Fixed: crash at getopt when parsing invalid command line (getopt)
  - Added: support for A,S,D keys (German keyboards :)
  - Lots of code cleanups

### 1.2.2
  - Added: scaler2x effect
  - Added: scaler3x effect
  - Added: simple x3 scaler
  - Fixed: sound effects
  - Major code cleanups

### 1.2.0
  - First public release!
