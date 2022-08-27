#ifndef M33_CPP
#define M33_CPP

#include "M33.hpp"
using namespace std;

inline void swap(float &x, float &y) {
    float temp = x;
    x = y;
    y = temp;
}

istream& operator>>(istream& in, M33& matrix) {
    cout << "Please enter matrix row vectors:\n";
    cout << "1: ";
    in >> matrix[0];
    cout << "2: ";
    in >> matrix[1];
    cout << "3: ";
    in >> matrix[2];
    return in;
}

ostream& operator<<(ostream& out, M33& matrix) {
    out << "Matrix (\n";
    out << "  " << matrix[Dim::X];
    out << "  " << matrix[Dim::Y];
    out << "  " << matrix[Dim::Z];
    out << ")\n";
    return out;
}

inline V3& M33::operator[](Dim dim) {
    return this->matrix[dim];
} 

inline V3& M33::operator[](int i) {
    return this->matrix[i];
}        

M33& M33::operator*(M33& matrix) {
    M33* result = new M33();
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            // transposed dot product
            (*result)[j][i] =
                (*this)[j][Dim::X] * matrix[Dim::X][i]
                + (*this)[j][Dim::Y] * matrix[Dim::Y][i]
                + (*this)[j][Dim::Z] * matrix[Dim::Z][i];
        }
    }
    return *result;
}

V3& M33::operator*(V3& vector) {
    V3* result = new V3();
    (*result)[Dim::X] = (*this)[Dim::X] * vector;
    (*result)[Dim::Y] = (*this)[Dim::Y] * vector;
    (*result)[Dim::Z] = (*this)[Dim::Z] * vector;
    return *result;
}

void M33::operator*=(float scalar) {
    (*this)[Dim::X] *= scalar;
    (*this)[Dim::Y] *= scalar;
    (*this)[Dim::Z] *= scalar;
}

M33& M33::operator*(float scalar) {
    M33* result = new M33(
        (*this)[Dim::X] * scalar,
        (*this)[Dim::Y] * scalar,
        (*this)[Dim::Z] * scalar
    );
    return *result;
}

void M33::transpose() {
    swap((*this)[0][1], (*this)[1][0]);
    swap((*this)[0][2], (*this)[2][0]);
    swap((*this)[2][1], (*this)[1][2]);
}

M33& M33::inverse_iter(int max_iter) {
    M33* inverse = new M33();
    V3 v1 = V3(1, 0, 0);
    (*inverse)[Dim::X] = conjugate_grad(v1, max_iter);
    V3 v2 = V3(0, 1, 0);
    (*inverse)[Dim::Y] = conjugate_grad(v2, max_iter);
    V3 v3 = V3(0, 0, 1);
    (*inverse)[Dim::Z] = conjugate_grad(v3, max_iter);
    inverse->transpose();
    return *inverse;
}

// CS314 time (from 1 of my HW assignments)!
inline V3& M33::conjugate_grad(V3 &b, int maxiter) {
    // A = matrix (this)
    M33 &A = *this;
    // x = our current guess
    V3 *x_ptr = new V3(1.0f, 1.0f, 1.0f);
    V3 &x = *x_ptr;
    // residuals
    V3 r = b - A * x;
    V3 p = r;
    float r_prev_mag_sq = r * r;

    for (int k = 1; k <= maxiter; k++) {
        V3 g = A * p;
        float alpha = r_prev_mag_sq / (p * g);
        x += p * alpha;
        r -= g * alpha;

        float r_curr_mag_sq = r * r;
        if (r_curr_mag_sq < 1e-3) return x;

        float beta = r_curr_mag_sq / r_prev_mag_sq;
        p = r + p * beta;

        r_prev_mag_sq = r_curr_mag_sq;
    }
    return x;
}

M33& M33::inverse() {
    // new 00
    float det1122 =
        ((*this)[1][1] * (*this)[2][2]
        - (*this)[2][1] * (*this)[1][2]);
    // new 10
    float det1022 =
        ((*this)[1][2] * (*this)[2][0]
        - (*this)[2][2] * (*this)[1][0]);
    // new 20
    float det1021 =
        ((*this)[1][0] * (*this)[2][1]
        - (*this)[2][0] * (*this)[1][1]);
    float determinant =
        (*this)[0][0] * det1122
        + (*this)[0][1] * det1022
        + (*this)[0][2] * det1021;
    
    // new 01
    float det0221 =
        ((*this)[0][2] * (*this)[2][1]
        - (*this)[0][1] * (*this)[2][2]);
    // new 11
    float det0022 =
        ((*this)[0][0] * (*this)[2][2]
        - (*this)[0][2] * (*this)[2][0]);
    // new 21
    float det0120 =
        ((*this)[0][1] * (*this)[2][0]
        - (*this)[0][0] * (*this)[2][1]);

    // new 02
    float det0112 =
        ((*this)[0][1] * (*this)[1][2]
        - (*this)[1][1] * (*this)[0][2]);
    // new 12
    float det0210 =
        ((*this)[0][2] * (*this)[1][0]
        - (*this)[0][0] * (*this)[1][2]);
    // new 22
    float det0011 =
        ((*this)[0][0] * (*this)[1][1]
        - (*this)[0][1] * (*this)[1][0]);
    
    V3 *v1 = new V3(det1122, det0221, det0112);
    V3 *v2 = new V3(det1022, det0022, det0210);
    V3 *v3 = new V3(det1021, det0120, det0011);
    M33* result = new M33(*v1, *v2, *v3);

    (*result) *= 1 / determinant;
    return *result;
}

#endif