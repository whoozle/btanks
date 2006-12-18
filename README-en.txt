Battle Tanks demo version
(Beta testers may use this file as their manual.)

Installation:
Unpack the files into any folder (for example, C:\Games\Btanks\).

The game requires OpenAL libraries.  The package includes the openalweax.exe that will install them.

The package does not include an installer, so you'll have to use command line for switching modes.  See the list below.

The game has two modes:
1. Using of hardware OpenGL acceleration.  Original drivers for your video card will probably be needed.  If the drivers were not installed, the performance may be extremely low.
2. Using of software rendering.  This will work on any video card.

Command line parameters:
--no-gl     turns off using of the hardware OpenGL renderer
--vsync     turns on vertical synchronization--may help if tearing effect is visible
--fs        turns on the full screen mode (will be the default setting in future)

The screen resolution can also be set:
-0 640x480
-1 800x600 (this is the default value)
-2 1024x768
-3 1152x864
-4 1280x1024

--connect=hostname/IP  joins the game on the specified hostname
--no-sound             turns off ALL sound

Game settings:

The demo has a configuration file with lots of various parameters.  Most of parameters have names those explain what is what; however the following ones are the most noticeable:
"stubs.default-mp-map": sets a map for a multiplayer mode. Three maps are available in the demo: "survival", "dm2", and "country2". 
"stubs.default-mp-vehicle": sets a default vehicle. Three vehicles are available: "tank", "launcher", and "shilka". 
"player.control-method": sets the configuration of control keys. The following values are supported:
  "keys":    cursor keys move the vehicle, left Alt and left Ctrl fire primary and secondary weapons
  "keys-1":  rdfg + q + a.
  "keys-2":  cursor keys, right Shift, right Ctrl. 
  (keys-1/keys-2 are used in split screen mode)

Gamepad or joystick can be chosen by setting "joy-1" and "joy-2".

Errors are logged in stderr.txt. 

