/*Authors: Yoav Bruker:206020372 and Dor Orbach:204589790.
* main function of the server program.
*/
#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS

//includes ---------------------------------------------
#include "server_connect.h"

int main(int argc, char* argv[]) {
	char* port_input = argv[1];
	int port = atoi(port_input);
	create_server(port);
	return 0;
}