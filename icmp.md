# Internet Control Message Protocol
IP の機能を補助するプロトコル 
ヘッダは共用体で定義  

|`#define` されている文字|タイプ|意味|
|---|---|---|
|`ICMP_ECHOREPLY`|0|エコー応答| 
|`ICMP_UNREACH`|3|到達不能|
|`ICMP_SOURCEQUENCH`|4|始点抑制|
|`ICMP_REDIRECT`|5|リダイレクト|
|`ICMP_ECHO`|8|エコー要求|
|`ICMP_TIMXCEED`|11|時間超過|
|`ICMP_PARAMPROB`|12|パラメータエラー|

## ICMP エコー要求・エコー応答メッセージ
`ping` コマンドで使用  

### パケット構造体
	struct icmp {
		u_char icmp_type;	//type of message
		u_char icmp_code;	//type sub code
		u_short icmp_cksum;	//ones complement cksum of struct
		n_short icmp_id;
		n_short icmp_seq;
		char icmp_data[1];
	};

`icmp_type`  
エコー要求なら8、エコー応答なら0が格納される  

`icmp_code`  
0が格納される  

`icmp_cksum`  
チェックサム  

`icmp_id`  
複数の `ping` コマンドを同じコンピュータで実行した時に識別する ID  

`icmp_seq`  
送信側が何番目の icmp パケットを送信したか  
受信側がエコー応答メッセージを返す場合にこの番号をそのまま返す  

`icmp_data`  
適当なデータを入れる  
受信側がエコー応答メッセージを返す場合にこのデータをそのまま返す  

## ICMP 到達不能メッセージ
何らかの原因で目的のホストまで IP データグラムを届けるのが不可能になった場合に生成  

### パケット構造体
	struct icmp {
		u_char icmp_type;	//type of message
		u_char icmp_code;	//type sub code
		u_short icmp_cksum;	//ones complement cksum of struct
		n_short icmp_pmvoid;
		n_short icmp_nextmtu;
		char icmp_data[1];
	};

`icmp_type`  
3が格納される  

`icmp_code`  

|`#define` されている文字|タイプ|意味|
|---|---|---|
|`ICMP_UNREACH_NET`|0|ネットワーク到達不可能(network unreachable)|
|`ICMP_UNREACH_HOST`|1|ホスト到達不可能(host unreachable)|
|`ICMP_UNREACH_PROTOCOL`|2|プロトコル到達不可能(protocol unreachable)|
|`ICMP_UNREACH_PORT`|3|ポート到達不可能(port unreachable)|
|`ICMP_UNREACH_NEEDFRAG`|4|フラグメントが必要だが DF がセットされている<br>(fragmentation needed and DF set)|
|`ICMP_UNREACH_SRCFAIL`|5|ソースルーティングが失敗した(source route failed)| 

`icmp_cksum`  
チェックサム  

`icmp_pmvoid`  
未使用  

`icmp_nextmtu`  
コード4の時にのみ使用  
経路 MTU 探索で使用  
次のデータリンクの MTU の値が格納される  

`icmp_data`  
エラーを発生させた IP データグラムの IP ヘッダとそれに続く8バイトのデータが格納される  
## ICMP リダイレクトメッセージ
IP データグラムを送信したホストのルーティングテーブルに非効率な点があったときに強制的に修正するために送信  

### パケット構造体

	struct icmp {
		u_char icmp_type;	//type of message
		u_char icmp_code;	//type sub code
		u_short icmp_cksum;	//ones complement cksum of struct
		struct in_addr icmp_gwaddr;
		char icmp_data[1];
	};

`icmp_type`  
5が格納される

`icmp_code`  

|`#define` されている文字|コード|意味|
|---|---|---|
|`ICMP_REDIRECT_NET`|0|ネットワークアドレスリダイレクト|
|`ICMP_REDIRECT_HOST`|1|ホストアドレスリダイレクト|
|`ICMP_REDIRECT_TOSNET`|2|そのネットワークと TOS に対するリダイレクト|
|`ICMP_REDIRECT_TOSHOST`|3|ホストアドレスと TOS に対するリダイレクト|

`icmp_cksum`  
チェックサム  

`icmp_gwaddr`  
ルーティングテーブルの転送先に記述する IP アドレスが格納される  
IP アドレスは`in_addr`構造体に格納される  

`in_addr`構造体  

	struct in_addr {
		u_int32_t s_addr;
	};

`icmp_data`  
リダイレクトが必要と考えられる IP データグラムの IP ヘッダとそれに続く8バイトのデータが格納される  

## ICMP 時間超過メッセージ
IP ヘッダの TTL が0になったとき、フラグメントされたパケットをリアセンブルできず廃棄した時に送信  

### パケット構造体

	struct icmp {
		u_char icmp_type;	//type of message
		u_char icmp_code;	//type sub code
		u_short icmp_cksum;	//ones complement cksum of struct
		n_int icmp_void;
		char icmp_data[1];
	};

`icmp_type`  
11が格納される  

`icmp_code`  
0 -> TTL が0になって IP データグラムが廃棄された  
1 -> IP フラグメントのリアセンブルがタイムアウトしてデータグラムが廃棄された

`icmp_cksum`  
チェックサム  

`icmp_void`  
使用しない  

`icmp_data`  
エラーを発生させた IP データグラムの IP ヘッダとそれに続く8バイトのデータが格納される  

## 共用体・実際の ICMP パケット構造体


