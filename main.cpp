#include "V3.cpp"
#include "M33.cpp"
#include "Dimension.hpp"
#include "Benchmark.hpp"
using namespace std;

int main() {
    V3 vec1 = V3(2, -1, 0);
    V3 vec2 = V3(-1, 2, -1);
    V3 vec3 = V3(0, -1, 2);
    M33 m1 = M33(vec1, vec2, vec3);
    M33 m2 = M33(vec3, vec2, vec1);

    // benchmark();
    cin >> m1;
    cout << m1;
}