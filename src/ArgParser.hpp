#pragma once

#include <string_view>
#include <initializer_list>
#include <vector>
#include <iostream>
#include <map>
#include <charconv>
#include <stdexcept>
#include <format>

//======================================

class ArgParser
{
public:
	// Option definition 
	class OptionDef
	{
	public:
		OptionDef(
			std::string_view full_name, 
			char short_name, 
			std::string_view description
		);

		OptionDef(
			std::string_view full_name, 
			std::string_view description
		);

	private:
		friend class ArgParser;

		std::string_view m_full_name;
		char m_short_name;
		std::string_view m_description;

	};

	// Option value wrapper
	class OptionGetter
	{
	public:
		template<typename T> 
		bool operator==(T other) const;

		template<typename T, typename... Args>
		T get(Args... args) const;

		template<typename T>
		operator T() const;

	private:
		friend class ArgParser;
		OptionGetter(const ArgParser* parser, std::string_view name);

		const ArgParser* m_parser;
		std::string_view m_name;

	};

	ArgParser(const std::initializer_list<OptionDef>& options);

	void parse(int argc, char* argv[]);

	// Integer types
	template<std::integral T> requires (!std::same_as<T, bool>)
	T get(std::string_view option_name) const;

	// Strings
	template<typename T> requires std::same_as<T, const char*>
	T get(std::string_view option_name) const;

	// Flags
	template<typename T> requires std::same_as<T, bool>
	T get(std::string_view option_name) const;

	// Or with default value
	template<typename T>
	T get(std::string_view option_name, T default_value) const;

	OptionGetter operator[](std::string_view option_name) const;

	bool contains(std::string_view option_name) const;

	std::ostream& printUsageReference(std::ostream& stream = std::cout) const;
	friend std::ostream& operator<<(std::ostream& stream, const ArgParser& parser);

private:
	std::string m_program_name {};
	std::vector<OptionDef> m_available_options;

	std::map<std::string_view, std::string_view> m_options {};

};

//======================================

template<typename T>
bool ArgParser::OptionGetter::operator==(T other) const
{
	return m_parser->get<T>(m_name) == other;
}

template<typename T, typename... Args>
T ArgParser::OptionGetter::get(Args... args) const
{
	return m_parser->get<T>(m_name, args...);
}

template<typename T>
ArgParser::OptionGetter::operator T() const
{
	return m_parser->get<T>(m_name);
}

//======================================

template<std::integral T> requires (!std::same_as<T, bool>)
T ArgParser::get(std::string_view option_name) const
{
	if (!contains(option_name))
		throw std::runtime_error(std::format("missing number value for option {}", option_name));

	std::string_view option = m_options.at(option_name);

	T value = static_cast<T>(0);
	auto result = std::from_chars(
		option.data(),
		option.data() + option.length(),
		value
	);

	if (result.ec == std::errc::invalid_argument)
		throw std::runtime_error(std::format("{} is not a valid numeric value for option {}", option, option_name));

	return value;
};

template<typename T> requires std::same_as<T, const char*>
T ArgParser::get(std::string_view option_name) const
{
	if (!contains(option_name))
		throw std::runtime_error(std::format("missing string for option {}", option_name));

	return m_options.at(option_name).data();
}

template<typename T> requires std::same_as<T, bool>
T ArgParser::get(std::string_view option_name) const
{
	return contains(option_name);
}

template<typename T>
T ArgParser::get(std::string_view option_name, T default_value) const
{
	if (!contains(option_name))
		return default_value;

	return get<T>(option_name);
}

//======================================			   