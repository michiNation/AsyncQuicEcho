#include "Server.h"
#include "Client.h"
#include <stdio.h>
#include <iostream>
#include <fmt/core.h>
#include <folly/FBString.h>
#include <folly/io/async/EventBase.h>
#include <glog/logging.h>
#include <gflags/gflags.h>
#include <fizz/crypto/Utils.h>
#include <folly/init/Init.h>
#include <folly/portability/GFlags.h>
#include "TypesAndHelpers.h"

int main(int argc, char* argv[]){

   /*  google::InitGoogleLogging("MAIN");
    google::SetCommandLineOptionWithMode("logtostderr", "1", gflags::SET_FLAGS_DEFAULT); */
   

    //folly::Init init(&argc, &argv);
    fizz::CryptoUtils::init();
    
    //get the mode
    if(argc < 3){
        std::cout << "Usage: ./binary 127.0.0.1 5050 server" << std::endl;
        std::cout << "Usage: ./binary 127.0.0.1 5050 client <modeNr> <instances> <loops>" << std::endl;
        std::cout << "Modes for Client: KEYBOARD = 0;STARTFIRECLOSE = 1;STARTLOOPCLOSE = 2;STARTDOWNLOADCLOSE = 3;STARTFIREDOWNLOADCLOSE = 4" << std::endl;
        return 0;
    }
    std::string mode = argv[3];
    
    if(mode.compare("client") == 0){
        MyClient myclient;   
        if(argc > 6){
            myclient.start(argv[1], atoi(argv[2]), getTestTypeFromInt(atoi(argv[4])), atoi(argv[5]),atoi(argv[6]));
        }
        else if (argc == 6){
            myclient.start(argv[1], static_cast<uint16_t>(atoi(argv[2])), getTestTypeFromInt(atoi(argv[4])),atoi(argv[5]));
        }
        else if (argc == 5){
            myclient.start(argv[1], static_cast<uint16_t>(atoi(argv[2])), getTestTypeFromInt(atoi(argv[4])));
        }
        
    }
    else if(mode.compare("server") == 0){
        EchoServer echoServer{argv[1],static_cast<uint16_t>(atoi(argv[2]))};
        echoServer.start(); 
    } 
    return 0;
}