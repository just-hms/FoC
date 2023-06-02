#include <functional>
#include <iostream>
#include <string>
#include <ncurses.h>
#include <unistd.h>

#include "./../entity/entity.h"

namespace cli {
    struct Command{
        std::function<void()> cmd;
        std::string name;
    };

    class Prompt{
    private:
        void printMenu(uint index);
        void runCommand(int index);
    public:
        std::vector<Command> commands;
        Prompt();
        void Run();
    };
}