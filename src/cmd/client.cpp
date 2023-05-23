#include <iostream>
#include <string>
#include <ncurses.h>

#include "./../entity/entity.h"
#include "./../network/network.h"
#include "./../config/config.h"
#include "./../protocol/protocol.h"
#include "./../router/router.h"

#define SELECTED "●"
#define UNSELECTED "○"
#define ENTER 10
#define QUIT 113

std::vector<std::string> choices = {"Balance", "Transfer", "History"};

void printMenu(unsigned int index) {
    if(index > 2) return;
    for(int i = 0; i < choices.size(); i++) {
        if(i == index) std::cout<<SELECTED;
        else std::cout<<UNSELECTED;
        std::cout<<" "<<choices[i]+"\r\n";
    }
}

bool Login(net::Client * client, std::string username, std::string password){
    Json::StreamWriterBuilder builder;
    Json::Value out;
    out["route"] = "login";
    out["content"]["username"] = username;
    out["content"]["password"] = password;

    std::string str = Json::writeString(builder, out);
    
    std::cout<<"sending"<<std::endl;
    auto [res, err] = client->Request(str);
    if(err == entity::ERR_BROKEN) {
        //socket should be closed
        exit(-1);
    }
    else if(err != entity::ERR_OK) return false;

    Json::Reader reader;
    Json::Value json;
    reader.parse(res, json);
    
    return json["status"].asInt() == router::STATUS_OK;
}

bool Transfer(net::Client * client, std::string username, float amount){
    Json::StreamWriterBuilder builder;
    Json::Value out;
    out["route"] = "transfer";
    out["content"]["to"] = username;
    out["content"]["amount"] = amount;

    std::string str = Json::writeString(builder, out);

    auto [res, err] = client->Request(str);
    if(err != entity::ERR_OK) return false;

    Json::Reader reader;
    Json::Value json;
    reader.parse(res, json);

    return json["status"].asInt() == router::STATUS_OK;
}

void History(net::Client * client){
    Json::StreamWriterBuilder builder;
    Json::Value out;
    out["route"] = "history";

    std::string str = Json::writeString(builder, out);

    auto [res, err] = client->Request(str);
    if(err != entity::ERR_OK) return;

    Json::Reader reader;
    Json::Value json;
    reader.parse(res, json);

    if (json["status"].asInt() != router::STATUS_OK){
        std::cout << "bad request" << std::endl; 
        return;
    }

    // TODO format the history better
    std::cout << json["history"] << std::endl;

    return;
}

void Balance(net::Client * client){
    Json::StreamWriterBuilder builder;
    Json::Value out;
    out["route"] = "balance";

    std::string str = Json::writeString(builder, out);

    auto [res, err] = client->Request(str);
    if(err != entity::ERR_OK) {
        return;
    }

    Json::Reader reader;
    Json::Value json;
    reader.parse(res, json);

    if (json["status"].asInt() != router::STATUS_OK){
        std::cout << "bad request" << std::endl; 
        return;
    }
    
    std::cout << json["balance"].asFloat() << std::endl;
}


int main() {
    config::Config cfg;

    protocol::FunkyOptions fOpt{
        .name = "client",
        .peerName = "server",
        .dataPath = "./data/",
        .secret = cfg.Secret,
    };
    
    protocol::FunkyProtocol protocol(&fOpt);

    net::ClientOption opt{
        .server_ip = "127.0.0.1",
        .port = cfg.ServerPort,
        .proto = &protocol,
        .timeout = 200,
    };

    
    net::Client client(&opt);
    auto err = client.Connect();
    if(err != entity::ERR_OK) {
        std::cout<<"Some error occured when connecting to the server\r\n";
        return -1;
    }

    std::string buffer;
    std::string uname;
    std::string pwd;
    bool res = false; 

    do {
        do {
            std::cout<<"Insert your username [max "<<sec::MAX_SANITIZATION_LEN-1<<" characters]: ";
            getline(std::cin, buffer);
            res = sec::sanitize(buffer, 0);
        } while(!res);
        uname = buffer;

        do {
            std::cout<<"Insert your password [8 - "<<sec::MAX_SANITIZATION_LEN-1<<" characters]: ";
            getline(std::cin, buffer);
            res = sec::sanitize(buffer, 1);
        } while(!res);
        pwd = buffer;
        if(!(res = Login(&client, uname, pwd))) std::cout<<"Incorrect username or password, try again\r\n";
    } while(!res);
    
    unsigned int index = 0;
    
    initscr();
    keypad(stdscr, TRUE);
    noecho();

    while(true) {
        refresh();
        printw("Use arrows to move across the options, press enter to confirm and q to quit\r\n");
        printMenu(index);
        switch(int(getch())) {
            case ENTER:
                refresh();
                endwin();
                system("clear");

                if(index == 0) Balance(&client);
                
                else if(index == 1) {
                    std::string beneficiary;
                    do {
                        std::cout<<"Insert the username of the beneficiary [max "<<sec::MAX_SANITIZATION_LEN-1<<" characters]: ";
                        getline(std::cin, buffer);
                        res = sec::sanitize(buffer, 0);
                    } while(!res);
                    beneficiary = buffer;

                    do {
                        std::cout<<"Insert the amount to transfer [max "<<sec::MAX_SANITIZATION_LEN-1<<" characters]: ";
                        getline(std::cin, buffer);
                        res = sec::sanitize(buffer, 2);
                    } while(!res);

                    if(Transfer(&client,beneficiary, stoi(buffer))) std::cout<<"Transfer successful\r\n";
                    else std::cout<<"Transfer unsuccessful\r\n";
                }
                
                else if(index == 2) History(&client);

                initscr();
                keypad(stdscr, TRUE);
                noecho();
                getch();
                system("clear");
                break;

            case KEY_UP:
                system("clear");
                index = (index - 1 > 2) ? 2 : index - 1;
                break;

            case KEY_DOWN:
                system("clear");
                index = (index + 1 > 2) ? 0 : index + 1;
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
