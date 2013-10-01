/***********************************

	Author: Matthew Pfister
	ACC: mpfister
	Date: 10/01/13
	Project: mpShell

************************************/

#include <string>
#include <vector>
#include <iostream>

#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>

#include <sys/wait.h>

using namespace std;

void runCMD(const vector< vector<string> >& commands, const string& redirect, const string& fileName);
int numberOfCommands(const string& input);
int checkInput(const string& input);

int main(int argc, char * argv[])
{
	string nextCommand;
	string input;
	string redirect;
	string fileName;
	vector<string> toParse;
	vector< vector<string> > toExecute;

	int i, j, k;
	int eCode = 0;
	int inLength = 0;
	int tempLength = 0;

	cout << endl << "mpShell" << endl << "Created by: Matthew Pfister (mpfister)" << endl << "Please input a command below" << endl; 
	cout << "--------------------------------------" << endl;

	while(1)
	{
		cout << "mpShell<: ";

		//Grab user input
		getline(cin, input);

		//check  user input
		eCode = checkInput(input);
		if(eCode == -1)			
			continue;
		else if(eCode == 1)
			exit(0);
;
		inLength = input.length();
		for(i=0, j=0; i < inLength; i=j+1)
		{
			j = input.find("|", i);

			if(j == string::npos)
				j = inLength;

			string temp = input.substr(i, j-i);

			for(k=0; temp.at(k) == ' '; k++)
				temp.erase(k, 1);

			for(k=temp.length()-1; temp.at(k) == ' '; k--)
				temp.erase(k, 1);

			toParse.push_back(temp);
		}

		/*
			//Print out input that was parsed
			for(i=0;i<toParse.size();i++)
				cout << toParse.at(i) << endl;
		*/

		for(i = 0; i < toParse.size(); i++)
		{
			vector<string> token_vec;
			string curr_token = toParse.at(i);
			tempLength = curr_token.length();

			for(j=0, k=0; k < tempLength; k=j+1)
			{
				j = curr_token.find(" ", k);

				if(j == string::npos)
					j = tempLength;

				string curr_substr = curr_token.substr(k, j-k);
				if(curr_substr.compare("<") == 0 || curr_substr.compare(">") == 0)
					redirect = curr_substr;
				else if(!redirect.empty())
					fileName = curr_substr;					
				else
					token_vec.push_back(curr_substr);
			}
			toExecute.push_back(token_vec);
		}
		
			//Print out commands to be executed
			for(i = 0; i < toExecute.size();i++)
			{
				vector<string> temp = toExecute.at(i);
				for(j=0; j < temp.size(); j++)
					cout << temp.at(j) << endl;
			}
		

		//Run cmd
		runCMD(toExecute, redirect, fileName);

		//Clear vectors for new input
		for(i = 0; i < toExecute.size();i++)
			toExecute.at(i).clear();
		toExecute.clear();
		toParse.clear();
		redirect.clear();
	}

	return 0;
}

/*----
parameters: char ***, int, char, char *, history *
return value: void

Takes in the array of array pointers (which are the tokenized commands), the number of commands in the array, a char to determine if there is a redirect present, the filename associated with the redirect, and the head of the linked list. The function then starts a loop:

for command in commands
create pipe
check for redirects
	if < && its the first command: then set file descriptor 0 to open(filepath, readonly)
	else if > && its the last command: then set file descriptor 1 to open(filepath, writeonly)
fork
child
	if not first command
		duplicate the old instream file descriptor (to use previous process' output)
		close old file descriptors (Don't need these anymore, got what I wanted)
	else
		duplicate file redirect in (need previous process' input, which was changed to a new file descriptor with open)
		close in stream file decriptor (got what I came for, so close)

	if not last command
		close in stream file descriptor (close childs in stream so it sends its output to parent)
		duplicate the out stream file descriptor (so execution is copied to out stream)
		close out stream file descriptor (got what I came for, so close)
	else
		duplicate file redirect (need previous process output, which was changed to a new file descriptor with open)
		close out stream file decriptor (got what I came for, so close)

	exec
parent
	if not first command
		close old file descriptors (close old pipe)
	if not last command
		copy new file descriptors to old filedescriptors (store new pipe)

wait for children
add usage to history linked list
----*/
void runCMD(const vector< vector<string> >& commands, const string& redirect, const string& fileName)
{
	int i = 0;
	int childStatus = -1;
	int num_cmd = commands.size();

	int oldpipefd[2];
	int pipefd[2];
	int pids[num_cmd];

	for(i = 0; i < num_cmd; i++)
	{
		if(i < num_cmd-1)
		{
			if(pipe(pipefd) < 0)
				puts("ERROR PIPE2");
		}
		else if(redirect.compare(">") == 0)
		{
			pipefd[1] = open(fileName.c_str(), O_WRONLY);
		}

		if(i == 0 && redirect.compare("<") == 0)
		{
			pipefd[0] = open(fileName.c_str(), O_RDONLY);
		} 

		if((pids[i] = fork()) < 0)
		{
			puts("ERROR FORK");
		}

		else if(pids[i] == 0) 
		{
			//child
			if(i > 0)
			{
				dup2(oldpipefd[0], 0);
				close(oldpipefd[0]);
				close(oldpipefd[1]);
			}
			else if(redirect.compare("<") == 0)
			{
				dup2(pipefd[0], 0);
				close(pipefd[0]);
			}

			if(i < num_cmd-1)
			{
				close(pipefd[0]);
				dup2(pipefd[1], 1);
				close(pipefd[1]);
			}
			else if(redirect.compare(">") == 0)
			{
				dup2(pipefd[1], 1);
				close(pipefd[1]);
			}

			int cmd_size = commands.at(i).size();
			const char * cmd = commands.at(i).at(0).c_str();
			const char * cmd_args[cmd_size];

			for(int j=0; j<cmd_size;j++)
				cmd_args[j] = commands.at(i).at(j).c_str();
			cmd_args[cmd_size] = NULL;

			execvp(cmd, (char **)cmd_args);
			printf("ERROR: NOT A COMMAND\n");
			exit(0);
		}
		else
		{
			//parent
			if(i > 0)
			{
				close(oldpipefd[0]);
				close(oldpipefd[1]);
			}
			if(i < num_cmd-1)
			{
				oldpipefd[0] = pipefd[0];
				oldpipefd[1] = pipefd[1];
			}
		}
	}

	for(i = 0; i < num_cmd; i++)
		waitpid(pids[i], &childStatus, 0);
}

/*----
parameters: char *
return: int
Takes in a string. Then determines if the input is a special case. If it is, compute what needs to be done and return proper exection code.
----*/
int checkInput(const string& input)
{
	
	if(input.compare("exit") == 0)
		return 1;
	
	else if(input.empty())
		return -1;

	return 0;
}
