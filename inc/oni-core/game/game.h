#pragma once

#include <oni-core/utils/timer.h>

namespace oni {
    namespace game {
        class Game {
        public:
            Game();

            virtual ~Game() = default;

            /**
             * Main loop should run this function as fast as possible.
             */
            virtual void run();

            /**
             * Condition under which the game should terminate. Usually happens if user
             * exists the game.
             * @return
             */
            virtual bool shouldTerminate() = 0;

        protected:
            virtual void tick(const int keyPressed) final;

            virtual void render() final;

            virtual void _tick(const float tickTime, const int keyPressed) = 0;

            virtual void _render() = 0;

            virtual void showFPS(unsigned short fps) = 0;

            virtual void showTPS(unsigned short tps) = 0;

            virtual int getKey() = 0;

        protected:
            utils::HighResolutionTimer mRunTimer;
            utils::HighResolutionTimer mFrameTimer;
            utils::HighResolutionTimer mTickTimer;

            double mRunLag;
            double mFrameLag;
            double mTickLag;
            unsigned short mRunCounter;
            unsigned short mTickCounter = 0;
            unsigned short mFrameCounter = 0;

            // 60Hz
            const float mTickMS = 1 / 60.0f;
            // 30Hz
            const float mMinTickMS = 1 / 30.0f;

            const float ep = 0.00001f;
        };
    }
}
