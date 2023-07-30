/**
 * @file SR.cpp
 * @brief Iterative retiming
 * @details use iterative retiming to optimize netlist
 * @author kxzhu
 * @version 1.0
 * @date 2023-03-13
*/


#include<cstdlib>
#include<string>
#include<cstdio>
#include<cstring>
#include<iostream>
#include<sstream>
#include<fstream>
#include<algorithm>
#include<deque>
#include "SR.hpp"
using namespace std;

const int N = 300;

/**
 * @brief Test if the character a is equal to b.
 * 
 * @param a 
 * @param b 
 * @return true 
 * @return false 
 */
bool eq(char* a,char* b) //a is longer
{
	bool flag = true;
	for(unsigned i=0; b[i]!=0; i++)
		if(a[i] != b[i])
			flag = false;
	return flag;
}

/**
 * @brief Test if the character a is in b.
 * 
 * @param a 
 * @param b 
 * @return true 
 * @return false 
 */
bool in(char* a,char* b) //a is longer
{
    
	if(std::strstr(a,b) != NULL){    //Strstr says does b contain a
      return true;
    } 
    return false;
}


/**
 * @brief Test if the character a is exactly in string b.
 * 
 * @param a 
 * @param b 
 * @return true 
 * @return false 
 */
bool in_exact(char* a,string b) //a is longer
{
    vector<string> res;
    string result;
    string tempstring = a;
    stringstream input(tempstring);
    while(input>>result)
        res.push_back(result);
    for(int i=0;i<res.size();i++){
        res[i].erase(remove(res[i].begin(),res[i].end(),'\n'),res[i].end());
        if(res[i] == b){
            return true;
        }
    }
    return false;
}


/**
 * @brief Compare the second element of the pair.
 * 
 * @param a 
 * @param b
 * @return true 
 * @return false 
 */
bool cmp(pair<string,float> a,pair<string,float> b)
{
    return a.first < b.first;
}

/**
 * @brief Execute command.
 * 
 * @param command 
 * @return void
 */
void SR::ExecuteCommand(string command){
    char line[N];
    FILE *fp;
    string cmd = command;
    
    // 系统调用
    const char *sysCommand = cmd.data();
    if ((fp = popen(sysCommand, "r")) == NULL) {
        cout << "error" << endl;
        return;
    }
    while (fgets(line, sizeof(line)-1, fp) != NULL){
        //cout << line ;
    }
    pclose(fp);
}


/**
 * @brief Parse the timing rpt for the forward retiming.
 * 
 * @param  
 * @return true 
 * @return false 
 */
bool SR::loadTimingRptFw(){
    FILE* infile;
    char buffer[300], *temps;
    infile = fopen("./report_timing.setup.rpt","r");
    if(infile == NULL){	
        cout <<"cannot open the timing Rpt file report_timing.setup.rpt\n"; 
        exit(0);	
    }
    cout<<"Start loading timing information from timing rpt."<<endl;
    while(1){
        fgets(buffer,300,infile);
        if(eq(buffer,(char*)"Startpoint")&& in_exact(buffer,"(.input")){
            cout<< "SR forward to input, need to stop"<<endl;
            fclose(infile);
            return false;
        }
        else{
            if(in(buffer,(char*)".latch") && in(buffer,(char*)"clk[0]")){    
                temps = strtok(buffer,".");
                cout<<"ForwardFF: "<<temps<<endl;
                forwardFF = temps;
            }
            if(in(buffer,(char*)".names")){
                temps = strtok(buffer,".");
                cout<<"ForwardLut: "<<temps<<endl;
                forwardLUT = temps;
                //input = temps;
                fclose(infile);
                return true;
                break;
            }
        }
    }
}

/**
 * @brief Parse the timing rpt for the backward retiming.
 *  
 * @return true 
 * @return false 
 */
bool SR::loadTimingRptBw(){
    FILE* infile;
    char buffer[300], *temps;
    infile = fopen("./report_timing.setup.rpt","r");
    if(infile == NULL){	
        cout <<"cannot open the timing Rpt file report_timing.setup.rpt\n"; 
        exit(0);	
    }
    
    string Backwardff,Backwardlut;
    cout<<"Start loading timing information from timing rpt."<<endl;
    while(1){
        fgets(buffer,300,infile);
        if(eq(buffer,(char*)"Endpoint")&& in_exact(buffer,"(.output")){
            cout<< "SR Backward to output, need to stop"<<endl;
            return false;
        }
        else{
            if(in(buffer,(char*)"Endpoint")){
                temps = strtok(buffer," ");
                int count(0);
                while(temps!=NULL){
                    if(count == 2 ){
                        Backwardff = temps;
                    }
                    temps = strtok(NULL," ");
                    count ++;
                }
                
            }
            if(in(buffer,(char*)".names")){
                temps = strtok(buffer,".");
                Backwardlut = temps;
            }
            if(in(buffer,(char*)".latch") && in_exact(buffer,Backwardff) && !in(buffer,(char*)"Endpoint")){    
                temps = strtok(buffer,".");
                cout<< "Backwardlut: " <<Backwardlut<<endl;
                cout<< "BackwardFF: " <<temps<<endl;
                backwardLUT = Backwardlut;
                backwardFF = temps;
                fclose(infile);
                return true;
                break;    
            }            
        }       
    }
}


/**
 * @brief Modify the netlist for the forward retiming.
 * 
 * @param fileName 
 * @param ofName 
 * @return true 
 * @return false 
 */
bool SR::BlifForwardModify(const char* fileName,string ofName){
    FILE* infile;
    ofstream outfile;

    
    string TempFFInput_0,TempFFInput_1,TempFFInput_2,TempFFInput_3,forwardFFInput;
    vector<string> sharelutInput;
    vector<string> blifInput;
    char buffer[300], *temps;
    
    infile = fopen(fileName,"r");
    
    
    cout << "new file is " << ofName <<endl;
    outfile.open(ofName);
    if(infile == NULL){	
        cout <<"cannot open the Blif file ["<<fileName<<"]\n"; 
        exit(0);	
    }
    while(1){
        //Find all blif input points for subsequent judgment
        fgets(buffer,300,infile);
        if(eq(buffer,(char*)".input")){    
            int count(0);
            temps = strtok(buffer," ");
            while(temps!=NULL){
                if(count > 0 ){
                    string tempinput = temps;
                    tempinput.erase(remove(tempinput.begin(),tempinput.end(),'\n'),tempinput.end());
                    blifInput.push_back(tempinput);
                }
                temps = strtok(NULL," ");
                count ++;
            }
        }
        //Find the input of the critical path starting point ff
        else if(eq(buffer,(char*)".latch")&&in_exact(buffer,forwardFF)){
            int count(0);
            temps = strtok(buffer," ");
            while(temps!=NULL){
                if(count == 1){
                    forwardFFInput = temps;
                    cout <<"forwardFFInput is "<< temps << endl;
                }
                temps = strtok(NULL," ");
                count ++;
            }
        }
        //Find the common input ff of the LUT connected to ff on the critical path
        else if(eq(buffer,(char*)".names")&&in_exact(buffer,forwardFF)&&in_exact(buffer,forwardLUT)){
            temps = strtok(buffer," ");
            vector<string> templutnames;
            while(temps!=NULL){
                string tempinput = temps;
                tempinput.erase(remove(tempinput.begin(),tempinput.end(),'\n'),tempinput.end());
                templutnames.push_back(tempinput);    
                temps = strtok(NULL," ");                
            }
            if(templutnames.back() == forwardLUT) {
                for(int i = 0;i<templutnames.size();i++){
                    if(templutnames[i] != forwardFF && templutnames[i] != forwardLUT && templutnames[i] != ".names"){
                       
                    sharelutInput.push_back(templutnames[i]);
                    cout << templutnames[i] <<endl;
                    }
                }
                
            }
        }
        if(eq(buffer,(char*)".end")){
            break;     
        }
    }
    rewind(infile);

    //Record all ff names for subsequent judgment
    vector<string> latchList;
    //Record the number of common inputs
    int inputNum = sharelutInput.size();

    while(1){
        fgets(buffer,300,infile);
        if(eq(buffer,(char*)".latch")){    
            int count(0);
            temps = strtok(buffer," ");
            while(temps!=NULL){
                if(count == 2 ){
                    //cout<<temps<<endl;
                    latchList.push_back(temps);
                }
                temps = strtok(NULL," ");
                count ++;
            }
        }
        if(eq(buffer,(char*)".end")){
            break;     
        }
    }


    rewind(infile);
    cout<<"rewind"<<endl;
    //Determine whether the common input of lut is the input of blif
    if(inputNum == 1){
        if(find(blifInput.begin(),blifInput.end(),sharelutInput[0]) != blifInput.end()){
            cout<<"share lut input is blif input,cannot retiming"<<endl;
            return false;
        }
        else if(find(latchList.begin(),latchList.end(),sharelutInput[0]) != latchList.end()){
            cout<<"share lut input is ff output, legal retiming!"<<endl;
            while(1){
                fgets(buffer,300,infile);
                //Find the old ff and its input, and perform forward retiming
                if(eq(buffer,(char*)".latch")&&in_exact(buffer,forwardFF)){

                    outfile << "#" <<buffer;
                    int count(0);
                    temps = strtok(buffer," ");
                    while(temps!=NULL){
                        if(count == 1){
                            forwardFFInput = temps;
                            cout << forwardFFInput <<endl;
                        }
                        temps = strtok(NULL," ");
                        count ++;
                    }
                    outfile << ".latch    "<< forwardFFInput << "new " << forwardFF << "new re clock 2"<<endl;   
                }
                //Find the commonly entered ff and delete it
                else if(eq(buffer,(char*)".latch")&&in_exact(buffer,(char*)sharelutInput[0].data())){
                    outfile << "#" <<buffer; 
                    int count(0);
                    temps = strtok(buffer," ");
                    while(temps!=NULL){
                        if(count == 1){
                            TempFFInput_0 = temps;
                            cout <<"TempFFInput_0"<< TempFFInput_0 <<endl;
                        }
                        temps = strtok(NULL," ");
                        count ++;
                    }
                }
                //Find the LUT connected to the original critical path ff and replace the output with a new one
                else if(eq(buffer,(char*)".names")&&in_exact(buffer,forwardFF)&&in_exact(buffer,sharelutInput[0])&&in_exact(buffer,forwardLUT)){
                    int count(0);
                    temps = strtok(buffer," ");
                    while(temps!=NULL){
                        if(temps == sharelutInput[0]){
                            outfile << TempFFInput_0 <<" ";
                        }
                        else if(temps == forwardFF){
                            outfile << forwardFFInput <<" ";
                        }
                        else if(temps == forwardLUT){
                            outfile << forwardFFInput <<"new" << endl;
                        }
                        else{
                            outfile << temps << " ";
                        }
                        temps = strtok(NULL," ");
                        count ++;
                    }
                    //outfile << ".names " << forwardFFInput << " " << TempFFInput << " " << forwardFFInput << "new" << endl;
                }
                else if(in_exact(buffer,sharelutInput[0])&&in_exact(buffer,forwardFF)){
                    temps = strtok(buffer," ");
                    while(temps!=NULL){
                        if(temps == sharelutInput[0]){
                            outfile << TempFFInput_0 <<" ";
                        }
                        else if(temps == forwardFF){
                            outfile << forwardFFInput << " ";
                        }
                        else if (eq(temps,(char*)".latch")){
                            outfile << temps << "    ";
                        }
                        else{
                            outfile << temps << " ";
                        }
                        temps = strtok(NULL," ");

                    }
                }
                //Update the port of the LUT to which the common input ff is connected
                else if(in_exact(buffer,sharelutInput[0])){
                    temps = strtok(buffer," ");
                    while(temps!=NULL){
                        if(temps == sharelutInput[0]){
                            outfile << TempFFInput_0 <<" ";
                        }
                        else if (eq(temps,(char*)".latch")){
                            outfile << temps << "    ";
                        }
                        else{
                            outfile << temps << " ";
                        }
                        temps = strtok(NULL," ");

                    }
                }
                //Update the lut port connected to the critical path ff
                else if(in_exact(buffer,forwardFF)){
                    temps = strtok(buffer," ");
                    while(temps!=NULL){
                        if(temps == forwardFF){
                            outfile << forwardFFInput << " ";
                        }
                        else if (eq(temps,(char*)".latch")){
                            outfile << temps << "    ";
                        }
                        else{
                            outfile << temps << " ";
                        }
                        temps = strtok(NULL," ");    
                    }
                }
                //Update the output of ff after retiming to lut
                else if(eq(buffer,(char*)".names")&&in_exact(buffer,forwardLUT)){
                    temps = strtok(buffer," ");
                    while(temps!=NULL){
                        if(temps == forwardLUT){
                            outfile << forwardFF << "new ";
                        }
                        else{
                            outfile << temps << " ";
                        }
                        temps = strtok(NULL," ");    
                    }
                }
                //Normal output under other conditions
                else{
                    outfile << buffer;
                }
                if(eq(buffer,(char*)".end")){
                    outfile.close();
                    break;     
                }
            }
            return true;
        }
        else{
            cout<<"share lut input is lut output,illegal retiming"<<endl;
            return false;
        }
    }
    if(inputNum == 2){
        if(find(blifInput.begin(),blifInput.end(),sharelutInput[0]) != blifInput.end()||find(blifInput.begin(),blifInput.end(),sharelutInput[1]) != blifInput.end()){
            cout<<"Two share lut input is blif input,cannot retiming"<<endl;
            return false;
        }
        else if(find(latchList.begin(),latchList.end(),sharelutInput[0]) != latchList.end() && find(latchList.begin(),latchList.end(),sharelutInput[1]) != latchList.end()){
            cout<<"Two share lut input is ff output, legal retiming!"<<endl;
            while(1){
                fgets(buffer,300,infile);
                //Find the old ff and its input, and perform forward retiming
                if(eq(buffer,(char*)".latch")&&in_exact(buffer,forwardFF)){

                    outfile << "#" <<buffer;
                    int count(0);
                    temps = strtok(buffer," ");
                    while(temps!=NULL){
                        if(count == 1){
                            forwardFFInput = temps;
                            cout <<"forwardFFInput"<< forwardFFInput <<endl;
                        }
                        temps = strtok(NULL," ");
                        count ++;
                    }
                    outfile << ".latch    "<< forwardFFInput << "new " << forwardFF << "new re clock 2"<<endl;   
                }
                //Find the commonly entered ff and delete it
                else if(eq(buffer,(char*)".latch")&&in_exact(buffer,(char*)sharelutInput[0].data())){
                    outfile << "#" <<buffer; 
                    int count(0);
                    temps = strtok(buffer," ");
                    while(temps!=NULL){
                        if(count == 1){
                            TempFFInput_0 = temps;
                            cout <<"TempFFInput_0"<< TempFFInput_0 <<endl;
                        }
                        temps = strtok(NULL," ");
                        count ++;
                    }
                }
                else if(eq(buffer,(char*)".latch")&&in_exact(buffer,(char*)sharelutInput[1].data())){
                    outfile << "#" <<buffer; 
                    int count(0);
                    temps = strtok(buffer," ");
                    while(temps!=NULL){
                        if(count == 1){
                            TempFFInput_1 = temps;
                            cout <<"TempFFInput_1" << TempFFInput_1 <<endl;
                        }
                        temps = strtok(NULL," ");
                        count ++;
                    }
                }
                else if(eq(buffer,(char*)".names")&&in_exact(buffer,forwardFF)&&in_exact(buffer,sharelutInput[0])&&in_exact(buffer,sharelutInput[1])&&in_exact(buffer,forwardLUT)){
                    int count(0);
                    temps = strtok(buffer," ");
                    while(temps!=NULL){
                        if(temps == sharelutInput[0]){
                            outfile << TempFFInput_0 <<" ";
                        }
                        else if(temps == sharelutInput[1]){
                            outfile << TempFFInput_1 <<" ";
                        }
                        else if(temps == forwardFF){
                            outfile << forwardFFInput <<" ";
                        }
                        else if(temps == forwardLUT){
                            outfile << forwardFFInput <<"new" << endl;
                        }
                        else{
                            outfile << temps << " ";
                        }
                        temps = strtok(NULL," ");
                        count ++;
                    }
                }
                else if(in_exact(buffer,sharelutInput[0])&&in_exact(buffer,sharelutInput[1])&&in_exact(buffer,forwardFF)){
                    temps = strtok(buffer," ");
                    while(temps!=NULL){
                        if(temps == sharelutInput[0]){
                            outfile << TempFFInput_0 <<" ";
                        }
                        else if(temps == sharelutInput[1]){
                            outfile << TempFFInput_1 <<" ";
                        }
                        else if(temps == forwardFF){
                            outfile << forwardFFInput << " ";
                        }
                        else{
                            outfile << temps << " ";
                        }
                        temps = strtok(NULL," ");

                    }
                }
                else if(in_exact(buffer,sharelutInput[0])&&in_exact(buffer,sharelutInput[1])){
                    temps = strtok(buffer," ");
                    while(temps!=NULL){
                        if(temps == sharelutInput[0]){
                            outfile << TempFFInput_0 <<" ";
                        }
                        else if(temps == sharelutInput[1]){
                            outfile << TempFFInput_1 <<" ";
                        }
                        else if (eq(temps,(char*)".latch")){
                            outfile << temps << "    ";
                        }
                        else{
                            outfile << temps << " ";
                        }
                        temps = strtok(NULL," ");

                    }
                }
                else if(in_exact(buffer,sharelutInput[0])){
                    temps = strtok(buffer," ");
                    while(temps!=NULL){
                        if(temps == sharelutInput[0]){
                            outfile << TempFFInput_0 <<" ";
                        }
                        else if (eq(temps,(char*)".latch")){
                            outfile << temps << "    ";
                        }
                        else{
                            outfile << temps << " ";
                        }
                        temps = strtok(NULL," ");

                    }
                }
                else if(in_exact(buffer,sharelutInput[1])){
                    temps = strtok(buffer," ");
                    while(temps!=NULL){
                        if(temps == sharelutInput[1]){
                            outfile << TempFFInput_1 <<" ";
                        }
                        else if (eq(temps,(char*)".latch")){
                            outfile << temps << "    ";
                        }
                        else{
                            outfile << temps << " ";
                        }
                        temps = strtok(NULL," ");

                    }
                }
                else if(in_exact(buffer,forwardFF)){
                    temps = strtok(buffer," ");
                    while(temps!=NULL){
                        if(temps == forwardFF){
                            outfile << forwardFFInput << " ";
                        }
                        else if (eq(temps,(char*)".latch")){
                            outfile << temps << "    ";
                        }
                        else{
                            outfile << temps << " ";
                        }
                        temps = strtok(NULL," ");    
                    }
                }
                else if(eq(buffer,(char*)".names")&&in_exact(buffer,forwardLUT)){
                    temps = strtok(buffer," ");
                    while(temps!=NULL){
                        if(temps == forwardLUT){
                            outfile << forwardFF << "new ";
                        }
                        else if (eq(temps,(char*)".latch")){
                            outfile << temps << "    ";
                        }
                        else{
                            outfile << temps << " ";
                        }
                        temps = strtok(NULL," ");    
                    }
                }
                
                else{
                    outfile << buffer;
                }
                if(eq(buffer,(char*)".end")){
                    outfile.close();
                    break;     
                }
            }
            return true;
        }
        else{
            cout<<"Two share lut input is lut output,illegal retiming"<<endl;
            return false;
        }
    }
    if(inputNum == 3){
        if(find(blifInput.begin(),blifInput.end(),sharelutInput[0]) != blifInput.end()||find(blifInput.begin(),blifInput.end(),sharelutInput[1]) != blifInput.end()
        ||find(blifInput.begin(),blifInput.end(),sharelutInput[2]) != blifInput.end()){
            cout<<"Three share lut input is blif input,cannot retiming"<<endl;
            return false;
        }
        //The common input of lut is ff
        else if(find(latchList.begin(),latchList.end(),sharelutInput[0]) != latchList.end() && find(latchList.begin(),latchList.end(),sharelutInput[1]) != latchList.end()
        && find(latchList.begin(),latchList.end(),sharelutInput[2]) != latchList.end()){
            cout<<"Three share lut input is ff output, legal retiming!"<<endl;
            
            while(1){

                fgets(buffer,300,infile);
                if(eq(buffer,(char*)".latch")&&in_exact(buffer,forwardFF)){

                    outfile << "#" <<buffer;
                    int count(0);
                    temps = strtok(buffer," ");
                    while(temps!=NULL){
                        if(count == 1){
                            forwardFFInput = temps;
                            cout <<"forwardFFInput"<< forwardFFInput <<endl;
                        }
                        temps = strtok(NULL," ");
                        count ++;
                    }
                    outfile << ".latch    "<< forwardFFInput << "new " << forwardFF << "new re clock 2"<<endl;   
                }
                else if(eq(buffer,(char*)".latch")&&in_exact(buffer,(char*)sharelutInput[0].data())){
                    outfile << "#" <<buffer; 
                    int count(0);
                    temps = strtok(buffer," ");
                    while(temps!=NULL){
                        if(count == 1){
                            TempFFInput_0 = temps;
                            cout <<"TempFFInput_0" << TempFFInput_0 <<endl;
                        }
                        temps = strtok(NULL," ");
                        count ++;
                    }
                }
                else if(eq(buffer,(char*)".latch")&&in_exact(buffer,(char*)sharelutInput[1].data())){
                    outfile << "#" <<buffer; 
                    int count(0);
                    temps = strtok(buffer," ");
                    while(temps!=NULL){
                        if(count == 1){
                            TempFFInput_1 = temps;
                            cout <<"TempFFInput_1" <<TempFFInput_1 <<endl;
                        }
                        temps = strtok(NULL," ");
                        count ++;
                    }
                }
                else if(eq(buffer,(char*)".latch")&&in_exact(buffer,(char*)sharelutInput[2].data())){
                    outfile << "#" <<buffer; 
                    int count(0);
                    temps = strtok(buffer," ");
                    while(temps!=NULL){
                        if(count == 1){
                            TempFFInput_2 = temps;
                            cout <<"TempFFInput_2" << TempFFInput_2 <<endl;
                        }
                        temps = strtok(NULL," ");
                        count ++;
                    }
                }
                else if(eq(buffer,(char*)".names")&&in_exact(buffer,forwardFF)&&in_exact(buffer,sharelutInput[0])&&in_exact(buffer,sharelutInput[1])
                &&in_exact(buffer,sharelutInput[2])&&in_exact(buffer,forwardLUT)){
                    int count(0);
                    temps = strtok(buffer," ");
                    while(temps!=NULL){
                        if(temps == sharelutInput[0]){
                            outfile << TempFFInput_0 <<" ";
                        }
                        else if(temps == sharelutInput[1]){
                            outfile << TempFFInput_1 <<" ";
                        }
                        else if(temps == sharelutInput[2]){
                            outfile << TempFFInput_2 <<" ";
                        }
                        else if(temps == forwardFF){
                            outfile << forwardFFInput <<" ";
                        }
                        else if(temps == forwardLUT){
                            outfile << forwardFFInput <<"new" << endl;
                        }
                        else{
                            outfile << temps << " ";
                        }
                        temps = strtok(NULL," ");
                        count ++;
                    }
                    //outfile << ".names " << forwardFFInput << " " << TempFFInput << " " << forwardFFInput << "new" << endl;
                }
                else if(in_exact(buffer,sharelutInput[0])&&in_exact(buffer,sharelutInput[1])&&in_exact(buffer,sharelutInput[2])&&in_exact(buffer,forwardFF)){
                    temps = strtok(buffer," ");
                    while(temps!=NULL){
                        if(temps == sharelutInput[0]){
                            outfile << TempFFInput_0 <<" ";
                        }
                        else if(temps == sharelutInput[1]){
                            outfile << TempFFInput_1 <<" ";
                        }
                        else if(temps == sharelutInput[2]){
                            outfile << TempFFInput_2 <<" ";
                        }
                        else if(temps == forwardFF){
                            outfile << forwardFFInput << " ";
                        }
                        else{
                            outfile << temps << " ";
                        }
                        temps = strtok(NULL," ");

                    }
                }
                else if(in_exact(buffer,sharelutInput[0])&&in_exact(buffer,sharelutInput[1])&&in_exact(buffer,sharelutInput[2])){
                    temps = strtok(buffer," ");
                    while(temps!=NULL){
                        if(temps == sharelutInput[0]){
                            outfile << TempFFInput_0 <<" ";
                        }
                        else if(temps == sharelutInput[1]){
                            outfile << TempFFInput_1 <<" ";
                        }
                        else if(temps == sharelutInput[2]){
                            outfile << TempFFInput_2 <<" ";
                        }
                        else{
                            outfile << temps << " ";
                        }
                        temps = strtok(NULL," ");

                    }
                }
                else if(in_exact(buffer,sharelutInput[0])){
                    temps = strtok(buffer," ");
                    while(temps!=NULL){
                        if(temps == sharelutInput[0]){
                            outfile << TempFFInput_0 <<" ";
                        }
                        else if (eq(temps,(char*)".latch")){
                            outfile << temps << "    ";
                        }
                        else{
                            outfile << temps << " ";
                        }
                        temps = strtok(NULL," ");

                    }
                }
                else if(in_exact(buffer,sharelutInput[1])){
                    temps = strtok(buffer," ");
                    while(temps!=NULL){
                        if(temps == sharelutInput[1]){
                            outfile << TempFFInput_1 <<" ";
                        }
                        else if (eq(temps,(char*)".latch")){
                            outfile << temps << "    ";
                        }
                        else{
                            outfile << temps << " ";
                        }
                        temps = strtok(NULL," ");

                    }
                }
                else if(in_exact(buffer,sharelutInput[2])){
                    temps = strtok(buffer," ");
                    while(temps!=NULL){
                        if(temps == sharelutInput[2]){
                            outfile << TempFFInput_2 <<" ";
                        }
                        else if (eq(temps,(char*)".latch")){
                            outfile << temps << "    ";
                        }
                        else{
                            outfile << temps << " ";
                        }
                        temps = strtok(NULL," ");

                    }
                }
                
                else if(in_exact(buffer,forwardFF)){
                    temps = strtok(buffer," ");
                    while(temps!=NULL){
                        if(temps == forwardFF){
                            outfile << forwardFFInput << " ";
                        }
                        else if (eq(temps,(char*)".latch")){
                            outfile << temps << "    ";
                        }
                        else{
                            outfile << temps << " ";
                        }
                        temps = strtok(NULL," ");    
                    }
                }
                
                else if(eq(buffer,(char*)".names")&&in_exact(buffer,forwardLUT)){
                    temps = strtok(buffer," ");
                    while(temps!=NULL){
                        if(temps == forwardLUT){
                            outfile << forwardFF << "new ";
                        }
                        else{
                            outfile << temps << " ";
                        }
                        temps = strtok(NULL," ");    
                    }
                }
                
                else{
                    outfile << buffer;
                }
                if(eq(buffer,(char*)".end")){
                    outfile.close();
                    break;     
                }
            }
            return true;

        }
        //The common input of lut is lut, which is the output of shareLutinput as. names
        else{
            cout<<"Three share lut input is lut output,illegal retiming"<<endl;
            return false;
        }
    }
    return false;
}

/**
 * @brief Modify the netlist for the forward retiming.
 * 
 * @param fileName 
 * @param ofName 
 * @return true 
 * @return false 
 */
bool SR::BlifBackwardModify(const char* fileName,string ofName){
    FILE* infile;
    ofstream outfile;

    //Record the lut input at the end of the critical path
    string backwardFFInput,LutInput_0,LutInput_1,LutInput_2,LutInput_3;
    vector<string> tempLutInput;
    int inputNum;

    vector<string> blifInput,blifOutput;
    char buffer[3000], *temps;
    infile = fopen(fileName,"r");
    
    cout << "new file is " << ofName <<endl;
    outfile.open(ofName);
    if(infile == NULL){	
        cout <<"cannot open the Blif file ["<<fileName<<"]\n"; 
        exit(0);	
    }
    while(1){
        //Find all blif input points for subsequent judgment
        fgets(buffer,3000,infile);
        if(eq(buffer,(char*)".inputs")){
            int count(0);
            temps = strtok(buffer," ");
            while(temps!=NULL){
                if(count > 0 ){
                    string tempoutput = temps;
                    tempoutput.erase(remove(tempoutput.begin(),tempoutput.end(),'\n'),tempoutput.end());
                    blifInput.push_back(tempoutput);
                    //cout << tempoutput<<endl;
                }
                temps = strtok(NULL," ");
                count ++;
            }
        }
        //Find the output points of all blifs for subsequent judgment
        if(eq(buffer,(char*)".outputs")){    
            int count(0);
            temps = strtok(buffer," ");
            while(temps!=NULL){
                if(count > 0 ){
                    string tempoutput = temps;
                    tempoutput.erase(remove(tempoutput.begin(),tempoutput.end(),'\n'),tempoutput.end());
                    blifOutput.push_back(tempoutput);
                    //cout << tempoutput<<endl;
                }
                temps = strtok(NULL," ");
                count ++;
            }
        }
        else if(eq(buffer,(char*)".latch")&&in_exact(buffer,backwardFF)){
            int count(0);
            temps = strtok(buffer," ");
            while(temps!=NULL){
                if(count == 1){
                    backwardFFInput = temps;
                    cout <<"backwardFFInput is "<< temps << endl;
                }
                temps = strtok(NULL," ");
                count ++;
            }
        }
        else if(eq(buffer,(char*)".names")&&in_exact(buffer,backwardLUT)){
            int count(0);
            
            temps = strtok(buffer," ");
            while(temps!=NULL){
                if(temps != ".names"){
                    //cout<<temps<<endl;
                    string templutinput = temps;
                    templutinput.erase(remove(templutinput.begin(),templutinput.end(),'\n'),templutinput.end());
                    //cout<<templutinput<<endl;
                    tempLutInput.push_back(templutinput);
                }         
                temps = strtok(NULL," ");
                count ++;
            }
            
            //Delete blank elements
            tempLutInput.erase(std::remove(tempLutInput.begin(),tempLutInput.end(),""),tempLutInput.end());
            
            inputNum = tempLutInput.size()-2;
            cout<<inputNum<<endl;
            if (inputNum == 2 && tempLutInput.back() == backwardLUT){
                LutInput_0 = tempLutInput[1];
                LutInput_1 = tempLutInput[2];
                cout<<"LutInput_0 "<<LutInput_0<<endl;
                cout<<"LutInput_1 "<<LutInput_1<<endl;
                break;   
            }
            else if (inputNum == 3 && tempLutInput.back() == backwardLUT){
                LutInput_0 = tempLutInput[1];
                LutInput_1 = tempLutInput[2];
                LutInput_2 = tempLutInput[3];
                cout<<"LutInput_0 "<<LutInput_0<<endl;
                cout<<"LutInput_1 "<<LutInput_1<<endl;
                cout<<"LutInput_2 "<<LutInput_2<<endl;
                break;  
            }
            else if (inputNum == 4 && tempLutInput.back() == backwardLUT){
                LutInput_0 = tempLutInput[1];
                LutInput_1 = tempLutInput[2];
                LutInput_2 = tempLutInput[3];
                LutInput_3 = tempLutInput[4];
                cout<<"LutInput_0 "<<LutInput_0<<endl;
                cout<<"LutInput_1 "<<LutInput_1<<endl;
                cout<<"LutInput_2 "<<LutInput_2<<endl;
                cout<<"LutInput_3 "<<LutInput_3<<endl;
                break; 
            }
            else{
                tempLutInput.clear();
            }
        }
        if(eq(buffer,(char*)".end")){
            break;     
        }
    }
    rewind(infile);
    cout<<"rewind"<<endl;
    cout<<inputNum<<endl;
    
    if(inputNum == 2){
        if(find(blifInput.begin(),blifInput.end(),LutInput_0)!= blifInput.end()){
            cout<<"LutInput_0 is blif input, illegal retiming!"<<endl;
            return false;
        }
        else if(find(blifInput.begin(),blifInput.end(),LutInput_1) != blifInput.end()){
            cout<<"LutInput_1 is blif input,illegal retiming!"<<endl;
            return false;
        }
        else{
            cout<<"All input is ff, legal retiming"<<endl;
            while(1){
                fgets(buffer,300,infile);
                //Find the old ff and its input, and perform forward retiming (found, for output: backwardFF and input: backwardLUT)
                if(eq(buffer,(char*)".latch")&&in_exact(buffer,(char*)backwardFF.data())&&in_exact(buffer,(char*)backwardLUT.data())){
                    cout<<"Both inputs to lut should be considered"<<endl;
                    outfile << "#" <<buffer;
                    //Both inputs to lut should be considered
                    outfile << ".latch    "<< backwardLUT << "new1 " << backwardFF << "new1 re clock 2"<<endl;
                    outfile << ".latch    "<< backwardLUT << "new2 " << backwardFF << "new2 re clock 2"<<endl;
                }
                else if(eq(buffer,(char*)".latch")&&in_exact(buffer,LutInput_0)){
                    temps = strtok(buffer," ");
                    while(temps!=NULL){
                        if(temps == LutInput_0){
                            outfile << backwardLUT << "new1 ";
                        }
                        else if(in(temps,(char*)"\n")){
                            outfile << temps;
                        }
                        else if(eq(temps,(char*)".latch")){
                            outfile << temps << "    ";
                        }
                        else{
                            outfile << temps << " ";
                        }
                        temps = strtok(NULL," "); 
                    }
                }
                else if(eq(buffer,(char*)".latch")&&in_exact(buffer,LutInput_1)){
                    temps = strtok(buffer," ");
                    while(temps!=NULL){
                        if(temps == LutInput_1){
                            outfile << backwardLUT << "new2 ";
                        }
                        else if(in(temps,(char*)"\n")){
                            outfile << temps;
                        }
                        else if(eq(temps,(char*)".latch")){
                            outfile << temps << "    ";
                        }
                        else{
                            outfile << temps << " ";
                        }
                        temps = strtok(NULL," "); 
                    }
                }
                else if(eq(buffer,(char*)".names")&&in_exact(buffer,LutInput_0)&&in_exact(buffer,LutInput_1)&&in(buffer,(char*)backwardLUT.data())){
                    temps = strtok(buffer," ");
                    while(temps!=NULL){
                        if(temps == LutInput_0){
                            outfile << backwardFF << "new1 ";
                        }
                        else if(temps == LutInput_1){
                            outfile << backwardFF << "new2 ";
                        }
                        else if(eq(temps,(char*)backwardLUT.data())){
                            
                            outfile << backwardFF<<endl;
                        }
                        else{
                            outfile << temps << " ";
                        }
                        temps = strtok(NULL," ");    
                    }
                }
                else if(eq(buffer,(char*)".names")&&in_exact(buffer,LutInput_0)&&in_exact(buffer,LutInput_1)){
                    temps = strtok(buffer," ");
                    while(temps!=NULL){
                        if(temps == LutInput_0 && backwardFF != LutInput_0){
                            outfile << backwardLUT << "new1 ";
                        }
                        else if(temps == LutInput_1 && backwardFF != LutInput_1){
                            outfile << backwardLUT << "new2 ";
                        }
                        else{
                            outfile << temps << " ";
                        }
                        temps = strtok(NULL," ");    
                    }
                }
                else if(eq(buffer,(char*)".names")&&in(buffer,(char*)LutInput_0.data())){
                    temps = strtok(buffer," ");
                    while(temps!=NULL){
                        if(temps == LutInput_0 && backwardFF != LutInput_0){
                            outfile << backwardLUT << "new1 ";
                        }
                        else if(temps == LutInput_0 + "\n"&& backwardFF != LutInput_0){
                            outfile << backwardLUT << "new1 " << endl;
                        }
                        else{
                            outfile << temps << " ";
                        }
                        temps = strtok(NULL," ");    
                    }
                }
                else if(eq(buffer,(char*)".names")&&in(buffer,(char*)LutInput_1.data())){
                    temps = strtok(buffer," ");
                    while(temps!=NULL){
                        if(temps == LutInput_1 && backwardFF != LutInput_1){
                            outfile << backwardLUT << "new2 ";
                        }
                        else if(temps == LutInput_1 + "\n" && backwardFF != LutInput_1){
                            outfile << backwardLUT << "new2 " << endl;
                        }
                        else{
                            outfile << temps << " ";
                        }
                        temps = strtok(NULL," ");    
                    }
                }
                //Update the port of ff to be deleted to lut
                else if(eq(buffer,(char*)".names")&&in_exact(buffer,backwardLUT)){
                    temps = strtok(buffer," ");
                    while(temps!=NULL){
                        if(temps == backwardLUT){
                            outfile << backwardFF << " ";
                        }
                        else{
                            outfile << temps << " ";
                        }
                        temps = strtok(NULL," ");    
                    }
                }
                //Normal output under other conditions
                else{
                    //cout<<"normal output"<<buffer<<endl;
                    outfile << buffer;
                }
                if(eq(buffer,(char*)".end")){
                    outfile.close();
                    break;     
                }
            }
        }
    }
    if(inputNum == 3){
        if(find(blifInput.begin(),blifInput.end(),LutInput_0)!= blifInput.end()){
            cout<<"LutInput_0 is blif input, illegal retiming!"<<endl;
            return false;
        }
        else if(find(blifInput.begin(),blifInput.end(),LutInput_1) != blifInput.end()){
            cout<<"LutInput_1 is blif input,illegal retiming!"<<endl;
            return false;
        }
        else if(find(blifInput.begin(),blifInput.end(),LutInput_2) != blifInput.end()){
            cout<<"LutInput_2 is blif input,illegal retiming!"<<endl;
            return false;
        }
        else{
            cout<<"All input is ff, legal retiming"<<endl;
            while(1){
                fgets(buffer,300,infile);
                //Find the old ff and its input, and perform forward retiming (found, for output: backwardFF and input: backwardLUT)
                if(eq(buffer,(char*)".latch")&&in_exact(buffer,(char*)backwardFF.data())&&in_exact(buffer,(char*)backwardLUT.data())){
                    cout<<"All three inputs to lut should be considered"<<endl;
                    outfile << "#" <<buffer;
                    outfile << ".latch    "<< backwardLUT << "new1 " << backwardFF << "new1 re clock 2"<<endl;
                    outfile << ".latch    "<< backwardLUT << "new2 " << backwardFF << "new2 re clock 2"<<endl;
                    outfile << ".latch    "<< backwardLUT << "new3 " << backwardFF << "new3 re clock 2"<<endl;
                    }
                else if(eq(buffer,(char*)".latch")&&in_exact(buffer,LutInput_0)){
                    temps = strtok(buffer," ");
                    while(temps!=NULL){
                        if(temps == LutInput_0){
                            outfile << backwardLUT << "new1 ";
                        }
                        else if(in(temps,(char*)"\n")){
                            outfile << temps;
                        }
                        else if(eq(temps,(char*)".latch")){
                            outfile << temps << "    ";
                        }
                        else{
                            outfile << temps << " ";
                        }
                        temps = strtok(NULL," "); 
                    }
                }
                else if(eq(buffer,(char*)".latch")&&in_exact(buffer,LutInput_1)){
                    temps = strtok(buffer," ");
                    while(temps!=NULL){
                        if(temps == LutInput_1){
                            outfile << backwardLUT << "new2 ";
                        }
                        else if(eq(temps,(char*)".latch")){
                            outfile << temps << "    ";
                        }
                        else if(in(temps,(char*)"\n")){
                            outfile << temps;
                        }
                        else{
                            outfile << temps << " ";
                        }
                        temps = strtok(NULL," "); 
                    }
                }
                else if(eq(buffer,(char*)".latch")&&in_exact(buffer,LutInput_2)){
                    temps = strtok(buffer," ");
                    while(temps!=NULL){
                        if(temps == LutInput_2){
                            outfile << backwardLUT << "new3 ";
                        }
                        else if(in(temps,(char*)"\n")){
                            outfile << temps;
                        }
                        
                        else if(eq(temps,(char*)".latch")){
                            outfile << temps << "    ";
                        }
                        else{
                            outfile << temps << " ";
                        }
                        temps = strtok(NULL," "); 
                    }
                }
                else if(eq(buffer,(char*)".names")&&in_exact(buffer,LutInput_0)&&in_exact(buffer,LutInput_1)&&in_exact(buffer,LutInput_2)&&in(buffer,(char*)backwardLUT.data())){
                    temps = strtok(buffer," ");
                    while(temps!=NULL){
                        if(temps == LutInput_0){
                            outfile << backwardFF << "new1 ";
                        }
                        else if(temps == LutInput_1){
                            outfile << backwardFF << "new2 ";
                        }
                        else if(temps == LutInput_2){
                            outfile << backwardFF << "new3 ";
                        }
                        else if(eq(temps,(char*)backwardLUT.data())){
                            
                            outfile << backwardFF<<endl;
                        }
                        else{
                            outfile << temps << " ";
                        }
                        temps = strtok(NULL," ");    
                    }
                }
                else if(eq(buffer,(char*)".names")&&in_exact(buffer,LutInput_0)&&in_exact(buffer,LutInput_1)&&in_exact(buffer,LutInput_2)){
                    temps = strtok(buffer," ");
                    while(temps!=NULL){
                        if(temps == LutInput_0 && backwardFF != LutInput_0){
                            outfile << backwardLUT << "new1 ";
                        }
                        else if(temps == LutInput_1 && backwardFF != LutInput_1){
                            outfile << backwardLUT << "new2 ";
                        }
                        else if(temps == LutInput_2 && backwardFF != LutInput_2){
                            outfile << backwardLUT << "new3 ";
                        }
                        else{
                            outfile << temps << " ";
                        }
                        temps = strtok(NULL," ");    
                    }
                }
                else if(eq(buffer,(char*)".names")&&in_exact(buffer,LutInput_0)&&in_exact(buffer,LutInput_1)){
                    temps = strtok(buffer," ");
                    while(temps!=NULL){
                        if(temps == LutInput_0 && backwardFF != LutInput_1){
                            outfile << backwardLUT << "new1 ";
                        }
                        else if(temps == LutInput_1 && backwardFF != LutInput_1){
                            outfile << backwardLUT << "new2 ";
                        }
                        else{
                            outfile << temps << " ";
                        }
                        temps = strtok(NULL," ");    
                    }
                }
                else if(eq(buffer,(char*)".names")&&in_exact(buffer,LutInput_0)&&in_exact(buffer,LutInput_2)){
                    temps = strtok(buffer," ");
                    while(temps!=NULL){
                        if(temps == LutInput_0 && backwardFF != LutInput_0){
                            outfile << backwardLUT << "new1 ";
                        }
                        else if(temps == LutInput_2 && backwardFF != LutInput_2){
                            outfile << backwardLUT << "new3 ";
                        }
                        else{
                            outfile << temps << " ";
                        }
                        temps = strtok(NULL," ");    
                    }
                }
                else if(eq(buffer,(char*)".names")&&in_exact(buffer,LutInput_1)&&in_exact(buffer,LutInput_2)){
                    temps = strtok(buffer," ");
                    while(temps!=NULL){
                        if(temps == LutInput_1 && backwardFF != LutInput_1){
                            outfile << backwardLUT << "new2 ";
                        }
                        else if(temps == LutInput_2 && backwardFF != LutInput_2){
                            outfile << backwardLUT << "new3 ";
                        }
                        else{
                            outfile << temps << " ";
                        }
                        temps = strtok(NULL," ");    
                    }
                }
                else if(eq(buffer,(char*)".names")&&in_exact(buffer,LutInput_0)){
                    temps = strtok(buffer," ");
                    while(temps!=NULL){
                        if(temps == LutInput_0 && backwardFF != LutInput_0){
                            outfile << backwardLUT << "new1 ";
                        }
                        else if(temps == LutInput_0 + "\n" && backwardFF != LutInput_0){
                            outfile << backwardLUT << "new1 " << endl;
                        }
                        else{
                            outfile << temps << " ";
                        }
                        temps = strtok(NULL," ");    
                    }
                }
                else if(eq(buffer,(char*)".names")&&in_exact(buffer,LutInput_1)){
                    temps = strtok(buffer," ");
                    while(temps!=NULL){
                        if(temps == LutInput_1 && backwardFF != LutInput_1){
                            outfile << backwardLUT << "new2 ";
                        }
                        else if(temps == LutInput_1 + "\n" && backwardFF != LutInput_1){
                            outfile << backwardLUT << "new2 " << endl;
                        }
                        else{
                            outfile << temps << " ";
                        }
                        temps = strtok(NULL," ");    
                    }
                }
                else if(eq(buffer,(char*)".names")&&in_exact(buffer,LutInput_2)){
                    temps = strtok(buffer," ");
                    while(temps!=NULL){
                        if(temps == LutInput_2 && backwardFF != LutInput_2){
                            outfile << backwardLUT << "new3 ";
                        }
                        else if(temps == LutInput_2 + "\n" && backwardFF != LutInput_2){
                            outfile << backwardLUT << "new3 " << endl;
                        }
                        else{
                            outfile << temps << " ";
                        }
                        temps = strtok(NULL," ");    
                    }
                }
                else if(eq(buffer,(char*)".names")&&in_exact(buffer,backwardLUT)){
                    temps = strtok(buffer," ");
                    while(temps!=NULL){
                        if(temps == backwardLUT){
                            outfile << backwardFF << " ";
                        }
                        else{
                            outfile << temps << " ";
                        }
                        temps = strtok(NULL," ");    
                    }
                }
                else{
                    outfile << buffer;
                }
                if(eq(buffer,(char*)".end")){
                    outfile.close();
                    break;     
                }
            
            }
        }

    }
    if(inputNum == 4){    
        if(find(blifInput.begin(),blifInput.end(),LutInput_0)!= blifInput.end()){
            cout<<"LutInput_0 is blif input, illegal retiming!"<<endl;
            return false;
        }
        else if(find(blifInput.begin(),blifInput.end(),LutInput_1) != blifInput.end()){
            cout<<"LutInput_1 is blif input,illegal retiming!"<<endl;
            return false;
        }
        else if(find(blifInput.begin(),blifInput.end(),LutInput_2) != blifInput.end()){
            cout<<"LutInput_2 is blif input,illegal retiming!"<<endl;
            return false;
        }
        else if(find(blifInput.begin(),blifInput.end(),LutInput_3) != blifInput.end()){
            cout<<"LutInput_3 is blif input,illegal retiming!"<<endl;
            return false;
        }
        else{
            cout<<"All input is ff, legal retiming"<<endl;
            while(1){
                fgets(buffer,300,infile);
                if(eq(buffer,(char*)".latch")&&in_exact(buffer,(char*)backwardFF.data())&&in_exact(buffer,(char*)backwardLUT.data())){
                    cout<<"Consider all four inputs to lut"<<endl;
                    outfile << "#" <<buffer;
                    outfile << ".latch    "<< backwardLUT << "new1 " << backwardFF << "new1 re clock 2"<<endl;
                    outfile << ".latch    "<< backwardLUT << "new2 " << backwardFF << "new2 re clock 2"<<endl;
                    outfile << ".latch    "<< backwardLUT << "new3 " << backwardFF << "new3 re clock 2"<<endl;
                    outfile << ".latch    "<< backwardLUT << "new4 " << backwardFF << "new4 re clock 2"<<endl;       
                }
                else if(eq(buffer,(char*)".latch")&&in_exact(buffer,LutInput_0)){

                    temps = strtok(buffer," ");
                    while(temps!=NULL){
                        if(temps == LutInput_0){
                            outfile << backwardLUT << "new1 ";
                        }
                        else if(in(temps,(char*)"\n")){
                            outfile << temps;
                        }
                        else if(eq(temps,(char*)".latch")){
                            outfile << ".latch    ";
                        }
                        else{
                            outfile << temps << " ";
                        }
                        temps = strtok(NULL," "); 
                    }
                }
                else if(eq(buffer,(char*)".latch")&&in_exact(buffer,LutInput_1)){
                    temps = strtok(buffer," ");
                    while(temps!=NULL){
                        
                        if(eq(temps,(char*)".latch")){
                            outfile << ".latch    ";
                        }
                        else if(temps == LutInput_1){
                            outfile << backwardLUT << "new2 ";
                        }
                        else if(in(temps,(char*)"\n")){
                            outfile << temps;
                        }
                        else{
                            outfile << temps << " ";
                        }
                        temps = strtok(NULL," "); 
                    }
                }
                else if(eq(buffer,(char*)".latch")&&in_exact(buffer,LutInput_2)){
                    temps = strtok(buffer," ");
                    while(temps!=NULL){
                        if(temps == LutInput_2){
                            outfile << backwardLUT << "new3 ";
                        }
                        else if(in(temps,(char*)"\n")){
                            outfile << temps;
                        }
                        else if(eq(temps,(char*)".latch")){
                            outfile << ".latch    ";
                        }
                        else{
                            outfile << temps << " ";
                        }
                        temps = strtok(NULL," "); 
                    }
                }
                else if(eq(buffer,(char*)".latch")&&in_exact(buffer,LutInput_3)){
                    //outfile << ".latch    "
                    temps = strtok(buffer," ");
                    while(temps!=NULL){
                        if(temps == LutInput_3){
                            outfile << backwardLUT << "new4 ";
                        }
                        else if(in(temps,(char*)"\n")){
                            outfile << temps;
                        }
                        else if(eq(temps,(char*)".latch")){
                            outfile << ".latch    ";
                        }
                        else{
                            outfile << temps << " ";
                        }
                        temps = strtok(NULL," "); 
                    }
                }
                else if(eq(buffer,(char*)".names")&&in_exact(buffer,LutInput_0)&&in_exact(buffer,LutInput_1)&&in_exact(buffer,LutInput_2)&&in_exact(buffer,LutInput_3)&&in(buffer,(char*)backwardLUT.data())){
                    temps = strtok(buffer," ");
                    while(temps!=NULL){
                        if(temps == LutInput_0){
                            outfile << backwardFF << "new1 ";
                        }
                        else if(temps == LutInput_1){
                            outfile << backwardFF << "new2 ";
                        }
                        else if(temps == LutInput_2){
                            outfile << backwardFF << "new3 ";
                        }
                        else if(temps == LutInput_3){
                            outfile << backwardFF << "new4 ";
                        }
                        else if(eq(temps,(char*)backwardLUT.data())){
                            
                            outfile << backwardFF<<endl;
                        }
                        else{
                            outfile << temps << " ";
                        }
                        temps = strtok(NULL," ");    
                    }
                }
                else if(eq(buffer,(char*)".names")&&in_exact(buffer,LutInput_0)&&in_exact(buffer,LutInput_1)&&in_exact(buffer,LutInput_2)&&in_exact(buffer,LutInput_3)){
                    temps = strtok(buffer," ");
                    while(temps!=NULL){
                        if(temps == LutInput_0 && backwardFF != LutInput_0){
                            outfile << backwardLUT << "new1 ";
                        }
                        else if(temps == LutInput_1 && backwardFF != LutInput_1){
                            outfile << backwardLUT << "new2 ";
                        }
                        else if(temps == LutInput_2 && backwardFF != LutInput_2){
                            outfile << backwardLUT << "new3 ";
                        }
                        else if(temps == LutInput_3 && backwardFF != LutInput_3){
                            outfile << backwardLUT << "new4 ";
                        }
                        else{
                            outfile << temps << " ";
                        }
                        temps = strtok(NULL," ");    
                    }
                }
                else if(eq(buffer,(char*)".names")&&in_exact(buffer,LutInput_0)&&in_exact(buffer,LutInput_1)&&in_exact(buffer,LutInput_2)){
                    temps = strtok(buffer," ");
                    while(temps!=NULL){
                        if(temps == LutInput_0 && backwardFF != LutInput_0){
                            outfile << backwardLUT << "new1 ";
                        }
                        else if(temps == LutInput_1 && backwardFF != LutInput_1){
                            outfile << backwardLUT << "new2 ";
                        }
                        else if(temps == LutInput_2 && backwardFF != LutInput_2){
                            outfile << backwardLUT << "new3 ";
                        }
                        else{
                            outfile << temps << " ";
                        }
                        temps = strtok(NULL," ");    
                    }
                }
                else if(eq(buffer,(char*)".names")&&in_exact(buffer,LutInput_0)&&in_exact(buffer,LutInput_1)&&in_exact(buffer,LutInput_3)){
                    temps = strtok(buffer," ");
                    while(temps!=NULL){
                        if(temps == LutInput_0 && backwardFF != LutInput_0){
                            outfile << backwardLUT << "new1 ";
                        }
                        else if(temps == LutInput_1 && backwardFF != LutInput_1){
                            outfile << backwardLUT << "new2 ";
                        }
                        else if(temps == LutInput_3 && backwardFF != LutInput_3){
                            outfile << backwardLUT << "new4 ";
                        }
                        else{
                            outfile << temps << " ";
                        }
                        temps = strtok(NULL," ");    
                    }
                }
                else if(eq(buffer,(char*)".names")&&in_exact(buffer,LutInput_0)&&in_exact(buffer,LutInput_2)&&in_exact(buffer,LutInput_3)){
                    temps = strtok(buffer," ");
                    while(temps!=NULL){
                        if(temps == LutInput_0 && backwardFF != LutInput_0){
                            outfile << backwardLUT << "new1 ";
                        }
                        else if(temps == LutInput_2 && backwardFF != LutInput_2){
                            outfile << backwardLUT << "new3 ";
                        }
                        else if(temps == LutInput_3 && backwardFF != LutInput_3){
                            outfile << backwardLUT << "new4 ";
                        }
                        else{
                            outfile << temps << " ";
                        }
                        temps = strtok(NULL," ");    
                    }
                }
                else if(eq(buffer,(char*)".names")&&in_exact(buffer,LutInput_1)&&in_exact(buffer,LutInput_2)&&in_exact(buffer,LutInput_3)){
                    temps = strtok(buffer," ");
                    while(temps!=NULL){
                        if(temps == LutInput_1 && backwardFF != LutInput_1){
                            outfile << backwardLUT << "new2 ";
                        }
                        else if(temps == LutInput_2 && backwardFF != LutInput_2){
                            outfile << backwardLUT << "new3 ";
                        }
                        else if(temps == LutInput_3 && backwardFF != LutInput_3){
                            outfile << backwardLUT << "new4 ";
                        }
                        else{
                            outfile << temps << " ";
                        }
                        temps = strtok(NULL," ");    
                    }
                }
                else if(eq(buffer,(char*)".names")&&in_exact(buffer,LutInput_0)&&in_exact(buffer,LutInput_1)){
                    temps = strtok(buffer," ");
                    while(temps!=NULL){
                        if(temps == LutInput_0 && backwardFF != LutInput_0){
                            outfile << backwardLUT << "new1 ";
                        }
                        else if(temps == LutInput_1 && backwardFF != LutInput_1){
                            outfile << backwardLUT << "new2 ";
                        }
                        else{
                            outfile << temps << " ";
                        }
                        temps = strtok(NULL," ");    
                    }
                }
                else if(eq(buffer,(char*)".names")&&in_exact(buffer,LutInput_0)&&in_exact(buffer,LutInput_2)){
                    temps = strtok(buffer," ");
                    while(temps!=NULL){
                        if(temps == LutInput_0&& backwardFF != LutInput_0){
                            outfile << backwardLUT << "new1 ";
                        }
                        else if(temps == LutInput_2&& backwardFF != LutInput_2){
                            outfile << backwardLUT << "new3 ";
                        }
                        else{
                            outfile << temps << " ";
                        }
                        temps = strtok(NULL," ");    
                    }
                }
                else if(eq(buffer,(char*)".names")&&in_exact(buffer,LutInput_1)&&in_exact(buffer,LutInput_2)){
                    temps = strtok(buffer," ");
                    while(temps!=NULL){
                        if(temps == LutInput_1&& backwardFF != LutInput_1){
                            outfile << backwardLUT << "new2 ";
                        }
                        else if(temps == LutInput_2&& backwardFF != LutInput_2){
                            outfile << backwardLUT << "new3 ";
                        }
                        else{
                            outfile << temps << " ";
                        }
                        temps = strtok(NULL," ");    
                    }
                }
                else if(eq(buffer,(char*)".names")&&in(buffer,(char*)LutInput_0.data())){
                    temps = strtok(buffer," ");
                    while(temps!=NULL){
                        if(temps == LutInput_0&& backwardFF != LutInput_0){
                            outfile << backwardLUT << "new1 ";
                        }
                        else if(temps == LutInput_0 + "\n"&& backwardFF != LutInput_0){
                            outfile << backwardLUT << "new1 " << endl;
                        }
                        else{
                            outfile << temps << " ";
                        }
                        temps = strtok(NULL," ");    
                    }
                }
                else if(eq(buffer,(char*)".names")&&in(buffer,(char*)LutInput_1.data())){
                    temps = strtok(buffer," ");
                    while(temps!=NULL){
                        if(temps == LutInput_1&& backwardFF != LutInput_1){
                            outfile << backwardLUT << "new2 ";
                        }
                        else if(temps == LutInput_1 + "\n"&& backwardFF != LutInput_1){
                            outfile << backwardLUT << "new2 " << endl;
                        }
                        else{
                            outfile << temps << " ";
                        }
                        temps = strtok(NULL," ");    
                    }
                }
                else if(eq(buffer,(char*)".names")&&in(buffer,(char*)LutInput_2.data())){
                    temps = strtok(buffer," ");
                    while(temps!=NULL){
                        if(temps == LutInput_2&& backwardFF != LutInput_2){
                            outfile << backwardLUT << "new3 ";
                        }
                        else if(temps == LutInput_2 + "\n"&& backwardFF != LutInput_2){
                            outfile << backwardLUT << "new3 " << endl;
                        }
                        else{
                            outfile << temps << " ";
                        }
                        temps = strtok(NULL," ");    
                    }
                }
                else if(eq(buffer,(char*)".names")&&in(buffer,(char*)LutInput_3.data())){
                    temps = strtok(buffer," ");
                    while(temps!=NULL){
                        if(temps == LutInput_3&& backwardFF != LutInput_3){
                            outfile << backwardLUT << "new4 ";
                        }
                        else if(temps == LutInput_3 + "\n"&& backwardFF != LutInput_3){
                            outfile << backwardLUT << "new4 " << endl;
                        }
                        else{
                            outfile << temps << " ";
                        }
                        temps = strtok(NULL," ");    
                    }
                }
                else if(eq(buffer,(char*)".names")&&in_exact(buffer,backwardLUT)){
                    temps = strtok(buffer," ");
                    while(temps!=NULL){
                        if(temps == backwardLUT){
                            outfile << backwardFF << " ";
                        }
                        else{
                            outfile << temps << " ";
                        }
                        temps = strtok(NULL," ");    
                    }
                }
                else{
                    //cout<<"normal output"<<buffer<<endl;
                    outfile << buffer;
                }
                if(eq(buffer,(char*)".end")){
                    outfile.close();
                    break;     
                }
            }
        }
    }

    //Delete empty lines in the file
    fstream targetFile(ofName,fstream::in | fstream::out);
    string line;
    string temp;
    deque<string> noBlankLineQueue;
    if(!targetFile){
        cerr << "Can't Open File!" << endl;       
    }
    auto StartPos = targetFile.tellp();
    while(getline(targetFile,line)){  
        if(line.empty()){
        }else{
                //cout << "empty" << endl;   
            noBlankLineQueue.push_back(line);
        }   
    }
    targetFile.close();
    ofstream emptyFile(ofName);
    emptyFile.close();
    fstream target(ofName,fstream::out | fstream::in);
    if(!target){
        cerr << "Can't Open File" << endl;
    }
    auto begin = noBlankLineQueue.begin();
    auto end = noBlankLineQueue.end();
    while(begin != end){
        temp = *begin;
        //cout << temp << endl;
        target << temp << endl; 
        ++begin;
    }
    target.close();
    return true;
}

/**
 * @brief Extract the critical path delay from the VPR report.
 * 
 * @param ofName 
 * @return true 
 * @return false 
 */
bool SR::RsltExtract(string ofName){
    FILE* infile;
    char buffer[300], *temps;
    infile = fopen("./vpr_stdout.log","r");
    if(infile == NULL){	
        cout <<"cannot open the vpr log file vpr_stdout.log\n" << endl; 
        exit(0);	
    }

    while(1){
        fgets(buffer,300,infile);
        if(eq(buffer,(char*)"Final critical path:")){
            temps = strtok(buffer," ");
                int count(0);
                while(temps!=NULL){
                    if(count == 3 ){
                        //temps = strtok(temps,".")
                        cout<<"Final critical path: "<<temps<<endl;
                        float CPD = std::atof(temps);
                        Rslt.insert(std::make_pair(ofName,CPD));
                        fclose(infile);
                        return false;
                    }
                    temps = strtok(NULL," ");
                    count ++;
                }
        }
    }
    
}


/**
 * @brief Print the critical path delay information.
 * 
 * @return true 
 * @return false 
 */
string SR::RsltPrint(){
    ofstream outfile;
    outfile.open("rslt.txt");

    vector<pair<string,float>> v(Rslt.begin(),Rslt.end());
    std::sort(v.begin(), v.end(),[](const auto& p1, const auto& p2) {return p1.first < p2.first;});

    for (auto it = v.begin(); it != v.end(); it++) {
        outfile << it->first << ": " << it->second << endl;
    }
    
    std::sort(v.begin(), v.end(), [](const auto& p1, const auto& p2) {return p1.second < p2.second;});
    std::cout << "The key with the smallest value is " << v.front().first << ", with value " << v.front().second << std::endl;

    return v.front().first;
}

