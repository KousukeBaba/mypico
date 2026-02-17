#include <string.h>
#include <time.h>
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"


#include "lwip/pbuf.h"
#include "lwip/tcp.h"

#include "myos/myport.h"
#include "myos/mytasks.h"
#include "myos/systick.h"
#include "myos/net_queue.h"
#include "myos/safe_printf.h"

// 設定
#define STACK_SIZE_LWIP 16384
#define STACK_SIZE_USER 2048
#define WIFI_SSID      ""
#define WIFI_PASSWORD  ""
#define TEST_TCP_SERVER_IP ""
#define TCP_PORT_BASE 4242 //ポートは接続数に応じて 4242,4243,4244,...
#define BUF_SIZE 2048 //TCP受信バッファサイズ

//メインスレッドに投げかけるキュー
typedef struct {
    char msg[64];
} NetReq;



static ip_addr_t remote_addr;

typedef struct TCP_CLIENT_T_ {
    struct tcp_pcb *tcp_pcb;
    ip_addr_t remote_addr;
    uint8_t buffer[BUF_SIZE];
    int buffer_len;
    int sent_len;
    bool complete;
    int run_count;
    volatile bool connected; //volatileが無い場合userタスクが接続状態を認識できない

	// 追加
    uint16_t port;   
	int user_id; //portで識別すればよいので後で消す
} TCP_CLIENT_T;


static err_t tcp_client_close(void *arg);
static err_t tcp_result(void *arg, int status);
static err_t tcp_client_connected(void *arg, struct tcp_pcb *tpcb, err_t err);
static err_t tcp_client_sent(void *arg, struct tcp_pcb *tpcb, u16_t len);
static err_t tcp_client_recv(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err);
static void tcp_client_err(void *arg, err_t err);

static TCP_CLIENT_T clients[USER_TASK_MAX];

static err_t tcp_client_close(void *arg) {
    TCP_CLIENT_T *state = (TCP_CLIENT_T*)arg;
    if (state->tcp_pcb) {
        tcp_arg(state->tcp_pcb, NULL);
        tcp_poll(state->tcp_pcb, NULL, 0);
        tcp_sent(state->tcp_pcb, NULL);
        tcp_recv(state->tcp_pcb, NULL);
        tcp_err(state->tcp_pcb, NULL);
        tcp_close(state->tcp_pcb);
        state->tcp_pcb = NULL;
    }
    return ERR_OK;
}

static err_t tcp_result(void *arg, int status) {
    TCP_CLIENT_T *state = (TCP_CLIENT_T*)arg;
    state->complete = true;
    return tcp_client_close(arg);
}

static err_t tcp_client_connected(void *arg, struct tcp_pcb *tpcb, err_t err) { //接続完了後のコールバック
    TCP_CLIENT_T *state = (TCP_CLIENT_T*)arg;
    if (err != ERR_OK) return tcp_result(arg, err);

    state->connected = true;
	safe_printf("Connected to server\n");


    // 接続直後にメッセージ送信
    const char *msg = "Hello from Pico W (scheduler test)\n";
	//cyw43_arch_lwip_begin();
    __asm volatile("cpsid i"); 
    err_t wr_err = tcp_write(tpcb, msg, strlen(msg), TCP_WRITE_FLAG_COPY);
	__asm volatile("cpsie i"); 
    //cyw43_arch_lwip_end();

    if (wr_err != ERR_OK) {
        safe_printf("tcp_write failed: %d\n", wr_err);
        return tcp_result(arg, wr_err);
    } else {
        safe_printf("Message sent to server\n");
    }

    return ERR_OK;
}

static err_t tcp_client_sent(void *arg, struct tcp_pcb *tpcb, u16_t len) {
    TCP_CLIENT_T *state = (TCP_CLIENT_T*)arg;
    state->sent_len += len;
    if (state->sent_len >= BUF_SIZE) tcp_result(arg, 0);
    return ERR_OK;
}

static err_t tcp_client_recv(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err) {
    if (!p) return tcp_result(arg, -1);

    TCP_CLIENT_T *state = (TCP_CLIENT_T*)arg;

    for (struct pbuf *q = p; q != NULL; q = q->next) {
        char temp[sizeof(state->buffer)];
        int len = q->len;
        if (len >= (int)sizeof(temp)) len = sizeof(temp) - 1; // バッファオーバー防止
        memcpy(temp, q->payload, len);
        temp[len] = '\0';  // 終端追加
        net_rx_push(state->user_id, temp); //受信データを対応ユーザタスクのキューに投げる
    }

    tcp_recved(tpcb, p->tot_len);
    pbuf_free(p);
    return ERR_OK;
}


static void tcp_client_err(void *arg, err_t err) {
    tcp_result(arg, err);
}

static void tcp_client_init(TCP_CLIENT_T *s, uint16_t port) {
    ip4addr_aton(TEST_TCP_SERVER_IP, &s->remote_addr);
    s->tcp_pcb = NULL;
    s->buffer_len = 0;
    s->sent_len = 0;
    s->complete = false;
    s->run_count = 0;
    s->connected = false;
    s->port = port;
}

 //接続処理 各ユーザタスクからメインスレッドに要求するよう変更
static bool tcp_client_open_port(TCP_CLIENT_T *s, uint16_t port) {
    s->tcp_pcb = tcp_new_ip_type(IP_GET_TYPE(&s->remote_addr));
    tcp_arg(s->tcp_pcb, s);
    tcp_sent(s->tcp_pcb, tcp_client_sent);
    tcp_recv(s->tcp_pcb, tcp_client_recv);
    tcp_err(s->tcp_pcb, tcp_client_err);

    __asm volatile("cpsid i");
    err_t err = tcp_connect(s->tcp_pcb, &s->remote_addr, port, tcp_client_connected);
    __asm volatile("cpsie i");
    return err == ERR_OK;
}



// lwIPメインスレッド
static StackType_t lwip_stack[STACK_SIZE_LWIP];

void task_lwip(void *arg) {
    safe_printf("LWIP main thread start\n");

    // 各接続初期化　
    for (int i = 0; i < USER_TASK_MAX; i++) {
    	TCP_CLIENT_T *s = &clients[i];
    	tcp_client_init(s, TCP_PORT_BASE + i);
    	s->user_id = i;
    	safe_printf("Connecting user%d to port %d\n", i, s->port);
    	if (!tcp_client_open_port(s, s->port)) {
        	safe_printf("tcp_client_open failed (user%d)\n", i);
    	}
	}


    // メインループ（全接続を一元管理）
    while (1) {

		__asm volatile("cpsid i"); 
        cyw43_arch_poll(); //ポーリング処理
		__asm volatile("cpsie i");

        //タイムアウト確認
        cyw43_arch_wait_for_work_until(make_timeout_time_ms(50)); //割り込み禁止NG

        //ユーザタスクからの送信要求確認、接続も後で実装
        for (int i = 0; i < USER_TASK_MAX; i++) {
            char msg[64];
            NetTxMsg tx;
			if (net_tx_pop(&tx)) {
    			int id = tx.sender_id;
    			if (clients[id].connected) {
        			err_t e = tcp_write(clients[id].tcp_pcb, tx.msg, strlen(tx.msg), TCP_WRITE_FLAG_COPY);
        				if (e == ERR_OK)
            				safe_printf("[Port %d] Sent: %s\n", clients[id].port, tx.msg);
        				else
            				safe_printf("tcp_write failed (port %d): %d\n", clients[id].port, e);
    			}
			}

        }
    }
}



static StackType_t user_stacks[USER_TASK_MAX][STACK_SIZE_USER];

void task_user(void *arg) {
    int my_id = (int)(uintptr_t)arg;
    safe_printf("User thread %d start\n", my_id);

    while (!clients[my_id].connected) tight_loop_contents(); //接続完了待ち

    int count = 0;
    char buf[64], recv_buf[64];


    for (int i = 0; i < 10; i++) {
        // 送信処理
		memset(buf, 0, sizeof(buf));
        snprintf(buf, sizeof(buf), "Message #%d from user%d\n", count++, my_id);
		if (net_tx_push(my_id, buf)) safe_printf("Queued: %s\n", buf);

        // 受信処理
		memset(recv_buf, 0, sizeof(recv_buf));
        if (net_rx_pop(my_id, recv_buf)) {
            safe_printf("[User%d received]: %s\n", my_id, recv_buf);
        }

        systick_delay_ms(500);
    }

    while (true) tight_loop_contents();
}



int main(void) {
    stdio_init_all();

    //30s待機
    safe_printf("Wait 30s...\n");
    sleep_ms(10000);  
    safe_printf("Wait 20s...\n");
    sleep_ms(10000);  
    safe_printf("Wait 10s...\n");
    sleep_ms(10000);

    if (cyw43_arch_init()) { safe_printf("Wi-Fi init failed\n"); while (1); }
    cyw43_arch_enable_sta_mode();

    safe_printf("Connecting to Wi-Fi...\n");
	//タイムアウト30秒
    if (cyw43_arch_wifi_connect_timeout_ms(WIFI_SSID, WIFI_PASSWORD,
                                           CYW43_AUTH_WPA2_AES_PSK, 30000)) {
        safe_printf("Connect failed\n");
        while (1);
    }
    safe_printf("Wi-Fi connected\n");

    systick_init();

    create_task(lwip_stack, STACK_SIZE_LWIP, task_lwip, NULL);

    for (int i = 0; i < USER_TASK_MAX; i++) {
        create_task(user_stacks[i], STACK_SIZE_USER, task_user, (void *)(uintptr_t)i);
    }

    start_scheduler();

    while (true) tight_loop_contents();
}

