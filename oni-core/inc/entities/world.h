#pragma once

#include <vector>
#include <bitset>
#include <limits>
#include <stack>

#include <components/render-components.h>
#include <components/component.h>
#include <entities/entity.h>

namespace oni {
    namespace entities {
        class World {
        private:
            /**
             * It can reuse free slots created by destroyEntity().
             * If there is no free slot and all the initial slots, marked by World(initialEntityCount), are taken
             * When adding batch of entities, first reserve the number of entities needed by calling
             * World::reserveEntity(BATCH_SIZE);
             *
             * This function guarantees the returned ID is a valid memory location.
             *
             * @return entity ID.
             */
            auto _createEntity();

        public:
            /**
             * Struct of lists that houses all the objects in a game based on Data Oriented Design that
             * keeps data in a contiguous stack-based memory. This is in contrast to the usual list of struct.
             *
             */
            World() = default;

            void reserveEntity(unsigned long count);

            void destroyEntity(unsigned long entity);

            unsigned long createEntity(components::Mask mask);

            void setEntityPos(unsigned long entity, components::Position position);

            void setEntityAppearance(unsigned long entity, components::Appearance appearance);

            const components::EntityMask &getEntities() const;

            const components::Mask &getEntity(unsigned long entity) const;

            const components::Position &getEntityPos(unsigned long entity) const;

            const components::Appearance &getEntityAppearance(unsigned long entity) const;

        private:
            std::stack<unsigned long> m_FreeEntitySlots;
            /**
             * For each entity, define a bitset showing
             * which components are enabled. For example, a simple static
             * game object could be defined by (Components:SPRITE | Components:NAME).
             */
            components::EntityMask m_Entities;

            std::vector<components::Position> m_Positions;
            std::vector<components::Appearance> m_Appearances;

        };
    }
}
