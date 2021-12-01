#include "Operation.hpp"

sf::Packet& operator>>(sf::Packet& in, Operation& op)
{
    std::uint8_t v;
    in >> v;
    op = Operation(v);
    return in;
}

sf::Packet& operator<<(sf::Packet& out, const Operation& op)
{
    return out << static_cast<std::uint8_t>(op);
}