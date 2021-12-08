#include "Block.hpp"
#include "sha256.h"

// selon l'article : https://davenash.com/2017/10/build-a-blockchain-with-c/
// cf : Blockchain-LICENSE.txt

#include <sstream>

Block::Block(const std::string &sDataIn) :
    sData(sDataIn)
{
    nNonce = -1;
    tTime = std::time(nullptr);
}

std::string Block::CalculateHash() const
{
    std::stringstream ss;
    ss << nIndex << tTime << sData << nNonce << sPrevHash;

    return sha256(ss.str());
}

sf::Packet& operator>>(sf::Packet& in, Block& block)
{
    return in
        >> block.sPrevHash
        >> block.nIndex
        >> block.nNonce
        >> block.sData
        >> block.sHash
        >> block.tTime;
}

sf::Packet& operator<<(sf::Packet& out, const Block& block)
{
    return out
        << block.sPrevHash
        << block.nIndex
        << block.nNonce
        << block.sData
        << block.sHash
        << block.tTime;
}