/*
 *  message.h
 */

#ifndef _MESSAGE_H_
#define _MESSAGE_H_

#include <string>
#include <map>
using namespace std;

#define DATA_BUF_LEN 3000
#define FILE_NAME_LEN 128

typedef struct DATA_MSG_tag
{
    char data[DATA_BUF_LEN]; // filename
}Data_Msg_T;

typedef enum CMD_tag
{
    CMD_LS = 1,
    CMD_SEND = 2,
    CMD_GET = 3,
    CMD_REMOVE = 4,
    CMD_RENAME = 5,
    CMD_SHUTDOWN = 6,
    CMD_QUIT = 7,
    CMD_ACK = 8,
}Cmd_T;

map<uint8_t, string> CMD_TAG_MAP = {
    {1, "CMD_LS"},
    {2, "CMD_SEND"},
    {3, "CMD_GET"},
    {4, "CMD_REMOVE"},
    {5, "CMD_RENAME"},
    {6, "CMD_SHUTDOWN"},
};

typedef struct CMD_MSG_tag
{
    uint8_t cmd;
    char filename[FILE_NAME_LEN];
    char expected_filename[FILE_NAME_LEN]; // for rename file
    uint32_t size;
    uint16_t port;
    uint16_t error;
}Cmd_Msg_T;
#endif
