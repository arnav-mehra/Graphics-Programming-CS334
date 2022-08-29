#pragma once

#include "V3.hpp"

class GEOMETRY {
	public:
		V3 color;

		inline unsigned int getColor() {
			unsigned int r = color[0];
			unsigned int g = color[1];
			unsigned int b = color[2];
			return (b << 16) | (g << 8) | r;
		}

		inline unsigned int getColor(float c) {
			unsigned int r = color[0] * c;
			unsigned int g = color[1] * c;
			unsigned int b = color[2] * c;
			return (b << 16) | (g << 8) | r;
		}
};

class LINE : public GEOMETRY {
	public:
		V3 start;
		V3 end;

		LINE() {}

		LINE(V3& start, V3& end) {
			this->start = start;
			this->end = end;
		}

		LINE(V3& start, V3& end, V3& color) {
			this->start = start;
			this->end = end;
			this->color = color;
		}
};

class POINT : public GEOMETRY {
	public:
		V3 point;

		POINT() {}

		POINT(V3& point) {
			this->point = point;
		}

		POINT(V3& point, V3& color) {
			this->point = point;
			this->color = color;
		}
};

class POLY : public GEOMETRY {
	public:
		vector<V3> points;

		POLY() {}

		POLY(vector<V3>& points) {
			this->points = points;
		}

		POLY(vector<V3>& points, V3& color) {
			this->points = points;
			this->color = color;
		}
};