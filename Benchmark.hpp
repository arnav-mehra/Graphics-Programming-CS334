#include <chrono>

using namespace std;

void benchmark() {
    V3 vec1 = V3(2, -1, 0);
    V3 vec2 = V3(-1, 2, -1);
    V3 vec3 = V3(0, -1, 2);
    M33 m1 = M33(vec1, vec2, vec3);
    M33 m2 = M33(vec3, vec2, vec1);

    using chrono::high_resolution_clock;
    using chrono::duration_cast;
    using chrono::duration;
    using chrono::milliseconds;
    auto t1 = high_resolution_clock::now();
    for (int i = 0; i < 1000000; i++) {
        M33 m3 = m1.inverse();
        if (i == 0) cout << m3;
    }
    auto t2 = high_resolution_clock::now();
    std::cout << ((t2 - t1).count() / 10.0e8) << " seconds\n";
}