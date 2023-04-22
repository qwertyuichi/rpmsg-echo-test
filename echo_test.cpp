#include <string>
#include <sstream>
#include <fstream>
#include <iostream>
#include <filesystem>
#include <cstring>
#include <climits>

#include <chrono>
#include <thread>

#include <stdlib.h>
#include <linux/rpmsg.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>

using std::string;

/**
 * RPMsgを初期化する関数
 * @param	fp_ep_dev	RPMsgエンドポイントへのパス
 * @return         		初期化失敗で負の値を返す
 */
int init_rpmsg(std::ostringstream &fp_ep_dev)
{
	int ret;
	const string RPMSG_DEV_NAME = "virtio0.rpmsg-openamp-demo-channel.-1.0";

	std::cout << "Start initialization for RPMsg." << std::endl;

	/* rpmsg_charドライバのロード */
	std::cout << "\t"
			  << "Loading rpmsg_char driver: ";
	ret = system("modprobe rpmsg_char ");
	if (ret < 0)
	{
		std::cout << "Failed." << std::endl;
		return ret;
	}
	else
	{
		std::cout << "OK." << std::endl;
	}

	/* ファームウェアのロード */
	std::cout << "\t"
			  << "Loading firmware for baremetal: ";
	std::this_thread::sleep_for(std::chrono::milliseconds(500));
	system("echo stop > /sys/class/remoteproc/remoteproc0/state ");
	std::this_thread::sleep_for(std::chrono::milliseconds(100));
	system("echo echo_test.elf > /sys/class/remoteproc/remoteproc0/firmware ");
	std::this_thread::sleep_for(std::chrono::milliseconds(100));
	ret = system("echo start > /sys/class/remoteproc/remoteproc0/state ");
	std::this_thread::sleep_for(std::chrono::milliseconds(100));
	if (ret < 0)
	{
		std::cout << "Failed." << std::endl;
		return ret;
	}
	else
	{
		std::cout << "OK." << std::endl;
	}

	/* デバイスの存在確認 */
	std::ostringstream fp_device_name;
	std::fstream fs_device_name;
	fp_device_name << "/sys/bus/rpmsg/devices/" << RPMSG_DEV_NAME;
	std::cout << "\t"
			  << "Searching rpmsg device: ";
	if (!std::filesystem::exists(fp_device_name.str()))
	{
		std::cout << "Failed." << std::endl;
		std::cerr << "\t\t"
				  << "Error: Not able to access rpmsg device: "
				  << fp_device_name.str() << std::endl;
		return -1;
	}
	else
	{
		std::cout << "OK." << std::endl;
		std::cout << "\t\t"
				  << "Found rpmsg device: "
				  << fp_device_name.str() << std::endl;
	}

	/* rpmsgデバイスドライバのオーバーライド設定 */
	std::ostringstream fp_driver_override;
	fp_driver_override << "/sys/bus/rpmsg/devices/" << RPMSG_DEV_NAME << "/driver_override";
	std::ofstream fs_driver_override(fp_driver_override.str());
	fs_driver_override << "rpmsg_chrdev" << std::endl;
	fs_driver_override.close();

	/* rpmsgデバイスドライバをrpmsg charドライバへバインド */
	std::ostringstream fp_bind;
	fp_bind << "/sys/bus/rpmsg/drivers/rpmsg_chrdev/bind";
	std::ofstream fs_bind(fp_bind.str());
	fs_bind << RPMSG_DEV_NAME << std::endl;
	fs_bind.close();

	/* RPMsgエンドポイント作成先の確認 */
	std::ostringstream fp_rpmsg_dir;
	std::ostringstream fp_rpmsg_ctrl;
	string rpmsg_ctrl_name = "";
	std::cout << "\t"
			  << "Searching control device: ";
	fp_rpmsg_dir << "/sys/bus/rpmsg/devices/" << RPMSG_DEV_NAME << "/rpmsg";
	for (const auto &file : std::filesystem::directory_iterator(fp_rpmsg_dir.str()))
	{
		string file_name = file.path().filename();
		if (file_name.find("rpmsg_ctrl") != string::npos)
		{
			rpmsg_ctrl_name = file_name;
			fp_rpmsg_ctrl << "/dev/" << rpmsg_ctrl_name;
			std::cout << "OK." << std::endl;
			std::cout << "\t\t"
					  << "Found control device: "
					  << rpmsg_ctrl_name << std::endl;
			break;
		}
	}
	if (rpmsg_ctrl_name == "")
	{
		std::cout << "Failed." << std::endl;
		return -1;
	}

	/* RPMsgエンドポイントの作成 */
	struct rpmsg_endpoint_info ep_info;
	strcpy(ep_info.name, "rpmsg-openamp-demo-channel");
	ep_info.src = 0;
	ep_info.dst = 0xFFFFFFFF;
	std::cout << "\t"
			  << "Creating endpoint: ";
	int fd_ep = open(fp_rpmsg_ctrl.str().c_str(), O_RDWR | O_NONBLOCK);
	ret = ioctl(fd_ep, RPMSG_CREATE_EPT_IOCTL, ep_info);
	if (ret < 0)
	{
		std::cout << "Failed." << std::endl;
		close(fd_ep);
		return ret;
	}
	else
	{
		std::cout << "OK." << std::endl;
		std::cout << "\t\t"
				  << "Created endpoint: "
				  << ep_info.name << std::endl;
	}

	/* RPMsgエンドポイントのデバイス名を取得 */
	std::ostringstream ep_dev_name;
	std::cout << "\t"
			  << "Searching endpoint device name: ";
	for (int n = 0; n < 128; n++)
	{
		std::ostringstream sys_rpmsg_ep_name_path;
		sys_rpmsg_ep_name_path << "/sys/class/rpmsg/" << rpmsg_ctrl_name << "/rpmsg" << n << "/name";

		std::ifstream fs(sys_rpmsg_ep_name_path.str());
		string data;
		std::getline(fs, data);
		fs.close();
		if (data == ep_info.name)
		{
			ep_dev_name << "rpmsg" << n;
			std::cout << "OK." << std::endl;
			std::cout << "\t\t"
					  << "Obtained endpoint device name: "
					  << ep_dev_name.str() << std::endl;
			break;
		}
	}
	if (ep_dev_name.str() == "")
	{
		std::cerr << "Failed." << std::endl;
		return -1;
	}

	/* RPMsgエンドポイントへのパスを書き込む */
	fp_ep_dev << "/dev/" << ep_dev_name.str();

	/* 終了処理 */
	std::cout << "Initialization completed." << std::endl;
	if (fd_ep >= 0)
		close(fd_ep);

	return 0;
}

/**
 * 文字列のCRC値(8bit)を計算する関数
 * @param  message CRC値(8bit)を計算する文字列
 * @return         CRC値(8bit)
 */
unsigned char GetCRC8(const std::string &message)
{
	const int MSB_CRC8 = 0x85;
	const char *p = message.c_str();
	unsigned char crc8 = 0;

	for (int length = message.length(); length != 0; length--)
	{
		crc8 ^= *p++;

		for (int i = 0; i < CHAR_BIT; i++)
		{
			if (crc8 & 0x80)
			{
				crc8 <<= 1;
				crc8 ^= MSB_CRC8;
			}
			else
			{
				crc8 <<= 1;
			}
		}
	}

	return crc8;
}

/**
 * メッセージをプライマリシリアルポートとセカンダリシリアルポートに同時に送信する関数
 * @param  message 送信するメッセージ
 * @return         送信成功/失敗
 */
bool SendMessage(std::string message) {
	//int ret;

	// CRC値の追加
	std::ostringstream crc;
	crc << std::setfill('0') << std::setw(3)<< (int)GetCRC8(message);
	message += ",CRC:" + crc.str() + "\n";

	return true;

}

int main(int argc, char *argv[])
{
	std::ostringstream fp_ep_dev;
	int ret = init_rpmsg(fp_ep_dev);
	if (ret < 0)
		return ret;

	/* メッセージの送信 */
	std::fstream fs_ept_dev;
	string str_out, str_in;
	
	fs_ept_dev.open(fp_ep_dev.str(), std::ios_base::in | std::ios_base::out);
	for (int i = 0; i < 1000; i++)
	{
		// テストメッセージの生成
		std::ostringstream dst;
		dst << std::setfill('0') << std::setw(4)<< i;
		str_out = "#MBJAZ#,JTX2,NO:02,FND:01,DEV:+200,DST:" + dst.str() + ",EDG:0000,TT2:03,DV2:+056,DT2:0340,TT3:01,DV3:+153,DT3:0450​";
		//str_out = "#MBJAZ#,JTX2,NO:02,FND:01,DEV:+200,DST:0500,EDG:0000,TT2:03,DV2:+056,DT2:0340,TT3:01,DV3:+153,DT3:0450​";

		// CRC値の追加
		std::ostringstream crc;
		crc << std::setfill('0') << std::setw(3)<< (int)GetCRC8(str_out);
		str_out += ",CRC:" + crc.str() + "\n";

		// メッセージの送信
		fs_ept_dev << str_out << std::endl;
		fs_ept_dev.flush();

		// メッセージの取得
		std::getline(fs_ept_dev, str_in);

		// 結果の表示
		std::cout << "str_out: " << str_out << std::endl;
		std::cout << "str_in : " <<str_in << std::endl;

		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}

	fs_ept_dev.close();

	return 0;
}
