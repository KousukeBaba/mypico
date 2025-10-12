#include <string.h>
#include "net_queue.h"

// 送信用キュー(全タスク共通)
static NetTxMsg tx_queue[NET_Q_SIZE];     // 送信用FIFOバッファ
static volatile int tx_head = 0;          // 取り出し位置
static volatile int tx_tail = 0;          // 追加位置
static volatile int tx_count = 0;         // 現在の要素数

bool net_tx_push(int id, const char *msg) {
    __asm volatile("cpsid i");
    if (tx_count >= NET_Q_SIZE) { __asm volatile("cpsie i"); return false; }

    tx_queue[tx_tail].sender_id = id;
    strncpy(tx_queue[tx_tail].msg, msg, MSG_LEN - 1);
    tx_queue[tx_tail].msg[MSG_LEN - 1] = '\0';

    tx_tail = (tx_tail + 1) % NET_Q_SIZE;
    tx_count++;
    __asm volatile("cpsie i");
    return true;
}

bool net_tx_pop(NetTxMsg *out) {
    __asm volatile("cpsid i");
    if (tx_count == 0) { __asm volatile("cpsie i"); return false; }

    *out = tx_queue[tx_head];
    tx_head = (tx_head + 1) % NET_Q_SIZE;
    tx_count--;

    __asm volatile("cpsie i");
    return true;
}

// 受信用キュー（各タスクごと）
static char rx_queue[USER_TASK_MAX][NET_Q_SIZE][MSG_LEN]; // [ユーザID][要素][文字列]
static volatile int rx_head[USER_TASK_MAX] = {0};         // 読み取り位置
static volatile int rx_tail[USER_TASK_MAX] = {0};         // 書き込み位置
static volatile int rx_count[USER_TASK_MAX] = {0};        // 要素数

bool net_rx_push(int id, const char *msg) {
    if (id < 0 || id >= USER_TASK_MAX) return false; 
    __asm volatile("cpsid i");
    if (rx_count[id] >= NET_Q_SIZE) {　//キューが満杯→新データ破棄
        __asm volatile("cpsie i");
        return false;
    }
    strncpy(rx_queue[id][rx_tail[id]], msg, MSG_LEN - 1); // 受信文字列コピー
    rx_queue[id][rx_tail[id]][MSG_LEN - 1] = '\0'; // 終端保証
    rx_tail[id] = (rx_tail[id] + 1) % NET_Q_SIZE;
    rx_count[id]++;
    __asm volatile("cpsie i");
    return true;
}

bool net_rx_pop(int id, char *out) {
    if (id < 0 || id >= USER_TASK_MAX) return false;
    __asm volatile("cpsid i");
    if (rx_count[id] == 0) {
        __asm volatile("cpsie i");
        return false;
    }
    strncpy(out, rx_queue[id][rx_head[id]], MSG_LEN - 1);
    out[MSG_LEN - 1] = '\0';
    rx_head[id] = (rx_head[id] + 1) % NET_Q_SIZE;
    rx_count[id]--;
    __asm volatile("cpsie i");
    return true;
}
