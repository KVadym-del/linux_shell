#include <fcntl.h>
#include <string.h>
#include <string_view>
#include <vector>

#include "include/util.hpp"

int main(int argc, char* argv[])
{
    std::vector<std::string_view> args(argv, argv + argc);

    if (args.size() < 2)
    {
        print_error("ERROR: rm: No file specified\r\n");
        return 1;
    }

    for (size_t i = 1; i < args.size(); ++i)
    {
        if (unlink(args[i].data()) != 0)
        {
            print_error("ERROR: rm '");
            print_error(args[i]);
            print_error("': ");
            print_error(strerror(errno));
            print_error("\r\n");
        }
        else
        {
            print_error("SUCCESS: Removed '");
            print_error(args[i]);
            print_error("'\r\n");
        }
    }
}