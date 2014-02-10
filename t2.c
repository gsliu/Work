/*  Copyright (C) 2011-2013  P.D. Buchan (pdbuchan@yahoo.com)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

// Send an IPv4 TCP packet via raw socket at the link layer (ethernet frame).
// Need to have destination MAC address.
// Values set for SYN packet with two IP options and two TCP options:
// IP: timestamp, and security option
// TCP: set maximum segment size, and provide TCP timestamp.
//
// Unless you're on a military or high security network,
// you'll likely receieve an ICMP packet back complaining about
// a bad parameter in the IP header because a hop didn't know what
// to do with a security option.

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>           // close()
#include <string.h>           // strcpy, memset(), and memcpy()

#include <netdb.h>            // struct addrinfo
#include <sys/types.h>        // needed for socket(), uint8_t, uint16_t, uint32_t
#include <sys/socket.h>       // needed for socket()
#include <netinet/in.h>       // IPPROTO_TCP, INET_ADDRSTRLEN
#include <netinet/ip.h>       // struct ip and IP_MAXPACKET (which is 65535)
#define __FAVOR_BSD           // Use BSD format of tcp header
#include <netinet/tcp.h>      // struct tcphdr
#include <arpa/inet.h>        // inet_pton() and inet_ntop()
#include <sys/ioctl.h>        // macro ioctl is defined
#include <bits/ioctls.h>      // defines values for argument "request" of ioctl.
#include <net/if.h>           // struct ifreq
#include <linux/if_ether.h>   // ETH_P_IP = 0x0800, ETH_P_IPV6 = 0x86DD
#include <linux/if_packet.h>  // struct sockaddr_ll (see man 7 packet)
#include <net/ethernet.h>

#include <errno.h>            // errno, perror()

// Define some constants.
#define ETH_HDRLEN 14  // Ethernet header length
#define IP4_HDRLEN 20  // IPv4 header length
#define TCP_HDRLEN 20  // TCP header length, excludes tcp_options data

// Function prototypes
uint16_t checksum (uint16_t *, int);
uint16_t ip4_checksum (struct ip, uint8_t *, int);
uint16_t tcp4_checksum (struct ip, struct tcphdr, uint8_t *, int);
char *allocate_strmem (int);
char **allocate_strmemp (int);
uint8_t *allocate_ustrmem (int);
uint8_t **allocate_ustrmemp (int);
int *allocate_intmem (int);

int
main (int argc, char **argv)
{
  int i, c, status, frame_length, sd, bytes, *ip_flags, *tcp_flags;
  int ip_nopt, tcp_nopt, *ip_opt_len, *tcp_opt_len, ip_buf_len, tcp_buf_len;
  char *interface, *target, *src_ip, *dst_ip;
  struct ip iphdr;
  struct tcphdr tcphdr;
  uint8_t *src_mac, *dst_mac, *ether_frame;
  uint8_t **ip_options, **tcp_options, *ip_opt_buf, *tcp_opt_buf;
  struct addrinfo hints, *res;
  struct sockaddr_in *ipv4;
  struct sockaddr_ll device;
  struct ifreq ifr;
  void *tmp;

  // Allocate memory for various arrays.
  src_mac = allocate_ustrmem (6);
  dst_mac = allocate_ustrmem (6);
  ether_frame = allocate_ustrmem (IP_MAXPACKET);
  interface = allocate_strmem (40);
  target = allocate_strmem (40);
  src_ip = allocate_strmem (INET_ADDRSTRLEN);
  dst_ip = allocate_strmem (INET_ADDRSTRLEN);
  ip_flags = allocate_intmem (4);
  tcp_flags = allocate_intmem (8);
  ip_opt_len = allocate_intmem (10);
  ip_options = allocate_ustrmemp (10);
  for (i=0; i<10; i++) {
    ip_options[i] = allocate_ustrmem (40);
  }
  tcp_opt_len = allocate_intmem (10);
  tcp_options = allocate_ustrmemp (10);
  for (i=0; i<10; i++) {
    tcp_options[i] = allocate_ustrmem (40);
  }
  ip_opt_buf = allocate_ustrmem (40);
  tcp_opt_buf = allocate_ustrmem (40);

  // Interface to send packet through.
  strcpy (interface, "eth0");

  // Submit request for a socket descriptor to look up interface.
  if ((sd = socket (PF_PACKET, SOCK_RAW, htons (ETH_P_ALL))) < 0) {
    perror ("socket() failed to get socket descriptor for using ioctl() ");
    exit (EXIT_FAILURE);
  }

  // Use ioctl() to look up interface name and get its MAC address.
  memset (&ifr, 0, sizeof (ifr));
  snprintf (ifr.ifr_name, sizeof (ifr.ifr_name), "%s", interface);
  if (ioctl (sd, SIOCGIFHWADDR, &ifr) < 0) {
    perror ("ioctl() failed to get source MAC address ");
    return (EXIT_FAILURE);
  }
  close (sd);

  // Copy source MAC address.
  memcpy (src_mac, ifr.ifr_hwaddr.sa_data, 6);

  // Report source MAC address to stdout.
  printf ("MAC address for interface %s is ", interface);
  for (i=0; i<5; i++) {
    printf ("%02x:", src_mac[i]);
  }
  printf ("%02x\n", src_mac[5]);

  // Find interface index from interface name and store index in
  // struct sockaddr_ll device, which will be used as an argument of sendto().
  if ((device.sll_ifindex = if_nametoindex (interface)) == 0) {
    perror ("if_nametoindex() failed to obtain interface index ");
    exit (EXIT_FAILURE);
  }
  printf ("Index for interface %s is %i\n", interface, device.sll_ifindex);

  // Set destination MAC address: you need to fill these out
  // 52:54:00:FE:4A:E5 
  dst_mac[0] = 0x52;
  dst_mac[1] = 0x54;
  dst_mac[2] = 0x00;
  dst_mac[3] = 0xFE;
  dst_mac[4] = 0x4A;
  dst_mac[5] = 0xE5;

  // Source IPv4 address: you need to fill this out
  strcpy (src_ip, "10.66.218.36");

  // Destination URL or IPv4 address: you need to fill this out
  strcpy (target, "10.66.192.96");

  // Fill out hints for getaddrinfo().
  memset (&hints, 0, sizeof (struct addrinfo));
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = hints.ai_flags | AI_CANONNAME;

  // Resolve target using getaddrinfo().
  if ((status = getaddrinfo (target, NULL, &hints, &res)) != 0) {
    fprintf (stderr, "getaddrinfo() failed: %s\n", gai_strerror (status));
    exit (EXIT_FAILURE);
  }
  ipv4 = (struct sockaddr_in *) res->ai_addr;
  tmp = &(ipv4->sin_addr);
  if (inet_ntop (AF_INET, tmp, dst_ip, INET_ADDRSTRLEN) == NULL) {
    status = errno;
    fprintf (stderr, "inet_ntop() failed.\nError message: %s", strerror (status));
    exit (EXIT_FAILURE);
  }
  freeaddrinfo (res);

  // Fill out sockaddr_ll.
  device.sll_family = AF_PACKET;
  memcpy (device.sll_addr, src_mac, 6);
  device.sll_halen = htons (6);

  // Number of IP options
  ip_nopt = 2;

  // First IP Option - Timestamp 
  ip_opt_len[0] = 0;
  ip_options[0][0] = 68u; ip_opt_len[0]++;  // Timestamp option
  ip_options[0][1] = 8u; ip_opt_len[0]++;  // Option length (bytes) - 40 is maximum
  ip_options[0][2] = 65u; ip_opt_len[0]++;  // Pointer
  ip_options[0][3] = 0u; ip_opt_len[0]++;  // Overflow and Flag
  ip_options[0][4] = 0u; ip_opt_len[0]++;  // Time in milliseconds (right-justified) - random here
  ip_options[0][5] = 0u; ip_opt_len[0]++;  //  Time cont'd
  ip_options[0][6] = 67u; ip_opt_len[0]++;  // Time cont'd
  ip_options[0][7] = 78u; ip_opt_len[0]++;  // Time cont'd

  // Second IP Option - We'll use military security classification for fun
  ip_opt_len[1] = 0;
  ip_options[1][0] = (1 << 7) | 2u; ip_opt_len[1]++;  // Option data is military security classification of datagrams
  ip_options[1][1] = 11u; ip_opt_len[1]++;  // Option length (bytes)
  ip_options[1][2] = 107u; ip_opt_len[1]++;  // Top Secret
  ip_options[1][3] = 197u; ip_opt_len[1]++;  // Top Secret
  ip_options[1][4] = 0u; ip_opt_len[1]++;  // Compartments: Not compartmented
  ip_options[1][5] = 0u; ip_opt_len[1]++;  // Compartments: Not compartmented
  ip_options[1][6] = 67u; ip_opt_len[1]++;  // Handling Restrictions: Critical Nuclear Weapon Design Information (CN) 
  ip_options[1][7] = 78u; ip_opt_len[1]++;  // Handling Restrictions: Critical Nuclear Weapon Design Information (CN)
  ip_options[1][8] = 65; ip_opt_len[1]++;
  ip_options[1][9] = 66; ip_opt_len[1]++;
  ip_options[1][10] = 67; ip_opt_len[1]++;

  // Copy all IP options into single options buffer.
  ip_buf_len = 0;
  c = 0;  // index to opt_buffer
  for (i=0; i<ip_nopt; i++) {
    memcpy (ip_opt_buf + c, ip_options[i], ip_opt_len[i]);
    c += ip_opt_len[i];
    ip_buf_len += ip_opt_len[i];
  }

  // Pad to the next 4-byte boundary.
  while ((ip_buf_len%4) != 0) {
    ip_opt_buf[ip_buf_len] = 0;
    ip_buf_len++;
  }

  // Number of TCP options
  tcp_nopt = 2;

  // First TCP option - Maximum segment size
  tcp_opt_len[0] = 0;
  tcp_options[0][0] = 2u; tcp_opt_len[0]++;  // Option kind 2 = maximum segment size
  tcp_options[0][1] = 4u; tcp_opt_len[0]++;  // This option kind is 4 bytes long
  tcp_options[0][2] = 0x1u; tcp_opt_len[0]++;  // Set maximum segment size to 0x100 = 256
  tcp_options[0][3] = 0x0u; tcp_opt_len[0]++;

  // Second TCP option - Timestamp option
  tcp_opt_len[1] = 0;
  tcp_options[1][0] = 8u; tcp_opt_len[1]++;  // Option kind 8 = Timestamp option (TSOPT)
  tcp_options[1][1] = 10u; tcp_opt_len[1]++;  // This option is 10 bytes long
  tcp_options[1][2] = 0x2u; tcp_opt_len[1]++;  // Set the sender's timestamp (TSval) (4 bytes) (need SYN set to be valid)
  tcp_options[1][3] = 0x3u; tcp_opt_len[1]++;
  tcp_options[1][4] = 0x4u; tcp_opt_len[1]++;
  tcp_options[1][5] = 0x5u; tcp_opt_len[1]++;
  tcp_options[1][6] = 0x6u; tcp_opt_len[1]++;  // Set the echo timestamp (TSecr) (4 bytes) (need ACK set to be valid)
  tcp_options[1][7] = 0x7u; tcp_opt_len[1]++;
  tcp_options[1][8] = 0x8u; tcp_opt_len[1]++;
  tcp_options[1][9] = 0x9u; tcp_opt_len[1]++;

  // Copy all options into single options buffer.
  tcp_buf_len = 0;
  c = 0;  // index to opt_buffer
  for (i=0; i<tcp_nopt; i++) {
    memcpy (tcp_opt_buf + c, tcp_options[i], tcp_opt_len[i]);
    c += tcp_opt_len[i];
    tcp_buf_len += tcp_opt_len[i];
  }

  // Pad to the next 4-byte boundary.
  while ((tcp_buf_len%4) != 0) {
    tcp_opt_buf[tcp_buf_len] = 0;
    tcp_buf_len++;
  }

  // IPv4 header

  // IPv4 header length (4 bits): Number of 32-bit words in header = 5
  iphdr.ip_hl = (IP4_HDRLEN + ip_buf_len) / sizeof (uint32_t);

  // Internet Protocol version (4 bits): IPv4
  iphdr.ip_v = 4;

  // Type of service (8 bits)
  iphdr.ip_tos = 0;

  // Total length of datagram (16 bits): IP header + IP options + TCP header + TCP options
  iphdr.ip_len = htons (IP4_HDRLEN + ip_buf_len + TCP_HDRLEN + tcp_buf_len);

  // ID sequence number (16 bits): unused, since single datagram
  iphdr.ip_id = htons (0);

  // Flags, and Fragmentation offset (3, 13 bits): 0 since single datagram

  // Zero (1 bit)
  ip_flags[0] = 0;

  // Do not fragment flag (1 bit)
  ip_flags[1] = 0;

  // More fragments following flag (1 bit)
  ip_flags[2] = 0;

  // Fragmentation offset (13 bits)
  ip_flags[3] = 0;

  iphdr.ip_off = htons ((ip_flags[0] << 15)
                      + (ip_flags[1] << 14)
                      + (ip_flags[2] << 13)
                      +  ip_flags[3]);

  // Time-to-Live (8 bits): default to maximum value
  iphdr.ip_ttl = 255;

  // Transport layer protocol (8 bits): 6 for TCP
  iphdr.ip_p = IPPROTO_TCP;

  // Source IPv4 address (32 bits)
  if ((status = inet_pton (AF_INET, src_ip, &(iphdr.ip_src))) != 1) {
    fprintf (stderr, "inet_pton() failed.\nError message: %s", strerror (status));
    exit (EXIT_FAILURE);
  }

  // Destination IPv4 address (32 bits)
  if ((status = inet_pton (AF_INET, dst_ip, &(iphdr.ip_dst))) != 1) {
    fprintf (stderr, "inet_pton() failed.\nError message: %s", strerror (status));
    exit (EXIT_FAILURE);
  }

  // IPv4 header checksum (16 bits): set to 0 when calculating checksum
  iphdr.ip_sum = 0;
  iphdr.ip_sum = ip4_checksum (iphdr, ip_opt_buf, ip_buf_len);

  // TCP header

  // Source port number (16 bits)
  tcphdr.th_sport = htons (80);

  // Destination port number (16 bits)
  tcphdr.th_dport = htons (80);

  // Sequence number (32 bits)
  tcphdr.th_seq = htonl (0);

  // Acknowledgement number (32 bits): 0 in first packet of SYN/ACK process
  tcphdr.th_ack = htonl (0);

  // Reserved (4 bits): should be 0
  tcphdr.th_x2 = 0;

  // Data offset (4 bits): size of TCP header + length of options, in 32-bit words
  tcphdr.th_off = (TCP_HDRLEN  + tcp_buf_len) / 4;

  // Flags (8 bits)

  // FIN flag (1 bit)
  tcp_flags[0] = 0;

  // SYN flag (1 bit): set to 1
  tcp_flags[1] = 1;

  // RST flag (1 bit)
  tcp_flags[2] = 0;

  // PSH flag (1 bit)
  tcp_flags[3] = 0;

  // ACK flag (1 bit)
  tcp_flags[4] = 0;

  // URG flag (1 bit)
  tcp_flags[5] = 0;

  // ECE flag (1 bit)
  tcp_flags[6] = 0;

  // CWR flag (1 bit)
  tcp_flags[7] = 0;

  tcphdr.th_flags = 0;
  for (i=0; i<8; i++) {
    tcphdr.th_flags += (tcp_flags[i] << i);
  }

  // Window size (16 bits)
  tcphdr.th_win = htons (65535);

  // Urgent pointer (16 bits): 0 (only valid if URG flag is set)
  tcphdr.th_urp = htons (0);

  // TCP checksum (16 bits)
  tcphdr.th_sum = tcp4_checksum (iphdr, tcphdr, tcp_opt_buf, tcp_buf_len);

  // Fill out ethernet frame header.

  // Ethernet frame length = ethernet header (MAC + MAC + ethernet type) + ethernet data (IP header + IP options + TCP header + TCP options)
  frame_length = 6 + 6 + 2 + IP4_HDRLEN + ip_buf_len + TCP_HDRLEN + tcp_buf_len;

  // Destination and Source MAC addresses
  memcpy (ether_frame, dst_mac, 6);
  memcpy (ether_frame + 6, src_mac, 6);

  // Next is ethernet type code (ETH_P_IP for IPv4).
  // http://www.iana.org/assignments/ethernet-numbers
  ether_frame[12] = ETH_P_IP / 256;
  ether_frame[13] = ETH_P_IP % 256;

  // Next is ethernet frame data (IPv4 header + TCP header).

  // IPv4 header
  memcpy (ether_frame + ETH_HDRLEN, &iphdr, IP4_HDRLEN);

  // IP options
  memcpy (ether_frame + ETH_HDRLEN + IP4_HDRLEN, ip_opt_buf, ip_buf_len);

  // TCP header
  memcpy (ether_frame + ETH_HDRLEN + IP4_HDRLEN + ip_buf_len, &tcphdr, TCP_HDRLEN);

  // TCP options
  memcpy (ether_frame + ETH_HDRLEN + IP4_HDRLEN + ip_buf_len + TCP_HDRLEN, tcp_opt_buf, tcp_buf_len);

  // Submit request for a raw socket descriptor.
  if ((sd = socket (PF_PACKET, SOCK_RAW, htons (ETH_P_ALL))) < 0) {
    perror ("socket() failed ");
    exit (EXIT_FAILURE);
  }

  // Send ethernet frame to socket.
  if ((bytes = sendto (sd, ether_frame, frame_length, 0, (struct sockaddr *) &device, sizeof (device))) <= 0) {
    perror ("sendto() failed");
    exit (EXIT_FAILURE);
  }

  // Close socket descriptor.
  close (sd);

  // Free allocated memory.
  free (src_mac);
  free (dst_mac);
  free (ether_frame);
  free (interface);
  free (target);
  free (src_ip);
  free (dst_ip);
  free (ip_flags);
  free (tcp_flags);
  free (ip_opt_len);
  free (tcp_opt_len);
  for (i=0; i<10; i++) {
    free (ip_options[i]);
  }
  free (ip_options);
  for (i=0; i<10; i++) {
    free (tcp_options[i]);
  }
  free (tcp_options);
  free (ip_opt_buf);
  free (tcp_opt_buf);

  return (EXIT_SUCCESS);
}

// Checksum function
uint16_t
checksum (uint16_t *addr, int len)
{
  int nleft = len;
  int sum = 0;
  uint16_t *w = addr;
  uint16_t answer = 0;

  while (nleft > 1) {
    sum += *w++;
    nleft -= sizeof (uint16_t);
  }

  if (nleft == 1) {
    *(uint8_t *) (&answer) = *(uint8_t *) w;
    sum += answer;
  }

  sum = (sum >> 16) + (sum & 0xFFFF);
  sum += (sum >> 16);
  answer = ~sum;
  return (answer);
}

// Build a buffer containing IPv4 header and IP options and call checksum function.
uint16_t
ip4_checksum (struct ip iphdr, uint8_t *options, int opt_len)
{
  char buf[IP_MAXPACKET];
  char *ptr;
  int chksumlen = 0;

  ptr = &buf[0];  // ptr points to beginning of buffer buf

  // Copy IPv4 header into buf (20 bytes)
  memcpy (ptr, &iphdr, IP4_HDRLEN);
  ptr += IP4_HDRLEN;
  chksumlen += IP4_HDRLEN;

  // Copy IP options into buf (variable length)
  memcpy (ptr, options, opt_len);
  ptr += opt_len;
  chksumlen += opt_len;

  return checksum ((uint16_t *) buf, chksumlen);
}

// Build IPv4 TCP pseudo-header and call checksum function.
uint16_t
tcp4_checksum (struct ip iphdr, struct tcphdr tcphdr, uint8_t *tcp_options, int tcp_opt_len)
{
  uint16_t svalue;
  char buf[IP_MAXPACKET], cvalue;
  char *ptr;
  int chksumlen = 0;

  ptr = &buf[0];  // ptr points to beginning of buffer buf

  // Copy source IP address into buf (32 bits)
  memcpy (ptr, &iphdr.ip_src.s_addr, sizeof (iphdr.ip_src.s_addr));
  ptr += sizeof (iphdr.ip_src.s_addr);
  chksumlen += sizeof (iphdr.ip_src.s_addr);

  // Copy destination IP address into buf (32 bits)
  memcpy (ptr, &iphdr.ip_dst.s_addr, sizeof (iphdr.ip_dst.s_addr));
  ptr += sizeof (iphdr.ip_dst.s_addr);
  chksumlen += sizeof (iphdr.ip_dst.s_addr);

  // Copy zero field to buf (8 bits)
  *ptr = 0; ptr++;
  chksumlen += 1;

  // Copy transport layer protocol to buf (8 bits)
  memcpy (ptr, &iphdr.ip_p, sizeof (iphdr.ip_p));
  ptr += sizeof (iphdr.ip_p);
  chksumlen += sizeof (iphdr.ip_p);

  // Copy TCP length to buf (16 bits)
  svalue = htons (sizeof (tcphdr) + tcp_opt_len);
  memcpy (ptr, &svalue, sizeof (svalue));
  ptr += sizeof (svalue);
  chksumlen += sizeof (svalue);

  // Copy TCP source port to buf (16 bits)
  memcpy (ptr, &tcphdr.th_sport, sizeof (tcphdr.th_sport));
  ptr += sizeof (tcphdr.th_sport);
  chksumlen += sizeof (tcphdr.th_sport);

  // Copy TCP destination port to buf (16 bits)
  memcpy (ptr, &tcphdr.th_dport, sizeof (tcphdr.th_dport));
  ptr += sizeof (tcphdr.th_dport);
  chksumlen += sizeof (tcphdr.th_dport);

  // Copy sequence number to buf (32 bits)
  memcpy (ptr, &tcphdr.th_seq, sizeof (tcphdr.th_seq));
  ptr += sizeof (tcphdr.th_seq);
  chksumlen += sizeof (tcphdr.th_seq);

  // Copy acknowledgement number to buf (32 bits)
  memcpy (ptr, &tcphdr.th_ack, sizeof (tcphdr.th_ack));
  ptr += sizeof (tcphdr.th_ack);
  chksumlen += sizeof (tcphdr.th_ack);

  // Copy data offset to buf (4 bits) and
  // copy reserved bits to buf (4 bits)
  cvalue = (tcphdr.th_off << 4) + tcphdr.th_x2;
  memcpy (ptr, &cvalue, sizeof (cvalue));
  ptr += sizeof (cvalue);
  chksumlen += sizeof (cvalue);

  // Copy TCP flags to buf (8 bits)
  memcpy (ptr, &tcphdr.th_flags, sizeof (tcphdr.th_flags));
  ptr += sizeof (tcphdr.th_flags);
  chksumlen += sizeof (tcphdr.th_flags);

  // Copy TCP window size to buf (16 bits)
  memcpy (ptr, &tcphdr.th_win, sizeof (tcphdr.th_win));
  ptr += sizeof (tcphdr.th_win);
  chksumlen += sizeof (tcphdr.th_win);

  // Copy TCP checksum to buf (16 bits)
  // Zero, since we don't know it yet
  *ptr = 0; ptr++;
  *ptr = 0; ptr++;
  chksumlen += 2;

  // Copy urgent pointer to buf (16 bits)
  memcpy (ptr, &tcphdr.th_urp, sizeof (tcphdr.th_urp));
  ptr += sizeof (tcphdr.th_urp);
  chksumlen += sizeof (tcphdr.th_urp);

  // Copy TCP options to buf (variable length, but in 32-bit chunks)
  memcpy (ptr, tcp_options, tcp_opt_len);
  ptr += tcp_opt_len;
  chksumlen += tcp_opt_len;

  return checksum ((uint16_t *) buf, chksumlen);
}

// Allocate memory for an array of chars.
char *
allocate_strmem (int len)
{
  void *tmp;

  tmp = (char *) malloc (len * sizeof (char));
  if (tmp != NULL) {
    memset (tmp, 0, len * sizeof (char));
    return (tmp);
  } else {
    fprintf (stderr, "ERROR: Cannot allocate memory for array allocate_strmem().\n");
    exit (EXIT_FAILURE);
  }
}

// Allocate memory for an array of pointers to arrays of chars.
char **
allocate_strmemp (int len)
{
  void *tmp;

  tmp = (char **) malloc (len * sizeof (char *));
  if (tmp != NULL) {
    memset (tmp, 0, len * sizeof (char *));
    return (tmp);
  } else {
    fprintf (stderr, "ERROR: Cannot allocate memory for array allocate_strmemp().\n");
    exit (EXIT_FAILURE);
  }
}

// Allocate memory for an array of unsigned chars.
uint8_t *
allocate_ustrmem (int len)
{
  void *tmp;

  tmp = (uint8_t *) malloc (len * sizeof (uint8_t));
  if (tmp != NULL) {
    memset (tmp, 0, len * sizeof (uint8_t));
    return (tmp);
  } else {
    fprintf (stderr, "ERROR: Cannot allocate memory for array allocate_ustrmem().\n");
    exit (EXIT_FAILURE);
  }
}

// Allocate memory for an array of pointers to arrays of unsigned chars.
uint8_t **
allocate_ustrmemp (int len)
{
  void *tmp;

  tmp = (uint8_t **) malloc (len * sizeof (uint8_t *));
  if (tmp != NULL) {
    memset (tmp, 0, len * sizeof (uint8_t *));
    return (tmp);
  } else {
    fprintf (stderr, "ERROR: Cannot allocate memory for array allocate_ustrmemp().\n");
    exit (EXIT_FAILURE);
  }
}

// Allocate memory for an array of ints.
int *
allocate_intmem (int len)
{
  void *tmp;

  tmp = (int *) malloc (len * sizeof (int));
  if (tmp != NULL) {
    memset (tmp, 0, len * sizeof (int));
    return (tmp);
  } else {
    fprintf (stderr, "ERROR: Cannot allocate memory for array allocate_intmem().\n");
    exit (EXIT_FAILURE);
  }
}
