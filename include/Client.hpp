#ifndef SRPF_CLIENT
#define SRPF_CLIENT

#include <SFML/Network.hpp>

#include <cstdint>
#include <functional>
#include <memory>

#include "Block.hpp"

class Client
{  
public:
    Client(sf::IpAddress serverAddress, std::uint16_t serverPort, std::size_t id);
    ~Client();

    void run();

    void sendTransaction(std::string transaction);

    void connect();
    void disconnect();

    void log(std::initializer_list<std::string> messages) const;

private:
    sf::IpAddress m_serverAddress;
    std::uint16_t m_serverPort;
    std::size_t m_id;

    sf::UdpSocket m_socket;

    bool m_connected = false;
    bool m_shouldMine = false;

    std::unique_ptr<sf::Thread> m_miningThread;

    void receiveError(sf::Packet & packet);
    
    void receiveMining(sf::Packet & packet);
    void mine(Block & block, std::uint32_t difficulty);

    void sendNonce(const Block & block);

    void ensureCorrectness(sf::Packet & packet);
    void sendConfirmCorrectness(const Block & block, bool isValid);
};

#endif // SRPF_CLIENT
