#include <iostream>
#include <string>

#include "./../entity/entity.h"
#include "./../router/router.h"
#include "./../network/network.h"
#include "./../config/config.h"

bool Login(Client * client, std::string username, std::string password){
    Json::StreamWriterBuilder builder;
    Json::Value out;
    out["type"] = "login";
    out["content"]["username"] = username;
    out["content"]["password"] = password;

    std::string str = Json::writeString(builder, out);

    auto res = client->Request(str);

    Json::Reader reader;
    Json::Value json;
    reader.parse(res.content, json);

    return json["status"].asInt() == router::STATUS_OK;
}

bool Transfer(Client * client, std::string username, float amount){
    Json::StreamWriterBuilder builder;
    Json::Value out;
    out["type"] = "transfer";
    out["content"]["to"] = username;
    out["content"]["amount"] = amount;

    std::string str = Json::writeString(builder, out);

    auto res = client->Request(str);

    Json::Reader reader;
    Json::Value json;
    reader.parse(res.content, json);

    return json["status"].asInt() == router::STATUS_OK;
}

void History(Client * client){
    Json::StreamWriterBuilder builder;
    Json::Value out;
    out["type"] = "history";

    std::string str = Json::writeString(builder, out);

    auto res = client->Request(str);

    Json::Reader reader;
    Json::Value json;
    reader.parse(res.content, json);

    if (json["status"].asInt() != router::STATUS_OK){
        std::cout << "bad request" << std::endl; 
        return;
    }

    // TODO
    //  - format the history better
    std::cout << json["history"] << std::endl;
}

void Balance(Client * client){
    Json::StreamWriterBuilder builder;
    Json::Value out;
    out["type"] = "balance";

    std::string str = Json::writeString(builder, out);

    auto res = client->Request(str);

    Json::Reader reader;
    Json::Value json;
    reader.parse(res.content, json);

    if (json["status"].asInt() != router::STATUS_OK){
        std::cout << "bad request" << std::endl; 
        return;
    }
    
    std::cout << json["balance"].asFloat() << std::endl; 
}


int main() {
    Config cfg{};

    protocol::RawProtocol p;

    ClientOption opt{
        .server_ip = "127.0.0.1",
        .port = cfg.ServerPort,
        .proto = &p,
        .timeout = 200,
    };

    Client client(&opt);
    client.Connect();

    Login(&client, "kek", "kek") ;
    Balance(&client) ;
    Transfer(&client, "giovanni", 10.5);
    History(&client) ;

    return 0;
}
