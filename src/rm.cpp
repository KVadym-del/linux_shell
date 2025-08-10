#include <string_view>
#include <sys/types.h>
#include <unistd.h>
#include <vector>

#include "include/util.hpp"

int32_t main(int32_t argc, char* argv[])
{
    auto args = make_args(argc, argv);
    auto prog = prog_name(args[0]);

    if (!require_args(prog, args.size(), 2, "No file specified"))
        return 1;

    int32_t ret = 0;
    for (size_t i = 1; i < args.size(); ++i)
    {
        if (::unlink(args[i].data()) != 0)
        {
            print_errno(prog, "unlink", args[i]);
            ret = 1;
        }
    }
    return ret;
}