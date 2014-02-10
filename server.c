#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>

#define PORT 		9999
#define DIRSIZE 	8192

main()
{
        char     dir[DIRSIZE];  /* used for incomming dir name, and
					outgoing data */
	int 	 sd, sd_current, cc, fromlen, tolen;
	int 	 addrlen;
	struct   sockaddr_in sin;
	struct   sockaddr_in pin;
	int 	 result;
	int      enable_keep_alive = 1;
 
	/* get an internet domain socket */
	if ((sd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		perror("socket");
		exit(1);
	}
	printf("sock sucess\n");

	/* complete the socket structure */
	memset(&sin, 0, sizeof(sin));
	sin.sin_family = AF_INET;
	sin.sin_addr.s_addr = INADDR_ANY;
	sin.sin_port = htons(PORT);

	/*
	result = setsockopt(sd, SOL_SOCKET, SO_KEEPALIVE, &enable_keep_alive, sizeof(int));
	 if (result < 0)
		           error("SO_KEEPALIVE");
			   */

	/* bind the socket to the port number */
	if (bind(sd, (struct sockaddr *) &sin, sizeof(sin)) == -1) {
		perror("bind");
		exit(1);
	}
	printf("bind sucess\n");

	/* show that we are willing to listen */
	if (listen(sd, 5) == -1) {
		perror("listen");
		exit(1);
	}
	printf("listen sucess\n");
	/* wait for a client to talk to us */
        addrlen = sizeof(pin); 
	if ((sd_current = accept(sd, (struct sockaddr *)  &pin, &addrlen)) == -1) {
		perror("accept");
		exit(1);
	}
	printf("accept sucess\n");
	sleep(10000);
	
/* if you want to see the ip address and port of the client, uncomment the 
    next two lines */

       /*
printf("Hi there, from  %s#\n",inet_ntoa(pin.sin_addr));
printf("Coming from port %d\n",ntohs(pin.sin_port));
        */

	/* get a message from the client */
	if (recv(sd_current, dir, sizeof(dir), 0) == -1) {
		perror("recv");
		exit(1);
	}

        /* get the directory contents */
   //      read_dir(dir);
    
      /* strcat (dir," DUDE");
       */
	/* acknowledge the message, reply w/ the file names */
	/*
	if (send(sd_current, dir, strlen(dir), 0) == -1) {
		perror("send");
		exit(1);
	}

	*/
        /* close up both sockets */
	close(sd_current); close(sd);
        
        /* give client a chance to properly shutdown */
        sleep(1);
}

