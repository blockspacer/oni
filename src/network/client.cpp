#include <oni-core/network/client.h>

#include <chrono>

#include <entt/entity/registry.hpp>

#include <oni-core/io/output.h>
#include <oni-core/entities/serialization.h>
#include <oni-core/components/geometry.h>

namespace oni {
    namespace network {

        Client::Client() : Peer::Peer(nullptr, 1, 2, 0, 0) {
        }

        Client::~Client() = default;

        void Client::connect(const Address &address) {
            auto enetAddress = ENetAddress{};
            enet_address_set_host(&enetAddress, address.host.c_str());
            enetAddress.port = address.port;

            mEnetPeer = enet_host_connect(mEnetHost, &enetAddress, 2, 0);
            if (!mEnetPeer) {
                std::runtime_error(
                        "Failed to initiate connection to: " + address.host + ":" + std::to_string(address.port));
            }
            ENetEvent event;
            if (enet_host_service(mEnetHost, &event, 5000) > 0 && event.type == ENET_EVENT_TYPE_CONNECT) {
                io::printl("Connected to: " + address.host + ":" + std::to_string(address.port));
            } else {
                std::runtime_error(
                        "Failed connecting to: " + address.host + ":" + std::to_string(address.port));
            }
        }

        void Client::pingServer() {
            auto now = std::chrono::system_clock::now().time_since_epoch().count();
            auto type = PacketType::PING;
            auto pingPacket = PingPacket{now};
            auto data = serialize<PingPacket>(pingPacket);

            send(type, std::move(data), mEnetPeer);
        }

        void Client::handle(ENetPeer *peer, enet_uint8 *data, size_t size, PacketType header) {
            switch (header) {
                case (PacketType::PING): {
                    auto packet = deserialize<PingPacket>(data, size);
                    std::cout << packet.timestamp << std::endl;

                    break;
                }
                case (PacketType::MESSAGE): {
                    auto packet = deserialize<DataPacket>(data, size);
                    std::cout << packet.data << std::endl;
                    break;
                }
                case (PacketType::ENTITY): {
                    auto entityData = std::string(reinterpret_cast<char *>(data), size);

                    entt::DefaultRegistry reg;
                    entities::deserialization(reg, entityData);

                    auto view = reg.view<components::Shape>();
                    for (auto e: view) {
                        auto a = reg.get<components::Shape>(e).vertexA;
                        auto d = reg.get<components::Shape>(e).vertexD;
                    }

                    break;
                }
                default: {
                    std::cout << "Unknown packet!" << std::endl;
                    break;
                }
            }
        }

        void Client::sendMessage(std::string &&message) {
            auto type = PacketType::MESSAGE;
            auto messagePacket = DataPacket{std::move(message)};
            auto data = serialize<DataPacket>(messagePacket);

            send(type, std::move(data), mEnetPeer);
        }

        void Client::postConnectHook(const ENetEvent *event) {

        }

        void Client::postDisconnectHook(const ENetEvent *event) {

        }
    }
}