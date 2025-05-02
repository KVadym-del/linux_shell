#include <cerrno>
#include <string_view>
#include <unistd.h>

inline static void print(std::string_view str)
{
    if (write(STDOUT_FILENO, str.data(), str.size()) == -1 && errno != EINTR)
        _exit(1);
}

inline static void print_error(std::string_view str)
{
    if (write(STDERR_FILENO, str.data(), str.size()) == -1 && errno != EINTR)
        _exit(1);
}

inline static void clear_line()
{
    print("\r\033[K");
}

inline static void clear_screen()
{
    print("\033[H\033[J");
}