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
int ParseCMD(vector<string>);

void SETENV(string name,string val){
	setenv(name.c_str(),val.c_str(),1);
}

void PRINTENV(string name){
	char *val = getenv(name.c_str());
	if(val) cout << val << endl;
}

int CheckPIPE(string input){
	vector<string> cmds;
	string delim = " | ";
	size_t pos = 0;
	while((pos = input.find(delim)) != string::npos){
		cmds.push_back(input.substr(0,pos));
		input.erase(0,pos + delim.length());
	}
	cmds.push_back(input);
	ParseCMD(cmds);
	return 0;
}

int ParseCMD(vector<string> input){
	string cmd;
	size_t pos = 0;
	//string numpipe_delim = "|";
	// Create pipe whose num = 2 * count of cmd
	int pipes[2*input.size()];
	for(int i = 0;i < 2*input.size();++i){
		pipe(pipes + i*2);
	}
	for(int i = 0;i < input.size();++i){
		istringstream iss(input[i]);
		vector<string> parm;
		while(getline(iss,cmd,' ')){
			//if still find "|" means pipe with number
			//if((pos = cmd.find(numpipe_delim)) != string::npos) contine;
			parm.push_back(cmd);
		}
		pid_t cpid;
		int status;
		cpid = fork();
		if(cpid != 0){
			//Check fork indormation
			//cout << "fork " << cpid << endl;
			if(i != 0){
				close(pipes[(i-1)*2]);
				close(pipes[(i-1)*2+1]);
			}
			waitpid(cpid,&status,0);
		}
		else{
			//connect pipes of each child process
			if(i != 0){
				dup2(pipes[(i-1)*2],STDIN_FILENO);
			}
			if(i != input.size()-1){
				dup2(pipes[i*2+1],STDOUT_FILENO);
			}
			for(int j = 0;j < 2*input.size();++j)
				close(pipes[j]);
			EXECCMD(parm);
		}
	}
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
	int fd;
	bool file_redirection = false;	
	const char **argv = new const char* [parm.size()+1];
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
		if(CheckPIPE(input)== -1) return 0;
	}	
	return 0;
}
