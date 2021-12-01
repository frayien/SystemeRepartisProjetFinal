#include "Server.hpp"

#include "Operation.hpp"

#include <stdexcept>

Server::Server(std::uint16_t port)
{
    m_socket.bind(port);
}

Server::~Server()
{
}

void Server::run()
{
    sf::Packet packet;
    sf::IpAddress remoteAddress;
    std::uint16_t remotePort;

    // block until we receive a packet
    if(m_socket.receive(packet, remoteAddress, remotePort) != sf::Socket::Done)
    {
        throw std::runtime_error("Error while receiving a packet.");
    }

    Operation op;
    if(!(packet >> op))
    {
        throw std::runtime_error("Attempted to read beyond the packet size.");
    }

    switch(op)
    {
    case Operation::NO_OP:
        break;
    case Operation::CONNECT:
        connect(remoteAddress, remotePort);
        break;
    case Operation::DISCONNECT:
        disconnect(remoteAddress, remotePort);
        break;
    case Operation::TRANSACTION:
        break;
    default:
        throw std::runtime_error("Unsupported operation.");
        break;
    }
}

void Server::sendOk(sf::IpAddress remoteAddress, std::uint16_t remotePort)
{
    sf::Packet packet;
    packet << Operation::OK;

    if(m_socket.send(packet, remoteAddress, remotePort) != sf::Socket::Done)
    {
        throw std::runtime_error("Error while sending a packet.");
    }
}

void Server::sendError(sf::IpAddress remoteAddress, std::uint16_t remotePort, std::string msg)
{
    sf::Packet packet;
    packet << Operation::ERROR << msg;

    if(m_socket.send(packet, remoteAddress, remotePort) != sf::Socket::Done)
    {
        throw std::runtime_error("Error while sending a packet.");
    }
}

void Server::connect(sf::IpAddress remoteAddress, std::uint16_t remotePort)
{
    m_clients.push_front({remoteAddress, remotePort});
    sendOk(remoteAddress, remotePort);
}

void Server::disconnect(sf::IpAddress remoteAddress, std::uint16_t remotePort)
{
    m_clients.remove({remoteAddress, remotePort});
    sendOk(remoteAddress, remotePort);
}