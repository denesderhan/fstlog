//Copyright © Dénes Derhán 2022.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#define CATCH_CONFIG_RUNNER
#include <catch2/catch_all.hpp>

int main(int argc, char* argv[]) {
	// global setup...

	int result = Catch::Session().run(argc, argv);

	// global clean-up...

	return result;
}
