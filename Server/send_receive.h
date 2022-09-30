#ifndef SEND_RECIEVE_H
#define SEND_RECIEVE_H

#include "server_connect.h"
#define MAX_PARAMS 1
#define NAME_LEN 21

typedef enum {
	TRNS_FAILED, TRNS_DISCONNECTED, TRNS_SUCCEEDED
} TransferResult_t;


typedef struct Message {
	char* message_type;
	char parameters[NAME_LEN];
}Message;

TransferResult_t SendString(const char* Str, SOCKET sd);
TransferResult_t ReceiveString(char** OutputStrPtr, SOCKET sd);
Message* break_message_to_params(char* rec_str);

#endif // !SEND_RECIEVE_H
