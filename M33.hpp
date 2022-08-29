#pragma once

#include <iostream>
#include <cmath>
#include "Dimension.hpp"
#include "V3.hpp"

using namespace std;

class M33 {
    private:

        V3 matrix[3];
    
    public:

        M33() {}
        // identity constructor
        M33(int i);
        // vector constructor
        M33(V3 &v1, V3 &v2, V3 &v3);
        // rotation constructors
        M33(Dim dim, float alpha);
        M33(Dim dim, float sin_theta, float cos_theta);   

        friend ostream& operator<<(ostream &out, M33 &matrix);
        friend istream& operator>>(istream &in, M33 &matrix);

        // Get and set each vector.
        inline V3& operator[](Dim dim);
        inline V3& operator[](int i);
        
        // Matrix (this) and matrix, vector, and scalar multiplication
        M33& operator*(M33& matrix);
        V3& operator*(V3& vector);
        void operator*=(float scalar);
        M33& operator*(float scalar);

        // Transpose this matrx.
        void transpose();
        
        // Get inverse of matrix.
        M33& inverse();
        M33& inverse_iter(int max_iter);
        inline V3& conjugate_grad(V3 &b, int maxiter);
};