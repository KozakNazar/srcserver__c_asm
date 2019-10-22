#define _CRT_SECURE_NO_WARNINGS
#define _CRT_NON_CONFORMING_SWPRINTFS
#define WIN32_LEAN_AND_MEAN // компоненти з заголовків Windows, 
                            // що рідко застосовуються
/******************************************************************
* N.Kozak // Lviv'2019 // example server C(LLNW)-Asm for pkt2 SP  *
*                         file: server.c                          *
*                                                          files: *
*                                                        calc.asm *
*                                                        server.c *
*******************************************************************/

// Server side C program to demonstrate Socket programming
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined _WIN64 || defined _WIN32
#include <winsock2.h>
//#include <winsock.h> // #include <sys/socket.h>
#include <stdint.h> 
typedef uint32_t socklen_t;
//#include <ws2tcpip.h>
#include <process.h> // #include <unistd.h>
//#include <netinet/in.h>
//#include <windows.h>
#pragma comment(lib, "WS2_32.lib")
#define close closesocket
#define read(S, B, L) recv(S, B, L, 0)
#define write(S, B, L) send(S, B, L, 0)
#else
#include <sys/socket.h>
#include <winsock2.h>
#include <unistd.h>
#include <netinet/in.h>
#endif

//// X = K + B2 - D2/C1 + E1*F2 // K=0x00025630
//extern "C" float calc(double b2, float c1, double d2, float e1, double f2); // C++
extern float calc(double b2, float c1, double d2, float e1, double f2);

#define PORT 80 // 8080

#define REQUEST_MAX_LENGTH 32768
#define RESPONSE_MAX_LENGTH 16384
#define HTML_CODE_MAX_LENGTH 8192

char *http_response_fmt =
"HTTP / 1.1 200 OK\r\n"
"Date : Mon, 27 Jul 2009 12 : 28 : 53 GMT\r\n"
"Server : Apache / 2.2.14 (Win32)\r\n"
"Last - Modified : Wed, 22 Jul 2009 19 : 15 : 56 GMT\r\n"
"Content - Length : %d\r\n"
"Content - Type : text / html\r\n"
"Connection : Closed\r\n"
"\r\n"
"%s"
;
char *html_code_fmt =
"<html>\r\n"
"<body>\r\n"
"<h2>" "X = K + B2 - D2/C1 + E1*F2" "</h2>\r\n"
"<h2>" "--------------------------" "</h2>\r\n"
"<h2>" "K = %d" "</h2>\r\n"
"<h2>" "B = %f" "</h2>\r\n"
"<h2>" "C = %f" "</h2>\r\n"
"<h2>" "D = %f" "</h2>\r\n"
"<h2>" "E = %f" "</h2>\r\n"
"<h2>" "F = %f" "</h2>\r\n"
"<h2>" "-------" "</h2>\r\n"
"<h2>" "X(Assembly) = %f" "</h2>\r\n"
"<h2>" "X(C) = %f" "</h2>\r\n"
"<h2>" "--------------------------" "</h1>\r\n"
"</body>\r\n"
"</html>"
;

char *http_response_server_stopped =
"HTTP / 1.1 200 OK\r\n"
"Date : Mon, 27 Jul 2009 12 : 28 : 53 GMT\r\n"
"Server : Apache / 2.2.14 (Win32)\r\n"
"Last - Modified : Wed, 22 Jul 2009 19 : 15 : 56 GMT\r\n"
"Content - Length : 10\r\n"
"Content - Type : text / html\r\n"
"Connection : Closed\r\n"
"\r\n"
"<html>\r\n"
"<body>\r\n"
"<h1>" "Server stopped" "</h1>\r\n"
"</body>\r\n"
"</html>"
;

int main(int argc, char const *argv[]){
	// -----compute------
    #define K 0x00025630 // (153136)
	double b2 = 10.;
	float c1 = 20.;
	double d2 = 30.;
	float e1 = 40.;
	double f2 = 50.;
	// ------------------

	int server_fd, new_socket; long valread;
	struct sockaddr_in address;
	int addrlen = sizeof(address);
	
	char http_response[RESPONSE_MAX_LENGTH] = { '\0' };
	char html_code[HTML_CODE_MAX_LENGTH] = { '\0' };
	
	float x_AssemblyResult = calc(b2, c1, d2, e1, f2);
	float x_CResult = (double)K + b2 - d2 / (double)c1 + (double)e1 * f2;	

	sprintf(html_code, html_code_fmt, K, b2, c1, d2, e1, f2, x_AssemblyResult, x_CResult);
	sprintf(http_response, http_response_fmt, strlen(html_code), html_code);

#if defined _WIN64 || defined _WIN32
	WSADATA wsaData;
	int iResult;
	// Initialize Winsock
	if (iResult = WSAStartup(MAKEWORD(2, 2), &wsaData)){
		printf("WSAStartup failed with error: %d\n", iResult);
		return 1;
	}
#endif

	// Creating socket file descriptor
	if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0){
#if defined _WIN64 || defined _WIN32
		WSACleanup();
#endif
		perror("In socket");
		exit(EXIT_FAILURE);
	}

	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons(PORT);

	memset(address.sin_zero, '\0', sizeof address.sin_zero);

	if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0){
		close(server_fd);
#if defined _WIN64 || defined _WIN32
		WSACleanup();
#endif
		perror("In bind");
		exit(EXIT_FAILURE);
	}

	if (listen(server_fd, 10) < 0){
		close(server_fd);
#if defined _WIN64 || defined _WIN32
		WSACleanup();
#endif
		perror("In listen");
		exit(EXIT_FAILURE);
	}

	while (1){
		printf("\r\n>> Waiting for new connection...");
		if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0){
			close(server_fd);
#if defined _WIN64 || defined _WIN32
			WSACleanup();
#endif
			perror("In accept");
			exit(EXIT_FAILURE);
		}

		char buffer[REQUEST_MAX_LENGTH] = { '\0' };
		valread = read(new_socket, buffer, REQUEST_MAX_LENGTH);
		printf("\r\n>> received new request(server lock, used one thread)\r\n");
		printf("%s\r\n", buffer);
		if (strstr(buffer, "close") != NULL){
			write(new_socket, http_response_server_stopped, strlen(http_response_server_stopped));
			printf("\r\n>> Server stopped!\r\n");
			close(new_socket);
			break;
		}
		write(new_socket, http_response, strlen(http_response));
		printf("\r\n>> response sent\r\n");
		close(new_socket);
	}

	close(server_fd);
#if defined _WIN64 || defined _WIN32
	WSACleanup();
#endif

	return 0;
}