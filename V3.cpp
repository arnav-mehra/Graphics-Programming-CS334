#include <iostream>
#include <cmath>

#include "Dimension.hpp"
#include "V3.hpp"
#include "M33.hpp"

using namespace std;

istream& operator>>(istream& in, V3& vector) {
    cout << "Please enter vector values:\n";
    cout << "X: ";
    in >> vector[Dim::X];
    cout << "Y: ";
    in >> vector[Dim::Y];
    cout << "Z: ";
    in >> vector[Dim::Z];
    return in;
}

ostream& operator<<(ostream& out, V3& vector) {
    out << '<' << vector[Dim::X]
        << ", " << vector[Dim::Y]
        << ", " << vector[Dim::Z]
        << ">\n";
    return out;
}

V3::V3() {}

V3::V3(float x, float y, float z) {
    (*this)[Dim::X] = x;
    (*this)[Dim::Y] = y;
    (*this)[Dim::Z] = z;
}

float& V3::operator[](Dim dim) {
    return this->vector[dim];
}

float& V3::operator[](int i) {
    return this->vector[i];
}

bool operator==(V3& v1, V3& v2) {
    return v1[Dim::X] == v2[Dim::X]
        && v1[Dim::Y] == v2[Dim::Y]
        && v1[Dim::Z] == v2[Dim::Z];
}

float V3::length() {
    const float len_sq = (*this) * (*this);
    return sqrt(len_sq);
}

float V3::size() { return this->length(); }

float V3::magnitude() { return this->length(); }

V3 V3::operator*(float scalar) {
    return V3(
        (*this)[Dim::X] * scalar,
        (*this)[Dim::Y] * scalar,
        (*this)[Dim::Z] * scalar
    );
}

void V3::operator*=(float scalar) {
    (*this)[Dim::X] *= scalar;
    (*this)[Dim::Y] *= scalar;
    (*this)[Dim::Z] *= scalar;
}

V3 V3::operator/(float scalar) {
    return (*this) * (1.0f / scalar);
}

void V3::operator/=(float scalar) {
    (*this) *= (1.0f / scalar);
}

void V3::normalize() {
    float scalar = 1.0f / this->length();
    (*this) *= scalar;
}

void V3::normalize_quake3() {
    float scalar =
        (*this)[Dim::X] * (*this)[Dim::X]
        + (*this)[Dim::Y] * (*this)[Dim::Y]
        + (*this)[Dim::Z] * (*this)[Dim::Z];
    quake3(scalar);
    (*this) *= scalar;
}

// Cool trick (fast inv sqrt).
// Source: https://en.wikipedia.org/wiki/Fast_inverse_square_root
// Note: Slightly faster, but far less accurate.
void V3::quake3(float &y) {
    float x2 = y * 0.5f;
    long i = *(long*) &y;
    i = 0x5f3759df - (i >> 1);
    y = *(float*) &i;
    y *= (1.5f - (x2 * y * y));
}

float V3::operator*(V3& vector) {
    return (*this)[Dim::X] * vector[Dim::X]
        + (*this)[Dim::Y] * vector[Dim::Y]
        + (*this)[Dim::Z] * vector[Dim::Z];
}

V3 V3::operator^(V3& vector) {
    V3 result = V3();
    result[Dim::X] =
        (*this)[Dim::Y] * vector[Dim::Z]
        - (*this)[Dim::Z] * vector[Dim::Y];
    result[Dim::Y] =
        (*this)[Dim::Z] * vector[Dim::X]
        - (*this)[Dim::X] * vector[Dim::Z];
    result[Dim::Z] =
        (*this)[Dim::X] * vector[Dim::Y]
        - (*this)[Dim::Y] * vector[Dim::X];
    return result;
}

float V3::cross_z(V3& vector) {
    return (*this)[Dim::X] * vector[Dim::Y]
        - (*this)[Dim::Y] * vector[Dim::X];
}

V3 V3::operator+(V3& vector) {
    V3 result = V3(
        (*this)[Dim::X] + vector[Dim::X],
        (*this)[Dim::Y] + vector[Dim::Y],
        (*this)[Dim::Z] + vector[Dim::Z]
    );
    return result;
}

void V3::operator+=(V3& vector) {
    (*this)[Dim::X] += vector[Dim::X];
    (*this)[Dim::Y] += vector[Dim::Y];
    (*this)[Dim::Z] += vector[Dim::Z];
}

V3 V3::operator-(V3& vector) {
    return V3(
        (*this)[Dim::X] - vector[Dim::X],
        (*this)[Dim::Y] - vector[Dim::Y],
        (*this)[Dim::Z] - vector[Dim::Z]
    );
}

void V3::operator-=(V3& vector) {
    (*this)[Dim::X] -= vector[Dim::X];
    (*this)[Dim::Y] -= vector[Dim::Y];
    (*this)[Dim::Z] -= vector[Dim::Z];
}

void V3::rotate(V3& axis1, V3& axis2, float alpha) {
    // translate all points so axis1 is at <0, 0, 0>
    axis2 -= axis1;
    (*this) -= axis1;
    // rotate about new axis vector (axis 2)
    rotate(axis2, alpha);
    // undo translate
    (*this) += axis1;
    axis2 += axis1;
}

void V3::rotate(V3& axis, float alpha) {
    // xy rotation to eliminate axis y dimension
    float xy_len_inverse = 1.0f / sqrt(axis[0] * axis[0] + axis[1] * axis[1]);
    float xy_cos_theta = axis[0] * xy_len_inverse;
    float xy_sin_theta = axis[1] * xy_len_inverse;
    M33 xy_rotation = M33(Dim::Z, -xy_sin_theta, xy_cos_theta);
    M33 xy_rotation_inv = M33(Dim::Z, xy_sin_theta, xy_cos_theta);
    // perform xy rotation
    axis = xy_rotation * axis;
    (*this) = xy_rotation * (*this);

    // xz rotation to eliminate axis z dimension
    float xz_len_inverse = 1.0f / sqrt(axis[0] * axis[0] + axis[2] * axis[2]);
    float xz_cos_theta = axis[0] * xz_len_inverse;
    float xz_sin_theta = axis[2] * xz_len_inverse;
    M33 xz_rotation = M33(Dim::Y, xz_sin_theta, xz_cos_theta);
    M33 xz_rotation_inv = M33(Dim::Y, -xz_sin_theta, xz_cos_theta);
    // perform xz rotation (axis no longer needed)
    (*this) = xz_rotation * (*this);

    // axis lays entirely on x-axis, rotate alpha degrees
    M33 yz_rotation = M33(Dim::X, alpha);
    (*this) = yz_rotation * (*this);

    // revert xy + xz rotations
    (*this) = xz_rotation_inv * (*this);
    (*this) = xy_rotation_inv * (*this);
}