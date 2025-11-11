module;

#define HYDROGENIUM_APPLICATION_VERMGR_VER 20250603L

#ifdef __INTELLISENSE__
#include <ranges>
#endif

export module Application;

import std;

using std::uint8_t;
using std::uint32_t;
using std::int32_t;

namespace Hydrogenium::appl_details
{
	template <std::intmax_t N>
	consteval auto int_to_string()
	{
		auto buflen =
			[]()
			{
				uint8_t len = N > 0 ? 1 : 2;
				for (auto n = N; n; ++len, n /= 10) {}

				return len;
			};

		std::array<char, buflen()> buf{};

		auto ptr = buf.end();
		*--ptr = '\0';

		if constexpr (N != 0)
		{
			for (auto n = N; n; n /= 10)
				*--ptr = "0123456789"[(N < 0 ? -1 : 1) * (n % 10)];

			if constexpr (N < 0)
				*--ptr = '-';
		}
		else
			buf[0] = '0';

		return buf;
	}

	template <size_t... Ns>
	consteval auto c_strcat(std::array<char, Ns> const&... args)
	{
		std::array<char, (... + Ns) - sizeof...(Ns) + 1> buf{};

		auto append =
			[ptr = buf.begin()](auto&& s) mutable
			{
				for (auto c : s)
				{
					if (c == '\0')
						break;

					*ptr++ = c;
				}
			};

		(append(args), ...);
		return buf;
	}
}

constexpr auto LocalBuildNumber(std::string_view COMPILE_DATE = __DATE__, int32_t bias = 0) noexcept
{
	constexpr std::string_view mon[12] =
	{ "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };
	constexpr uint8_t mond[12] =
	{ 31,    28,    31,    30,    31,    30,    31,    31,    30,    31,    30,    31 };

	// #UPDATE_AT_CPP23 P2647R1
	const auto today_m = COMPILE_DATE.substr(0, 3);
	const auto today_d = (COMPILE_DATE[5] - '0') + (COMPILE_DATE[4] != ' ' ? (COMPILE_DATE[4] - '0') : 0) * 10;
	const auto today_y = (COMPILE_DATE[10] - '0') + (COMPILE_DATE[9] - '0') * 10 + (COMPILE_DATE[8] - '0') * 100 + (COMPILE_DATE[7] - '0') * 1000;

	const auto this_leap = std::chrono::year{ today_y }.is_leap();

	const auto m = std::ranges::find(mon, today_m) - std::ranges::begin(mon) + 1;	// "Jan" at index 0
	const auto d = std::ranges::fold_left(mond | std::views::take(m - 1), today_d - (this_leap ? 0 : 1), std::plus<>{});
	const auto y = today_y - 1900;

	return
		d
		// the rounding down is actually dropping the years before next leap.
		// hence the averaged 0.25 wont affect the accurate value.
		// Gregorian additional rule wont take effect in the next 100 years.
		// Let's adjust the algorithm then.
		+ static_cast<decltype(d)>((y - 1) * 365.25)
		- bias;
}

struct app_version_t final
{
	uint8_t m_major{};
	uint8_t m_minor{};
	uint8_t m_revision{};
	uint8_t m_build{};

	[[nodiscard]]
	std::string ToString() const noexcept
	{
		return std::format("{}.{}.{}.{}", m_major, m_minor, m_revision, m_build);
	}

	[[nodiscard]]
	constexpr uint32_t U32() const noexcept
	{
		return std::bit_cast<uint32_t>(*this);
	}

	[[nodiscard]]
	static constexpr app_version_t Parse(uint32_t i) noexcept
	{
		return std::bit_cast<app_version_t>(i);
	}

	[[nodiscard]]
	constexpr std::strong_ordering operator<=>(app_version_t rhs) const noexcept
	{
		if (this->m_major != rhs.m_major)
			return this->m_major <=> rhs.m_major;

		if (this->m_minor != rhs.m_minor)
			return this->m_minor <=> rhs.m_minor;

		if (this->m_revision != rhs.m_revision)
			return this->m_revision <=> rhs.m_revision;

		return this->m_build <=> rhs.m_build;
	}
};

static_assert(sizeof(app_version_t) == sizeof(uint32_t));

export inline constexpr auto APP_CRATED = LocalBuildNumber("Sep 01 2021");
export inline constexpr auto BUILD_NUMBER = LocalBuildNumber(__DATE__, APP_CRATED);

export inline constexpr app_version_t APP_VERSION
{
	.m_major = 1,
	.m_minor = 1,
	.m_revision = 0,
	.m_build = static_cast<uint8_t>(BUILD_NUMBER % 255),
};

export inline constexpr uint32_t APP_VERSION_COMPILED = APP_VERSION.U32();

namespace Hydrogenium::appl_details
{
	inline constexpr std::array dot{ '.', '\0' };

	inline constexpr auto ver = c_strcat(
		int_to_string<APP_VERSION.m_major>(), dot,
		int_to_string<APP_VERSION.m_minor>(), dot,
		int_to_string<APP_VERSION.m_revision>(), dot,
		int_to_string<BUILD_NUMBER>()
	);
}

export inline constexpr std::string_view APP_VERSION_STRING{ Hydrogenium::appl_details::ver.begin(), Hydrogenium::appl_details::ver.end() - 1 };	// Remove the '\0'
