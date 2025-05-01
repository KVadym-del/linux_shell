#include <cstring>
#include <stdio.h>
#include <string>
#include <sys/wait.h>
#include <unistd.h>
#include <vector>

extern "C"
{
    int real_waitid(idtype_t idtype, id_t id, siginfo_t* infop, int options, void*);
}

constexpr const size_t PREALLOC_COMMAND_SIZE = 255;

constexpr const char* PROMPT = "#> ";
constexpr const char* UP_ARROW = "\033[A";
constexpr const char* DOWN_ARROW = "\033[B";

static std::vector<std::string> g_history;

int main(void)
{
    std::string command{};
    command.reserve(PREALLOC_COMMAND_SIZE);
    while (true)
    {
        size_t res = write(1, PROMPT, std::strlen(PROMPT));
        if (res == -1)
            return 1;

        res = read(0, command.data(), command.capacity());
        if (res == -1)
            return 1;

        if (res > 0 && command[res - 1] == '\n')
            command[res - 1] = '\0';
        else
            command[res] = '\0';

        g_history.push_back(command);

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
