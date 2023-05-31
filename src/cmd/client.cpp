#include <cstdlib>
#include <memory>
#include <vector>

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

void resetPrompt(){
    prompt.commands = {
        cli::Command{
            .cmd = Login,
            .name = "login"
        },
    };
}


void Login(){
    std::cout<<"Insert your username" << std::endl;
    std::string username;
    getline(std::cin, username);
    if (!sanitize::isUsername(username)){
        std::cout << "username is not correctly formatted" << std::endl;
        return;
    }

    std::cout<<"Insert your password" << std::endl;
    std::string password;
    getline(std::cin, password);
    if (!sanitize::isPassword(password)){
        std::cout << "password is not correctly formatted" << std::endl;
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
        std::cout << "error during the request to the server" << std::endl;
        resetPrompt();
        return;
    } 


    // read the response
    Json::Value json;
    reader.parse(res, json);

    if (json["status"].asInt() != router::STATUS_OK){
        std::cout << "wrong username or password" << std::endl;
        return;
    }

    std::cout << "correctly logged in!!!" << std::endl;

    prompt.commands = std::vector<cli::Command>{
        cli::Command{
            .cmd = Transfer,
            .name = "tranfer",
        },
        cli::Command{
            .cmd = Balance,
            .name = "balance",
        },
        cli::Command{
            .cmd = History,
            .name = "history",
        },
    };
}

void Transfer(){

    std::cout<<"Insert the username of the beneficiary" << std::endl;
    std::string beneficiary;
    getline(std::cin, beneficiary);
    if (!sanitize::isUsername(beneficiary)){
        std::cout << "username is not correctly formatted" << std::endl;
        return;
    }

    std::cout<<"Insert the amount" << std::endl;
    std::string amount;
    getline(std::cin, amount);
    if (!sanitize::isCurrency(amount)){
        std::cout << "the amount is not correctly formatted (try with: 10.20)" << std::endl;
        return;
    }

    Json::Value out;
    out["route"] = "transfer";
    out["content"]["to"] = beneficiary;
    out["content"]["amount"] = stof(amount);

    std::string str = Json::writeString(builder, out);
    
    auto [res, err] = client->Request(str);
    if(err != entity::ERR_OK){
        std::cout << "error during the request to the server" << std::endl;
        resetPrompt();
        return;
    } 

    Json::Reader reader;
    Json::Value json;
    reader.parse(res, json);

    if (json["status"].asInt() != router::STATUS_OK){
        std::cout << "transfer unsuccessfull" << std::endl;
        return;
    }

    std::cout << "transfer successfull" << std::endl;
}

void History(){
    Json::Value out;
    out["route"] = "history";

    std::string str = Json::writeString(builder, out);

    auto [res, err] = client->Request(str);
    if(err != entity::ERR_OK) {
        std::cout << "error during the request to the server" << std::endl;
        resetPrompt();
        return;
    } 

    Json::Value json;
    reader.parse(res, json);

    if (json["status"].asInt() != router::STATUS_OK){
        std::cout << "bad request" << std::endl; 
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
        std::cout << "error during the request to the server" << std::endl;
        resetPrompt();
        return;
    }

    Json::Value json;
    reader.parse(res, json);

    if (json["status"].asInt() != router::STATUS_OK){
        std::cout << "bad request" << std::endl; 
        return;
    }
    
    std::cout << json["balance"].asFloat() << std::endl;
}

int main(int argc, char** argv){
    if (argc != 2 || !sanitize::isUsername(argv[1])){
        std::cout << "must provide the name of the private key file" << std::endl;
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

    resetPrompt();
    prompt.Run();
}