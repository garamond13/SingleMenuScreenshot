#pragma once

#include "pch.h"

#ifdef NDEBUG
#define smss_assert(keep, discard_if_ndebug) keep
#else
#define smss_assert(keep, discard_if_ndebug) (assert(keep discard_if_ndebug))
#endif

template<typename>
inline constexpr bool always_false_v{ false };

inline void strtoval(const std::string& str, auto& val, size_t* idx = nullptr, [[maybe_unused]] int base = 10)
{
	if constexpr (std::is_same_v<decltype(val), bool&>)
		val = static_cast<bool>(std::stoi(str.data(), idx, base));
	else if constexpr (std::is_same_v<decltype(val), char&>)
		val = static_cast<char>(std::stoi(str.data(), idx, base));
	else if constexpr (std::is_same_v<decltype(val), short&>)
		val = static_cast<short>(std::stoi(str.data(), idx, base));
	else if constexpr (std::is_same_v<decltype(val), int&>)
		val = std::stoi(str.data(), idx, base);
	else if constexpr (std::is_same_v<decltype(val), long&>)
		val = std::stol(str.data(), idx, base);
	else if constexpr (std::is_same_v<decltype(val), long long&>)
		val = std::stoll(str.data(), idx, base);
	else if constexpr (std::is_same_v<decltype(val), unsigned short&>)
		val = static_cast<unsigned short>(std::stoul(str.data(), idx, base));
	else if constexpr (std::is_same_v<decltype(val), unsigned char&>)
		val = static_cast<unsigned char>(std::stoi(str.data(), idx, base));
	else if constexpr (std::is_same_v<decltype(val), unsigned int&>)
		val = static_cast<unsigned int>(std::stoul(str.data(), idx, base));
	else if constexpr (std::is_same_v<decltype(val), unsigned long&>)
		val = std::stoul(str.data(), idx, base);
	else if constexpr (std::is_same_v<decltype(val), unsigned long long&>)
		val = std::stoull(str.data(), idx, base);
	else if constexpr (std::is_same_v<decltype(val), float&>)
		val = std::stof(str.data(), idx);
	else if constexpr (std::is_same_v<decltype(val), double&>)
		val = std::stod(str.data(), idx);
	else if constexpr (std::is_same_v<decltype(val), long double&>)
		val = std::stold(str.data(), idx);
	else
		static_assert(always_false_v<decltype(val)>, "strtoval faild");
}

// Workaround for string literal as template-argument.
template <size_t n>
struct Char_array
{
	constexpr Char_array(const char(&str)[n]) :
		val{ std::to_array(str) }
	{}

	const std::array<char, n> val;
};