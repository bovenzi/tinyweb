/*
	实例 HTTP/1.0 web 服务，使用get方法来响应 静态/动态内容
*/

#include"csapp.h"
#include<iostream>

using namespace std;



// 函数声明
void doit(int fd);

void read_requesthdrs(rio_t *rp);

void parse_uri(char *uri, char *filename, char *cgicargs);

void serve_static(int fd, char *filename, int filesize);

void get_filetype(char *filename, char *filetype);

void serve_dynamic(int fd, char *filename, char *cgiargs);

void clienterror(int fd, char *cause, char *errnum, char *shortmsg, char *longmsg);



int main(int argc, char **argv){
	cout << "this is the begin of the tiny web sever " << endl;	
	int listenfd, connfd, port, clientlen;
	struct sockaddr_in clientaddr;
	
	// check args

	if(argc != 2 ){
		fprintf(stderr, "usage: %s <port>\n", argv[0]);
		exit(1);
	}
	port = atoi(argv[1]);
	
	// open_listenfd : helper function that opens and returns a listening socket.
	listenfd = Open_listenfd(port);
	
	while(1){
		clientlen = sizeof(clientaddr);
		connfd = Accept(listenfd, (SA *)&clientaddr, &clientlen);
		doit(connfd);
		Close(connfd);
	}
	exit(0);
}
