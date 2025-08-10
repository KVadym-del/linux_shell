#ifndef UTIL_HPP
#define UTIL_HPP

#include <cerrno>
#include <dirent.h>
#include <string.h>
#include <string_view>
#include <unistd.h>
#include <vector>

inline void print(std::string_view str)
{
    if (write(STDOUT_FILENO, str.data(), str.size()) == -1 && errno != EINTR)
        _exit(1);
}

inline void print_error(std::string_view str)
{
    if (write(STDERR_FILENO, str.data(), str.size()) == -1 && errno != EINTR)
        _exit(1);
}

inline void clear_line()
{
    print("\r\033[K");
}

inline void clear_screen()
{
    print("\033[H\033[J");
}

inline std::vector<std::string_view> make_args(int argc, char* argv[])
{
    return {argv, argv + argc};
}
inline std::vector<std::string_view> make_args(int argc, const char* argv[])
{
    return {argv, argv + argc};
}

inline std::string_view prog_name(std::string_view full)
{
    size_t pos = full.rfind('/');
    return (pos == std::string_view::npos) ? full : full.substr(pos + 1);
}

inline bool require_args(std::string_view prog, size_t have, size_t need, std::string_view msg)
{
    if (have >= need)
        return true;
    print_error("ERROR: ");
    print_error(prog);
    print_error(": ");
    print_error(msg);
    print_error("\r\n");
    return false;
}

inline void print_errno(std::string_view prog, std::string_view action, std::string_view target)
{
    print_error("ERROR: ");
    print_error(prog);
    if (!action.empty())
    {
        print_error(": ");
        print_error(action);
    }
    if (!target.empty())
    {
        print_error(" '");
        print_error(target);
        print_error("'");
    }
    print_error(": ");
    print_error(strerror(errno));
    print_error("\r\n");
}

inline bool write_all(int fd, const void* data, size_t len)
{
    const char* p = static_cast<const char*>(data);
    while (len > 0)
    {
        ssize_t w = ::write(fd, p, len);
        if (w < 0)
        {
            if (errno == EINTR)
                continue;
            return false;
        }
        p += static_cast<size_t>(w);
        len -= static_cast<size_t>(w);
    }
    return true;
}

struct FD
{
    int fd{-1};
    FD() = default;
    explicit FD(int f) : fd(f)
    {
    }
    ~FD()
    {
        if (fd != -1)
            ::close(fd);
    }
    FD(const FD&) = delete;
    FD& operator=(const FD&) = delete;
    FD(FD&& other) noexcept : fd(other.fd)
    {
        other.fd = -1;
    }
    FD& operator=(FD&& other) noexcept
    {
        if (this != &other)
        {
            if (fd != -1)
                ::close(fd);
            fd = other.fd;
            other.fd = -1;
        }
        return *this;
    }
    int get() const
    {
        return fd;
    }
    int release()
    {
        int t = fd;
        fd = -1;
        return t;
    }
    explicit operator bool() const
    {
        return fd != -1;
    }
};

struct Dir
{
    DIR* d{nullptr};
    Dir() = default;
    explicit Dir(DIR* dir) : d(dir)
    {
    }
    ~Dir()
    {
        if (d)
            ::closedir(d);
    }
    Dir(const Dir&) = delete;
    Dir& operator=(const Dir&) = delete;
    Dir(Dir&& o) noexcept : d(o.d)
    {
        o.d = nullptr;
    }
    Dir& operator=(Dir&& o) noexcept
    {
        if (this != &o)
        {
            if (d)
                ::closedir(d);
            d = o.d;
            o.d = nullptr;
        }
        return *this;
    }
    DIR* get() const
    {
        return d;
    }
    explicit operator bool() const
    {
        return d != nullptr;
    }
};

#endif // UTIL_HPP