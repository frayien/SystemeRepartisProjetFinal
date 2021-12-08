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

    std::cout << "[SERVER] received op " << static_cast<int>(op) << std::endl;

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
        handleTransaction(packet);
        break;
    case Operation::VALID_BLOCK:
        handleValidBlock(packet);
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

void Server::handleTransaction(sf::Packet & packet)
{
    std::string transaction;
    packet >> transaction;

    m_nextBlockData += transaction;

    if(!m_currentlyMining) createBlock(transaction);
}

void Server::createBlock(std::string data)
{
    m_currentlyMining = true;

    auto previousIndex = m_blockchain.GetLastBlock().nIndex;
    Block block(data);

    m_blockchain.prepareBlock(block);

    sendBlockForValidation(block);
}

void Server::sendBlockForValidation(const Block & block)
{
    sf::Packet packet;
    packet << Operation::REQUEST_VALIDATION;
    packet << m_blockchain.getDifficulty();
    packet << block;

    for(const auto & [ipaddr, port] : m_clients)
    {
        if(m_socket.send(packet, ipaddr, port) != sf::Socket::Done)
        {
            throw std::runtime_error("Error while sending a block for validation.");
        }
    }
}

void Server::handleValidBlock(sf::Packet & packet)
{
    Block block;
    packet >> block;

    m_blockchain.prepareBlock(block);

    if(block.CalculateHash() != block.sHash)
        return;

    m_blockchain.AddBlock(block);

    sendEndMining();
    
    if(!m_nextBlockData.empty())
    {
        std::string data = m_nextBlockData;
        m_nextBlockData.clear();
        createBlock(data);
    }
    else
    {
        m_currentlyMining = false;
    }
}

void Server::sendEndMining()
{
    sf::Packet packet;
    packet << Operation::END_MINING;

    for(const auto & [ipaddr, port] : m_clients)
    {
        if(m_socket.send(packet, ipaddr, port) != sf::Socket::Done)
        {
            throw std::runtime_error("Error while sending end mining.");
        }
    }
}