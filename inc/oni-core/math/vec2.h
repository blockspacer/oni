#pragma once

#include <iostream>

#include <oni-core/common/typedefs.h>

namespace oni {
    namespace math {
        struct vec2 {
            oni::common::real32 x{0.0f};
            oni::common::real32 y{0.0f};

            vec2 &
            add(const vec2 &other);

            vec2 &
            subtract(const vec2 &other);

            vec2 &
            multiply(const vec2 &other);

            vec2 &
            divide(const vec2 &other);

            vec2 &
            divide(common::real32);

            friend vec2
            operator+(const vec2 &left,
                      const vec2 &right);

            friend vec2
            operator-(const vec2 &left,
                      const vec2 &right);

            friend vec2
            operator*(const vec2 &left,
                      const vec2 &right);

            friend vec2
            operator/(const vec2 &left,
                      const vec2 &right);

            friend vec2
            operator/(const vec2 &left,
                      common::real32 right);

            vec2 &
            operator+=(const vec2 &other);

            vec2 &
            operator-=(const vec2 &other);

            vec2 &
            operator*=(const vec2 &other);

            vec2 &
            operator/=(const vec2 &other);

            bool
            operator==(const vec2 &other);

            bool
            operator!=(const vec2 &other);

            common::real32
            len() const;

            friend std::ostream &
            operator<<(std::ostream &stream,
                       const vec2 &vector);

        };
    }
}
