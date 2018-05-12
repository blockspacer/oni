#pragma once

#include <oni-core/utils/timer.h>
#include <oni-core/common/typedefs.h>

namespace oni {
    namespace game {
        class Game {
        public:
            Game();

            virtual ~Game();

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

        private:
            virtual void tick() final;

            virtual void render() final;

            virtual void display() final;

            virtual void _tick(const common::real32 tickTime) = 0;

            virtual void _render() = 0;

            virtual void _display() = 0;

            virtual void showFPS(common::uint16 fps) = 0;

            virtual void showTPS(common::uint16 tps) = 0;

            /**
             * Accumulated time in ms over 1 second spent sleeping due to excess.
             * @param fet
             */
            virtual void showFET(common::int16 fet) = 0;

        protected:
            utils::HighResolutionTimer mRunTimerA{};
            utils::HighResolutionTimer mRunTimerB{};
            utils::HighResolutionTimer mFrameTimer{};
            utils::HighResolutionTimer mTickTimer{};

            oni::common::real64 mRunLagAccumulator{0.0f};
            oni::common::real64 mRunLag{0.0f};
            oni::common::real64 mFrameLag{0.0f};
            oni::common::real64 mTickLag{0.0f};
            oni::common::real64 mFrameExcessTime{0.0f};
            common::uint16 mRunCounter{0};
            common::uint16 mTickCounter{0};
            common::uint16 mFrameCounter{0};

            // 60Hz
            const common::real32 mTickMS{1 / 60.0f};
            // 30Hz
            // const common::real32 mMinTickMS{1 / 30.0f};
        };
    }
}
