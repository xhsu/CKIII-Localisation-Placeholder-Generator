// CKIII Localisation Placeholder Generator.cpp : This file contains the 'main' function. Program execution begins and ends there.
//


// C++ STL
#include <filesystem>
#include <iostream>
#include <fstream>
#include <string>

// MSVC STL modules
//import std.core;
//import std.filesystem;

using namespace std;

void UTIL_Replace(string& str, const string& from, const string& to)	// Replace all.
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

int main()
{
	string szBuffer;

	for (const auto& hPath : filesystem::recursive_directory_iterator("english"))
	{
		if (hPath.is_directory() || hPath.path().string().find("_l_english.yml") == string::npos)
			continue;

		string szDummyFilePath = hPath.path().string();
		UTIL_Replace(szDummyFilePath, "_l_english.yml", "_l_simp_chinese.yml");
		UTIL_Replace(szDummyFilePath, "english\\", "simp_chinese\\");

		auto hDummyFilePath = filesystem::path(szDummyFilePath);

		if (filesystem::exists(hDummyFilePath))
		{
			cout << "Skipping: " << szDummyFilePath << endl;
			continue;
		}

		if (fstream input_file(hPath.path(), ios::in); input_file)
		{
			if (auto hDummyFileDir = hDummyFilePath.parent_path(); !filesystem::exists(hDummyFileDir))
			{
				filesystem::create_directories(hDummyFileDir);
				cout << "Folder Created: " << hDummyFileDir.string() << endl;
			}

			ofstream output_file(szDummyFilePath, ios::out);

			if (!output_file)
				continue;

			while (!input_file.eof())
			{
				getline(input_file, szBuffer);
				UTIL_Replace(szBuffer, "l_english:", "l_simp_chinese:");

				output_file << szBuffer << endl;
			}

			cout << "Created: " << szDummyFilePath << endl;
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
