#include "Client.hpp"

#include "Operation.hpp"

#include <stdexcept>

Client::Client(sf::IpAddress serverAddress, std::uint16_t serverPort) :
    m_serverAddress(serverAddress),
    m_serverPort(serverPort)
{
    m_socket.bind(sf::Socket::AnyPort);
    connect();
}

Client::~Client()
{
    disconnect();
}

void Client::send(sf::Packet packet)
{
    if(m_socket.send(packet, m_serverAddress, m_serverPort) != sf::Socket::Done)
    {
        throw std::runtime_error("Error while sending a packet.");
    }
}

sf::Packet Client::receive()
{
    sf::Packet packet;
    sf::IpAddress addr;
    std::uint16_t port;
    if(m_socket.receive(packet, addr, port) != sf::Socket::Done)
    {
        throw std::runtime_error("Error while receiving a packet.");
    }

    return packet;
}

void Client::receiveAck()
{
    sf::Packet packet = receive();

    Operation op;
    packet >> op;

    if(op == Operation::OK)
    {
        return;
    }

    if(op == Operation::ERROR)
    {
        std::string msg;
        packet >> msg;

        throw std::runtime_error(msg);
    }

    throw std::runtime_error("Uknown error.");
}

void Client::connect()
{
    sf::Packet packet;
    packet << Operation::CONNECT;

    send(packet);
    receiveAck();
}

void Client::disconnect()
{
    sf::Packet packet;
    packet << Operation::DISCONNECT;

    send(packet);
    receiveAck();
}
