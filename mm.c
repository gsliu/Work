#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <strings.h>
 
int sock;
 
int main(int argc, char *argv[])
{
  int addr_len;
  struct sockaddr_in sin,sin_recv;
  struct ip_mreq multi;
  char mes[255];
 
  bzero(&sin,sizeof(sin));
  multi.imr_multiaddr.s_addr=inet_addr("230.4.4.1");
  multi.imr_interface.s_addr=inet_addr("192.168.122.7");  // Fill in with your local IP
  sin.sin_family = AF_INET;
  // With this line uncommented, it works
  //sin.sin_addr.s_addr=INADDR_ANY;
  // With this line uncommented, it doesn't work
  sin.sin_addr.s_addr=inet_addr("192.168.122.7");  // Fill in with your local IP
  sin.sin_port = htons(1044);
  if ((sock=socket(AF_INET,SOCK_DGRAM,0))==-1) {
    perror("Error creating socket");
    exit(1);
  }
  if (bind(sock,(struct sockaddr *)&sin,sizeof(sin))==-1) {
    perror("Error binding socket");
    close(sock);
    exit(1);
  }
  if (setsockopt(sock,IPPROTO_IP,IP_ADD_MEMBERSHIP,(char *)&multi,sizeof(multi))== -1) {
    perror("Error joining multicast group");
    close(sock);
    exit(1);
  }
  while (1) {
    bzero(mes,sizeof(mes));
    bzero(&sin_recv,sizeof(sin_recv));
    addr_len=sizeof(sin_recv);
    printf("Waiting for packet...\n");
    if (recvfrom(sock,mes,sizeof(mes),0,(struct sockaddr *)&sin_recv,&addr_len)==-1) {
      perror("error recving socket");
      close(sock);
      exit(1);
    }
    printf("Got packet\n");
    printf("Origin: %s port %d\n",inet_ntoa(sin_recv.sin_addr),sin_recv.sin_port);
    printf("Message: %s\n",mes);
  }
  return 0;
}
