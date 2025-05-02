#include <cctype>
#include <csignal>
#include <cstddef>
#include <cstdlib>
#include <cstring>
#include <dirent.h>
#include <errno.h>
#include <string>
#include <string_view>
#include <sys/wait.h>
#include <termios.h>
#include <unistd.h>
#include <vector>

constexpr const size_t PREALLOC_COMMAND_SIZE = 255;
constexpr const size_t MAX_HISTORY = 100;
constexpr const char* PROMPT_PRELUDE = "#> ";

struct termios g_orig_termios{};

static std::vector<std::string> g_history{};
static size_t g_history_index{0};

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

inline static void disable_raw_mode();

inline static void die(std::string_view s)
{
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &g_orig_termios);
    print("\r\n");
    print_error("ERROR: ");
    print_error(s);
    print_error(": ");
    print_error(strerror(errno));
    print_error("\r\n");
    exit(1);
}

inline static void enable_raw_mode()
{
    if (tcgetattr(STDIN_FILENO, &g_orig_termios) == -1)
        die("tcgetattr");
    atexit(disable_raw_mode);

    struct termios raw{g_orig_termios};
    raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
    raw.c_oflag &= ~(OPOST);
    raw.c_cflag |= (CS8);
    raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
    raw.c_cc[VMIN] = 1;
    raw.c_cc[VTIME] = 0;

    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1)
        die("tcsetattr raw");
}

inline static void disable_raw_mode()
{
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &g_orig_termios);
    print("\r\n");
}

inline static void add_history(const std::string& command)
{
    if (command.empty())
        return;
    if (!g_history.empty() && g_history.back() == command)
    {
        g_history_index = g_history.size();
        return;
    }
    if (g_history.size() >= MAX_HISTORY)
    {
        g_history.erase(g_history.begin());
    }
    g_history.push_back(command);
    g_history_index = g_history.size();
}

inline static void clear_line()
{
    print("\r\033[K");
}

inline static void clear_screen()
{
    print("\033[H\033[J");
}

inline static void handle_cd(const std::vector<std::string>& args)
{
    if (args.size() < 2)
    {

        const char* home = getenv("HOME");
        if (home == nullptr)
        {
            print_error("ERROR: cd: HOME not set\r\n");
        }
        else if (chdir(home) != 0)
        {
            print_error("ERROR: cd home: ");
            print_error(strerror(errno));
            print_error("\r\n");
        }
    }
    else
    {
        if (chdir(args[1].c_str()) != 0)
        {
            print_error("ERROR: cd '");
            print_error(args[1]);
            print_error("': ");
            print_error(strerror(errno));
            print_error("\r\n");
        }
    }
}

inline static void handle_ls(const std::vector<std::string>& args)
{
    if (args.size() < 2)
    {
        print_error("ERROR: ls: No directory specified\r\n");
        return;
    }

    DIR* dir = opendir(args[1].c_str());
    if (dir == nullptr)
    {
        print_error("ERROR: ls: Unable to open directory '");
        print_error(args[1]);
        print_error("': ");
        print_error(strerror(errno));
        print_error("\r\n");
        return;
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
}

inline static void execute_command(const std::string& command_str)
{
    if (command_str.empty())
        return;

    if (command_str == "exit")
    {
        exit(0);
    }

    std::vector<std::string> args_storage;
    std::string current_arg;
    for (char c : command_str)
    {
        if (isspace(static_cast<unsigned char>(c)))
        {
            if (!current_arg.empty())
            {
                args_storage.push_back(current_arg);
                current_arg.clear();
            }
        }
        else
        {
            current_arg += c;
        }
    }
    if (!current_arg.empty())
    {
        args_storage.push_back(current_arg);
    }

    if (args_storage.empty())
        return;

    if (args_storage[0] == "cd")
    {
        handle_cd(args_storage);
        return;
    }
    else if (args_storage[0] == "ls")
    {
        handle_ls(args_storage);
        return;
    }
    else if (args_storage[0] == "history")
    {
        for (const auto& hist : g_history)
        {
            print(hist);
            print("\r\n");
        }
        return;
    }
    else if (args_storage[0] == "clear")
    {
        clear_screen();
        return;
    }

    std::vector<char*> argv;
    for (const auto& arg : args_storage)
    {
        argv.push_back(const_cast<char*>(arg.c_str()));
    }
    argv.push_back(nullptr);

    pid_t pid = fork();
    if (pid == -1)
    {
        print_error("ERROR: fork: ");
        print_error(strerror(errno));
        print_error("\r\n");
        return;
    }
    else if (pid == 0)
    {

        signal(SIGINT, SIG_DFL);
        signal(SIGTSTP, SIG_DFL);
        if (execvp(argv[0], argv.data()) == -1)
        {
            print_error("ERROR: execvp '");
            print_error(argv[0]);
            print_error("': ");
            print_error(strerror(errno));
            print_error("\r\n");
            _exit(1);
        }
    }
    else
    {

        int status;
        if (waitpid(pid, &status, 0) == -1)
        {
            if (errno != EINTR)
            {
                print_error("ERROR: waitpid: ");
                print_error(strerror(errno));
                print_error("\r\n");
            }
        }
    }
}

int main(void)
{
    std::string command{};
    command.reserve(PREALLOC_COMMAND_SIZE);

    enable_raw_mode();

    while (true)
    {
        clear_line();
        print(PROMPT_PRELUDE);
        print(command);

        char c = '\0';
        ssize_t nread = read(STDIN_FILENO, &c, 1);

        if (nread == -1)
        {
            if (errno == EINTR)
                continue;
            die("read");
        }
        if (nread == 0)
        {
            break;
        }

        switch (c)
        {
        case '\r':
        case '\n':
            print("\r\n");
            tcsetattr(STDIN_FILENO, TCSAFLUSH, &g_orig_termios);
            if (!command.empty())
            {
                add_history(command);
                execute_command(command);
            }
            enable_raw_mode();
            command.clear();
            g_history_index = g_history.size();
            break;

        case '\x7f':
            if (!command.empty())
            {
                command.pop_back();
                print("\b \b");
            }
            break;

        case '\x03':
            print("^C\r\n");
            command.clear();
            g_history_index = g_history.size();
            break;

        case '\x04':
            if (command.empty())
            {
                goto exit_loop;
            }
            break;

        case '\x1b': {
            char seq[3];
            if (read(STDIN_FILENO, &seq[0], 1) != 1)
                continue;
            if (read(STDIN_FILENO, &seq[1], 1) != 1)
                continue;

            if (seq[0] == '[')
            {
                switch (seq[1])
                {
                case 'A':
                    if (g_history_index > 0)
                    {
                        g_history_index--;
                        command = g_history[g_history_index];
                    }
                    break;
                case 'B':
                    if (g_history_index < g_history.size())
                    {
                        g_history_index++;
                        if (g_history_index == g_history.size())
                        {
                            command.clear();
                        }
                        else
                        {
                            command = g_history[g_history_index];
                        }
                    }
                    break;
                }
            }
            break;
        }

        default:
            if (isprint(static_cast<unsigned char>(c)))
            {
                command += c;
                if (write(STDOUT_FILENO, &c, 1) == -1 && errno != EINTR)
                {
                    die("write echo");
                }
            }
            break;
        }
    }

exit_loop:

    return 0;
}