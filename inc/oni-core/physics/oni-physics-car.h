#pragma once

#include <cmath>

#include <oni-core/common/oni-common-typedef.h>

/**
 * The idea is from https://github.com/sina-/carphysics2d
 */

namespace oni {
    namespace component {
        class Car;

        class CarConfig;
    }

    namespace io {
        struct CarInput;
    }

    namespace physics {

        void
        tickCar(component::Car &car,
                const component::CarConfig &config,
                const io::CarInput &inputs,
                common::r64 dt);


        common::CarSimDouble
        applySmoothSteer(const component::Car &car,
                         common::CarSimDouble steerInput,
                         common::r64 dt);

        common::CarSimDouble
        applySafeSteer(const component::Car &car,
                       common::CarSimDouble steerInput);

    }
}