#include "Server.hpp"

#include "Operation.hpp"

#include <stdexcept>

Server::Server(std::uint16_t port, std::uint32_t difficulty) :
    m_blockchain(difficulty)
{
    m_socket.bind(port);
}

Server::~Server()
{
}

void Server::log(std::initializer_list<std::string> messages) const
{
    auto now = std::time(nullptr);
    auto tm = std::localtime(&now);
    
    std::cout << "[SERVER][";
    if(tm->tm_hour < 10) std::cout << 0;
    std::cout << tm->tm_hour << ":";
    if(tm->tm_min < 10) std::cout << 0;
    std::cout << tm->tm_min << ":";
    if(tm->tm_sec < 10) std::cout << 0;
    std::cout << tm->tm_sec << "] ";

    for(std::string msg : messages)
    {
        std::cout << msg;
    }
    std::cout << std::endl;
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

    log({"received op ", std::to_string(static_cast<int>(op)), " ", to_string(op)});

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

    if(!m_currentlyMining) createBlock();
}

void Server::createBlock()
{
    if(m_nextBlockData.empty()) return;

    m_currentlyMining = true;

    log({"Creating block with content : '", m_nextBlockData, "'"});

    Block block(m_nextBlockData);
    m_nextBlockData.clear();

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
        createBlock();
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