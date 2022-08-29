
#include "framebuffer.h"
#include "math.h"
#include "scene.h"
#include "Dimension.hpp"
#include "V3.hpp"
#include "M33.hpp"
#include "Geometry.hpp"
#include <vector>
#include <iostream>
#include <tiffio.h>
#include <algorithm>

#define COLOR(r,g,b) ((b << 16) | (g << 8) | r)
#define STROKE_WIDTH 4

using namespace std;


FrameBuffer::FrameBuffer(int u0, int v0, int _w, int _h) : Fl_Gl_Window(u0, v0, _w, _h, 0) {
	w = _w;
	h = _h;
	origin = new V3(_w / 2, _h / 2, 0.0f);
	pix = new unsigned int[w * h];

	// add lines!
	lines.push_back(LINE(V3(-200, 0, 0), V3(200, 0, 0), V3(255, 0, 0)));
	lines.push_back(LINE(V3(0, -200, 0), V3(0, 200, 0), V3(255, 0, 0)));
	lines.push_back(LINE(V3(0, 0, -200), V3(0, 0, 200), V3(255, 0, 0)));
	lines.push_back(LINE(V3(69, 120, -2), V3(32, 49, 81), V3(0, 0, 255)));

	// rotate lines to showcase 3D.
	M33 rotation_matrix = M33(Dim::X, 0.2) * M33(Dim::Y, 0.2);
	for (LINE& line : lines) {
		line.start = rotation_matrix * line.start;
		line.start += *origin;
		line.start[2] = 0;
		line.end = rotation_matrix * line.end;
		line.end += *origin;
		line.end[2] = 0;
	}
}

void FrameBuffer::draw() {
	cout << "DRAWING.";
	
	// PRECOMPUTE

	// rotate image to show 3D
	M33 rotation_matrix = M33(Dim::Y, 0.1);
	for (LINE& line : lines) {
		line.start = rotation_matrix * line.start;
		line.start[2] = 0;
		line.end = rotation_matrix * line.end;
		line.end[2] = 0;
	}

	// create and determine pixels to render
	vector<int[4]> render_boxes(lines.size());
	vector<bool> rendered_pixels(w * h, false);
	for (int i = 0; i < lines.size(); i++) {
		LINE& line = lines[i];
		V3& start = line.start;
		V3& end = line.end;
		
		int (&box)[4] = render_boxes[i];
		box[0] = min(start[Dim::X], end[Dim::X]) - STROKE_WIDTH / 2;
		if (box[0] < 0) box[0] = 0;
		box[1] = min(start[Dim::Y], end[Dim::Y]) - STROKE_WIDTH / 2;
		if (box[1] < 0) box[1] = 0;
		box[2] = max(start[Dim::X], end[Dim::X]) + STROKE_WIDTH / 2;
		if (box[2] >= w) box[2] = w - 1;
		box[3] = max(start[Dim::Y], end[Dim::Y]) + STROKE_WIDTH / 2;
		if (box[3] >= h) box[3] = h - 1;

		for (int x = box[0]; x <= box[2]; x++) {
			for (int y = box[1]; y <= box[3]; y++) {
				int p = y * w + x;
				rendered_pixels[p] = true;
			}
		}
	}
	// line_vec and inv dot precompute
	vector<V3> line_vecs(lines.size());
	vector<float> inv_dots(lines.size());
	for (int i = 0; i < lines.size(); i++) {
		V3& start = lines[i].start;
		V3& end = lines[i].end;
		V3 line_vec = end - start;
		line_vecs[i] = line_vec;
		inv_dots[i] = 1 / (line_vec * line_vec);
	}

	// COMPUTE PIXELS

	for (int y = 0; y < h; y++) {
		for (int x = 0; x < w; x++) {
			int p = y * w + x;
			pix[p] = COLOR(0, 0, 0);

			// Filter out pixels we not in any boxes.
			if (!rendered_pixels[p]) {
				continue;
			}
			// pix[p] = COLOR(0, 0, 0);
			
			// Color pixel from overlapping lines.
			for (int i = 0; i < lines.size(); i++) {
				// Filter out pixels not in this box.
				int (&box)[4] = render_boxes[i];
				if (x < box[0] || x > box[2] || y < box[1] || y > box[3]) {
					continue;
				}

				// Compute distance from line.
				LINE& line = lines[i];
				V3 line_vec = line_vecs[i];
				V3 delta = V3(x, y, 0) - line.start;
				line_vec *= (delta * line_vec) * inv_dots[i]; // project delta onto line
				V3 dist_vec = delta - line_vec;
				float dist_sq = dist_vec * dist_vec;

				// Determine color from distance.
				float d = min(STROKE_WIDTH * STROKE_WIDTH * 0.25f - dist_sq, 5.0f);
				if (d >= 0) {
					pix[p] = line.getColor(0.2f * d);
					break;
				}
			}
		}
	}

	glDrawPixels(w, h, GL_RGBA, GL_UNSIGNED_BYTE, pix);
}

int FrameBuffer::handle(int event) {
	switch (event) {
		case FL_KEYBOARD: {
			KeyboardHandle();
			return 0;
		}
		case FL_MOVE: {
			int u = Fl::event_x();
			int v = Fl::event_y();
			cerr << u << " " << v << "      \r";
			return 0;
		}
		default:
			return 0;
	}
	return 0;
}

void FrameBuffer::KeyboardHandle() {
	int key = Fl::event_key();
	switch (key) {
		case FL_Left:
			cerr << "INFO: pressed left" << endl;
			break;
		default:
			cerr << "INFO: do not understand keypress" << endl;
			return;
	}
}

void FrameBuffer::SetBGR(unsigned int bgr) {
	for (int uv = 0; uv < w*h; uv++)
		pix[uv] = bgr;
}