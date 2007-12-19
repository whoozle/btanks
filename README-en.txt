Battle Tanks demo version
(Beta testers may use this file as their manual.)

Installation:
Unpack the files into any folder (for example, C:\Games\Btanks\).

The package does not include an installer, so you'll have to use command line for switching modes.  See the list below.

The game has two modes:
1. Using of hardware OpenGL acceleration.  Original drivers for your video card will probably be needed.  If the drivers were not installed, the performance may be extremely low.
2. Using of software rendering.  This will work on any video card.

Command line parameters:
--no-gl     	turns off using of the hardware OpenGL renderer
--force-gl		forces using of the GL renderer, even if no acceleration was detected.
--fs        	turns on the full screen mode (will be the default setting in future)
--lang=XX		override language setting (XX - 2 letter ISO code: en, ru, de, fr)
--vsync     	turns on vertical synchronization--may help if tearing effect is visible
--connect=host  connects to the given host
--no-sound		turns off sound completely.
--sound			turns on sound, even if it was turned off in bt.xml

The screen resolution can also be set:
-1 800x600 (this is the default value)
-2 1024x768
-3 1152x864
-4 1280x1024

--connect=hostname/IP  joins the game on the specified hostname
--no-sound             turns off ALL sound

Errors are logged in stderr.txt. 

