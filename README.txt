INSTRUCTIONS:
	
1.	PPC:
		1. View "ppc.hpp" and "_ppc.hpp" to see PPC implementation.
		2. Test PPC while running using:
			- TRANSLATIONS: 'w', 'a', 's', 'd', 'c', 'v' to move forward/left/backward/right/up/down.
			- ZOOM: '-' and '=' to zoom out/in.
			- ROLL & PAN: Arrow keys to look up/down/left/right.
			- TILT: 'e' and 'r' to tilt clockwise/counter-clockwise
		2. Test INTERPOLATION by clicking "Play", the 3 key frames are clearly present.
		3. Test Txt IO:
			- Modify INPUT_TXT and OUTPUT_TXT in ppc.hpp to whatever you wish.
			- Save PPC to .txt by clicking "Load Txt" and "Save Txt" buttons in GUI.
		4. Visualization: Simply run program.

2.	TRIANGLE MESH

		1. View "Geometry.hpp" and "_Geometry.hpp" to see "MESH" class implementation.
			- NOTE: RenderAsWireFrame is found in "COMPUTED_GEOMETRY"'s "add_mesh" function in "_Geometry.hpp".
			- NOTE: Color interpolation is found in "Framebuffer"'s "applyGeometry".
		2. Test Bin IO:
			- Modify "INPUT_BIN" and "OUTPUT_BIN" in "Geometry.hpp" to control files written to/read from.
			- (WARNING) You can modify "SEL_MESH" to control mesh we save, write over, and rotate, but keep the index in out of bounds.
			- Save MESH to .bin by clicking "Load Bin" and "Save Bin" buttons in GUI.
		3. Run application to view meshes, change between 
		4. Click "Rotate Mesh" to see mesh rotate about axis defined by rotation_axis1 and rotation_axis2 (starting and end points of rotation line segment).

EXTRA CREDIT - SPHERE:
	1. Uncomment sphere wire frame section in "scene.cpp".
	2. Comment box wire frame.
	3. Run program.