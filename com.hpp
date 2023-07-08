#ifndef COM_H
#define COM_H

#include <string>
#include <sstream>
#include <fstream>
#include <iostream>
#include <filesystem>
#include <cstring>
#include <climits>
#include <vector>
#include <chrono>
#include <thread>
#include <future>

#include <stdlib.h>
#include <linux/rpmsg.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>

using std::string;

// R5と受け渡しされるペイロード構造体
struct Payload {
    int ball_found[9];          // ボール認識の有無を示すステータスコード
    int ball_direction[9];      // ターゲットの子や中心からの逸脱の程度
    int ball_distance[9];       // ボールまでの距離
    int ball_height[9];         // ボールの床面からの高さ[mm]
    int white_line_found;       // 白線認識の有無を示すステータスコード
    int white_line_distance;    // 白線までの距離 [mm]
    int blue_line_found;        // 青線認識の有無を示すステータスコード
    int blue_line_distance;     // 青線までの距離 [mm]
    int checksum;              // チェックサム
};

/**
 * R5との通信まわりのクラス
 */
class ComWithR5
{
public:
    /**
     * コンストラクタ
     */
    ComWithR5();

    /**
     * R5へpayloadを送信する関数
     * @param[in]	payload 	送信するpayload
     */
    void SendMessage(Payload payload);

    /**
     * R5からpayloadを受信する関数
     * @return		受信したpayload
     */
    Payload ReceiveMessage();

private:
    /**
     * メンバ変数の宣言
     */
    std::ostringstream fp_ep_dev; // RPMsgエンドポイントへのパス

    /**
     * RPMsgを初期化する関数
     * @param[in]	fp_ep_dev	RPMsgエンドポイントへのパス
     * @return		初期化失敗で負の値を返す
     */
    int init_rpmsg(std::ostringstream &fp_ep_dev);

    /**
     * @brief payloadのchecksumを計算して返す関数
     * 
     * @return checksum 
     */
    int calcChecksum(Payload payload);
};
#endif