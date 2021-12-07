#pragma comment(lib,"ws2_32.lib")
#include "echoserver.h"
#include <string>

int main()
{
	EchoServer server;
	server.InitSocket();
	server.BindListen();
	server.Run(MAXClient);

	cout << "input quit for stop service   ";
	while (true)
	{
		std::string cmd;
		std::getline(std::cin, cmd);
		if (!cmd.compare("quit"))
			break;
	}
	server.Exit();
	return 0;
}