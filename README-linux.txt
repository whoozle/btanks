random notes about linux compilation: 

0) If you're gentoo user, try ebuild located at 
	http://bugs.gentoo.org/show_bug.cgi?id=172772

1) you have to use cmake with either Ninja or make

2) build requirements (you need development packages for this libraries too, 
consult your distro's documentation for details): 

	`) g++ (at least 3.x, 4.x recommended)
	a) SDL >= 1.2.10 (you can use any version lower down to 1.2.5. 
                                         dont report bugs if you do so)
	b) SDL_image 	 (PNG support required)
	c) libvorbisfile, libvorbis and family
	d) expat - XML parsing library
	e) zlib
	f) lua 5.1 (maybe 5.0, not tested)
	g) smpeg - SDL mpeg1 library (for campaign preview movies)
	h) OpenGL headers file (usually from the Mesa project)

//for debian-based systems(i.e. ubuntu) you could use the following command 
to install all dependencies: 
sudo apt-get install gcc g++ cmake libsdl-image1.2-dev libsdl1.2-dev libvorbis-dev libexpat1-dev zlib1g-dev liblua5.1-0-dev libsmpeg-dev mesa-common-dev 

3) Run
  mkdir build
  cd build
  cmake ..
  make
