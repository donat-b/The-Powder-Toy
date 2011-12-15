------------------------------------------------------------------------------
If you're missing any DLL's, download them here: http://min.us/lYzg3yW4VN2ce

This is a mod that basically does the same thing as the Lua script I made. 
It has a favorites menu under the Tools menu, that has all of the elements 
you selected recently, including walls and tools. Ctrl+Shift+left clicking 
an element in any menu will pin it permanantely to the menu, even after you 
exit the Powder Toy. Ctrl+Shift+ right clicking an element will unpin an 
element from the menu.

If you click the more button, a new menu will appear with 4 options. The HUD 
button activates a better HUD that fixes some problems, has less useless 
info, and has more decimal places in debug mode. The FIND button colors 
whatever element you have selected red, and dims everything else. The INFO 
button displays statistics about how long and how many times you have played 
powder toy, and your average and highest FPS.

I will take any suggestions for new elements or other features, no matter 
how hard they are to code, and if I like it, I will try to add it in.

Things in my mod suggested by other people:
Atrayin - Powered Portals, RAZR
OmegaSupreme - Animated LCRY
Videogamer555 - New heat displays
disturbed666 - A way to turn PHOT into ELEC (but I used PINV instead of TESC)




Explanation of other new features:

Moving Solids: No lag, and it actually works. They are found in the special
menu, and you can have a maximum of 256 at once. They are drawn the same 
shape as your brush, no matter what shape or size it is. They are very 
bouncy, and fall with gravity. Pressure over 4.0 will destroy them, and if 
the center particle is destroyed, it will fall apart, making really tiny 
bouncing pixels. Go into the fav2 menu and click SPIN to make them be able 
to rotate. Don't save moving solids if you allowed them to rotate, reloading 
them after that is glitchy and won't work.

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
minimum temp. Enter the temperatures in Celcius. It will change the display 
mode into manual automatically after this.

Virus: Found in liquids. Turns everything it touches into VIRS. It has 3
states: Solid, liquid, and gas; depending on the temperature. If you infect
something room temp or less, it will all be solid virus, and you will be able
to cure it with CURE (in liquids) without noticing any difference in the
save. If you don't cure it in time, it will start to die slowly and randomly.
It is also cured randomly, and not instantly. Note:VIRS can burn and be 
destroyed, even if it infected something like DMND.




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
the lowest, and everything inbetween will be based off that. 
I also added powdered portals and virus/cure, and updated to the latest 
source to get rid of the update box.

Version 5.1: http://min.us/lDCKmb9BdFazn
Updated to latest version, added activator, which can be used in creations 
to prevent people from sparking buttons until specific times. Also, re-added 
in buttons to switch to the different modes, except blob and heat gradient.

Version 6.0: http://min.us/lUgHNsGndzdbh
Added powered invisible, blob and heat gradient display, portal effects, 
and wifi/portal lines. Also added the old lua console commands: 
sound(must be enable with a shortcut), load, bubble, reset_temp, and 
reset_pressure(works with the same parameters reset_velocity does). 
There are three new lua commands: get_pressure, 
get_gravity(doesn't work for some reason), 
and maxframes(sets the number of frames ANIM can have when created, 1-256). 
Also, render_ui uses the old cmode pictures in color.
Note: the button keys to change display modes, blob & heat gradient 
display, portal effects (Simon forgot the wifi/portal lines), and new 
icons all got into the official version. Not much of this code was 
actually written by me, a lot of it was taken from github after Simon 
deleted it. He said he might add in more stuff that I wrote later.

Version 7.0 http://min.us/lK604TH6bFNV4
Added readme. Faster drawing and deleting of elements. Added RAZR, an element
heavier than all other elements. Added new button in FAV2 menu that can make
saves compatible with the latest beta or release version and able to be
published. It makes sure that you don't add in any new elements not in those
versions. PINV+PHOT = ELEC. SPNG drags along other particles with it, even 
solids that normally can't move.

Next Version
Wall names display in the hud. New lua functions: get_wall and create_wall.




Bugs:
Moving solids can't be drawn on an empty screen.
Moving solids destroyed by large pressure leave behind pieces inside solids
Moving solid rotation doesn't save
In virs, after a long infection/cure time, everything past a certain point 
    will be destroyed/cured at about the same time




Future Ideas/TODO: > means currently being worked on/added next
>Include lua code in save (like autorun.lua does)
buttons to decide what is in the HUD
brush rotation
more statistics - http://powdertoy.co.uk/Discussions/Thread/View.html?Thread=11998
ambient heat blocking wall
old console commands destroy/safe that I made a long time ago, + still more lua commands
rechargeable battery - http://powdertoy.co.uk/Discussions/Thread/View.html?Thread=11652
PRHC - powered heater/cooler - dark purple deactivated - insl when inactivated - Tetracon
More deco tools