#include <iostream>
#include <cstdlib>
#include <string>
#include <unistd.h>
#include <sys/wait.h>
#include <sstream>
#include <vector>

using namespace std;

void SETENV(string name,string val){
	setenv(name.c_str(),val.c_str(),1);
}

void PRINTENV(string name){
	char *val = getenv(name.c_str());
	if(val) cout << val << endl;
}

int EXECCMD(string input){
	vector<string> parm;
	string cmd;
	istringstream iss(input);
	while(getline(iss,cmd,' ')){
		parm.push_back(cmd);
	}
	if(parm[0] == "setenv"){
		SETENV(parm[1],parm[2]);
		return 0;
	}
	else if(parm[0] == "printenv"){
		PRINTENV(parm[1]);
		return 0;
	}
	else if(parm[0] == "exit"){
		exit(0);
	}
	const char **argv = new const char* [parm.size()+1];
	for(int i=0;i < parm.size();++i){
		argv[i] = parm[i].c_str();
	}
	argv[parm.size()] = NULL;
	pid_t cpid;
	int status;
	cpid = fork();
	if(cpid != 0){
		waitpid(cpid,&status,0);
	}
	else{
		cmd = parm[0];
		if(execvp(cmd.c_str(),(char **)argv) == -1){
			return -1;
		}
	}
	return 0;
}

int main(){
	clearenv();
	SETENV("PATH","bin:.");
	while(1){
		string input;
		cout << "% ";
		getline(cin,input);
		if(!cin)
			if(cin.eof())
			{
				cout << endl;
				return 0;
			}
		if(EXECCMD(input)==-1) return 0;
	}	
	return 0;
}
