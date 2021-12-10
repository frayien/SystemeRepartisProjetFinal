#ifndef SRPF_SERVER
#define SRPF_SERVER

#include <SFML/Network.hpp>

#include <forward_list>
#include <cstdint>
#include <utility>

#include "Blockchain.hpp"

class Server
{
public:
    Server(std::uint16_t port, std::uint32_t difficulty);
    ~Server();

    void run();
    
private:
    sf::UdpSocket m_socket;
    std::forward_list<std::pair<sf::IpAddress, std::uint16_t>> m_clients;

    Blockchain m_blockchain;

    bool m_currentlyMining = false;
    std::string m_nextBlockData;

    void sendOk(sf::IpAddress remoteAddress, std::uint16_t remotePort);
    void sendError(sf::IpAddress remoteAddress, std::uint16_t remotePort, std::string msg);

    void connect(sf::IpAddress remoteAddress, std::uint16_t remotePort);
    void disconnect(sf::IpAddress remoteAddress, std::uint16_t remotePort);

    void handleTransaction(sf::Packet & packet);
    void createBlock(std::string data);

    void sendBlockForValidation(const Block & block);

    void handleValidBlock(sf::Packet & packet);
    void sendEndMining();
};

#endif // SRPF_SERVER
