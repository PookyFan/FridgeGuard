#include <tuple>
#include <utility>

#include <gtest/gtest.h>
#include "DatetimeUtils.hpp"

using namespace testing;
using namespace std::chrono_literals;

namespace
{
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wliteral-suffix"
constexpr std::chrono::month operator""m(unsigned long long m) noexcept
{
    return std::chrono::month(static_cast<int>(m));
}
#pragma GCC diagnostic pop
}

namespace FG::data::test
{
using YMD = std::chrono::year_month_day;
using Str = std::string_view;
using DateFormats = std::tuple<Str, Timestamp, Datetime>;

auto makeDates(Str&& iso, Timestamp&& ts, YMD&& ymd)
{
    auto dt = std::chrono::sys_days(ymd);
    return std::make_tuple<Str, Timestamp, Datetime>(std::move(iso), std::move(ts), std::move(dt));
}

struct DatetimeUtilsTestFixture : public TestWithParam<DateFormats>
{};

TEST_P(DatetimeUtilsTestFixture, datetimeToUnixTimestampShouldReturnValidUnixTimestampsForDatesFollowingEpoch)
{
    auto expected = std::get<Timestamp>(GetParam());
    auto actual = datetimeToUnixTimestamp(std::get<Datetime>(GetParam()));
    ASSERT_EQ(expected, actual);
}

TEST_P(DatetimeUtilsTestFixture, unixTimestampToDatetimeShouldReturnValidDatetimeObjecsForDatesFollowingEpoch)
{
    auto expected = std::get<Datetime>(GetParam());
    auto actual = unixTimestampToDatetime(std::get<Timestamp>(GetParam()));
    ASSERT_EQ(expected, actual);
}

TEST_P(DatetimeUtilsTestFixture, parseIsoDateShouldReturnValidDatetimeObjecsForDatesFollowingEpoch)
{
    auto expected = std::get<Datetime>(GetParam());
    auto actual = parseIsoDate(std::get<Str>(GetParam()));
    ASSERT_EQ(expected, actual);
}

INSTANTIATE_TEST_SUITE_P(DatetimeUtilsTest, DatetimeUtilsTestFixture, Values(
    makeDates("1970-01-01", 0,           YMD(1970y,  1m, 1d)),
    makeDates("2024-11-17", 1731801600,  YMD(2024y, 11m, 17d)),
    makeDates("2038-01-19", 2147472000,  YMD(2038y,  1m, 19d)),
    makeDates("2106-02-07", 4294944000,  YMD(2106y,  2m,  7d))
));
}