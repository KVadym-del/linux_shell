#include <cerrno>
#include <csignal>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fcntl.h>
#include <string>
#include <sys/stat.h>
#include <sys/types.h>
#include <termios.h>
#include <unistd.h>
#include <vector>

#include "include/util.hpp"

static std::string fileName{};
static struct termios orig_termios{};
static bool raw_enabled = false;
static bool alt_screen = false;

constexpr char ctrl_key(char c)
{
    return c & 0x1f;
}
constexpr char CTRL_Q = ctrl_key('Q');
constexpr char CTRL_S = ctrl_key('S');
constexpr char BACKSPACE{0x7f};

static bool dirty{false};
static bool running{true};
static volatile sig_atomic_t got_signal = 0;

static std::vector<std::string> lines{""};
static size_t cx{0};
static size_t cy{0};

static void disable_raw();

static void die(const char* msg)
{
    disable_raw();
    print_error("edit ERROR: ");
    print_error(msg);
    print_error(": ");
    print_error(strerror(errno));
    print_error("\r\n");
    _exit(1);
}

static void disable_raw()
{
    if (raw_enabled)
    {
        tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios);
        raw_enabled = false;
    }
    if (alt_screen)
    {
        print("\x1b[?1049l");
        alt_screen = false;
    }
}

static void enable_raw()
{
    if (raw_enabled)
        return;
    if (tcgetattr(STDIN_FILENO, &orig_termios) == -1)
        die("tcgetattr");
    termios raw = orig_termios;
    raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
    raw.c_oflag &= ~(OPOST);
    raw.c_cflag |= (CS8);
    raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
    raw.c_cc[VMIN] = 1;
    raw.c_cc[VTIME] = 0;
    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1)
        die("tcsetattr");
    raw_enabled = true;
    if (!alt_screen)
    {
        print("\x1b[?1049h\x1b[H");
        alt_screen = true;
    }
}

static void handle_signal(int)
{
    got_signal = 1;
}

static void open_file()
{
    if (fileName.empty())
        return;
    int fd = open(fileName.c_str(), O_RDONLY);
    if (fd == -1)
    {
        lines.assign(1, "");
        cx = cy = 0;
        dirty = false;
        return;
    }
    lines.clear();
    std::string cur{};
    char buf[4096]{};
    ssize_t r{};
    while ((r = read(fd, buf, sizeof buf)) > 0)
    {
        for (ssize_t i = 0; i < r; ++i)
        {
            char ch = buf[i];
            if (ch == '\n')
            {
                lines.push_back(cur);
                cur.clear();
            }
            else if (ch != '\r')
            {
                cur.push_back(ch);
            }
        }
    }
    close(fd);
    if (!cur.empty() || lines.empty())
        lines.push_back(cur);
    cx = cy = 0;
    dirty = false;
}

static bool save_file()
{
    if (fileName.empty())
        return false;
    FD fd(open(fileName.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644));
    if (!fd)
        return false;
    if (lines.back() != "")
        lines.push_back("");
    for (size_t i = 0; i < lines.size(); ++i)
    {
        const std::string& l = lines[i];
        if (!l.empty())
        {
            if (!write_all(fd.get(), l.data(), l.size()))
                return false;
        }
        if (i + 1 < lines.size())
        {
            if (!write_all(fd.get(), "\n", 1))
                return false;
        }
    }
    dirty = false;
    return true;
}

static void editor_insert_char(char c)
{
    if (c == '\n')
    {
        std::string rest = lines[cy].substr(cx);
        lines[cy].erase(cx);
        lines.insert(lines.begin() + cy + 1, rest);
        cy++;
        cx = 0;
        dirty = true;
        return;
    }
    lines[cy].insert(lines[cy].begin() + static_cast<long>(cx), c);
    cx++;
    dirty = true;
}

static void editor_backspace()
{
    if (cx == 0)
    {
        if (cy == 0)
            return;
        size_t prevLen = lines[cy - 1].size();
        lines[cy - 1] += lines[cy];
        lines.erase(lines.begin() + cy);
        cy--;
        cx = prevLen;
        dirty = true;
        return;
    }
    lines[cy].erase(lines[cy].begin() + static_cast<long>(cx) - 1);
    cx--;
    dirty = true;
}

static void move_cursor(char dir)
{
    switch (dir)
    {
    case 'A': // up
        if (cy > 0)
        {
            cy--;
            if (cx > lines[cy].size())
                cx = lines[cy].size();
        }
        break;
    case 'B': // down
        if (cy + 1 < lines.size())
        {
            cy++;
            if (cx > lines[cy].size())
                cx = lines[cy].size();
        }
        break;
    case 'C': // right
        if (cx < lines[cy].size())
            cx++;
        else if (cy + 1 < lines.size())
        {
            cy++;
            cx = 0;
        }
        break;
    case 'D': // left
        if (cx > 0)
            cx--;
        else if (cy > 0)
        {
            cy--;
            cx = lines[cy].size();
        }
        break;
    }
}

static void refresh_screen()
{
    // Hide cursor
    print("\x1b[?25l");
    print("\x1b[H");

    for (const auto& l : lines)
    {
        print(l);
        print("\x1b[K\r\n");
    }

    // Status line (inverse video)
    print("\x1b[7m ");
    std::string status = "EDIT ";
    status += fileName.empty() ? "[No Name]" : fileName;
    if (dirty)
        status += "*";
    status += "  Ctrl-S=Save  Ctrl-Q=Quit";
    print(status);
    print("\x1b[m");
    print("\x1b[K\r\n");

    char buf[64];
    std::snprintf(buf, sizeof(buf), "\x1b[%zu;%zuH", cy + 1, cx + 1);
    print(buf);

    print("\x1b[?25h");
}

static void process_key()
{
    char c;
    ssize_t n = read(STDIN_FILENO, &c, 1);
    if (n <= 0)
        return;

    if (c == '\x1b')
    {
        char seq[2];
        if (read(STDIN_FILENO, &seq[0], 1) != 1)
            return;
        if (read(STDIN_FILENO, &seq[1], 1) != 1)
            return;
        if (seq[0] == '[')
        {
            move_cursor(seq[1]);
        }
        return;
    }

    switch (c)
    {
    case CTRL_Q:
        running = false;
        return;
    case CTRL_S:
        save_file();
        return;
    case BACKSPACE:
        editor_backspace();
        return;
    case '\r':
        editor_insert_char('\n');
        return;
    default:
        if (c >= 32 && c <= 126)
        {
            editor_insert_char(c);
        }
        break;
    }
}

int main(int argc, char* argv[])
{
    if (argc < 2)
    {
        print_error("ERROR: edit: No file specified\r\n");
        return 1;
    }
    fileName = argv[1];

    struct sigaction sa{};
    sa.sa_handler = handle_signal;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGINT, &sa, nullptr);
    sigaction(SIGTERM, &sa, nullptr);
    sigaction(SIGHUP, &sa, nullptr);

    enable_raw();
    open_file();

    while (running)
    {
        if (got_signal)
            running = false;
        refresh_screen();
        process_key();
    }

    disable_raw();
    return 0;
}