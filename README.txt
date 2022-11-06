Movie name: MOVIE.mov (uploaded separately to this zip)

NOTE: I modified the color of the teapot to make it easier to see reflections.

Steps:
	- Run .exe.
	- Click 'Load Teapot' Button in GUI.
	- User Arrow Keys and E/R to pan/roll/tilt about the object.
	- WASD to move (if you like).
	- Adjust specular via '+PhongExp' and '-PhongExp' in GUI.
	- Click 't' to toggle between the teapot being a mirror vs. regular object.
	- Click 'y' to toggle between bilinear interpolation vs. mip-mapping.

Extra Credit: Viewable in EXTRA.mov.
	- Refraction: Square piece of glass in the middle of the screen. Math lies in the if-statement at line 99 of frambuffer.cpp.
	- Panarama: Called "panarama.tiff" in folder. Format-specific read starts at line 74 of frambuffer.hpp.
	- Mipmapping: Functions start at line 61 and 93 of Geometry.hpp. 