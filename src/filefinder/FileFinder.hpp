#pragma once
#include "FileInfo.hpp"
#include <functional>
#include <vector>
#include <mutex>

namespace filefinder {

typedef bool(*FileFilterFunc_t)(const wchar_t *, const wchar_t *, u64);

class FileFinder
{
public:
    void setRootDirs(const std::vector<std::wstring>& rootdirs);
    void setMinimalFileSize(u64 minimumfsize);
    void setIgnoreDirs(const std::vector<std::wstring>& ignoredirs);
    void run(int threadcount, std::vector<std::wstring>& dirsout, std::vector<FileInfo>& filesout);
    u64 getSkippedSmallFileCount() const;
    void setFileFilter(FileFilterFunc_t f);

private:
    //thread safe for use in work():
    std::wstring getNextDir();
    void addDirs(const std::vector<std::wstring>& dirs);
    void addFiles(const std::vector<FileInfo>& files, u64 skipped);

    //worker function of the thread:
    void work();

    //input info
    std::vector<std::wstring> m_rootdirs;
    std::vector<std::wstring> m_ignoredirs; //make it a (hash)set if perf is ever a problem
    u64 m_minimumfsize = 0u;

    //mutex + output info
    std::mutex m_mutex;
    std::vector<std::wstring> m_dirs;
    std::vector<FileInfo> m_files;
    std::size_t m_dirit = 0u;
    u64 m_skippedsmallfilecount = 0u;
    FileFilterFunc_t m_filter = 0x0;

};

}
