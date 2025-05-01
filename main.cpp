#include <cstring>
#include <string>
#include <sys/wait.h>
#include <unistd.h>

extern "C"
{
    int real_waitid(idtype_t idtype, id_t id, siginfo_t* infop, int options, void*);
}

constexpr const size_t PREALLOC_COMMAND_SIZE = 255;
constexpr const char* PROMPT = "#> ";

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

        pid_t pid = fork();
        if (pid == 0)
        {
            execve(command.c_str(), nullptr, nullptr);
            break;
        }
        else
        {
            siginfo_t info{};
            real_waitid(P_ALL, 0, &info, WEXITED, nullptr);
        }
    }

    _exit(0);
}
