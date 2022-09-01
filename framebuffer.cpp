#include <vector>
#include <iostream>
#include <tiffio.h>
#include <algorithm>

#include "framebuffer.h"

#include "_Geometry.hpp"

#define COLOR(r,g,b) ((b << 16) | (g << 8) | r)

using namespace std;

FrameBuffer::FrameBuffer(int u0, int v0, int _w, int _h) : Fl_Gl_Window(u0, v0, _w, _h, 0) {
	w = _w;
	h = _h;
	pix = new unsigned int[w * h];
	precompute = PRECOMPUTE_GEOMETRY();
}

void FrameBuffer::draw() {
	cout << "DRAWING.";

	for (int y = 0; y < h; y++) {
		for (int x = 0; x < w; x++) {
			int p = y * w + x;
			pix[p] = COLOR(0, 0, 0);

			// Filter out pixels not in any boxes.
			if (!precompute.rendered_pixels[p]) {
				continue;
			}
			// pix[p] = COLOR(5, 5, 5);
			
			// Color pixel from overlapping lines.
			for (int i = 0; i < precompute.lines.size(); i++) {
				// Filter out pixels not in this box.
				vector<int>& box = precompute.render_boxes[i];
				if (x < box[0] || x > box[2] || y < box[1] || y > box[3]) {
					continue;
				}

				// Compute distance from line.
				LINE3& line = precompute.lines[i];
				V3 line_vec = precompute.line_vecs[i];
				V3 delta = V3((float) x, (float) y, 0) - line.start;
				line_vec *= (delta * line_vec) * precompute.inv_dots[i]; // project delta onto line
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

	for (int y = 0; y < h; y++) {
		for (int x = 0; x < w; x++) {
			int p = y * w + x;
			pix[p] = COLOR(0, 0, 0);

			// Filter out pixels not in any boxes.
			if (!precompute.rendered_pixels[p]) {
				continue;
			}
			// pix[p] = COLOR(5, 5, 5);

			// Color pixel from overlapping lines.
			for (int i = 0; i < precompute.lines.size(); i++) {
				// Filter out pixels not in this box.
				vector<int>& box = precompute.render_boxes[i];
				if (x < box[0] || x > box[2] || y < box[1] || y > box[3]) {
					continue;
				}

				// Compute distance from line.
				LINE3& line = precompute.lines[i];
				V3 line_vec = precompute.line_vecs[i];
				V3 delta = V3((float)x, (float)y, 0) - line.start;
				line_vec *= (delta * line_vec) * precompute.inv_dots[i]; // project delta onto line
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