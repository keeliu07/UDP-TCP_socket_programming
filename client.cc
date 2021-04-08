#include <sys/types.h>
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
#include <netdb.h>
#include <algorithm>

#include "message.h"
#include "client.h"

using namespace std;

int main(int argc, char *argv[])
{
    unsigned short udp_port = 0;
    const char* server_host = "127.0.0.1";
    //process input arguments
	if ((argc != 3) && (argc != 5))
	{
		cout << "Usage: " << argv[0];
		cout << " [-s <server_host>] -p <udp_port>" << endl;
		return 1;
	}
	else
	{
		//system("clear");
		for (int i = 1; i < argc; i++)
		{				
			if (strcmp(argv[i], "-p") == 0)
				udp_port = (unsigned short) atoi(argv[++i]);
			else if (strcmp(argv[i], "-s") == 0)
			{
				server_host = argv[++i];
				if (argc == 3)
				{
				    cout << "Usage: " << argv[0];
		            cout << " [-s <server_host>] -p <udp_port>" << endl;
		            return 1;
				}
		    }
	        else
	        {
	            cout << "Usage: " << argv[0];
		        cout << " [-s <server_host>] -p <udp_port>" << endl;
		        return 1;
	        }
		}
	}
	
	
	
	Client_State_T client_state = WAITING;
	string in_cmd;
	while(true)
	{
	    usleep(100);
	    switch(client_state)
	    {
	        case WAITING:
	        {
	            cout<<"$ ";
	            cin>>in_cmd;
	            
	            if(in_cmd == "ls")
	            {
	                client_state = PROCESS_LS;
	            }
	            else if(in_cmd == "send")
	            {
	                client_state = PROCESS_SEND;
	            }
	            else if(in_cmd == "remove")
	            {
	                client_state = PROCESS_REMOVE;
	            }
	            else if(in_cmd == "rename")
                {
                    client_state = PROCESS_RENAME;
                }
	            else if(in_cmd == "shutdown")
	            {
	                client_state = SHUTDOWN;
	            }
	            else if(in_cmd == "quit")
	            {
	                client_state = QUIT;
	            }
	            else
	            {
	                cout<<" - wrong command."<<endl;
	                client_state = WAITING;
	            }
	            break;
	        }
	        case PROCESS_LS:
	        {  
		        client_state = WAITING;
	            break;
	        }
	        case PROCESS_SEND:
	        {
	            
		        client_state = WAITING;
		        break;
	        }
	        case PROCESS_REMOVE:
	        {	           
	            client_state = WAITING;
	            break;
	        }
	        case PROCESS_RENAME:
	        {	           
	            client_state = WAITING;
	            break;
	        }	
	        case SHUTDOWN:
	        {	            
	            break;	            
	        }
	        case QUIT:
	        {	         
	        }
	        default:
	        {
	        	client_state = WAITING;
	            break;
	        }    
	    }
	}
    return 0;
}
