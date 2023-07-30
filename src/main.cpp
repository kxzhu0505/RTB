/**
 * @brief Using VPR timing reports to generate benchmarks for latch-based timing analysis
 * 
 * @return true 
 * @return false 
 */

#include<stdlib.h>
#include<cstdlib>
#include<string>
#include<cstdio>
#include<cstring>
#include<iostream>
#include<sstream>
#include<fstream>
#include<algorithm>
#include<deque>
#include<unordered_map>
#include<sys/stat.h>
#include<unistd.h>
#include "SR.hpp"
#include "blifoutput.hpp"

using namespace std;

vector<string> Test(char* fileName, int loopcount){

    vector<string> res;
    string fn = fileName;
    string fp;
    if(loopcount<9){
        string iternum = "run00" + to_string(loopcount+1);
        //cout<<iternum<<endl;
        fp = iternum;
        fn = iternum + fn;
    }
    else{
        string iternum = "run0" + to_string(loopcount+1);
        //cout<<iternum<<endl;
        fp = iternum;
        fn = iternum + fn;
    }
    
    const char* folderPath = nullptr; 
    folderPath= fp.c_str(); 
    mkdir(folderPath,S_IRUSR | S_IWUSR | S_IXUSR | S_IRWXG | S_IRWXO);
    cout << "moving to :" << folderPath << endl;
    chdir(folderPath);
    
    res.push_back(fp);
    res.push_back(fn);
    return res;

}

int main(int argc, char* argv[]){
    SR sr;
    const char* path = nullptr;

    string baselinePath = argv[1];
    baselinePath = baselinePath.substr(0, baselinePath.length() - 5);
    //cout<<tmp<<endl;
    path = baselinePath.c_str();
    //cout<<path<<endl;
    mkdir(path,S_IRUSR | S_IWUSR | S_IXUSR | S_IRWXG | S_IRWXO);
    chdir(path);

    path = "Baseline";
    mkdir(path,S_IRUSR | S_IWUSR | S_IXUSR | S_IRWXG | S_IRWXO);
    chdir(path);

    const char* VTR_path_value = std::getenv("VTR_ROOT");
    string VTR_path = VTR_path_value;

    string baseline = argv[1];
    size_t pos = baseline.find_last_of('/');
    if (pos != std::string::npos) {
        baseline = baseline.substr(pos + 1);
    }

    char* filename = strrchr(argv[1], '/');
        if (filename != nullptr) {
            filename++;
        }

    string command = VTR_path + "/vpr/vpr " + VTR_path + "/vtr_flow/arch/timing/k6_frac_N10_mem32K_40nm.xml ../../" + baseline +" --timing_report_detail aggregated --timing_report_npaths 10000";
    vector<string> FP;
    string RsltFP;
    sr.ExecuteCommand(command);
    sr.RsltExtract("Baseline");
    for(int i = 0;i < 64 ; i++){
        if(i == 0 ){
            if(sr.loadTimingRptBw()){
                cout<<"Backward succeed"<<endl;
                chdir("..");
                FP = Test(filename,i);
                string baselinefile = "../../" + baselinePath +".blif";
                const char* path = baselinefile.c_str();
                sr.BlifBackwardModify(path,FP[1]);
                string command = VTR_path + "/vpr/vpr " + VTR_path + "/vtr_flow/arch/timing/k6_frac_N10_mem32K_40nm.xml " + FP[1] + " --timing_report_detail aggregated --timing_report_npaths 10000";
                sr.ExecuteCommand(command);
                sr.RsltExtract(FP[1]);
                cout << "=================================================================================="<<endl;
            }
            else if(sr.loadTimingRptFw()){
                cout<<"Foward succeed"<<endl;
                chdir("..");
                FP = Test(filename,i);
                string baselinefile = "../../" + baselinePath +".blif";
                const char* path = baselinefile.c_str();
                if(sr.BlifForwardModify(path,FP[1])){
                    string command = VTR_path + "/vpr/vpr " + VTR_path + "/vtr_flow/arch/timing/k6_frac_N10_mem32K_40nm.xml " + FP[1] + " --timing_report_detail aggregated --timing_report_npaths 10000";
                    sr.ExecuteCommand(command);
                    sr.RsltExtract(FP[1]);
                    cout << "=================================================================================="<<endl;
                }
                else{
                        chdir("..");
                        RsltFP =  sr.RsltPrint();
                        benchmark_vtr(baselinePath,RsltFP);
                        return 0;
                    }
            }
        }
        else if(i == 63){
            if(sr.loadTimingRptBw()){
                sr.loadTimingRptFw();
                chdir("..");
                string temp = "../" + FP[0] + "/" + FP[1];
                const char* path = temp.c_str();
                cout << temp <<endl;
                FP = Test(filename,i);
                if(sr.BlifBackwardModify(path,FP[1])){
                    cout<<"Backward succeed"<<endl;
                    string command = VTR_path + "/vpr/vpr " + VTR_path + "/vtr_flow/arch/timing/k6_frac_N10_mem32K_40nm.xml " + FP[1] + " --timing_report_detail aggregated --timing_report_npaths 10000";
                    sr.ExecuteCommand(command);
                    sr.RsltExtract(FP[1]);
                    cout << "=================================================================================="<<endl;
                    cout<<"iteration ends"<<endl;
                    chdir("..");
                    RsltFP = sr.RsltPrint();
                    benchmark_vtr(baselinePath,RsltFP);
                    return 0;
                }
                else{
                    if(sr.BlifForwardModify(path,FP[1])){
                        cout<<"Foward succeed"<<endl;
                        string command = VTR_path + "/vpr/vpr " + VTR_path + "/vtr_flow/arch/timing/k6_frac_N10_mem32K_40nm.xml " + FP[1] + " --timing_report_detail aggregated --timing_report_npaths 10000";
                        sr.ExecuteCommand(command);
                        sr.RsltExtract(FP[1]);
                        cout << "=================================================================================="<<endl;
                        cout<<"iteration ends"<<endl;
                        chdir("..");
                        RsltFP = sr.RsltPrint();
                        benchmark_vtr(baselinePath,RsltFP);
                        return 0;
                    }
                    else{
                        cout<<"iteration ends"<<endl;
                        chdir("..");
                        RsltFP =  sr.RsltPrint();
                        benchmark_vtr(baselinePath,RsltFP);
                        return 0;
                    }
                    
                }
            }   
            else if(sr.loadTimingRptFw()){
                cout<<"Forward succeed"<<endl;
                chdir("..");
                string temp = "../" + FP[0] + "/" + FP[1];
                const char* path = temp.c_str();
                FP = Test(filename,i);
                if(sr.BlifForwardModify(path,FP[1])){
                    string command = VTR_path + "/vpr/vpr " + VTR_path + "/vtr_flow/arch/timing/k6_frac_N10_mem32K_40nm.xml " + FP[1] + " --timing_report_detail aggregated --timing_report_npaths 10000";
                    sr.ExecuteCommand(command);
                    sr.RsltExtract(FP[1]);
                    cout << "=================================================================================="<<endl;
                    cout<<"iteration ends"<<endl;
                    chdir("..");
                    RsltFP = sr.RsltPrint();
                    benchmark_vtr(baselinePath,RsltFP);
                    return 0;
                }
                else{
                    cout<<"iteration ends"<<endl;
                    chdir("..");
                    RsltFP = sr.RsltPrint();
                    benchmark_vtr(baselinePath,RsltFP);
                    return 0;
                }
            }
            else{
                chdir("..");
                RsltFP = sr.RsltPrint();
                benchmark_vtr(baselinePath,RsltFP);
                return 0;
            }   
        }
        else{
            if(sr.loadTimingRptBw()){
                sr.loadTimingRptFw();
                chdir("..");
                string temp = "../" + FP[0] + "/" + FP[1];
                const char* path = temp.c_str();
                cout << temp <<endl;
                FP = Test(filename,i);
                if(sr.BlifBackwardModify(path,FP[1])){
                    cout<<"Test Backward succeed"<<endl;
                    string command = VTR_path + "/vpr/vpr " + VTR_path + "/vtr_flow/arch/timing/k6_frac_N10_mem32K_40nm.xml " + FP[1] + " --timing_report_detail aggregated --timing_report_npaths 10000";
                    sr.ExecuteCommand(command);
                    sr.RsltExtract(FP[1]);
                    cout << "=================================================================================="<<endl;
                }
                else{
                    if(sr.BlifForwardModify(path,FP[1])){
                        cout<<"Foward succeed"<<endl;
                        string command = VTR_path + "/vpr/vpr " + VTR_path + "/vtr_flow/arch/timing/k6_frac_N10_mem32K_40nm.xml " + FP[1] + " --timing_report_detail aggregated --timing_report_npaths 10000";
                        sr.ExecuteCommand(command);
                        sr.RsltExtract(FP[1]);
                        cout << "=================================================================================="<<endl;
                    }
                    else{
                        chdir("..");
                        RsltFP =  sr.RsltPrint();
                        benchmark_vtr(baselinePath,RsltFP);
                        return 0;
                    }
                    
                }
            }   
            else if(sr.loadTimingRptFw()){
                cout<<"Forward succeed"<<endl;
                chdir("..");
                string temp = "../" + FP[0] + "/" + FP[1];
                const char* path = temp.c_str();
                FP = Test(filename,i);
                if(sr.BlifForwardModify(path,FP[1])){
                    string command = VTR_path + "/vpr/vpr " + VTR_path + "/vtr_flow/arch/timing/k6_frac_N10_mem32K_40nm.xml " + FP[1] + " --timing_report_detail aggregated --timing_report_npaths 10000";
                    sr.ExecuteCommand(command);
                    sr.RsltExtract(FP[1]);
                    cout << "=================================================================================="<<endl;
                }
                else{
                    chdir("..");
                    RsltFP = sr.RsltPrint();
                    benchmark_vtr(baselinePath,RsltFP);
                    return 0;
                }
            }
            else{
                chdir("..");
                RsltFP = sr.RsltPrint();
                benchmark_vtr(baselinePath,RsltFP);
                return 0;
            }   
        }   
    }

}