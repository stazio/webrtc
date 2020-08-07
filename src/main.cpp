#include <iostream>
#include "version.h"
#include <memory>
#include <seasocks/PrintfLogger.h>
#include <seasocks/Server.h>
#include <libs/libdatachannel/include/rtc/log.hpp>
#include <libs/libdatachannel/include/rtc/rtc.hpp>

#include <nlohmann/json.hpp>
//#include <libs/libdatachannel/deps/json/single_include/nlohmann/json.hpp>

using json = nlohmann::json;

class MyHandler : public seasocks::WebSocket::Handler {
    seasocks::Server* server;

public:
    MyHandler(seasocks::Server* server): server(server) {}

    struct t {
        std::shared_ptr<rtc::PeerConnection> pc;
        std::shared_ptr<rtc::DataChannel> dc;
    };
    std::map<seasocks::WebSocket*, t> peers;

    void onConnect(seasocks::WebSocket* connection) override {
        std::cout << "CONNECT" << std::endl;

        rtc::Configuration config;
        config.iceServers.emplace_back("stun:stun.l.google.com:19302");
        peers[connection] = {.pc = std::make_shared<rtc::PeerConnection>(config)};

        auto pc = peers[connection].pc;

        pc->onStateChange([](rtc::PeerConnection::State state) { std::cout << "State: " << state << std::endl; });

        pc->onGatheringStateChange(
                [](rtc::PeerConnection::GatheringState state) { std::cout << "Gathering State: " << state << std::endl; });

        pc->onLocalDescription([connection, this](const rtc::Description &description) {
            json message = {
                    {"type", description.typeString()}, {"sdp", std::string(description)}
            };

            std::cout << message << std::endl;
            server->execute([connection, message](){connection->send(message.dump());});
        });

        pc->onLocalCandidate([connection, this](const rtc::Candidate &candidate) {
            json message = {
                            {"type", "candidate"},
                            {"candidate", std::string(candidate)},
                            {"sdpMLineIndex", candidate.mid()}
            };

            server->execute([connection, message](){connection->send(message.dump());});
        });
        rtc::Reliability rel;
        rel.unordered = true;
        rel.type = rtc::Reliability::TYPE_PARTIAL_RELIABLE_REXMIT;
        rel.rexmit = 0;
        peers[connection].dc = pc->createDataChannel("test", "", rel);
        peers[connection].dc->onOpen([]{
            std::cout << "OPEN" << std::endl;
        });
        auto dc = peers[connection].dc;
        peers[connection].dc->onMessage([dc](std::variant<rtc::binary, rtc::string> data) {
//            std::cout << "DATA" << std::endl;

            std::string res = std::get<rtc::string>(data);
            std::cout << res << std::endl;
            dc->send(res);
        });
    }
    void onData(seasocks::WebSocket* connection, const char* data) override {

        json parsed = json::parse(data);
        if (parsed.contains("candidate")) {
            std::cout << "candidate" << " " << parsed["candidate"] << std::endl;
            std::cout << data << std::endl;
            peers[connection].pc->addRemoteCandidate(rtc::Candidate(parsed["candidate"],parsed["sdpMid"]));
        }
        if (parsed["type"] == "answer") {
            std::cout << "answer" << std::endl;
            peers[connection].pc->setRemoteDescription(rtc::Description(parsed["sdp"], rtc::Description::Type::Answer));
        }
    }

    void onDisconnect(seasocks::WebSocket* connection) override {
        std::cout << "BYE" << std::endl;
    }

};

int main(int argc, char** argv) {
    std::cout << "Starting " << PROJECT_NAME << " v. [" << GIT_REV << "]" <<std::endl;

    auto logger = std::make_shared<seasocks::PrintfLogger>(seasocks::Logger::Level::Info);
    seasocks::Server server(logger);
    rtc::InitLogger(rtc::LogLevel::Debug);

    auto handler = std::make_shared<MyHandler>(&server);
    server.addWebSocketHandler("/ws", handler);

    server.serve("web", 8080);

    return 0;
}
