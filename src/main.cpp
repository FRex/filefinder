#include "filefinder/FileFinder.hpp"

int wmain(int argc, wchar_t ** argv)
{
    filefinder::FileFinder finder;
    finder.setRootDirs({ argv[1] });
    finder.setIgnoreDirs({ L"C:/Windows" });

    std::vector<std::wstring> dirs;
    std::vector<filefinder::FileInfo> files;

    finder.run(8, dirs, files);

    wprintf(L"file count = %lld\n", (long long)dirs.size());
    wprintf(L"file count = %lld\n", (long long)files.size());
}
