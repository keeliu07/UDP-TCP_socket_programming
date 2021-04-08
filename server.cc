#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <errno.h>
#include <vector>
#include <string.h>
#include <iostream>
#include <fstream>
#include <stdio.h>
#include <unistd.h>
#include <cstdlib>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sstream>
#include <string>

#include "message.h"
#include "server.h"

using namespace std;

Server_State_T server_state;
string cmd_string[] = {" ", "CMD_LS", "CMD_SEND","CMD_GET","CMD_REMOVE","CMD_RENAME","CMD_SHUTDOWN"};

int main(int argc, char *argv[])
{
    unsigned short udp_port = 0;
	if ((argc != 1) && (argc != 3))
	{
		cout << "Usage: " << argv[0];
		cout << " [-p <udp_port>]" << endl;
		return 1;
	}
	else
	{
		//system("clear");
		//process input arguments
		for (int i = 1; i < argc; i++)
		{				
			if (strcmp(argv[i], "-p") == 0)
				udp_port = (unsigned short) atoi(argv[++i]);
		    else
		    {
		        cout << "Usage: " << argv[0];
		        cout << " [-p <udp_port>]" << endl;
		        return 1;
		    }
		}
	}
	
    while(true)
    {
        usleep(100);
        
        switch(server_state)
        {
            case WAITING:
            {                
                break;
            }
            case PROCESS_LS:
            {                
                server_state = WAITING;
                break;
            }
            case PROCESS_SEND:
            {                
                server_state = WAITING;
                break;
            }
            case PROCESS_REMOVE:
            {                
		        server_state = WAITING;
                break;
            }
            case PROCESS_RENAME:
            {                
                server_state = WAITING;
                break;
            }
            case SHUTDOWN:
            {
            }
            default:
            {
           		server_state = WAITING;
                break;
            }
        }
    }
    return 0;
}

//this function check if the backup folder exist
int checkDirectory (string dir)
{
	DIR *dp;
	if((dp  = opendir(dir.c_str())) == NULL) {
        //cout << " - error(" << errno << ") opening " << dir << endl;
        if(mkdir(dir.c_str(), S_IRWXU) == 0)
            cout<< " - Note: Folder "<<dir<<" does not exist. Created."<<endl;
        else
            cout<< " - Note: Folder "<<dir<<" does not exist. Cannot created."<<endl;
        return errno;
    }
    closedir(dp);
}


//this function is used to get all the filenames from the
//backup directory
int getDirectory (string dir, vector<string> &files)
{
    DIR *dp;
    struct dirent *dirp;
    if((dp  = opendir(dir.c_str())) == NULL) {
        //cout << " - error(" << errno << ") opening " << dir << endl;
        if(mkdir(dir.c_str(), S_IRWXU) == 0)
            cout<< " - Note: Folder "<<dir<<" does not exist. Created."<<endl;
        else
            cout<< " - Note: Folder "<<dir<<" does not exist. Cannot created."<<endl;
        return errno;
    }

    int j=0;
    while ((dirp = readdir(dp)) != NULL) {
    	//do not list the file "." and ".."
        if((string(dirp->d_name)!=".") && (string(dirp->d_name)!=".."))
        	files.push_back(string(dirp->d_name));
    }
    closedir(dp);
    return 0;
}
//this function check if the file exists
bool checkFile(const char *fileName)
{
    ifstream infile(fileName);
    return infile.good();
}

