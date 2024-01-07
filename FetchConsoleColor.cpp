#include <Windows.h>

#include <fmt/core.h>

void FetchConsoleColor() noexcept
{
	static HANDLE s_hConsole{};
	static CONSOLE_SCREEN_BUFFER_INFOEX info{ .cbSize = sizeof(CONSOLE_SCREEN_BUFFER_INFOEX) };

	[[unlikely]]
	if (!s_hConsole)
		s_hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

	GetConsoleScreenBufferInfoEx(s_hConsole, &info);

	for (auto&& Color : info.ColorTable)
	{
		auto const R = 0x0000FF & Color;
		auto const G = (0x00FF00 & Color) >> 8;
		auto const B = (0xFF0000 & Color) >> 16;
		//fmt::print("BGR: 0x{:0>6X}: {} {} {}\t#{:0>6X}\n", Color, R, G, B, (R << 16) | (G << 8) | B);
		fmt::print("0x{:0>6X},\n", (R << 16) | (G << 8) | B);
	}
}