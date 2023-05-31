#include "cli.h"
#include <exception>

cli::Prompt::Prompt(){}

void cli::Prompt::printMenu(uint index){
    for(int i = 0; i < this->commands.size(); i++) {
        
        std::cout << ((i == index) ? "●" : "○");

        std::cout<<" "<<this->commands[i].name+"\r\n";
    }
}

#define ENTER 10
#define QUIT 113
    
void cli::Prompt::Run(){ 
    uint index = 0;
    
    initscr();
    keypad(stdscr, TRUE);
    noecho();

    while(true) {
        refresh();
        printw("Use arrows to move across the options, press enter to confirm and q to quit\r\n");
        this->printMenu(index);
        int commandsCount = this->commands.size();
        if (index < 0 || index >= commandsCount) index = 0;
        switch(int(getch())) {
            case ENTER:
                // reset terminal
                refresh();
                endwin();
                system("clear");
                try {
                    this->commands[index].cmd();
                } catch (std::exception ex) {
                    std::cout << "Catched exception running " << this->commands[index].name << std::endl;
                }
                
                // reset terminal back to ncurses
                initscr();
                keypad(stdscr, TRUE);
                noecho();
                getch();
                system("clear");
                break;
            case KEY_UP:
                system("clear");
                index = (index - 1 < 0) ? index : index - 1;
                break;

            case KEY_DOWN:
                system("clear");
                index = (index + 1 >= commandsCount) ? index : index + 1;
                break;

            case QUIT:
                endwin();
                exit(0);

            default:
                system("clear");
                break;
        }

    }
}