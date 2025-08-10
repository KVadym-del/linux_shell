#include <string_view>
#include <unistd.h>
#include <vector>

#include "include/util.hpp"

int main(int argc, char* argv[])
{
    auto args = make_args(argc, argv);
    auto prog = prog_name(args[0]);

    if (!require_args(prog, args.size(), 2, "No file specified"))
        return 1;

    int ret = 0;
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