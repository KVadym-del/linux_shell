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
        print_error("ERROR: touch: No file specified\r\n");
        return 1;
    }

    int fd = open(args[1].data(), O_CREAT | O_WRONLY | O_TRUNC, 0644);
    if (fd == -1)
    {
        print_error("ERROR: touch '");
        print_error(args[1]);
        print_error("': ");
        print_error(strerror(errno));
        print_error("\r\n");
        return 1;
    }

    if (close(fd) == -1)
    {
        print_error("ERROR: touch '");
        print_error(args[1]);
        print_error("': ");
        print_error(strerror(errno));
        print_error("\r\n");
        return 1;
    }
}