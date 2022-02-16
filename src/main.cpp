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
#include "StopWatch.h"
#include "Client.h"

int main(int argc, char* argv[]){

   google::InitGoogleLogging("MAIN");
   google::SetCommandLineOptionWithMode("logtostderr", "1", gflags::SET_FLAGS_DEFAULT);
   

  //folly::Init init(&argc, &argv);
    fizz::CryptoUtils::init();
    
    
    LOG(INFO) << "Start Client";
    if(argc > 1){
         MyClient myclient {argv[1], atoi(argv[2])};
            myclient.start();
    }else{
        MyClient myclient {"127.0.0.1", 6666};
           myclient.start();
    }

/*     LOG(ERROR) << "Output " << myclient.getString();

    folly::EventBase base;
    auto thread1 = std::thread([&](){
        try{
             base.loopForever();
        }catch(...){
            printf("Error");
        }
        printf("Exit thread\n");
       
    });
    base.runInEventBaseThread([&](){
        printf("This will be printed in thread1\n");
    });
    base.terminateLoopSoon();
    thread1.join(); */

 

   

    return 0;
}