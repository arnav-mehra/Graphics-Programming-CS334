#include "Dimension.hpp"
#include "V3.cpp"
#include <cmath>
using namespace std;

class M33 {
    private:

        V3 matrix[3];
    
    public:

        M33() {}

        // identity constructor
        M33(int i) {
            (*this)[Dim::X] = V3(1, 0, 0);
            (*this)[Dim::Y] = V3(0, 1, 0);
            (*this)[Dim::Z] = V3(0, 0, 1);
        }

        M33(V3 &v1, V3 &v2, V3 &v3) {
            (*this)[Dim::X] = v1;
            (*this)[Dim::Y] = v2;
            (*this)[Dim::Z] = v3;
        }

        // rotation constructor
        M33(Dim dim, float alpha) {
            float sin_theta = sin(alpha);
            float cos_theta = cos(alpha);
            switch (dim) {
                case Dim::X:
                    (*this)[Dim::X] = V3(1, 0, 0);
                    (*this)[Dim::Y] = V3(0, cos_theta, -sin_theta);
                    (*this)[Dim::Z] = V3(0, sin_theta, cos_theta);
                    break;
                case Dim::Y:
                    (*this)[Dim::X] = V3(cos_theta, 0, sin_theta);
                    (*this)[Dim::Y] = V3(0, 1, 0);
                    (*this)[Dim::Z] = V3(-sin_theta, 0, cos_theta);
                    break;
                case Dim::Z:
                    (*this)[Dim::X] = V3(cos_theta, -sin_theta, 0);
                    (*this)[Dim::Y] = V3(sin_theta, cos_theta, 0);
                    (*this)[Dim::Z] = V3(0, 0, 1);
                    break;
            }
        }      

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