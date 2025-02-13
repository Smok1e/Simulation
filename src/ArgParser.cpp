#include <iomanip>
#include <filesystem>
#include <cstring>

#include "ArgParser.hpp"

//======================================

ArgParser::OptionDef::OptionDef(
	std::string_view full_name,
	char short_name, 
	std::string_view description,
	bool expect_value /*= false*/
):
	m_full_name(full_name),
	m_short_name(short_name),
	m_description(description),
	m_expect_value(expect_value)
{}

ArgParser::OptionDef::OptionDef(
	std::string_view full_name,
	std::string_view description,
	bool expect_value /*= false*/
):
	OptionDef(
		full_name, 
		full_name[0],
		description,
		expect_value
	)
{}

//======================================

ArgParser::ArgParser(const std::initializer_list<OptionDef>& options):
	m_available_options(options)
{
	// Prevent short option duplicates
	for (auto current = m_available_options.begin(); current < m_available_options.end(); current++)
	{
		auto duplicate = std::find_if(
			std::next(current),
			m_available_options.end(),
			[&](OptionDef& option) -> bool
			{
				return current->m_short_name == option.m_short_name;
			}
		);

		if (duplicate != m_available_options.end())
			throw std::runtime_error(
				std::format(
					"found short option duplicates for -{} (--{} and --{})", 
					current->m_short_name,
					current->m_full_name,
					duplicate->m_full_name
				)
			);
	}
}

void ArgParser::parse(int argc, char* argv[])
{
	m_executable_path = *argv;

	for (size_t i = 1; i < argc; i++)
	{
		// Option
		if (*argv[i] == '-')
		{
			// Full option name variant, e. g. --option
			if (argv[i][1] == '-')
			{
				const char* begin = argv[i] + 2;
				const char* end = strchr(begin, '=');

				auto full_name = end
					? std::string_view(begin, end)
					: std::string_view(begin);

				auto option = std::find_if(
					m_available_options.begin(),
					m_available_options.end(),
					[full_name](const OptionDef& option) -> bool
					{
						return option.m_full_name == full_name;
					}
				);

				if (option == m_available_options.end())
					throw ArgParserException(std::format("unrecognized option '--{}'", full_name));

				const char* value = "";
				if (option->m_expect_value)
				{
					// --option=value
					if (end)
						value = end + 1;

					// --option value
					else if (i + 1 < argc && *argv[i+1] != '-')
						value = argv[++i];

					else
						throw ArgParserException(std::format("expected value for option '--{}'", full_name));
				}

				m_options[option->m_full_name] = value;
			}

			// Short option name variant, e. g. -o
			else
			{
				char short_name = argv[i][1];
				auto option = std::find_if(
					m_available_options.begin(),
					m_available_options.end(),
					[short_name](const OptionDef& option) -> bool
					{
						return option.m_short_name == short_name;
					}
				);

				if (option == m_available_options.end())
					throw ArgParserException(std::format("unrecognized option '-{}'", short_name));

				const char* value = "";
				if (option->m_expect_value)
				{
					if (i + 1 < argc && *argv[i+1] != '-')
						value = argv[++i];

					else
						throw ArgParserException(std::format("expected value for option '-{}'", short_name));
				}

				m_options[option->m_full_name] = value;
			}
		}

		// Plain arguments
		else
			m_arguments.push_back(argv[i]);
	}
}

//======================================

const std::filesystem::path& ArgParser::getExecutablePath() const
{
	return m_executable_path;
}

std::ostream& ArgParser::printAvailableOptions(std::ostream& stream /*= std::cout*/) const
{
	auto calc_optlen = [](const ArgParser::OptionDef& option) -> size_t
	{
		// option[=<value>]
		return option.m_full_name.length() + 8 * option.m_expect_value;
	};

	auto longest_option = std::max_element(
		m_available_options.begin(), 
		m_available_options.end(), 
		[&](const OptionDef& a, const OptionDef& b) -> bool
		{
			return calc_optlen(a) < calc_optlen(b);
		}
	);

	size_t longest_option_length = calc_optlen(*longest_option);
	for (const auto& option: m_available_options)
	{
		stream 
			<< "-" << option.m_short_name << ", "
			<< "--" << std::left;

		if (option.m_expect_value)
			stream 
				<< option.m_full_name 
				<< std::setw(longest_option_length - option.m_full_name.length()) << "=<value>";

		else
			stream 
				<< std::setw(longest_option_length) << option.m_full_name;

		stream
			<< " - " << option.m_description 
			<< std::endl;
	}

	return stream;
}

std::ostream& operator<<(std::ostream& stream, const ArgParser& parser)
{
	return parser.printAvailableOptions(stream);
}

//======================================