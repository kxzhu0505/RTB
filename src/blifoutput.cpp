/**
 * @file blifoutput.cpp
 * @brief Using VPR timing reports to generate benchmarks for latch-based timing analysis
 * @details
 * @author kxzhu
 * @version 1.0
 * @date 2023-02-27
*/

#include <iostream>
#include <fstream>
#include <cstdlib>
#include <cstring>
#include <vector>
#include<sys/stat.h>
#include<unistd.h>

using namespace std;


/**
 * @brief Test if the character c is in the string str.
 * 
 * @param c 
 * @param str 
 * @return true 
 * @return false 
 */
inline bool isCharIn(char c, string str)
{
    for (size_t i = 0; i < str.size(); ++i)
        if (c == str[i])
            return true;
    return false;
}


/**
 * @brief Split a text line into seperate words, and store them in a vector of string.
 * 
 * @param str 
 * @param seps 
 * @param words 
 */
inline void splitString(const string &str, const string &seps, vector<string> &words)
{
    words.clear();
    size_t i = 0;
    while (isCharIn(str[i], seps))
        ++i;
    size_t beginIndex;
    bool isSepBefore = false;
    for (beginIndex = i; i != str.size(); ++i)
    {
        if (isCharIn(str[i], seps))
        {
            if (!isSepBefore)
            {
                words.push_back(str.substr(beginIndex, i - beginIndex));
                beginIndex = i + 1;
                isSepBefore = true;
            }
            else
                beginIndex = i + 1;
        }
        else
            isSepBefore = false;
    }
    if (!str.empty() && !isSepBefore)
        words.push_back(str.substr(beginIndex, i - beginIndex));
}


/**
 * @brief Convert the timing rpt of VPR to temp file
 * 
 * @param rptfile 
 * @param tempfile 
 * @return true 
 * @return false
 */
bool benchmarkConversion(const char* rptfile,const char* tempfile)
{
    ifstream inrpt(rptfile);
    if (!inrpt)
    {
        cout << "Error in reading file " << rptfile << endl;
        return false;
    }
    ofstream temp(tempfile);
    string textLine;
    vector<string> words;
    string record;
    int cunt=0;
    while (inrpt)
    {
        getline(inrpt, textLine);
        words.clear();
        splitString(textLine, " \t", words);
        if (textLine == "#End of timing report")
        {
            temp << "End";
            temp << endl;
        }
        if ((words.size() == 0) || (words[0] == "#")|| (words[0] == "-") || ((words[0] != "Startpoint:") && (words[0] != "Endpoint") && (words[1] != "arrival")))
            continue;
        if (words[0] == "Startpoint:")
        {
            temp << words[1] << "\t" ;
            continue;
        }
        if (words[0] == "Endpoint")
        {
            temp << words[2] << "\t" ;
            continue;
        }
        if (words[1] == "arrival")
        {
            if(cunt%3 == 0)
            {
                temp << words[3] << "\n" ;
            }
            cunt ++;
            continue;
        }
    }
    inrpt.close();
    temp.close();
    return true;
}

/**
 * @brief Output the rsltfile for latch-based timing analysis
 * 
 * @param setupfile 
 * @param holdfile
 * @param ofName 
 * @return true 
 * @return false
 */
bool printrsltfile(const char* setupfile,const char* holdfile,const char* ofName)
{   
    cout << "rslt fiel step1"<<endl;
    ofstream outfile;
    outfile.open(ofName);
    ifstream insetup(setupfile);
    if (!insetup)
    {
        cout << "Error in reading file " << setupfile << endl;
        return false;
    }
    ifstream inhold(holdfile);
    if (!inhold)
    {
        cout << "Error in reading file " << holdfile << endl;
        return false;
    }
    cout << "rslt fiel step2"<<endl;
    outfile << "Network " << ofName << "\n" 
    << "latch latch max_delay min_delay" << "\n"
    << "INPUT latch max_delay min_delay" << "\n"
    << "latch OUTPUT max_delay min_delay" << "\n";
    string setupLine,holdLine;
    vector<string> setupwords,holdwords;
    vector<vector<string> > setup,hold;
    while (insetup)
    {
        int i = 0;
        getline(insetup, setupLine);
        setupwords.clear();
        splitString(setupLine, " \t", setupwords);
        setup.push_back(setupwords);
    }
    cout << "rslt fiel step3"<<endl;
    while (inhold)
    {
        getline(inhold, holdLine);
        holdwords.clear();
        splitString(holdLine, " \t", holdwords);
        hold.push_back(holdwords);
    }
    cout << "rslt fiel step4"<<endl;
    insetup.close();
    inhold.close();
    int arr[2000] = {0};
    bool flag;
    int i,j;
    for(i=0; i < setup.size()-2; i++)
    {
        int mark=0;
        for(j=0; j < hold.size()-2; j++)
        {   
            string hold_name_0 = hold[j][0].substr(0,hold[j][0].find_last_of("."));
            string hold_name_1 = hold[j][1].substr(0,hold[j][1].find_last_of("."));
	        string setup_name_0 = setup[i][0].substr(0,setup[i][0].find_last_of("."));
            string setup_name_1 = setup[i][1].substr(0,setup[i][1].find_last_of("."));
            if ((setup[i][0] == hold[j][0]) && (setup[i][1] == hold[j][1]))
            {
                if(hold[j][0].find("outpad") != string::npos)
                    hold_name_0 = "OUTPUT";
                if(hold[j][1].find("outpad") != string::npos)
                    hold_name_1 = "OUTPUT";
                if(hold[j][0].find("inpad") != string::npos)
                    hold_name_0 = "INPUT";
                if(hold[j][1].find("inpad") != string::npos)
                    hold_name_1 = "INPUT";
                float maxDelay = (stof(setup[j][2]) > stof(hold[j][2])) ? stof(setup[j][2]):stof(hold[j][2]);
                float minDelay = (stof(setup[j][2]) < stof(hold[j][2])) ? stof(setup[j][2]):stof(hold[j][2]);
                outfile << hold_name_0 << "\t" << hold_name_1 << "\t" << maxDelay << "\t" << minDelay << "\n";
                
                arr[j] = 1;
                mark++;
                flag = 1;
            }    
        }
        if(!flag)
        {
            string setup_name_0 = setup[i][0].substr(0,setup[i][0].find_last_of("."));
            string setup_name_1 = setup[i][1].substr(0,setup[i][1].find_last_of("."));
            if(setup[i][0].find("outpad") != string::npos)
                    setup_name_0 = "OUTPUT";
            if(setup[i][1].find("outpad") != string::npos)
                    setup_name_1 = "OUTPUT";
            if(setup[i][0].find("inpad") != string::npos)
                   setup_name_0 = "INPUT";
            if(setup[i][1].find("inpad") != string::npos)
                    setup_name_1 = "INPUT";
            outfile << setup_name_0 << "\t" << setup_name_1 << "\t" << setup[i][2] << "\t" << setup[i][2] << "\n";
        
        }
    }
    int mark=0;
    for(i=0; i < hold.size()-2; i++)
    {
        if(!arr[mark]){
            string hold_name_0 = hold[i][0].substr(0,hold[i][0].find_last_of("."));
            string hold_name_1 = hold[i][1].substr(0,hold[i][1].find_last_of("."));
            if(hold[i][0].find("outpad") != string::npos)
                    hold_name_0 = "OUTPUT";
            if(hold[i][1].find("outpad") != string::npos)
                    hold_name_1 = "OUTPUT";
            if(hold[i][0].find("inpad") != string::npos)
                    hold_name_0 = "INPUT";
            if(hold[i][1].find("inpad") != string::npos)
                    hold_name_1 = "INPUT";            
            outfile << hold_name_0 << "\t" << hold_name_1 << "\t" << hold[i][2] << "\t" << hold[i][2] << "\n";
        }
        mark++;
    }
    outfile << endl;
    outfile.close();
    return true;

}

/**
 * @brief read the timing rpt and output the rslt
 * 
 * @param baselinePath 
 * @param filename
 * @return void
 */
void benchmark_vtr(string baselinePath, string filename)
{
    const char* folderPath = nullptr;
    string prefix = "Base";
    string fp,ofName;
    if(filename.substr(0,prefix.size()) == prefix){
        fp = "Baseline";
        ofName = baselinePath + ".out";
    }
    else{
        fp =  string(filename.begin(),filename.begin() + 6);
        ofName = filename + ".out";
    }
    cout<<fp<<endl;
    
    cout<<ofName<<endl;
    folderPath = fp.c_str();
    chdir(folderPath);
    string fn = filename;
    

    string setup = "report_timing.hold.rpt";
    string hold = "report_timing.setup.rpt";
    const char* setupfp = setup.c_str();
    const char* holdfp = hold.c_str();

    benchmarkConversion(setupfp,"hold.out");
    benchmarkConversion(holdfp,"setup.out");
    
    cout << "set.out and hold.out succeed"<<endl;
    printrsltfile("setup.out","hold.out",ofName.c_str());
    
    string cpcommand = "cp " + ofName + " /home/kxzhu/vtr/vtr-verilog-to-routing-8.0.0/work_flow/SRlut4/rlst";
    system(cpcommand.c_str());
}

