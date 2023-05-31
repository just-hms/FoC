#include "router.h"
#include <jsoncpp/json/value.h>

Json::Value ExitWithJSON(int status, std::string message=""){
    Json::StreamWriterBuilder builder;
    Json::Value out;
    out["status"] = status;
    out["message"] = message;
    return out;
}

Json::Value router::Router::Login(router::Context *ctx){
    if (ctx->user != nullptr){
        return ExitWithJSON(router::STATUS_UNAUTHORIZED);
    }

    auto content = ctx->req["content"];
    if (!content.isObject()){
        return ExitWithJSON(router::STATUS_BAD_REQUEST);
    }

    auto [us, err] = this->repo->Login(
        content["username"].asString(),
        content["password"].asString()
    );

    if (err != entity::ERR_OK){
        return ExitWithJSON(
            router::STATUS_UNAUTHORIZED, 
            "wrong username or password"
        );
    }

    // add user to the map of logged in users
    this->users[ctx->connectionID] = us;

    return ExitWithJSON(router::STATUS_OK);
}

Json::Value router::Router::Balance(router::Context *ctx){
    if (ctx->user == nullptr){
        return ExitWithJSON(router::STATUS_UNAUTHORIZED);
    }

    auto [balance, err] = this->repo->Balance(ctx->user->username);

    if (err != entity::ERR_OK){
        return ExitWithJSON(
            router::STATUS_INTERNAL_ERROR, 
            "internal error"
        );
    }

    // return the custom balance response
    Json::Value out;
    out["status"] = router::STATUS_OK;
    out["balance"] = balance.amount;
    out["accountID"] = balance.accountID;
    
    return out;
}

Json::Value router::Router::Transfer(router::Context *ctx){
    if (ctx->user == nullptr){
        return ExitWithJSON(router::STATUS_UNAUTHORIZED);
    }

    auto content = ctx->req["content"];
    if (!content.isObject()){
        return ExitWithJSON(router::STATUS_BAD_REQUEST);
    }

    auto username = content["username"].asString();
    auto amount = content["amount"].asFloat();

    auto t = entity::Transaction{
        .from = ctx->user->username,
        .to = content["to"].asString(),
        .amount = content["amount"].asFloat()
    };

    auto [res, err] =  this->repo->Transfer(&t);

    if (err != entity::ERR_OK){
        return ExitWithJSON(
            router::STATUS_INTERNAL_ERROR, 
            "internal error"
        );
    }
    
    if (!res){
        return ExitWithJSON(router::STATUS_BAD_REQUEST);
    }

    return ExitWithJSON(router::STATUS_OK);
}

Json::Value router::Router::History(router::Context *ctx){
    if (ctx->user == nullptr){
        return ExitWithJSON(router::STATUS_UNAUTHORIZED);
    }

    auto [history, err] = this->repo->History(ctx->user->username);

    if (err != entity::ERR_OK){
        return ExitWithJSON(
            router::STATUS_INTERNAL_ERROR, 
            "internal error"
        );
    }
    
    // return the custom balance history
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
    
    return out;
}


void router::Router::Disconnect(int sd){
    auto it = this->users.find(sd);
    if (it == this->users.end()){
        return;
    }

    std::string route = "logout";
    std::cout << "/" << route <<  std::string(10 - route.length(), ' ' ) << 201 << std::endl; 
    this->users.erase(sd);
}


std::string router::Router::Handle(int sd, std::string message){
    Json::Reader reader;
    Json::Value content;
    reader.parse(message, content);

    std::shared_ptr<entity::User> us = nullptr;

    // extract the user from the map
    auto it = this->users.find(sd);

    // if exists set it
    if (it != this->users.end()){
        us = it->second;
    }

    // create the context
    router::Context ctx{
        .user = us,
        .req = content,
        .connectionID = sd,
    };


    auto out = ExitWithJSON(STATUS_NOT_FOUND); 

    std::string route = "";
    // this should happen in cas of bad formatting of the request
    try {
        route = content["route"].asString();
        if (route == "login")           out = this->Login(&ctx);
        else if (route == "balance")    out = this->Balance(&ctx);
        else if (route == "transfer")   out = this->Transfer(&ctx);
        else if (route == "history")    out = this->History(&ctx);
    } catch (...) {
        out = ExitWithJSON(STATUS_BAD_REQUEST); 
    }

    // log the request
    std::cout << "/" << route <<  std::string(10 - route.size(), ' ' ) << out["status"] << std::endl; 

    Json::StreamWriterBuilder builder;
    std::string str = Json::writeString(builder, out);
    return str;
}

router::Router::Router(router::IRepo* repo){
    if(repo == nullptr){
        std::cerr << "You must specify a repo " << std::endl;
        exit(EXIT_FAILURE);
    }
    this->repo = repo;
}