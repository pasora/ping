### ARP のパケットフォーマット
	#define arp_hrd ea_hdr.ar_hrd
		//ハードウェアアドレスフォーマット
		//Ethernet の MAC アドレスが格納される場合は1(ARPHRD_ETHER=1)
	#define arp_pro ea_hdr.ar_pro
		//プロトコルアドレスの種類を表す
	#define arp_hln ea_hdr.ar_hln
		//ハードウェアアドレスの長さ
		//Ethernet の場合は6オクテット
		//IPの場合は4オクテット
	#define arp_pln ea_hdr.ar_pln
	#define arp_op ea_hdr.ar_op
		//ARP がどのような処理を要求しているか
		//ARP 要求なら1(ARPOP_REQUEST)
		//ARP 応答なら2(ARPOP_REPLY)
`arp_sha`	送信元ハードウェアアドレス(MAC アドレス)  
`arp_spa`	送信元プロトコルアドレス(IP アドレス)  
`arp_tha`	ターゲットハードウェアアドレス(MAC アドレス)  
`arp_tpa`	ターゲットプロトコルアドレス(IP アドレス)  

#### ARP 構造体  
	struct arphdr {
		u_short ar_hrd;
		u_short ar_pro;
		u_char ar_hln;
		u_char ar_pln;
		u_short ar_op;
	};
	struct ether_arp {
		struct arphdr ea_hdr;
		u_char arp_sha[ETHER_ADDR_LEN];
		u_char arp_spa[4];
		u_char arp_tha[ETHER_ADDR_LEN];
		u_char arp_tpa[4];
	};

