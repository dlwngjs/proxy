#include <stdio.h>
#include <sys/types.h>   // definitions of a number of data types used in socket.h and netinet/in.h
#include <sys/socket.h>  // definitions of structures needed for sockets, e.g. sockaddr
#include <netinet/in.h>  // constants and structures needed for internet domain addresses, e.g. sockaddr_in
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <ctype.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#define BUFSIZE 4096


int host_name_check(char *host, char **host_list){
	
	
	int check = 0;
	for (int i = 0; i < 100; ++i)
	{		

		if(strcmp(host,host_list[i]) == 0){
		//strcmp는 두 문자열이 같으면 0  달랐을땐 앞에 문자열이 사전에서 더 빨리 찾을수 있으면 1 아니면 -1을 리턴한다.
			check = 1;
			break;
		}
	}


	return check;
}





int main(){

	int proxy_sock_fd_for_clien, client_sock_fd;
	char file_bufer[BUFSIZE];
	char bad_request_message[] = "HTTP/1.1 400 Bad Request\r\nContent-Type: text/html\r\nConnection: close\r\n\r\n";
	

	char not_found_message[] = "HTTP/1.1 404 Not Found\r\n"
	            "Content-type: text/html\r\n"
	            "Content-Length: 116\r\n"
	            "Connection: Keep-Alive\r\n"
	            "\r\n"
	            "<html>\r\n"
	            " <body>\r\n"
	            "   <h1>Not Found<h1>\r\n"
	            "   <p>The requested URL was not found on this server.</p>\r\n"
	            " </body>\r\n"
	            "<html>\r\n";

	bzero(file_bufer,BUFSIZE);
	

	struct sockaddr_in proxy_addr,client_addr, server_addr; //소켓(서버와 클라이언트)의 정보를 담기위한 구조체
	socklen_t client_len; //길이, 크기를 위한변수
	
	proxy_sock_fd_for_clien = socket(AF_INET, SOCK_STREAM, 0);
    if (proxy_sock_fd_for_clien < 0) {
        perror("ERROR opening socket");
        exit(1);


    }
	
	bzero((char *) &proxy_addr, sizeof(proxy_addr)); //서버 소켓의 정보를 위한 구조체를 초기화 해서 0으로 체운다.
	
	proxy_addr.sin_family = AF_INET;
	proxy_addr.sin_addr.s_addr =INADDR_ANY; //현재 컴퓨터의 ip주소를 소켓에 저장
	proxy_addr.sin_port = htons(7027); //프로그램의 포트번호를 변환해서 소켓에 저장

	if(bind(proxy_sock_fd_for_clien, (struct sockaddr *) &proxy_addr,sizeof(proxy_addr)) < 0 ){
		//만들어진 서버 소켓과 서버 구조체를 묶는다. 바인드가 성공하면 0을 리턴하는데 0은 거짓을 의미하므로
		printf("ERROR on binging\n"); //if문이 실행되면 바인드가 실패 되었단 얘기.(bind의 리턴이 0이 아니기떄문에)
		exit(1); //에러 메시지를 띄우고 프로그램을 종료한다.
	}

	listen(proxy_sock_fd_for_clien,5); 

	client_len = sizeof(client_addr);


	int host_file = open("name.txt",O_RDWR); //name.txt파일의 위치정보를 host_file에 담는다.
	char *host_name_list[100]; //문자열을 담을 배열을 100개 선언한다.
	for (int i = 0; i < 100; ++i)
	{
		host_name_list[i] = (char *)malloc(sizeof(char)*50); //실제로 메모리 공간을 할방받는는데 문자가 50개 들어갈 공간만
	}

	
	char buf[1024]={0,};
	char *temp_ptr;
	read(host_file,buf,1024);
	
	temp_ptr = strtok(buf, "\n"); 
	int i =0;

	while(temp_ptr != NULL){
		host_name_list[i] = temp_ptr;
		temp_ptr = strtok(NULL, "\n"); 
		i++;
	}

	close(host_file);



	while(1){

		client_sock_fd = accept(proxy_sock_fd_for_clien, (struct sockaddr *) &client_addr, &client_len);
	    
	    if (client_sock_fd < 0){
	     	perror("ERROR on accept");
	     	write(client_sock_fd,not_found_message,strlen(not_found_message));
	    		continue;

	    }

	    int n = read(client_sock_fd,file_bufer,BUFSIZE-1); //Read is a block function. It will read at most 1023 bytes
	    
	    if (n < 0) {
	    		perror("ERROR reading from socket");
	    		write(client_sock_fd,not_found_message,strlen(not_found_message));
	    		continue;

	    }
	    
	   	printf("----------http request-----------\n");
	     printf("%s\n", file_bufer);
	     printf("----------http request-----------\n");
		if(strlen(file_bufer) <=0) continue;		
		
		char *ptr = NULL;
		char *method = NULL;
		method = strtok(file_bufer," ");
		
		strtok(NULL,"//");
		strtok(NULL,"/");
		
		char *filename =NULL;
		filename = strtok(NULL,"H");
		

		ptr = strtok(NULL,"\r");
		ptr = strtok(NULL,"\n");
		ptr = strtok(ptr," ");
		char *host_name = NULL;
		
		host_name = strtok(NULL,"\r");
		
		
		char request[BUFSIZE*2];
		memset(request,'\0',BUFSIZE*2);
		sprintf(request,"%s /%sHTTP/1.1\r\nHost: %s\r\nConnection: close\r\n\r\n",method,filename,host_name);



		if(host_name_check(host_name,host_name_list) == 0){
			struct hostent *destination = gethostbyname(host_name);

			if (destination == NULL){
		        	perror("Host name error!");
		     	write(client_sock_fd,not_found_message,strlen(not_found_message));
	    			continue;

			}
		    
		   

		    struct sockaddr_in dest_addr;
		    bzero((char *) &dest_addr, sizeof(dest_addr)); //dest를 위한 구조체를 초기화

		    dest_addr.sin_family = AF_INET; //목적지의 주소를 초기화
							
			bcopy((char *)destination->h_addr, (char *)&dest_addr.sin_addr.s_addr, destination->h_length);
			//destination에있는 정보를 dset_addr에 복사
			dest_addr.sin_port = htons(80); //서버의 포트넘버는 항상 80이기때문에 
			
			int proxy_sock_fd_for_dest;

			proxy_sock_fd_for_dest = socket(AF_INET,SOCK_STREAM,0); //목적지와 통신하기 위한 포트를 만든다
			
			
			int ser_sock_fd = connect(proxy_sock_fd_for_dest,(struct sockaddr *)&dest_addr,sizeof(dest_addr));

			if(ser_sock_fd<0){
				perror("Error in connecting to remote server");
				write(client_sock_fd,not_found_message,strlen(not_found_message));
	    			continue;
			}

			
			write(proxy_sock_fd_for_dest,request,strlen(request));
			memset(file_bufer, '\0', BUFSIZE);
			
			int k = read(proxy_sock_fd_for_dest,file_bufer,BUFSIZE-1);
			

			while(k!=0){
							
							write(client_sock_fd,file_bufer,k);
							memset(file_bufer, '\0', BUFSIZE);
							k = read(proxy_sock_fd_for_dest,file_bufer,BUFSIZE-1);
							
						}

			close(proxy_sock_fd_for_dest);
			close(client_sock_fd);
		}else{


			int fd = open("./index.html",O_RDONLY); //file을 열어서 어디에 열어놨는지 저장한다.
			int size = lseek(fd,0,SEEK_END); //파일의 크기를 계산한다.
			lseek(fd,0,SEEK_SET); //파일을 다시 처음으로 되돌린다.
			
			bzero(file_bufer,BUFSIZE);
			sprintf(file_bufer, "%s%d%s%s", "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length: ", size, "\r\nConnection: Keep - Alive", "\r\n\r\n");
			//http response메세즈를 file_bufer에 쓰라는 뜻
			write(client_sock_fd,file_bufer,strlen(file_bufer));//클라 소켓에 파일버퍼에 있는 메세지를 전송
			
			bzero(file_bufer,BUFSIZE); //파일 버퍼를 초기화
			read(fd,file_bufer,BUFSIZE);	//fd에 저장되어있는 파일을 읽어서 최대 bufsize만큼만 파일버퍼에 읽어오라는 뜻
			write(client_sock_fd,file_bufer,strlen(file_bufer)); //클라이언트 소켓에 파일버퍼에 있는 내용을 그 길이만큼 쓰라는 뜻!
			bzero(file_bufer,BUFSIZE);
			close(client_sock_fd);
			}
		
	}	











}