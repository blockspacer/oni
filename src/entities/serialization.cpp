#include <oni-core/entities/serialization.h>

#include <sstream>

#include <cereal/types/string.hpp>
#include <cereal/types/vector.hpp>
#include <cereal/types/array.hpp>
#include <cereal/types/map.hpp>
#include <cereal/archives/portable_binary.hpp>

#include <oni-core/entities/entity-manager.h>
#include <oni-core/component/geometry.h>
#include <oni-core/component/hierarchy.h>
#include <oni-core/component/visual.h>
#include <oni-core/component/gameplay.h>
#include <oni-core/gameplay/lap-tracker.h>


namespace oni {
    namespace gameplay {
        template<class Archive>
        void
        serialize(Archive &archive,
                  gameplay::CarLapInfo &carLapInfo) {
            archive(carLapInfo.entityID, carLapInfo.lap, carLapInfo.lapTimeS, carLapInfo.bestLapTimeS);
        }
    }

    namespace component {
        template<class Archive>
        void
        serialize(Archive &archive,
                  component::Shape &shape) {
            archive(shape.vertexA, shape.vertexB, shape.vertexC, shape.vertexD);
        }

        template<class Archive>
        void
        serialize(Archive &archive,
                  component::Point &point) {
            archive(point);
        }

        template<class Archive>
        void
        serialize(Archive &archive,
                  component::WorldP2D &pos) {
            archive(pos.value);
        }

        template<class Archive>
        void
        serialize(Archive &archive,
                  component::Size &size) {
            archive(size.value);
        }

        template<class Archive>
        void
        serialize(Archive &archive,
                  component::Trail &trail) {
            archive(trail.previousPos, trail.velocity);
        }

        template<class Archive>
        void
        serialize(Archive &archive,
                  component::EntityAttachment &attachment) {
            archive(attachment.entities, attachment.entityTypes);
        }

        template<class Archive>
        void
        serialize(Archive &archive,
                  component::EntityAttachee &attachee) {
            archive(attachee.entityID, attachee.entityType);
        }

        template<class Archive>
        void
        serialize(Archive &archive,
                  component::Heading &heading) {
            archive(heading.value);
        }

        template<class Archive>
        void
        serialize(Archive &archive,
                  component::Scale &scale) {
            archive(scale.value);
        }

        template<class Archive>
        void
        serialize(Archive &archive,
                  component::CarConfig &carConfig) {
            archive(carConfig.gravity,
                    carConfig.mass,
                    carConfig.inertialScale,
                    carConfig.halfWidth,
                    carConfig.cgToFront,
                    carConfig.cgToRear,
                    carConfig.cgToFrontAxle,
                    carConfig.cgToRearAxle,
                    carConfig.cgHeight,
                    carConfig.wheelRadius,
                    carConfig.wheelWidth,
                    carConfig.tireGrip,
                    carConfig.lockGrip,
                    carConfig.engineForce,
                    carConfig.brakeForce,
                    carConfig.eBrakeForce,
                    carConfig.weightTransfer,
                    carConfig.maxSteer,
                    carConfig.cornerStiffnessFront,
                    carConfig.cornerStiffnessRear,
                    carConfig.airResist,
                    carConfig.rollResist,
                    carConfig.gearRatio,
                    carConfig.differentialRatio
            );
        }

        template<class Archive>
        void
        serialize(Archive &archive,
                  component::Car &car) {
            archive(
                    car.heading,
                    car.velocityAbsolute,
                    car.angularVelocity,
                    car.steer,
                    car.steerAngle,
                    car.inertia,
                    car.wheelBase,
                    car.axleWeightRatioFront,
                    car.axleWeightRatioRear,
                    car.rpm,
                    car.maxVelocityAbsolute,
                    car.accumulatedEBrake,
                    car.slipAngleFront,
                    car.slipAngleRear,
                    car.position,
                    car.velocity,
                    car.velocityLocal,
                    car.acceleration,
                    car.accelerationLocal,
                    car.accelerating,
                    car.slippingFront,
                    car.slippingRear,
                    car.smoothSteer,
                    car.safeSteer,
                    car.distanceFromCamera,
                    car.isColliding
            );
        }

        template<class Archive>
        void
        serialize(Archive &archive,
                  component::TransformParent &transformParent) {
            archive(transformParent.parent, transformParent.transform);
        }

        template<class Archive>
        void
        serialize(Archive &archive,
                  component::Tag_Static &) {}

        template<class Archive>
        void
        serialize(Archive &archive,
                  component::Tag_Dynamic &) {}

        template<class Archive>
        void
        serialize(Archive &archive,
                  component::Tag_TextureShaded &) {}

        template<class Archive>
        void
        serialize(Archive &archive,
                  component::Tag_ColorShaded &) {}

        template<class Archive>
        void
        serialize(Archive &archive,
                  component::Tag_Audible &) {}

        template<class Archive>
        void
        serialize(Archive &archive,
                  component::Appearance &appearance) {
            archive(appearance.color);
        }

        template<class Archive>
        void
        serialize(Archive &archive,
                  component::Image &image) {
            archive(image.data, image.width, image.height);
        }

        template<class Archive>
        void
        serialize(Archive &archive,
                  component::Age &age) {
            archive(age.currentAge, age.maxAge);
        }

        template<class Archive>
        void
        serialize(Archive &archive,
                  component::Texture &texture) {
            archive(texture.image, texture.textureID, texture.format, texture.type, texture.filePath,
                    texture.uv, texture.status);
        }
    }

    namespace entities {
        std::string
        serialize(entities::EntityManager &manager,
                  entities::SnapshotType snapshotType) {
            std::stringstream storage{};
            {
                cereal::PortableBinaryOutputArchive output{storage};
                manager.snapshot<cereal::PortableBinaryOutputArchive,
                        component::Car,
                        component::CarConfig,
                        component::WorldP3D,
                        component::WorldP2D,
                        component::Heading,
                        component::Scale,
                        gameplay::CarLapInfo,
                        //components::Chunk,
                        component::Shape,
                        component::Size,
                        component::Point,
                        component::Appearance,
                        component::Age,
                        component::Texture,
                        component::Trail,
                        entities::EntityType,
                        component::EntityAttachment,
                        component::EntityAttachee,
                        component::SoundTag,

                        // TODO: This is a cluster fuck of a design. This is just a raw pointer. Client doesnt need
                        // to know what it points to at the moment because sever does the physics calculations and only
                        // send the results back to the client, so I can skip it. But for the future I have to
                        // find a solution to this shit.
                        //components::PhysicalProperties,

                        component::TransformParent,
                        component::Tag_Dynamic,
                        component::Tag_Static,
                        component::Tag_TextureShaded,
                        component::Tag_ColorShaded,
                        component::Tag_Audible
                >(output, snapshotType);
            }

            return storage.str();
        }

        void
        deserialize(EntityManager &manager,
                    const std::string &data,
                    entities::SnapshotType snapshotType) {
            std::stringstream storage;
            storage.str(data);

            {
                cereal::PortableBinaryInputArchive input{storage};
                manager.restore<cereal::PortableBinaryInputArchive,
                        component::Car,
                        component::CarConfig,
                        component::WorldP3D,
                        component::WorldP2D,
                        component::Heading,
                        component::Scale,
                        gameplay::CarLapInfo,
                        //components::Chunk,
                        component::Shape,
                        component::Size,
                        component::Point,
                        component::Appearance,
                        component::Age,
                        component::Texture,
                        component::Trail,
                        entities::EntityType,
                        component::EntityAttachment,
                        component::EntityAttachee,
                        component::SoundTag,

                        //components::PhysicalProperties,

                        component::TransformParent,
                        component::Tag_Dynamic,
                        component::Tag_Static,
                        component::Tag_TextureShaded,
                        component::Tag_ColorShaded,
                        component::Tag_Audible
                >(snapshotType, input,
                        // NOTE: Entities might keep references to other entities but those ids might change during
                        // client-server sync process, this will make sure that the client side does the correct
                        // mapping from client side ids to server side ids for each entity.
                  &component::EntityAttachment::entities,
                  &component::EntityAttachee::entityID,
                  &gameplay::CarLapInfo::entityID
                );
            }
        }
    }
}