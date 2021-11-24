#ifndef SRPF_BLOCK
#define SRPF_BLOCK

// selon l'article : https://davenash.com/2017/10/build-a-blockchain-with-c/
// cf : Blockchain-LICENSE.txt

#include <cstdint>
#include <iostream>

class Block
{
public:
    std::string sPrevHash;

    Block(std::uint32_t nIndexIn, const std::string &sDataIn);

    std::string GetHash();

    void MineBlock(std::uint32_t nDifficulty);

private:
    std::uint32_t _nIndex;
    std::int64_t  _nNonce;
    std::string   _sData;
    std::string   _sHash;
    std::time_t   _tTime;

    std::string _CalculateHash() const;
};

#endif // SRPF_BLOCK
