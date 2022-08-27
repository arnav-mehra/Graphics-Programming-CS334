#include "Dimension.hpp"
#include <cmath>
#include <iostream>
using namespace std;

#ifndef V3_HPP
#define V3_HPP

class V3 {
    private:

        float vector[3];

    public:

        V3() {}
        V3(float x, float y, float z) {
            (*this)[Dim::X] = x;
            (*this)[Dim::Y] = y;
            (*this)[Dim::Z] = z;
        }

        // Get and Set each Dim value.
        inline float& operator[](Dim dim) {
            return this->vector[dim];
        }
        inline float& operator[](int i) {
            return this->vector[i];
        }

        // Print vector values.
        void print() {
            cout << '<' << (*this)[Dim::X] << ", " << (*this)[Dim::Y] << ", " << (*this)[Dim::Z] << ">\n";
        }

        // Read vector vales.
        void write() {
            cout << "Please enter vector values:\n";
            cout << "X: ";
            float f;
            cin >> f;
            (*this)[Dim::X] = f;
            cout << "Y: ";
            cin >> f;
            (*this)[Dim::Y] = f;
            cout << "Z: ";
            cin >> f;
            (*this)[Dim::Z] = f;
        }

        // Multiply vector by scalar.
        inline V3& operator*(float scalar) {
            V3* result = new V3(
                (*this)[Dim::X] * scalar,
                (*this)[Dim::Y] * scalar,
                (*this)[Dim::Z] * scalar
            );
            return *result;
        }
        inline void operator*=(float scalar) {
            (*this)[Dim::X] *= scalar;
            (*this)[Dim::Y] *= scalar;
            (*this)[Dim::Z] *= scalar;
        }

        // Divide vector by scalar.
        inline void operator/(float scalar) {
            (*this) * (1 / scalar);
        }
        inline void operator/=(float scalar) {
            (*this) *= (1 / scalar);
        }

        // Get length of vector.
        inline float length() {
            const float selfDot =
                (*this)[Dim::X] * (*this)[Dim::X]
                + (*this)[Dim::Y] * (*this)[Dim::Y]
                + (*this)[Dim::Z] * (*this)[Dim::Z];
            return sqrt(selfDot);
        }
        inline float size() { return this->length(); }
        inline float magnitude() { return this->length(); }

        // Normalize vector (scale magnitude to 1).
        inline void normalize() {
            float scalar = 1 / this->length();
            (*this) *= scalar;
        }
        // Not trying to cheat here, I just think its cool.
        // Source: https://en.wikipedia.org/wiki/Fast_inverse_square_root
        // Note: Slightly faster, but far more inaccurate. 
        inline void normalize_quake3() {
            float scalar =
                (*this)[Dim::X] * (*this)[Dim::X]
                + (*this)[Dim::Y] * (*this)[Dim::Y]
                + (*this)[Dim::Z] * (*this)[Dim::Z];
            quake3(scalar);
            (*this) *= scalar;
        }
        inline void quake3(float &y) {
            float x2 = y * 0.5F;
            long i = *(long*) &y;
            i = 0x5f3759df - (i >> 1);
            y = *(float*) &i;
            y *= (1.5F - (x2 * y * y));
        }

        // Dot product with another vector.
        inline float operator*(V3& vector) {
            return (*this)[Dim::X] * vector[Dim::X]
                + (*this)[Dim::Y] * vector[Dim::Y]
                + (*this)[Dim::Z] * vector[Dim::Z];
        }

        // Cross product with another vector.
        inline V3& operator^(V3& vector) {
            V3* result  = new V3(0, 0, 0);
            (*result)[Dim::X] =
                (*this)[Dim::Y] * vector[Dim::Z]
                - (*this)[Dim::Z] * vector[Dim::Y];
            (*result)[Dim::Y] =
                (*this)[Dim::Z] * vector[Dim::X]
                - (*this)[Dim::X] * vector[Dim::Z];
            (*result)[Dim::Z] =
                (*this)[Dim::X] * vector[Dim::Y]
                - (*this)[Dim::Y] * vector[Dim::X];
            return *result;
        }

        // Add with another vector.
        inline V3& operator+(V3& vector) {
            V3* result  = new V3(
                (*this)[Dim::X] + vector[Dim::X],
                (*this)[Dim::Y] + vector[Dim::Y],
                (*this)[Dim::Z] + vector[Dim::Z]
            );
            return *result;
        }
        inline void operator+=(V3& vector) {
            (*this)[Dim::X] += vector[Dim::X];
            (*this)[Dim::Y] += vector[Dim::Y];
            (*this)[Dim::Z] += vector[Dim::Z];
        }

        // Subtract with another vector.
        inline V3& operator-(V3& vector) {
            V3* result  = new V3(
                (*this)[Dim::X] - vector[Dim::X],
                (*this)[Dim::Y] - vector[Dim::Y],
                (*this)[Dim::Z] - vector[Dim::Z]
            );
            return *result;
        }
        inline void operator-=(V3& vector) {
            (*this)[Dim::X] -= vector[Dim::X];
            (*this)[Dim::Y] -= vector[Dim::Y];
            (*this)[Dim::Z] -= vector[Dim::Z];
        }

        // Rotate point (this) about some axis.
        void rotate(V3& axis1, V3& axis2, float alpha) {
            V3 axis = axis2 - axis1;
            V3 diff = (*this) - axis1;

            float proj_magnitude = (axis * diff) / (axis * axis);
            V3 offset = diff - axis * proj_magnitude;
            (*this) -= offset;

            // reflect this about X axis
            // , then Z, vector normally, save for matrix time.


            (*this) += offset;
        }

        // Rotate vector (this) about some axis.
        void rotate(V3 &axis, float alpha) {

        }
};

#endif