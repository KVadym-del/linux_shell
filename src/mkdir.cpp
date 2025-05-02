#include <dirent.h>
#include <errno.h>
#include <string.h>
#include <string_view>
#include <sys/stat.h>
#include <vector>

#include "include/util.hpp"

int main(int argc, char* argv[])
{
    std::vector<std::string_view> args(argv, argv + argc);

    if (args.size() < 2)
    {
        print_error("ERROR: mkdir: No directory specified\r\n");
        return 1;
    }

    if (mkdir(args[1].data(), 0755) != 0)
    {
        print_error("ERROR: mkdir '");
        print_error(args[1]);
        print_error("': ");
        print_error(strerror(errno));
        print_error("\r\n");
        return 1;
    }
}