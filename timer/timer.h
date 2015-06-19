#include <chrono>

namespace Mouse
{

class Timer
{
public:
    Timer() : begin_(std::chrono::high_resolution_clock::now())
    {
    }

    void reset()
    {
        begin_ = std::chrono::high_resolution_clock::now();
    }

    template<typename Duration = std::chrono::milliseconds>
    int64_t elapsed() const
    {
        return std::chrono::duration_cast<Duration>
            (std::chrono::high_resolution_clock::now() - begin_).count();
    }

    int64_t elapsed_micro() const
    {
        return elapsed<std::chrono::microseconds>();
    }

    int64_t elapsed_seconds() const
    {
        return elapsed<std::chrono::seconds>();
    }

private:
    std::chrono::time_point<std::chrono::high_resolution_clock> begin_;
};

} //namespace Mouse
