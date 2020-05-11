/*
	实例 HTTP/1.0 web 服务，使用get方法来响应 静态/动态内容
*/

#include"csapp.h"
#include<iostream>

//using namespace std;



// 函数声明

void doit(int fd);

void read_requesthdrs(rio_t *rp);

void parse_uri(char *uri, char *filename, char *cgicargs);

void serve_static(int fd, char *filename, int filesize);

void get_filetype(char *filename, char *filetype);

void serve_dynamic(int fd, char *filename, char *cgiargs);

void clienterror(int fd, char *cause, char *errnum, char *shortmsg, char *longmsg);



int main(int argc, char **argv){
	printf("this is the begin of the tiny web sever \n");	
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
		connfd = Accept(listenfd, (SA *)&clientaddr, (socklen_t *)&clientlen);
		doit(connfd);
		Close(connfd);
	}
	exit(0);
}



void doit(int fd)
{
	int is_static;
	struct stat sbuf;
	char buf[MAXLINE];
	char method[MAXLINE];
	char uri[MAXLINE];
	char version[MAXLINE];
	char filename[MAXLINE];
	char cgiagrs[MAXLINE];
	rio_t rio;
	
	// 读取请求和header

	Rio_readinitb(&rio, fd);	
	Rio_readlineb(&rio, buf, MAXLINE);
	sscanf(buf, "%s %s %s", method, uri , version);

	if(strcasecmp(method, "GET")){ // strcasecmp is a CASE-ignored compared function, if same, return 0
		clienterror(fd, method, "501", "Not Implemented", "Tiny does not implement this method");
		return ;	
	}
	
	read_requesthdrs(&rio);
	
	// 从GET请求中获取 URI 
	is_static = parse_uri(uri, filename, cgiagrs); //parse_uri function will implenment later

	if( stat(filename,&sbuf) < 0){   // stat() is function in sys/stat.h, which is used to obtain the status of file(filename) and purse into buf
		clienterror(fd, filename, "404", "Not found", "Tiny cannot find this file");
		return;	
	}
	//处理静态请求 static 
	if(is_static){
		if(!(S_ISREG(sbuf.st_mode)) || !(S_IRUSR & sbuf.st_mode) ){
			clienterror(fd,filename,"403", "Forbidden", "Tiny cannot read the file");
			return;		
		}
		server_static(fd, filename, sbuf.st_size);
	}
	//处理动态请求 dynamic
	else{
		if(!(S_ISREG(sbuf.st_mode)) || !(S_IXUSR & sbuf.st_mode)){
			clienterror(fd,filename,"403", "Forbidden" , "Tiny cannot run the CGI app");
			return;
		}
		serve_dynamic(fd, filename, cgiargs);	
	}
}



/*

	clienterror function throw a error message to the client 
*/
void clienterror(int fd, char *cause, char *errnum, char *shortmsg, char *longmsg){
	char buf[MAXLINE];
	char body[MAXBUF];
	
	// http body
	sprintf(body, "<html><title>Tiny Error</title>");
	sprintf(body, "%s<body bgcolor=""ffffff"">\r\n", body);
	sprintf(body, "%s%s: %s\r\n", body, errnum, shortmsg);
	sprintf(body, "%s<p>%s: %s\r\n", body, longmsg, cause);
	sprintf(body, "%s<hr><em>The Tiny Web server</em>\r\n", body);
	
	// http response
	sprintf(buf, "HTTP/1.0 %s %s\r\n", errnum, shortmsg);
	Rio_writen(fd, buf, strlen(buf));
	sprintf(buf, "Content-type: text/html\r\n");
	Rio_writen(fd, buf, strlen(buf));
	sprintf(buf, "Content-length: %d\r\n\r\n", (int)strlne(body));
	Rio_writen(fd, buf, strlen(buf));
	Rio_writen(fd, body, strlen(body));	
}


/*
	Reads and ignores request hearders
*/
void read_requesthdrs(rio_t *rp){
	char buf[MAXLINE];
	Rio_readlineb(rp, buf, MAXLINE);
	while(strcmp(buf, "\r\n")){
		Rio_readlineb(rp,buf,MAXLINE);
		printf("%s",buf);/
	}
	return;
}


/*
	
*/
int parse_uri(char *uri, char *filename, char *cgiargs){
	char *ptr;
	
	//C 库函数 char *strstr(const char *haystack, const char *needle) 在字符串 haystack 中查找第一次出现字符串 needle 的位置，不包含终止符 '\0'
	if(!strstr(uri, "cgi-bin")){ // static 
		strcpy(cgiargs, "");
		strcpy(filename,".");
		strcat(filename, uri);
		if(uri[strlen(uri)-1] == '/')
			strcat(filename, "home.html");
		return 1;
	}
	else{ // dynamic 
		ptr = index(uri, '?');
		if(ptr) {
			strcpy(cgiargs,ptr+1);
			*ptr = '\0';
		}
		else{
			strcpy(cgiargs, "");
		}
		strcpy(filename , ".");
		strcat(filename, uri);
		return 0;
	}
} 


/*

*/
void serve_static(int fd, char *filename, int filesize){
	int scrfd;
	char *srcp;
	char filetype[MAXLINE];
	char buf[MAXLINE];
	
	get_filetype(filename, filetype);
	sprintf(buf, "HTTP/1.0 200 OK\r\n");
	sprintf(buf, "%sServer: Tiny Web Server\r\n", buf);	
	sprintf(buf, "%sContent-length: %d\r\n", buf, filesze);
	sprintf(buf, "%sContent-style: %s\r\n\r\n",buf,filetype);
	Rio_writen(fd, buf, strlen(buf));

	// Send response body to client

	srcfd = Open(filename , O_RDONLY, 0);
	srcp = Mmap(0,filesize, PROT_READ, MAP_PRIVATE, srcfd, 0);
	Close(srcfd);
	Rio_writen(fd, srcp. filesize);
	Munmap(srcp, filesize);
}


void get_filetype(char *filename, char *filetype){
	if( strstr(filename, ".html") )
		strcpy(filename, "text/html");
	else if( strstr(filename, ".gif") )
		strcpy(filename, "image/gif");
	else if( strstr(filename, ".jpg") )
		strcpy(filename, "image/jpge");
	else strcpy(filename, "text/plain");
}


void serve_dynamic(int fd, char *filename, char *cgiargs){
	char buf[MAXLINE];
	char *emptylist[]={NULL};

	// return first part of http response
	sprintf(buf, "HTTP/1.0 200 OK\r\n");
	Rio_writen(fd, buf, strlen(buf));
	sprintf(buf, "Server: Tiny Web Server\r\n");
	Rio_writen(fd, buf, strlen(buf));
	
	if( Fork()==0 ){
		// real server would set all CGI vars here
		setenv("QUERY_STRING", cgiargs, 1);
		Dup2(fd, STDOUT_FILENO);
		Execve(filename, emptylist, environ);
	}
	Wait(NULL);
}







