/*
 * ping.c
 *
 * usage: sudo ./ping destination
 *
 */

#include<stdio.h>
#include<string.h>
#include<unistd.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<netinet/ip.h>
#include<netinet/ip_icmp.h>
#include<arpa/inet.h>
#include<sys/types.h>
#include<errno.h>
#include<stdlib.h>
#include<netdb.h>
#include<netinet/ip6.h>
#include<netinet/icmp6.h>

#define IP6_HDRLEN 40
#define ICMP_HDRLEN 8

unsigned short checksumv4(unsigned short *data, int datasize);
int ping4(struct addrinfo *res);
uint16_t checksum (uint16_t *addr, int len);
uint16_t checksumv6 (struct ip6_hdr iphdr, struct icmp6_hdr icmp6hdr);
int ping6(struct addrinfo *res);

int main(int argc, char *argv[])
{
	struct addrinfo hints, *res, *res0;
	int error;
	struct sockaddr_storage from;
	socklen_t fromlen;
	int ls;
	int smax;
	int sockmax;
	fd_set rfd, rfd0;
	int n;
	int i;
	char hbuf[NI_MAXHOST];

	if (argc != 2) {
		fprintf(stderr, "usage: ping [destination]\n");
		exit(1);
	}

	memset(&hints, 0, sizeof(hints));
	error = getaddrinfo(argv[1], NULL, &hints, &res0);
	if (error) {
		fprintf(stderr, "%s: %s\n", argv[1], gai_strerror(error));
		exit(1);
	}

	smax = 0;
	sockmax = -1;
	for (res = res0; res; res = res->ai_next) {
		if (res->ai_family == AF_INET6) {
			puts("IPv6");
			ping6(res);
			return 0;
		} else if (res->ai_family == AF_INET) {
			puts("IPv4");
			ping4(res);
			return 0;
		}
	}
	close(sockd);
	return -1;
}

unsigned short checksumv4(unsigned short *data, int datasize)
{
	unsigned long sum = 0;

	while (datasize > 1) {
		sum += *data;
		data++;
		datasize = datasize - 2;
	}

	if (datasize == 1) {
		sum += *(unsigned char *)data;
	}

	sum = (sum & 0xffff) + (sum >> 16);
	sum = (sum & 0xffff) + (sum >> 16);

	return ~sum;
}

int ping4(struct addrinfo *res)
{
	char addr[50];
	void *ptr;
	int sockd;
	sockd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
	if(sockd == -1) {
		perror("socket");
		return -1;
	}

	struct icmp icmp_send;
	icmp_send.icmp_type = 8;
	icmp_send.icmp_code = 0;
	icmp_send.icmp_cksum = checksumv4((unsigned short *)&icmp_send, sizeof(icmp_send));
	icmp_send.icmp_id = 0;
	icmp_send.icmp_seq = 0;

	struct in_addr dstaddr;
	ptr = &(((struct sockaddr_in *)res->ai_addr)->sin_addr);
	inet_ntop(AF_INET, ptr, addr, sizeof(addr));

	struct sockaddr_in recvSockAddr;

	memset(&recvSockAddr, 0, sizeof(recvSockAddr));
	recvSockAddr.sin_port = htons(80);
	recvSockAddr.sin_family = AF_INET;
	recvSockAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	bind(sockd, (struct sockaddr_in *) &recvSockAddr, sizeof(recvSockAddr));

	struct sockaddr_in to;
	to.sin_len = sizeof(to);
	to.sin_family = AF_INET;
	to.sin_port = htons(80);
	to.sin_addr.s_addr = inet_addr(addr);
	memset(to.sin_zero, 0, 8);

	ssize_t recv_bytes;
	char recv_data[100];
	memset(recv_data, 0, sizeof(recv_data));
	int addr_len = 16;

	for (int i = 0; i < 10; i++) {
		ssize_t send_bytes = sendto(sockd, (const void *)&icmp_send, sizeof(icmp_send), 0, (const struct sockaddr *)&to, sizeof(to));
		recv_bytes = read(sockd, recv_data, sizeof(recv_data));
		if (recv_bytes < 1) {
			perror("### recvfrom:");
			return -1;
		}

		printf("%zd bytes from %s\n", recv_bytes, addr);
	}

	close(sockd);

	return 0;
}

uint16_t checksumv6 (struct ip6_hdr iphdr, struct icmp6_hdr icmp6hdr)
{
  char buf[IP_MAXPACKET];
  char *ptr;
  int chksumlen = 0;
  int i;

  ptr = &buf[0];  // ptr points to beginning of buffer buf

  // Copy source IP address into buf (128 bits)
  memcpy (ptr, &iphdr.ip6_src.s6_addr, sizeof (iphdr.ip6_src.s6_addr));
  ptr += sizeof (iphdr.ip6_src);
  chksumlen += sizeof (iphdr.ip6_src);

  // Copy destination IP address into buf (128 bits)
  memcpy (ptr, &iphdr.ip6_dst.s6_addr, sizeof (iphdr.ip6_dst.s6_addr));
  ptr += sizeof (iphdr.ip6_dst.s6_addr);
  chksumlen += sizeof (iphdr.ip6_dst.s6_addr);

  // Copy Upper Layer Packet length into buf (32 bits).
  // Should not be greater than 65535 (i.e., 2 bytes).
  *ptr = 0; ptr++;
  *ptr = 0; ptr++;
  *ptr = (ICMP_HDRLEN) / 256;
  ptr++;
  *ptr = (ICMP_HDRLEN) % 256;
  ptr++;
  chksumlen += 4;

  // Copy zero field to buf (24 bits)
  *ptr = 0; ptr++;
  *ptr = 0; ptr++;
  *ptr = 0; ptr++;
  chksumlen += 3;

  // Copy next header field to buf (8 bits)
  memcpy (ptr, &iphdr.ip6_nxt, sizeof (iphdr.ip6_nxt));
  ptr += sizeof (iphdr.ip6_nxt);
  chksumlen += sizeof (iphdr.ip6_nxt);

  // Copy ICMPv6 type to buf (8 bits)
  memcpy (ptr, &icmp6hdr.icmp6_type, sizeof (icmp6hdr.icmp6_type));
  ptr += sizeof (icmp6hdr.icmp6_type);
  chksumlen += sizeof (icmp6hdr.icmp6_type);

  // Copy ICMPv6 code to buf (8 bits)
  memcpy (ptr, &icmp6hdr.icmp6_code, sizeof (icmp6hdr.icmp6_code));
  ptr += sizeof (icmp6hdr.icmp6_code);
  chksumlen += sizeof (icmp6hdr.icmp6_code);

  // Copy ICMPv6 ID to buf (16 bits)
  memcpy (ptr, &icmp6hdr.icmp6_id, sizeof (icmp6hdr.icmp6_id));
  ptr += sizeof (icmp6hdr.icmp6_id);
  chksumlen += sizeof (icmp6hdr.icmp6_id);

  // Copy ICMPv6 sequence number to buff (16 bits)
  memcpy (ptr, &icmp6hdr.icmp6_seq, sizeof (icmp6hdr.icmp6_seq));
  ptr += sizeof (icmp6hdr.icmp6_seq);
  chksumlen += sizeof (icmp6hdr.icmp6_seq);

  // Copy ICMPv6 checksum to buf (16 bits)
  // Zero, since we don't know it yet.
  *ptr = 0; ptr++;
  *ptr = 0; ptr++;
  chksumlen += 2;

  return checksum ((uint16_t *) buf, chksumlen);
}

int ping6(struct addrinfo *res)
{
	void *ptr;
	int sockd;
	struct sockaddr_in6 *ipv6;
	struct senddata{
		struct ip6_hdr iphdr;
		struct icmp6_hdr icmphdr;
	}senddata;
	char  *src_ip, *dst_ip;
	uint8_t *data, *ether_frame;
	char recv_data[100];

	strcpy (src_ip, "(INPUT your IPv6 address)");

	ipv6 = (struct sockaddr_in6 *)res->ai_addr;
	ptr = &(ipv6->sin6_addr);
	inet_ntop (AF_INET6, ptr, dst_ip, INET6_ADDRSTRLEN);

	//IPv6 header
	senddata.iphdr.ip6_flow = htonl((6 << 28) | (0 << 20) | 0);
	senddata.iphdr.ip6_plen = htons(ICMP_HDRLEN);
	senddata.iphdr.ip6_nxt = IPPROTO_ICMPV6;
	senddata.iphdr.ip6_hlim = 255;
	inet_pton(AF_INET6, src_ip, &(senddata.iphdr.ip6_src));
	inet_pton(AF_INET6, dst_ip, &(senddata.iphdr.ip6_dst));

	//ICMPv6 header
	senddata.icmphdr.icmp6_type = ICMP6_ECHO_REQUEST;
	senddata.icmphdr.icmp6_code = 0;
	senddata.icmphdr.icmp6_id = htons(1000);
	senddata.icmphdr.icmp6_seq = htons(0);

	senddata.icmphdr.icmp6_cksum = 0;
	senddata.icmphdr.icmp6_cksum = checksumv6(senddata.iphdr, senddata.icmphdr);

	sockd = socket(AF_INET6, SOCK_RAW, IPPROTO_ICMPV6);

	bind(sockd, (struct sockaddr *)&dst_ip, sizeof(dst_ip));


	for (int i = 0; i < 10; i++) {
		ssize_t send_bytes = sendto(sockd, (void *)&senddata, sizeof(senddata), 0, (const struct sockaddr *)&src_ip, sizeof(src_ip));
		ssize_t recv_bytes = read(sockd, recv_data, sizeof(recv_data));
		if (recv_bytes < 1) {
			perror("### recvfrom:");
			return -1;
		}

		printf("%zd bytes from %s\n", recv_bytes, dst_ip);
	}
	return 0;
}
