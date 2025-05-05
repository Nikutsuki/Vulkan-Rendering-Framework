// giereczka.cpp : Defines the entry point for the application.
//

#include "giereczka.h"

using namespace std;

int main()
{
	game_engine::Engine engine{};

	try {
		engine.run();
	}
	catch (const std::exception& e) {
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
