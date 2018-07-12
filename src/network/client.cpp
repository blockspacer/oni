#include <oni-core/network/client.h>

#include <string>
#include <chrono>
#include <iostream>

#include <enet/enet.h>

#include <oni-core/io/output.h>
#include <oni-core/network/packet.h>
#include <oni-core/network/packet-operation.h>

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
            auto pingPacket = PingPacket(static_cast<common::uint64>(now));

            sendPacket<PingPacket>(&pingPacket, mEnetPeer);
        }

        void Client::handle(ENetEvent *event) {
        }

        void Client::sendMessage(const std::string &message) {
            auto messagePacket = MessagePacket(message);

            sendPacket<MessagePacket>(&messagePacket, mEnetPeer);
        }

    }
}