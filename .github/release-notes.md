online co op for twilight princess. 2 to 16 players in one world.

put crests_of_courage.dusk in dusklight's mods folder. a co op tab shows up in the menu bar.
one player presses host and reads out the room, everyone else types it and presses join.

stability build on top of 1.10.0, for dusklight 2.0.3. everyone has to have this same file: it only connects to 1.10.50.

this build:
- model changes wait until you are not transforming, warping, in a cutscene or loading
- fixed the warp effect crashing on custom models (and it stays off for them for now)
- no more memory leaks from players coming and going, freed during loading screens too
- players are shown from the game's own archive instead of going invisible when memory is tight
- stuck bombs and other thrown things get cleaned up
- old debug settings are turned off in this build, and a problems box says what was turned off
- a slow connection skips position updates instead of dropping the player
- a full session tells you it is full
- every model file is checked at startup; the models tab says which parts are link's
- story progress is shared by default
- small keys add up the same for everyone when two of you use and pick them up at once
- no-save chests stay open for whoever comes later
- dead enemies are always handed over when you walk in, and no hit is dropped
- dropped rupees and hearts can only be picked up once
- the host's clock is the one used
- the boss door waits 20 seconds with a countdown, anyone can let it start
- joining waits and asks if your save could not be backed up
- items and checks are not shared on a different randomizer seed
- new: sync status on the network tab, and "resync the world now"

1.10.0 had:
- added voice chat
- voice is directional and gets quieter with distance
- added a mute key and muting players
- mac and phones can only listen for now
- talking and muted icons over names
- shows mic on or off for a moment when you change it
- pick your own name tag colour
- new hyrule online page
- added an optional hyrule online box on screen
- players show on the minimap
- chat can scroll up
- the game keeps going if the host leaves or crashes
- asks to send a crash report after a crash, or sends it by itself if you turn that on
- better connections on bad networks
- fixed the customisation menu with a controller
- fixed the hyrule online player list being invisible
- fixed blocking sometimes hitting the wrong player
- fixed / and ? opening the debug console in chat
- fixed ordon being on the wrong day
- fixed the sky book not finishing
- fixed epona being invisible
- fixed wolf senses not unlocking
- fixed bo's hallway in rando
- fixed bo's talk looping
- fixed the kakariko bomb house cutscene softlock
- fixed staying a wolf after faron
- fixed rando swords skipping a level
- fixed double milk and oil bottles
- fixed light sword not syncing
- fixed the light sword glow being in the floor
- fixed double poe souls
- fixed other players' spike traps hurting you in the temple of time
- fixed a stuck helmasaur
- fixed the forest temple plant freezing you
- fixed a crash putting on the magic armor
- fixed a boomerang crash
- fixed a crash when someone gets an item
- fixed a crash hitting rocks
- fixed running out of memory with other players
