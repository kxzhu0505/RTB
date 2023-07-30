#include <iostream>
#include <fstream>
#include <cstdlib>
#include <cstring>
#include <vector>
#include<sys/stat.h>
#include<unistd.h>
using namespace std;

inline bool isCharIn(char c, string str);
inline void splitString(const string &str, const string &seps, vector<string> &words);
bool benchmarkConversion(const char* rptfile,const char* tempfile);
bool printrsltfile(const char* setupfile,const char* holdfile,const char* ofName);
void benchmark_vtr(string baselinePath, string filename);
void blifoutput(const char* filename);