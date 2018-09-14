Minetest
========
Piper's adaptation of the open source voxel engine, Minetest.

Piper modifications
===================

### Nicknames
To support multiplayer, aka PiperNet, we put in 1,000 inventor names in a random generator.
Caveat #1: they don't have passwords. The nicknames serve as the account.
This means users can go on other people's accounts if they just select the
same name as someone else used. A proper account system was in development
but never finished.

### Plugins
Before Minetest added mod channels, we added our own mod messaging system. This system is used
to allow client mods custom communication with server mods (e.g., in the mpv mod, server
tells client to play a video). Either our system or the official one can be used moving forward.
Theirs is more bulletproof probably but ours is perhaps easier and more intuitive.

### Main Menu

#### Updating
Our main menu is custom and automatically updates via Rsync. Note Rsync ports (22 if ssh, 873
if Rsync protocol) are blocked by many schools' firewalls. In the future http on port 80 or 443
is a more reliable choice (like in the debian updater). Despite being inferior in terms of school
compatibility, Rsync has the benefit of doing incremental updates via timestamps or file checksums.
Alternatives aside from .deb (which is not incremental) are wget, which can compare timestamps:
see debian/postinst in story, as well as [zsync](http://zsync.moria.org.uk/).

#### Give me back the old menu!
The default, full-featured minetest menu can be re-enabled by clicking on the rainbow flag multiple times.
You can also set dev\_mode = true in minetest.conf to bypass the Piper menu.

### Perf tips
* Turn down ABMs
    * Weather
    * Animals
* Turn down entities
* Don't run timers when no users are near
* Decrease view distance (ctrl - / ctrl + are useful hotkeys)
    * TODO: implement automatic view distance scaling if the FPS drops below a threshold.
    * Custom client functions for setting view distance have already been added (see eye of sauron triggers)!
    * The view distance in MCPI edition was only ~40 blocks radius. So the bar isn't
      super high
* Less geometry - make flatter levels.

### Ghost blocks
We've added the ability to show blocks to only certain players.
This might be useful in a multiplayer quest/tutorial environment.


### Programmatic key sending
This is for GPIO-based buttons.

### Programmatic map switching
This is useful for if we go away from having all our levels in one map. Can step into portal
and open a new world (might want to synchronize items).

### Ladder climbing
Climbing ladders in Minetest differs from MCPI. By default you can only climb while holding the jump button.
But in Mars, you have a lot of ladders and no jump button. So we hacked together a solution
to be able to just run into a ladder to go up like in MCPI. Right now the only caveat is
you have to be looking at the ladder block or else you fall. Sorry about that.

Upstream
========
Keep an eye on the upstream Minetest repo for useful things to merge. Also, they've taken
our PR's in the past.

Their IRC channel on freenode is very active and a useful resource for tough engine questions.
