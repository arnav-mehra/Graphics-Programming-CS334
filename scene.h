#pragma once

#include "V3.hpp"
#include "M33.hpp"
#include "Geometry.hpp"
#include "framebuffer.h"
#include "gui.h"

#define PLAY_PONG false
#define PLAY_NAME_SCROLL false
#define SHOW_GEOMETRY false
#define PLAY_TETRIS true
#define TIFF_FILE_IN "name.tif" // what we read from
#define TIFF_FILE_OUT "random.tif" // what we write to

class Scene {
public:
	GUI *gui;
	FrameBuffer *fb;

	int w, h;
	int frame;

	M33 perspective;
	V3 origin;
	GEOMETRY geometry;

	// pong stuff
	V3 player1;
	V3 player2;
	V3 ball_pos;
	V3 ball_vel;
	int s1 = 0, s2 = 0;

	// tetris stuff :|
	bool grid[20][10];
	bool shapes[3][3][3] = {
		{
			{ 0, 1, 0 },
			{ 1, 1, 1 },
			{ 0, 0, 1 }
		},
		{
			{ 0, 1, 0 },
			{ 1, 1, 0 },
			{ 0, 1, 0 }
		},
		{
			{ 0, 1, 0 },
			{ 1, 1, 0 },
			{ 0, 1, 0 }
		}
	};
	int curr_shape = -1;
	pair<int, int> pos = { 17, 0 };
	int score = 0;
	int high_score = 0;

	void check_and_remove_lines() {
		for (int r = 0; r < 20; r++) {
			bool is_line = true;
			for (int c = 0; c < 10; c++) {
				if (!grid[r][c]) {
					is_line = false;
					break;
				}
			}
			if (is_line) {
				inc_score();
				for (int i = r + 1; i < 20; i++) {
					for (int j = 0; j < 10; j++) {
						grid[i - 1][j] = grid[i][j];
					}
				}
			}
		}
	}

	void inc_score() {
		score++;
		if (high_score > score) high_score = score;
	}

	void add_shape() {
		int shape_num = (curr_shape + 1) % 3;
		for (int c = 0; c < 10; c++) {
			bool fits = will_fit({ 0, c }, shape_num);
			if (fits) {
				curr_shape = shape_num;
				pos = { 17, c };
				return;
			}
		}
		// END GAME.
		reset();
	}

	void reset() {
		score = 0;
		for (int r = 0; r < 20; r++) {
			for (int c = 0; c < 10; c++) {
				grid[r][c] = 0;
			}
 		}
		pos = { 0, 19 };
	}

	void drop_shape() {
		pair<int, int> new_pos = { pos.first - 1, pos.second };
		if (will_fit(new_pos, curr_shape)) {
			pos = new_pos;
		}
		else {
			place_shape();
		}
	}

	void place_shape() {
		for (int i = 0; i < 3; i++) {
			for (int j = 0; j < 3; j++) {
				int r = pos.first + i;
				int c = pos.second + j;
				if (shapes[curr_shape][i][j])
					grid[r][c] = shapes[curr_shape][i][j];
			}
		}
		curr_shape = -1;
	}

	bool will_fit(pair<int, int> p, int shape_num) {
		for (int i = 0; i < 3; i++) {
			for (int j = 0; j < 3; j++) {
				if (shapes[shape_num][i][j]) {
					int r = p.first + i;
					int c = p.second + j;
					if (c < 0 || c > 9) return false; // out of bounds bit
					if (r < 0) return false;
					if (grid[r][c]) return false; // already a 1 there
				}
			}
		}
		return true;
	}

	void rotate() {
		clock();
		if (will_fit(pos, curr_shape)) {
			return;
		}
		clock();
		clock();
		clock();
	}

	void clock() {
		bool(&shape)[3][3] = shapes[curr_shape];
		int temp = shape[0][0];
		shape[0][0] = shape[0][2];
		shape[0][2] = shape[2][2];
		shape[2][2] = shape[2][0];
		shape[2][0] = temp;

		temp = shape[0][1];
		shape[0][1] = shape[1][2];
		shape[1][2] = shape[2][1];
		shape[2][1] = shape[1][0];
		shape[1][0] = temp;
	}
	
	void move_left() {
		pair<int, int> new_pos = { pos.first, pos.second - 1 };
		if (will_fit(new_pos, curr_shape)) {
			pos = new_pos;
		}
	}
	void move_right() {
		pair<int, int> new_pos = { pos.first, pos.second + 1 };
		if (will_fit(new_pos, curr_shape)) {
			pos = new_pos;
		}
	}


	Scene();
	void LoadTiffButton();
	void SaveTiffButton();
	void TranslateImage();
};

extern Scene *scene;