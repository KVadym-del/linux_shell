#include <cstring>
#include <fcntl.h>
#include <string_view>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <vector>

#include "include/util.hpp"

constexpr size_t PREALLOC_BUFFER_SIZE = 4096;

static bool cat_fd(int fd, std::string_view name, std::vector<char>& buffer)
{
    if (fd < 0)
        return false;

    struct stat st{};
    if (fstat(fd, &st) == 0 && S_ISDIR(st.st_mode))
    {
        print_error("cat: '");
        print_error(name);
        print_error("': Is a directory\r\n");
        return false;
    }

    if (buffer.empty())
        buffer.resize(PREALLOC_BUFFER_SIZE);

    bool ok = true;
    while (true)
    {
        ssize_t r = read(fd, buffer.data(), buffer.size());
        if (r == 0)
            break; // EOF
        if (r < 0)
        {
            if (errno == EINTR)
                continue;
            print_error("cat: error reading '");
            print_error(name);
            print_error("': ");
            print_error(strerror(errno));
            print_error("\r\n");
            ok = false;
            break;
        }
        if (!write_all(STDOUT_FILENO, buffer.data(), static_cast<size_t>(r)))
        {
            print_error("cat: write error: ");
            print_error(strerror(errno));
            print_error("\r\n");
            ok = false;
            break;
        }
    }
    return ok;
}

static bool cat_file(std::string_view path, std::vector<char>& buffer)
{
    if (path == "-")
        return cat_fd(STDIN_FILENO, path, buffer);

    FD fd(open(path.data(), O_RDONLY | O_CLOEXEC));
    if (!fd)
    {
        print_error("cat: cannot open '");
        print_error(path);
        print_error("': ");
        print_error(strerror(errno));
        print_error("\r\n");
        return false;
    }
    return cat_fd(fd.get(), path, buffer);
}

int main(int argc, const char** argv)
{
    auto args = make_args(argc, argv);
    std::vector<char> buffer;
    buffer.reserve(PREALLOC_BUFFER_SIZE);

    if (args.size() == 1)
    {
        return cat_fd(STDIN_FILENO, "-", buffer) ? 0 : 1;
    }

    bool all_ok = true;
    for (size_t i = 1; i < args.size(); ++i)
    {
        if (!cat_file(args[i], buffer))
            all_ok = false;
    }
    return all_ok ? 0 : 1;
}