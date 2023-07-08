#include "com.hpp"

void print_payload(Payload payload)
{
	std::cout << "ball_found: " << std::endl
			  << "\t";
	for (int j = 0; j < 9; j++)
		std::cout << payload.ball_found[j] << " ";
	std::cout << std::endl;

	std::cout << "ball_direction: " << std::endl
			  << "\t";
	for (int j = 0; j < 9; j++)
		std::cout << payload.ball_direction[j] << " ";
	std::cout << std::endl;

	std::cout << "ball_distance: " << std::endl
			  << "\t";
	for (int j = 0; j < 9; j++)
		std::cout << payload.ball_distance[j] << " ";
	std::cout << std::endl;

	std::cout << "white_line_found: " << payload.white_line_found << std::endl;
	std::cout << "blue_line_found: " << payload.blue_line_found << std::endl;
	std::cout << "white_line_distance: " << payload.white_line_distance << std::endl;
	std::cout << "blue_line_distance: " << payload.blue_line_distance << std::endl;
}

int main(int argc, char *argv[])
{
	ComWithR5 cwr5;
	Payload payload = {};

	for (int i = 0; i < 100; i++)
	{
		// payloadの生成
		for (int j = 0; j < 9; j++)
			payload.ball_found[j] = i;
		for (int j = 0; j < 9; j++)
			payload.ball_direction[j] = i * 2;
		for (int j = 0; j < 9; j++)
			payload.ball_distance[j] = i * 3;
		payload.white_line_found = i * 4;
		payload.blue_line_found = i * 5;
		payload.white_line_distance = i * 6;
		payload.blue_line_distance = i * 7;

		// payloadの送信
		cwr5.SendMessage(payload);
		print_payload(payload);

		// payloadの受信
		Payload _payload = {};
		_payload = cwr5.ReceiveMessage();
		print_payload(_payload);
	}

	// for (int i = 0; i < 1000; i++)
	// {
	// 	// テストメッセージの生成
	// 	std::ostringstream dst;
	// 	dst << std::setfill('0') << std::setw(4) << i;
	// 	str_out = "#MSG#,FV4C,NO:10,FND:01,DEV:+123,DST:" + dst.str();
	// 	cwr5.SendMessage( str_out);

	// 	// メッセージの取得
	// 	str_in = "";
	// 	str_in = cwr5.ReceiveMessage();

	// 	// 結果の表示
	// 	std::cout << "str_out: " << str_out << std::endl;
	// 	std::cout << "str_in : " << str_in << std::endl;
	// 	std::cout << std::endl;

	// 	std::this_thread::sleep_for(std::chrono::milliseconds(100));
	// }

	return 0;
}
