#include <chrono>
#include <fstream>
#include <mutex>
#include <ctime>
#include <iomanip>

using Timepoint = std::chrono::time_point<std::chrono::steady_clock>;


class StopWatch {
private:
    std::mutex lock;
    std::mutex lock_2;
    std::string filepath;
    std::ofstream stream;
    std::string protocol;
    Timepoint start = Timepoint();
    Timepoint stop = Timepoint();

    //File 2
    std::ofstream steam_2;

public:

    static std::string GetUtcString(){
        auto t = std::time(nullptr);
        //auto tm = *std::localtime(&t);
        auto tm = *std::gmtime(&t);
        auto str = std::put_time(&tm, "%Y-%m-%d %H:%M:%S");
        auto ms = getCurrentTimeMicroSec() % 1000000; //to get micro seconds
        std::ostringstream oss;
        oss << str << ":" << std::to_string(ms);
        return oss.str();
    }

    void CreateFile(std::string filename, std::string testrun, std::string protocol){

        //File 1 for Events
        {
            std::lock_guard<std::mutex> guard(lock);
            this->protocol = protocol;
            stream.open(filename);
            stream << "Start of Testrun: " << testrun << " Start at: " << GetUtcString() << std::endl;
            stream << "UTC,Event,message" << std::endl;
        }

        {
            //File 2 for RTT Events
            std::lock_guard<std::mutex> guard(lock_2);
            steam_2.open(filename + "RTT");
            steam_2 << "Start of Testrun: " << testrun << " Start at: " << GetUtcString() << std::endl;
            steam_2 << "UTC,Event,RTT,Usecase" << std::endl;
        }

    }

    void CloseFile(){
        {
            std::lock_guard<std::mutex> guard(lock);
            stream << "File Closed at: " << GetUtcString() << std::endl;
            stream.close();
        }
        {
            std::lock_guard<std::mutex> guard(lock_2);
            stream << "File Closed at: " << GetUtcString() << std::endl;
            steam_2.close();
        }
    }

    static std::chrono::milliseconds::rep getCurrentTimeMicroSec(){
        return std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
    }

    static std::chrono::milliseconds::rep getCurrentTimeMs(){
        return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
    }

    static Timepoint getCurrentTime() {
        return std::chrono::steady_clock::now();
    }

    void Start(){
        std::lock_guard<std::mutex> guard(lock);
        this->start = this->getCurrentTime();
    }
    void Stop(){
        std::lock_guard<std::mutex> guard(lock);
        this->stop = this->getCurrentTime();
    }

    static std::chrono::milliseconds::rep getTimeDifMicroSec(Timepoint start, Timepoint stop){
        return std::chrono::duration_cast<std::chrono::microseconds>(stop - start).count();
    }

    void CreateLogEntry(std::string eventstr, std::string message, Timepoint start, Timepoint stop){
        std::lock_guard<std::mutex> guard(lock_2);
        steam_2 << GetUtcString() << "," << eventstr << "," << getTimeDifMicroSec(start, stop) << "," << message << std::endl;
    }

    void CreateLogEntry(std::string eventstr, std::string message){
        std::lock_guard<std::mutex> guard(lock_2);
        steam_2  << GetUtcString() << "," << eventstr << "," << getTimeDifMicroSec(this->start, this->stop) << "," << message << std::endl;
    }

    void ConnectedEvent(std::string message = ""){
        std::lock_guard<std::mutex> guard(lock);
        stream << GetUtcString() << ",Connected," << message << std::endl;
    }

    void DisconnectedEvent(std::string message = ""){
        std::lock_guard<std::mutex> guard(lock);
        stream << GetUtcString() << ",Disconnected," << message << std::endl;
    }

    void SendEvent(std::string message = ""){
        std::lock_guard<std::mutex> guard(lock);
        stream << GetUtcString() << ",Send," << message << std::endl;
    }

    void ReceivedEvent(std::string message = "null"){
        std::lock_guard<std::mutex> guard(lock);
        stream << GetUtcString() << ",Received," << message << std::endl;
    }
};
