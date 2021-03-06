#include <oni-core/network/oni-network-server.h>

#include <enet/enet.h>

#include <oni-core/entities/oni-entities-manager.h>
#include <oni-core/entities/oni-entities-serialization.h>
#include <oni-core/entities/oni-entities-serialization-network.h>
#include <oni-core/component/oni-component-audio.h>


namespace oni {
    Server::Server(const Address *address,
                   u8 numClients,
                   u8 numChannels) :
            Peer::Peer(address, numClients, numChannels, 0, 0) {
    }

    Server::Server() = default;

    Server::~Server() = default;

    void
    Server::postConnectHook(const ENetEvent *event) {
    }

    void
    Server::postDisconnectHook(const ENetEvent *event) {
        auto clientID = getPeerID(*event->peer);

        mPostDisconnectHook(clientID);
    }

    void
    Server::handle(ENetPeer *peer,
                   enet_uint8 *data,
                   size size,
                   PacketType header) {
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
                auto packet = deserialize<Packet_Data>(data, size);
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

    void
    Server::sendEntitiesAll(EntityManager &manager,
                            std::string &&data) {
        auto type = PacketType::REGISTRY_REPLACE_ALL_ENTITIES;
        broadcast(type, std::move(data));
    }

    void
    Server::sendComponentsUpdate(EntityManager &manager,
                                 std::string &&data) {
        // TODO: What happens if broadcast fails for some clients? Would they miss these entities forever?
        auto type = PacketType::REGISTRY_ONLY_COMPONENT_UPDATE;
        broadcast(type, std::move(data));
    }

    void
    Server::sendNewEntities(EntityManager &manager,
                            std::string &&data) {
        // TODO: What happens if broadcast fails for some clients? Would they miss these entities forever?
        auto type = PacketType::REGISTRY_ADD_NEW_ENTITIES;

        broadcast(type, std::move(data));
    }

    void
    Server::broadcastDeletedEntities(EntityManager &manager) {
        auto type = PacketType::REGISTRY_DESTROYED_ENTITIES;
        auto data = serialize(manager.getDeletedEntities());
        manager.clearDeletedEntitiesList();

        broadcast(type, std::move(data));
    }

    void
    Server::sendCarEntityID(EntityID entityID,
                            const std::string &peerID) {
        auto packet = Packet_EntityID{entityID};
        auto data = oni::serialize(packet);
        auto type = PacketType::CAR_ENTITY_ID;

        send(type, data, mPeers[peerID]);
    }

    void
    Server::handleEvent_Collision(const oni::Event_Collision &event) {
        auto data = oni::serialize(event);
        auto packetType = oni::PacketType::EVENT_COLLISION;

        broadcast(packetType, std::move(data));
    }

    void
    Server::handleEvent_SoundPlay(const oni::Event_SoundPlay &event) {
        auto data = oni::serialize(event);
        auto packetType = oni::PacketType::EVENT_SOUND_PLAY;

        broadcast(packetType, std::move(data));
    }

    void
    Server::handleEvent_RocketLaunch(const oni::Event_RocketLaunch &event) {
        auto data = oni::serialize(event);
        auto packetType = oni::PacketType::EVENT_ROCKET_LAUNCH;

        broadcast(packetType, std::move(data));
    }
}
