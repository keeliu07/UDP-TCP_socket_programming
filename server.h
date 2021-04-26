#ifndef _SERVER_H
#define _SERVER_H

#include <vector>
#include <string>
#include <map>

using namespace std;

typedef enum SERVER_STATE_tag
{
    WAITING = 0,
    PROCESS_LS = 1,
    PROCESS_SEND = 2,
    PROCESS_GET = 3,
    PROCESS_REMOVE = 4,
    PROCESS_RENAME = 5,
    SHUTDOWN = 6
}Server_State_T;

// Helper Map for Processing CMD_MSG_tag cmd
map<uint8_t, Server_State_T> SERVER_STATE_MAP = {
    {0, WAITING},
    {1, PROCESS_LS},
    {2, PROCESS_SEND},
    {3, PROCESS_GET},
    {4, PROCESS_REMOVE},
    {5, PROCESS_RENAME},
    {6, SHUTDOWN}
};

bool checkFile(const char *fileName);
int checkDirectory (string dir);
int getDirectory (string dir, vector<string> &files);
int invoke_ls(Cmd_Msg_T msg);
#endif
