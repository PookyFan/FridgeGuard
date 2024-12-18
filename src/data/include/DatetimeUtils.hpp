#pragma once

#include <chrono>
#include <string_view>

namespace FG::data
{
using Datetime = std::chrono::time_point<std::chrono::system_clock>;
using Timestamp = Datetime::duration::rep;

Timestamp datetimeToUnixTimestamp(const Datetime& dt);

Datetime unixTimestampToDatetime(const Timestamp ts);

Timestamp isoDateToTimestamp(std::string_view dtStr);

Datetime parseIsoDate(std::string_view dtStr);
}