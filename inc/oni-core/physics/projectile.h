#pragma once

#include <oni-core/common/typedefs.h>
#include <oni-core/component/geometry.h>

class b2World;

namespace oni {
    namespace math {
        struct vec2;
        struct vec3;
    }
    namespace entities {
        class EntityManager;

        class ClientDataManager;
    }

    namespace physics {
        class Projectile {
        public:
            Projectile(b2World *);

            ~Projectile();

            void tick(entities::EntityManager &, entities::ClientDataManager &, common::real64 tickTime);

            void destroyBullet(entities::EntityManager&, common::EntityID);

        private:
            common::EntityID createBullet(entities::EntityManager &, const component::Placement &, common::real32);

        private:
            b2World *mPhysicsWorld{};
        };
    }
}