#include "com.hpp"

int main(int argc, char *argv[])
{
	CommunicationWithR5 cwr5;


	/* メッセージの送信 */
	string str_out, str_in;

	for (int i = 0; i < 1000; i++)
	{
		// テストメッセージの生成
		std::ostringstream dst;
		dst << std::setfill('0') << std::setw(4) << i;
		str_out = "#MSG#,FV4C,NO:10,FND:01,DEV:+123,DST:" + dst.str();
		cwr5.SendMessage( str_out);
		

		// メッセージの取得
		str_in = "";
		str_in = cwr5.ReceiveMessage();

		// 結果の表示
		std::cout << "str_out: " << str_out << std::endl;
		std::cout << "str_in : " << str_in << std::endl;
		std::cout << std::endl;

		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}

// 	std::ostringstream fp_ep_dev;
// 	int ret = init_rpmsg(fp_ep_dev);
// 	if (ret < 0)
// 		return ret;

// 	/* メッセージの送信 */
// 	string str_out, str_in;

// 	for (int i = 0; i < 1000; i++)
// 	{
// 		// テストメッセージの生成
// 		std::ostringstream dst;
// 		dst << std::setfill('0') << std::setw(4) << i;
// 		str_out = "#MSG#,FV4C,NO:10,FND:01,DEV:+123,DST:" + dst.str();
// 		SendMessage(fp_ep_dev, str_out);
// 		std::cout << "str_out: " << str_out << std::endl;

// 		// メッセージの取得
// 		str_in = "";
// 		str_in = ReceiveMessage(fp_ep_dev);
// 		std::cout << "!!!!!\n";

// 		// 結果の表示
// 		std::cout << "str_in : " << str_in << std::endl;
// 		std::cout << std::endl;

// 		std::this_thread::sleep_for(std::chrono::milliseconds(100));
// 	}

	return 0;
}
