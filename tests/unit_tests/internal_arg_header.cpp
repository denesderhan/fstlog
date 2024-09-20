//Copyright © Dénes Derhán 2022.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#include <catch2/catch_all.hpp>

#include <cstring>
#include <utility>
#include <vector>

#include <fstlog/detail/internal_arg_header.hpp>
#include <fstlog/detail/type_signature.hpp>

TEST_CASE("internal_arg_header") {
	SECTION("signed_char") {
		fstlog::internal_arg_header<signed char> arg_h;
		CHECK(arg_h.data_size <= sizeof(arg_h));
		CHECK(arg_h.signature.size() >= arg_h.signature_length);
		CHECK(arg_h.data_size == 4);
		CHECK(arg_h.signature_length == 1);
		CHECK(arg_h.signature[0] == 'X');
	};
	SECTION("unsigned_char") {
		fstlog::internal_arg_header<unsigned char> arg_h;
		CHECK(arg_h.data_size <= sizeof(arg_h));
		CHECK(arg_h.signature.size() >= arg_h.signature_length);
		CHECK(arg_h.data_size == 4);
		CHECK(arg_h.signature_length == 1);
		CHECK(arg_h.signature[0] == 'P');
	};
	SECTION("unsigned_int") {
		fstlog::internal_arg_header<unsigned int> arg_h;
		CHECK(arg_h.data_size <= sizeof(arg_h));
		CHECK(arg_h.signature.size() >= arg_h.signature_length);
		CHECK(arg_h.data_size == 4);
		CHECK(arg_h.signature_length == 1);
		CHECK(arg_h.signature[0] == 'R');
	};
	SECTION("vec_pair_char_int") {
		fstlog::internal_arg_header<std::vector<std::pair<char, int>>> arg_h;
		CHECK(arg_h.data_size <= sizeof(arg_h));
		CHECK(arg_h.signature.size() >= arg_h.signature_length);
		CHECK(arg_h.data_size == 8);
		CHECK(arg_h.signature_length == 5);
		CHECK(int(arg_h.signature[0]) == 96);
		CHECK(int(arg_h.signature[1]) == 97);
		CHECK(int(arg_h.signature[2]) == 2);
		CHECK(int(arg_h.signature[3]) == 16);
		CHECK(int(arg_h.signature[4]) == 90);

		arg_h.arg_size = 0xffff;
		unsigned char buff[arg_h.data_size];
		std::memcpy(buff, &arg_h, arg_h.data_size);
		CHECK(buff[0] == 0xff);
		CHECK(buff[1] == 0xff);
		CHECK(buff[2] == 5);
		CHECK(buff[3] == 96);
		CHECK(buff[4] == 97);
		CHECK(buff[5] == 2);
		CHECK(buff[6] == 16);
		CHECK(buff[7] == 90);
	};
	
	SECTION("long_type") {
		typedef std::pair<std::vector<std::tuple<int, unsigned long long int, std::string>>, int> long_type;

		constexpr auto t = fstlog::internal_arg_header<long_type>{0xabcd};

		std::vector<unsigned char> buff2(t.data_size);
		std::memcpy(&buff2[0], &t, t.data_size);
		uint16_t num;
		std::memcpy(&num, &buff2[0], 2);
		CHECK(num == 0xabcd);
		CHECK(int(buff2[2]) == fstlog::type_signature<long_type>::value.size());
		for (size_t i = 0; i < fstlog::type_signature<long_type>::value.size(); i++) {
			CHECK(buff2[i + 3] == fstlog::type_signature<long_type>::value[i]);
		}
	};
}
