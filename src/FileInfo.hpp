#pragma once
#include <string>
#include <vector>
#include <cstdint>

typedef std::uint64_t u64;

class FileInfo
{
public:
    FileInfo(const std::wstring fname, u64 fsize) : fname(fname), fsize(fsize) {}

    std::wstring fname;
    u64 fsize;

};
