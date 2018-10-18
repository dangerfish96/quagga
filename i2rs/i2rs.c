#include <sys/stat.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <netinet/in.h>
#include "../lib/zebra.h"
#include "../lib/log.h"

void error(const char *msg)
{
    perror(msg);
    exit(1);
}
void listenSocket(char * buffer, int newsockfd){
     int n;
	 char exitConst[5];
     strcpy(exitConst, "exit");
    n = read(newsockfd,buffer,255);
   if (n < 0) error("ERROR reading from socket");
   buffer[strlen(buffer)-2] = '\0';
   printf("Here is the message: (%s)\n",buffer);

   if(strncmp(buffer,exitConst,255)==0){
	  n = write(newsockfd,"Shutting down... \n",19);
	return;
   }
   n = write(newsockfd,"I got your message \n",19);
   if (n < 0) error("ERROR writing to socket");

}

//http://www.linuxhowtos.org/data/6/server.c
int createSocket(int portno)
{
     int sockfd, newsockfd;
     socklen_t clilen;
     char buffer[256];
     struct sockaddr_in serv_addr, cli_addr;
     sockfd = socket(AF_INET, SOCK_STREAM, 0);
     if (sockfd < 0)
        error("ERROR opening socket");
	 int enable = 1;
	 if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0)
		     error("setsockopt(SO_REUSEADDR) failed");
     bzero((char *) &serv_addr, sizeof(serv_addr));
     serv_addr.sin_family = AF_INET;
     serv_addr.sin_addr.s_addr = INADDR_ANY;
     serv_addr.sin_port = htons(portno);
     if (bind(sockfd, (struct sockaddr *) &serv_addr,
              sizeof(serv_addr)) < 0)
              error("ERROR on binding");
     listen(sockfd,5);
     clilen = sizeof(cli_addr);
     newsockfd = accept(sockfd,
                 (struct sockaddr *) &cli_addr,
                 &clilen);
     if (newsockfd < 0)
          error("ERROR on accept");
     bzero(buffer,256);

	 while(1){
	listenSocket(buffer,newsockfd);
	}
     close(newsockfd);
     close(sockfd);
     return 0;
}





/* Making connection to protocol daemon. */
static int
zebra_connect (int fd)
{
  int ret;
  int sock, len;
  struct sockaddr_un addr;
  struct stat s_stat;

  /* Stat socket to see if we have permission to access it. */
  ret = stat (ZEBRA_VTYSH_PATH, &s_stat);
  if (ret < 0 && errno != ENOENT)
    {
      fprintf  (stderr, "vtysh_connect(%s): stat = %s\n", 
		ZEBRA_VTYSH_PATH, safe_strerror(errno)); 
      exit(1);
    }
  
  if (ret >= 0)
    {
      if (! S_ISSOCK(s_stat.st_mode))
	{
	  fprintf (stderr, "vtysh_connect(%s): Not a socket\n",
		   ZEBRA_VTYSH_PATH);
	  exit (1);
	}
      
    }

  sock = socket (AF_UNIX, SOCK_STREAM, 0);
  if (sock < 0)
    {
#ifdef DEBUG
      fprintf(stderr, "vtysh_connect(%s): socket = %s\n", vclient->path,
	      safe_strerror(errno));
#endif /* DEBUG */
      return -1;
    }

  memset (&addr, 0, sizeof (struct sockaddr_un));
  addr.sun_family = AF_UNIX;
  strncpy (addr.sun_path, ZEBRA_VTYSH_PATH, strlen (ZEBRA_VTYSH_PATH));
#ifdef HAVE_STRUCT_SOCKADDR_UN_SUN_LEN
  len = addr.sun_len = SUN_LEN(&addr);
#else
  len = sizeof (addr.sun_family) + strlen (addr.sun_path);
#endif /* HAVE_STRUCT_SOCKADDR_UN_SUN_LEN */

  ret = connect (sock, (struct sockaddr *) &addr, len);
  if (ret < 0)
    {
#ifdef DEBUG
      fprintf(stderr, "vtysh_connect(%s): connect = %s\n", vclient->path,
	      safe_strerror(errno));
#endif /* DEBUG */
      close (sock);
      return -1;
    }
  fd = sock;

  return 0;
}



              
int main(int argc, char *argv[]) { 
	pid_t pid = 0;
    pid_t sid = 0;
    pid = fork();// fork a new child process

    if (pid < 0)
    {
        printf("fork failed!\n");
        exit(1);
    }

    if (pid > 0)// its the parent process
    {
       printf("pid of child process %d \n", pid);
        exit(0); //terminate the parent process succesfully
    }
	

    umask(0);//unmasking the file mode

    sid = setsid();//set new session
    if(sid < 0)
    {
        exit(1);
    }
	 if ((chdir("/")) < 0) { 
		 /* Log the failure */ 
		 exit(1); 
	 } 

    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);

  
	createSocket(10000);
    return (0);
}


