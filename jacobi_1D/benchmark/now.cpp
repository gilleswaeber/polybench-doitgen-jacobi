#include <chrono>
#include <iostream>

int main() {
    auto now = std::chrono::high_resolution_clock::now();
    auto d = now.time_since_epoch();
    std::cout << "Now is " << std::chrono::duration_cast<std::chrono::nanoseconds>(d).count() << "ns since epoch" << std::endl;
}