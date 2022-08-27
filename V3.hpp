#include "Dimension.hpp"
#include <cmath>
#include <iostream>
using namespace std;

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
        inline float& operator[](Dim dim);
        inline float& operator[](int i);

        // Print and write in vector values.
        friend ostream& operator<<(ostream& out, V3& vector);
        friend istream& operator>>(istream& in, V3& vector);

        // Get length of vector.
        inline float length();
        inline float size();
        inline float magnitude();

        // Multiply and divide vector by scalar.
        inline V3& operator*(float scalar);
        inline void operator*=(float scalar);
        inline void operator/(float scalar);
        inline void operator/=(float scalar);

        // Normalize vector (scale magnitude to 1).
        inline void normalize();
        inline void normalize_quake3();
        inline void quake3(float& y);

        // Dot product with another vector.
        inline float operator*(V3& vector);
        // Cross product with another vector.
        inline V3& operator^(V3& vector);

        // Add and subtract vectors.
        inline V3& operator+(V3& vector);
        inline void operator+=(V3& vector);
        inline V3& operator-(V3& vector);
        inline void operator-=(V3& vector);

        // Rotate point (this) about axis alpha radians
        void rotate(V3& axis1, V3& axis2, float alpha);
        // Rotate vector (this) about vector axis alpha radians
        void rotate(V3& axis, float alpha);
};