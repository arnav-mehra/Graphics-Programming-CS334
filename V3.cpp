#ifndef V3_CPP
#define V3_CPP

#include "V3.hpp"
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

inline float& V3::operator[](Dim dim) {
    return this->vector[dim];
}

inline float& V3::operator[](int i) {
    return this->vector[i];
}

inline float V3::length() {
    const float selfDot =
        (*this)[Dim::X] * (*this)[Dim::X]
        + (*this)[Dim::Y] * (*this)[Dim::Y]
        + (*this)[Dim::Z] * (*this)[Dim::Z];
    return sqrt(selfDot);
}

inline float V3::size() { return this->length(); }

inline float V3::magnitude() { return this->length(); }

inline V3& V3::operator*(float scalar) {
    V3* result = new V3(
        (*this)[Dim::X] * scalar,
        (*this)[Dim::Y] * scalar,
        (*this)[Dim::Z] * scalar
    );
    return *result;
}

inline void V3::operator*=(float scalar) {
    (*this)[Dim::X] *= scalar;
    (*this)[Dim::Y] *= scalar;
    (*this)[Dim::Z] *= scalar;
}

inline void V3::operator/(float scalar) {
    (*this) * (1 / scalar);
}

inline void V3::operator/=(float scalar) {
    (*this) *= (1 / scalar);
}

inline void V3::normalize() {
    float scalar = 1 / this->length();
    (*this) *= scalar;
}

inline void V3::normalize_quake3() {
    float scalar =
        (*this)[Dim::X] * (*this)[Dim::X]
        + (*this)[Dim::Y] * (*this)[Dim::Y]
        + (*this)[Dim::Z] * (*this)[Dim::Z];
    quake3(scalar);
    (*this) *= scalar;
}

// Not trying to cheat here, I just think its cool.
// Source: https://en.wikipedia.org/wiki/Fast_inverse_square_root
// Note: Slightly faster, but far more inaccurate.
inline void V3::quake3(float &y) {
    float x2 = y * 0.5F;
    long i = *(long*) &y;
    i = 0x5f3759df - (i >> 1);
    y = *(float*) &i;
    y *= (1.5F - (x2 * y * y));
}

inline float V3::operator*(V3& vector) {
    return (*this)[Dim::X] * vector[Dim::X]
        + (*this)[Dim::Y] * vector[Dim::Y]
        + (*this)[Dim::Z] * vector[Dim::Z];
}

inline V3& V3::operator^(V3& vector) {
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

inline V3& V3::operator+(V3& vector) {
    V3* result  = new V3(
        (*this)[Dim::X] + vector[Dim::X],
        (*this)[Dim::Y] + vector[Dim::Y],
        (*this)[Dim::Z] + vector[Dim::Z]
    );
    return *result;
}

inline void V3::operator+=(V3& vector) {
    (*this)[Dim::X] += vector[Dim::X];
    (*this)[Dim::Y] += vector[Dim::Y];
    (*this)[Dim::Z] += vector[Dim::Z];
}

inline V3& V3::operator-(V3& vector) {
    V3* result  = new V3(
        (*this)[Dim::X] - vector[Dim::X],
        (*this)[Dim::Y] - vector[Dim::Y],
        (*this)[Dim::Z] - vector[Dim::Z]
    );
    return *result;
}

inline void V3::operator-=(V3& vector) {
    (*this)[Dim::X] -= vector[Dim::X];
    (*this)[Dim::Y] -= vector[Dim::Y];
    (*this)[Dim::Z] -= vector[Dim::Z];
}

void V3::rotate(V3& axis1, V3& axis2, float alpha) {
    V3 axis = axis2 - axis1;
    V3 diff = (*this) - axis1;

    float proj_magnitude = (axis * diff) / (axis * axis);
    V3 offset = diff - axis * proj_magnitude;
    (*this) -= offset;

    // reflect this about X axis
    // , then Z, vector normally, save for matrix time.


    (*this) += offset;
}

void V3::rotate(V3 &axis, float alpha) {

}

#endif