#pragma once

#include <fstream>
#include <vector>
class FileAbstraction {

private:

static std::ifstream inStream;
static int64_t inStreamBytes;

static std::ofstream ofStream;
static int64_t ofStreamBytesWritten;

static bool isInstreamFile;
public:
    FileAbstraction(bool IwantToReadaFile){
        this->isInstreamFile = IwantToReadaFile;
    }

    static void LodeFile(std::string path){
        if(isInstreamFile){
            inStream.open(path);
            inStream.ignore( std::numeric_limits<std::streamsize>::max() );
            std::streamsize length = inStream.gcount();
            inStream.clear();   //  Since ignore will have set eof.
            inStream.seekg( 0, std::ios_base::beg );
            inStreamBytes = reinterpret_cast<int64_t>(length);
        }
        else{
            ofStream.open(path);
        }
    }

    static void CloseFile(){
        if(isInstreamFile){
            if(inStream.is_open()){
                inStream.close();
            }
        }
        else{
            if(ofStream.is_open()){
                ofStream.close();
            }
        }
    }

    static void WriteBytes(std::vector<uint8_t>& bytes){
        if(!isInstreamFile){
            if(ofStream.is_open()){
                ofStreamBytesWritten+= bytes.size();
                ofStream.write((const char*)&bytes[0], bytes.size());        
            }
        }
    }


    static void WriteBytes(const char * bytes, int size){
        if(!isInstreamFile){
            if(ofStream.is_open()){
                ofStream.write(bytes, size);        
            }
        }
    }

    static std::vector<uint8_t> ReadBytes(uint length){
        std::vector<uint8_t> v(length);
        if(isInstreamFile && inStream.is_open()){
            inStream.read((char*)&v[0],length);
        }
        return v;
    }

    static auto GetFileSize(){
        if(isInstreamFile){
            return inStreamBytes;
        }else{
            return ofStreamBytesWritten;
        }
    }

};

 std::ifstream FileAbstraction::inStream;
 int64_t FileAbstraction::inStreamBytes = 0;
 std::ofstream FileAbstraction::ofStream;
 int64_t FileAbstraction::ofStreamBytesWritten = 0;
 bool FileAbstraction::isInstreamFile{false};