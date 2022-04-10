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
    FileAbstraction(bool IwantToReadaFile);;

    static void LodeFile(std::string path);

    static void CloseFile();

    static void WriteBytes(std::vector<uint8_t>& bytes);


    static void WriteBytes(const char * bytes, int size);

    static std::vector<uint8_t> ReadBytes(uint length);

    static int64_t GetFileSize();

};

