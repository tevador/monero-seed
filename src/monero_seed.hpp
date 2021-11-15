/*
	Copyright (c) 2020 tevador <tevador@gmail.com>
	All rights reserved.
*/

#pragma once
#include <string>
#include <array>
#include <cstdint>
#include <iostream>
#include <ctime>
#include <climits>
#include "gf_poly.hpp"

constexpr unsigned seed_bits = 150;

class monero_seed {
public:
	static const std::string erasure;
	static constexpr size_t size = (seed_bits + CHAR_BIT - 1) / CHAR_BIT;
	static constexpr size_t key_size = 32;
	using secret_key = std::array<uint8_t, key_size>;
	using secret_seed = std::array<uint8_t, size>;
	monero_seed(const std::string& phrase, const std::string& coin);
	monero_seed(std::time_t date_created, const std::string& coin);
	std::time_t date() const {
		return date_;
	}
	const std::string& correction() const {
		return correction_;
	}
	const secret_key& key() const {
		return key_;
	}
	friend std::ostream& operator<<(std::ostream& os, const monero_seed& seed);
private:
	secret_seed seed_;
	secret_key key_;
	std::time_t date_;
	unsigned reserved_;
	std::string correction_;
	gf_poly message_;
};

std::ostream& operator<<(std::ostream& os, const monero_seed::secret_key& key);
