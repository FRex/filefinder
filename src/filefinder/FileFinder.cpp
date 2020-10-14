#include "FileFinder.hpp"
#include <Windows.h>
#include <thread>
#include <chrono>

namespace filefinder {

void FileFinder::setRootDirs(const std::vector<std::wstring>& rootdirs)
{
    m_rootdirs = rootdirs;
}

void FileFinder::setMinimalFileSize(u64 minimumfsize)
{
    m_minimumfsize = minimumfsize;
}

void FileFinder::setIgnoreDirs(const std::vector<std::wstring>& ignoredirs)
{
    m_ignoredirs = ignoredirs;
}

void FileFinder::run(int threadcount, std::vector<std::wstring>& dirsout, std::vector<FileInfo>& filesout)
{
    m_dirs = m_rootdirs;
    m_dirit = 0u;

    std::vector<std::thread> threads;
    threads.reserve(threadcount);
    for(int i = 0; i < threadcount; ++i)
        threads.emplace_back(&FileFinder::work, this);

    for(auto& t : threads)
        t.join();

    dirsout.clear();
    filesout.clear();
    dirsout.swap(m_dirs);
    filesout.swap(m_files);
}

u64 FileFinder::getSkippedSmallFileCount() const
{
    return m_skippedsmallfilecount;
}

std::wstring FileFinder::getNextDir()
{
    std::lock_guard<std::mutex> lock(m_mutex);
    while(m_dirit < m_dirs.size())
    {
        const std::wstring ret = m_dirs[m_dirit++];
        const auto fb = m_ignoredirs.begin();
        const auto fe = m_ignoredirs.end();
        if(std::find(fb, fe, ret) == fe)
            return ret;
    }

    return L"";
}

void FileFinder::addDirs(const std::vector<std::wstring>& dirs)
{
    if(dirs.empty())
        return;

    std::lock_guard<std::mutex> lock(m_mutex);
    for(const std::wstring& d : dirs) // TODO: resize + range assign?
        m_dirs.push_back(d);
}

void FileFinder::addFiles(const std::vector<FileInfo>& files, u64 skipped)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    m_skippedsmallfilecount += skipped;
    for(const FileInfo& f : files) // TODO: resize + range assign?
        m_files.push_back(f);
}

static inline bool isDirFileData(const WIN32_FIND_DATAW& f)
{
    return (f.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY);
}

static inline bool isDotOrDoubleDot(const WIN32_FIND_DATAW& f)
{
    if(0 == wcscmp(f.cFileName, L".")) return true;
    if(0 == wcscmp(f.cFileName, L"..")) return true;
    return false;
}

static inline u64 fsizeFromFileData(const WIN32_FIND_DATAW& f)
{
    const u64 high = f.nFileSizeHigh;
    const u64 low = f.nFileSizeLow;
    const u64 maxdword = MAXDWORD;
    return (high * (maxdword + 1)) + low;
}

static u64 listDir(const std::wstring& dir, std::vector<FileInfo>& files, std::vector<std::wstring>& dirs, u64 minfsize)
{
    WIN32_FIND_DATAW data;
    HANDLE h = FindFirstFileW((dir + L"\\*").c_str(), &data);
    if(h == INVALID_HANDLE_VALUE) //what happened ???
        return 0u;

    u64 retskipped = 0u;
    while(1)
    {
        const auto fname = dir + L"/" + data.cFileName;
        if(!isDotOrDoubleDot(data))
        {
            if(isDirFileData(data))
            {
                dirs.push_back(fname);
            }
            else
            {
                const u64 fsize = fsizeFromFileData(data);
                if(fsize < minfsize)
                    ++retskipped;
                else
                    files.push_back(FileInfo(fname, fsize));
            }
        }//if not dot or 2 dots

        if(!FindNextFileW(h, &data)) //check if get last error is no more files ??
            break;
    }//while 1

    FindClose(h); //check for error too?
    return retskipped;
}

void FileFinder::work()
{
    int failcount = 0;
    u64 skipped = 0;
    std::vector<FileInfo> ret;
    while(1)
    {
        const std::wstring dir = getNextDir();
        std::vector<std::wstring> dirs;

        if(dir.empty())
        {
            ++failcount;
            if(failcount > 15)
                break;

            using namespace std::chrono_literals;
            std::this_thread::sleep_for(5ms);
            //TODO: cond vars instead and stop each thread if N/N threads all ran of dirs at once
            continue;
        }//if dir

        skipped += listDir(dir, ret, dirs, m_minimumfsize);
        addDirs(dirs);
    }//while 1

    addFiles(ret, skipped);
}

}
