#include <time.h>
#include <string.h>
#include <langinfo.h>
#include <stdlib.h>

size_t strftime(char* s, size_t max,
                const char* format,
				const struct tm* tm)
{
	size_t i = 0;
	char buf[128];

	while (*format != 0)
	{
		if (i + 1 >= max) return 0;

		if (*format != '%')
		{
			s[i++] = *format++;
		}
		else
		{
			switch (*++format)
			{
				case '%':
					s[i++] = '%';
					break;

				case 'a':
				{
					const char* day = nl_langinfo(ABDAY_1 + tm->tm_wday);
					strncpy(s + i, day, strlen(day));
					i += strlen(day);
					break;
				}

				case 'b':
				{
					const char* mon = nl_langinfo(ABMON_1 + tm->tm_mon);
					strncpy(s + i, mon, strlen(mon));
					i += strlen(mon);
					break;
				}

				case 'd':
				{
					itoa(tm->tm_mday, buf, 10);
					strcpy(s + i, buf);
					i += strlen(buf);
					break;
				}

				case 'H':
				{
					itoa(tm->tm_hour, buf, 10);
					strcpy(s + i, buf);
					i += strlen(buf);
					break;
				}

				case 'M':
				{
					itoa(tm->tm_min, buf, 10);
					strcpy(s + i, buf);
					i += strlen(buf);
					break;
				}

				case 'S':
				{
					itoa(tm->tm_sec, buf, 10);
					strcpy(s + i, buf);
					i += strlen(buf);
					break;
				}

				case 'Y':
				{
					itoa(tm->tm_year, buf, 10);
					strcpy(s + i, buf);
					i += strlen(buf);
					break;
				}
					
				default:
					break;
			}

			format++;
		}
	}

	return i;
}