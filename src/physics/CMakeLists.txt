add_library(oni-core-physics
        oni-physics.cpp
        oni-physics-car.cpp
        oni-physics-system-time-to-live.cpp
        oni-physics-system-splat-on-rest.cpp
        oni-physics-system-sync-pos.cpp
        oni-physics-system-car.cpp
        oni-physics-system-car-input.cpp
        oni-physics-system-jet-force.cpp
        oni-physics-system-position.cpp
        oni-physics-car-collision.cpp)

target_compile_features(oni-core-physics
        PUBLIC
        cxx_std_17
        )

target_include_directories(oni-core-physics
        PUBLIC
        $<BUILD_INTERFACE:${oni_SOURCE_DIR}/inc>
        PRIVATE SYSTEM
        $<BUILD_INTERFACE:${oni_SOURCE_DIR}/lib/cereal>
        $<BUILD_INTERFACE:${oni_SOURCE_DIR}/lib/box2d>
        $<BUILD_INTERFACE:${oni_SOURCE_DIR}/lib/glfw/include>
        )

find_library(BOX2D_LIBRARY Box2D PATHS ${oni_SOURCE_DIR}/lib/box2d/Build/bin/x86_64/Debug NO_DEFAULT_PATH)

target_link_libraries(oni-core-physics
        PRIVATE
        ${BOX2D_LIBRARY}
        oni-core-entities
        oni-core-math
        oni-core-io
        )

add_dependencies(oni-core-physics
        oni-core-io
        oni-core-math
        )
