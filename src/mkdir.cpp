#include <string_view>
#include <sys/stat.h>
#include <vector>

#include "include/util.hpp"

int main(int argc, char* argv[])
{
    auto args = make_args(argc, argv);
    auto prog = prog_name(args[0]);

    if (!require_args(prog, args.size(), 2, "No directory specified"))
        return 1;

    if (::mkdir(args[1].data(), 0755) != 0)
    {
        print_errno(prog, "mkdir", args[1]);
        return 1;
    }
    return 0;
}