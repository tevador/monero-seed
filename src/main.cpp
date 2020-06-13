/*
	Copyright (c) 2020 tevador <tevador@gmail.com>
	All rights reserved.
*/

#include "monero_seed.hpp"
#include <iostream>
#include <iomanip>
#include <sstream>
#include <stdexcept>
#include <cstring>

static inline void read_string_option(const char* option, int argc, char** argv, char** out) {
	for (int i = 0; i < argc - 1; ++i) {
		if (strcmp(argv[i], option) == 0) {
			*out = argv[i + 1];
			return;
		}
	}
	*out = NULL;
}

static time_t parse_date(const char* s) {
	std::istringstream iss(s);
	char delimiter;
	int day, month, year;
	if (iss >> year >> delimiter >> month >> delimiter >> day) {
		struct tm t = { 0 };
		t.tm_mday = day;
		t.tm_mon = month - 1;
		t.tm_year = year - 1900;
		t.tm_isdst = -1;

		time_t dt = mktime(&t);
		if (dt != -1) {
			return dt;
		}
	}
	throw std::runtime_error("invalid date");
}

void print_seed(const monero_seed& seed, bool phrase) {
	if (!seed.correction().empty()) {
		std::cout << "Warning: corrected erasure: " << monero_seed::erasure << " -> " << seed.correction() << std::endl;
	}
	if (phrase) {
		std::cout << "Mnemonic phrase: " << seed << std::endl;
	}
	std::cout << "- version: " << seed.version() << std::endl;
	std::cout << "- private key: " << seed.key() << std::endl;
	auto created_on = seed.date();
	std::tm tm = *std::localtime(&created_on);
	std::cout << "- created on or after: " << std::put_time(&tm, "%d/%b/%Y") << std::endl;
}

int main(int argc, char** argv) {
	char* create;
	char* restore;
	read_string_option("--create", argc, argv, &create);
	read_string_option("--restore", argc, argv, &restore);

	try {
		if (create != NULL) {
			monero_seed seed(parse_date(create));
			print_seed(seed, true);
		}
		else if (restore != NULL) {
			monero_seed seed(restore);
			print_seed(seed, false);
		}
		else {
			std::cout << "Monero 14-word mnemonic seed proof of concept" << std::endl;
			std::cout << "Usage: " << std::endl;
			std::cout << argv[0] << " --create <yyyy-MM-dd>" << std::endl;
			std::cout << argv[0] << " --restore <14-word seed>" << std::endl;
		}
	}
	catch (const std::exception & ex) {
		std::cout << "ERROR: " << ex.what() << std::endl;
		return 1;
	}
	return 0;
}
