#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netinet/ip6.h>

#define IPV6_RTHDR_TYPE_1 1
#define IPV6_RTHDR_TYPE_2 2

socklen_t
inet6_rth_space2 (int type, int segments)
{
  switch (type)
    {
    case IPV6_RTHDR_TYPE_2:
      if (segments < 0 || segments > 127)
	return 0;

      return sizeof (struct ip6_rthdr0) + segments * sizeof (struct in6_addr);
    }

  return 0;
}


/* RFC 3542, 7.2

   This function initializes the buffer pointed to by BP to contain a
   Routing header of the specified type and sets ip6r_len based on the
   segments parameter.  */
void *
inet6_rth_init2 (void *bp, socklen_t bp_len, int type, int segments)
{
  struct ip6_rthdr *rthdr = (struct ip6_rthdr *) bp;

  switch (type)
    {
    case IPV6_RTHDR_TYPE_2:
      /* Make sure the parameters are valid and the buffer is large enough.  */
      if (segments < 0 || segments > 127)
	break;

      socklen_t len = (sizeof (struct ip6_rthdr0)
		       + segments * sizeof (struct in6_addr));
      if (len > bp_len)
	break;

      /* Some implementations seem to initialize the whole memory area.  */
      memset (bp, '\0', len);

      /* Length in units of 8 octets.  */
      rthdr->ip6r_len = segments * sizeof (struct in6_addr) / 8;
      rthdr->ip6r_type = type;
      return bp;
    }

  return NULL;
}


/* RFC 3542, 7.3

   This function adds the IPv6 address pointed to by addr to the end of
   the Routing header being constructed.  */
int 
inet6_rth_add2 (void * bp,const struct in6_addr * addr)
{
  struct ip6_rthdr *rthdr = (struct ip6_rthdr *) bp;

  switch (rthdr->ip6r_type)
    {
      struct ip6_rthdr0 *rthdr0;
    case IPV6_RTHDR_TYPE_2:
      rthdr0 = (struct ip6_rthdr0 *) rthdr;
      if (rthdr0->ip6r0_len * 8 / sizeof (struct in6_addr)
	  - rthdr0->ip6r0_segleft < 1)
        return -1;

      memcpy (&rthdr0->ip6r0_addr[rthdr0->ip6r0_segleft++],
	      addr, sizeof (struct in6_addr));

      return 0;
    }

  return -1;
}


int main(void)
{
    int sock = socket(PF_INET6, SOCK_STREAM, IPPROTO_TCP);
    struct sockaddr_in6 addr;

	
    void  *extptr;
    socklen_t   extlen;
    struct msghdr  msg;
    struct cmsghdr  *cmsgptr;
    int   cmsglen;
    struct sockaddr_in6  server;
    memset((char *)&server, 0, sizeof(server));
    inet_pton(AF_INET6, "fda0:c365:c175:7e1::23", &server.sin6_addr);
    //server.sin6_family = AF_INET6;
    //server.sin6_port = htons(22);

    extlen = inet6_rth_space2(IPV6_RTHDR_TYPE_2, 0);
    cmsglen = CMSG_SPACE(extlen);
    cmsgptr = malloc(cmsglen);
    cmsgptr->cmsg_len = CMSG_LEN(extlen);
    cmsgptr->cmsg_level = IPPROTO_IPV6;
    cmsgptr->cmsg_type = IPV6_RTHDR;

    extptr = CMSG_DATA(cmsgptr);
    printf("inet6_rth_init\n");
    extptr = inet6_rth_init2(extptr, extlen, IPV6_RTHDR_TYPE_2, 1);

    printf("inet6_rth_add\n");
    inet6_rth_add2(extptr, &server.sin6_addr);


    printf("setsockopt extlen:%d %x\n", extlen, extlen);
    //if (0) {
	if (setsockopt(sock, IPPROTO_IPV6, IPV6_RTHDR, extptr, extlen) == -1) {
        perror("setsockopt");
        exit(1);
    }

    addr.sin6_family = AF_INET6;
    printf("inet_pton\n");
    //inet_pton(AF_INET6, "fda0:c365:c175:7e1::16", &addr.sin6_addr);
    inet_pton(AF_INET6, "2620:52:0:42c0:5eff:35ff:fe0a:29f8", &addr.sin6_addr);
    addr.sin6_port = htons(7378);
    printf("connect\n");
    if (connect(sock, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
        perror("connect");
        exit(1);
    }

    return 0;
}

