/*
 * ping.c
 *
 * usage: sudo ./ping [IP address(only)]
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

uint16_t checksumv6 (void * buffer, int bytes) {
	uint32_t total;
	uint16_t *ptr;
	int words;

	total = 0;
	ptr = (uint16_t *) buffer;
	words = (bytes + 1) / 2;

	while (words--) {
		total += *ptr++;
	}

	while (total & 0xffff0000) {
		total = (total >> 16) + (total & 0xffff);
	}

	return (uint16_t) total;
}

int ping4(struct addrinfo *res)
{
	char addr[50];
	void *ptr;
	/*
	 * ソケットを開く
	 */
	int sockd;
	sockd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
	if(sockd == -1) {
		perror("socket");
		return -1;
	} else {
		printf("### socket number is %d\n", sockd);
	}

	/*
	 * ICMPエコー要求メッセージを用意
	 */
	struct icmp icmp_send;
	icmp_send.icmp_type = 8;
	icmp_send.icmp_code = 0;
	icmp_send.icmp_cksum = checksumv4((unsigned short *)&icmp_send, sizeof(icmp_send));
	icmp_send.icmp_id = 0;
	icmp_send.icmp_seq = 0;
	printf("### icmp message created(%lu bytes)\n", sizeof(icmp_send));

	/*
	 * ICMPエコー要求メッセージを送信
	 */
	struct in_addr dstaddr;
	ptr = &(((struct sockaddr_in *)res->ai_addr)->sin_addr);
	inet_ntop(AF_INET, ptr, addr, sizeof(addr));
	printf("### destination address is %s\n", addr);

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

	ssize_t send_bytes = sendto(sockd, (const void *)&icmp_send, sizeof(icmp_send), 0, (const struct sockaddr *)&to, sizeof(to));
	printf("### %d bytes message sent\n", (int)send_bytes);

	/* 
	 * パケットを受信
	 */
	ssize_t recv_bytes;
	char recv_data[100];
	memset(recv_data, 0, sizeof(recv_data));
	struct sockaddr_in resv_src;
	int addr_len = 16;

	recv_bytes = read(sockd, recv_data, sizeof(recv_data) /*, 0, (struct sockaddr*)&resv_src, (socklen_t *)&addr_len*/);
	if (recv_bytes < 1) {
		perror("### recvfrom:");
		return -1;
	}
	
	printf("%zd bytes from %s: icmp_seq=() ttl=() time=() ms\n", recv_bytes, addr);

	/* 
	 * ソケットを閉じる
	 */
	close(sockd);
	puts("### socket closed\n\n");

	return 0;
}

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

#ifdef IPV6_V6ONLY
	const int on = 1;
#endif

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
		//s[smax] = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
		if (res->ai_family == AF_INET6) {
			//ping6();
			puts("IPv6");
			return 0;
		} else if (res->ai_family == AF_INET) {
			ping4(res);
			puts("IPv4");
			return 0;
		}
	}
	puts("cannot reach to destination");
	return -1;
}


