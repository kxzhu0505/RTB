#include<cstdlib>
#include<string>
#include<cstdio>
#include<cstring>
#include<iostream>
#include<algorithm>
#include <vector>
#include <string>
#include <unordered_map>
using namespace std;

class SR{
private:
    string forwardFF;
    string forwardLUT;

    string backwardFF;
    string backwardLUT;
    std::unordered_map<std::string,float> Rslt;

public:
    void ExecuteCommand(string command);
    bool loadTimingRptFw();
    bool loadTimingRptBw();
    bool BlifForwardModify(const char* fileName,string ofName);
    bool BlifBackwardModify(const char* fileName,string ofName);
    bool RsltExtract(string ofName);
    string RsltPrint();
};