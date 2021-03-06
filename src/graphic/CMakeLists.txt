subdirs(${oni_SOURCE_DIR}/src/graphic/buffer)

add_library(oni-core-graphic
        oni-graphic-debug-draw-box2d.cpp
        oni-graphic-window.cpp
        oni-graphic-shader.cpp
        oni-graphic-font-manager.cpp
        oni-graphic-texture-manager.cpp
        oni-graphic-scene-manager.cpp
        oni-graphic-renderer.cpp
        oni-graphic-renderer-ogl.cpp
        oni-graphic-renderer-ogl-tessellation.cpp
        oni-graphic-renderer-ogl-strip.cpp
        oni-graphic-renderer-ogl-quad.cpp
        oni-graphic-system-grow-over-time.cpp
        oni-graphic-system-particle-emitter.cpp
        oni-graphic-system-material-transition.cpp)

target_compile_features(oni-core-graphic
        PUBLIC
        cxx_std_17
        )

target_compile_definitions(oni-core-graphic
        PRIVATE
        GLEW_NO_GLU
        )

target_include_directories(oni-core-graphic
        PUBLIC
        $<BUILD_INTERFACE:${oni_SOURCE_DIR}/inc>
        $<BUILD_INTERFACE:${oni_SOURCE_DIR}/lib/box2d>
        PRIVATE SYSTEM
        $<BUILD_INTERFACE:${oni_SOURCE_DIR}/lib/freetype-gl/include>
        $<BUILD_INTERFACE:${oni_SOURCE_DIR}/lib/freeimage/Source>
        $<BUILD_INTERFACE:${oni_SOURCE_DIR}/lib/freetype2/include>
        $<BUILD_INTERFACE:${oni_SOURCE_DIR}/lib/anttweakbar/include>
        $<BUILD_INTERFACE:${oni_SOURCE_DIR}/lib/glew/include>
        $<BUILD_INTERFACE:${oni_SOURCE_DIR}/lib/glfw/include>
        )

# TODO: tweak bar should not be an engine dependency
find_library(ANT_TWEAK_BAR AntTweakBar PATHS ${oni_SOURCE_DIR}/lib/anttweakbar/lib NO_DEFAULT_PATH)
find_library(FREETYPE_GL freetype-gl PATHS ${oni_SOURCE_DIR}/lib/freetype-gl/bin NO_DEFAULT_PATH)
find_library(FREETYPE_LIBRARY freetype PATHS ${oni_SOURCE_DIR}/lib/freetype2/build-cmake NO_DEFAULT_PATH)
find_library(FREEIMAGE_LIBRARY freeimage PATHS ${oni_SOURCE_DIR}/lib/freeimage NO_DEFAULT_PATH)
find_library(GLFW_LIBRARY glfw3 PATHS ${oni_SOURCE_DIR}/lib/glfw/build/src NO_DEFAULT_PATH)
find_library(GLEW_LIBRARY GLEW PATHS ${oni_SOURCE_DIR}/lib/glew/lib NO_DEFAULT_PATH)
find_library(BOX2D_LIBRARY Box2D PATHS ${oni_SOURCE_DIR}/lib/box2d/Build/bin/x86_64/Debug NO_DEFAULT_PATH)

set(OpenGL_GL_PREFERENCE "GLVND")
find_package(OpenGL REQUIRED)
find_package(X11 REQUIRED)
find_package(Threads REQUIRED)

target_link_libraries(oni-core-graphic
        PRIVATE
        ${GLFW_LIBRARY}
        ${GLEW_LIBRARY}
        # Not sure why this is needed, but bunch of GLFW linking fails without it
        ${CMAKE_DL_LIBS}
        ${X11_LIBRARIES}
        ${FREEIMAGE_LIBRARY}
        ${FREETYPE_GL}
        # NOTE: FreeType must be linked after freetype-gl because fucking g++
        ${FREETYPE_LIBRARY}
        ${OPENGL_LIBRARY}
        ${ANT_TWEAK_BAR}
        ${BOX2D_LIBRARY}
        # NOTE: For some reason when building glfw from source these are not linked into the binary...
        Xinerama Xrandr Xcursor Xxf86vm
        oni-core-buffer
        oni-core-math
        oni-core-utils
        oni-core-entities
        # Fixes: /lib/x86_64-linux-gnu/libpthread.so.0: error adding symbols: DSO missing from command line
        ${CMAKE_THREAD_LIBS_INIT}
        )

add_dependencies(oni-core-graphic
        oni-core-buffer
        oni-core-math
        oni-core-utils
        )