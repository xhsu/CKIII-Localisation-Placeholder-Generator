// CKIII Localisation Placeholder Generator.cpp : This file contains the 'main' function. Program execution begins and ends there.
//


// C++ STL
#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <map>
#include <ranges>
#include <string_view>
#include <string>

#include <fmt/chrono.h>
#include <fmt/color.h>
#include <fmt/ostream.h>

namespace Style
{
	inline constexpr auto Error = fmt::fg(fmt::color::red);
	inline constexpr auto Warning = fmt::fg(fmt::color::golden_rod);
	inline constexpr auto Info = fmt::fg(fmt::color::light_gray);
	inline constexpr auto Skipping = fmt::fg(fmt::color::gray);
	inline constexpr auto Name = fmt::emphasis::underline | fmt::fg(fmt::color::cyan);
	inline constexpr auto Action = fmt::fg(fmt::color::sky_blue);
}

namespace chrono = std::chrono;
namespace filesystem = std::filesystem;
namespace ranges = std::ranges;

using std::fstream;
using std::ios;
using std::map;
using std::ofstream;
using std::string;
using std::string_view;

struct CLocalizationInfo final	// #UPDATE_AT_CPP26 compile-time string.
{
	string_view m_FileSuffix{};
	string_view m_LocalizationFolder{};
	string_view m_YamlFieldKey{};
};

inline const map<string_view, CLocalizationInfo, std::less<>> g_rgszLocalizationInfo
{
	{ "english",      { "_l_english.yml", "english\\", "l_english:" } },
	{ "french",       { "_l_french.yml", "french\\", "l_french:" } },
	{ "german",       { "_l_german.yml", "german\\", "l_german:" } },
	{ "japanese",     { "_l_japanese.yml", "japanese\\", "l_japanese:" } },
	{ "korean",       { "_l_korean.yml", "korean\\", "l_korean:" } },
	{ "polish",       { "_l_polish.yml", "polish\\", "l_polish:" } },
	{ "russian",      { "_l_russian.yml", "russian\\", "l_russian:" } },
	{ "simp_chinese", { "_l_simp_chinese.yml", "simp_chinese\\", "l_simp_chinese:" } },
	{ "spanish",      { "_l_spanish.yml", "spanish\\", "l_spanish:" } },
};

static string_view UTIL_LanguageForShort(char c) noexcept
{
	switch (c)
	{
	case 'f':
		return "french";

	case 'g':
		return "german";

	case 'j':
		return "japanese";

	case 'k':
		return "korean";

	case 'p':
		return "polish";

	case 'r':
		return "russian";

	case 'c':
		return "simp_chinese";

	case 's':
		return "spanish";

	default:
		fmt::print(Style::Error, "Unknown language code '{}'!\n", c);	// don't input random stuff, thanks for your cooperation.
		[[fallthrough]];

	case 'e':
		return "english";
	}
}

static constexpr void UTIL_ReplaceAll(string* str, string_view from, string_view to) noexcept
{
	if (from.empty())
		return;

	size_t start_pos = 0;
	while ((start_pos = str->find(from, start_pos)) != string::npos)
	{
		str->replace(start_pos, from.length(), to);
		start_pos += to.length();	// In case 'to' contains 'from', like replacing 'x' with 'yx'
	}
}

static void UTIL_Trim(string* str) noexcept
{
	static constexpr auto fnNotSpace =
		[](unsigned char c) static noexcept { return !std::isspace(c); };

	str->erase(str->begin(), ranges::find_if(*str, fnNotSpace));	// L trim
	str->erase(ranges::find_if(str->rbegin(), str->rend(), fnNotSpace).base(), str->end());	// R trim. std::reverse_iterator<Iter>::base() represents the true position of reversed iterator.
}

static auto UTIL_GetSpaceCount(string_view str) noexcept // L space
{
	static constexpr auto fnNotSpace =
		[](unsigned char c) static noexcept { return !std::isspace(c); };

	return std::distance(str.begin(), ranges::find_if(str, fnNotSpace));
}

static auto LoadYAMLIntoMap(const filesystem::path& hPath, auto* m) noexcept
{
	decltype(UTIL_GetSpaceCount("")) iResult = 0;

	if (fstream hFileStream(hPath, ios::in); hFileStream)
	{
		while (!hFileStream.eof())
		{
			string szBuffer;
			std::getline(hFileStream, szBuffer);

			if (!iResult)
				iResult = UTIL_GetSpaceCount(szBuffer);	// Gets space counts before trimming.

			UTIL_Trim(&szBuffer);

			if (szBuffer.starts_with('#'))	// Skip the commented line in original texts.
				continue;

			if (szBuffer.ends_with(':'))	// Skip the first line, like "l_english:" stuff.
				continue;

			if (szBuffer.empty())
				continue;

			m->try_emplace(
				szBuffer.substr(0U, szBuffer.find_first_of(':')),
				std::move(szBuffer)
			);
		}
	}

	return iResult;
}

static void CreatePlaceholder(const filesystem::path& mod, string_view from, string_view to) noexcept
{
	string szBuffer;

	for (const auto& hPath : filesystem::recursive_directory_iterator(mod / "localization" / from))	// "english"
	{
		auto& InfoOfSource = g_rgszLocalizationInfo.at(from);
		auto& InfoOfDest = g_rgszLocalizationInfo.at(to);

		if (hPath.is_directory() || !hPath.path().u8string().ends_with(InfoOfSource.m_FileSuffix))	// "_l_english.yml"
			continue;

		auto szOutputFilePath = hPath.path().u8string();
		UTIL_ReplaceAll(&szOutputFilePath, InfoOfSource.m_FileSuffix, InfoOfDest.m_FileSuffix);	// "_l_english.yml", "_l_simp_chinese.yml"
		UTIL_ReplaceAll(&szOutputFilePath, InfoOfSource.m_LocalizationFolder, InfoOfDest.m_LocalizationFolder);	// "english\\", "simp_chinese\\"

		auto hOutputFilePath = filesystem::path(szOutputFilePath);

		if (!filesystem::exists(hOutputFilePath))
		{
			if (fstream hSourceStream(hPath.path(), ios::in); hSourceStream)
			{
				if (auto hOutputFileDir = hOutputFilePath.parent_path(); !filesystem::exists(hOutputFileDir))
				{
					filesystem::create_directories(hOutputFileDir);	// Use create_directories() to create entire folder structure.
					fmt::print(Style::Info, "Folder Created: {}\n", filesystem::relative(hOutputFileDir, mod.parent_path()).u8string());
				}

				ofstream output_file(hOutputFilePath, ios::out);

				if (!output_file)
					continue;

				while (!hSourceStream.eof())
				{
					std::getline(hSourceStream, szBuffer);
					UTIL_ReplaceAll(&szBuffer, InfoOfSource.m_YamlFieldKey, InfoOfDest.m_YamlFieldKey);	// "l_english:", "l_simp_chinese:"

					if (szBuffer.length())	// Skip empty lines.
						output_file << szBuffer << std::endl;
				}

				fmt::print(Style::Info, "Created: {}\n", filesystem::relative(hOutputFilePath, mod).u8string());
			}
			else
				fmt::print(Style::Error, "Error: Could not open source file: {}\n", filesystem::relative(hPath, mod).u8string());
		}
		else	// i.e. Need to compare with the source one.
		{
			bool bFirstInserted = false;
			map<string, string, std::less<>> mSource, mOutput;	// #UPDATE_AT_CPP23 flat_map

			LoadYAMLIntoMap(hPath.path(), &mSource);
			auto const iSpaceCounts = LoadYAMLIntoMap(hOutputFilePath, &mOutput);
			auto const szSpaces = string(iSpaceCounts, ' ');

			if (ofstream hOutputStream(hOutputFilePath, ios::out | ios::app); hOutputStream)	// std::ios::app means output this file with append mode. This will not have the original one deleted.
			{
				for (auto& [szKey, szValue] : mSource)
				{
					if (mOutput.contains(szKey))
						continue;

					if (!bFirstInserted)
					{
						fmt::print(
							hOutputStream, 
							"\n{}# Automatically added at: {:%Y-%b-%d}\n",
							szSpaces,
							fmt::gmtime(std::time((nullptr)))
						);

						bFirstInserted = true;
					}

					hOutputStream << szSpaces << szValue << std::endl;
					fmt::print(Style::Info, "Inserting: '{}' into file: {}\n", szKey, filesystem::relative(hOutputFilePath, mod).u8string());
				}

				if (!bFirstInserted)	// Nothing had inserted.
					fmt::print(Style::Skipping, "Skipping: {}\n", filesystem::relative(hOutputFilePath, mod).u8string());
			}
		}
	}
}

static auto GetModName(const filesystem::path& mod) noexcept -> string
{
	if (!atoi(mod.filename().u8string().c_str()))
		return mod.filename().u8string();	// It must be a mod with proper text name.

	const auto hDesc = mod / "descriptor.mod";

	if (!filesystem::exists(hDesc))
		return "ERROR - NO DESCRIPTOR FOUNDED!";

	string ret = "ERROR - NO DESCRIPTOR FOUNDED!";

	if (FILE* f = fopen(hDesc.u8string().c_str(), "rb"); f != nullptr)
	{
		fseek(f, 0, SEEK_END);
		const auto iLength = ftell(f);
		char* pbuf = (char*)calloc(iLength + 1, sizeof(char));
		const char* pend = pbuf + iLength + 1;

		fseek(f, 0, SEEK_SET);
		fread(pbuf, sizeof(char), iLength, f);
		fclose(f);
		f = nullptr;

		for (const char* p = pbuf; p != pend; ++p)
		{
			if (_strnicmp(p, "name", 4) == 0)	// Must use strnicmp instead of strcmp. The later have '\0' included.
			{
				for (; p != pend; ++p)
				{
					if (*p == '"')
					{
						++p;
						break;
					}
				}

				ret = p;
				break;
			}
		}

		free(pbuf);
		ret.erase(ret.find_first_of('"'));
	}

	return ret;
}

int main(int argc, char* argv[]) noexcept
{
	std::ios_base::sync_with_stdio(false);	// We are not using C function "printf" thus just turn it off.

	const bool bBatchMode = !filesystem::exists("descriptor.mod");

	if (bBatchMode)
		fmt::print("\"{}\" no found in current folder.\nRunning in batch mode.\n", fmt::styled("descriptor.mod", Style::Name));
	else
		fmt::print("Mod detected: {}\n", fmt::styled(GetModName(filesystem::current_path()), Style::Name));
	fmt::println("===================================================================================");

	char from = 0, to = 0;
	string_view szFrom{}, szTo{};

	switch (argc)
	{
	case 3:
		to = *(argv[2]);
		[[fallthrough]];
	case 2:
		szFrom = UTIL_LanguageForShort(from = *(argv[1]));
		[[fallthrough]];
	default:
		break;
	}

	switch (argc)
	{
	case 3:
		goto LAB_ANALYZING_ARG;
	case 2:
		goto LAB_ASK_FOR_TO;
	default:
		break;
	}

LAB_ASK_FOR_FROM:;
	fmt::println("From what language?");
	std::cin >> from;
	szFrom = UTIL_LanguageForShort(from);

	if (!bBatchMode && !filesystem::exists(filesystem::current_path() / L"localization" / szFrom))
	{
		fmt::print(Style::Error, "Localization folder \"{}\" no found!\n", szFrom);
		goto LAB_ASK_FOR_FROM;
	}

	fmt::print("Generate from language: \"{}\"\n\n", szFrom);

LAB_ASK_FOR_TO:;
	fmt::println("To what language?");
	std::cin >> to;

	if (to == from)
	{
		fmt::print(Style::Error, "Same languague between source and target.\n");
		goto LAB_ASK_FOR_TO;
	}

LAB_ANALYZING_ARG:;

	const auto fnCreatePlaceholderForMod = [&](const filesystem::path& hModPath = filesystem::current_path()) noexcept
	{
		if (hModPath != filesystem::current_path())
		{
			fmt::print(Style::Action, "\nProcessing mod: ");
			fmt::print(Style::Name, "{}\n", GetModName(hModPath));
		}

		if (!filesystem::exists(hModPath / L"localization" / szFrom))
		{
			fmt::print(Style::Error, "Skipping: Source language \"{}\" no found.\n", szFrom);
			return;
		}

		if (to == 'a')	// ALL languages.
		{
			for (auto&& szKey : g_rgszLocalizationInfo | std::views::keys)
			{
				if (szKey == szFrom)	// Skipping original language.
					continue;

				fmt::print(Style::Action, "Generating placeholder for language \"");
				fmt::print(Style::Name, "{}", szKey);
				fmt::print(Style::Action, "\" ...\n");

				CreatePlaceholder(hModPath, szFrom, szKey);
				fmt::println("");
			}
		}
		else
		{
			szTo = UTIL_LanguageForShort(to);

			fmt::print(Style::Action, "Generating placeholder for language \"");
			fmt::print(Style::Name, "{}", szTo);
			fmt::print(Style::Action, "\" ...\n");

			CreatePlaceholder(hModPath, szFrom, szTo);
			fmt::println("");
		}
	};

	if (bBatchMode)	// For all mods in this folder.
	{
		for (const auto& hEntry : filesystem::directory_iterator(filesystem::current_path()))
		{
			if (!hEntry.is_directory())
				continue;

			if (!filesystem::exists(hEntry.path() / "descriptor.mod"))	// Is this folder a mod?
			{
				fmt::print(Style::Warning, "Skipping non-mod folder: {}\n", fmt::styled(hEntry.path().filename().u8string(), Style::Name));
				continue;
			}

			if (!filesystem::exists(hEntry.path() / "localization"))	// Is this mod contains a localization folder?
			{
				fmt::print(Style::Warning, "Skipping non-localizable mod: {}\n", fmt::styled(GetModName(hEntry), Style::Name));
				continue;
			}

			fnCreatePlaceholderForMod(hEntry);
		}
	}
	else
		fnCreatePlaceholderForMod();

	fmt::println("");
	system("pause");
	return EXIT_SUCCESS;
}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
