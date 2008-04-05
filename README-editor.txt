!!!!BIG FAT WARNING!!!!

This editor is buggy and unstable. It contains the absolute minimum of 
the usability features. If you want something with windows interface - 
download hacked tiled(mapeditor.org) from our download page. 

This editor gone public only because of the multiply requests and was not 
designed for the unprepared users. 

So, use at your own risk. 

!!!!end of the BIG FAT WARNING!!!! :^)

First dialog : open/create map. 

Upper chooser: base directory for the map file (you don't need this)
Lower chooser: existing maps 
Numbers: width and height of the new map

try opening existing map for learning basic techniques. 


keyboard controls: 

alt-q                exit (DOES NOT SAVE MAP!)
ctrl-c/ctrl-x        copy/cut
escape               close dialog/remove brush
insert,i             add object from the list. 
ctrl-z               undo
ctrl-shift-z,ctrl-r  redo
b                    get the last used brush
g                    show grid position
f                    fill
e                    get special 'eraser' brush
m                    choose special 'morph' brush from the dialog.
o                    display objects on/off
alt-o                open map dialog
alt-r                resize map dialog
alt-s                save map
tab                  show tileset dialog (brush chooser)
tab + n in dialog    add new tileset from the list
shift                show layers menu/blink active layer
shift-n              add new layer
shift-up/shift-down  move layer up/down
delete               delete selected object
shift + delete       delete layer (no undo for layer operations yet!)
[ ]                  rotate selected object left and right. (if your object 
                            suddenly dissapeared, try undo or rotating back)
num+ num-            +10/-10 to the object's z value.


mouse controls: 

left button          paint with the brush if the brush is active. 
space + left button  move map 
right button         select tiles for the copy/cut, show object properties dialog
wheel                scroll map up/down



TMX map extensions overview: 
Battle Tanks uses some magic property names:  

"ambient-sound"
ambient sound looped and played with the music

"config:XXX"="TYPE:YYY"
config file override. There's useful config overrides, discussed later. 

"object:OBJECT:ANIMATION:ID"="position"
Battle Tanks object on the map. id - any string makes property name unique. 
position - x,y or x,y,z position of the object. if position starts with @, 
then position calculated as x * tile_width, y * tile_height. 
Usually you do not need to override z value. Each object have nice default value.

"spawn:ID" = "position"
Respawn point. x,y - usual position, z from here means 'z-box'[2] value. 
See Z-values section later. 

"zone:TYPE:NAME:SUBNAME:ID" = "position:size"
special zone: TYPE could be one of the following types: 
	"checkpoint"  checkpoint (save points in cooperative games)
	"hint"        shows hint "NAME" from the "hints" area of the localized strings.
	"message"     shows message "NAME" from the "message" area of the localized strings.
	"timer-lose", "timer-win" starts timer (you lose or win at the end)
    "reset-timer" switch timer off
	"disable-ai", "enable-ai" disables/enables given OBJECT's ai or CLASS' ai. [1]
	"play-tune"   plays tune
	"reset-tune"  stops tune started by "play-tune"
	"script"      calls NAME as lua function. (works once for any player)
	"local-script" calls NAME as lua function. (works for every player)
	
"waypoint:OBJECT:ID"              waypoints for the vehicles such cars. 
"edge:WAYPOINT_ID1:WAYPOINT_ID2"  edge for the waypoint graphs id1->id2

Useful config overrides (look bt.xml for examples) : 

<property name="config:multiplayer.game-type" value="string:cooperative"/>
"multiplayer.game-type" is the name, "string:" - type and "cooperative" - value.
There are 4 types available: bool, int, float and string. 
This override changes game type to cooperative one. 

"map.spawn-limit" "int:LIVES" 
Limits life counter to the LIVES. Useful only in cooperative missions.

"map.OBJECT.respawn-interval" "int:SECONDS"
This override changes respawn interval of the OBJECT to the given interval.

"timer.stay-alive.spawn-limit" "int:SECONDS"
life limits for the timers. Useful for some survival missions. 

"engine.global-targeting-multiplier" "float:VALUE" 
multiply targeting range for all objects.

[1] some objects share the same class: 
"zombie", "acid-slime", "sandworm" are "monster"s, 
"machinegunner"s, "thrower"s and other "troopers" are "troopers" :))

"disable-ai" zone on "monster" disables all monsters. this api is obsoleted. 
Use scripting for show/hiding items and enemies. 

[2] Battle Tanks map could contain separate 'worlds' or 'z-boxes'. 
layers/objects with z from the -1000(exclusive) to 1000(inclusive) are in the z-box (0). default.
-3000 to -1000 = z-box -1
-5000 to -3000 = z-box -2
 1000 to  3000 = z-box 1 (for helicopters)
etc...
