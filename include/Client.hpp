#ifndef SRPF_CLIENT
#define SRPF_CLIENT

#include <SFML/Network.hpp>

#include <cstdint>

class Client
{  
public:
    Client(sf::IpAddress serverAddress, std::uint16_t serverPort);
    ~Client();

private:
    sf::IpAddress m_serverAddress;
    std::uint16_t m_serverPort;

    sf::UdpSocket m_socket;

    void send(sf::Packet packet);
    sf::Packet receive();
    void receiveAck();

    void connect();
    void disconnect();
};

#endif // SRPF_CLIENT
