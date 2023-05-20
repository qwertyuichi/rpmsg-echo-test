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

/**
 * R5との通信まわりのクラス
 */
class CommunicationWithR5
{
public:
    /**
     * コンストラクタ
     */
    CommunicationWithR5();

    /**
     * R5へメッセージを送信する関数
     * @param[in]	message 	送信するメッセージ
     */
    void SendMessage(string message);

    /**
     * R5からメッセージを受信する関数
     * @return		受信したメッセージ
     */
    string ReceiveMessage();

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
     * 文字列のCRC値(8bit)を計算する関数
     * @param[in]	message CRC値(8bit)を計算する文字列
     * @return		CRC値(8bit)
     */
    unsigned char GetCRC8(const string &message);

    /**
     * 入力文字列を指定の区切り文字で分割する関数
     * @param[in] message   入力文字列
     * @param[in] delimiter 区切り文字
     * @return 分割された文字列
     */
    std::vector<string> SplitMessage(const string &str, char delimiter);
};
#endif