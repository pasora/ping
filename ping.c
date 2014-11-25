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

unsigned short checksum(unsigned short *data, int datasize)
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

int main(int argc, char* argv[])
{
	/*
	 * ソケットを開く
	 */
	int sockd;
	sockd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
	if(sockd == -1) {
		puts("### socket open error\n");
		return -1;
	} else {
		printf("socket number is %d\n", sockd);
	}

	/*
	 * ICMPエコー要求メッセージを用意
	 */
	struct icmp icmp_send;
	icmp_send.icmp_type = 8;
	icmp_send.icmp_code = 0;
	icmp_send.icmp_cksum = checksum((unsigned short *)&icmp_send, sizeof(icmp_send));
	icmp_send.icmp_id = 0;
	icmp_send.icmp_seq = 0;
	printf("### icmp message created(%lu bytes)\n", sizeof(icmp_send));

	/*
	 * ICMPエコー要求メッセージを送信
	 */
	struct in_addr dstaddr;
	inet_aton(argv[1], &dstaddr);
	printf("### destination address is %s\n", argv[1]);

	struct sockaddr_in to;
	to.sin_len = sizeof(to);
	to.sin_family = AF_INET;
	to.sin_port = htons(80);
	to.sin_addr.s_addr = inet_addr(argv[1]);
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

	recv_bytes = recvfrom(sockd, recv_data, sizeof(recv_data), 0, (struct sockaddr*)&resv_src, (socklen_t *)&addr_len);
	if (recv_bytes < 1) {
		perror("### recvfrom:");
		return -1;
	}
	
	/*
	recv_bytes = recv(sockd, recv_data, sizeof(recv_data), 0);
	if (recv_bytes < 1) {
		perror("### recvfrom:");
		return -1;
	}
	*/

	printf("%zd bytes from %s: icmp_seq=() ttl=() time=() ms\n", recv_bytes, argv[1]);

	/* 
	 * ソケットを閉じる
	 */
	close(sockd);
	puts("### socket closed\n\n");

	return 0;
}

