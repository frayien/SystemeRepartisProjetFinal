#include "Client.hpp"

#include "Operation.hpp"

#include <stdexcept>
#include <ctime>

Client::Client(sf::IpAddress serverAddress, std::uint16_t serverPort) :
    m_serverAddress(serverAddress),
    m_serverPort(serverPort)
{
    m_socket.bind(sf::Socket::AnyPort);
}

Client::~Client()
{
    if(m_connected) disconnect();
}

void Client::log(std::initializer_list<std::string> messages) const
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

    log({"received op ", std::to_string(static_cast<int>(op)), " ", to_string(op)});

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

    m_connected = false;
}

void Client::receiveMining(sf::Packet & packet)
{
    Block block;
    std::uint32_t difficulty;
    packet >> difficulty >> block;

    log({"Received block with content : '", block.sData, "'"});

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

    log({"Block mined with hash : ", block.sHash});
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