#include <vector>
#include <iostream>
#include <tiffio.h>
#include <algorithm>
#include <chrono>
#include <cmath>

#include "framebuffer.h"
#include "_Geometry.hpp"

#define COLOR(r,g,b) ((b << 16) | (g << 8) | r)

#define DOT_SIZE 8
#define STROKE_WIDTH 4
const int HALF_DOT = DOT_SIZE >> 1;
const int HALF_DOT_SQUARE = HALF_DOT * HALF_DOT;
const int HALF_STROKE = STROKE_WIDTH >> 1;
const float HALF_STROKE_SQUARE = HALF_STROKE * HALF_STROKE;

using namespace std;

FrameBuffer::FrameBuffer(int u0, int v0, int _w, int _h) : Fl_Gl_Window(u0, v0, _w, _h, 0) {
	w = _w;
	h = _h;
	pix = new unsigned int[w * h];
	precompute = PRECOMPUTE_GEOMETRY();
}

void FrameBuffer::draw() {
	// auto time_start = std::chrono::system_clock::now();

	vector<float> z_index(w * h, FLT_MAX);

	for (LINE3& line : precompute.lines) {
		V3& start = line.start;
		V3& end = line.end;

		// determine box
		int min_x = (int) min(start[Dim::X], end[Dim::X]) - HALF_STROKE;
		if (min_x < 0) min_x = 0;
		int min_y = (int) min(start[Dim::Y], end[Dim::Y]) - HALF_STROKE;
		if (min_y < 0) min_y = 0;
		int max_x = (int) max(start[Dim::X], end[Dim::X]) + HALF_STROKE;
		if (max_x >= w) max_x = w - 1;
		int max_y = (int) max(start[Dim::Y], end[Dim::Y]) + HALF_STROKE;
		if (max_y >= h) max_y = h - 1;

		// precompute line_vec unit vector as if z = 0
		V3 line_vec = end - start;
		float proj_den = line_vec[Dim::X] * line_vec[Dim::X] + line_vec[Dim::Y] * line_vec[Dim::Y];
		line_vec *= 1 / sqrt(proj_den);

		// iterate over box pixels
		for (int y = min_y; y <= max_y; y++) {
			for (int x = min_x; x <= max_x; x++) {
				// project delta onto line as if z = 0
				float dx = x - start[Dim::X];
				float dy = y - start[Dim::Y];
				const float proj_num = dx * line_vec[Dim::X] + dy * line_vec[Dim::Y];
				V3 proj = line_vec * proj_num;

				// calculate distance while ignoring z.
				dx -= proj[Dim::X];
				dy -= proj[Dim::Y];
				const float dist_sq = dx * dx + dy * dy;

				// determine squared distance from line
				if (HALF_STROKE_SQUARE < dist_sq) continue;

				// check if z if high enough to render over another item.
				const float z_value = proj[Dim::Z] + start[Dim::Z];
				const int p = y * w + x;
				if (z_value > z_index[p]) continue;
				z_index[p] = z_value;

				// overwrite pixel color
				const float d = min(HALF_STROKE_SQUARE - dist_sq, 5.0f);
				pix[p] = line.getColor(0.2f * d);
			}
		}
	}

	for (POINT3& point3 : precompute.points) {
		V3& point = point3.point;

		int min_x = (int) point[Dim::X] - HALF_DOT;
		if (min_x < 0) min_x = 0;
		int min_y = (int) point[Dim::Y] - HALF_DOT;
		if (min_y < 0) min_y = 0;
		int max_x = (int) point[Dim::X] + HALF_DOT;
		if (max_x >= w) max_x = w - 1;
		int max_y = (int) point[Dim::Y] + HALF_DOT;
		if (max_y >= h) max_y = h - 1;

		for (int y = min_y; y <= max_y; y++) {
			for (int x = min_x; x <= max_x; x++) {
				// project delta onto line as if z = 0
				float dx = x - point[Dim::X];
				float dy = y - point[Dim::Y];
				const float dist_sq = dx * dx + dy * dy;

				// determine squared distance from point
				if (HALF_DOT_SQUARE < dist_sq) continue;

				// check if z if high enough to render over another item.
				const int p = y * w + x;
				if (point[Dim::Z] > z_index[p]) continue;
				z_index[p] = point[Dim::Z];

				// overwrite pixel color
				const float d = min(HALF_DOT_SQUARE - dist_sq, 5.0f);
				pix[p] = point3.getColor(0.2f * d);
			}
		}
	}

	// auto time_end = std::chrono::system_clock::now();
	// cout << "RENDERED IN: " << (time_end - time_start).count() * 1e-6 << "\n";
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