#include "stdafx.h"
std::vector<uint8_t> FileContents(const std::wstring& filename)
{
    std::ifstream file(to_string(filename),std::ios::binary);
    if (!file) throw std::runtime_error("File not found: "+to_string(filename));
    return {std::istreambuf_iterator<char>(file),std::istreambuf_iterator<char>()};
}
std::mt19937& random_source()
{
    static std::mt19937 rng(std::random_device{}()); return rng;
}
uint32_t random_uint32() { return random_source()(); }
