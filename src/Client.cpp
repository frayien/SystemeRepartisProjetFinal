#include "Client.hpp"

#include "Operation.hpp"

#include <stdexcept>

Client::Client(sf::IpAddress serverAddress, std::uint16_t serverPort) :
    m_serverAddress(serverAddress),
    m_serverPort(serverPort)
{
    m_socket.bind(sf::Socket::AnyPort);
}

Client::~Client()
{
    disconnect();
}

void Client::run()
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

    std::cout << "[CLIENT] received op " << static_cast<int>(op) << std::endl;

    switch(op)
    {
    case Operation::NO_OP:
        break;
    case Operation::OK:
        m_connected = true;
        break;
    case Operation::ERROR:
        receiveError(packet);
        break;
    case Operation::REQUEST_VALIDATION:
        receiveMining(packet);
        break;
    case Operation::END_MINING:
        m_shouldMine = false;
        break;
    default:
        throw std::runtime_error("Unsupported operation.");
        break;
    }
}

void Client::sendTransaction(std::string transaction)
{
    sf::Packet packet;
    packet << Operation::TRANSACTION << transaction;

    if(m_socket.send(packet, m_serverAddress, m_serverPort) != sf::Socket::Done)
    {
        throw std::runtime_error("Error while sending a transaction.");
    }
}

void Client::receiveError(sf::Packet & packet)
{
    std::string msg;
    packet >> msg;

    throw std::runtime_error(msg);
}

void Client::connect()
{
    sf::Packet packet;
    packet << Operation::CONNECT;

    if(m_socket.send(packet, m_serverAddress, m_serverPort) != sf::Socket::Done)
    {
        throw std::runtime_error("Error while connecting to server.");
    }

    sf::Clock clock;
    while(!m_connected)
    {
        sf::sleep(sf::milliseconds(1));

        if(clock.getElapsedTime() > sf::seconds(5.f))
        {
            throw std::runtime_error("Timeout while connecting to server.");
        }
    }
}

void Client::disconnect()
{
    sf::Packet packet;
    packet << Operation::DISCONNECT;

    if(m_socket.send(packet, m_serverAddress, m_serverPort) != sf::Socket::Done)
    {
        throw std::runtime_error("Error while disconnecting from server.");
    }
}

void Client::receiveMining(sf::Packet & packet)
{
    Block block;
    std::uint32_t difficulty;
    packet >> difficulty >> block;

    if(m_miningThread)
    {
        m_shouldMine = false;
        m_miningThread->wait();
    }

    m_shouldMine = true;

    m_miningThread = std::make_unique<sf::Thread>([this, block, difficulty](){
        Block l_block = block;
        mine(l_block, difficulty);

        if(m_shouldMine) sendBlock(l_block);
    });

    m_miningThread->launch();
}

void Client::mine(Block & block, std::uint32_t difficulty)
{
    std::string str(difficulty, '0');

    do
    {
        block.nNonce++;
        block.sHash = block.CalculateHash();
    }
    while (!block.sHash.starts_with(str) && m_shouldMine);

    std::cout << "Block mined: " << block.sHash << std::endl;
}

void Client::sendBlock(const Block & block)
{
    sf::Packet packet;
    packet << Operation::VALID_BLOCK;
    packet << block;

    if(m_socket.send(packet, m_serverAddress, m_serverPort) != sf::Socket::Done)
    {
        throw std::runtime_error("Error while sending a valid block.");
    }
    
}