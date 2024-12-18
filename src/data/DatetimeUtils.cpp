#include <iomanip>
#include <ctime>
#include <sstream>
#include <time.h>
#include "DatetimeUtils.hpp"

namespace FG::data
{
Timestamp datetimeToUnixTimestamp(const Datetime& dt)
{
    return std::chrono::time_point_cast<std::chrono::seconds>(dt).time_since_epoch().count();
}

Datetime unixTimestampToDatetime(const Timestamp ts)
{
    auto secsSinceEpoch = std::chrono::seconds(ts);
    return Datetime(std::chrono::duration_cast<Datetime::duration>(secsSinceEpoch));
}

Timestamp isoDateToTimestamp(std::string_view dtStr)
{
    std::tm tm{};
    std::stringstream ss;
    ss << dtStr;
    ss << "T00:00:00";
    ss >> std::get_time(&tm, "%Y-%m-%dT%H:%M:%S");
    if(ss.fail())
        throw std::runtime_error("Parsing date failed");
    return timegm(&tm);
}

Datetime parseIsoDate(std::string_view dtStr)
{
    return unixTimestampToDatetime(isoDateToTimestamp(dtStr));
}
}