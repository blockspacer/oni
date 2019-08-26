#include <oni-core/entities/oni-entities-manager.h>

#include <Box2D/Dynamics/b2Body.h>
#include <Box2D/Dynamics/b2Fixture.h>
#include <Box2D/Collision/Shapes/b2PolygonShape.h>
#include <Box2D/Dynamics/b2World.h>

#include <oni-core/component/oni-component-physics.h>
#include <oni-core/component/oni-component-audio.h>
#include <oni-core/component/oni-component-gameplay.h>
#include <oni-core/graphic/oni-graphic-texture-manager.h>
#include <oni-core/math/oni-math-rand.h>


namespace oni {
    EntityManager::EntityManager(SimMode sMode,
                                 b2World *physicsWorld) : mSimMode(sMode),
                                                          mPhysicsWorld(physicsWorld) {
        mRegistry = std::make_unique<entt::basic_registry<u32 >>();
        mLoader = std::make_unique<entt::basic_continuous_loader<EntityID>>(*mRegistry);
        mDispatcher = std::make_unique<entt::dispatcher>();
        mRand = std::make_unique<Rand>(0, 0);

        switch (sMode) {
            case SimMode::CLIENT: {
                mEntityOperationPolicy = EntityOperationPolicy::client();
                break;
            }
            case SimMode::SERVER: {
                mEntityOperationPolicy = EntityOperationPolicy::server();
                break;
            }
            case SimMode::CLIENT_SIDE_SERVER: {
                mEntityOperationPolicy = EntityOperationPolicy::clientServer();
                break;
            }
            default: {
                assert(false);
                break;
            }
        }
    }

    EntityManager::~EntityManager() = default;

    void
    EntityManager::clearDeletedEntitiesList() {
        mDeletedEntities.clear();
    }

    void
    EntityManager::attach(EntityID parent,
                          EntityID child,
                          EntityType parentType,
                          EntityType childType) {
        auto &attachment = mRegistry->get<EntityAttachment>(parent);
        attachment.entities.emplace_back(child);
        attachment.entityTypes.emplace_back(childType);

        auto &attachee = mRegistry->get<EntityAttachee>(child);
        attachee.entityID = parent;
        attachee.entityType = parentType;
    }

    size_t
    EntityManager::size() {
        auto result = mRegistry->size();
        return result;
    }

    size
    EntityManager::alive() {
        auto result = mRegistry->alive();
        return result;
    }

    EntityID
    EntityManager::map(EntityID entityID) {
        auto result = mLoader->map(entityID);
        if (result == entt::null) {
            return 0;
        }
        return result;
    }

    const std::vector<DeletedEntity> &
    EntityManager::getDeletedEntities() const {
        return mDeletedEntities;
    }

    void
    EntityManager::markForNetSync(EntityID entity) {
        if (mEntityOperationPolicy.track) {
            assert(mSimMode == SimMode::SERVER);
            accommodate<Tag_NetworkSyncComponent>(entity);
        }
    }

    void
    EntityManager::dispatchEvents() {
        mDispatcher->update();
    }

    EntityID
    EntityManager::create() {
        auto result = mRegistry->create();
        return result;
    }

    void
    EntityManager::destroy(EntityID entityID) {
        mRegistry->destroy(entityID);
    }

    void
    EntityManager::destroyAndTrack(EntityID id) {
        auto type = mRegistry->get<EntityType>(id);
        mDeletedEntities.push_back({id, type});
        mRegistry->destroy(id);
    }

    bool
    EntityManager::valid(EntityID entityID) {
        return mRegistry->valid(entityID);
    }

    void
    EntityManager::markForDeletion(EntityID id) {
        mEntitiesToDelete.push_back(id);
    }

    void
    EntityManager::flushDeletions() {
        for (auto &&i : mEntitiesToDelete) {
            deleteEntity(i);
        }

        mEntitiesToDelete.clear();
    }

    EntityID
    EntityManager::createEntity(EntityType type) {
        assert(mSimMode == SimMode::SERVER || mSimMode == SimMode::CLIENT);
        auto id = mRegistry->create();
        if (mEntityOperationPolicy.track) {
            assert(mSimMode == SimMode::SERVER);
            assignTag<Tag_NetworkSyncEntity>(id);
        }
        createComponent<EntityType>(id, type);
        return id;
    }

    void
    EntityManager::deleteEntity(EntityID id) {
        deleteEntity(id, mEntityOperationPolicy);
    }

    void
    EntityManager::deleteEntity(EntityID id,
                                const EntityOperationPolicy &policy) {
        if (policy.safe && !valid(id)) {
            return;
        }

        if (mRegistry->has<EntityAttachment>(id)) {
            for (auto &&childID: mRegistry->get<EntityAttachment>(id).entities) {
                deleteEntity(childID, policy);
            }
        }

        if (mRegistry->has<PhysicalProperties>(id)) {
            removePhysicalBody(id);
        }

        if (mRegistry->has<Texture>(id)) {
            // TODO: Clean up the texture in video-memory or tag it stale or something if I end up having a asset manager
        }

        if (mRegistry->has<Sound_Tag>(id)) {
            // TODO: Same as texture, audio system needs to free the resource or let its resource manager at least know about this
        }

        if (policy.track) {
            destroyAndTrack(id);
        } else {
            destroy(id);
        }
    }

    void
    EntityManager::printEntityType(EntityID id) {
        const auto &t = mRegistry->get<EntityType>(id);
        auto name = std::string();
        switch (t) {
            case EntityType::BACKGROUND: {
                name = "background";
                break;
            }
            case EntityType::ROAD: {
                name = "road";
                break;
            }
            case EntityType::WALL: {
                name = "wall";
                break;
            }
            case EntityType::RACE_CAR: {
                name = "race_car";
                break;
            }
            case EntityType::VEHICLE: {
                name = "vehicle";
                break;
            }
            case EntityType::VEHICLE_GUN: {
                name = "vehicle_gun";
                break;
            }
            case EntityType::VEHICLE_TIRE_REAR: {
                name = "vehicle_tire_rear";
                break;
            }
            case EntityType::VEHICLE_TIRE_FRONT: {
                name = "vehicle_tire_front";
                break;
            }
            case EntityType::UI: {
                name = "ui";
                break;
            }
            case EntityType::CANVAS: {
                name = "canvas";
                break;
            }
            case EntityType::SIMPLE_SPRITE: {
                name = "simple_sprite";
                break;
            }
            case EntityType::SIMPLE_PARTICLE: {
                name = "simple_particle";
                break;
            }
            case EntityType::SIMPLE_BLAST_PARTICLE: {
                name = "simple_blast_particle";
                break;
            }
            case EntityType::SIMPLE_BLAST_ANIMATION: {
                name = "simple_blast_animation";
                break;
            }
            case EntityType::SIMPLE_ROCKET: {
                name = "simple_rocket";
                break;
            }
            case EntityType::TRAIL_PARTICLE: {
                name = "trail_particle";
                break;
            }
            case EntityType::TEXT: {
                name = "text";
                break;
            }
            case EntityType::WORLD_CHUNK: {
                name = "world_chunk";
                break;
            }
            case EntityType::DEBUG_WORLD_CHUNK: {
                name = "debug_world_chunk";
                break;
            }
            case EntityType::SMOKE_CLOUD: {
                name = "smoke_cloud";
                break;
            }
            case EntityType::COMPLEMENT: {
                name = "complement";
                break;
            }
            case EntityType::UNKNOWN:
            case EntityType::LAST:
            default: {
                assert(false);
                break;
            }
        }
        std::cout << name << '\n';
    }

    void
    EntityManager::createPhysics(EntityID id,
                                 const WorldP3D &pos,
                                 const vec2 &size,
                                 r32 heading) {
        auto &props = mRegistry->get<PhysicalProperties>(id);
        auto shape = b2PolygonShape{};
        shape.SetAsBox(size.x / 2.0f, size.y / 2.0f);

        // NOTE: This is non-owning pointer. physicsWorld owns it.
        b2Body *body{};

        b2BodyDef bodyDef;
        bodyDef.bullet = props.highPrecision;
        bodyDef.angle = heading;
        bodyDef.linearDamping = props.linearDamping;
        bodyDef.angularDamping = props.angularDamping;
        bodyDef.gravityScale = props.gravityScale;

        b2FixtureDef fixtureDef;
        // NOTE: Box2D will create a copy of the shape, so it is safe to pass a local ref.
        fixtureDef.shape = &shape;
        fixtureDef.density = props.density;
        fixtureDef.friction = props.friction;

        switch (props.bodyType) {
            case BodyType::DYNAMIC : {
                bodyDef.position.x = pos.x;
                bodyDef.position.y = pos.y;
                bodyDef.type = b2_dynamicBody;
                body = mPhysicsWorld->CreateBody(&bodyDef);

                b2FixtureDef collisionSensor;
                collisionSensor.isSensor = true;
                collisionSensor.shape = &shape;
                collisionSensor.density = props.density;
                collisionSensor.friction = props.friction;

                if (!props.collisionWithinCategory) {
                    collisionSensor.filter.groupIndex = -static_cast<i16>(props.physicalCategory);
                }

                if (props.disableCollision) {
                    fixtureDef.isSensor = true;
                } else {
                    body->CreateFixture(&collisionSensor);
                }
                body->CreateFixture(&fixtureDef);
                break;
            }
            case BodyType::STATIC: {
                bodyDef.position.x = pos.x;
                bodyDef.position.y = pos.y;
                bodyDef.type = b2_staticBody;
                body = mPhysicsWorld->CreateBody(&bodyDef);
                body->CreateFixture(&shape, 0.f);
                break;
            }
            case BodyType::KINEMATIC: {
                bodyDef.position.x = pos.x;
                bodyDef.position.y = pos.y;
                bodyDef.type = b2_kinematicBody;
                body = mPhysicsWorld->CreateBody(&bodyDef);
                body->CreateFixture(&fixtureDef);
                break;
            }
            case BodyType::UNKNOWN: {
                assert(false);
                break;
            }
            default: {
                assert(false);
                break;
            }
        }
        mEntityBodyMap[id] = body;
    }

    void
    EntityManager::removePhysicalBody(EntityID id) {
        auto *body = mEntityBodyMap[id];
        assert(body);
        mPhysicsWorld->DestroyBody(body);
    }

    void
    EntityManager::setRandAge(EntityID id,
                              r32 lower,
                              r32 upper) {
        auto &age = mRegistry->get<Age>(id);
        age.maxAge = mRand->next_r32(lower, upper);
    }

    void
    EntityManager::setRandVelocity(EntityID id,
                                   u32 lower,
                                   u32 upper) {
        auto &velocity = mRegistry->get<Velocity>(id);
        velocity.current = mRand->next(lower, upper);
        velocity.max = velocity.current;
    }

    r32
    EntityManager::setRandHeading(EntityID id) {
        return setRandHeading(id, 0.f, FULL_CIRCLE_IN_RAD);
    }

    r32
    EntityManager::setRandHeading(EntityID id,
                                  r32 lower,
                                  r32 upper) {
        auto &heading = mRegistry->get<Heading>(id);
        heading.value = mRand->next_r32(lower, upper);
        return heading.value;
    }

    void
    EntityManager::setWorldP3D(EntityID id,
                               r32 x,
                               r32 y,
                               r32 z) {
        auto &pos = mRegistry->get<WorldP3D>(id);
        pos.x = x;
        pos.y = y;
        pos.z = z;
    }

    void
    EntityManager::setScale(EntityID id,
                            r32 x,
                            r32 y) {
        auto &scale = mRegistry->get<Scale>(id);
        scale.x = x;
        scale.y = y;
    }

    void
    EntityManager::setEntityPreset(EntityID id,
                                   EntityPreset tag) {

        auto &ep = mRegistry->get<EntityPreset>(id);
        ep = tag;
    }

    void
    EntityManager::setHeading(EntityID id,
                              r32 heading) {
        auto &h = mRegistry->get<Heading>(id);
        h.value = heading;
    }

    void
    EntityManager::setText(EntityID id,
                           std::string_view content) {
        auto &text = mRegistry->get<Text>(id);
        text.textContent = content;

    }

    void
    EntityManager::setColor(EntityID id,
                            r32 red,
                            r32 green,
                            r32 blue,
                            r32 alpha) {
        auto &color = mRegistry->get<Color>(id);
        color.set_r(red);
        color.set_g(green);
        color.set_b(blue);
        color.set_a(alpha);
    }

    b2Body *
    EntityManager::getEntityBody(EntityID id) {
        auto result = mEntityBodyMap[id];
        assert(result);
        return result;
    }

    SimMode
    EntityManager::getSimMode() {
        return mSimMode;
    }
}