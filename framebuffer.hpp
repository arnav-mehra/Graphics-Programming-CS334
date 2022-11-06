#pragma once

#include <FL/Fl.H>
#include <FL/Fl_Gl_Window.H>
#include <GL/glut.h>

#include "ppc.hpp"
#include "Geometry.hpp"

extern bool USE_MIPMAP;

class BACKDROP {
public:
	V3 norms[6] = {
		V3(-1.0f, 0.0f, 0.0f),
		V3(0.0f, 1.0f, 0.0f),
		V3(0.0f, 0.0f, 1.0f),
		V3(1.0f, 0.0f, 0.0f),
		V3(0.0f, -1.0f, 0.0f),
		V3(0.0f, 0.0f, -1.0f),
	};
	TEXTURE textures[6];
	enum Dir {
		RIGHT = 0,
		UP = 1,
		FRONT = 2,
		LEFT = 3,
		DOWN = 4,
		BACK = 5,
	};

	BACKDROP(char* fname) {
		TIFF* in = TIFFOpen(fname, "r");
		if (in == NULL) return;
		int w, h;
		TIFFGetField(in, TIFFTAG_IMAGEWIDTH, &w);
		TIFFGetField(in, TIFFTAG_IMAGELENGTH, &h);
		U32* pix = new U32[w * h];
		if (TIFFReadRGBAImage(in, w, h, pix, 0) == 0) {
			cout << "failed to load " << endl;
			return;
		}
		TIFFClose(in);

		if (fname[0] == 'u') { // 334 format
			int iw = w / 3;
			int ih = h / 4;

			for (int i = 0; i < 6; i++) {
				textures[i].texture.resize(ih, vector<COLOR>(iw));
				textures[i].w = iw;
				textures[i].h = ih;
			}

			for (int i = 0; i < iw; i++) {
				for (int j = 0; j < ih; j++) {
					textures[Dir::RIGHT].texture[j][i] = COLOR(pix[(ih * 2 + j) * w + (iw * 2 + i)]);
					textures[Dir::LEFT].texture[j][i] = COLOR(pix[(ih * 2 + j) * w + (iw * 0 + i)]);

					textures[Dir::UP].texture[j][i] = COLOR(pix[(ih * 3 + j) * w + (iw * 1 + i)]);
					textures[Dir::DOWN].texture[j][i] = COLOR(pix[(ih * 1 + j) * w + (iw * 1 + i)]);

					textures[Dir::FRONT].texture[j][i] = COLOR(pix[(ih * 2 + j) * w + (iw * 1 + i)]);
					textures[Dir::BACK].texture[j][i] = COLOR(pix[(ih * 0 + j) * w + (iw * 1 + i)]);
				}
			}

			textures[Dir::RIGHT].transform(false, false, true);
			textures[Dir::FRONT].transform(true, true, false);
			textures[Dir::BACK].transform(true, false, false);
			textures[Dir::UP].transform(true, true, true);
			textures[Dir::DOWN].transform(true, false, true);
		}
		else if (fname[0] == 'p') { // panarama format
			int iw = w / 4;
			int ih = h;

			for (int i = 0; i < 6; i++) {
				textures[i].texture.resize(ih, vector<COLOR>(iw));
				textures[i].w = iw;
				textures[i].h = ih;
			}

			for (int i = 0; i < iw; i++) {
				for (int j = 0; j < ih; j++) {
					textures[Dir::RIGHT].texture[j][i] = COLOR(pix[(ih * 0 + j) * w + (iw * 2 + i)]);
					textures[Dir::LEFT].texture[j][i] = COLOR(pix[(ih * 0 + j) * w + (iw * 0 + i)]);

					textures[Dir::UP].texture[j][i] = COLOR(0, 0, 0);
					textures[Dir::DOWN].texture[j][i] = COLOR(0, 0, 0);

					textures[Dir::FRONT].texture[j][i] = COLOR(pix[(ih * 0 + j) * w + (iw * 1 + i)]);
					textures[Dir::BACK].texture[j][i] = COLOR(pix[(ih * 0 + j) * w + (iw * 3 + i)]);
				}
			}

			textures[Dir::RIGHT].transform(false, false, true);
			textures[Dir::FRONT].transform(true, true, false);
			textures[Dir::BACK].transform(true, true, true);
		}
		else { // normal format
			int iw = w / 4;
			int ih = h / 3;

			for (int i = 0; i < 6; i++) {
				textures[i].texture.resize(ih, vector<COLOR>(iw));
				textures[i].w = iw;
				textures[i].h = ih;
			}

			for (int i = 0; i < iw; i++) {
				for (int j = 0; j < ih; j++) {
					textures[Dir::RIGHT].texture[j][i] = COLOR(pix[(ih * 1 + j) * w + (iw * 3 + i)]);
					textures[Dir::LEFT].texture[j][i] = COLOR(pix[(ih * 1 + j) * w + (iw * 1 + i)]);

					textures[Dir::UP].texture[j][i] = COLOR(pix[(ih * 2 + j) * w + (iw * 1 + i)]);
					textures[Dir::DOWN].texture[j][i] = COLOR(pix[(ih * 0 + j) * w + (iw * 1 + i)]);

					textures[Dir::FRONT].texture[j][i] = COLOR(pix[(ih * 1 + j) * w + (iw * 2 + i)]);
					textures[Dir::BACK].texture[j][i] = COLOR(pix[(ih * 1 + j) * w + (iw * 0 + i)]);
				}
			}
			textures[Dir::RIGHT].transform(false, false, true);
			textures[Dir::FRONT].transform(true, true, false);
			textures[Dir::BACK].transform(true, true, true);
			textures[Dir::UP].transform(false, false, true);
			textures[Dir::DOWN].transform(false, true, true);
		}

		for (TEXTURE& tx : textures) {
			tx.initMipMap();
		}
	}

	COLOR getColor(V3 dir) {
		float best = FLT_MAX; int ind = 0;
		for (int i = 0; i < 6; i++) {
			float dot = norms[i] * dir;
			if (dot < best) {
				best = dot;
				ind = i;
			}
		}

		switch (ind % 3) {
		case 0: dir /= dir[Dim::X]; break;
		case 1: dir /= dir[Dim::Y]; break;
		case 2: dir /= dir[Dim::Z]; break;
		}

		dir += V3(1.0f, 1.0f, 1.0f);
		dir *= 0.5f;

		float xt, yt;
		switch (ind % 3) {
			case 0: {
				yt = dir[Dim::Y];
				xt = dir[Dim::Z];
				break;
			}
			case 1: {
				yt = dir[Dim::X];
				xt = dir[Dim::Z];
				break;
			}
			case 2: {
				yt = dir[Dim::X];
				xt = dir[Dim::Y];
				break;
			}
		}

		// mipmap lookup
		if (USE_MIPMAP)	return textures[ind].getMip(xt, yt);

		// bilinear lookup
		yt *= (textures[ind].h - 1);
		xt *= (textures[ind].w - 1);

		float x1 = floorf(xt);
		float x2 = ceilf(xt);
		float y1 = floorf(yt);
		float y2 = ceilf(yt);

		float dx = xt - x1;
		float dy = yt - y1;

		COLOR c1 = textures[ind].texture[y1][x1];
		COLOR c2 = textures[ind].texture[y1][x2];
		COLOR c3 = textures[ind].texture[y2][x1];
		COLOR c4 = textures[ind].texture[y2][x2];
		COLOR inter = c1 * (1.0f - dx) * (1.0f - dy)
			+ c2 * dx * (1.0f - dy)
			+ c3 * (1.0f - dx) * dy
			+ c4 * dx * dy;
		return inter;
	}
};

class FRAMEBUFFER : public Fl_Gl_Window {
public:
	unsigned int *pix; // pixel array
	U32 w, h;
	COMPUTED_GEOMETRY compute;

	PPC cam1;
	PPC cam2;
	PPC cam3;
	PPC cam4;
	float transition1;
	float transition2;
	float transition3;
	U32 frame = 0;

	vector<float> z_buffer; // z-buffer, make w & h defines -> stack
	vector<int> tri_buffer;

	FRAMEBUFFER(int u0, int v0, U32 _w, U32 _h);

	void draw();
	void startThread();
	void KeyboardHandle();
	int handle(int guievent);
	void SetBGR(unsigned int bgr);
	void setBackdrop();
	
	void applyGeometry();
	inline void applySphere(SPHERE& sphere);
	inline void applySegment(U32 i);
	inline void applyTriangle(U32 i);
	void applyTriangleLight(U32 t_i, U32 l_i);
	inline void applyLights();
	inline void applySM2Light(U32 i);

	void LoadTiff();
	void SaveAsTiff();
	void SaveAsTiff(const char* fname);
};