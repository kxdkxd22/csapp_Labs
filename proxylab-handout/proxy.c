#include <stdio.h>
#include "csapp.h"
#include "cache.h"
/* Recommended max cache and object sizes */
#define MAX_CACHE_SIZE 1049000
#define MAX_OBJECT_SIZE 102400
/*#define LRU_MAGIC_NUMBER 9999
#define CACHE_OBJS_COUNT 10
*/
/* You won't lose style points for including this long line in your code */
static const char *user_agent_hdr = "User-Agent: Mozilla/5.0 (X11; Linux x86_64; rv:10.0.3) Gecko/20120305 Firefox/10.0.3\r\n";
static const char *conn_hdr = "Connection: close\r\n";
static const char *prox_hdr = "Proxy-Connection: close\r\n";
static const char *host_hdr_format = "Host: %s\r\n";
static const char *requestlint_hdr_format = "GET %s HTTP/1.0\r\n";
static const char *endof_hdr = "\r\n";

static const char *connection_key = "Connection";
static const char *user_agent_key = "User-Agent";
static const char *proxy_connection_key = "Proxy-Connection";
static const char *host_key = "Host";

void doit(int connfd);
int parse_uri(char *uri,char *hostname,char *path,char *port);
void build_http_header(char *http_header,char *hostname,char *path,char *port,rio_t *rio);
void *thread(void *vargp);


int main(int argc, char **argv)
{
	int listenfd;int *connfd;
	socklen_t clientlen;
	struct sockaddr_storage clientaddr;	
	char hostname[MAXLINE],port[MAXLINE];
	pthread_t tid;
	
	

	if(argc !=2){
		fprintf(stderr,"usage: %s <port\n>",argv[0]);
		exit(1);
	}	
	Signal(SIGPIPE,SIG_IGN);
	/*cache_init();*/
	init_cache();
	listenfd = Open_listenfd(argv[1]);	

	while(1){
		printf("liteniing...\n");
		clientlen = sizeof(clientaddr);
		connfd = Malloc(sizeof(int));
		*connfd = Accept(listenfd,(SA*)&clientaddr,&clientlen);
		Getnameinfo((SA*)&clientaddr,clientlen,hostname,MAXLINE,port,MAXLINE,0);
		printf("Accepted connection from (%s %s).\n",hostname,port);
		Pthread_create(&tid,NULL,thread,(void *)connfd);
	}

	Close(listenfd);    	
	free_cache();
	return 0;
}

void *thread(void *vargp){
	int connfd = *((int *)vargp);
	Pthread_detach(pthread_self());
	Free(vargp);
	printf("hello thread\n");
	doit(connfd);
	Close(connfd);
	return NULL;
}


void doit(int connfd){
	rio_t server_rio,client_rio;
	char buf[MAXLINE],method[MAXLINE],uri[MAXLINE],version[MAXLINE];
	char hostname[MAXLINE];
	char path[MAXLINE];
	char port[MAXLINE];
	
	char http_header[MAXLINE];
	int servfd;	
	int res;
	int n;
	
	Rio_readinitb(&client_rio,connfd);
	Rio_readlineb(&client_rio,buf,MAXLINE);
	printf("connfd %d\n",connfd);	
	sscanf(buf,"%s %s %s",method,uri,version);
	
	if(strcasecmp(method,"GET")){
		/*printf("501 Not implemented Tiny does not implement this method\n");*/
		return;
	}
	
	int ret = read_cache(uri,connfd);
	if(ret==1){
		return;
	} 
		
	

	memset(hostname,'\0',MAXLINE);
	memset(port,'\0',MAXLINE);
	memset(path,'\0',MAXLINE);
	if((res=parse_uri(uri,hostname,path,port))==-1){
		fprintf(stderr,"not http protocol\n");
		return;
	}else if(res == 0){
		fprintf(stderr,"not a abslute request path\n");
		return;
	}

	build_http_header(http_header,hostname,path,port,&client_rio);
	printf("hostname %s\n",hostname);
	servfd=open_clientfd(hostname,port);
	printf("server fd %d\n",servfd);

	if(servfd<0){
		printf("connection server failed\n");
		return;
	}
	Rio_readinitb(&server_rio,servfd);
	
	Rio_writen(servfd,http_header,strlen(http_header));
	
	char cachebuf[MAX_OBJECT_SIZE];
	int sizebuf = 0;
	
	while((n=Rio_readlineb(&server_rio,buf,MAXLINE))!=0){
		printf("proxy recieved from end server %d bytes.\n",n);
		
		if(sizebuf<=MAX_OBJECT_SIZE){
			memcpy(cachebuf+sizebuf,buf,n);
			sizebuf+=n;
		} 
			
		Rio_writen(connfd,buf,n);
	}

	Close(servfd);

	if(sizebuf<MAX_OBJECT_SIZE){
		write_cache(uri,cachebuf,sizebuf);
	}
	
}

int parse_uri(char *uri,char *hostname,char *path,char *port){
	/*if(uri[0] == '/')
		return 0;	
	char *prefix = "http://";
	int prelen = strlen(prefix);

	if(strncmp(uri,prefix,prelen)!=0)
		return -1;
	*/
	
	char *start,*end;
	start = strstr(uri,"//");
	start = start!=NULL?start+2:uri;
	end = start;

	while(*end!=':'&&*end!='/'&&*end!='\0'){
		end++;
	}
	
	strncpy(hostname,start,end-start);
	
	if((*end) == ':'){
		++end;
		start=end;
		while(*end != '/'&&*end!='\0')
			end++;
		strncpy(port,start,end-start);
	}else{
		strncpy(port,"80",2);
	}
	
	strcpy(path,end);

	return 1;
}

void build_http_header(char *http_header,char *hostname,char *path,char *port,rio_t *rio){
	char buf[MAXLINE],request_hdr[MAXLINE],other_hdr[MAXLINE],host_hdr[MAXLINE];
	
	sprintf(request_hdr,requestlint_hdr_format,path);
	
	while(Rio_readlineb(rio,buf,MAXLINE)>0){
		if(strcmp(buf,endof_hdr)==0) break;
		
		if(!strncasecmp(buf,host_key,strlen(host_key))){
			strcpy(host_hdr,buf);
			continue;
		}
		
		if(!strncasecmp(buf,connection_key,strlen(connection_key))&&!strncasecmp(buf,proxy_connection_key,strlen(proxy_connection_key))&&!strncasecmp(buf,user_agent_key,strlen(user_agent_key))){
			strcat(other_hdr,buf);
		}

	}
	
	if(strlen(host_hdr)==0){
		sprintf(host_hdr,host_hdr_format,hostname);
	}	

	sprintf(http_header,"%s%s%s%s%s%s%s",request_hdr,host_hdr,conn_hdr,prox_hdr,user_agent_hdr,other_hdr,endof_hdr);

	return;
}























