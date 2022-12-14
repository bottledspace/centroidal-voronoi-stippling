#include <iostream>
#include <chrono>

//
//  A simple timer for benchmarking which prints out elapsed time from
//  construction upon leaving scope.
//
struct ScopedTimer {
    using hrc = std::chrono::high_resolution_clock;

    auto diff() const { return hrc::now() - m_start; }


    ScopedTimer(std::string &&name)
        : m_name{std::move(name)},
          m_start{hrc::now()}
    {}
    ~ScopedTimer() {
        std::cout << m_name << " finished in "
            << std::chrono::duration<double>(diff()).count()
            << "s." << std::endl;
    }
private:
    const std::string m_name;
    hrc::time_point m_start;
};
