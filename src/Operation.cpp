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

std::string to_string(Operation op)
{
    switch (op)
    {
    case Operation::NO_OP:              return "No Operation";
    case Operation::CONNECT:            return "Connection";
    case Operation::DISCONNECT:         return "Disconnection";
    case Operation::TRANSACTION:        return "Transaction";
    case Operation::REQUEST_VALIDATION: return "Request Validation";
    case Operation::VALID_BLOCK:        return "Return Valid Block";
    case Operation::END_MINING:         return "End Mining";
    case Operation::OK:                 return "Ok";
    case Operation::ERROR:              return "Error";
    default:                            return "Unknown Operation";
    }
}