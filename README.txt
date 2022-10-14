INSTRUCTIONS:

NOTE: CURRENTLY SET TO SCREENSPACE INTERPOLATION, CAN BE CHANGED TO REAL SPACE INTERPOLATION BY SETTING SCREENSPACE in framebuffer.cpp to false.

1. RENDERING IN FILLED MODE:
	1. Choose a piece of geometry to uncomment in "scene.cpp" (skip 1 and 2 for the teapot).
	2. make sure to add mesh.fill = true if not already present.
	3. Run program.
	4. Click "Load Teapot" Button, if you are loading the teapot.
	5. Click "Load Txt" Button.
	6. Move camera around until you can see the selected geometry, which will be filled in.

2. LIGHT:
	1. Run program.
	2. Click "Load Teapot" Button.
	3. Click "Load Txt" Button.
	4. Click the arrows in the top right of the GUI to move light in the +x, -x, +y, -y, and +/- for +z, -z
	Note: The white dot with the line segment represent the light source and direction. 
	Note: I have a very high tolerance set for the shadow buffer, this may create some oddities while fixing others.

3. LIGHTING:
	1. Run program.
	2. Click "Load Teapot" Button.
	3. Click "Load Txt" Button.
	4. Click "+PhongExp" and "-PhongExp" to adjust phong exponent, the actual exponent is printed to the cmd line.
	5. Click "+Ambient" and "-Ambient" to adjust k_ambient, the actual value is printed to the cmd line.
	Note: I have a very high tolerance set for the shadow buffer, this may create some oddities while fixing others.

4. THREE SHADING MODES:
	1. Run program.
	2. Click "Load Teapot" Button.
	3. Click "Load Txt" Button.
	4. Click "SM1", "SM2", "SM3" in the GUI to change lighting modes.
	NOTE: LIGHT SOURCES DO NOT AFFECT SM1. (if you dont believe me, try moving the light source in SM1).

5. EXAMPLE
	1. View MOVIE.mov in the folder.

EXTRA CREDIT - DIRECTIONAL LIGHT SOURCE:
	1. Click "Load Teapot" Button.
	2. Click "Load Txt" Button.
	3. Click "SM3" (or "SM2") Button.
	4. Run program. This is already baked into the light projection.

EXTRA CREDIT - MULTIPLE LIGHTS:
	1. Uncomment 2nd light section in "scene.cpp".
	2. Change the last parameter of the first LIGHT object in "scene.cpp" to "DEG_TO_RAD(40.0f)".
	3. Run program.
	4. Click "SM3" Button.
	5. Click "Load Teapot" Button.
	6. Click "Load Txt" Button.
	Note: There will be 2 lights shining on the teapot now, with distinct projections.

EXTRA CREDIT - CONIC LIGHT SPOT:
	1. Change the last parameter of the first LIGHT object in "scene.cpp" to "DEG_TO_RAD(40.0f)".
	2. Run program.
	3. Click "SM3" Button.
	4. Click "Load Teapot" Button.
	5. Click "Load Txt" Button.
	6. Click "Play" Button.
	Note: The beam will be restricted to the angle provided (which is 40 degrees in this case),
	thus creating a conic light spot on the teapot as the light moves.