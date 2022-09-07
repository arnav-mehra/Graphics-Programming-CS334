#include <vector>
#include <iostream>
#include <tiffio.h>
#include <algorithm>
#include <chrono>
#include <cmath>

#include "framebuffer.h"
#include "_Geometry.hpp"

using namespace std;

FrameBuffer::FrameBuffer(int u0, int v0, int _w, int _h) : Fl_Gl_Window(u0, v0, _w, _h, 0) {
	w = _w;
	h = _h;
	pix = new unsigned int[w * h];
}

void FrameBuffer::apply_geometry() {
	precompute = PRECOMPUTE_GEOMETRY();
	vector<float> z_index(w * h, FLT_MAX);

	for (int i = 0; i < precompute.num_segments; i++) {
		SEGMENT& segment = precompute.segments[i];
		V3& start = segment.start;
		V3& end = segment.end;

		// determine box
		int min_x = (int)(min(start[Dim::X], end[Dim::X]) + 0.5f) - HALF_STROKE;
		if (min_x < 0) min_x = 0;
		int min_y = (int)(min(start[Dim::Y], end[Dim::Y]) + 0.5f) - HALF_STROKE;
		if (min_y < 0) min_y = 0;
		int max_x = (int)(max(start[Dim::X], end[Dim::X]) - 0.5f) + HALF_STROKE;
		if (max_x >= w) max_x = w - 1;
		int max_y = (int)(max(start[Dim::Y], end[Dim::Y]) - 0.5f) + HALF_STROKE;
		if (max_y >= h) max_y = h - 1;

		// precompute line_vec unit vector as if z = 0
		V3 line_vec = end - start;
		float proj_den = line_vec[Dim::X] * line_vec[Dim::X] + line_vec[Dim::Y] * line_vec[Dim::Y];
		line_vec *= 1 / sqrt(proj_den);

		// iterate over box pixels
		for (int y = min_y; y <= max_y; y++) {
			for (int x = min_x; x <= max_x; x++) {
				// project delta onto segment as if z = 0
				float dx = x - start[Dim::X];
				float dy = y - start[Dim::Y];
				const float proj_num = dx * line_vec[Dim::X] + dy * line_vec[Dim::Y];
				V3 proj = line_vec * proj_num;

				// calculate distance while ignoring z.
				dx -= proj[Dim::X];
				dy -= proj[Dim::Y];
				const float dist_sq = dx * dx + dy * dy;

				// determine squared distance from segment
				if (HALF_STROKE_SQUARE < dist_sq) continue;

				// check if z if high enough to render over another item.
				const float z_value = proj[Dim::Z] + start[Dim::Z];
				const int p = y * w + x;
				if (z_value > z_index[p]) continue;
				z_index[p] = z_value;

				// overwrite pixel color
				const float d = min(HALF_STROKE_SQUARE - dist_sq, 5.0f);
				pix[p] = segment.scaleColor(0.2f * d);
			}
		}
	}

	for (int i = 0; i < precompute.num_spheres; i++) {
		SPHERE& sphere = precompute.spheres[i];
		V3& point = sphere.point;

		int min_x = (int)(point[Dim::X] + 0.5f) - HALF_DOT;
		if (min_x < 0) min_x = 0;
		int min_y = (int)(point[Dim::Y] + 0.5f) - HALF_DOT;
		if (min_y < 0) min_y = 0;
		int max_x = (int)(point[Dim::X] - 0.5f) + HALF_DOT;
		if (max_x >= w) max_x = w - 1;
		int max_y = (int)(point[Dim::Y] - 0.5f) + HALF_DOT;
		if (max_y >= h) max_y = h - 1;

		for (int y = min_y; y <= max_y; y++) {
			for (int x = min_x; x <= max_x; x++) {
				// project delta onto segment as if z = 0
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
				pix[p] = sphere.scaleColor(0.2f * d);
			}
		}
	}
}

void FrameBuffer::draw() {
	auto time_start = std::chrono::system_clock::now();

	

	auto time_end = std::chrono::system_clock::now();
	cout << "RENDERED IN: " << (time_end - time_start).count() * 1e-6 << "\n";
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

// load a tiff image to pixel buffer
void FrameBuffer::LoadTiff(char* fname) {
	TIFF* in = TIFFOpen(fname, "r");

	if (in == NULL) {
		cout << fname << " could not be opened" << endl;
		return;
	}

	int width, height;
	TIFFGetField(in, TIFFTAG_IMAGEWIDTH, &width);
	TIFFGetField(in, TIFFTAG_IMAGELENGTH, &height);
	if (w != width || h != height) {
		w = width;
		h = height;
		delete[] pix;
		pix = new unsigned int[w * h];
		size(w, h);
		glFlush();
		glFlush();
	}

	if (TIFFReadRGBAImage(in, w, h, pix, 0) == 0) {
		cout << "failed to load " << fname << endl;
	}
	cout << "image read in\n";

	TIFFClose(in);
}

// save as tiff image
void FrameBuffer::SaveAsTiff(char* fname) {

	TIFF* out = TIFFOpen(fname, "w");

	if (out == NULL) {
		cout << fname << " could not be opened" << endl;
		return;
	}

	TIFFSetField(out, TIFFTAG_IMAGEWIDTH, w);
	TIFFSetField(out, TIFFTAG_IMAGELENGTH, h);
	TIFFSetField(out, TIFFTAG_SAMPLESPERPIXEL, 4);
	TIFFSetField(out, TIFFTAG_BITSPERSAMPLE, 8);
	TIFFSetField(out, TIFFTAG_ORIENTATION, ORIENTATION_TOPLEFT);
	TIFFSetField(out, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);
	TIFFSetField(out, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_RGB);

	for (uint32 row = 0; row < (unsigned int)h; row++) {
		TIFFWriteScanline(out, &pix[(h - row - 1) * w], row);
	}

	TIFFClose(out);
}