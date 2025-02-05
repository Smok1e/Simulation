#pragma once

#include <string_view>
#include <initializer_list>
#include <vector>
#include <iostream>
#include <map>
#include <charconv>
#include <stdexcept>
#include <format>
#include <concepts>
#include <filesystem>

//======================================

// Helper concept for restricting T to be any of N types
template<typename T, typename... U>
concept AnyOf = (std::same_as<T, U> || ...);

// Option/argument index (can be either string for options or number for arguments)
template<typename T>
concept ArgParserValueIndex = AnyOf<T, std::string_view, const char*> || std::integral<T>;

//======================================

class ArgParserException: public std::runtime_error
{
public:
	using std::runtime_error::runtime_error;

};

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
			std::string_view description,
			bool m_expect_value = false
		);

		OptionDef(
			std::string_view full_name, 
			std::string_view description,
			bool m_expect_value = false
		);

	private:
		friend class ArgParser;

		std::string_view m_full_name;
		char m_short_name;
		std::string_view m_description;
		bool m_expect_value;

	};

	// Option/argument value proxy 
	template<ArgParserValueIndex IndexT>
	class ValueProxy
	{
	public:
		bool exists() const;

		// Get raw string value
		template<AnyOf<std::string_view, const char*> T>
		T as() const;

		// Interpret as flag (just check if such key exists)
		template<std::same_as<bool> T>
		T as() const;

		// Interpret as integer
		template<std::integral T> requires (!std::same_as<bool, T>)
		T as() const;

		// Any variant above with default variant
		template<typename T>
		T as(T default_value) const;

		template<typename T>
		T operator()(T default_value) const;

		template<typename T>
		bool operator==(T other) const;

		template<typename T>
		operator T() const;

	private:
		friend class ArgParser;

		ValueProxy(const ArgParser* parser, IndexT index);

		const ArgParser* m_parser;
		IndexT m_index;

	};

	template<ArgParserValueIndex IndexT>
	friend class ValueProxy;

	ArgParser(const std::initializer_list<OptionDef>& options);

	void parse(int argc, char* argv[]);

	template<ArgParserValueIndex IndexT>
	ValueProxy<IndexT> get(IndexT index) const;

	template<ArgParserValueIndex IndexT>
	ValueProxy<IndexT> operator[](IndexT index) const;

	const std::filesystem::path& getExecutablePath() const;

	std::ostream& printAvailableOptions(std::ostream& stream = std::cout) const;
	friend std::ostream& operator<<(std::ostream& stream, const ArgParser& parser);

private:
	std::filesystem::path m_executable_path {};
	std::vector<OptionDef> m_available_options;

	std::map<std::string_view, std::string_view> m_options {};
	std::vector<std::string_view> m_arguments {};

};

//======================================

template<ArgParserValueIndex IndexT>
ArgParser::ValueProxy<IndexT> ArgParser::get(IndexT index) const
{
	return ValueProxy(this, index);
}

template<ArgParserValueIndex IndexT>
ArgParser::ValueProxy<IndexT> ArgParser::operator[](IndexT index) const
{
	return get(index);
}

//======================================

template<ArgParserValueIndex IndexT>
ArgParser::ValueProxy<IndexT>::ValueProxy(const ArgParser* parser, IndexT index):
	m_parser(parser),
	m_index(index)
{}

template<ArgParserValueIndex IndexT>
bool ArgParser::ValueProxy<IndexT>::exists() const
{
	if constexpr (std::is_integral_v<IndexT>)
		return m_index < m_parser->m_arguments.size();

	else
		return m_parser->m_options.contains(m_index);
}

template<ArgParserValueIndex IndexT>
template<AnyOf<std::string_view, const char*> T>
T ArgParser::ValueProxy<IndexT>::as() const
{
	if constexpr (std::is_integral_v<IndexT>)
	{
		if (!exists())
			throw ArgParserException(std::format("missing required argument at position {}", m_index + 1));

		return m_parser->m_arguments[m_index];
	}

	else
	{
		if (!exists())
			throw ArgParserException(std::format("missing required option --{}", m_index));

		auto value = m_parser->m_options.at(m_index);
		if constexpr (std::is_same_v<T, std::string_view>)
			return value.data();

		else
			return value;
	}
}

template<ArgParserValueIndex IndexT>
template<std::same_as<bool> T>
T ArgParser::ValueProxy<IndexT>::as() const
{
	return exists();
}

template<ArgParserValueIndex IndexT>
template<std::integral T> requires (!std::same_as<bool, T>)
T ArgParser::ValueProxy<IndexT>::as() const
{
	auto option = as<std::string_view>();

	T value = static_cast<T>(0);
	auto result = std::from_chars(
		option.data(),
		option.data() + option.length(),
		value
	);

	if (result.ec == std::errc::invalid_argument)
		throw ArgParserException(std::format("{} is not a valid numeric value", option));

	return value;
}

template<ArgParserValueIndex IndexT>
template<typename T>
T ArgParser::ValueProxy<IndexT>::as(T default_value) const
{
	return exists()? as<T>(): default_value;
}

template<ArgParserValueIndex IndexT>
template<typename T>
T ArgParser::ValueProxy<IndexT>::operator()(T default_value) const
{
	return as<T>(default_value);
}

template<ArgParserValueIndex IndexT>
template<typename T>
bool ArgParser::ValueProxy<IndexT>::operator==(T other) const
{
	return as<T>() == other;
}

template<ArgParserValueIndex IndexT>
template<typename T>
ArgParser::ValueProxy<IndexT>::operator T() const
{
	return as<T>();
}

//======================================			   