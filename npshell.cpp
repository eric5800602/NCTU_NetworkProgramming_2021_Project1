#include <iostream>
#include <cstdlib>
#include <string>
#include <unistd.h>
#include <sys/wait.h>
#include <sstream>
#include <vector>
#include <fcntl.h>
#include <sys/stat.h>
#include <poll.h>
#include <memory.h>

using namespace std;
int EXECCMD(vector<string>);
int ParseCMD(vector<string>);

void HandleChild(int sig){
	int status;
	while(waitpid(-1,&status,WNOHANG) > 0){}
}

void SETENV(string name,string val){
	setenv(name.c_str(),val.c_str(),1);
}

void PRINTENV(string name){
	char *val = getenv(name.c_str());
	if(val) cout << val << endl;
}

bool CheckPipeHasContent(int fd){
	struct pollfd fdtab[1];
	memset (fdtab, 0, sizeof(fdtab));
	fdtab[0].fd = fd;
	fdtab[0].events = POLLIN;
	fdtab[0].revents = 0;
	int retpoll = poll(fdtab, 1, 100);
	if(retpoll > 0)
		if(fdtab[0].revents & POLLIN)
			return true;
	perror("poll fail");
	return false;
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

struct npipe{
	int in;
	int out;
	int num;
};

//used to store numberpipe
vector<npipe> numberpipe_vector;

int ParseCMD(vector<string> input){
	string cmd;
	size_t pos = 0;
	bool has_numberpipe = false,has_errpipe = false;
	string numpipe_delim = "|";
	string errpipe_delim = "!";
	// Create pipe whose num = 2 * count of cmd
	int pipes[2*input.size()];
	for(int i = 0;i < input.size();++i){
		pipe(pipes + i*2);
	}
	for(int i = 0;i < input.size();++i){
		istringstream iss(input[i]);
		vector<string> parm;
		// Create pipe for number pipe, last one is for number
		while(getline(iss,cmd,' ')){
			//if still find "!" means errorpipe with number,and record the number
			if((pos = cmd.find(errpipe_delim)) != string::npos){
				int numberpipe[2];
				pipe(numberpipe);
				int tmpnum = atoi(cmd.erase(0,pos+numpipe_delim.length()).c_str());
				npipe np = {numberpipe[0],numberpipe[1],tmpnum};
				numberpipe_vector.push_back(np);
				has_errpipe = true;
				continue;
			}

			//if still find "|" means numberpipe with number,and record the number
			if((pos = cmd.find(numpipe_delim)) != string::npos){
				int numberpipe[2];
				pipe(numberpipe);
				int tmpnum = atoi(cmd.erase(0,pos+numpipe_delim.length()).c_str());
				npipe np = {numberpipe[0],numberpipe[1],tmpnum};
				numberpipe_vector.push_back(np);
				has_numberpipe = true;
				continue;
			}
			parm.push_back(cmd);
		}
		signal(SIGCHLD, HandleChild);
		pid_t cpid;
		int status;
		cpid = fork();
		if(cpid != 0){
			//Check fork information
			//cout << "fork " << cpid << endl;
			if(i != 0){
				close(pipes[(i-1)*2]);
				close(pipes[(i-1)*2+1]);
			}
			//numberpipe reciever close
			for(int j = 0;j < numberpipe_vector.size();++j){
				 numberpipe_vector[j].num--;
				//numberpipe erase
				if(numberpipe_vector[j].num < 0){
					close(numberpipe_vector[j].in);
					close(numberpipe_vector[j].out);	
					numberpipe_vector.erase(numberpipe_vector.begin() + j);
					j--;
				}
			}
			waitpid(cpid,&status,0);
		}
		else{
			//numberpipe recieve
			if(i == 0){
				bool has_front_pipe = false;
				int front_fd = 0;
				for(int j = numberpipe_vector.size()-1;j >= 0;--j){
					if(numberpipe_vector[j].num == 0){
						if(has_front_pipe && front_fd != 0){
							fcntl(front_fd, F_SETFL, O_NONBLOCK);
							while (1) {
								char tmp;
								if (read(front_fd, &tmp, 1) < 1){
									break;
								}
								int rt = write(numberpipe_vector[j].out,&tmp,1);
								//fprintf(stderr, "rt = %d\n", rt);
							}
							has_front_pipe = false;
							dup2(numberpipe_vector[j].in,STDIN_FILENO);
							//fprintf(stderr,"%d\n",numberpipe_vector[j].in);
						}
						else{
							dup2(numberpipe_vector[j].in,STDIN_FILENO);
							front_fd = numberpipe_vector[j].in;
							has_front_pipe = true;
						}
					}
				}
				for(int j = 0;j < numberpipe_vector.size();++j)	{
					if(numberpipe_vector[j].num == 0){
						close(numberpipe_vector[j].in);
						close(numberpipe_vector[j].out);
					}
				}
			}
			//connect pipes of each child process
			if(i != 0){
				dup2(pipes[(i-1)*2],STDIN_FILENO);
			}
			if(i != input.size()-1){
				dup2(pipes[i*2+1],STDOUT_FILENO);
			}
			//numberpipe send
			if(i == input.size()-1 && has_numberpipe){
				dup2(numberpipe_vector[numberpipe_vector.size()-1].out,STDOUT_FILENO);
				close(numberpipe_vector[numberpipe_vector.size()-1].in);
				close(numberpipe_vector[numberpipe_vector.size()-1].out);
			}
			if(i == input.size()-1 && has_errpipe){
				dup2(numberpipe_vector[numberpipe_vector.size()-1].out,STDERR_FILENO);
				dup2(numberpipe_vector[numberpipe_vector.size()-1].out,STDOUT_FILENO);
				close(numberpipe_vector[numberpipe_vector.size()-1].in);
				close(numberpipe_vector[numberpipe_vector.size()-1].out);
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
		if(CheckPIPE(input)== -1){
			return 0;
		}
	}	
	return 0;
}
