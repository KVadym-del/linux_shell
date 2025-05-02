#include <cstddef>
#include <cstring>
#include <stdio.h>
#include <string>
#include <string_view>
#include <sys/wait.h>
#include <unistd.h>
#include <vector>

extern "C"
{
    int real_waitid(idtype_t idtype, id_t id, siginfo_t* infop, int options, void*);
}

constexpr const size_t PREALLOC_COMMAND_SIZE = 255;

constexpr const char* PROMPT_PRELUDE = "#> ";
constexpr const char* LEDT_ARROW = "\033[D";
constexpr const char* RIGHT_ARROW = "\033[C";

static std::string g_promt{PROMPT_PRELUDE};

static std::vector<std::string> g_history{};
static size_t g_history_index{0};

static void print(std::string_view str)
{
    size_t res = write(1, str.data(), str.size());
    if (res == -1)
        _exit(1);
}

int main(void)
{
    std::string command{};
    command.reserve(PREALLOC_COMMAND_SIZE);
    while (true)
    {
        print(g_promt);

        command.clear();
        size_t res = read(0, command.data(), command.capacity());
        if (res == -1)
            return 1;

        print(command.c_str());
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
