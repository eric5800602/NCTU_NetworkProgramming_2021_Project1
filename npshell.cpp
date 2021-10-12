#include <iostream>
#include <cstdlib>
#include <string>
#include <unistd.h>
#include <sys/wait.h>
#include <sstream>
#include <vector>

using namespace std;

int main(){
	clearenv();
	setenv("PATH","bin:.",1);
	while(1){
		vector<string> parm;
		string input,cmd;
		cout << "% ";
		getline(cin,input);
		if(!cin)
			if(cin.eof())
			{
				cout << endl;
				return 0;
			}
		istringstream iss(input);
		while(getline(iss,cmd,' ')){
			if(cmd == "exit"){
				exit(0);
			}else{
				parm.push_back(cmd);
			}
			
		}
		const char **argv = new const char* [parm.size()+2];
		for(int i=0;i < parm.size()+1;++i){
			argv[i] = parm[i].c_str();
		}
		argv[parm.size()+1] = NULL;
		pid_t cpid;
		int status;
	    	cpid = fork();
		if(cpid != 0){
			waitpid(cpid,&status,0);
		}
		else{
			cmd = parm[0];
			if(execvp(cmd.c_str(),(char **)argv) == -1){
				exit(0);
			}
		}
	}	
	return 0;
}
