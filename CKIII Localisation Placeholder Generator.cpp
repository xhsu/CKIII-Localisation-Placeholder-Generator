// CKIII Localisation Placeholder Generator.cpp : This file contains the 'main' function. Program execution begins and ends there.
//


// C++ STL
#include <chrono>
#include <filesystem>
#include <iostream>
#include <fstream>
#include <string>
#include <unordered_map>

// MSVC STL modules
//import std.core;
//import std.filesystem;

using namespace std;

void UTIL_ReplaceAll(string& str, const string& from, const string& to)
{
	if (from.empty())
		return;

	size_t start_pos = 0;
	while ((start_pos = str.find(from, start_pos)) != string::npos)
	{
		str.replace(start_pos, from.length(), to);
		start_pos += to.length();	// In case 'to' contains 'from', like replacing 'x' with 'yx'
	}
}

void UTIL_Trim(string& str)
{
	static const auto fnNotSpace = [](unsigned char c) { return !std::isspace(c); };

	str.erase(str.begin(), find_if(str.begin(), str.end(), fnNotSpace));	// L trim
	str.erase(find_if(str.rbegin(), str.rend(), fnNotSpace).base(), str.end());	// R trim. std::reverse_iterator<Iter>::base() represents the true position of reversed iterator.
}

auto UTIL_GetSpaceCount(const string& str)	// L space
{
	static const auto fnNotSpace = [](unsigned char c) { return !std::isspace(c); };

	return distance(str.begin(), find_if(str.begin(), str.end(), fnNotSpace));
}

auto LoadYAMLIntoMap(const filesystem::path& hPath, unordered_map<string, string>& m)
{
	invoke_result_t<decltype(UTIL_GetSpaceCount), const string&> iResult = 0;

	if (fstream hFileStream(hPath, ios::in); hFileStream)
	{
		while (!hFileStream.eof())
		{
			string szBuffer;
			getline(hFileStream, szBuffer);

			if (!iResult)
				iResult = UTIL_GetSpaceCount(szBuffer);	// Gets space counts before trimming.

			UTIL_Trim(szBuffer);

			if (szBuffer.ends_with(':'))	// Skip the first line, like "l_english:" stuff.
				continue;

			size_t iBreakPos = szBuffer.find_first_of(':');
			m[szBuffer.substr(0U, iBreakPos)] = move(szBuffer);
		}
	}

	return iResult;
}

int main()
{
	string szBuffer;

	for (const auto& hPath : filesystem::recursive_directory_iterator("english"))
	{
		if (hPath.is_directory() || !hPath.path().string().ends_with("_l_english.yml"))
			continue;

		string szOutputFilePath = hPath.path().string();
		UTIL_ReplaceAll(szOutputFilePath, "_l_english.yml", "_l_simp_chinese.yml");
		UTIL_ReplaceAll(szOutputFilePath, "english\\", "simp_chinese\\");

		auto hOutputFilePath = filesystem::path(szOutputFilePath);

		if (!filesystem::exists(hOutputFilePath))
		{
			if (fstream hSourceStream(hPath.path(), ios::in); hSourceStream)
			{
				if (auto hOutputFileDir = hOutputFilePath.parent_path(); !filesystem::exists(hOutputFileDir))
				{
					filesystem::create_directories(hOutputFileDir);	// Use create_directories() to create entire folder structure.
					cout << "Folder Created: " << hOutputFileDir.string() << endl;
				}

				ofstream output_file(szOutputFilePath, ios::out);

				if (!output_file)
					continue;

				while (!hSourceStream.eof())
				{
					getline(hSourceStream, szBuffer);
					UTIL_ReplaceAll(szBuffer, "l_english:", "l_simp_chinese:");

					output_file << szBuffer << endl;
				}

				cout << "Created: " << szOutputFilePath << endl;
			}
			else
				cout << "Error: Could not open source file: " << hPath.path() << endl;
		}
		else	// i.e. Need to compare with the source one.
		{
			bool bFirstInserted = false;
			unordered_map<string, string> mSource, mOutput;

			LoadYAMLIntoMap(hPath.path(), mSource);
			auto iSpaceCounts = LoadYAMLIntoMap(hOutputFilePath, mOutput);
			const string szSpaces = string(iSpaceCounts, ' ');

			if (ofstream hOutputStream(szOutputFilePath, ios::out | ios::app); hOutputStream)	// std::ios::app means output this file with append mode. This will not have the original one deleted.
			{
				for (auto& [szKey, szValue] : mSource)
				{
					if (mOutput.contains(szKey))
						continue;

					if (!bFirstInserted)
					{
						auto hTimeNow =chrono::system_clock::to_time_t(chrono::system_clock::now());	// #FIXME MSVC C++20 have no std::chrono::operator<< (std::chrono::year_month_day)
						
						hOutputStream << endl << szSpaces << "# Automatically added at: " << ctime(&hTimeNow);	// end of line included.
						bFirstInserted = true;
					}

					hOutputStream << szSpaces << szValue << endl;
					cout << "Inserting: " << quoted(szKey) << " into file: " << szOutputFilePath << endl;
				}
			}
		}
	}

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
