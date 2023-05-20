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
#include <map>

#include <stdlib.h>
#include <linux/rpmsg.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>

using std::string;

/**
 * RPMsgを初期化する関数
 * @param[in]	fp_ep_dev	RPMsgエンドポイントへのパス
 * @return		初期化失敗で負の値を返す
 */
int init_rpmsg(std::ostringstream &);

/**
 * 文字列のCRC値(8bit)を計算する関数
 * @param[in]	message CRC値(8bit)を計算する文字列
 * @return		CRC値(8bit)
 */
unsigned char GetCRC8(const std::string &);

/**
 * 入力文字列を指定の区切り文字で分割する関数
 * @param[in] message   入力文字列
 * @param[in] delimiter 区切り文字
 * @return 分割された文字列
 */
std::vector<string> SplitMessage(const string &, const char &);

/**
 * R5へメッセージを送信する関数
 * @param[in]	fp_ep_dev	RPMsgエンドポイントへのパス
 * @param[in]	message 	送信するメッセージ
 */
void SendMessage(const std::ostringstream &, string );

/**
 * R5からメッセージを受信する関数
 * @param[in]	fp_ep_dev	RPMsgエンドポイントへのパス
 * @return		受信したメッセージ
 */
string ReceiveMessage(const std::ostringstream &);

#endif