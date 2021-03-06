#include <oni-core/entities/oni-entities-manager.h>

#include <cmath>

#include <Box2D/Dynamics/b2Body.h>
#include <Box2D/Dynamics/Contacts/b2Contact.h>
#include <Box2D/Dynamics/b2Fixture.h>
#include <Box2D/Collision/Shapes/b2PolygonShape.h>
#include <Box2D/Dynamics/b2World.h>

#include <oni-core/math/oni-math-rand.h>
#include <oni-core/entities/oni-entities-structure.h>
#include <oni-core/entities/oni-entities-factory.h>
#include <oni-core/entities/oni-entities-structure.h>
#include <oni-core/physics/oni-physics.h>


namespace oni {
    EntityManager::EntityManager(SimMode sMode,
                                 Physics *p) : mSimMode(sMode),
                                               mPhysics(p) {
        mRegistry = std::make_unique<entt::basic_registry<u32 >>();
        mLoader = std::make_unique<entt::basic_continuous_loader<EntityID>>(*mRegistry);
        for (auto i = 0; i < NumEventDispatcher; ++i) {
            mDispatcher[i] = std::make_unique<entt::dispatcher>();
        }
        mRand = std::make_unique<Rand>(0, 0);
        mEventRateLimiter = std::make_unique<EventRateLimiter>();

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
    EntityManager::attach(const EntityContext &parent,
                          const EntityContext &child) {
        // TODO: Implicit that this is the case for when client entity is attached to server entity.
        // Maybe I can have two methods, attach and attach as a shadow, and in attach I just assert
        // that we are using the same manager.
        if (parent.mng != child.mng) {
            if (!parent.mng->has<EntityAttachmentShadow>(parent.id)) {
                parent.mng->createComponent<EntityAttachmentShadow>(parent.id);
            }
            auto &attachment = parent.mng->get<EntityAttachmentShadow>(parent.id);
            attachment.entities.emplace_back(child.id);
            attachment.mngs.emplace_back(child.mng);
        } else {
            if (!parent.mng->has<EntityAttachment>(parent.id)) {
                parent.mng->createComponent<EntityAttachment>(parent.id);
            }
            auto &attachment = parent.mng->get<EntityAttachment>(parent.id);
            attachment.entities.emplace_back(child.id);
            attachment.mngs.emplace_back(child.mng);
        }

        if (!child.mng->has<EntityAttachee>(child.id)) {
            child.mng->createComponent<EntityAttachee>(child.id);
        }
        auto &attachee = child.mng->get<EntityAttachee>(child.id);
        attachee.id = parent.id;
        attachee.mng = parent.mng;
    }

    void
    EntityManager::bindLifetime(const EntityContext &parent,
                                const EntityContext &child) {
        if (!parent.mng->has<BindLifetimeParent>(parent.id)) {
            parent.mng->createComponent<BindLifetimeParent>(parent.id);
        }
        if (!child.mng->has<BindLifetimeChild>(child.id)) {
            child.mng->createComponent<BindLifetimeChild>(child.id);
        }

        auto &blp = parent.mng->get<BindLifetimeParent>(parent.id);
        blp.children.emplace_back(child);

        auto &blc = child.mng->get<BindLifetimeChild>(child.id);
        blc.parent = parent;
    }

    void
    EntityManager::fixupAttachments() {
        {
            auto view = createView<EntityAttachment>();
            for (auto &&entity: view) {
                auto &attachment = view.get<EntityAttachment>(entity);
                if (attachment.entities.size() != attachment.mngs.size()) {
                    attachment.mngs.clear();
                    for (auto &&id: attachment.entities) {
                        attachment.mngs.push_back(this);
                    }
                }
            }
        }
        {
            auto view = createView<EntityAttachee>();
            for (auto &&entity: view) {
                auto &attachee = view.get<EntityAttachee>(entity);
                attachee.mng = this;
            }
        }
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
    EntityManager::dispatchEvents(EventDispatcherType type) {
        auto idx = enumCast(type);
        mDispatcher[idx]->update();
    }

    void
    EntityManager::dispatchEventsAndFlush(EventDispatcherType type) {
        dispatchEvents(type);
        flushDeletions();
    }

    void
    EntityManager::destroy(EntityID entityID) {
        mRegistry->destroy(entityID);
    }

    void
    EntityManager::destroyAndTrack(EntityID id) {
        auto name = mRegistry->get<EntityName>(id);
        mDeletedEntities.push_back({id, name});
        mRegistry->destroy(id);
    }

    bool
    EntityManager::valid(EntityID entityID) {
        return mRegistry->valid(entityID);
    }

    void
    EntityManager::markForDeletion(EntityID id) {
        mEntitiesToDelete.emplace(id);
    }

    void
    EntityManager::flushDeletions() {
        for (auto &&i : mEntitiesToDelete) {
            deleteEntity(i);
        }

        mEntitiesToDelete.clear();
    }

    void
    EntityManager::flushDeletions(const EntityOperationPolicy &policy) {
        for (auto &&i : mEntitiesToDelete) {
            deleteEntity(i, policy);
        }

        mEntitiesToDelete.clear();
    }

    EntityID
    EntityManager::createEntity() {
        return mRegistry->create();
    }

    EntityID
    EntityManager::createEntity(const EntityName &name) {
        assert(mSimMode == SimMode::SERVER || mSimMode == SimMode::CLIENT);
        auto id = mRegistry->create();
        if (mEntityOperationPolicy.track) {
            assert(mSimMode == SimMode::SERVER);
            assignTag<Tag_NetworkSyncEntity>(id);
        }
        createComponent<EntityName>(id, name);
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
            for (auto &&child: mRegistry->get<EntityAttachment>(id).entities) {
                deleteEntity(child, policy);
            }
        }

        if (mRegistry->has<BindLifetimeParent>(id)) {
            for (auto &&childContext: mRegistry->get<BindLifetimeParent>(id).children) {
                childContext.mng->deleteEntity(childContext.id);
            }
        }

        if (mRegistry->has<PhysicalProperties>(id)) {
            removePhysicalBody(id);
        }

        if (mRegistry->has<Texture>(id)) {
            // TODO: Clean up the texture in video-memory or tag it stale or something if I end up having a asset manager
        }

        if (mRegistry->has<Sound>(id)) {
            // TODO: Same as texture, audio system needs to free the resource or let its resource manager at least know about this
        }

        if (policy.track) {
            destroyAndTrack(id);
        } else {
            destroy(id);
        }
    }

    void
    EntityManager::reset() {
        mRegistry->reset();
    }

    void
    EntityManager::printEntityType(EntityID id) const {
        const auto &name = mRegistry->get<EntityName>(id);
        printf("%d, %s ", id, name.name.str.data());
        if (mRegistry->has<WorldP3D>(id)) {
            const auto &pos = mRegistry->get<WorldP3D>(id);
            pos.value.print();
        } else {
            printf("\n");
        }
    }

    void
    EntityManager::printEntityType(const b2Body *body) {
        auto id = body->GetEntityID();
        auto *em = (EntityManager *) body->GetUserData();
        em->printEntityType(id);
    }

    void
    EntityManager::createPhysics(EntityID id) {
        if (!mPhysics) {
            assert(false);
            return;
        }
        const auto &size = mRegistry->get<Scale>(id);
        const auto &pos = mRegistry->get<WorldP3D>(id);
        const auto &ornt = mRegistry->get<Orientation>(id);

        auto &props = get<PhysicalProperties>(id);
        auto shape = b2PolygonShape{};
        shape.SetAsBox(size.x / 2.0f, size.y / 2.0f);

        // NOTE: This is non-owning pointer. physicsWorld owns it.
        b2Body *body = nullptr;

        b2BodyDef bodyDef;
        bodyDef.bullet = props.highPrecision;
        bodyDef.angle = ornt.value;
        bodyDef.linearDamping = props.linearDamping;
        bodyDef.angularDamping = props.angularDamping;
        bodyDef.gravityScale = props.gravityScale;

        b2FixtureDef fixtureDef;
        // NOTE: Box2D will create a copy of the shape, so it is safe to pass a local ref.
        fixtureDef.shape = &shape;
        fixtureDef.density = props.density;
        fixtureDef.friction = props.friction;
        fixtureDef.isSensor = props.isSensor;

        switch (props.bodyType.id) {
            case BodyType::GET("Dynamic").id : {
                bodyDef.position.x = pos.x;
                bodyDef.position.y = pos.y;
                bodyDef.type = b2_dynamicBody;
                body = mPhysics->getPhysicsWorld()->CreateBody(&bodyDef);
                assert(body);

                if (!props.collisionWithinCat) {
                    assert(props.physicalCat.id < std::numeric_limits<i16>::max());
                    assert(props.physicalCat.id > std::numeric_limits<i16>::min());
                    fixtureDef.filter.groupIndex = -static_cast<i16>(props.physicalCat.id);
                }

                body->CreateFixture(&fixtureDef);
                break;
            }
            case BodyType::GET("Static").id: {
                bodyDef.position.x = pos.x;
                bodyDef.position.y = pos.y;
                bodyDef.type = b2_staticBody;
                body = mPhysics->getPhysicsWorld()->CreateBody(&bodyDef);
                assert(body);
                body->CreateFixture(&shape, 0.f);
                break;
            }
            case BodyType::GET("Kinematic").id: {
                bodyDef.position.x = pos.x;
                bodyDef.position.y = pos.y;
                bodyDef.type = b2_kinematicBody;
                body = mPhysics->getPhysicsWorld()->CreateBody(&bodyDef);
                assert(body);
                body->CreateFixture(&fixtureDef);
                break;
            }
            default: {
                assert(false);
                break;
            }
        }

        if (body) {
            body->SetEntityID(id);
            body->SetUserData(this);
            createComponent<PhysicalBody>(id, body);
        } else {
            assert(false);
        }
    }

    void
    EntityManager::removePhysicalBody(EntityID id) {
        auto &body = get<PhysicalBody>(id);
        if (mPhysics) {
            mPhysics->getPhysicsWorld()->DestroyBody(body.value);
        } else {
            assert(false);
        }
        removeComponent<PhysicalBody>(id);
    }

    void
    EntityManager::setRandTTL(EntityID id,
                              r32 lower,
                              r32 upper) {
        auto &age = mRegistry->get<TimeToLive>(id);
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
    EntityManager::setRandOrientation(EntityID id) {
        return setRandOrientation(id, 0.f, FULL_CIRCLE_IN_RAD);
    }

    r32
    EntityManager::setRandOrientation(EntityID id,
                                      r32 lower,
                                      r32 upper) {
        auto &ornt = mRegistry->get<Orientation>(id);
        ornt.value = mRand->next_r32(lower, upper);
        return ornt.value;
    }

    void
    EntityManager::setDirectionFromOrientation(EntityID id) {
        const auto &ornt = mRegistry->get<Orientation>(id);
        auto &dir = mRegistry->get<Direction>(id);
        dir.x = std::cos(ornt.value);
        dir.y = std::sin(ornt.value);
    }

    void
    EntityManager::setRandDirection(EntityID id,
                                    r32 lowerX,
                                    r32 lowerY,
                                    r32 upperX,
                                    r32 upperY) {
        auto &dir = mRegistry->get<Direction>(id);
        dir.x = mRand->next_r32(lowerX, upperX);
        dir.y = mRand->next_r32(lowerY, upperY);
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
    EntityManager::setWorldP3D(EntityID id,
                               r32 x,
                               r32 y) {
        auto &pos = mRegistry->get<WorldP3D>(id);
        pos.x = x;
        pos.y = y;
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
    EntityManager::setOrientation(EntityID id,
                                  r32 ornt) {
        auto &h = mRegistry->get<Orientation>(id);
        h.value = ornt;
    }

    void
    EntityManager::setText(EntityID id,
                           std::string_view content) {
        auto &text = mRegistry->get<Material_Text>(id);
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

    SimMode
    EntityManager::getSimMode() {
        return mSimMode;
    }

    Rand *
    EntityManager::getRand() {
        return mRand.get();
    }
}