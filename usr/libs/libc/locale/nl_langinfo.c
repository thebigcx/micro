#include <langinfo.h>

char* __en_US_langinfo[] =
{
    [CODESET] = "UTF-8",
    [D_T_FMT] = "%a %d %b %Y %r %Z",
    [D_FMT] = "%m/%d/%Y",
    [T_FMT] = "%r",
    [AM_STR] = "AM",
    [PM_STR] = "PM",
    [T_FMT_AMPM] = "%I:%M:%S %p",
    [ERA] = "",
    [ERA_D_T_FMT] = "",
    [ERA_D_FMT] = "",
    [ERA_T_FMT] = "",
    [DAY_1] = "Sunday",
    [DAY_2] = "Monday",
    [DAY_3] = "Tuesday",
    [DAY_4] = "Wednesday",
    [DAY_5] = "Thursday",
    [DAY_6] = "Friday",
    [DAY_7] = "Saturday",
    [ABDAY_1] = "Sun",
    [ABDAY_2] = "Mon",
    [ABDAY_3] = "Tue",
    [ABDAY_4] = "Wed",
    [ABDAY_5] = "Thu",
    [ABDAY_6] = "Fri",
    [ABDAY_7] = "Sat",
    [MON_1] = "January",
    [MON_2] = "February",
    [MON_3] = "March",
    [MON_4] = "April",
    [MON_5] = "May",
    [MON_6] = "June",
    [MON_7] = "July",
    [MON_8] = "August",
    [MON_9] = "September",
    [MON_10] = "October",
    [MON_11] = "November",
    [MON_12] = "December",
    [ABMON_1] = "Jan",
    [ABMON_2] = "Feb",
    [ABMON_3] = "Mar",
    [ABMON_4] = "Apr",
    [ABMON_5] = "May",
    [ABMON_6] = "Jun",
    [ABMON_7] = "Jul",
    [ABMON_8] = "Aug",
    [ABMON_9] = "Sep",
    [ABMON_10] = "Oct",
    [ABMON_11] = "Nov",
    [ABMON_12] = "Dec",
    [RADIXCHAR] = ".",
    [THOUSEP] = ",",
    [YESEXPR] = "^[+1yY]",
    [NOEXPR] = "^[-0nN]",
    [CRNCYSTR] = ""
};

char* nl_langinfo(nl_item item)
{
    return __en_US_langinfo[item];
}