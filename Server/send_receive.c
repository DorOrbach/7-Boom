/*This module is taken almost exactly from recitaion 10 code example, so we won't elaborate about each function.
* except for the last one becasue it is original!!
*/

#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include "send_receive.h"
#include "server_connect.h"
#include <stdio.h>
#include <string.h>

//The functions for send and recieve are taken from the recitation 10 code example:

TransferResult_t SendBuffer(const char* Buffer, int BytesToSend, SOCKET sd)
{
	const char* CurPlacePtr = Buffer;
	int BytesTransferred;
	int RemainingBytesToSend = BytesToSend;

	while (RemainingBytesToSend > 0)
	{
		/* send does not guarantee that the entire message is sent */
		BytesTransferred = send(sd, CurPlacePtr, RemainingBytesToSend, 0);
		if (BytesTransferred == SOCKET_ERROR)
		{
			//printf("send() failed, error %d\n", WSAGetLastError());
			return TRNS_FAILED;
		}

		RemainingBytesToSend -= BytesTransferred;
		CurPlacePtr += BytesTransferred; // <ISP> pointer arithmetic
	}

	return TRNS_SUCCEEDED;
}

TransferResult_t SendString(const char* Str, SOCKET sd)
{
	/* Send the the request to the server on socket sd */
	int TotalStringSizeInBytes;
	TransferResult_t SendRes;

	/* The request is sent in two parts. First the Length of the string (stored in
	   an int variable ), then the string itself. */

	TotalStringSizeInBytes = (int)(strlen(Str) + 1); // terminating zero also sent	

	SendRes = SendBuffer(
		(const char*)(&TotalStringSizeInBytes),
		(int)(sizeof(TotalStringSizeInBytes)), // sizeof(int) 
		sd);

	if (SendRes != TRNS_SUCCEEDED) return SendRes;

	SendRes = SendBuffer(
		(const char*)(Str),
		(int)(TotalStringSizeInBytes),
		sd);

	return SendRes;
}

TransferResult_t ReceiveBuffer(char* OutputBuffer, int BytesToReceive, SOCKET sd)
{
	char* CurPlacePtr = OutputBuffer;
	int BytesJustTransferred;
	int RemainingBytesToReceive = BytesToReceive;

	while (RemainingBytesToReceive > 0)
	{
		/* send does not guarantee that the entire message is sent */
		BytesJustTransferred = recv(sd, CurPlacePtr, RemainingBytesToReceive, 0);
		if (BytesJustTransferred == SOCKET_ERROR)
		{
			//printf("recv() failed, error %d\n", WSAGetLastError());
			return TRNS_FAILED;
		}
		else if (BytesJustTransferred == 0)
			return TRNS_DISCONNECTED; // recv() returns zero if connection was gracefully disconnected.

		RemainingBytesToReceive -= BytesJustTransferred;
		CurPlacePtr += BytesJustTransferred; // <ISP> pointer arithmetic
	}

	return TRNS_SUCCEEDED;
}


TransferResult_t ReceiveString(char** OutputStrPtr, SOCKET sd)
{
	/* Recv the the request to the server on socket sd */
	int TotalStringSizeInBytes;
	TransferResult_t RecvRes;
	char* StrBuffer = NULL;

	if ((OutputStrPtr == NULL) || (*OutputStrPtr != NULL))
	{
		printf("The first input to ReceiveString() must be "
			"a pointer to a char pointer that is initialized to NULL. For example:\n"
			"\tchar* Buffer = NULL;\n"
			"\tReceiveString( &Buffer, ___ )\n");
		return TRNS_FAILED;
	}

	/* The request is received in two parts. First the Length of the string (stored in
	   an int variable ), then the string itself. */

	RecvRes = ReceiveBuffer(
		(char*)(&TotalStringSizeInBytes),
		(int)(sizeof(TotalStringSizeInBytes)), // 4 bytes
		sd);

	if (RecvRes != TRNS_SUCCEEDED) return RecvRes;

	StrBuffer = (char*)malloc(TotalStringSizeInBytes * sizeof(char));

	if (StrBuffer == NULL)
		return TRNS_FAILED;

	RecvRes = ReceiveBuffer(
		(char*)(StrBuffer),
		(int)(TotalStringSizeInBytes),
		sd);

	if (RecvRes == TRNS_SUCCEEDED)
	{
		*OutputStrPtr = StrBuffer;
	}
	else
	{
		free(StrBuffer);
	}

	return RecvRes;
}


//Description:
/*Function that take the received message and break it into the deifferent paramaters.
* It creates a struct that holds the type and parameters of the message, and store the information inside.
*/
Message* break_message_to_params(char* rec_str) {
	char* copy = NULL;
	copy = (char*)malloc(sizeof(char) * strlen(rec_str));
	if (copy == NULL) {
		printf("memory allocation failed\n");
		return -1;
	}
	strcpy(copy, rec_str);

	Message* message = NULL;
	int rec_len = strlen(rec_str);
	message = (Message*)malloc(sizeof(Message) + rec_len * sizeof(char));
	if (NULL == message) {
		printf("memory allocation failed\n");
		return -1;
	}
	//get message type:
	char* message_break = strtok(rec_str, ":");
	rec_len = strlen(message_break);
	message->message_type = (char*)malloc(sizeof(char) * rec_len);
	if (message->message_type == NULL) {
		printf("memory allocation failed\n");
		return -1;
	}
	strcpy(message->message_type, message_break);
	
	//get argument:
	while (copy != NULL) {
		copy = strstr(copy, ":");
		copy++;
		if (copy == NULL) {
			strcpy(message->parameters, "NULL");
			return message;
		}
		strcpy(message->parameters, copy);
		break;
	}
	return message;
}

