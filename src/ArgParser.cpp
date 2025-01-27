#include <iomanip>
#include <filesystem>
#include <cstring>

#include "ArgParser.hpp"

//======================================

ArgParser::OptionDef::OptionDef(
	std::string_view full_name, 
	char short_name, 
	std::string_view description
):
	m_full_name(full_name),
	m_short_name(short_name),
	m_description(description)
{}

ArgParser::OptionDef::OptionDef(
	std::string_view full_name, 
	std::string_view description
):
	OptionDef(
		full_name, 
		full_name[0], 
		description
	)
{}

//======================================

ArgParser::OptionGetter::OptionGetter(const ArgParser* parser, std::string_view name):
	m_parser(parser),
	m_name(name)
{}

//======================================

ArgParser::ArgParser(const std::initializer_list<OptionDef>& options):
	m_available_options(options)
{}

void ArgParser::parse(int argc, char* argv[])
{
	m_program_name = std::filesystem::path(*argv).filename().string();

	for (size_t i = 1; i < argc; i++)
	{
		// Option
		if (*argv[i] == '-')
		{
			// Full option name
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
					continue;

				// --option=value / --option value / --option
				const char* value = end
					? end + 1
					: i + 1 < argc && *argv[i+1] != '-'
						? argv[i+1]
						: "";

				m_options[option->m_full_name] = value;
			}

			// Short option name
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
					continue;

				// -o value / -o
				const char* value = i + 1 < argc && *argv[i+1] != '-'
					? argv[i+1]
					: "";

				m_options[option->m_full_name] = value;
			}
		}

		// Arguments are ignored
	}
}

//======================================

bool ArgParser::contains(std::string_view option_name) const
{
	return m_options.contains(option_name);
}

ArgParser::OptionGetter ArgParser::operator[](std::string_view option_name) const
{
	return OptionGetter(this, option_name);
}

//======================================

std::ostream& ArgParser::printUsageReference(std::ostream& stream /*= std::cout*/) const
{
	stream << "Usage: " << m_program_name << " [ARGUMENTS] [OPTIONS]" << std::endl;
	stream << "Available options:" << std::endl;

	auto longest_option = std::max_element(
		m_available_options.begin(), 
		m_available_options.end(), 
		[](const OptionDef& a, const OptionDef& b) -> bool
		{
			return a.m_full_name.length() < b.m_full_name.length();
		}
	);

	// -o, --[option] - description
	size_t option_length = longest_option->m_full_name.length();

	for (const auto& option: m_available_options)
	{
		stream 
			<< "  -" << option.m_short_name << ", "
			<< "--" << std::left << std::setw(option_length) << option.m_full_name
			<< " - " << option.m_description 
			<< std::endl;
	}

	return stream;
}

std::ostream& operator<<(std::ostream& stream, const ArgParser& parser)
{
	return parser.printUsageReference(stream);
}

//======================================