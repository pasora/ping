## IPヘッダの構造体
	struct ip {
		#if BYTE_ORDER == LITTLE_ENDIAN
			u_int	ip_hl:4,	//ヘッダ長
						ip_v:4;		//バージョン
		#endif
		#if BYTE_ORDER == BIG_ENDIAN
			u_int	ip_v:4,
						ip_hl:4;
		#endif
		u_char	ip_tos;	//サービスタイプ
		u_short	ip_len;	//パケット長
		u_short	ip_id;	//識別子
		u_short	ip_off;	//コントロールフラグ、フラグメントオフセット
		u_char	ip_ttl;	//生存時間
		u_char	ip_p;	//プロトコル番号
		u_short	ip_sum;	//ヘッダチェックサム
		struct	in_addr	ip_src;	//始点IPアドレス
		struct	in_addr	ip_dst;	//終点IPアドレス
	};


`ip_hl`  

	hlen = ip.ip_hl << 2; //IP ヘッダからヘッダ長を計算する  
	ip.ip_hl = hlen >> 2; //ヘッダ長フィールドに値を設定する  

`ip_len`  

IP データグラム全体の長さ  
`u_short` は16ビットなので IP が運べる最大データ長は65535-20=65515オクテット  

`ip_id`  

IP フラグメント処理で使用  
電源投入時に識別子の初期値が決められる  
IP データグラムを送信する毎に値が1増える  

`ip_off`  

	#define IP_RF 0x8000	//予約フラグ、0を格納する決まり
	#define IP_DF 0x4000	//分割禁止フラグ、途中ルータでフラグメント禁止するかどうか
	#define IP_MF 0x2000	//モアフラグメントフラグ、データがフラグメントの途中かどうか
	#define IP_OFFMASK 0x1fff	//フラグメントオフセット、オリジナル IP データグラムのどこに位置していたか

	off = (ip_off & IP_OFFMASK << 3;	//フィールドから値を取り出す
	ip_off = (ip_off & ~IP_OFFMASK) | (off >> 3);	//フィールドに値を設定する

`ip_ttl`  

IP データグラムが通過できるルータの個数  
ルータは IP データグラムを転送するときにこの値を1ずつ減らす  
0になった IP データグラムは廃棄される  

`ip_p`  

上位層のプロトコルが何か  

|番号 |プロトコル |説明 |  
|:-|:-|:-|  
|1|ICMP|Internet Control Message Protocol|  
|2|IGMP|Internet Group Management Protocol|
|4|IP|IP in IP(encapsulasion)|
|6|TCP|Transmission Control Protocol|
|8|EGP|Exterior Gateway Protocol|
|17|UDP|User Datagram Protocol|
|41|IPv6|IPv6|
|46|RSVP|Resource ReSerVation Protocol|
|89|OSPF|Open Shortest Path First|

`ip_sum`  

IP ヘッダのチェックサム  

`ip_src`, `ip_dst`  

それぞれ IP アドレスが格納  

`in_addr`構造体  

	struct in_addr {
		u_int32_t s_addr;
	};


***  

### 6オクテットなど変数型に当てはまらないデータサイズ
→配列で処理
>MACアドレスは `unsigned char` を6つ持つ配列  

	#include<string.h>  
	void *memset(void *s, int c, size_t n);
		//アドレス s から始まる n バイトの領域を値 c で埋める
	void *memcpy(void *dest, const void *src, size_t n);
		//アドレス src から始まる n バイトの領域を
		//アドレス dest から始まる領域にコピー
		//速度重視・コピー元とコピー先のメモリ領域が重なったとき動作保証なし
	void *memmove(void *dest, const void *src, size_t n);
		//アドレス src から始まる n バイトの領域を
		///アドレス dest から始まる領域にコピー
		///コピー元とコピー先のメモリ領域が重なったときも正しく移動できる
	int memcmp(const void *s1, const void *s2, size_t n);
		//アドレス s1 と s2 の領域の先頭から n バイトを比較
		//s1 が大きければ正の値を返す
		//s2 が大きければ負の値を返す
***
### バイトオーダ変換  
	#include <netinet/in.h>

	u_long int htonl(u_long int hostlong);
		//host のバイトオーダを network のバイトオーダに long 型で変換する
	u_short int htons(u_short int hostshort);
	u_long int htohl(u_long int netlong);
	u_short int ntohs(u_short int netshort);

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

