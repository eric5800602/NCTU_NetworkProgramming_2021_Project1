#include <iostream>
#include <cstdlib>
#include <string>
#include <unistd.h>
#include <sys/wait.h>
#include <sstream>
#include <vector>
#include <fcntl.h>
#include <sys/stat.h>

using namespace std;
int EXECCMD(vector<string>);

void SETENV(string name,string val){
	setenv(name.c_str(),val.c_str(),1);
}

void PRINTENV(string name){
	char *val = getenv(name.c_str());
	if(val) cout << val << endl;
}

int ParseCMD(string input){
	vector<string> parm;
	string cmd;
	istringstream iss(input);
	while(getline(iss,cmd,' ')){
		parm.push_back(cmd);
	}
	EXECCMD(parm);
	return 0;
}

int EXECCMD(vector<string> parm){
	/*Built in cmd*/
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
	int fd;
	bool file_redirection = false;
	for(int i=0;i < parm.size();++i){
		//file redirect
		if(parm[i] == ">"){
			file_redirection = true;
			fd = open(parm.back().c_str(),O_CREAT|O_RDWR|O_TRUNC, S_IREAD|S_IWRITE);
			parm.pop_back();
			parm.pop_back();
		}
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
		if(file_redirection){
			// stdout to file
			if(dup2(fd,1) < 0){
				cerr << "dup error" << endl;
			}
			close(fd);
		}
		if(execvp(parm[0].c_str(),(char **)argv) == -1){
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
		if(ParseCMD(input)==-1) return 0;
	}	
	return 0;
}
