#ifndef SRPF_OPERATION
#define SRPF_OPERATION

#include <SFML/Network/Packet.hpp>

#include <cstdint>

enum class Operation : std::uint8_t
{
    NO_OP = 0,
    CONNECT = 1,
    DISCONNECT = 2,
    TRANSACTION = 3,
    OK = 10,
    ERROR = 11,
};

sf::Packet& operator>>(sf::Packet& in, Operation& op);
sf::Packet& operator<<(sf::Packet& out, const Operation& op);

#endif // SRPF_OPERATION
