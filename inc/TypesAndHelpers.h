#pragma once

#include <iostream>
#include <functional>
#include <chrono>
#include <thread>


//helpers
/**
 * Used to wait for *waitForTrue* Lamda.
 * Every *sleepms* the Lamda will be called again.
 * Once the *timeoutms* is reached it will terminate and log an error. 
 */
static void waitFor(std::function<bool(void)> waitForTrue, int sleepms, int timeoutms){
    std::chrono::time_point<std::chrono::steady_clock> start = std::chrono::steady_clock::now();
    //std::cout << "start" << " check: " << waitForTrue() << std::endl;
    while(!waitForTrue()){
        std::chrono::time_point<std::chrono::steady_clock> now = std::chrono::steady_clock::now();
        std::this_thread::sleep_for(std::chrono::milliseconds(sleepms));
        if((now -start) > std::chrono::milliseconds(timeoutms)){
            std::cerr << "Timeout" << std::endl;
            break;
        }
       // std::cout << "next round" << std::endl;
    }
}
static void Sleep(int sleepms){
    std::this_thread::sleep_for(std::chrono::milliseconds(sleepms));
}

enum TESTTYPE {
    KEYBOARD = 0,
    STARTFIRECLOSE = 1,
    STARTLOOPCLOSE = 2,
    STARTDOWNLOADCLOSE = 3,
    STARTFIREDOWNLOADCLOSE = 4
};
static TESTTYPE getTestTypeFromInt(int i){
    switch(i){
        case 0:
        return TESTTYPE::KEYBOARD;
        case 1: 
        return TESTTYPE::STARTFIRECLOSE;
        case 2:
        return TESTTYPE::STARTLOOPCLOSE;
        case 3:
        return TESTTYPE::STARTDOWNLOADCLOSE;
        case 4:
        return TESTTYPE::STARTFIREDOWNLOADCLOSE;
        default:
        return TESTTYPE::KEYBOARD;
    }
};
static std::string getStringfromTesttype(int i){
    switch(i){
        case 0:
        return "TESTTYPE::KEYBOARD";
        case 1: 
        return "TESTTYPE::STARTFIRECLOSE";
        case 2:
        return "TESTTYPE::STARTLOOPCLOSE";
        case 3:
        return "TESTTYPE::STARTDOWNLOADCLOSE";
        case 4:
        return "TESTTYPE::STARTFIREDOWNLOADCLOSE";
        default:
        return "TESTTYPE::KEYBOARD";
    }
};