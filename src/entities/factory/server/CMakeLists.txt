add_library(oni-core-entities-factory-server
        oni-entities-factory-server.cpp
        )

target_compile_features(oni-core-entities-factory-server
        PUBLIC
        cxx_std_17
        )

target_include_directories(oni-core-entities-factory-server
        PUBLIC
        $<BUILD_INTERFACE:${oni_SOURCE_DIR}/inc>
#        $<BUILD_INTERFACE:${oni_SOURCE_DIR}/lib/entt/src>
#        $<BUILD_INTERFACE:${oni_SOURCE_DIR}/lib/cereal>
        PRIVATE
        $<BUILD_INTERFACE:${oni_SOURCE_DIR}/lib/box2d>
        )

#find_library(BOX2D_LIBRARY Box2D PATHS ${oni_SOURCE_DIR}/lib/box2d/Build/bin/x86_64/Debug NO_DEFAULT_PATH)

target_link_libraries(oni-core-entities-factory-server
        PUBLIC
        oni-core-entities
#        oni-core-physics
#        PRIVATE
#        ${BOX2D_LIBRARY}
        )
