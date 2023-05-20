#include "com.hpp"
//#include <string>

//using std::string;

int main(int argc, char *argv[])
{
	std::ostringstream fp_ep_dev;
	int ret = init_rpmsg(fp_ep_dev);
	if (ret < 0)
		return ret;

	/* メッセージの送信 */
	std::fstream fs_ept_dev;
	string str_out, str_in;

	for (int i = 0; i < 1000; i++)
	{
		// テストメッセージの生成
		std::ostringstream dst;
		dst << std::setfill('0') << std::setw(4) << i;
		str_out = "#MBJAZ#,JTX2,NO:02,FND:01,DEV:+200,DST:" + dst.str() + ",EDG:0000,TT2:03,DV2:+056,DT2:0340,TT3:01,DV3:+153,DT3:0450​";
		SendMessage(fp_ep_dev, str_out);

		// メッセージの取得
		str_in = "";
		str_in = ReceiveMessage(fp_ep_dev);

		// 結果の表示
		std::cout << "str_out: " << str_out << std::endl;
		std::cout << "str_in : " << str_in << std::endl;
		std::cout << std::endl;

		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}

	return 0;
}
