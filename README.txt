POINT ROTATION TESTING:

VARIABLES (already set in code): 

	Rotating Point:
		POINT3(
			V3(20.0f, 50.0f, 50.0f), // point
			V3(255, 0, 0) // color
		)
	Axis:
		LINE3(
			V3(-100.0f, 0.0f, 200.0f), // starting/origin point
			V3(100.0f, 100.0f, -100.0f), // end point, makes the direction: <200, 100, -300>
			V3(0, 0, 255) // color
		)

BUTTONS:

	Click "Rotate Point" to see point rotate by 2 degrees about axis.
	Click "Rotate Perspective" to rotate 3D coords (you can also adjust the starting perspective in ).

INSTRUCTIONS:
	
	1. Run. Point and axis should appear.
	2. Click "Rotate Point" until 360 degrees are completed.
	   X (red), Y (green), and Z (blue) graphs are automatically graphed.
	   The graph ("GRAPH.png") should match that provided by Google Spreadsheets ("GRAPH_SPREADSHEETS.png").
	3. Have fun! Play around with the perspective, add polygons, try breaking it, etc.