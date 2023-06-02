#include "cli.h"
#include <exception>

cli::Prompt::Prompt() {}

void cli::Prompt::printMenu(uint index) {
    std::cout << "Use arrows to move across the options, press enter to confirm and q to quit\r\n";
    for (uint i = 0; i < commands.size(); i++) {
        std::cout << ((i == index) ? "●" : "○") << " " << commands[i].name << "\r\n";
    }
}

void cli::Prompt::runCommand(int index) {

    // resetTerminal
    refresh();
    endwin();
    system("clear");

    // launch command
    try {
        commands[index].cmd();
    } catch (std::exception& ex) {
        std::cout << "Caught exception running " << commands[index].name << ": " << ex.what() << std::endl;
    }

    // wait for a key press
    initscr();
    keypad(stdscr, TRUE);
    noecho();
    getch();
    system("clear");
}

void cli::Prompt::Run() {
    uint index = 0;

    initscr();
    keypad(stdscr, TRUE);
    noecho();

    while (true) {
        refresh();
        system("clear");
        printMenu(index);
        int commandsCount = commands.size();
        if (index < 0 || index >= commandsCount) {
            index = 0;
        }
        int input = getch();
        switch (input) {
            case 10: // ENTER
                runCommand(index);
                break;
            case KEY_UP:
                system("clear");
                index = (index - 1 < 0) ? index : index - 1;
                break;
            case KEY_DOWN:
                system("clear");
                index = (index + 1 >= commandsCount) ? index : index + 1;
                break;
            case 113: // QUIT
                endwin();
                exit(0);
            default:
                system("clear");
                break;
        }
    }
}
