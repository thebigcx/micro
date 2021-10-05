#include <locale.h>

static struct lconv __en_US_lconv =
{
    .decimal_point = ".",
    .thousands_sep = ",",
    .grouping = "\x03\x03",
    .int_curr_symbol = "$",
    .currency_symbol = "$",
    .mon_decimal_point = ".",
    .mon_thousands_sep = ",",
    .mon_grouping = "\x03\x03",
    .positive_sign = "+",
    .negative_sign = "-",
    .int_frac_digits = 127,
    .frac_digits = 127,
    .p_cs_precedes = 1,
    .p_sep_by_space = 1,
    .n_cs_precedes = 1,
    .n_sep_by_space = 1,
    .p_sign_posn = 127,
    .n_sign_posn = 127
};

struct lconv* localeconv()
{
    return &__en_US_lconv;
}