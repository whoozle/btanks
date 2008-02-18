random notes about linux compilation: 

0) If you're gentoo user, try ebuild located at 
	http://bugs.gentoo.org/show_bug.cgi?id=172772

1) This game must be built with scons build system, no matter you like it or not. You 
have no choice. Autotools & family is stupid crap and I hope it'll die soon. 

2) if you have some cheap audio hardware without hardware mixer, ALSA project 
might help you. Just put this lines into your ~/.openalrc or /etc/openalrc : 

(define devices '(alsa))
(define speaker-num 2)
(define alsa-out-device 'plughw')
(define sampling-rate 44100)

usually "alutInit: There was an error opening the ALC device" error message 
indicates such problems.

3) build requirements (you need development packages for this libraries too, 
consult your distro's documentation for details): 

	`) g++ (at least 3.x, 4.x recommended)
	a) SDL >= 1.2.10 (you can use any version lower down to 1.2.5. 
                                         dont report bugs if you do so)
	b) SDL_image 	 (PNG support required)
	c) openal
	d) libvorbisfile, libvorbis and family
	e) expat - XML parsing library
	f) zlib
	g) lua 5.1 (maybe 5.0, not tested)
	h) smpeg - SDL mpeg1 library (for campaign preview movies)

4) you could add something like this in 'options.cache' file 
(located in trunk/ directory): 

CCFLAGS = ' -O -march=YOUR_CPU -mtune=YOUR_CPU  '
CXXFLAGS = ' -O -march=YOUR_CPU -mtune=YOUR_CPU '

KNOWN BUGS: 
*) Frequent crashes on amd64 platform: you have openal built with mmx support.
Disable mmx and rebuild openal. Patched package for debian available here: 
http://dump.iof.ru/people/megath/openal
