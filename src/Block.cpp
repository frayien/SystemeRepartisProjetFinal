#include "Block.hpp"
#include "sha256.h"

// selon l'article : https://davenash.com/2017/10/build-a-blockchain-with-c/
// cf : Blockchain-LICENSE.txt

#include <sstream>

Block::Block(std::uint32_t nIndexIn, const std::string &sDataIn) : 
    _nIndex(nIndexIn),
    _sData(sDataIn)
{
    _nNonce = -1;
    _tTime = std::time(nullptr);
}

std::string Block::GetHash()
{
    return _sHash;
}

void Block::MineBlock(std::uint32_t nDifficulty)
{
    std::string str(nDifficulty, '0');

    do
    {
        _nNonce++;
        _sHash = _CalculateHash();
    }
    while (_sHash.substr(0, nDifficulty) != str);

    std::cout << "Block mined: " << _sHash << std::endl;
}

inline std::string Block::_CalculateHash() const
{
    std::stringstream ss;
    ss << _nIndex << _tTime << _sData << _nNonce << sPrevHash;

    return sha256(ss.str());
}