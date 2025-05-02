#include <cstddef>
#include <cstring>
#include <stdio.h>
#include <string>
#include <string_view>
#include <sys/wait.h>
#include <termios.h>
#include <unistd.h>
#include <vector>

extern "C"
{
    int real_waitid(idtype_t idtype, id_t id, siginfo_t* infop, int options, void*);
}

constexpr const size_t PREALLOC_COMMAND_SIZE = 255;
constexpr const size_t MAX_HISTORY = 100;

constexpr const char* PROMPT_PRELUDE = "#> ";
constexpr const char* LEDT_ARROW = "\033[D";
constexpr const char* RIGHT_ARROW = "\033[C";

static std::string g_promt{PROMPT_PRELUDE};

struct termios g_orig_termios{};

static std::vector<std::string> g_history{};
static size_t g_history_index{0};

inline static void print(std::string_view str)
{
    size_t res = write(1, str.data(), str.size());
    if (res == -1)
        _exit(1);
}

inline static void print_error(std::string_view str)
{
    size_t res = write(2, str.data(), str.size());
    if (res == -1)
        _exit(1);
}

inline static void die(std::string_view str)
{
    print_error(str);
    _exit(1);
}

inline static void disable_raw_mode()
{
    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &g_orig_termios) == -1)
        die("tcsetattr");

    print("\r\n");
}

inline static void add_history(std::string_view command)
{
    if (command.empty())
        return;

    if (g_history.size() > 0 && g_history[g_history.size() - 1] == command)
        return;

    if (g_history.size() >= MAX_HISTORY)
        g_history.erase(g_history.begin());

    g_history.push_back(command.data());
    g_history_index = g_history.size();
}

inline static void clear_line()
{
    print("\r\033[K");
}

int main(void)
{
    std::string command{};
    command.reserve(PREALLOC_COMMAND_SIZE);
    while (true)
    {
        command.clear();
        size_t res = read(0, command.data(), command.capacity());
        if (res == -1)
            return 1;

        print(g_promt);
        if (res > 0 && command[res - 1] == '\n')
            command[res - 1] = '\0';
        else
            command[res] = '\0';

        if (0 == std::strcmp(command.c_str(), LEDT_ARROW))
        {
            if (!g_history.empty() && g_history_index > 0)
            {
                g_history_index--;
                std::string const& hist_cmd = g_history[g_history_index];
                print("\r");
                print(g_promt);
                print(hist_cmd);
                print("\n");
            }
            continue;
        }
        else if (0 == std::strcmp(command.c_str(), RIGHT_ARROW))
        {
            if (!g_history.empty() && g_history_index + 1 < g_history.size())
            {
                g_history_index++;
                std::string const& hist_cmd = g_history[g_history_index];
                print("\r");
                print(g_promt);
                print(hist_cmd);
                print("\n");
            }
            continue;
        }
        else if (std::strlen(command.c_str()) > 0)
        {
            g_history.push_back(command.c_str());
            g_history_index = g_history.size();
        }

        if (0 == std::strcmp(command.c_str(), "exit"))
            _exit(0);

        pid_t pid = fork();
        if (pid == 0)
        {
            execve(command.c_str(), nullptr, nullptr);
            _exit(0);
        }
        else
        {
            siginfo_t info{};
            real_waitid(P_ALL, 0, &info, WEXITED, nullptr);
        }
    }

    _exit(0);
}
