#include <iostream>
#include <graphics/batch-renderer2d.h>
#include <graphics/window.h>
#include <chrono>
#include <graphics/tilelayer.h>
#include <physics/movement.h>

int main() {
    using namespace oni;
    using namespace graphics;
    using namespace math;
    using namespace std;
    using namespace buffers;
    using namespace components;

    int width = 1600;
    int height = 900;

    entities::World world;
    physics::Movement movement;

    // NOTE: any call to GLEW functions will fail with Segfault if GLEW is uninitialized (initialization happens in Window).
    Window window("Oni Demo", width, height);

    auto drawLayerShader = std::make_unique<Shader>("shaders/basic.vert", "shaders/basic.frag");
    auto backGroundLayerShader = std::make_unique<Shader>("shaders/basic.vert", "shaders/basic.frag");

    auto lightLayerShader = std::make_unique<Shader>("shaders/basic.vert", "shaders/spotlight.frag");

//    auto carLayerShader = std::make_unique<Shader>("shaders/basic.vert", "shaders/basic.frag");

    auto drawLayer = std::make_unique<TileLayer>(std::move(drawLayerShader), 10000);
    auto spriteLayer = std::make_unique<TileLayer>(std::move(backGroundLayerShader), 500000);
/*    auto lightLayer = std::make_unique<TileLayer>(std::move(lightLayerShader), 10);
    lightLayer->add(make_unique<Sprite>(vec4(1, 1, 1, 0), vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 9.0f, 0.0f),
                                        vec3(16.0f, 9.0f, 0.0f), vec3(16.0f, 0.0f, 0.0f)));*/

    srand(static_cast<unsigned int>(time(nullptr)));

    float yStep = 0.06f;
    float xStep = 0.04f;

    float xStart = 0.5f;
    float xEnd = 15.5f;

    float yStart = 0.5f;
    float yEnd = 8.5f;

    auto batchSize = static_cast<unsigned long>(((yEnd - yStart + 1) / yStep) * ((xEnd - xStart + 1) / xStep));
    world.reserveEntity(batchSize);

    for (float y = yStart; y < yEnd; y += yStep) {
        for (float x = xStart; x < xEnd; x += xStep) {
            auto sprite = world.createEntity(components::Mask().set(components::POSITION).set(components::APPEARANCE));
            world.setPosition(sprite, Position(vec3(x, y, 0.0f), vec3(x, y + yStep, 0.0f),
                                               vec3(x + xStep, y + yStep, 0.0f),
                                               vec3(x + xStep, y, 0.0f)));
            world.setAppearance(sprite, Appearance(math::vec4(rand() % 1000 / 1000.0f, 0, 1, 1)));
        }
    }

//    auto carLayer = std::make_unique<TileLayer>(std::move(carLayerShader), 2);
    auto car = world.createEntity(
            components::Mask().set(components::POSITION).set(components::APPEARANCE).set(components::DYNAMIC));
    world.setPosition(car, Position(vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 3.0f, 0.0f),
                                    vec3(1.0f, 3.0f, 0.0f), vec3(1.0f, 0.0f, 0.0f)));
    world.setAppearance(car, Appearance(vec4(0.5f, 0.5f, 0.0f, 1.0f)));


    float frameTime = 0.0f;

    float totalFPS = 0.0f;
    int secondsPassed = 0;

    while (!window.closed()) {
        auto start = std::chrono::steady_clock::now();

        window.clear();

        const auto mouseX = window.getCursorX();
        const auto mouseY = window.getCursorY();

        width = window.getWidth();
        height = window.getHeight();

        auto keyPressed = window.getKeyPressed();
        if (window.getMouseButton() != GLFW_KEY_UNKNOWN) {
            auto _x = ((const float) mouseX) * 16.0f / width;
            auto _y = 9.0f - ((const float) mouseY) * 9.0f / height;
            auto sprite = world.createEntity(components::Mask().set(components::POSITION).set(components::APPEARANCE));
            world.setPosition(sprite, Position(vec3(_x, _y, 0.0f),
                                               vec3(_x, _y + 0.07f, 0.0f),
                                               vec3(_x + 0.05f, _y + 0.07f, 0.0f),
                                               vec3(_x + 0.05f, _y, 0.0f)));
            world.setAppearance(sprite, Appearance(vec4(rand() % 1000 / 1000.0f, 0, 0, 1)));
        }


/*        const auto &lightShader = lightLayer->getShader();
        lightShader->enable();
        lightShader->setUniform2f("light_pos", vec2(static_cast<float>(mouseX * 16.0f / width),
                                                    static_cast<float>(9.0f - mouseY * 9.0f / height)));
        lightShader->disable();

        lightLayer->render();*/
        movement.update(world, window);
        spriteLayer->render(world);
//        drawLayer->render();
//        carLayer->update(keyPressed);
//        carLayer->render();

        window.update();

        auto end = std::chrono::steady_clock::now();
        std::chrono::duration<float> diff = end - start;
        frameTime += diff.count();
        if (frameTime > 1.0f) {
            auto fps = 1.0f / diff.count();
            totalFPS += fps;
            secondsPassed++;
            printl(fps);
            frameTime = 0;
        }

    }
    printl("Average FPS: ");
    printl(totalFPS / secondsPassed);

    return 0;
}