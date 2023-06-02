#include <cstdlib>
#include <memory>
#include <vector>
#include <signal.h>

#include "./../cli/cli.h"
#include "./../network/network.h"
#include "./../entity/entity.h"
#include "./../config/config.h"
#include "./../protocol/protocol.h"
#include "./../router/router.h"


net::Client *client;
cli::Prompt prompt;
config::Config cfg;

Json::StreamWriterBuilder builder;
Json::Reader reader;

void Login();
void Transfer();
void History();
void Balance();

void handleServerError(){
    std::cout << "Error during the request to the server..." << std::endl;
    prompt.commands = {
        cli::Command{
            .cmd = Login,
            .name = "Login"
        },
    };
}


void Login(){
    std::cout<<"Insert your username" << std::endl;
    std::string username;
    getline(std::cin, username);
    if (!sanitize::isUsername(username)){
        std::cout << "Username is not correctly formatted" << std::endl;
        return;
    }

    std::cout<<"Insert your password" << std::endl;
    std::string password;
    getline(std::cin, password);
    if (!sanitize::isPassword(password)){
        std::cout << "Password is not correctly formatted" << std::endl;
        return;
    }
    // build the request

    Json::Value out;
    out["route"] = "login";
    out["content"]["username"] = username;
    out["content"]["password"] = password;
    std::string str = Json::writeString(builder, out);

    
    auto [res, err] = client->Request(str);
    if(err != entity::ERR_OK){
        handleServerError();
        return;
    } 

    // read the response
    Json::Value json;
    reader.parse(res, json);

    if (json["status"].asInt() != router::STATUS_OK){
        std::cout << "Wrong username or password" << std::endl;
        return;
    }

    std::cout << "Correctly logged in!!!" << std::endl;

    prompt.commands = std::vector<cli::Command>{
        cli::Command{
            .cmd = Balance,
            .name = "Balance",
        },
        cli::Command{
            .cmd = Transfer,
            .name = "Transfer",
        },
        cli::Command{
            .cmd = History,
            .name = "History",
        },
    };
}

void Transfer(){

    std::cout<<"Insert the username of the beneficiary" << std::endl;
    std::string beneficiary;
    getline(std::cin, beneficiary);
    if (!sanitize::isUsername(beneficiary)){
        std::cout << "Username is not correctly formatted" << std::endl;
        return;
    }

    std::cout<<"Insert the amount" << std::endl;
    std::string amount;
    getline(std::cin, amount);
    if (!sanitize::isCurrency(amount)){
        std::cout << "The amount is not correctly formatted (try with: 10.20)" << std::endl;
        return;
    }

    Json::Value out;
    out["route"] = "transfer";
    out["content"]["to"] = beneficiary;
    out["content"]["amount"] = stof(amount);

    std::string str = Json::writeString(builder, out);
    
    auto [res, err] = client->Request(str);
    if(err != entity::ERR_OK){
        handleServerError();
        return;
    } 

    Json::Value json;
    reader.parse(res, json);

    if (json["status"].asInt() != router::STATUS_OK){
        std::cout << "Transfer unsuccessfull" << std::endl;
        return;
    }

    std::cout << "Transfer successfull" << std::endl;
}

void History(){
    Json::Value out;
    out["route"] = "history";

    std::string str = Json::writeString(builder, out);

    auto [res, err] = client->Request(str);
    if(err != entity::ERR_OK) {
        handleServerError();
        return;
    } 

    Json::Value json;
    reader.parse(res, json);

    if (json["status"].asInt() != router::STATUS_OK){
        std::cout << "Bad request" << std::endl; 
        return;
    }

    // write the history with correctly formatted floats
    Json::StreamWriterBuilder wbuilder;
    wbuilder.settings_["precision"] = 2;
    std::unique_ptr<Json::StreamWriter> writer(wbuilder.newStreamWriter());
    // Write to file.
    writer->write(json["history"], &std::cout);
    std::cout << std::endl;
}

void Balance(){
    Json::Value out;
    out["route"] = "balance";

    std::string str = Json::writeString(builder, out);

    auto [res, err] = client->Request(str);
    if(err != entity::ERR_OK) {
        handleServerError();
        return;
    }

    Json::Value json;
    reader.parse(res, json);

    if (json["status"].asInt() != router::STATUS_OK){
        std::cout << "Bad request" << std::endl; 
        return;
    }
    
    std::cout << json["balance"].asFloat() << std::endl;
}

int main(int argc, char** argv){

    signal(SIGPIPE, SIG_IGN);

    if (argc != 2 || !sanitize::isUsername(argv[1])){
        std::cout << "Must provide the name of the private key file" << std::endl;
        return 1;
    }

    protocol::FunkyOptions fOpt{
        .name = argv[1],
        .peerName = "server",
        .dataPath = "./data/keys/",
        .secret = cfg.Secret,
    };

    protocol::FunkyProtocol proto(&fOpt);

    net::ClientOption clientOpt{
        .server_ip = "127.0.0.1",
        .port = cfg.ServerPort,
        .proto = &proto,
        .timeout = 200,
    };

    auto c = net::Client(&clientOpt);
    client = &c;

    prompt.commands = {
        cli::Command{
            .cmd = Login,
            .name = "Login"
        },
    };    
    prompt.Run();
}