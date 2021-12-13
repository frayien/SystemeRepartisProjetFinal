#ifndef SRPF_OPERATION
#define SRPF_OPERATION

#include <SFML/Network/Packet.hpp>

#include <cstdint>
#include <string>

enum class Operation : std::uint8_t
{
    NO_OP = 0,
    CONNECT = 1,
    DISCONNECT = 2,
    TRANSACTION = 3,
    REQUEST_VALIDATION = 4,
    FOUND_NONCE = 5,
    ENSURE_CORRECTNESS = 6,
    CONFIRM_CORRECTNESS = 7,
    END_MINING = 8,
    OK = 10,
    ERROR = 11,
};

sf::Packet& operator>>(sf::Packet& in, Operation& op);
sf::Packet& operator<<(sf::Packet& out, const Operation& op);

std::string to_string(Operation op);

#endif // SRPF_OPERATION
