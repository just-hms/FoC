#include "router.h"


// map of currently logged users SD -> entity:User
std::map<int, entity::User*> users;

std::string ExitWithJSON(int status, std::string message=""){
    Json::StreamWriterBuilder builder;
    Json::Value out;
    out["status"] = status;
    out["message"] = message;
    std::string str = Json::writeString(builder, out);
    return str;
}

std::string Login(router::Context *ctx){
    if (ctx->user != nullptr){
        return ExitWithJSON(router::STATUS_UNAUTHORIZED);
    }

    auto content = ctx->req["content"];
    if (!content.isObject()){
        return ExitWithJSON(router::STATUS_BAD_REQUEST);
    }

    auto res =  repo::Login(
        content["username"].asString(),
        content["password"].asString()
    );

    if (!res){
        return ExitWithJSON(
            router::STATUS_OK, 
            "wrong username or password"
        );
    }

    std::cout << res->username  << " connected" << std::endl;  
    // add user to the map of logged in users
    users[ctx->connectionID] = res; 

    return ExitWithJSON(router::STATUS_OK);
}

std::string Balance(router::Context *ctx){
    if (ctx->user == nullptr){
        return ExitWithJSON(router::STATUS_UNAUTHORIZED);
    }

    auto balance = repo::Balance(ctx->user->ID);

    // return the custom balance response
    Json::StreamWriterBuilder builder;
    Json::Value out;
    out["status"] = router::STATUS_OK;
    out["balance"] = balance;
    std::string str = Json::writeString(builder, out);
    return str;
}

std::string Transfer(router::Context *ctx){
    if (ctx->user == nullptr){
        return ExitWithJSON(router::STATUS_UNAUTHORIZED);
    }

    auto content = ctx->req["content"];
    if (!content.isObject()){
        return ExitWithJSON(router::STATUS_BAD_REQUEST);
    }

    auto username = content["username"].asString();
    auto amount = content["amount"].asInt();

    auto res =  repo::Transfer(
        content["to"].asString(),
        content["amount"].asInt()
    );
    
    if (!res){
        return ExitWithJSON(router::STATUS_BAD_REQUEST);
    }

    return ExitWithJSON(router::STATUS_OK);
}

std::string History(router::Context *ctx){
    if (ctx->user == nullptr){
        return ExitWithJSON(router::STATUS_UNAUTHORIZED);
    }

    auto history = repo::History(ctx->user->username);

    // return the custom balance history
    Json::StreamWriterBuilder builder;
    Json::Value out;
    out["status"] = router::STATUS_OK;
    out["history"] = Json::arrayValue;
    
    for (const auto& transfer : history) {
        Json::Value jsonTransfer;
        
        jsonTransfer["amount"] = transfer.amount;
        jsonTransfer["from"] = transfer.from;
        jsonTransfer["to"] = transfer.to;

        out["history"].append(jsonTransfer);
    }
    
    std::string str = Json::writeString(builder, out);
    return str;
}


void router::Disconnect(int sd){
    auto it = users.find(sd);
    if (it == users.end()){
        return;
    }

    std::cout << it->second->username  << " disconnected" << std::endl;  

    // remove the user from the heap
    delete it->second;
    users.erase(sd);


}


std::string router::Handle(int sd, std::string message){
    Json::Reader reader;
    Json::Value content;
    reader.parse(message, content);

    auto type = content["type"].asString();
    if (type == ""){
        return ExitWithJSON(STATUS_BAD_REQUEST);
    } 

    entity::User *us = nullptr;

    // extract the user from the map
    auto it = users.find(sd);
    if (it != users.end()){
        us = it->second;
    }

    // create the context
    router::Context ctx{
        .user = us,
        .req = content,
        .connectionID = sd,
    };

    if (type == "login")    return Login(&ctx);
    if (type == "balance")  return Balance(&ctx);
    if (type == "transfer") return Transfer(&ctx);
    if (type == "history")  return History(&ctx);

    return ExitWithJSON(STATUS_BAD_REQUEST);
}