#include <dirent.h>
#include <errno.h>
#include <string.h>
#include <string_view>
#include <unistd.h>
#include <vector>

#include "include/util.hpp"

int main(int argc, char* argv[])
{
    std::vector<std::string_view> args(argv, argv + argc);

    if (args.size() < 2)
    {
        print_error("ERROR: ls: No directory specified\r\n");
        return 1;
    }

    DIR* dir = opendir(args[1].data());
    if (dir == nullptr)
    {
        print_error("ERROR: ls: Unable to open directory '");
        print_error(args[1]);
        print_error("': ");
        print_error(strerror(errno));
        print_error("\r\n");
        return 1;
    }

    struct dirent* entry;
    while ((entry = readdir(dir)) != nullptr)
    {
        if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0)
        {
            if (entry->d_type == DT_DIR)
            {
                print("\033[1;34m");
            }
            else
            {
                print("\033[0m");
            }
            print(entry->d_name);
            print("\r\n\033[0m");
        }
    }

    if (closedir(dir) == -1)
    {
        print_error("ERROR: ls: Unable to close directory '");
        print_error(args[1]);
        print_error("': ");
        print_error(strerror(errno));
        print_error("\r\n");
    }

    return 0;
}