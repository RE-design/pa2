#include <stdlib.h>
#include <assert.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include<glib.h>
#include <stdbool.h>

typedef struct RequestLine{
	int rlSize;
	int numberOfHeaders;
	char requestType[5];
	char version[9];
	char headers[15][40];
	char URL[40];
	char body[100];
	char urlCommand[6];
	char color[10];
	char urlArgs[40];
	char cookie[16];
	char requestCookie[16];	
}RequestLine;

typedef struct ResponseLine{
	int rsLSize;
	char statuscode[4];
	char URI[80];
	char version[9];
	char host[40];
	char conType[24];
	char server[27];
	char conClose[18];
	char conAlive[23];
	char date[21];
	char responseLine[256];
	char reasonPhrase[12];
	bool connectionAlive;
	char clientIP[INET_ADDRSTRLEN];
	char clientPort[6]; 
}ResponseLine;

typedef struct IPtoIndex{
	int index;
	char clientIP[INET_ADDRSTRLEN];
}IPtoIndex;

void setIPindex(IPtoIndex *IP, struct sockaddr_in client, int *count){
	printf("inIPindex\n");
	struct sockaddr_in* ip4Add = (struct sockaddr_in*)&client;
	int ipAddr = ip4Add->sin_addr.s_addr;
	inet_ntop( AF_INET, &ipAddr, IP[*count].clientIP, INET_ADDRSTRLEN); 
	IP[*count].index = *count;
	*count++; 
}
void constructURI(ResponseLine *RsL, RequestLine RL){
	memset(RsL->URI, '\0', sizeof(RsL->URI));
	strcat(RsL->URI, RsL->host);
	strcat(RsL->URI, RL.URL);
}

void setClientIPandPort(ResponseLine *RsL, struct sockaddr_in client){
	memset(RsL->clientPort, '\0', sizeof(RsL->clientPort));
 	memset(RsL->clientIP, '\0', sizeof(RsL->clientIP));
	struct sockaddr_in* ip4Add = (struct sockaddr_in*)&client;
	int ipAddr = ip4Add->sin_addr.s_addr;
	inet_ntop( AF_INET, &ipAddr, RsL->clientIP, INET_ADDRSTRLEN);
	int portnum = (int) ntohs(client.sin_port);
	sprintf(RsL->clientPort, "%d", portnum);
}

void setUrlArgs(RequestLine *RL){
	printf("seturlargs\n");
	memset(RL->urlCommand, '\0', sizeof(RL->urlCommand));
	memset(RL->color, '\0', sizeof(RL->color));
	memset(RL->urlArgs, '\0', sizeof(RL->urlArgs));
	memset(RL->cookie, '\0', sizeof(RL->cookie));

	int i = 1;
	int  counter = 0;
	if(RL->URL[i] == '\0'){
		RL->urlCommand[counter] = '\0';
		RL->urlArgs[counter] = '\0';
		return;
	}
	    while(RL->URL[i] != '?' && RL->URL[i] != '\0'){
		RL->urlCommand[counter] = RL->URL[i];
		i++;
		counter++;
	}
	counter = 0;
	int cookiecount = 0;
	if(strcmp(RL->urlCommand, "color") == 0){
		while(RL->URL[i-1] != '=' && RL->URL[i] != '\0'){		
			i++;
		}
		
		while(RL->URL[i] != '\0'){
		    RL->color[counter] = RL->URL[i];
		    i++;
		    counter++;
		}
		if(RL->color[1] != '\0'){
			strcat(RL->cookie, " color=");
			strcat(RL->cookie, RL->color);
		}
	}else{
		printf("URL%s\n", RL->URL);
	    i++;
	    while(RL->URL[i] != '\0'){
		if(RL->URL[i] == '&'){
		    RL->urlArgs[counter] = '\n';
		    counter++;
		    i++;
		}
		RL->urlArgs[counter] = RL->URL[i];
			i++;
			counter++;
	    }
	}
		RL->urlArgs[i] = '\0';
	printf("urlargs:%s\n", RL->urlArgs);
}

//Checks if the connection is keep alive and gets the host
void setValuesFromRequestHeaders(ResponseLine *RsL, RequestLine RL){
	printf("setVFRH\n");
	memset(RsL->host, '\0', sizeof(RsL->host));
	RsL->connectionAlive = false;
	int i, j;
	for (i = 0; i < RL.numberOfHeaders; i++){
	    if(strncmp(RL.headers[i], "Connection: keep-alive", 22) == 0){
		RsL->connectionAlive = true;
		printf("connection is keep alice");	
	    }
	    if(strncmp(RL.headers[i], "Host:",4) == 0){
		int count = 0;
		for(j = 6; RL.headers[i][j] != '\0'; j++){
		    RsL->host[count] = RL.headers[i][j];
		    count++;
		}

		RsL->host[count] = '\0';
	    }
	}	
}

void constructResponseLine(RequestLine RL, ResponseLine *RsL){	   
	    setValuesFromRequestHeaders(RsL, RL);
		memset(RsL->version, '\0', sizeof(RsL->version));
		memset(RsL->conType, '\0', sizeof(RsL->conType));
		memset(RsL->server, '\0', sizeof(RsL->server));
		memset(RsL->conClose, '\0', sizeof(RsL->conClose));
		memset(RsL->conAlive, '\0', sizeof(RsL->conAlive));
		memset(RsL->date, '\0', sizeof(RsL->date));
		memset(RsL->responseLine, '\0', sizeof(RsL->responseLine));
		memset(RsL->reasonPhrase, '\0', sizeof(RsL->reasonPhrase));
	   constructURI(RsL, RL);
	   time_t t;
       struct tm *tmpt;
	
           t = time(NULL);
           tmpt = localtime(&t);
           if (tmpt == NULL) {
               perror("getting localtime failed");
           }
	               printf("CRLtop\n"); 
	strftime(RsL->date,21,"%Y-%m-%d %H:%M:%S", tmpt);
	RsL->date[20] = '\0';
	strcpy(RsL->version, RL.version);
	if(memcmp(RsL->version, "HTTP/1.0", 7) != 0 && 
		memcmp(RsL->version, "HTTP/1.0", 7) != 0){
		strcpy(RsL->statuscode, "300");
		strcpy(RsL->reasonPhrase, "Bad Request"); 
	}
	else{
		strcpy(RsL->statuscode, "200");
		strcpy(RsL->reasonPhrase, "OK");
	}
;
	strcpy(RsL->server,"Server: Simple HTTP server");
	strcpy(RsL->conType, "Content-Type: text/html");
	strcpy(RsL->conAlive, "Connection: keep-alive");
	strcpy(RsL->conClose, "Connection: close");
	strcat(RsL->responseLine, " ");
	strcat(RsL->responseLine, RsL->version);
	strcat(RsL->responseLine," ");
	strcat(RsL->responseLine, RsL->statuscode);
	strcat(RsL->responseLine, " ");
	strcat(RsL->responseLine, RsL->reasonPhrase);
	strcat(RsL->responseLine, "\r\n");	
	strcat(RsL->responseLine, "date: ");
	strcat(RsL->responseLine, RsL->date);
	strcat(RsL->responseLine, "\r\n"); 	
	strcat(RsL->responseLine, RsL->conType);
	strcat(RsL->responseLine, "\r\n");
	strcat(RsL->responseLine, RsL->server);
	strcat(RsL->responseLine, "\r\n");
	if(RL.cookie[0] != '\0'){
		strcat(RsL->responseLine, "Set-Cookie: ");
		strcat(RsL->responseLine, RL.cookie);
		strcat(RsL->responseLine, "\r\f");
	}
	if(RsL->connectionAlive == false){
		strcat(RsL->responseLine, RsL->conClose);
	}
	else{
		strcat(RsL->responseLine, RsL->conAlive);
	}
	strcat(RsL->responseLine, "\r\n");
	strcat(RsL->responseLine, "\r\n");
	RsL->rsLSize = strlen(RsL->responseLine);
	printf("response:! %s\n", RsL->responseLine);
}

void fillRequestStruct(char *buffer, RequestLine *RL){
	printf("fillrequeststruct\n");
	memset(RL->version, '\0', sizeof(RL->version));
	memset(RL->headers, '\0', sizeof(RL->headers));
	memset(RL->URL, '\0', sizeof(RL->URL));
	memset(RL->body, '\0', sizeof(RL->body));

	int index = 0;
	int counter = 0;
	RL->rlSize = 0;
	while(buffer[index] != ' '){
		RL->requestType[counter] = buffer[index];
		index++;
		counter++;
		RL->rlSize++;
	}
	
	RL->requestType[counter] = '\0';
	counter = 0;
	index++;
	RL->rlSize++;
	printf("mid fill request\n");
	while(buffer[index] != ' '){
		RL->URL[counter] = buffer[index];
		index++;
		counter++;
		RL->rlSize++;
	}
	printf("afterurl\n");

	RL->URL[counter] = '\0';
	counter = 0;
	index++;
	RL->rlSize++;
	while(buffer[index] != '\r'){

		RL->version[counter] = buffer[index];
		printf("%c", RL->version[counter]);
		index++;
		counter++;
		RL->rlSize++; 
	}
	printf("after version\n");	
	RL->rlSize = RL->rlSize + 2; 
	RL->version[counter] = '\0';
	int HL = 0;
	RL->numberOfHeaders = 0;
	while(1){	
	    counter = 0;
 	    while(buffer[RL->rlSize] != '\r' && buffer[RL->rlSize + 1] != '\n'){
		RL->headers[HL][counter] = buffer[RL->rlSize];
		RL->rlSize++;
		printf("%c", RL->headers[HL][counter]);	
		counter++;
	    }
		RL->headers[HL][counter] = '\0';
		HL++;
			printf("\n");
		counter = 0;
		RL->rlSize = RL->rlSize + 2;
		if((buffer[RL->rlSize] == '\r' && buffer[RL->rlSize + 1] == '\n') || HL >= 15){
		    RL->rlSize = RL->rlSize + 2;
		    break;}
	}
	
	RL->numberOfHeaders = HL;
	index = RL->rlSize;
	counter = 0;
	printf("read headers\n");
	if(strcmp(RL->requestType, "POST") == 0){
		while(buffer[index] != '\0'){
		    RL->body[counter] = buffer[index];
		    index++;
		    counter++;
		}

		RL->body[counter] = '\0';
		printf("body:%s\n", RL->body);
	}
	setUrlArgs(RL);
}

void htmlToBuffer(char *replyMessage, RequestLine RL,ResponseLine RsL){
	if(strcmp(RL.requestType, "HEAD") == 0){return;}
	char document[800];
  	memset(document, '\0', sizeof(document));
  
  	strcpy(document, "<!DOCTYPE html>\r\n<html>\r\n\t<head>\r\n\t\t<title>Sample HTML</title> />\r\n\t</head>\r\n\t<body");
  	char pContent[200];
  	char pOpen[] = "\t\t<p>\n";;
  	char pClose[] = "</p>\r\n";
  	char docClose[] = "\t</body>\r\n</html>\r\n\r\n\0";
  	char closingSymbol[] = ">";
  	if(strcmp(RL.urlCommand, "color") == 0){
			strcat(document, " style='background-color:");
			
    	    strcat(document, RL.color);
    	    strcat(document, "'>\n");
  	}
  	else if (strcmp(RL.urlCommand, "test") == 0){
			printf("TEST");
	    strcat(document, ">\n");
        strcat(pContent, pOpen);
			strcat(pContent, "\t\t\t");
            strcat(pContent, RsL.URI);
            strcat(pContent, "<br />\n\t\t\t");
    	    strcat(pContent, RsL.clientIP);
			strcat(pContent, ":");
			strcat(pContent, RsL.clientPort);
            strcat(pContent, "<br />\n\t\t\t");
            strcat(pContent, RL.urlArgs);
            strcat(pContent, "<br />\n\t\t");
            strcat(pContent, pClose);
	        strcat(document, pContent);
  	}
	else{
		strcat(pContent, ">\n");
		strcat(pContent, pOpen);
		strcat(pContent, "\t\t\t");
        strcat(pContent, RsL.URI);
        strcat(pContent, "<br />\n\t\t\t");
    	strcat(pContent, RsL.clientIP);
        strcat(pContent, ":");
	    strcat(pContent, RsL.clientPort);
        strcat(pContent, "<br />\n\t\t");
		strcat(pContent, pClose);
		strcat(document, pContent);
	}

 	if(strcmp(RL.requestType, "POST") == 0){
 	       strcat(document, "\t\t<p>\n\t\t");
	       strcat(document, RL.body);
		   strcat(document, "\n");
		   strcat(document, "\t\t</p>\n");
       }
	strcat(document, docClose); 

 	memcpy(replyMessage + RsL.rsLSize, document, sizeof(document));
}
void writeMessage(char *buffer, ResponseLine RsL){
	memcpy(buffer + 0, RsL.responseLine, sizeof(RsL.responseLine));
	buffer[strlen(RsL.responseLine)] = '\0';	

}

void writeToFile(struct sockaddr_in client, RequestLine RL, ResponseLine RsL){

	FILE* fd;
	fd = fopen("request.log", "a");
	if(fd == NULL){	
	    fd = fopen("request.log", "w+");
		if(fd == NULL){
		printf("error opening file");
	    }
	}	

	if(fputs(RsL.date, fd) < 0)printf("error writing to file");
	fputs(" : ", fd);
	fputs(RsL.clientIP, fd);
	fputs(":", fd);
	fputs(RsL.clientPort, fd);
 	fputs(" ", fd);
	fputs(RL.requestType, fd);
	fputs(" ", fd);
	fputs(RsL.URI, fd);
	fputs(":", fd);
	fputs(RsL.statuscode, fd);
	fputs("\r\n", fd);
	fclose(fd);
}

int main(int argc, char **argv)
{
		struct timeval tv;
		const int MAXCLIENTS = 20;
		int yes = 1;	
        int i, max_fd, sockfd, clients[MAXCLIENTS], retval, connfd, j;
        struct sockaddr_in server, client;
		fd_set rfds, master;
		FD_ZERO(&rfds);
		FD_ZERO(&master);
        char message[512];
		char replyMessage[1024];
       	IPtoIndex IP[MAXCLIENTS];
		int countIP = 0;
		memset(IP, 0, sizeof(IP));
        sockfd = socket(AF_INET, SOCK_STREAM, 0);
        memset(&server, 0, sizeof(server));
        server.sin_family = AF_INET;
        server.sin_addr.s_addr = htonl(INADDR_ANY);
        server.sin_port = htons(atoi(argv[1]));
		

	//	setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)); 
        bind(sockfd, (struct sockaddr *) &server, (socklen_t) sizeof(server));
		if(listen(sockfd, 5) < 0){
			perror("listen");
		}
 
		FD_SET(sockfd, &master);
		max_fd = sockfd;
        for (;;) {
				ssize_t n;
				rfds = master;
				tv.tv_sec = 30;
                tv.tv_usec = 0;
                retval = select( max_fd + 1, &rfds, NULL, NULL, &tv);
                if (retval == 0) {
                        perror("select()");
                }
				else if(retval == 0){
					printf("retval is 0");
				}
				printf("retval:%d\n", retval);
                /* Check whether there is data on the socket fd. */
				for (i = 0; i <= max_fd; i++){
				//	printf("forloop;\nmax_fd:%d\n", max_fd);
					if(FD_ISSET(i, &rfds)){
						printf("socket:%d  %ld.%06ld\n",i , tv.tv_sec); 
						printf("isset!\n");
						FD_SET(i, &rfds);
						if(i == sockfd){
							printf("i == sockfd\n");
							socklen_t len = (socklen_t) sizeof(client);
                        	connfd = accept(i, (struct sockaddr *) &client, &len);
							if(connfd == -1){
								perror("accept()");
							}
							else{
								FD_SET(connfd, &master);
								if(connfd > max_fd){
									max_fd = connfd;
								}
							
						//		setIPindex(IP, client, &countIP);
							}			
		 				}else{
							RequestLine RL;
							ResponseLine RsL;
							memset(&message, 0, sizeof(message));
							memset(&replyMessage, 0, sizeof(replyMessage));
							if(n = read(i, message, sizeof(message) - 1) < 0){
								perror("read()");
							}
								printf("reading\n");
								fprintf(stdout, "MessageRequest!:%s\n", message);
								fillRequestStruct(message,&RL);
								constructResponseLine(RL, &RsL);
								setClientIPandPort(&RsL, client);
								writeMessage(replyMessage, RsL);
								htmlToBuffer(replyMessage, RL, RsL);		
								
		
	    	            	if(write(i, replyMessage, sizeof(replyMessage)) < 0){
								perror("writing failed");
							}	
			
 							writeToFile(client, RL, RsL); 



					    	if(RsL.connectionAlive == true && tv.tv_sec > 0){
			  					shutdown(i, SHUT_RDWR);
								close(i);
								FD_CLR(i, &master); 
							}
							else{
			   					shutdown(i, SHUT_RDWR);
                			    close(i);
								FD_CLR(i, &master);
							}
							memset(&RL, 0, sizeof(RL));
							memset(&RsL, 0, sizeof(RsL));
							

							
				}							                    
			}
					fflush(stdout);		
		}
	}
}
