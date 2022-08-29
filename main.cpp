#include "Dimension.hpp"
#include "V3.hpp"
#include "M33.hpp"
#include "_V3.hpp"
#include "_M33.hpp"
#include "Benchmark.hpp"
#include <iostream>

using namespace std;

int main() {
    V3 vec1 = V3(3, 2, 1);
    V3 vec2 = V3(1, 1, 1);
    V3 vec3 = V3(5, 6, 2);
    M33 m1 = M33(vec1, vec2, vec3);
    M33 m2 = M33(vec3, vec2, vec1);

    // benchmark();
    cout << vec1;
    for (int i = 0; i < 9; i++) {
        vec1.rotate(vec3, vec2, 6.28/9);
        cout << vec1;
    }
}