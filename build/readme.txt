------------------------------------------------------------------------------
Use the legacy version of my mod if using an older computer that doesn't 
support sse2 (yours probably does), or if it just runs faster

Scroll to the bottom for recent changes and a todo list.
I am going to port this to c++, so it won't be updated except to add any 
elements made in tpt++.

This is a mod that basically does the same thing as the Lua script I made. 
It has a favorites menu under the Tools menu, that has all of the elements 
you selected recently, including walls and tools. Ctrl+Shift+left clicking 
an element in any menu will pin it permanently to the menu, even after you 
exit the Powder Toy. Ctrl+Shift+right clicking an element will unpin it from 
the menu.

If you click the more button, a new menu will appear with more options. The 
HUD button activates a better HUD that fixes some problems, has less useless 
info, and has more decimal places in debug mode. HUD's are customizable using 
the HUD2 button. The FIND button colors whatever element you have selected 
red, and dims everything else. Ctrl + F also activates it. The INFO button 
displays statistics about how long and how many times you have played powder 
toy, your average and highest FPS, and info about the current save.

I will take any suggestions for new elements or other features, no matter 
how hard they are to code, and if I like it, I will try to add it in.

Things in my mod suggested by other people:
Atrayin - Powered Portals, RAZR
OmegaSupreme - Animated LCRY
Videogamer555 - New heat displays
disturbed666 (aka grandmaster) - A way to turn PHOT into ELEC (but I used PINV instead of TESC)
Dynotec - new statistics
therocketeer - smudge tool
cip - INWR with tmp of 1 doesn't conduct to SWCH
BoredInSchool - fusion
tommig - VOID ctypes
Galacticruler - SING can emit gravity instead of pressure (set tmp2 to 1, or 2 for both)
Joeboy25 - AMTR ctypes
baizuo - Multiple element highlights
The-Fall - GEL and SPNG can absorb water from more elements (ex. PSTE + SPNG -> CLST)



Explanation of other new features that need a long explanation:

Moving Solids: No lag, and it actually works. They are found in the special
menu, and you can have a maximum of 256 at once. They are drawn the same 
shape as your brush, no matter what shape or size it is. They are very 
bouncy, and fall with gravity. Pressure over 10.0 will destroy them, and if 
the center particle is destroyed, it will fall apart, making really tiny 
bouncing pixels. Go into the fav2 menu and click SPIN to make them be able 
to rotate. NOTE: They don't bounce off walls. This is a glitch that I won't 
fix, because it would make them a little laggier and they would get eaten 
by the wall anyway.

Animated LCRY: Found in the powered elements menu. In the decoration editor, 
press left or right to change frames, and paint each frame normally. Press 
Del to delete a frame or press Ctrl+right when going to a new frame to copy 
the old one over to the new one. PSCN activates it, and the speed that it 
changes depends on its temperature. NSCN deactivates it. METL pauses/unpauses
it. If you set the tmp2 of the PSCN or NSCN that (de)activates it, you can 
set which frame ANIM starts or freezes at. Also, you can use the tmp of the 
PSCN to set the delay for the first frame. You can have a maximum of 25 
frames, unless you use the lua command tpt.maxframes(num), where num is the 
new maximum, between 1 and 256. Using that command will reset all existing 
ANIM. If you delete a lot of it at once, the game will freeze for a while, 
just wait until it keeps working.

New heat displays: Found in the Fav2 menu. Click the HEAT button once to go 
to automatic display mode, where the hottest temp is always pink, the lowest 
is dark blue, and everything else in between is based off of that. Click it 
again to go into manual heat display. The min and max temps will stop 
changing when you go into this, so it will help if automatic mode flickers 
because of something like fire clone constantly changing the max temp. If 
you right click the  button, it will ask you to enter the new maximum and 
minimum temp. Enter the temperatures in Celsius. It will change the display 
mode into manual automatically after this.

Virus: Found in liquids. Turns everything it touches into VIRS. It has 3
states: Solid, liquid, and gas; depending on the temperature. If you infect
something room temp or less, it will all be solid virus, and you will be able
to cure it with CURE (in liquids) without noticing any difference in the
save. If you don't cure it in time, it will start to die slowly and randomly.
It is also cured randomly, and not instantly. Note:VIRS can burn and be 
destroyed, even if it infected something like DMND.

Powered elements: All powered elements are now activated instantly. When 
you draw a powered element, a flag is set which causes it to be instantly 
activated. Saves loaded from old saves don't have this set, so won't act this 
way. Use the prop tool to set the flags to 0 to go back to the old way.

Lua code: Put a lua script in luacode.txt and click the LUA button in the 
FAV2 menu. It will change the script into INDI and put it in the top left 
corner. It will then run the code once, so if you want it to run every frame, 
use tpt.register_step. When you save it in a stamp, and will allow it to be 
run one more time when you open it. After you save a stamp/save or open one, 
you can delete the INDI. The code will already be in newluacode.txt, and you 
can look at it to see what it does. I also added in virus prevention. You can 
only use certain approved functions, and all of the tpt functions. You also 
can't do while 1 do end, because the loop will exit after 3 seconds.

Lua Graphics Functions: Allows you to change an element's graphics function. 
The values i, colr, colg, and colb are passed into the function. You must 
return cache, pixel_mode, cola, colr, colg, colb, firea, firer, fireg, and 
fireb in that order. If you leave any out or as nil, it will get it's default 
value. The cache value is a boolean that tells if it will add these values 
into the graphics cache, meaning it will remember the return values and not 
run your function again. This eliminates lag when a lot of particles are 
drawn. The pixel_mode properties can be found in powdergraphics.h.
Example: 
function graphics(i, colr, colg, colb)
   return 1,0x00FF0008,0,0,255,255,255,0,0,255; --cola is ignored with PMODE_FLAT, so it didn't matter that I accidentally made it 0
end -- blue green with a bluish glow
tpt.graphics_func(graphics,tpt.el.eqve.id)
--see the lua section on the wiki for a better explanation, with pictures and pixel_mode descriptions




Changes:

Version 1.0: http://min.us/lbuOtsPdEMibD2
Original release, with most of the stuff from my Lua Mod

Version 2.0: http://min.us/lzFWVBYhl5Xfi
Now with MOVING SOLIDS!

Version 2.1: http://min.us/lbvyL8rkNJlcni
Latest source + source now on github

Version 3.0: http://min.us/lbjHzEoxmXAFzP
Added Animated LCRY. Moving solids and ANIM save when you make a stamp or 
save.

Version 4.0: http://min.us/l2FzyIz4AYBS9
Moving solids can rotate now, disabled by default, 
go into the Fav2 menu and click SPIN to allow them to rotate. 
Also added Indestructible insulator and One time wire, 
and did some fixes to FIND and BALL.

Version 5.0: http://min.us/lbh6DDTJzdNFU3
Added automatic and manual heat displays. In automatic heat display, 
the highest temp is pink and the lowest is dark blue. In Manual, 
you can decide which temp is displayed as the hottest color and which is 
the lowest, and everything in between will be based off that. 
I also added powdered portals and virus/cure, and updated to the latest 
source to get rid of the update box.

Version 5.1: http://min.us/lDCKmb9BdFazn
Updated to latest version, added activator, which can be used in creations 
to prevent people from sparking buttons until specific times. Also, re-added 
in buttons to switch to the different modes, except blob and heat gradient.

Version 6.0: http://min.us/lUgHNsGndzdbh
Added powered invisible, blob and heat gradient display, portal effects, and 
wifi/portal lines. Also added the old lua console commands: sound(must be 
enable with a shortcut), load, bubble, reset_temp, and reset_pressure(works 
with the same parameters reset_velocity does). There are three new lua 
commands: get_pressure, get_gravity, and maxframes(sets the number of frames 
ANIM can have when created, 1-256). Also, render_ui uses the old cmode 
pictures in color.Note: the button keys to change display modes, blob & heat 
gradient display, portal effects (Simon forgot the wifi/portal lines), and 
new icons all got into the official version. Not much of this code was 
actually written by me, a lot of it was taken from github after Simon deleted 
it. He said he might add in more stuff that I wrote later.

Version 7.0: http://min.us/lK604TH6bFNV4
Added readme. Faster drawing and deleting of elements. Added RAZR, an element
heavier than all other elements. Added new button in FAV2 menu that can make
saves compatible with the latest beta or release version and able to be
published. It makes sure that you don't add in any new elements not in those
versions. PINV+PHOT = ELEC. SPNG drags along other particles with it, even 
solids that normally can't move.

Version 8.0: http://min.us/muN92vQxC
Wall names display in the hud. New lua functions: get_wall and create_wall. A 
New Wall: ERASEALL. It can erase walls, signs, and particles all at once. 
Most powered elements drawn in this version are now activated instantly. Set 
the tmp to zero to keep the old way of activating. New properties that can be 
set using the lua console: CLONE (0x40000), BREAKABLECLONE (0x80000), 
POWERED (0x100000), and INDESTRUCTIBLE (0x20000). Use the first 3 together to 
make different kinds of clone using elements. The last one makes things 
invincible to every element, and even prevents it from being destroyed in 
reactions. You can make things like indestructible WATR or METL. Lua code can 
now be saved. Put it in luacode.txt, and it will be saved and run in 
stamps/saves. Press up in the console and then enter to load an example.

Version 9: http://min.us/muN92vQxC
Fix loading some saves and fixed crash with HUD. Removed update button 
permanently and updated to newer source. Also, decoration color is now drawn 
on stamps in the stamp browser. 2 new quickoptions: Newtonian gravity and 
ambient heat. Custom HUDs. To change one, click the HUD2 button in the second 
favorites menu and different options will come up. You can change all 4 of 
the original HUD's and it will save and be loaded the next time you start. 
Also added a few more options not in any normal HUD.

Version 10
Added display mode toggles, press Ctrl+ a number key besides 4 and 8 to 
toggle the main feature of that display temporarily. Added more statistics. 
Fixed some moving solid bugs and made rotation save. Updated to newest 
source, which includes the new save format, but you can choose to save in the 
old format instead. You can delete corrupted stamps now. Fix three powered 
electronic related bugs, all electronics should work like they're supposed 
to now. Added a decoration smudge (blend) tool

Version 11 - 2/21/12
Added PROP2 tool, you can draw properties on with the normal brush now. 
Improved decoration drawing speed and added a hex value display and floodfill 
to it. Changed menus of some elements. Added a new test icon for the favorites 
menu. Added an update check, if there is a new version, you can use the link 
in the message of the day to download a zip file containing the next version 
of this mod. Added COND, only conducts to other COND with same tmp within a 
tmp2 radius. Added PWHT, flood fill heats the element above it to it's temp. 
Use PROP on it to make it create other properties. Added rechargeable battery, 
set the tmp of battery to the maximum charge it can hold, METL to charge, 
PSCN/NSCN to turn on/off. Some other stuff not listed here.

Version 12 - 3/2/12
Added tabs. Hit the tab button in the quickoptions menu or press Ctrl to show 
a menu of all the tabs you have open. You can switch between them, or open a 
new one (what you are doing is copied over). Tabs are saved and not simulated 
when you are not using them. Press shift to save pressure in stamps and saves, 
RCTRL to keep it when transforming stamps. Added fusion. see thread 12901 for 
more info (http://powdertoy.co.uk/Discussions/Thread/View.html?Thread=12901). 
Added realistic mode, not by me and not too noticeable. Autosave button, 
disabled by default. If Powder Toy unexpectedly closes, the next time you 
open it your work will appear. Fix decoration tools.

Version 12.1 - 3/3/12
Make text say version 12, not 10. Remove update check and replace it with 
another link. Fix crash when you click the LUA button and luacode.txt was 
missing

Version 13 - 3/18/12
Fix glitch when VOID-like elements eat particles. Changes and fixes to FIND 
tool, can also now find walls. Ctrl+F to quickly find and Ctrl+S to save the 
current tab/create a backup in case of a crash. Shift+R reloads the current 
tab without saving, Ctrl+Q then clicking yes completely exits from anywhere, 
and Ctrl+O opens the saves catalogue. You can now edit alpha in the deco 
editor. Alpha is displayed in (OPS) stamps, all wall types drawn in all 
stamps. save/load more things in powder.pref. Now uses static Libraries, no 
dll's are needed. Some fusion changes to make it more controllable.

Version 14 - 4/1/12
Lua code in saves run in a limited environment without access to dangerous 
functions that could cause a virus. Prevent infinite loops in Lua, they will 
exit after 3 seconds (you can click cancel to let it continue running though).
Oxygen fusion added. 4 New commands: tpt.clear_sim(), tpt.restore_defaults(),
tpt.reset_elements(). Moving solid ability added to all elements, use 
tpt.enable_moving_solids() to make all solids move like BALL does. That last 
one was an April fools joke, if you couldn't tell already.

Version 14.1 - 4/7/12
Really update to latest source. Fix infinite fusion and burning oxygen. CLNE 
and PCLN are indestructible, BCLN and PBCN break with DEST and SING again. 
BHOL, VENT, and VOID are not destroyed by DEST or SING now. New lua commands: 
tpt.draw(or fill)circle(x,y,rx,ry,r,g,b,a) and tpt.indestructible(string name 
or particle number). An optional argument to that is a 0, which makes it 
destructible again. Added Easter egg hunt, press 'e' to start. Removed in 
version 15.

Version 14.2 - 4/30/12
update to latest source again. reversible pipe - spark with PSCN to reverse, 
NSCN to unreverse. This mod is temporarily discontinued, but will be started 
again in May/June.

Version 15 - 5/22/12
Fix some deco editor bugs. Element descriptions fade in and out nicely.Fixes 
to the old menu, the favorites menu is always drawn on bottom when using that. 
Ability to make custom lua graphics functions. Fix glitch in HUD (life 
wouldn't show up). You can scroll through comments on saves, but only 10 at 
most are able to show up. User ID's are now drawn on comments. You can write 
in color now. Use Ctrl  + one of these letters: w (white), g (grey), o 
(orange), r (red), l (red-orange), b (blue), and t (light blue). Colors can 
go in comments, tags, and save names and be seen by all tpt users. They can't 
go in signs or local saves names (colors are removed). Update to 79.2 source.

Version 16 - 6/14/12
VOID and PVOD can be set to only eat their ctype (or only not eat it if their 
tmp is 1). Local save deletion inside tpt. Fix saving as release/beta version, 
fix many saving problems. Fix SING explosion. lua functions can be auto-
completed when writing commands in the console (ex. type "tpt.s" and press 
tab, it changes to "tpt.set_property("). Works with many common lua console 
commands/things. Lightning can be cloned and it's initial direction is 
affected by Newtonian gravity too. Right HUD always drawn on top. Moving 
solids are solid inside and much less glitchy. Also, use tpt.moving_solid() 
to allow any element to move like moving solid.

Version 16.1 - 6/18/12
Fix saving as release version, fix saves with LIGH in them. Fix PIPE exploding 
when energy particles went in it. Front page labeled as page 1, not page 0. 
Up to 100 comments on saves can be loaded now, they are loaded as you scroll 
down. Moving solids don't collide with particles they can go through. Fix 
moving solids causing stacking and BHOL being created. Update to latest source 
again (still 80.4) which fixes some other bugs.

Version 16.2 - 6/18/12
Fixed saving saves publicly, remove ability to create colored text.

Version 16.3 - 6/25/12
Make Some lua console shortcuts less annoying. Fix compression during fusion 
and with moving sponge. Fix moderator name colors, also, your name is yellow 
and save creators is red. Update to latest source.

Version 17.0 - 7/5/12
Many minor graphical fixes to the HUD. FIND can find 3 things at once. Left 
selected element is red, right selected is blue, and alternate selected is 
green. LCtrl/RCtrl/Shift + F to toggle these. Also, a new FIND2 button which 
toggles an alternate find mode more similar to my lua script, where first 
everything is dimmed, and then a dot is put over what you're finding. Click 
a user ID to switch to showing comment post times and dates instead. Use the 
DATE button to change the format. Date format also applies to the HUD now, if 
you chose to display it. The areas for stamp saving, copying, and cutting can 
be drawn backwards now. Fix first comment on saves not showing. Fix crash bug 
with powered electronics and moving solids, ANIM activated instantly now too. 
New flood-fill brush shape. Many speed improvements by skipping unneeded 
things. WOOD and PLNT look burnt when hot and frozen when cold. VIRS has a 
small chance of destroying CURE. saving stamps looks better & you can delete 
multiple stamps at once with Ctrl+red delete button. Many other minor fixes 
and changes.

Version 18 - 8/2/12
Fix bug where all solids could move like SPNG, fix low pressure transition not 
working. Fix lava's ctype getting changed. You can now highlight text in 
textboxes and copy/delete just that. SING with a tmp2 of 1 emits gravity 
instead of pressure (still creates tons of pressure anyway), and with a tmp2 
of 2 does both. Antimatter's ctype and tmp act tike VOID's (eats it's ctype). 
Right click the reload button to go back to open_ui. Fix moving solids not 
showing in HUD. Add scrollbar to scrolling menus. Fix VACU/VENT. Limit the 
FPS everywhere, so tpt doesn't use 100% CPU when you're not in the main game. 
Tons of new lua commands, you can pass nothing into functions that set a 
variable to return that variable instead. When changing the screen size, you 
can change it even if your screen if too small if you want. Improvements to 
open_ui, you can click a username to search for their saves, and shift click 
to see their profile.

Version 18.1 8/7/12
Fix crash when saving invalid elements (that you shouldn't have anyway). Add 
an icon to the deco menu and element search ui. Disable reversible pipe on non 
mod saves. Improvements to the deco menu to make it feel more like the editor, 
fix middle click to get decocolor inside of it. Fix graphics function crash. 
Fixes to PWHT so that it works and saves better. Fix instantly powered 
electronics not saving. Combine PROP & PROP2. To use old prop, flood fill with 
Ctrl + Shift. Fix TRON. GEL and SPNG can absorb water from elements and leaves 
behind what the element was mixed with. (ex. PSTE + SPNG -> CLST). Ask to run 
lua code in saves, instead of just running automatically.

Version 19 8/22/12
Update to version 82, with all the things I added to that including powered 
pipe. Fix comments with '/n' in them not displaying correctly. Fix BRAY/EMBR 
graphics (PMODE_GLOW), make it easier to find stickmen with FIND tool. Add 
tpt.outside_airtemp, sets the outside ambient heat temperature. More custom 
HUD options, plus fix a few bugs with it. Fix H2 and O2 not burning at high 
temperatures (from Triclops200 in tpt++). Edge loop option.

Version 19.1 9/2/12
Fix some stickmen spawning glitches (still in tpt though). Fast quit option 
(found in the simulation options). Compatibility with saves made in tpt++ 
(fixes walls). Fix deco not showing when drawing on ANIM. The extra 1st 
page saves option stays on like the other search options do. Add things from 
tpt version 83.0 (DTEC, things from my pull request).

Version 19.2 10/5/12
Allow longer comments on saves, and the comment box resizes to fit. Allow 
longer signs to be loaded in saves and make the sign creation ui larger to 
allow you to make signs the size they are in tpt++. Fix clicking usernames on 
the bottom row of saves not working. Add some minor simulation changes made 
in tpt and tpt++.

Version 19.3 10/19/12
Fix minor problem where PBCN and PLCN wouldn't clone until a frame after they 
were activated. Fix really long comments not loading, the comments not being 
drawn to the very bottom, bugs with the resizing comment box, jump when 
scrolling past certain comments, and scrolling when typing certain letters in 
the comment box. Use {t:id|text} to link to forum threads and {s:query|text} 
to start a save search (added by me in tpt++ first). Added tpt++ elements: 
DMG and TSNS. Fix being unable to save PIPE sometimes (error when saving, 
invalid element). Fix some moving solid crashes. Fix HUD showing incorrect 
values with double scale enabled. Don't load selected elements or render mode 
in saves anymore. Fix tpt bug that will cause you to get logged out sometimes 
when you shouldn't. Fix another tpt bug where Newtonian gravity wouldn't get 
reloaded after reloading a save - thanks to jacksonmj for the fix. Make VIRS 
last longer. Enable the # of votes icon on all saves for everyone.

Version 19.4 11/3/12
Remove DISPLAY_EFFE from render options. Fix clicking the copy id button 
exiting the save preview. Fix being able to click usernames and id's from 
under the comment box (again). Shift-clicking a username from the my own 
section to search for their saves will work correctly now. Fix shift/ctrl 
clicking moderators' not searching correctly. Increase number of comments 
that can be loaded to 200. Add in VIBR and BVBR from tpt++ (still not the 
final version, 19.4.1 will be soon). Fix PROP_NEUTPENETRATE. Fix bug causing 
switch to act differently in some rare cases. Fix tools menusection having 
an extra empty space in old menu ('o') mode. Add 'Empty' edge mode option.

Version 19.4.1 11/5/12
Fix save as current name and improve how saving works (will allow public / 
mod saves easily now). Put in the final version of VIBR. Make the thumbnails 
look as good as I can get them, tpt++'s thumbnail renderer isn't as good, 
so new thumbnails won't look as good. Add sensors menu icon.




Bugs:
None (at least that I know of)




Future Ideas/TODO: '>' means currently being worked on/added next
>Add things from tpt++ that look cool, and any new simulation changes it gets
fix comment error deleting comment
Add color presets to deco editor
improve comment scrolling
---
C++ version todo:
done - [compile tpt++ with visual studio]
>>fix tpt++ bugs before I start my mod
Start with version 1 changes like Fav elements menu, Find tool, better HUD; but add in the complete versions of those how they act in version 19
Add back everything in the Fav2 menu, including custom HUD
Make improvements to tpt++ to fix, add, or change some things I don't like
Add in all elements except INDI, OTWR, and powered portals (use powered pipe now)
Start adding in the other features, like the search ui improvements (may start earlier, like some save preview ui improvements)
Finish adding in other minor things (will look at github logs and changelogs in order probably
Done! continue work on my mod like normal

0% complete, haven't started yet. With school, it will be slow, but since it's mostly copying it might be easier

maybe in c++ version:
Don't save lua code inside of INDI, since this isn't a lua script anymore
RAND - deco tool - Uberness - 10/1 #powder at end
FRME
PROT - Protons
CMND - command element to run old commands
ADGS - Adhesive gas - http://powdertoy.co.uk/Discussions/Thread/View.html?Thread=15188
http://powdertoy.co.uk/Discussions/Thread/View.html?Post=220547