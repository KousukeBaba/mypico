#pragma once
#include <stdbool.h>

#define NET_Q_SIZE 16 // 各キューの最大要素数
#define MSG_LEN    64 // 1メッセージの最大長
#define USER_TASK_MAX 2   // clientタスク数

typedef struct {
    int sender_id;       // 送信元タスクID
    char msg[MSG_LEN];   // データ部
} NetTxMsg;

// 送信用
bool net_tx_push(int id, const char *msg);
bool net_tx_pop(NetTxMsg *out);

// 受信用
bool net_rx_push(int id, const char *msg);
bool net_rx_pop(int id, char *out);
