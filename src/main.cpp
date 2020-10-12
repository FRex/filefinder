#include "FileFinder.hpp"

int wmain(int argc, wchar_t ** argv)
{
    FileFinder finder;
    finder.setRootDirs({ L"C:", L"D:", L"F:" });
    finder.setIgnoreDirs({ L"C:/Windows" });

    std::vector<std::wstring> dirs;
    std::vector<FileInfo> files;

    finder.run(8, dirs, files);

    wprintf(L"file count = %lld\n", (long long)dirs.size());
    wprintf(L"file count = %lld\n", (long long)files.size());
}
