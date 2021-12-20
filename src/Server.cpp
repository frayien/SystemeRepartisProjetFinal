#include "Server.hpp"

#include "Operation.hpp"

#include <stdexcept>
#include <sstream>
#include <random>

Server::Server(std::uint16_t port, std::uint32_t difficulty, Mode mode) :
    m_blockchain(difficulty),
    m_mode(mode)
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

    std::stringstream sstream;
    
    sstream << "[SERVER][";
    if(tm->tm_hour < 10) sstream << 0;
    sstream << tm->tm_hour << ":";
    if(tm->tm_min < 10) sstream << 0;
    sstream << tm->tm_min << ":";
    if(tm->tm_sec < 10) sstream << 0;
    sstream << tm->tm_sec << "] ";

    for(std::string msg : messages)
    {
        sstream << msg;
    }
    sstream << std::endl;
    std::cout << sstream.str();
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

    log({"received op ", std::to_string(static_cast<int>(op)), " : ", to_string(op)});

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
    case Operation::FOUND_NONCE:
        handleFoundNonce(packet);
        break;
    case Operation::CONFIRM_CORRECTNESS:
        handleConfirmCorrectness(packet, remoteAddress, remotePort);
        break;
    default:
        throw std::runtime_error("Unsupported operation.");
        break;
    }

    if(m_currentlyEnsuringCorrectness && m_nonce_confirmation_timer.getElapsedTime() > NONCE_CONFIRMATION_TIMEOUT)
    {
        endMining();
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
    log({"New Client : ", remoteAddress.toString(), ":", std::to_string(remotePort)});
    m_clients.push_front({remoteAddress, remotePort});
    sendOk(remoteAddress, remotePort);
}

void Server::disconnect(sf::IpAddress remoteAddress, std::uint16_t remotePort)
{
    m_clients.remove({remoteAddress, remotePort});
    m_nonce_confirmation_waited_for_clients.remove({remoteAddress, remotePort});
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

    m_currentlyMinedBlock = Block(m_nextBlockData);
    m_nextBlockData.clear();

    m_blockchain.prepareBlock(m_currentlyMinedBlock);

    sendBlockForValidation();
}

void Server::sendBlockForValidation()
{
    sf::Packet packet;
    packet << Operation::REQUEST_VALIDATION;
    packet << m_blockchain.getDifficulty();
    packet << m_currentlyMinedBlock;

    if (m_mode == Mode::PoW)
    {
        for(const auto& [ipaddr, port] : m_clients)
        {
            if(m_socket.send(packet, ipaddr, port) != sf::Socket::Done)
            {
                throw std::runtime_error("Error while sending a block for validation.");
            }
        }
    }
    else if (m_mode == Mode::PoS)
    {
        std::random_device rd;
        std::mt19937 mt(rd());
        
        std::uniform_int_distribution dist(0ull, m_clients.size()-1);

        std::size_t random_i = dist(mt);
        auto it = m_clients.begin();
        std::advance(it, random_i);
        auto & [ipaddr, port] = *it;

        if(m_socket.send(packet, ipaddr, port) != sf::Socket::Done)
        {
            throw std::runtime_error("Error while sending a block for validation.");
        }
    }

}

void Server::handleFoundNonce(sf::Packet & packet)
{
    if(!m_currentlyMining) throw std::runtime_error("Error no block is currently mined.");
    if(m_currentlyEnsuringCorrectness) throw std::runtime_error("Error nonce alredy found.");

    std::uint32_t index;
    std::int64_t nonce;

    packet >> index >> nonce;

    if(m_currentlyMinedBlock.nIndex != index) throw std::runtime_error("Error received wrong block.");

    m_currentlyMinedBlock.nNonce = nonce;

    if(m_mode == Mode::PoW)
    {
        m_currentlyEnsuringCorrectness = true;
    }

    sendEnsureCorrectness();

    if(m_mode == Mode::PoS)
    {
        endMining();
    }
}

void Server::sendEnsureCorrectness()
{
    sf::Packet packet;
    packet << Operation::ENSURE_CORRECTNESS;
    packet << m_blockchain.getDifficulty();
    packet << m_currentlyMinedBlock;
    
    m_nonce_confirmation_waited_for_clients.clear();
    m_nonce_confirmation_timer.restart();

    for(const auto & [ipaddr, port] : m_clients)
    {
        if(m_socket.send(packet, ipaddr, port) != sf::Socket::Done)
        {
            throw std::runtime_error("Error while sending a block for validation.");
        }
         if(m_mode == Mode::PoW) m_nonce_confirmation_waited_for_clients.push_front({ipaddr, port});
    }
}

void Server::handleConfirmCorrectness(sf::Packet & packet, sf::IpAddress remoteAddress, std::uint16_t remotePort)
{
    std::uint32_t index;
    bool isValid;
    packet >> index >> isValid;

    if(m_mode == Mode::PoW && m_currentlyMinedBlock.nIndex != index) throw std::runtime_error("Error received wrong block.");

    if(!isValid && m_mode == Mode::PoW)
    {
        m_nonce_confirmation_waited_for_clients.clear();
        m_currentlyEnsuringCorrectness = false;
        sendBlockForValidation();
        return;
    }

    if(!isValid && m_mode == Mode::PoS)
    {
        while(m_blockchain.GetLastBlock().nIndex >= index)
        {
            m_currentlyMinedBlock = m_blockchain.popLastBlock();
        }
        if(m_currentlyMinedBlock.nIndex == index) sendBlockForValidation();
        return;
    }

    if(m_mode == Mode::PoW)
    {
        m_nonce_confirmation_waited_for_clients.remove({remoteAddress, remotePort});

        if(m_nonce_confirmation_waited_for_clients.empty() || m_nonce_confirmation_timer.getElapsedTime() > NONCE_CONFIRMATION_TIMEOUT)
        {
            endMining();
        }
    }
}

void Server::endMining()
{
    m_nonce_confirmation_waited_for_clients.clear();
    m_currentlyEnsuringCorrectness = false;

    m_currentlyMinedBlock.sHash = m_currentlyMinedBlock.CalculateHash();
    m_blockchain.AddBlock(m_currentlyMinedBlock);
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