#ifndef SRPF_SERVER
#define SRPF_SERVER

#include <SFML/Network.hpp>

#include <forward_list>
#include <cstdint>
#include <utility>

class Server
{
public:
    Server(std::uint16_t port);
    ~Server();

    void run();
    
private:
    sf::UdpSocket m_socket;
    std::forward_list<std::pair<sf::IpAddress, std::uint16_t>> m_clients;

    void sendOk(sf::IpAddress remoteAddress, std::uint16_t remotePort);
    void sendError(sf::IpAddress remoteAddress, std::uint16_t remotePort, std::string msg);

    void connect(sf::IpAddress remoteAddress, std::uint16_t remotePort);
    void disconnect(sf::IpAddress remoteAddress, std::uint16_t remotePort);
};

#endif // SRPF_SERVER
