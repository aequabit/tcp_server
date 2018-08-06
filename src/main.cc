#define _CRT_SECURE_NO_WARNINGS
#include "tcp/server.h"

int main(const int argc, const char **argv) {
	tcp::server *server = nullptr;

	try
	{
		server = new tcp::server("127.0.0.1", 13337);
		server->set_data_callback([](tcp::peer *peer, const char* data)
		{
			printf(data);
		});
		server->start();
	}
	catch (std::exception& ex)
	{
		printf("exception: %s", ex.what());
	}

	while (true) {
		char buffer[4096];
		scanf("%s", buffer);

		for (auto client : server->get_peers())
		{
			if (!strcmp(buffer, "disconnect"))
			{
				client->disconnect();
			}
			else
			{
				client->send(buffer);
			}
		}
	}
}
