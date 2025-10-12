## ビルド手順　基本的にはpico-exampleと同様
Pico SDK のパスを環境変数 `PICO_SDK_PATH` に設定

main.c l19       `WIFI_SSID`  `WIFI_PASSWORD` `TEST_TCP_SERVER_IP` をそれぞれ設定
server.py         `HOST`：ホストのIPアドレス 


mkdir build
cd build
cmake .. -DPICO_BOARD=pico_w
make
server.py実行
pico側実行




通信処理 main.c メインスレッド：task_lwip()
            　 ユーザスレッド：task_user()

<後で変更する場合>
myos/netqueue.h  `USER_TASK_MAX`：接続するクライアント(スレッド)の数
myos/mytasks.c   `MAX_TASKS`：起動するスレッドの合計数
server.py        `PORTS`：ポート番号