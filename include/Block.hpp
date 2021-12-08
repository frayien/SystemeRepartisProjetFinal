#ifndef SRPF_BLOCK
#define SRPF_BLOCK

// selon l'article : https://davenash.com/2017/10/build-a-blockchain-with-c/
// cf : Blockchain-LICENSE.txt

#include <cstdint>
#include <iostream>

#include <SFML/Network/Packet.hpp>

class Block
{
public:
    std::string sPrevHash;
    std::uint32_t nIndex;
    std::int64_t  nNonce;
    std::string   sData;
    std::string   sHash;
    std::time_t   tTime;

    Block() = default;
    Block(const std::string &sDataIn);

    friend sf::Packet& operator>>(sf::Packet& in, Block& block);
    friend sf::Packet& operator<<(sf::Packet& out, const Block& block);

    std::string CalculateHash() const;
};

#endif // SRPF_BLOCK
