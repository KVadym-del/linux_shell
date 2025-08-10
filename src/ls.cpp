#include <dirent.h>
#include <string.h>
#include <string_view>
#include <sys/types.h>
#include <unistd.h>
#include <vector>

#include "include/util.hpp"

static inline void set_color(bool is_dir)
{
    if (is_dir)
        print("\033[1;34m");
    else
        print("\033[0m");
}

int32_t main(int32_t argc, char* argv[])
{
    auto args = make_args(argc, argv);
    auto prog = prog_name(args[0]);

    if (!require_args(prog, args.size(), 2, "No directory specified"))
        return 1;

    Dir dir(opendir(args[1].data()));
    if (!dir)
    {
        print_errno(prog, "opendir", args[1]);
        return 1;
    }

    while (dirent* entry = readdir(dir.get()))
    {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue;
        set_color(entry->d_type == DT_DIR);
        print(entry->d_name);
        print("\r\n\033[0m");
    }
    return 0;
}