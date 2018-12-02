#pragma once

#include <oni-core/common/typedefs.h>
#include <oni-core/math/vec2.h>
#include <oni-core/math/vec3.h>
#include <oni-core/math/vec4.h>
#include <oni-core/components/physics.h>
#include <oni-core/common/typedefs-graphics.h>
#include <oni-core/entities/entity-manager.h>

class b2World;

namespace oni {
    namespace entities {
        class EntityManager;
    }

    namespace components {
        struct Texture;
        struct CarConfig;
    }

    namespace graphics {
        class FontManager;

        class SceneManager;
    }

    namespace entities {

        common::EntityID createEntity(EntityManager &manager);

        void destroyEntity(EntityManager &manager, common::EntityID entityID);

        void assignShapeLocal(EntityManager &manager, common::EntityID entityID, const math::vec2 &size);

        void removeShape(EntityManager &manager, common::EntityID entityID);

        void assignShapeWorld(EntityManager &manager,
                              common::EntityID entityID,
                              const math::vec2 &size,
                              const math::vec3 &worldPos);

        void assignPlacement(EntityManager &manager,
                             common::EntityID entityID,
                             const math::vec3 &worldPos,
                             const math::vec3 &scale,
                             common::real32 heading);

        void removePlacement(EntityManager &manager, common::EntityID entityID);

        void assignAppearance(EntityManager &manager, common::EntityID entityID, const math::vec4 &color);

        void removeAppearance(EntityManager &manager, common::EntityID entityID);

        void assignPhysicalProperties(EntityManager &manager,
                                      b2World &physicsWorld,
                                      common::EntityID,
                                      const math::vec3 &worldPos,
                                      const math::vec2 &size,
                                      components::BodyType bodyType,
                                      bool highPrecisionPhysics);

        void removePhysicalProperties(EntityManager &manager, b2World &physicsWorld, common::EntityID entityID);

        void assignTextureToLoad(EntityManager &manager, common::EntityID entity, const std::string &path);

        void assignTextureLoaded(EntityManager &manager,
                                 common::EntityID entity,
                                 const components::Texture &texture);

        void removeTexture(EntityManager &manager, common::EntityID entityID);

        void assignText(EntityManager &manager,
                        graphics::FontManager &fontManager,
                        common::EntityID entity,
                        const std::string &text,
                        const math::vec3 &worldPos);

        void removeText(EntityManager &manager, common::EntityID entityID);

        void assignCar(EntityManager &manager, common::EntityID entityID, const math::vec3 &worldPos,
                       const components::CarConfig &carConfig);

        void removeCar(EntityManager &manager, common::EntityID entityID);

        template<class T>
        void assignTag(EntityManager &manager, common::EntityID entityID) {
            manager.assign<T>(entityID);
        }

        template<class T>
        void removeTag(EntityManager &manager, common::EntityID entityID) {
            manager.remove<T>(entityID);
        }
    }
}
