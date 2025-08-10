#include <fcntl.h>
#include <string_view>
#include <vector>

#include "include/util.hpp"

int main(int argc, char* argv[])
{
    auto args = make_args(argc, argv);
    auto prog = prog_name(args[0]);

    if (!require_args(prog, args.size(), 2, "No file specified"))
        return 1;

    FD fd(open(args[1].data(), O_CREAT | O_WRONLY | O_TRUNC, 0644));
    if (!fd)
    {
        print_errno(prog, "open", args[1]);
        return 1;
    }
    return 0;
}