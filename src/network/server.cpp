#include <oni-core/network/server.h>

#include <enet/enet.h>

#include <oni-core/entities/entity-manager.h>
#include <oni-core/entities/serialization.h>
#include <oni-core/component/audio.h>


namespace oni {
    namespace network {
        Server::Server(const Address *address, common::uint8 numClients, common::uint8 numChannels) :
                Peer::Peer(address, numClients, numChannels, 0, 0) {
        }

        Server::Server() = default;

        Server::~Server() = default;

        void Server::postConnectHook(const ENetEvent *event) {
        }

        void Server::postDisconnectHook(const ENetEvent *event) {
            auto clientID = getPeerID(*event->peer);

            mPostDisconnectHook(clientID);
        }

        void Server::handle(ENetPeer *peer, enet_uint8 *data, size_t size, PacketType header) {
            auto peerID = getPeerID(*peer);
            assert(mPacketHandlers.find(header) != mPacketHandlers.end() || header == PacketType::PING ||
                   header == PacketType::MESSAGE);
            switch (header) {
                case (PacketType::PING): {
                    auto type = PacketType::PING;
                    auto pingData = std::string{};

                    send(type, pingData, peer);

                    break;
                }
                case (PacketType::MESSAGE): {
                    auto packet = entities::deserialize<DataPacket>(data, size);
                    break;
                }
                case (PacketType::SETUP_SESSION): {
                    mPacketHandlers[PacketType::SETUP_SESSION](peerID, "");
                    break;
                }
                case (PacketType::CLIENT_INPUT): {
                    auto dataString = std::string(reinterpret_cast<char *>(data), size);
                    mPacketHandlers[PacketType::CLIENT_INPUT](peerID, dataString);
                    break;
                }
                default: {
                    // TODO: Need to keep stats on clients with bad packets and block them when threshold reaches.
                    assert(false);
                    break;
                }
            }
        }

        void Server::sendEntitiesAll(entities::EntityManager &manager) {
            std::string data = entities::serialize(manager, component::SnapshotType::ENTIRE_REGISTRY);
            auto type = PacketType::REPLACE_ALL_ENTITIES;

            if (data.size() > 1) {
                broadcast(type, data);
            }
        }

        void Server::sendComponentsUpdate(entities::EntityManager &manager) {
            std::string data = entities::serialize(manager, component::SnapshotType::ONLY_COMPONENTS);
            auto type = PacketType::ONLY_COMPONENT_UPDATE;

            if (data.size() > 1) {
                broadcast(type, data);

                auto lock = manager.scopedLock();
                // TODO: What happens if broadcast fails for some clients? Would they miss these entities forever?
                manager.reset<component::Tag_OnlyComponentUpdate>();

                auto view = manager.createView<component::Trail>();
                for (auto &&entity: view) {
                    view.get<component::Trail>(entity).velocity.clear();
                    view.get<component::Trail>(entity).previousPos.clear();
                }
            }
        }

        void Server::sendNewEntities(entities::EntityManager &manager) {
            std::string data = entities::serialize(manager, component::SnapshotType::ONLY_NEW_ENTITIES);
            auto type = PacketType::ADD_NEW_ENTITIES;

            if (data.size() > 1) {
                broadcast(type, data);

                auto lock = manager.scopedLock();
                // TODO: What happens if broadcast fails for some clients? Would they miss these entities forever?
                manager.reset<component::Tag_SyncUsingRegistry>();

                auto view = manager.createView<component::Trail>();
                for (auto &&entity: view) {
                    view.get<component::Trail>(entity).velocity.clear();
                    view.get<component::Trail>(entity).previousPos.clear();
                }
            }
        }

        void Server::broadcastDeletedEntities(entities::EntityManager &manager) {
            auto type = PacketType::DESTROYED_ENTITIES;
            auto data = entities::serialize(manager.getDeletedEntities());

            if (data.size() > 1) {
                broadcast(type, data);
            }
        }

        void Server::sendCarEntityID(common::EntityID entityID, const common::PeerID &peerID) {
            auto packet = EntityPacket{entityID};
            auto data = entities::serialize(packet);
            auto type = PacketType::CAR_ENTITY_ID;

            send(type, data, mPeers[peerID]);
        }

        void Server::broadcastSpawnParticle(entities::EntityManager &manager) {
            std::vector<component::Particle> particles;
            {
                // TODO: This is stupid, the whole particle and related components are created server side just so
                // to send a dummy packet to clients. Why not just create a minimum data type that is only for
                // network event triggering? And let the clients handle the full entity creation as it is only
                // client side data anyway.
                auto view = manager.createViewScopeLock<component::Particle, component::Tag_SyncUsingPacket>();
                for (auto &&entity: view) {
                    particles.emplace_back(view.get<component::Particle>(entity));
                    manager.destroy(entity);
                }
            }

            // TODO: This is just way too many packets. I should batch them together.
            for (auto &&particle: particles) {
                auto data = entities::serialize(particle);
                auto type = PacketType::SPAWN_PARTICLE;

                broadcast(type, data);
            }
        }

        void Server::broadcastOneShotSoundEffects(entities::EntityManager &manager) {
            std::vector<component::SoundEffect> soundEffects;
            {
                auto view = manager.createViewScopeLock<component::SoundEffect, component::Tag_OneShot, component::Tag_SyncUsingPacket>();
                for (auto &&entity: view) {
                    soundEffects.emplace_back(view.get<component::SoundEffect>(entity));
                    manager.destroy(entity);
                }
            }

            // TODO: Why not send one packet with all the effects?
            for (auto &&soundEffect: soundEffects) {
                auto data = entities::serialize(soundEffect);
                auto type = PacketType::ONE_SHOT_SOUND_EFFECT;

                broadcast(type, data);
            }
        }
    }
}
