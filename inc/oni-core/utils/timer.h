#pragma once

#include <cstdint>
#include <chrono>

#include <oni-core/common/typedefs.h>

namespace oni {
    namespace utils {
        class Timer {
        public:
            Timer() : mStartTime(now()) {}

            void
            restart() {
                mStartTime = now();
            }

            static std::chrono::steady_clock::time_point
            now() {
                return std::chrono::steady_clock::now();
            }

            template<class Duration>
            Duration
            elapsed() const {
                return std::chrono::duration_cast<Duration>(now() - mStartTime);
            }

            // Return elapsed time in seconds
            oni::common::real64
            elapsedInSeconds() const { return (now() - mStartTime).count() * 1e-9; }

            common::uint64
            elapsedInNanoseconds() const {
                return static_cast<common::uint64>((now() - mStartTime).count());
            }

        private:
            std::chrono::steady_clock::time_point mStartTime{};
        };
    }
}
