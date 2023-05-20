#include "com.hpp"

CommunicationWithR5::CommunicationWithR5()
{
	int ret = init_rpmsg(fp_ep_dev);
	if (ret < 0)
		throw std::runtime_error("Error: cannot initialize RPMsg");
}

void CommunicationWithR5::SendMessage(string message)
{
	// CRC値の追加
	std::ostringstream crc;
	crc << std::setfill('0') << std::setw(3) << (int)GetCRC8(message);
	message += ",CRC:" + crc.str() + "\n";

	// メッセージの送信
	std::ofstream fs;
	fs.open(fp_ep_dev.str());
	fs << message << std::endl;
	fs.close();
}

string CommunicationWithR5::ReceiveMessage()
{
	string message;

	// メッセージの取得
	std::ifstream fs;
	fs.open(fp_ep_dev.str());
	std::getline(fs, message);
	fs.close();

	// CRC値の取得
	std::vector<string> fields = SplitMessage(message, ',');
	int crc8 = stoi(SplitMessage(fields.back(), ':').back());

	// CRCなしのメッセージを取得
	fields.pop_back();
	string message_without_crc = "";
	for (auto field : fields)
	{
		message_without_crc += field;
		if (field != fields.back())
		{
			message_without_crc += ",";
		}
	}

	// CRC値が一致しないときは空のメッセージを返す
	if ((int)GetCRC8(message_without_crc) != crc8)
		return "";

	return message;
}

int CommunicationWithR5::init_rpmsg(std::ostringstream &fp_ep_dev)
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
				  << "Found rpmsg device: " << fp_device_name.str()
				  << std::endl;
	}

	/* rpmsgデバイスドライバのオーバーライド設定 */
	std::ostringstream fp_driver_override;
	fp_driver_override << "/sys/bus/rpmsg/devices/" << RPMSG_DEV_NAME
					   << "/driver_override";
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
	for (const auto &file :
		 std::filesystem::directory_iterator(fp_rpmsg_dir.str()))
	{
		string file_name = file.path().filename();
		if (file_name.find("rpmsg_ctrl") != string::npos)
		{
			rpmsg_ctrl_name = file_name;
			fp_rpmsg_ctrl << "/dev/" << rpmsg_ctrl_name;
			std::cout << "OK." << std::endl;
			std::cout << "\t\t"
					  << "Found control device: " << rpmsg_ctrl_name
					  << std::endl;
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
				  << "Created endpoint: " << ep_info.name << std::endl;
	}

	/* RPMsgエンドポイントのデバイス名を取得 */
	std::ostringstream ep_dev_name;
	std::cout << "\t"
			  << "Searching endpoint device name: ";
	for (int n = 0; n < 128; n++)
	{
		std::ostringstream sys_rpmsg_ep_name_path;
		sys_rpmsg_ep_name_path << "/sys/class/rpmsg/" << rpmsg_ctrl_name
							   << "/rpmsg" << n << "/name";

		std::ifstream fs(sys_rpmsg_ep_name_path.str());
		string data;
		std::getline(fs, data);
		fs.close();
		if (data == ep_info.name)
		{
			ep_dev_name << "rpmsg" << n;
			std::cout << "OK." << std::endl;
			std::cout << "\t\t"
					  << "Obtained endpoint device name: " << ep_dev_name.str()
					  << std::endl;
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

unsigned char CommunicationWithR5::GetCRC8(const string &message)
{
	const int MSB_CRC8 = 0x85; // 生成多項式(x8 + x7 + x2 + 1)
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

std::vector<string> CommunicationWithR5::SplitMessage(const string &str, char delimiter)
{
	std::vector<string> result;
	std::stringstream ss{str};
	string buf;

	while (std::getline(ss, buf, delimiter))
	{
		result.push_back(buf);
	}

	return result;
}
