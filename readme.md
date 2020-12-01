Rogue Forest
============

This is an entry for Cemetech Contest #24. The theme for this is
"Collaborative Safari", which requires a team consisting of exactly
two people, and to create a game that is related to exploring a wilderness.

The Game
--------
Planned: A top-down roguelike game which takes place in a forest. You are a
cherry-flavoured dragon, who wants to explore ruins in a magical forest while
searching for the legendary Cherry of Yendor.

Installing and Running
----------------------
Send the following files to the calculator:
* `bin/ROFOREST.8xp`
* `src/gfx/output/RoFoDAT0.8xv`
* `src/gfx/output/RoFoDAT1.8xv`
* `src/gfx/output/RoFoDAT2.8xv`
* `src/gfx/output/RoFoDAT3.8xv`

Run `ROFOREST.8xp` on your calc as an assembly program, or use your favorite
shell (such as Cesium) to run it.

Building the Game
-----------------
Ooooooh boooooy...

You'll need to have installed, in this order:
* The latest [CE C toolchain build](https://github.com/CE-Programming/toolchain)
* The latest version of [convimg](https://github.com/mateoconlechuga/convimg)
* The patience of a Saint.

You'll need to verify that...
* ... the version of the CE C toolchain you have is the ZDS-based version,
  not the LLVM version. I haven't tested it with the LLVM version yet so there's
  no telling if it'll actually compile there.
* ... you have downloaded the new convimg program and overwritten the preexisting
  version in the CE C toolchain's `bin` directory.
* ... the first time you `make` the file, it fails because of problems with
  the icon.src file. This is normal since this version of convimg is meant to be
  used with the LLVM version of the toolchain. **fix this by copying `icon.src` in the root directory to the
  newly-created `obj` directory**.

To actually build the thing:
* Open a command line in the project's root diretory.
* Run `make gfx`. If problems happen here, your convimg program probably isn't the right version.
* Run `make`. If this is the first time running, it'll fail. See fix above, then try again.
* You're done. Go send the generated files to your calculator and have fun.

TODO
----
Making the enemies move and attack. Also make you attack as well.
Perhaps add in a tab for overworld map tiles seen so you don't get totally lost.
Oh. Also. Link in the title screen to the actual game. Probably a dummied out
class select?

Credits
-------
* Cemetech - This game wouldn't have been worked on if it weren't for the guys there.
  A great place with great people. 10/10 would visit again.
* "Geekboy1011" - The other guy working on this project. He provides cherries, sanity,
  project management, and sanity.
* MateoConLechuga - His work on the toolchain and specifically the convimg utility.
* Other entries will go here as I'm going to draw tilesets from various sources.
  Proper attribution will be made at least before something releasable occurs.
* "DragonDePlatino" - The author attributed for the creation of the
  [Dawnlike tileset](https://opengameart.org/content/dawnlike-16x16-universal-rogue-like-tileset-v181) ,
  from which parts of the tileset was sourced.
  
License
-------
* My program code is licensed under the MIT License. Hopefully it is compatible
  with whatever graphical assets is added. Idk if I can or not, but the graphics
  will have their own attribution.
* This section will grow depending on what I add.
* "Dawnlike" at the time of retrieval is licensed under the
  [CC-BY 4.0 License](https://creativecommons.org/licenses/by/4.0/) .
  The additional request might not be honored by the time the contest ends due to
  time limits, but intended to be included later on.




