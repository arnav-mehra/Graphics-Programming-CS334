#pragma once

#include <iostream>
#include <cmath>

#include "Dimension.hpp"

#define PI 3.14159265358979323846
#define DEG_TO_RAD(x) ((x) * ((float) PI / 180.0f))

using namespace std;

class V3 {
private:
    float vector[3];

public:
    V3();
    V3(float x, float y, float z);

    // Get and Set each Dim value.
    float& operator[](Dim dim);
    float& operator[](int i);

    // Comparison
    friend bool operator==(V3& v1, V3& v2);

    // Print and write in vector values.
    friend ostream& operator<<(ostream& out, V3& vector);
    friend istream& operator>>(istream& in, V3& vector);

    // Get length of vector.
    float length();
    float size();
    float magnitude();

    // Multiply and divide vector by scalar.
    V3 operator*(float scalar);
    void operator*=(float scalar);
    V3 operator/(float scalar);
    void operator/=(float scalar);

    // Normalize vector (scale magnitude to 1).
    void normalize();
    void normalize_quake3();
    void quake3(float& y);

    // Dot product with another vector.
    float operator*(V3& vector);
    // Cross product with another vector.
    V3 operator^(V3& vector);
    float V3::cross_z(V3& vector);

    // Add and subtract vectors.
    V3 operator+(V3& vector);
    void operator+=(V3& vector);
    V3 operator-(V3& vector);
    void operator-=(V3& vector);

    // Rotate point (this) about axis alpha radians
    void rotate(V3& axis1, V3& axis2, float alpha);
    // Rotate vector (this) about vector axis alpha radians
    void rotate(V3& axis, float alpha);
};