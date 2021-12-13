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

    void log(std::initializer_list<std::string> messages) const;

    void run();
    
private:
    sf::UdpSocket m_socket;
    std::forward_list<std::pair<sf::IpAddress, std::uint16_t> > m_clients;

    const sf::Time NONCE_CONFIRMATION_TIMEOUT = sf::seconds(5.f);
    sf::Clock m_nonce_confirmation_timer;
    std::forward_list<std::pair<sf::IpAddress, std::uint16_t> > m_nonce_confirmation_waited_for_clients;

    Blockchain m_blockchain;

    bool m_currentlyMining = false;
    Block m_currentlyMinedBlock;
    std::string m_nextBlockData;

    void sendOk(sf::IpAddress remoteAddress, std::uint16_t remotePort);
    void sendError(sf::IpAddress remoteAddress, std::uint16_t remotePort, std::string msg);

    void connect(sf::IpAddress remoteAddress, std::uint16_t remotePort);
    void disconnect(sf::IpAddress remoteAddress, std::uint16_t remotePort);

    void handleTransaction(sf::Packet & packet);
    void createBlock();

    void sendBlockForValidation();

    void handleFoundNonce(sf::Packet & packet);
    void sendEnsureCorrectness();

    void handleConfirmCorrectness(sf::Packet & packet, sf::IpAddress remoteAddress, std::uint16_t remotePort);
    void endMining();
    void sendEndMining();
};

#endif // SRPF_SERVER
