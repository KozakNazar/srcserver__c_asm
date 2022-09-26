#define _CRT_SECURE_NO_WARNINGS
#define _CRT_NON_CONFORMING_SWPRINTFS
#define WIN32_LEAN_AND_MEAN // êîìïîíåíòè ç çàãîëîâê³â Windows, 
                            // ùî ð³äêî çàñòîñîâóþòüñÿ
/******************************************************************
* N.Kozak // Lviv'2019 // example server C(LLNW)-Asm for pkt2 SP  *
*                         file: server.cpp                        *
*                                                          files: *
*                                                        calc.asm *
*                                                      server.cpp *
*                                                                 *
*                                               *extended version *
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
extern "C" float calc(double b2, float c1, double d2, float e1, double f2); // C++
//extern float calc(double b2, float c1, double d2, float e1, double f2);

#define PORT 80 // 8080

#define REQUEST_MAX_LENGTH 32768
#define RESPONSE_MAX_LENGTH 16384
#define HTML_CODE_MAX_LENGTH 8192

char usePostSubmit = true; // defaault value

#define K 0x00025630 // (153136)
double b2 = 10.;  // defaault value
float c1 = 20.;   // defaault value
double d2 = 30.;  // defaault value
float e1 = 40.;   // defaault value
double f2 = 50.;  // defaault value

char *http_response_fmt = (char*)
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

char *http_response_server_stopped = (char*)
"HTTP / 1.1 200 OK\r\n"
"Date : Mon, 27 Jul 2009 12 : 28 : 53 GMT\r\n"
"Server : Apache / 2.2.14 (Win32)\r\n"
"Last - Modified : Wed, 22 Jul 2009 19 : 15 : 56 GMT\r\n"
"Content - Length : 57\r\n"
"Content - Type : text / html\r\n"
"Connection : Closed\r\n"
"\r\n"
"<html>\r\n"
"<body>\r\n"
"<h1>" "Server stopped" "</h1>\r\n"
"</body>\r\n"
"</html>"
;

char * html_code_fmt__withGetSubmit = (char*)
"<html>\r\n"
"<head>\r\n"
"<link rel = \"icon\" href = \"data:;base64,=\">\r\n"
"</head>\r\n"
"<body>\r\n"
"\r\n"
"<form action = \"/setSettings\" method=\"get\">\r\n"
"<h1>Settings:</h1>\r\n"
"<p>Select mode:</p>\r\n"
"<input type = \"radio\" name = \"mode\" value = \"1\"> mode 1<br>\r\n"
"<input type = \"radio\" name = \"mode\" value = \"2\"> mode 2<br>\r\n"
"<input type = \"radio\" name = \"mode\" value = \"3\"> mode 3<br>\r\n"
"<p>Change used http - method : </p>\r\n"
"<input type = \"radio\" name = \"http_method\" value = \"0\" checked = \"checked\"> GET<br>\r\n"
"<input type = \"radio\" name = \"http_method\" value = \"1\"> POST<br>\r\n"
"\r\n"
"<input type = \"submit\" value = \"Submit parameters and reload page\" text = \"\">\r\n"
"</form>\r\n"
"<h1>Compute board:</h1>\r\n"
"<button type = \"submit\" form = \"calcData\">Send values by GET and compute result</button>\r\n"
"\r\n"
"<form id = \"calcData\"  method = \"get\" action = \"callCalc\">\r\n"
"\r\n"
"<h2>X = K + B2 - D2/C1 + E1*F2</h2>\r\n"
"<h2>--------------------------</h2>\r\n"
"<h2>K = %d</h2>\r\n"
"<h2>B = <input name = \"B\" type = \"text\" value = \"%lf\"></h2>\r\n"
"<h2>C = <input name = \"C\" type = \"text\" value = \"%f\"></h2>\r\n"
"<h2>D = <input name = \"D\" type = \"text\" value = \"%lf\"></h2>\r\n"
"<h2>E = <input name = \"E\" type = \"text\" value = \"%f\"></h2>\r\n"
"<h2>F = <input name = \"F\" type = \"text\" value = \"%lf\"></h2>\r\n"
"<h2>-------</h2>\r\n"
"<h2>X(Assembly) = %f</h2>\r\n"
"<h2>X(Go) = %f</h2>\r\n"
"<h2>--------------------------</h2>\r\n"
"<input type = \"submit\" value = \"Send values by GET and compute result\">\r\n"
"\r\n"
"</form>\r\n"
"</body>\r\n"
"</html>\r\n"
;

char *html_code_fmt__withPostSubmit = (char*)
"<html>\r\n"
"<head>\r\n"
"<link rel = \"icon\" href = \"data:;base64,=\">\r\n"
"</head>\r\n"
"<body>\r\n"
"\r\n"
"<form action = \"/setSettings\" method=\"post\">\r\n"
"<h1>Settings:</h1>\r\n"
"<p>Select mode:</p>\r\n"
"<input type = \"radio\" name = \"mode\" value = \"1\"> mode 1<br>\r\n"
"<input type = \"radio\" name = \"mode\" value = \"2\"> mode 2<br>\r\n"
"<input type = \"radio\" name = \"mode\" value = \"3\"> mode 3<br>\r\n"
"<p>Change used http - method : </p>\r\n"
"<input type = \"radio\" name = \"http_method\" value = \"0\"> GET<br>\r\n"
"<input type = \"radio\" name = \"http_method\" value = \"1\" checked = \"checked\"> POST<br>\r\n"
"\r\n"
"<input type = \"submit\" value = \"Submit parameters and reload page\" text = \"\">\r\n"
"</form>\r\n"
"<h1>Compute board:</h1>\r\n"
"<button type = \"submit\" form = \"calcData\">Send values by POST and compute result</button>\r\n"
"\r\n"
"<form id = \"calcData\"  method = \"post\" action = \"callCalc\">\r\n"
"\r\n"
"<h2>X = K + B2 - D2/C1 + E1*F2</h2>\r\n"
"<h2>--------------------------</h2>\r\n"
"<h2>K = %d</h2>\r\n"
"<h2>B = <input name = \"B\" type = \"text\" value = \"%lf\"></h2>\r\n"
"<h2>C = <input name = \"C\" type = \"text\" value = \"%f\"></h2>\r\n"
"<h2>D = <input name = \"D\" type = \"text\" value = \"%lf\"></h2>\r\n"
"<h2>E = <input name = \"E\" type = \"text\" value = \"%f\"></h2>\r\n"
"<h2>F = <input name = \"F\" type = \"text\" value = \"%lf\"></h2>\r\n"
"<h2>-------</h2>\r\n"
"<h2>X(Assembly) = %f</h2>\r\n"
"<h2>X(C++) = %f</h2>\r\n"
"<h2>--------------------------</h2>\r\n"
"<input type = \"submit\" value = \"Send values by POST and compute result\">\r\n"
"\r\n"
"</form>\r\n"
"</body>\r\n"
"</html>\r\n"
;

void buildResponse(char * html_code, char * http_response){
	memset(html_code, 0, HTML_CODE_MAX_LENGTH); // html_code[HTML_CODE_MAX_LENGTH - 1] = '\0';
	memset(http_response, 0, RESPONSE_MAX_LENGTH); // http_response[RESPONSE_MAX_LENGTH - 1] = '\0';

	float x_AssemblyResult = calc(b2, c1, d2, e1, f2);
	float x_CResult = (double)K + b2 - d2 / (double)c1 + (double)e1 * f2;

	char * html_code_fmt = html_code_fmt__withGetSubmit;

	if (usePostSubmit){
		html_code_fmt = html_code_fmt__withPostSubmit;
	}

	sprintf(html_code, html_code_fmt, K, b2, c1, d2, e1, f2, x_AssemblyResult, x_CResult);
	sprintf(http_response, http_response_fmt, strlen(html_code), html_code);
}

int handleClient(int new_socket, char * html_code, char * http_response){
	long valread;

	char buffer[REQUEST_MAX_LENGTH] = { '\0' }; // TODO: optimize
	valread = read(new_socket, buffer, REQUEST_MAX_LENGTH);
	printf("\r\n>> received new request(server lock, used one thread)\r\n");
	if (valread <= 0){		
	    buildResponse(html_code, http_response);
        
	    write(new_socket, http_response, strlen(http_response));
	    printf("\r\n>> response sent\r\n");
	    close(new_socket);
        
	    return 0;				
	}	
	printf("%s\r\n", buffer);
	if (strstr(buffer, "close") != NULL){
		write(new_socket, http_response_server_stopped, strlen(http_response_server_stopped));
		printf("\r\n>> Server stopped!\r\n");
		close(new_socket);
		return -1;//break;
	}

	char * http_method_key =  (char*)"http_method=";
	char * B_key =  (char*)"B=";
	char * C_key =  (char*)"C=";
	char * D_key =  (char*)"D=";
	char * E_key =  (char*)"E=";
	char * F_key =  (char*)"F=";

	char * strPOSTValues = NULL, * strPtr = NULL;

	if (strstr(buffer, "POST") && (strPOSTValues = strstr(buffer, "\r\n\r\n"))) {
		usePostSubmit = 1;

		if (strPtr = strstr(strPOSTValues, http_method_key)) {
			(void)sscanf(strPtr + strlen(http_method_key), "%hhd", &usePostSubmit);
		}

		if (strPtr = strstr(strPOSTValues, B_key)) {				
			(void)sscanf(strPtr + strlen(B_key), "%lf", &b2);
		}
		if (strPtr = strstr(strPOSTValues, C_key)) {
			(void)sscanf(strPtr + strlen(C_key), "%f", &c1);
		}
		if (strPtr = strstr(strPOSTValues, D_key)) {
			(void)sscanf(strPtr + strlen(D_key), "%lf", &d2);
		}
		if (strPtr = strstr(strPOSTValues, E_key)) {
			(void)sscanf(strPtr + strlen(E_key), "%f", &e1);
		}
		if (strPtr = strstr(strPOSTValues, F_key)) {
			(void)sscanf(strPtr + strlen(F_key), "%lf", &f2);
		}
	}
	else{
		if (strPtr = strstr(buffer, http_method_key)) {
			(void)sscanf(strPtr + strlen(http_method_key), "%hhd", &usePostSubmit);
		}
		else{
		    if (strPtr = strstr(buffer, B_key)) {
		    	usePostSubmit = 0;
		    	(void)sscanf(strPtr + strlen(B_key), "%lf", &b2);
		    }
		    if (strPtr = strstr(buffer, C_key)) {
		    	usePostSubmit = 0;
		    	(void)sscanf(strPtr + strlen(C_key), "%f", &c1);
		    }
		    if (strPtr = strstr(buffer, D_key)) {
		    	usePostSubmit = 0;
		    	(void)sscanf(strPtr + strlen(D_key), "%lf", &d2);
		    }
		    if (strPtr = strstr(buffer, E_key)) {
		    	usePostSubmit = 0;
		    	(void)sscanf(strPtr + strlen(E_key), "%f", &e1);
		    }
		    if (strPtr = strstr(buffer, F_key)) {
		    	usePostSubmit = 0;
		    	(void)sscanf(strPtr + strlen(F_key), "%lf", &f2);
		    }
		}
	}

	buildResponse(html_code, http_response);

	write(new_socket, http_response, strlen(http_response));
	printf("\r\n>> response sent\r\n");
	close(new_socket);

	return 0;
}

int main(int argc, char const *argv[]){
	int server_fd, new_socket; long valread;
	struct sockaddr_in address;
	int addrlen = sizeof(address);

	char * html_code = (char *)malloc(sizeof(char)* HTML_CODE_MAX_LENGTH);
	char * http_response = (char *)malloc(sizeof(char)* RESPONSE_MAX_LENGTH);

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
		if ((new_socket = accept(server_fd, (struct sockaddr *)&address, /*(socklen_t*)*/&addrlen)) < 0){
			close(server_fd);
#if defined _WIN64 || defined _WIN32
			WSACleanup();
#endif
			perror("In accept");
			exit(EXIT_FAILURE);
		}

		if (handleClient(new_socket, html_code, http_response)){
			break;
		}
	}

	close(server_fd);
#if defined _WIN64 || defined _WIN32
	WSACleanup();
#endif

	return 0;
}
