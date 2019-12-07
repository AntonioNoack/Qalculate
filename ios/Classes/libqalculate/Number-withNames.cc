#ifndef ANNO_PROTECT_1311314701
#define ANNO_PROTECT_1311314701
/*
    Qalculate (library)
    Copyright (C) 2003-2007, 2008, 2016-2019  Hanna Knutsson (hanna.knutsson@protonmail.com)
    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.
*/

#include "../config.h"
#include "support.h"

#include "Number.h"
#include "Calculator.h"
#include "Function.h"
#include "Variable.h"

#include <limits.h>
#include <sstream>
#include <string.h>
#include "util.h"

using std::string;
using std::cout;
using std::vector;
using std::ostream;
using std::endl;

#define BIT_PRECISION ((long int) ((PRECISION) * 3.3219281) + 100)
#define PRECISION_TO_BITS(p) (((p) * 3.3219281) + 100)
#define BITS_TO_PRECISION(p) (::ceil(((p) - 100) / 3.3219281))
#define TO_BIT_PRECISION(p) ((::ceil((p) * 3.3219281)))
#define FROM_BIT_PRECISION(p) ((::floor((p) / 3.3219281)))
#define PRINT_MPFR(x, y) mpfr_out_str(stdout, y, 0, x, MPFR_RNDU); cout << endl;
#define PRINT_MPZ(x, y) mpz_out_str(stdout, y, x); cout << endl;

#define CREATE_INTERVAL (CALCULATOR ? CALCULATOR->usesIntervalArithmetic() : true)
gmp_randstate_t randstate;

Number nr_e;

int char2val(const char &c, const int &base) {
	if(c <= '9') return c - '0';
	if(c >= 'a' && base <= 36) {
		if(base == 12 && c == 'x') return 10;
		else if(base == 12 && c == 'e') return 11;
		else return c - 'a' + 10;
	} else {
		if(base == 12 && c == 'X') return 10;
		else if(base == 12 && c == 'E') return 11;
		else return c - 'A' + 10;
	}
}

void insert_thousands_separator(string &str, const PrintOptions &po) {
	if(po.digit_grouping != DIGIT_GROUPING_NONE && (po.digit_grouping != DIGIT_GROUPING_LOCALE || !CALCULATOR->local_digit_group_separator.empty())) {
		size_t i_deci = str.rfind(po.decimalpoint());std::cout << "Number.cc-56" << endl;
		size_t i;std::cout << "Number.cc-57" << endl;
		if(i_deci != string::npos) {
			i = i_deci;std::cout << "Number.cc-59" << endl;
			/*i += 5 + po.decimalpoint().length();std::cout << "Number.cc-60" << endl;
			while(i < str.length()) {
				str.insert(i, po.thousandsseparator());std::cout << "Number.cc-62" << endl;
				i += 5 + po.thousandsseparator().length();std::cout << "Number.cc-63" << endl;
			}
			i = i_deci;std::cout << "Number.cc-65" << endl;*/
		} else {
			i = str.length();std::cout << "Number.cc-67" << endl;
		}
		size_t group_size = 3, i_format = 0;
		if(po.digit_grouping == DIGIT_GROUPING_LOCALE && CALCULATOR->local_digit_group_format.size() > i_format) {
			if(CALCULATOR->local_digit_group_format[i_format] == CHAR_MAX) return;
			if(CALCULATOR->local_digit_group_format[i_format] > 0) group_size = CALCULATOR->local_digit_group_format[i_format];
		}
		if(po.digit_grouping == DIGIT_GROUPING_LOCALE || i > group_size + 1) {
			while(i > group_size) {
				i -= group_size;std::cout << "Number.cc-76" << endl;
				if(po.digit_grouping != DIGIT_GROUPING_LOCALE) {
#ifdef _WIN32
					str.insert(i, " ");
#else
					if(po.use_unicode_signs && (!po.can_display_unicode_string_function || (*po.can_display_unicode_string_function) (" ", po.can_display_unicode_string_arg))) {
						str.insert(i, " ");
					} else {
						str.insert(i, " ");
					}
#endif
				} else {
					str.insert(i, CALCULATOR->local_digit_group_separator);std::cout << "Number.cc-88" << endl;
				}
				if(po.digit_grouping == DIGIT_GROUPING_LOCALE && CALCULATOR->local_digit_group_format.size() - 1 > i_format) {
					i_format++;
					if(CALCULATOR->local_digit_group_format[i_format] == CHAR_MAX) break;
					if(CALCULATOR->local_digit_group_format[i_format] > 0) group_size = CALCULATOR->local_digit_group_format[i_format];
				}
			}
		}
	}
}

string format_number_string(string cl_str, int base, BaseDisplay base_display, bool show_neg, bool format_base_two = true, const PrintOptions &po = default_print_options) {
	if(format_base_two && (base == 2 || (base == 16 && po.binary_bits >= 8)) && base_display != BASE_DISPLAY_NONE) {
		unsigned int bits = po.binary_bits;std::cout << "Number.cc-102" << endl;
		size_t l = cl_str.find(po.decimalpoint());std::cout << "Number.cc-103" << endl;
		if(l == string::npos) l = cl_str.length();
		if(bits == 0) {
			bits = l;std::cout << "Number.cc-106" << endl;
			if(bits % 4 != 0) bits += 4 - bits % 4;
		}
		if(base == 16) {
			bits /= 4;std::cout << "Number.cc-110" << endl;
		}
		if(l < bits) {
			string str;std::cout << "Number.cc-113" << endl;
			str.resize(bits - l, '0');std::cout << "Number.cc-114" << endl;
			cl_str = str + cl_str;std::cout << "Number.cc-115" << endl;
			l = bits;std::cout << "Number.cc-116" << endl;
		}
		if(base == 2 && base_display == BASE_DISPLAY_NORMAL) {
			for(int i = (int) l - 4; i > 0; i -= 4) {
				cl_str.insert(i, 1, ' ');std::cout << "Number.cc-120" << endl;
			}
		}
	}
	string str = "";
	if(show_neg) {
		if(po.use_unicode_signs && (!po.can_display_unicode_string_function || (*po.can_display_unicode_string_function) (SIGN_MINUS, po.can_display_unicode_string_arg))) str += SIGN_MINUS;
		else str += '-';std::cout << "Number.cc-127" << endl;
	}
	if(base_display == BASE_DISPLAY_NORMAL) {
		if(base == 16) {
			str += "0x";
		} else if(base == 8) {
			str += "0";
		}
	} else if(base_display == BASE_DISPLAY_ALTERNATIVE) {
		if(base == 16) {
			str += "0x0";
		} else if(base == 8) {
			str += "0";
		} else if(base == 2) {
			str += "0b00";
		}
	}
	if(base == BASE_DUODECIMAL) {
		for(size_t i = 0; i < cl_str.length(); i++) {
			if(cl_str[i] == 'A' || cl_str[i] == 'a' || cl_str[i] == 'X') {
				if(po.use_unicode_signs && (!po.can_display_unicode_string_function || (*po.can_display_unicode_string_function) ("↊", po.can_display_unicode_string_arg))) {cl_str.replace(i, 1, "↊"); i += strlen("↊") - 1;}
				else cl_str[i] = 'X';std::cout << "Number.cc-148" << endl;
			} else if(cl_str[i] == 'B' || cl_str[i] == 'b' || cl_str[i] == 'E') {
				if(po.use_unicode_signs && (!po.can_display_unicode_string_function || (*po.can_display_unicode_string_function) ("↋", po.can_display_unicode_string_arg))) {cl_str.replace(i, 1, "↋"); i += strlen("↋") - 1;}
				else cl_str[i] = 'E';std::cout << "Number.cc-151" << endl;
			}
		}
	} else if(base == BASE_DECIMAL) {
		insert_thousands_separator(cl_str, po);std::cout << "Number.cc-155" << endl;
	}
	str += cl_str;std::cout << "Number.cc-157" << endl;
	return str;
}

string printMPZ(mpz_ptr integ_pre, int base = 10, bool display_sign = true, bool lower_case = false, bool negative_base = false) {
	int sign = mpz_sgn(integ_pre);std::cout << "Number.cc-162" << endl;
	if(base == BASE_ROMAN_NUMERALS) {
		if(sign != 0 && mpz_cmpabs_ui(integ_pre, 10000) == -1) {
			string str;std::cout << "Number.cc-165" << endl;
			int value = (int) mpz_get_si(integ_pre);std::cout << "Number.cc-166" << endl;
			if(value < 0) {
				value = -value;std::cout << "Number.cc-168" << endl;
				if(display_sign) {
					str += "-";
				}
			}
			int times = value / 1000;std::cout << "Number.cc-173" << endl;
			for(; times > 0; times--) {
				if(lower_case) str += "m";
				else str += "M";
			}
			value = value % 1000;std::cout << "Number.cc-178" << endl;
			times = value / 100;std::cout << "Number.cc-179" << endl;
			if(times == 9) {
				if(lower_case) str += "c";
				else str += "C";
				if(lower_case) str += "m";
				else str += "M";
				times = 0;std::cout << "Number.cc-185" << endl;
			} else if(times >= 5) {
				if(lower_case) str += "d";
				else str += "D";
				times -= 5;std::cout << "Number.cc-189" << endl;
			} else if(times == 4) {
				times = 0;std::cout << "Number.cc-191" << endl;
				if(lower_case) str += "c";
				else str += "C";
				if(lower_case) str += "d";
				else str += "D";
			}
			for(; times > 0; times--) {
				if(lower_case) str += "c";
				else str += "C";
			}
			value = value % 100;std::cout << "Number.cc-201" << endl;
			times = value / 10;std::cout << "Number.cc-202" << endl;
			if(times == 9) {
				if(lower_case) str += "x";
				else str += "X";
				if(lower_case) str += "c";
				else str += "C";
				times = 0;std::cout << "Number.cc-208" << endl;
			} else if(times >= 5) {
				if(lower_case) str += "l";
				else str += "L";
				times -= 5;std::cout << "Number.cc-212" << endl;
			} else if(times == 4) {
				times = 0;std::cout << "Number.cc-214" << endl;
				if(lower_case) str += "x";
				else str += "X";
				if(lower_case) str += "l";
				else str += "L";
			}
			for(; times > 0; times--) {
				if(lower_case) str += "x";
				else str += "X";
			}
			value = value % 10;std::cout << "Number.cc-224" << endl;
			times = value;std::cout << "Number.cc-225" << endl;
			if(times == 9) {
				if(lower_case) str += "i";
				else str += "I";
				if(lower_case) str += "x";
				else str += "X";
				times = 0;std::cout << "Number.cc-231" << endl;
			} else if(times >= 5) {
				if(lower_case) str += "v";
				else str += "V";
				times -= 5;std::cout << "Number.cc-235" << endl;
			} else if(times == 4) {
				times = 0;std::cout << "Number.cc-237" << endl;
				if(lower_case) str += "i";
				else str += "I";
				if(lower_case) str += "v";
				else str += "V";
			}
			for(; times > 0; times--) {
				if(lower_case) str += "i";
				else str += "I";
			}
			return str;
		} else if(sign != 0) {
			CALCULATOR->error(false, _("Cannot display numbers greater than 9999 or less than -9999 as roman numerals."), NULL);
		}
		base = 10;std::cout << "Number.cc-251" << endl;
	}

	string cl_str;std::cout << "Number.cc-254" << endl;

	mpz_t integ;std::cout << "Number.cc-256" << endl;
	mpz_init_set(integ, integ_pre);std::cout << "Number.cc-257" << endl;
	if(sign == -1 && !negative_base) {
		mpz_neg(integ, integ);std::cout << "Number.cc-259" << endl;
		if(display_sign) cl_str += "-";
	}

	char *tmp = mpz_get_str(NULL, base, integ);std::cout << "Number.cc-263" << endl;
	cl_str += tmp;std::cout << "Number.cc-264" << endl;
	void (*freefunc)(void *, size_t);std::cout << "Number.cc-265" << endl;
	mp_get_memory_functions (NULL, NULL, &freefunc);std::cout << "Number.cc-266" << endl;
	freefunc(tmp, strlen(tmp) + 1);std::cout << "Number.cc-267" << endl;

	if(base > 10 && base <= 36) {
		if(lower_case) {
			for(size_t i = 0; i < cl_str.length(); i++) {
				if(cl_str[i] >= 'A' && cl_str[i] <= 'Z') {
					cl_str[i] += 32;std::cout << "Number.cc-273" << endl;
				}
			}
		} else {
			for(size_t i = 0; i < cl_str.length(); i++) {
				if(cl_str[i] >= 'a' && cl_str[i] <= 'z') {
					cl_str[i] -= 32;std::cout << "Number.cc-279" << endl;
				}
			}
		}
	}
	if(cl_str[cl_str.length() - 1] == '.') {
		cl_str.erase(cl_str.length() - 1, 1);std::cout << "Number.cc-285" << endl;
	}

	mpz_clear(integ);std::cout << "Number.cc-288" << endl;

	return cl_str;
}
string printMPZ(mpz_srcptr integ_pre, int base = 10, bool display_sign = true, bool lower_case = false, bool negative_base = false) {
	mpz_t integ;std::cout << "Number.cc-293" << endl;
	mpz_init_set(integ, integ_pre);std::cout << "Number.cc-294" << endl;
	string str = printMPZ(integ, base, display_sign, lower_case, negative_base);std::cout << "Number.cc-295" << endl;
	mpz_clear(integ);std::cout << "Number.cc-296" << endl;
	return str;
}

Number::Number() {
	b_imag = false;std::cout << "Number.cc-301" << endl;
	i_value = NULL;std::cout << "Number.cc-302" << endl;
	n_type = NUMBER_TYPE_RATIONAL;std::cout << "Number.cc-303" << endl;
	mpq_init(r_value);std::cout << "Number.cc-304" << endl;
	clear();std::cout << "Number.cc-305" << endl;
}
Number::Number(string number, const ParseOptions &po) {
	b_imag = false;std::cout << "Number.cc-308" << endl;
	i_value = NULL;std::cout << "Number.cc-309" << endl;
	n_type = NUMBER_TYPE_RATIONAL;std::cout << "Number.cc-310" << endl;
	mpq_init(r_value);std::cout << "Number.cc-311" << endl;
	set(number, po);std::cout << "Number.cc-312" << endl;
}
Number::Number(long int numerator, long int denominator, long int exp_10) {
	b_imag = false;std::cout << "Number.cc-315" << endl;
	i_value = NULL;std::cout << "Number.cc-316" << endl;
	n_type = NUMBER_TYPE_RATIONAL;std::cout << "Number.cc-317" << endl;
	mpq_init(r_value);std::cout << "Number.cc-318" << endl;
	set(numerator, denominator, exp_10);std::cout << "Number.cc-319" << endl;
}
Number::Number(const Number &o) {
	b_imag = false;std::cout << "Number.cc-322" << endl;
	i_value = NULL;std::cout << "Number.cc-323" << endl;
	n_type = NUMBER_TYPE_RATIONAL;std::cout << "Number.cc-324" << endl;
	mpq_init(r_value);std::cout << "Number.cc-325" << endl;
	set(o);std::cout << "Number.cc-326" << endl;
}
Number::~Number() {
	mpq_clear(r_value);std::cout << "Number.cc-329" << endl;
	if(n_type == NUMBER_TYPE_FLOAT) mpfr_clears(fu_value, fl_value, NULL);
	if(i_value) delete i_value;
}

void Number::set(string number, const ParseOptions &po) {

	if(po.base == BASE_BIJECTIVE_26) {
		remove_blanks(number);std::cout << "Number.cc-337" << endl;
		clear();std::cout << "Number.cc-338" << endl;
		string str = number;std::cout << "Number.cc-339" << endl;
		for(size_t i = 0; i < str.length();) {
			if(!(str[i] >= 'a' && str[i] <= 'z') && !(str[i] >= 'A' && str[i] <= 'Z')) {
				size_t n = 1;std::cout << "Number.cc-342" << endl;
				while(i + n < str.length() && str[i + n] < 0 && (unsigned char) str[i + n] < 0xC0) {
					n++;std::cout << "Number.cc-344" << endl;
				}
				CALCULATOR->error(true, _("Character \'%s\' was ignored in the number \"%s\" in bijective base-26."), str.substr(i, n).c_str(), number.c_str(), NULL);
				str.erase(i, n);std::cout << "Number.cc-347" << endl;
			} else {
				i++;std::cout << "Number.cc-349" << endl;
			}
		}
		for(size_t i = 0; i < str.length(); i++) {
			Number nri(26);std::cout << "Number.cc-353" << endl;
			nri ^= (str.length() - i - 1);std::cout << "Number.cc-354" << endl;
			if(str[i] >= 'a' && str[i] <= 'z') nri *= (str[i] - 'a' + 1);
			else if(str[i] >= 'A' && str[i] <= 'Z') nri *= (str[i] - 'A' + 1);
			add(nri);std::cout << "Number.cc-357" << endl;
		}
		return;
	}
	if(po.base < BASE_CUSTOM || (po.base == BASE_CUSTOM && (!CALCULATOR->customInputBase().isInteger() || CALCULATOR->customInputBase() < 2 || CALCULATOR->customInputBase() > 62))) {
		Number base;std::cout << "Number.cc-362" << endl;
		switch(po.base) {
			case BASE_GOLDEN_RATIO: {
				base.set(5);std::cout << "Number.cc-365" << endl;
				base.sqrt();std::cout << "Number.cc-366" << endl;
				base.add(1);std::cout << "Number.cc-367" << endl;
				base.divide(2);std::cout << "Number.cc-368" << endl;
				break;
			}
			case BASE_SUPER_GOLDEN_RATIO: {
				base.set(93);std::cout << "Number.cc-372" << endl;
				base.sqrt();std::cout << "Number.cc-373" << endl;
				base.multiply(3);std::cout << "Number.cc-374" << endl;
				Number b2(base);std::cout << "Number.cc-375" << endl;
				b2.negate();std::cout << "Number.cc-376" << endl;
				b2.add(29);std::cout << "Number.cc-377" << endl;
				b2.divide(2);std::cout << "Number.cc-378" << endl;
				b2.cbrt();std::cout << "Number.cc-379" << endl;
				base.add(29);std::cout << "Number.cc-380" << endl;
				base.divide(2);std::cout << "Number.cc-381" << endl;
				base.cbrt();std::cout << "Number.cc-382" << endl;
				base.add(b2);std::cout << "Number.cc-383" << endl;
				base.add(1);std::cout << "Number.cc-384" << endl;
				base.divide(3);std::cout << "Number.cc-385" << endl;
				break;
			}
			case BASE_PI: {base.pi(); break;}
			case BASE_E: {base.e(); break;}
			case BASE_SQRT2: {base.set(2); base.sqrt(); break;}
			case BASE_UNICODE: {base.set(1114112L); break;}
			default: {base = CALCULATOR->customInputBase();std::cout << "Number.cc-392" << endl;}
		}
		Number abs_base(base);std::cout << "Number.cc-394" << endl;
		abs_base.abs();std::cout << "Number.cc-395" << endl;
		abs_base.ceil();std::cout << "Number.cc-396" << endl;
		if(abs_base < 2) abs_base = 2;
		size_t i_dot = number.length();std::cout << "Number.cc-398" << endl;
		vector<Number> digits;std::cout << "Number.cc-399" << endl;
		bool b_minus = false;std::cout << "Number.cc-400" << endl;
		if(abs_base <= 62) {
			remove_blanks(number);std::cout << "Number.cc-402" << endl;
			bool b_case = abs_base > 36;std::cout << "Number.cc-403" << endl;
			i_dot = number.length();std::cout << "Number.cc-404" << endl;
			for(size_t i = 0; i < number.length(); i++) {
				long int c = -1;std::cout << "Number.cc-406" << endl;
				if(number[i] >= '0' && number[i] <= '9') {
					c = number[i] - '0';std::cout << "Number.cc-408" << endl;
				} else if(number[i] >= 'a' && number[i] <= 'z') {
					if(b_case) c = number[i] - 'a' + 36;
					else c = number[i] - 'a' + 10;std::cout << "Number.cc-411" << endl;
				} else if(number[i] >= 'A' && number[i] <= 'Z') {
					c = number[i] - 'A' + 10;std::cout << "Number.cc-413" << endl;
				} else if(number[i] == '.') {
					if(i_dot == number.length()) i_dot = digits.size();
				} else if(number[i] == '-' && digits.empty()) {
					b_minus = !b_minus;std::cout << "Number.cc-417" << endl;
				} else {
					string str_char = number.substr(i, 1);std::cout << "Number.cc-419" << endl;
					while(i + 1 < number.length() && number[i + 1] < 0 && (unsigned char) number[i + 1] < 0xC0) {
						i++;std::cout << "Number.cc-421" << endl;
						str_char += number[i];std::cout << "Number.cc-422" << endl;
					}
					CALCULATOR->error(true, _("Character \'%s\' was ignored in the number \"%s\" with base %s."), str_char.c_str(), number.c_str(), base.print().c_str(), NULL);
				}
				if(c >= 0) {
					if(abs_base <= c && !abs_base.isFraction()) {
						CALCULATOR->error(false, _("Digit \'%s\' is too high for number base."), number.substr(i, 1).c_str(), NULL);
					}
					digits.push_back(c);std::cout << "Number.cc-430" << endl;
				}
			}
		} else {
			for(size_t i = 0; i < number.length(); i++) {
				size_t i_prev = i;std::cout << "Number.cc-435" << endl;
				long int c = (unsigned char) number[i];std::cout << "Number.cc-436" << endl;
				bool b_esc = false;std::cout << "Number.cc-437" << endl;
				if(number[i] == '\\' && i < number.length() - 1) {
					i++;std::cout << "Number.cc-439" << endl;
					Number nrd;std::cout << "Number.cc-440" << endl;
					if(is_in(NUMBERS, number[i])) {
						size_t i2 = number.find_first_not_of(NUMBERS, i);std::cout << "Number.cc-442" << endl;
						if(i2 == string::npos) i2 = number.length();
						nrd.set(number.substr(i, i2 - i));std::cout << "Number.cc-444" << endl;
						i = i2 - 1;std::cout << "Number.cc-445" << endl;
						b_esc = true;std::cout << "Number.cc-446" << endl;
					} else if(number[i] == 'x' && i < number.length() - 1 && is_in(NUMBERS "ABCDEFabcdef", number[i + 1])) {
						i++;std::cout << "Number.cc-448" << endl;
						size_t i2 = number.find_first_not_of(NUMBERS "ABCDEFabcdef", i);
						if(i2 == string::npos) i2 = number.length();
						ParseOptions po;std::cout << "Number.cc-451" << endl;
						po.base = BASE_HEXADECIMAL;std::cout << "Number.cc-452" << endl;
						nrd.set(number.substr(i, i2 - i), po);std::cout << "Number.cc-453" << endl;
						i = i2 - 1;std::cout << "Number.cc-454" << endl;
						b_esc = true;std::cout << "Number.cc-455" << endl;
					}
					if(digits.empty() && number[i] == (char) -30 && i + 3 < number.length() && number[i + 1] == (char) -120 && number[i + 2] == (char) -110) {
						i += 2;std::cout << "Number.cc-458" << endl;
						b_minus = !b_minus;std::cout << "Number.cc-459" << endl;
						b_esc = true;std::cout << "Number.cc-460" << endl;
					} else if(digits.empty() && number[i] == '-') {
						b_minus = !b_minus;std::cout << "Number.cc-462" << endl;
						b_esc = true;std::cout << "Number.cc-463" << endl;
					} else if(i_dot == number.size() && (number[i] == CALCULATOR->getDecimalPoint()[0] || (!po.dot_as_separator && number[i] == '.'))) {
						i_dot = digits.size();std::cout << "Number.cc-465" << endl;
						b_esc = true;std::cout << "Number.cc-466" << endl;
					} else if(b_esc) {
						if(abs_base.isLessThanOrEqualTo(nrd)) {
							CALCULATOR->error(false, _("Digit \'%s\' is too high for number base."), number.substr(i_prev, i - i_prev + 1).c_str(), NULL);
						}
						digits.push_back(nrd);std::cout << "Number.cc-471" << endl;

					} else if(number[i] != '\\') {
						i--;std::cout << "Number.cc-474" << endl;
					}
				}
				if(!b_esc) {
					if((c & 0x80) != 0) {
						if(c<0xe0) {
							i++;std::cout << "Number.cc-480" << endl;
							if(i >= number.length()) return;
							c = ((c & 0x1f) << 6) | (((unsigned char) number[i]) & 0x3f);std::cout << "Number.cc-482" << endl;
						} else if(c<0xf0) {
							i++;std::cout << "Number.cc-484" << endl;
							if(i + 1 >= number.length()) return;
							c = (((c & 0xf) << 12) | ((((unsigned char) number[i]) & 0x3f) << 6)|(((unsigned char) number[i + 1]) & 0x3f));std::cout << "Number.cc-486" << endl;
							i++;std::cout << "Number.cc-487" << endl;
						} else {
							i++;std::cout << "Number.cc-489" << endl;
							if(i + 2 >= number.length()) return;
							c = ((c & 7) << 18) | ((((unsigned char) number[i]) & 0x3f) << 12) | ((((unsigned char) number[i + 1]) & 0x3f) << 6) | (((unsigned char) number[i + 2]) & 0x3f);std::cout << "Number.cc-491" << endl;
							i += 2;std::cout << "Number.cc-492" << endl;
						}
					}
					if(abs_base.isLessThanOrEqualTo(c)) {
						CALCULATOR->error(false, _("Digit \'%s\' is too high for number base."), number.substr(i_prev, i - i_prev + 1).c_str(), NULL);
					}
					digits.push_back(c);std::cout << "Number.cc-498" << endl;
				}
			}
		}
		clear();std::cout << "Number.cc-502" << endl;
		if(i_dot > digits.size()) i_dot = digits.size();
		Number nr_mul;std::cout << "Number.cc-504" << endl;
		for(size_t i = 0; i < digits.size(); i++) {
			long int exp = i_dot - 1 - i;std::cout << "Number.cc-506" << endl;
			if(exp != 0) {
				nr_mul = base;std::cout << "Number.cc-508" << endl;
				nr_mul.raise(exp);std::cout << "Number.cc-509" << endl;
				digits[i].multiply(nr_mul);std::cout << "Number.cc-510" << endl;
			}
			add(digits[i]);std::cout << "Number.cc-512" << endl;
		}
		if(b_minus) negate();
		return;
	}

	size_t pm_index = number.find(SIGN_PLUSMINUS);std::cout << "Number.cc-518" << endl;
	if(pm_index == string::npos) pm_index = number.find("+/-");
	if(pm_index != string::npos) {
		ParseOptions po2 = po;std::cout << "Number.cc-521" << endl;
		po2.read_precision = DONT_READ_PRECISION;std::cout << "Number.cc-522" << endl;
		set(number.substr(0, pm_index), po2);std::cout << "Number.cc-523" << endl;
		number = number.substr(pm_index + (number[pm_index] == '+' ? strlen("+/-") : strlen(SIGN_PLUSMINUS)));
		if(!number.empty()) {
			Number pm_nr(number, po2);std::cout << "Number.cc-526" << endl;
			setUncertainty(pm_nr);std::cout << "Number.cc-527" << endl;
		}
		return;
	}

	if(po.base == BASE_ROMAN_NUMERALS) {
		remove_blanks(number);std::cout << "Number.cc-533" << endl;
		string number_bak = number;std::cout << "Number.cc-534" << endl;
		bool rev_c = (number.find("Ɔ") != string::npos);
		if(rev_c) gsub("Ɔ", ")", number);
		Number nr;std::cout << "Number.cc-537" << endl;
		Number cur;std::cout << "Number.cc-538" << endl;
		bool large = false;std::cout << "Number.cc-539" << endl;
		vector<Number> numbers;std::cout << "Number.cc-540" << endl;
		for(size_t i = 0; i < number.length(); i++) {
			switch(number[i]) {
				case 'I': {
					if(i > 0 && i == number.length() - 1 && number[i - 1] != 'i' && number[i - 1] != 'j' && number.find_first_of("jvxlcdm") != string::npos && number.find_first_of("IJVXLCDM") == i) {
						cur.set(2);std::cout << "Number.cc-545" << endl;
						CALCULATOR->error(false, _("Assuming the unusual practice of letting a last capital I mean 2 in a roman numeral."), NULL);
						break;
					}
				}
				case 'J': {}
				case 'i': {}
				case 'j': {
					cur.set(1);std::cout << "Number.cc-553" << endl;
					break;
				}
				case 'V': {}
				case 'v': {
					cur.set(5);std::cout << "Number.cc-558" << endl;
					break;
				}
				case 'X': {}
				case 'x': {
					cur.set(10);std::cout << "Number.cc-563" << endl;
					break;
				}
				case 'L': {}
				case 'l': {
					cur.set(50);std::cout << "Number.cc-568" << endl;
					break;
				}
				case 'C': {}
				case 'c': {
					if(rev_c) {
						size_t i2 = number.find_first_not_of("Cc", i);
						if(i2 != string::npos && number[i2] == '|' && i2 + (i2 - i) < number.length() && number[i2 + (i2 - i)] == ')') {
							bool b = true;std::cout << "Number.cc-576" << endl;
							for(size_t i3 = i2 + 1; i3 < i2 + (i2 - i); i3++) {
								if(number[i3] != ')') {b = false; break;}
							}
							if(b) {
								cur.set(1, 1, i2 - i + 2);std::cout << "Number.cc-581" << endl;
								i = i2 + (i2 - i);std::cout << "Number.cc-582" << endl;
								if(i + 1 < number.length() && number[i + 1] == ')') {
									i2 = number.find_first_not_of(")", i + 2);
									if(i2 == string::npos) {
										cur += Number(5, 1, number.length() - i);std::cout << "Number.cc-586" << endl;
										i = number.length() - 1;std::cout << "Number.cc-587" << endl;
									} else {
										cur += Number(5, 1, i2 - i);std::cout << "Number.cc-589" << endl;
										i = i2 - 1;std::cout << "Number.cc-590" << endl;
									}
								}
								break;
							}
						}
					}
					cur.set(100);std::cout << "Number.cc-597" << endl;
					break;
				}
				case 'D': {}
				case 'd': {
					cur.set(500);std::cout << "Number.cc-602" << endl;
					break;
				}
				case 'M': {}
				case 'm': {
					cur.set(1000);std::cout << "Number.cc-607" << endl;
					break;
				}
				case '|': {
					if(large) {
						cur.clear();std::cout << "Number.cc-612" << endl;
						large = false;std::cout << "Number.cc-613" << endl;
						break;
					} else if(i + 1 < number.length() && number[i + 1] == ')') {
						size_t i2 = number.find_first_not_of(")", i + 2);
						if(i2 == string::npos) {
							cur.set(5, 1, number.length() - i - 1);std::cout << "Number.cc-618" << endl;
							i = number.length() - 1;std::cout << "Number.cc-619" << endl;
						} else {
							cur.set(5, 1, i2 - i);std::cout << "Number.cc-621" << endl;
							i = i2 - 1;std::cout << "Number.cc-622" << endl;
						}
						break;
					} else if(i + 2 < number.length() && number.find("|", i + 2) != string::npos) {
						cur.clear();std::cout << "Number.cc-626" << endl;
						large = true;std::cout << "Number.cc-627" << endl;
						break;
					}
					CALCULATOR->error(true, _("Error in roman numerals: %s."), number_bak.c_str(), NULL);
					break;
				}
				case '(': {
					if(!rev_c) {
						size_t i2 = number.find_first_not_of("(", i);
						if(i2 != string::npos && number[i2] == '|' && i2 + (i2 - i) < number.length() && number[i2 + (i2 - i)] == ')') {
							bool b = true;std::cout << "Number.cc-637" << endl;
							for(size_t i3 = i2 + 1; i3 < i2 + (i2 - i); i3++) {
								if(number[i3] != ')') {b = false; break;}
							}
							if(b) {
								cur.set(1, 1, i2 - i + 2);std::cout << "Number.cc-642" << endl;
								i = i2 + (i2 - i);std::cout << "Number.cc-643" << endl;
								if(i + 1 < number.length() && number[i + 1] == ')') {
									i2 = number.find_first_not_of(")", i + 2);
									if(i2 == string::npos) {
										cur += Number(5, 1, number.length() - i);std::cout << "Number.cc-647" << endl;
										i = number.length() - 1;std::cout << "Number.cc-648" << endl;
									} else {
										cur += Number(5, 1, i2 - i);std::cout << "Number.cc-650" << endl;
										i = i2 - 1;std::cout << "Number.cc-651" << endl;
									}
								}
								break;
							}
						}
						CALCULATOR->error(true, _("Error in roman numerals: %s."), number_bak.c_str(), NULL);
						break;
					}
				}
				case ')': {
					CALCULATOR->error(true, _("Error in roman numerals: %s."), number_bak.c_str(), NULL);
					break;
				}
				default: {
					cur.clear();std::cout << "Number.cc-666" << endl;
					CALCULATOR->error(true, _("Unknown roman numeral: %c."), number[i], NULL);
				}
			}
			if(!cur.isZero()) {
				if(large) {
					cur.multiply(100000L);std::cout << "Number.cc-672" << endl;
				}
				numbers.resize(numbers.size() + 1);std::cout << "Number.cc-674" << endl;
				numbers[numbers.size() - 1].set(cur);std::cout << "Number.cc-675" << endl;
			}
		}
		vector<Number> values;std::cout << "Number.cc-678" << endl;
		values.resize(numbers.size());std::cout << "Number.cc-679" << endl;
		bool error = false;std::cout << "Number.cc-680" << endl;
		int rep = 1;std::cout << "Number.cc-681" << endl;
		for(size_t i = 0; i < numbers.size(); i++) {
			if(i == 0 || numbers[i].isLessThanOrEqualTo(numbers[i - 1])) {
				nr.add(numbers[i]);std::cout << "Number.cc-684" << endl;
				if(i > 0 && numbers[i].equals(numbers[i - 1])) {
					rep++;std::cout << "Number.cc-686" << endl;
					if(rep > 3 && numbers[i].isLessThan(1000)) {
						error = true;std::cout << "Number.cc-688" << endl;
					} else if(rep > 1 && (numbers[i].equals(5) || numbers[i].equals(50) || numbers[i].equals(500))) {
						error = true;std::cout << "Number.cc-690" << endl;
					}
				} else {
					rep = 1;std::cout << "Number.cc-693" << endl;
				}
			} else {
				numbers[i - 1].multiply(10);std::cout << "Number.cc-696" << endl;
				if(numbers[i - 1].isLessThan(numbers[i])) {
					error = true;std::cout << "Number.cc-698" << endl;
				}
				numbers[i - 1].divide(10);std::cout << "Number.cc-700" << endl;
				for(int i2 = i - 2; ; i2--) {
					if(i2 < 0) {
						nr.negate();std::cout << "Number.cc-703" << endl;
						nr.add(numbers[i]);std::cout << "Number.cc-704" << endl;
						break;
					} else if(numbers[i2].isGreaterThan(numbers[i2 + 1])) {
						Number nr2(nr);std::cout << "Number.cc-707" << endl;
						nr2.subtract(values[i2]);std::cout << "Number.cc-708" << endl;
						nr.subtract(nr2);std::cout << "Number.cc-709" << endl;
						nr.subtract(nr2);std::cout << "Number.cc-710" << endl;
						nr.add(numbers[i]);std::cout << "Number.cc-711" << endl;
						if(numbers[i2].isLessThan(numbers[i])) {
							error = true;std::cout << "Number.cc-713" << endl;
						}
						break;
					}
					error = true;std::cout << "Number.cc-717" << endl;
				}
			}
			values[i].set(nr);std::cout << "Number.cc-720" << endl;
		}
		if(error && number.find('|') == string::npos) {
			PrintOptions pro;std::cout << "Number.cc-723" << endl;
			pro.base = BASE_ROMAN_NUMERALS;std::cout << "Number.cc-724" << endl;
			CALCULATOR->error(false, _("Errors in roman numerals: \"%s\". Interpreted as %s, which should be written as %s."), number.c_str(), nr.print().c_str(), nr.print(pro).c_str(), NULL);
		}
		values.clear();std::cout << "Number.cc-727" << endl;
		numbers.clear();std::cout << "Number.cc-728" << endl;
		set(nr);std::cout << "Number.cc-729" << endl;
		return;
	}

	int base = po.base;std::cout << "Number.cc-733" << endl;
	if(base == BASE_CUSTOM) base = CALCULATOR->customInputBase().intValue();
	else if(base < 2 || base > 36) base = 10;

	long int i_unc = 0;std::cout << "Number.cc-737" << endl;
	mpz_t num, den;std::cout << "Number.cc-738" << endl;
	mpq_t unc;std::cout << "Number.cc-739" << endl;
	mpz_init(num);std::cout << "Number.cc-740" << endl;
	mpz_init_set_ui(den, 1);std::cout << "Number.cc-741" << endl;

	remove_blank_ends(number);std::cout << "Number.cc-743" << endl;
	if(po.base == 16 && number.length() >= 2 && number[0] == '0' && (number[1] == 'x' || number[1] == 'X')) {
		number = number.substr(2, number.length() - 2);std::cout << "Number.cc-745" << endl;
	} else if(po.base == 8 && number.length() >= 2 && number[0] == '0' && (number[1] == 'o' || number[1] == 'O')) {
		number = number.substr(2, number.length() - 2);std::cout << "Number.cc-747" << endl;
	} else if(po.base == 8 && number.length() > 1 && number[0] == '0' && number[1] != '.') {
		number.erase(number.begin());std::cout << "Number.cc-749" << endl;
	} else if(po.base == 2 && number.length() >= 2 && number[0] == '0' && (number[1] == 'b' || number[1] == 'B')) {
		number = number.substr(2, number.length() - 2);std::cout << "Number.cc-751" << endl;
	}
	bool b_twos = (po.twos_complement && po.base == 2 && number.length() > 1 && number[0] == '1') || (po.hexadecimal_twos_complement && po.base == 16 && number.length() > 0 && (number[0] == '8' || number[0] == '9' || (number[0] >= 'a' && number[0] <= 'f') || (number[0] >= 'A' && number[0] <= 'F')));std::cout << "Number.cc-753" << endl;

	long int readprec = 0;std::cout << "Number.cc-755" << endl;
	bool numbers_started = false, minus = false, in_decimals = false, b_cplx = false, had_nonzero = false;std::cout << "Number.cc-756" << endl;
	for(size_t index = 0; index < number.size(); index++) {
		if(number[index] >= '0' && ((base >= 10 && number[index] <= '9') || (base < 10 && number[index] < '0' + base))) {
			mpz_mul_si(num, num, base);std::cout << "Number.cc-759" << endl;
			if(number[index] != (b_twos ? '0' + (base - 1) : '0')) {
				mpz_add_ui(num, num, b_twos ? (unsigned long int) (base - 1) - (number[index] - '0') : (unsigned long int) number[index] - '0');std::cout << "Number.cc-761" << endl;
				if(!had_nonzero) readprec = 0;
				had_nonzero = true;std::cout << "Number.cc-763" << endl;
			}
			if(in_decimals) {
				mpz_mul_si(den, den, base);std::cout << "Number.cc-766" << endl;
			}
			readprec++;std::cout << "Number.cc-768" << endl;
			numbers_started = true;std::cout << "Number.cc-769" << endl;
		} else if(po.base == BASE_DUODECIMAL && (number[index] == 'X' || number[index] == 'E' || number[index] == 'x' || number[index] == 'e')) {
			mpz_mul_si(num, num, base);std::cout << "Number.cc-771" << endl;
			mpz_add_ui(num, num, (number[index] == 'E' || number[index] == 'e') ? 11L : 10L);std::cout << "Number.cc-772" << endl;
			if(in_decimals) {
				mpz_mul_si(den, den, base);std::cout << "Number.cc-774" << endl;
			}
			if(!had_nonzero) readprec = 0;
			had_nonzero = true;std::cout << "Number.cc-777" << endl;
			readprec++;std::cout << "Number.cc-778" << endl;
			numbers_started = true;std::cout << "Number.cc-779" << endl;
		} else if(base > 10 && number[index] >= 'a' && number[index] < 'a' + base - (base > 36 ? 36 : 10)) {
			mpz_mul_si(num, num, base);std::cout << "Number.cc-781" << endl;
			if(!b_twos || (number[index] != 'a' + (base - (base > 36 ? 37 : 11)))) {
				mpz_add_ui(num, num, b_twos ? (unsigned long int) (base - 1) - (number[index] - 'a' + (base > 36 ? 36 : 10)) : (unsigned long int) number[index] - 'a' + (base > 36 ? 36 : 10));std::cout << "Number.cc-783" << endl;
				if(!had_nonzero) readprec = 0;
				had_nonzero = true;std::cout << "Number.cc-785" << endl;
			}
			if(in_decimals) {
				mpz_mul_si(den, den, base);std::cout << "Number.cc-788" << endl;
			}
			readprec++;std::cout << "Number.cc-790" << endl;
			numbers_started = true;std::cout << "Number.cc-791" << endl;
		} else if(base > 10 && number[index] >= 'A' && number[index] < 'A' + base - 10) {
			mpz_mul_si(num, num, base);std::cout << "Number.cc-793" << endl;
			if(!b_twos || (number[index] != 'A' + (base - 11))) {
				mpz_add_ui(num, num, b_twos ? (unsigned long int) (base - 1) - (number[index] - 'A' + 10) : (unsigned long int) number[index] - 'A' + 10);std::cout << "Number.cc-795" << endl;
				if(!had_nonzero) readprec = 0;
				had_nonzero = true;std::cout << "Number.cc-797" << endl;
			}
			if(in_decimals) {
				mpz_mul_si(den, den, base);std::cout << "Number.cc-800" << endl;
			}
			readprec++;std::cout << "Number.cc-802" << endl;
			numbers_started = true;std::cout << "Number.cc-803" << endl;
		} else if((number[index] == 'E' || number[index] == 'e') && base <= 10) {
			index++;std::cout << "Number.cc-805" << endl;
			numbers_started = false;std::cout << "Number.cc-806" << endl;
			bool exp_minus = false;std::cout << "Number.cc-807" << endl;
			unsigned long int exp = 0;std::cout << "Number.cc-808" << endl;
			unsigned long int max_exp = ULONG_MAX / 10;std::cout << "Number.cc-809" << endl;
			while(index < number.size()) {
				if(number[index] >= '0' && number[index] <= '9') {
					if(exp > max_exp) {
						CALCULATOR->error(true, _("Too large exponent."), NULL);
					} else {
						exp = exp * 10;std::cout << "Number.cc-815" << endl;
						exp = exp + number[index] - '0';std::cout << "Number.cc-816" << endl;
						numbers_started = true;std::cout << "Number.cc-817" << endl;
					}
				} else if(!numbers_started && number[index] == '-') {
					exp_minus = !exp_minus;std::cout << "Number.cc-820" << endl;
				}
				index++;std::cout << "Number.cc-822" << endl;
			}
			if(exp_minus) {
				mpz_t e_den;std::cout << "Number.cc-825" << endl;
				mpz_init(e_den);std::cout << "Number.cc-826" << endl;
				mpz_ui_pow_ui(e_den, 10, exp);std::cout << "Number.cc-827" << endl;
				mpz_mul(den, den, e_den);std::cout << "Number.cc-828" << endl;
				if(i_unc > 0) mpz_mul(mpq_denref(unc), mpq_denref(unc), e_den);
				mpz_clear(e_den);std::cout << "Number.cc-830" << endl;
			} else {
				mpz_t e_num;std::cout << "Number.cc-832" << endl;
				mpz_init(e_num);std::cout << "Number.cc-833" << endl;
				mpz_ui_pow_ui(e_num, 10, exp);std::cout << "Number.cc-834" << endl;
				mpz_mul(num, num, e_num);std::cout << "Number.cc-835" << endl;
				if(i_unc > 0) mpz_mul(mpq_numref(unc), mpq_numref(unc), e_num);
				mpz_clear(e_num);std::cout << "Number.cc-837" << endl;
			}
			break;
		} else if(number[index] == '.') {
			if(in_decimals) CALCULATOR->error(false, "Misplaced decimal separator ignored", NULL);
			else in_decimals = true;std::cout << "Number.cc-842" << endl;
		} else if(number[index] == ':') {
			if(in_decimals) {
				CALCULATOR->error(true, _("\':\' in decimal number ignored (decimal point detected)."), NULL);
			} else {
				size_t index_colon = index;std::cout << "Number.cc-847" << endl;
				Number divisor(1, 1);std::cout << "Number.cc-848" << endl;
				Number num_temp;std::cout << "Number.cc-849" << endl;
				clear();std::cout << "Number.cc-850" << endl;
				i_precision = -1;std::cout << "Number.cc-851" << endl;
				index = 0;std::cout << "Number.cc-852" << endl;
				while(index_colon < number.size()) {
					num_temp.set(number.substr(index, index_colon - index), po);std::cout << "Number.cc-854" << endl;
					if(!num_temp.isZero()) {
						num_temp.divide(divisor);std::cout << "Number.cc-856" << endl;
						add(num_temp);std::cout << "Number.cc-857" << endl;
					}
					index = index_colon + 1;std::cout << "Number.cc-859" << endl;
					index_colon = number.find(":", index);
					divisor.multiply(Number(60, 1));std::cout << "Number.cc-861" << endl;
				}
				num_temp.set(number.substr(index), po);std::cout << "Number.cc-863" << endl;
				if(!num_temp.isZero()) {
					num_temp.divide(divisor);std::cout << "Number.cc-865" << endl;
					add(num_temp);std::cout << "Number.cc-866" << endl;
				}
				mpz_clears(num, den, NULL);std::cout << "Number.cc-868" << endl;
				return;
			}
		} else if(!numbers_started && number[index] == '-') {
			minus = !minus;std::cout << "Number.cc-872" << endl;
		} else if(number[index] == 'i' || (CALCULATOR && number[index] == 'j' && CALCULATOR->v_i->hasName("j"))) {
			b_cplx = true;std::cout << "Number.cc-874" << endl;
		} else if(base == 10 && number[index] == '(' && index <= number.length() - 2) {
			size_t par_i = number.find(')', index + 1);std::cout << "Number.cc-876" << endl;
			if(par_i == string::npos) {
				i_unc = s2i(number.substr(index + 1));std::cout << "Number.cc-878" << endl;
				index = number.length() - 1;std::cout << "Number.cc-879" << endl;
			} else if(par_i > index + 1) {
				i_unc = s2i(number.substr(index + 1, par_i - index - 1));std::cout << "Number.cc-881" << endl;
				index = par_i;std::cout << "Number.cc-882" << endl;
			}
			if(i_unc > 0) {
				mpq_init(unc);std::cout << "Number.cc-885" << endl;
				mpz_set(mpq_denref(unc), den);std::cout << "Number.cc-886" << endl;
				mpz_set_ui(mpq_numref(unc), i_unc);std::cout << "Number.cc-887" << endl;
			}
		} else if(number[index] != ' ') {
			string str_char = number.substr(index, 1);std::cout << "Number.cc-890" << endl;
			while(index + 1 < number.length() && number[index + 1] < 0 && (unsigned char) number[index + 1] < 0xC0) {
				index++;std::cout << "Number.cc-892" << endl;
				str_char += number[index];std::cout << "Number.cc-893" << endl;
			}
			CALCULATOR->error(true, _("Character \'%s\' was ignored in the number \"%s\" with base %s."), str_char.c_str(), number.c_str(), i2s(base).c_str(), NULL);
		}
	}
	if(b_twos) {
		mpz_add_ui(num, num, 1);std::cout << "Number.cc-899" << endl;
		minus = !minus;std::cout << "Number.cc-900" << endl;
	}
	clear();std::cout << "Number.cc-902" << endl;
	if(i_unc <= 0 && (po.read_precision == ALWAYS_READ_PRECISION || (in_decimals && po.read_precision == READ_PRECISION_WHEN_DECIMALS))) {
		mpz_mul_si(num, num, 2);std::cout << "Number.cc-904" << endl;
		mpz_mul_si(den, den, 2);std::cout << "Number.cc-905" << endl;

		mpq_t rv1, rv2;std::cout << "Number.cc-907" << endl;
		mpq_inits(rv1, rv2, NULL);std::cout << "Number.cc-908" << endl;

		mpz_add_ui(num, num, 1);std::cout << "Number.cc-910" << endl;
		if(minus) mpz_neg(mpq_numref(rv1), num);
		else mpz_set(mpq_numref(rv1), num);std::cout << "Number.cc-912" << endl;
		mpz_set(mpq_denref(rv1), den);std::cout << "Number.cc-913" << endl;
		mpq_canonicalize(rv1);std::cout << "Number.cc-914" << endl;

		mpz_sub_ui(num, num, 2);std::cout << "Number.cc-916" << endl;
		if(minus) mpz_neg(mpq_numref(rv2), num);
		else mpz_set(mpq_numref(rv2), num);std::cout << "Number.cc-918" << endl;
		mpz_set(mpq_denref(rv2), den);std::cout << "Number.cc-919" << endl;
		mpq_canonicalize(rv2);std::cout << "Number.cc-920" << endl;

		mpfr_init2(fu_value, BIT_PRECISION);std::cout << "Number.cc-922" << endl;
		mpfr_init2(fl_value, BIT_PRECISION);std::cout << "Number.cc-923" << endl;
		mpfr_clear_flags();std::cout << "Number.cc-924" << endl;

		mpfr_set_q(fu_value, minus ? rv2 : rv1, MPFR_RNDD);std::cout << "Number.cc-926" << endl;
		mpfr_set_q(fl_value, minus ? rv1 : rv2, MPFR_RNDU);std::cout << "Number.cc-927" << endl;
		for(int i = 0; i < 3; i++) {mpfr_nextbelow(fu_value); mpfr_nextabove(fl_value);}


		if(mpfr_cmp(fl_value, fu_value) > 0) mpfr_swap(fl_value, fu_value);

		n_type = NUMBER_TYPE_FLOAT;std::cout << "Number.cc-933" << endl;

		b_approx = true;std::cout << "Number.cc-935" << endl;

		testErrors(2);std::cout << "Number.cc-937" << endl;

		if(b_cplx) {
			if(!i_value) {i_value = new Number(); i_value->markAsImaginaryPart();}
			i_value->set(*this, false, true);std::cout << "Number.cc-941" << endl;
			clearReal();std::cout << "Number.cc-942" << endl;
		}

		mpq_clears(rv1, rv2, NULL);std::cout << "Number.cc-945" << endl;
	} else {
		if(minus) mpz_neg(num, num);
		if(b_cplx) {
			if(!i_value) {i_value = new Number(); i_value->markAsImaginaryPart();}
			i_value->setInternal(num, den, false, true);std::cout << "Number.cc-950" << endl;
			mpq_canonicalize(i_value->internalRational());std::cout << "Number.cc-951" << endl;
		} else {
			mpz_set(mpq_numref(r_value), num);std::cout << "Number.cc-953" << endl;
			mpz_set(mpq_denref(r_value), den);std::cout << "Number.cc-954" << endl;
			mpq_canonicalize(r_value);std::cout << "Number.cc-955" << endl;
		}
		if(i_unc > 0) {
			Number nr_unc;std::cout << "Number.cc-958" << endl;
			mpq_canonicalize(unc);std::cout << "Number.cc-959" << endl;
			nr_unc.setInternal(unc);std::cout << "Number.cc-960" << endl;
			setUncertainty(nr_unc);std::cout << "Number.cc-961" << endl;
			mpq_clear(unc);std::cout << "Number.cc-962" << endl;
		} else if(po.read_precision == ALWAYS_READ_PRECISION || (in_decimals && po.read_precision == READ_PRECISION_WHEN_DECIMALS)) {
			if(base != 10) {
				Number precmax(10);std::cout << "Number.cc-965" << endl;
				precmax.raise(readprec);std::cout << "Number.cc-966" << endl;
				precmax--;std::cout << "Number.cc-967" << endl;
				precmax.log(base);std::cout << "Number.cc-968" << endl;
				precmax.floor();std::cout << "Number.cc-969" << endl;
				readprec = precmax.intValue();std::cout << "Number.cc-970" << endl;
			}
			if(b_cplx) i_value->setPrecision(readprec);
			setPrecision(readprec);std::cout << "Number.cc-973" << endl;
		}
	}
	mpz_clears(num, den, NULL);std::cout << "Number.cc-976" << endl;
}
void Number::set(long int numerator, long int denominator, long int exp_10, bool keep_precision, bool keep_imag) {
	if(!keep_precision) {
		b_approx = false;std::cout << "Number.cc-980" << endl;
		i_precision = -1;std::cout << "Number.cc-981" << endl;
	}
	if(denominator < 0) {
		denominator = -denominator;std::cout << "Number.cc-984" << endl;
		numerator = -numerator;std::cout << "Number.cc-985" << endl;
	}
	mpq_set_si(r_value, numerator, denominator == 0 ? 1 : denominator);std::cout << "Number.cc-987" << endl;
	mpq_canonicalize(r_value);std::cout << "Number.cc-988" << endl;
	if(n_type == NUMBER_TYPE_FLOAT) mpfr_clears(fu_value, fl_value, NULL);
	n_type = NUMBER_TYPE_RATIONAL;std::cout << "Number.cc-990" << endl;
	if(exp_10 != 0) {
		exp10(exp_10);std::cout << "Number.cc-992" << endl;
	}
	if(!keep_imag && i_value) i_value->clear();
	else if(i_value) setPrecisionAndApproximateFrom(*i_value);
}
void Number::setFloat(long double d_value) {
	b_approx = true;std::cout << "Number.cc-998" << endl;
	if(n_type != NUMBER_TYPE_FLOAT) {mpfr_init2(fu_value, BIT_PRECISION); mpfr_init2(fl_value, BIT_PRECISION);}
	if(CREATE_INTERVAL) {
		mpfr_set_ld(fu_value, d_value, MPFR_RNDU);std::cout << "Number.cc-1001" << endl;
		mpfr_set_ld(fl_value, d_value, MPFR_RNDD);std::cout << "Number.cc-1002" << endl;
	} else {
		mpfr_set_ld(fl_value, d_value, MPFR_RNDN);std::cout << "Number.cc-1004" << endl;
		mpfr_set(fu_value, fl_value, MPFR_RNDN);std::cout << "Number.cc-1005" << endl;
	}
	n_type = NUMBER_TYPE_FLOAT;std::cout << "Number.cc-1007" << endl;
	mpq_set_ui(r_value, 0, 1);std::cout << "Number.cc-1008" << endl;
	if(i_value) i_value->clear();
}
bool Number::setInterval(const Number &nr_lower, const Number &nr_upper, bool keep_precision) {

	Number nr_l(nr_lower), nr_u(nr_upper);std::cout << "Number.cc-1013" << endl;

	if(nr_l == nr_u) {
		set(nr_l, true);std::cout << "Number.cc-1016" << endl;
		setPrecisionAndApproximateFrom(nr_u);std::cout << "Number.cc-1017" << endl;
		return true;
	}

	if(!nr_l.setToFloatingPoint() || !nr_u.setToFloatingPoint()) return false;

	clear(keep_precision);std::cout << "Number.cc-1023" << endl;

	mpfr_init2(fu_value, BIT_PRECISION);std::cout << "Number.cc-1025" << endl;
	mpfr_init2(fl_value, BIT_PRECISION);std::cout << "Number.cc-1026" << endl;

	mpfr_clear_flags();std::cout << "Number.cc-1028" << endl;

	if(mpfr_cmp(nr_l.internalUpperFloat(), nr_u.internalUpperFloat()) > 0) mpfr_set(fu_value, nr_l.internalUpperFloat(), MPFR_RNDU);
	else mpfr_set(fu_value, nr_u.internalUpperFloat(), MPFR_RNDU);std::cout << "Number.cc-1031" << endl;
	if(mpfr_cmp(nr_l.internalLowerFloat(), nr_u.internalLowerFloat()) > 0) mpfr_set(fl_value, nr_u.internalLowerFloat(), MPFR_RNDD);
	else mpfr_set(fl_value, nr_l.internalLowerFloat(), MPFR_RNDD);std::cout << "Number.cc-1033" << endl;

	setPrecisionAndApproximateFrom(nr_l);std::cout << "Number.cc-1035" << endl;
	setPrecisionAndApproximateFrom(nr_u);std::cout << "Number.cc-1036" << endl;

	if(!b_imag && (nr_l.hasImaginaryPart() || nr_u.hasImaginaryPart())) {
		if(!i_value) {i_value = new Number(); i_value->markAsImaginaryPart();}
		i_value->setInterval(nr_l.imaginaryPart(), nr_u.imaginaryPart(), keep_precision);std::cout << "Number.cc-1040" << endl;
		setPrecisionAndApproximateFrom(*i_value);std::cout << "Number.cc-1041" << endl;
	}

	b_approx = true;std::cout << "Number.cc-1044" << endl;

	n_type = NUMBER_TYPE_FLOAT;std::cout << "Number.cc-1046" << endl;

	return true;

}

void Number::setInternal(mpz_srcptr mpz_value, bool keep_precision, bool keep_imag) {
	if(!keep_precision) {
		b_approx = false;std::cout << "Number.cc-1054" << endl;
		i_precision = -1;std::cout << "Number.cc-1055" << endl;
	}
	mpq_set_z(r_value, mpz_value);std::cout << "Number.cc-1057" << endl;
	if(n_type == NUMBER_TYPE_FLOAT) mpfr_clears(fu_value, fl_value, NULL);
	n_type = NUMBER_TYPE_RATIONAL;std::cout << "Number.cc-1059" << endl;
	if(!keep_imag && i_value) i_value->clear();
	else if(i_value) setPrecisionAndApproximateFrom(*i_value);
}
void Number::setInternal(const mpz_t &mpz_value, bool keep_precision, bool keep_imag) {
	if(!keep_precision) {
		b_approx = false;std::cout << "Number.cc-1065" << endl;
		i_precision = -1;std::cout << "Number.cc-1066" << endl;
	}
	mpq_set_z(r_value, mpz_value);std::cout << "Number.cc-1068" << endl;
	if(n_type == NUMBER_TYPE_FLOAT) mpfr_clears(fu_value, fl_value, NULL);
	n_type = NUMBER_TYPE_RATIONAL;std::cout << "Number.cc-1070" << endl;
	if(!keep_imag && i_value) i_value->clear();
	else if(i_value) setPrecisionAndApproximateFrom(*i_value);
}
void Number::setInternal(const mpq_t &mpq_value, bool keep_precision, bool keep_imag) {
	if(!keep_precision) {
		b_approx = false;std::cout << "Number.cc-1076" << endl;
		i_precision = -1;std::cout << "Number.cc-1077" << endl;
	}
	mpq_set(r_value, mpq_value);std::cout << "Number.cc-1079" << endl;
	if(n_type == NUMBER_TYPE_FLOAT) mpfr_clears(fu_value, fl_value, NULL);
	n_type = NUMBER_TYPE_RATIONAL;std::cout << "Number.cc-1081" << endl;
	if(!keep_imag && i_value) i_value->clear();
	else if(i_value) setPrecisionAndApproximateFrom(*i_value);
}
void Number::setInternal(const mpz_t &mpz_num, const mpz_t &mpz_den, bool keep_precision, bool keep_imag) {
	if(!keep_precision) {
		b_approx = false;std::cout << "Number.cc-1087" << endl;
		i_precision = -1;std::cout << "Number.cc-1088" << endl;
	}
	mpz_set(mpq_numref(r_value), mpz_num);std::cout << "Number.cc-1090" << endl;
	mpz_set(mpq_denref(r_value), mpz_den);std::cout << "Number.cc-1091" << endl;
	if(n_type == NUMBER_TYPE_FLOAT) mpfr_clears(fu_value, fl_value, NULL);
	n_type = NUMBER_TYPE_RATIONAL;std::cout << "Number.cc-1093" << endl;
	if(!keep_imag && i_value) i_value->clear();
	else if(i_value) setPrecisionAndApproximateFrom(*i_value);
}
void Number::setInternal(const mpfr_t &mpfr_value, bool merge_precision, bool keep_imag) {
	b_approx = true;std::cout << "Number.cc-1098" << endl;
	if(n_type != NUMBER_TYPE_FLOAT) {mpfr_init2(fu_value, BIT_PRECISION); mpfr_init2(fl_value, BIT_PRECISION);}
	if(CREATE_INTERVAL) {
		mpfr_set(fu_value, mpfr_value, MPFR_RNDU);std::cout << "Number.cc-1101" << endl;
		mpfr_set(fl_value, mpfr_value, MPFR_RNDD);std::cout << "Number.cc-1102" << endl;
	} else {
		mpfr_set(fl_value, mpfr_value, MPFR_RNDN);std::cout << "Number.cc-1104" << endl;
		mpfr_set(fu_value, fl_value, MPFR_RNDN);std::cout << "Number.cc-1105" << endl;
	}
	n_type = NUMBER_TYPE_FLOAT;std::cout << "Number.cc-1107" << endl;
	mpq_set_ui(r_value, 0, 1);std::cout << "Number.cc-1108" << endl;
	if(!keep_imag && i_value) i_value->clear();
}

void Number::setImaginaryPart(const Number &o) {
	if(!i_value) {i_value = new Number(); i_value->markAsImaginaryPart();}
	i_value->set(o, false, true);std::cout << "Number.cc-1114" << endl;
	setPrecisionAndApproximateFrom(*i_value);std::cout << "Number.cc-1115" << endl;
}
void Number::setImaginaryPart(long int numerator, long int denominator, long int exp_10) {
	if(!i_value) {i_value = new Number(); i_value->markAsImaginaryPart();}
	i_value->set(numerator, denominator, exp_10);std::cout << "Number.cc-1119" << endl;
}
void Number::set(const Number &o, bool merge_precision, bool keep_imag) {
	mpq_set(r_value, o.internalRational());std::cout << "Number.cc-1122" << endl;
	if(o.internalType() == NUMBER_TYPE_FLOAT) {
		if(n_type != NUMBER_TYPE_FLOAT) {mpfr_init2(fu_value, BIT_PRECISION); mpfr_init2(fl_value, BIT_PRECISION);}
		if(CREATE_INTERVAL || o.isInterval()) {
			mpfr_set(fu_value, o.internalUpperFloat(), MPFR_RNDU);std::cout << "Number.cc-1126" << endl;
			mpfr_set(fl_value, o.internalLowerFloat(), MPFR_RNDD);std::cout << "Number.cc-1127" << endl;
		} else {
			mpfr_set(fl_value, o.internalLowerFloat(), MPFR_RNDN);std::cout << "Number.cc-1129" << endl;
			mpfr_set(fu_value, fl_value, MPFR_RNDN);std::cout << "Number.cc-1130" << endl;
		}
	}
	n_type = o.internalType();std::cout << "Number.cc-1133" << endl;
	if(!merge_precision) {
		i_precision = -1;std::cout << "Number.cc-1135" << endl;
		b_approx = false;std::cout << "Number.cc-1136" << endl;
	}
	if(o.isApproximate()) b_approx = true;
	if(i_precision < 0 || o.precision() < i_precision) i_precision = o.precision();
	if(!keep_imag && !b_imag) {
		if(o.hasImaginaryPart()) {
			setImaginaryPart(*o.internalImaginary());std::cout << "Number.cc-1142" << endl;
		} else if(i_value) {
			i_value->clear();std::cout << "Number.cc-1144" << endl;
		}
	}
}
void Number::setPlusInfinity(bool keep_precision, bool keep_imag) {
	if(keep_imag) clearReal();
	else clear(keep_precision);std::cout << "Number.cc-1150" << endl;
	n_type = NUMBER_TYPE_PLUS_INFINITY;std::cout << "Number.cc-1151" << endl;
}
void Number::setMinusInfinity(bool keep_precision, bool keep_imag) {
	if(keep_imag) clearReal();
	else clear(keep_precision);std::cout << "Number.cc-1155" << endl;
	n_type = NUMBER_TYPE_MINUS_INFINITY;std::cout << "Number.cc-1156" << endl;
}

void Number::clear(bool keep_precision) {
	if(i_value) i_value->clear();
	if(!keep_precision) {
		b_approx = false;std::cout << "Number.cc-1162" << endl;
		i_precision = -1;std::cout << "Number.cc-1163" << endl;
	}
	if(n_type == NUMBER_TYPE_FLOAT) {
		mpfr_clear(fl_value);std::cout << "Number.cc-1166" << endl;
		mpfr_clear(fu_value);std::cout << "Number.cc-1167" << endl;
	}
	n_type = NUMBER_TYPE_RATIONAL;std::cout << "Number.cc-1169" << endl;
	mpq_set_si(r_value, 0, 1);std::cout << "Number.cc-1170" << endl;
}
void Number::clearReal() {
	if(n_type == NUMBER_TYPE_FLOAT) {
		mpfr_clear(fl_value);std::cout << "Number.cc-1174" << endl;
		mpfr_clear(fu_value);std::cout << "Number.cc-1175" << endl;
	}
	n_type = NUMBER_TYPE_RATIONAL;std::cout << "Number.cc-1177" << endl;
	mpq_set_si(r_value, 0, 1);std::cout << "Number.cc-1178" << endl;
}
void Number::clearImaginary() {
	if(i_value) i_value->clear();
}

const mpq_t &Number::internalRational() const {
	return r_value;
}
const mpfr_t &Number::internalUpperFloat() const {
	return fu_value;
}
const mpfr_t &Number::internalLowerFloat() const {
	return fl_value;
}
mpq_t &Number::internalRational() {
	return r_value;
}
mpfr_t &Number::internalUpperFloat() {
	return fu_value;
}
mpfr_t &Number::internalLowerFloat() {
	return fl_value;
}
Number *Number::internalImaginary() const {
	return i_value;
}
void Number::markAsImaginaryPart(bool is_imag) {
	b_imag = is_imag;std::cout << "Number.cc-1206" << endl;
}
const NumberType &Number::internalType() const {
	return n_type;
}
bool Number::setToFloatingPoint() {
	if(n_type != NUMBER_TYPE_FLOAT) {
		mpfr_init2(fu_value, BIT_PRECISION);std::cout << "Number.cc-1213" << endl;
		mpfr_init2(fl_value, BIT_PRECISION);std::cout << "Number.cc-1214" << endl;

		mpfr_clear_flags();std::cout << "Number.cc-1216" << endl;

		if(n_type == NUMBER_TYPE_RATIONAL) {
			if(!CREATE_INTERVAL) {
				mpfr_set_q(fl_value, r_value, MPFR_RNDN);std::cout << "Number.cc-1220" << endl;
				mpfr_set(fu_value, fl_value, MPFR_RNDN);std::cout << "Number.cc-1221" << endl;
			} else {
				mpfr_set_q(fu_value, r_value, MPFR_RNDU);std::cout << "Number.cc-1223" << endl;
				mpfr_set_q(fl_value, r_value, MPFR_RNDD);std::cout << "Number.cc-1224" << endl;
			}
			if(!testFloatResult(true, 1, false)) {
				mpfr_clears(fu_value, fl_value, NULL);std::cout << "Number.cc-1227" << endl;
				return false;
			}
		} else if(n_type == NUMBER_TYPE_PLUS_INFINITY) {
			mpfr_set_inf(fl_value, 1);std::cout << "Number.cc-1231" << endl;
			mpfr_set_inf(fu_value, 1);std::cout << "Number.cc-1232" << endl;
		} else if(n_type == NUMBER_TYPE_MINUS_INFINITY) {
			mpfr_set_inf(fl_value, -1);std::cout << "Number.cc-1234" << endl;
			mpfr_set_inf(fu_value, -1);std::cout << "Number.cc-1235" << endl;
		} else {
			mpfr_clears(fu_value, fl_value, NULL);std::cout << "Number.cc-1237" << endl;
			return false;
		}
		mpq_set_ui(r_value, 0, 1);std::cout << "Number.cc-1240" << endl;
		n_type = NUMBER_TYPE_FLOAT;std::cout << "Number.cc-1241" << endl;
	}
	return true;
}
void Number::precisionToInterval() {
	if(hasImaginaryPart()) i_value->precisionToInterval();
	if(i_precision >= 0 && !isInfinite(true) && !isInterval()) {
		if(!setToFloatingPoint()) return;
		mpfr_clear_flags();std::cout << "Number.cc-1249" << endl;
		mpfr_t f_log;std::cout << "Number.cc-1250" << endl;
		mpfr_init2(f_log, mpfr_get_prec(fl_value));std::cout << "Number.cc-1251" << endl;
		mpfr_abs(f_log, fu_value, MPFR_RNDN);std::cout << "Number.cc-1252" << endl;
		mpfr_log10(f_log, f_log, MPFR_RNDN);std::cout << "Number.cc-1253" << endl;
		mpfr_floor(f_log, f_log);std::cout << "Number.cc-1254" << endl;
		mpfr_sub_ui(f_log, f_log, i_precision, MPFR_RNDN);std::cout << "Number.cc-1255" << endl;
		mpfr_ui_pow(f_log, 10, f_log, MPFR_RNDD);std::cout << "Number.cc-1256" << endl;
		mpfr_div_ui(f_log, f_log, 2, MPFR_RNDD);std::cout << "Number.cc-1257" << endl;
		mpfr_sub(fl_value, fl_value, f_log, MPFR_RNDU);std::cout << "Number.cc-1258" << endl;
		mpfr_add(fu_value, fu_value, f_log, MPFR_RNDD);std::cout << "Number.cc-1259" << endl;
		mpfr_clear(f_log);std::cout << "Number.cc-1260" << endl;
		testErrors(2);std::cout << "Number.cc-1261" << endl;
		i_precision = -1;std::cout << "Number.cc-1262" << endl;
	}
}
bool Number::intervalToPrecision(long int min_precision) {
	if(n_type == NUMBER_TYPE_FLOAT && !mpfr_equal_p(fl_value, fu_value)) {
		if(mpfr_inf_p(fl_value) || mpfr_inf_p(fu_value)) return false;
		mpfr_clear_flags();std::cout << "Number.cc-1268" << endl;
		mpfr_t f_diff, f_mid;
		mpfr_inits2(mpfr_get_prec(fl_value), f_diff, f_mid, NULL);
		mpfr_sub(f_diff, fu_value, fl_value, MPFR_RNDN);
		mpfr_div_ui(f_diff, f_diff, 2, MPFR_RNDN);
		mpfr_add(f_mid, fl_value, f_diff, MPFR_RNDN);
		mpfr_mul_ui(f_diff, f_diff, 2, MPFR_RNDN);
		mpfr_div(f_diff, f_mid, f_diff, MPFR_RNDN);
		mpfr_abs(f_diff, f_diff, MPFR_RNDN);
		if(mpfr_zero_p(f_diff)) {mpfr_clears(f_diff, f_mid, NULL); return false;}
		mpfr_log10(f_diff, f_diff, MPFR_RNDN);
		long int i_prec = mpfr_get_si(f_diff, MPFR_RNDD) + 1;
		if(i_prec < min_precision || testErrors(0)) {mpfr_clears(f_diff, f_mid, NULL); return false;}
		if(i_value && !i_value->intervalToPrecision()) {mpfr_clears(f_diff, f_mid, NULL); return false;}
		if(i_precision < 0 || i_prec < i_precision) i_precision = i_prec;
		mpfr_set(fl_value, f_mid, MPFR_RNDN);std::cout << "Number.cc-1283" << endl;
		mpfr_set(fu_value, f_mid, MPFR_RNDN);std::cout << "Number.cc-1284" << endl;
		mpfr_clears(f_diff, f_mid, NULL);
		b_approx = true;std::cout << "Number.cc-1286" << endl;
	} else if(i_value && !i_value->intervalToPrecision()) return false;
	return true;
}
void Number::intervalToMidValue() {
	if(n_type == NUMBER_TYPE_FLOAT && !mpfr_equal_p(fl_value, fu_value)) {
		if(mpfr_inf_p(fl_value) || mpfr_inf_p(fu_value)) {
			if(mpfr_inf_p(fl_value) && mpfr_inf_p(fu_value) && mpfr_sgn(fl_value) != mpfr_sgn(fu_value)) clearReal();
			else if(mpfr_inf_p(fl_value)) mpfr_set(fu_value, fl_value, MPFR_RNDN);
			else mpfr_set(fl_value, fu_value, MPFR_RNDN);std::cout << "Number.cc-1295" << endl;
		} else {
			mpfr_clear_flags();std::cout << "Number.cc-1297" << endl;
			mpfr_sub(fu_value, fu_value, fl_value, MPFR_RNDN);std::cout << "Number.cc-1298" << endl;
			mpfr_div_ui(fu_value, fu_value, 2, MPFR_RNDN);std::cout << "Number.cc-1299" << endl;
			mpfr_add(fl_value, fl_value, fu_value, MPFR_RNDN);std::cout << "Number.cc-1300" << endl;
			mpfr_set(fu_value, fl_value, MPFR_RNDN);std::cout << "Number.cc-1301" << endl;
			if(!testFloatResult()) clearReal();
		}
	}
	if(i_value) i_value->intervalToMidValue();
}
void Number::splitInterval(unsigned int nr_of_parts, vector<Number> &v) const {
	if(n_type == NUMBER_TYPE_FLOAT && isReal()) {
		if(nr_of_parts == 2) {
			mpfr_t f_mid;std::cout << "Number.cc-1310" << endl;
			mpfr_init2(f_mid, mpfr_get_prec(fl_value));std::cout << "Number.cc-1311" << endl;
			mpfr_sub(f_mid, fu_value, fl_value, MPFR_RNDN);std::cout << "Number.cc-1312" << endl;
			mpfr_div_ui(f_mid, f_mid, 2, MPFR_RNDN);std::cout << "Number.cc-1313" << endl;
			mpfr_add(f_mid, f_mid, fl_value, MPFR_RNDN);std::cout << "Number.cc-1314" << endl;
			v.push_back(*this);std::cout << "Number.cc-1315" << endl;
			mpfr_set(v.back().internalUpperFloat(), f_mid, MPFR_RNDU);std::cout << "Number.cc-1316" << endl;
			v.push_back(*this);std::cout << "Number.cc-1317" << endl;
			//mpfr_nextabove(f_mid);std::cout << "Number.cc-1318" << endl;
			mpfr_set(v.back().internalLowerFloat(), f_mid, MPFR_RNDD);std::cout << "Number.cc-1319" << endl;
		} else {
			mpfr_t value_diff, lower_value, upper_value, value_add;
			mpfr_inits2(mpfr_get_prec(fl_value), value_diff, lower_value, upper_value, value_add, NULL);
			mpfr_sub(value_diff, fu_value, fl_value, MPFR_RNDN);
			mpfr_div_ui(value_diff, value_diff, nr_of_parts, MPFR_RNDN);
			mpfr_set(lower_value, fl_value, MPFR_RNDD);std::cout << "Number.cc-1325" << endl;
			for(unsigned int i = 1; i <= nr_of_parts; i++) {
				mpfr_mul_ui(value_add, value_diff, i, MPFR_RNDU);
				mpfr_add(upper_value, fl_value, value_add, MPFR_RNDU);std::cout << "Number.cc-1328" << endl;
				if(mpfr_cmp(upper_value, fu_value) > 0) mpfr_set(upper_value, fu_value, MPFR_RNDU);
				v.push_back(*this);std::cout << "Number.cc-1330" << endl;
				mpfr_set(v.back().internalLowerFloat(), lower_value, MPFR_RNDD);std::cout << "Number.cc-1331" << endl;
				mpfr_set(v.back().internalUpperFloat(), upper_value, MPFR_RNDU);std::cout << "Number.cc-1332" << endl;
				mpfr_set(lower_value, upper_value, MPFR_RNDD);std::cout << "Number.cc-1333" << endl;
			}
		}
	}
}
bool Number::getCentralInteger(Number &nr_int, bool *b_multiple, vector<Number> *v) const {
	if(!isInterval() || !isReal()) {
		if(b_multiple) *b_multiple = false;
		return false;
	}
	mpfr_t fintl, fintu;std::cout << "Number.cc-1343" << endl;
	mpfr_init2(fintl, mpfr_get_prec(fl_value));std::cout << "Number.cc-1344" << endl;
	mpfr_init2(fintu, mpfr_get_prec(fu_value));std::cout << "Number.cc-1345" << endl;
	mpfr_floor(fintu, fu_value);std::cout << "Number.cc-1346" << endl;
	mpfr_ceil(fintl, fl_value);std::cout << "Number.cc-1347" << endl;
	int cmp = mpfr_cmp(fintl, fintu);std::cout << "Number.cc-1348" << endl;
	if(cmp == 0) {
		mpz_t z_int;std::cout << "Number.cc-1350" << endl;
		mpz_init(z_int);std::cout << "Number.cc-1351" << endl;
		mpfr_get_z(z_int, fl_value, MPFR_RNDN);std::cout << "Number.cc-1352" << endl;
		nr_int.setInternal(z_int);std::cout << "Number.cc-1353" << endl;
		if(b_multiple) *b_multiple = false;
		if(v) {
			mpfr_t f_prec;std::cout << "Number.cc-1356" << endl;
			mpfr_init2(f_prec, mpfr_get_prec(fl_value));std::cout << "Number.cc-1357" << endl;
			mpfr_ui_pow_ui(f_prec, 10, PRECISION + 10, MPFR_RNDN);std::cout << "Number.cc-1358" << endl;
			mpfr_div(f_prec, fintl, f_prec, MPFR_RNDN);std::cout << "Number.cc-1359" << endl;
			if(mpfr_cmp(fintl, fl_value) > 0) {
				mpfr_sub(fintl, fintl, f_prec, MPFR_RNDD);std::cout << "Number.cc-1361" << endl;
				v->push_back(*this);std::cout << "Number.cc-1362" << endl;
				mpfr_set(v->back().internalUpperFloat(), fintl, MPFR_RNDD);std::cout << "Number.cc-1363" << endl;
			}
			if(mpfr_cmp(fintu, fu_value) < 0) {
				mpfr_add(fintu, fintu, f_prec, MPFR_RNDU);std::cout << "Number.cc-1366" << endl;
				v->push_back(*this);std::cout << "Number.cc-1367" << endl;
				mpfr_set(v->back().internalLowerFloat(), fintu, MPFR_RNDU);std::cout << "Number.cc-1368" << endl;
			}
		}
		return true;
	} else if(cmp > 0) {
		if(b_multiple) *b_multiple = false;
	} else {
		if(b_multiple) *b_multiple = true;
	}
	return false;
}
bool Number::mergeInterval(const Number &o, bool set_to_overlap) {
	if(equals(o)) return true;
	if(!isReal() || !o.isReal()) return false;
	if(isRational()) {
		mpfr_init2(fu_value, BIT_PRECISION);std::cout << "Number.cc-1383" << endl;
		mpfr_init2(fl_value, BIT_PRECISION);std::cout << "Number.cc-1384" << endl;

		mpfr_clear_flags();std::cout << "Number.cc-1386" << endl;

		if(o.isRational()) {
			if(set_to_overlap) {mpfr_clears(fu_value, fl_value, NULL); return false;}
			if(mpq_cmp(r_value, o.internalRational()) > 0) {
				mpfr_set_q(fl_value, o.internalRational(), MPFR_RNDD);std::cout << "Number.cc-1391" << endl;
				mpfr_set_q(fu_value, r_value, MPFR_RNDU);std::cout << "Number.cc-1392" << endl;
			} else {
				mpfr_set_q(fu_value, o.internalRational(), MPFR_RNDU);std::cout << "Number.cc-1394" << endl;
				mpfr_set_q(fl_value, r_value, MPFR_RNDD);std::cout << "Number.cc-1395" << endl;
			}
		} else {
			if(mpfr_cmp_q(o.internalUpperFloat(), r_value) < 0) {
				if(set_to_overlap) {mpfr_clears(fu_value, fl_value, NULL); return false;}
				mpfr_set(fl_value, o.internalLowerFloat(), MPFR_RNDD);std::cout << "Number.cc-1400" << endl;
				mpfr_set_q(fu_value, r_value, MPFR_RNDU);std::cout << "Number.cc-1401" << endl;
			} else if(mpfr_cmp_q(o.internalLowerFloat(), r_value) > 0) {
				if(set_to_overlap) {mpfr_clears(fu_value, fl_value, NULL); return false;}
				mpfr_set(fu_value, o.internalUpperFloat(), MPFR_RNDU);std::cout << "Number.cc-1404" << endl;
				mpfr_set_q(fl_value, r_value, MPFR_RNDD);std::cout << "Number.cc-1405" << endl;
			} else {
				if(set_to_overlap) {
					mpfr_clears(fu_value, fl_value, NULL);std::cout << "Number.cc-1408" << endl;
					setPrecisionAndApproximateFrom(o);std::cout << "Number.cc-1409" << endl;
					return true;
				}
				mpfr_set(fl_value, o.internalLowerFloat(), MPFR_RNDD);std::cout << "Number.cc-1412" << endl;
				mpfr_set(fu_value, o.internalUpperFloat(), MPFR_RNDU);std::cout << "Number.cc-1413" << endl;
			}
		}

		if(!testFloatResult(true, 1, false)) {
			mpfr_clears(fu_value, fl_value, NULL);std::cout << "Number.cc-1418" << endl;
			return false;
		}
		mpq_set_ui(r_value, 0, 1);std::cout << "Number.cc-1421" << endl;
		n_type = NUMBER_TYPE_FLOAT;std::cout << "Number.cc-1422" << endl;
	} else if(o.isRational()) {
		if(mpfr_cmp_q(fu_value, o.internalRational()) < 0) {
			if(set_to_overlap) return false;
			mpfr_set_q(fu_value, o.internalRational(), MPFR_RNDU);std::cout << "Number.cc-1426" << endl;
		} else if(mpfr_cmp_q(fl_value, o.internalRational()) > 0) {
			if(set_to_overlap) return false;
			mpfr_set_q(fl_value, o.internalRational(), MPFR_RNDD);std::cout << "Number.cc-1429" << endl;
		} else {
			if(set_to_overlap) {
				set(o, true);std::cout << "Number.cc-1432" << endl;
				return true;
			}
		}
	} else if(set_to_overlap) {
		if(mpfr_cmp(fl_value, o.internalUpperFloat()) > 0 || mpfr_cmp(fu_value, o.internalLowerFloat()) < 0) {
			return false;
		} else {
			if(mpfr_cmp(fl_value, o.internalLowerFloat()) < 0) mpfr_set(fl_value, o.internalLowerFloat(), MPFR_RNDD);
			if(mpfr_cmp(fu_value, o.internalUpperFloat()) > 0) mpfr_set(fu_value, o.internalUpperFloat(), MPFR_RNDU);
		}
	} else {
		if(mpfr_cmp(fl_value, o.internalLowerFloat()) > 0) mpfr_set(fl_value, o.internalLowerFloat(), MPFR_RNDD);
		if(mpfr_cmp(fu_value, o.internalUpperFloat()) < 0) mpfr_set(fu_value, o.internalUpperFloat(), MPFR_RNDU);
	}
	setPrecisionAndApproximateFrom(o);std::cout << "Number.cc-1447" << endl;
	return true;
}

void Number::setUncertainty(const Number &o, bool to_precision) {
	if(o.hasImaginaryPart()) {
		if(!i_value) i_value = new Number();
		i_value->setUncertainty(o.imaginaryPart(), to_precision);std::cout << "Number.cc-1454" << endl;
		setPrecisionAndApproximateFrom(*i_value);std::cout << "Number.cc-1455" << endl;
		if(o.hasRealPart()) setUncertainty(o.realPart(), to_precision);
		return;
	}
	if(o.isInfinite()) {
		if(n_type != NUMBER_TYPE_FLOAT) {
			mpfr_inits2(BIT_PRECISION, fl_value, fu_value, NULL);std::cout << "Number.cc-1461" << endl;
		}
		mpfr_set_inf(fl_value, -1);std::cout << "Number.cc-1463" << endl;
		mpfr_set_inf(fu_value, 1);std::cout << "Number.cc-1464" << endl;
		mpq_set_ui(r_value, 0, 1);std::cout << "Number.cc-1465" << endl;
		n_type = NUMBER_TYPE_FLOAT;std::cout << "Number.cc-1466" << endl;
		return;
	}
	if(isInfinite()) return;
	b_approx = true;std::cout << "Number.cc-1470" << endl;
	if(to_precision && !isInterval()) {
		Number nr(*this);std::cout << "Number.cc-1472" << endl;
		nr.divide(o);std::cout << "Number.cc-1473" << endl;
		nr.abs();std::cout << "Number.cc-1474" << endl;
		nr.divide(2);std::cout << "Number.cc-1475" << endl;
		nr.log(10);std::cout << "Number.cc-1476" << endl;
		nr.floor();std::cout << "Number.cc-1477" << endl;
		long int i_prec = nr.lintValue();std::cout << "Number.cc-1478" << endl;
		if(i_prec > 0) {
			if(i_precision < 0 || i_prec < i_precision) i_precision = i_prec;
			return;
		}
	}
	if(o.isNegative()) {
		Number o_abs(o);std::cout << "Number.cc-1485" << endl;
		o_abs.negate();std::cout << "Number.cc-1486" << endl;
		setUncertainty(o_abs, to_precision);std::cout << "Number.cc-1487" << endl;
		return;
	}
	mpfr_clear_flags();std::cout << "Number.cc-1490" << endl;
	if(n_type == NUMBER_TYPE_RATIONAL) {
		mpfr_inits2(BIT_PRECISION, fl_value, fu_value, NULL);std::cout << "Number.cc-1492" << endl;
		if(o.isRational()) {
			mpq_sub(r_value, r_value, o.internalRational());std::cout << "Number.cc-1494" << endl;
			mpfr_set_q(fl_value, r_value, MPFR_RNDD);std::cout << "Number.cc-1495" << endl;
			mpq_add(r_value, r_value, o.internalRational());std::cout << "Number.cc-1496" << endl;
			mpq_add(r_value, r_value, o.internalRational());std::cout << "Number.cc-1497" << endl;
			mpfr_set_q(fu_value, r_value, MPFR_RNDU);std::cout << "Number.cc-1498" << endl;
		} else {
			mpfr_sub_q(fl_value, o.internalUpperFloat(), r_value, MPFR_RNDU);std::cout << "Number.cc-1500" << endl;
			mpfr_neg(fl_value, fl_value, MPFR_RNDD);std::cout << "Number.cc-1501" << endl;
			mpfr_add_q(fu_value, o.internalUpperFloat(), r_value, MPFR_RNDU);std::cout << "Number.cc-1502" << endl;
		}
		mpq_set_ui(r_value, 0, 1);std::cout << "Number.cc-1504" << endl;
		n_type = NUMBER_TYPE_FLOAT;std::cout << "Number.cc-1505" << endl;
	} else if(o.isRational()) {
		mpfr_sub_q(fl_value, fl_value, o.internalRational(), MPFR_RNDD);std::cout << "Number.cc-1507" << endl;
		mpfr_add_q(fu_value, fu_value, o.internalRational(), MPFR_RNDU);std::cout << "Number.cc-1508" << endl;
	} else {
		mpfr_sub(fl_value, fl_value, o.internalUpperFloat(), MPFR_RNDD);std::cout << "Number.cc-1510" << endl;
		mpfr_add(fu_value, fu_value, o.internalUpperFloat(), MPFR_RNDU);std::cout << "Number.cc-1511" << endl;
	}
	testErrors(2);std::cout << "Number.cc-1513" << endl;
}
void Number::setRelativeUncertainty(const Number &o, bool to_precision) {
	Number nr(*this);std::cout << "Number.cc-1516" << endl;
	nr.multiply(o);std::cout << "Number.cc-1517" << endl;
	setUncertainty(nr, to_precision);std::cout << "Number.cc-1518" << endl;
}
Number Number::uncertainty() const {
	if(!isInterval(false)) return Number();
	Number nr;std::cout << "Number.cc-1522" << endl;
	if(n_type == NUMBER_TYPE_FLOAT && !mpfr_equal_p(fl_value, fu_value)) {
		if(mpfr_inf_p(fl_value) || mpfr_inf_p(fu_value)) {
			nr.setPlusInfinity();std::cout << "Number.cc-1525" << endl;
		} else {
			mpfr_clear_flags();std::cout << "Number.cc-1527" << endl;
			mpfr_t f_mid;std::cout << "Number.cc-1528" << endl;
			mpfr_init2(f_mid, BIT_PRECISION);std::cout << "Number.cc-1529" << endl;
			mpfr_sub(f_mid, fu_value, fl_value, MPFR_RNDU);std::cout << "Number.cc-1530" << endl;
			mpfr_div_ui(f_mid, f_mid, 2, MPFR_RNDU);std::cout << "Number.cc-1531" << endl;
			nr.setInternal(f_mid);std::cout << "Number.cc-1532" << endl;
			mpfr_clear(f_mid);std::cout << "Number.cc-1533" << endl;
			nr.testFloatResult();std::cout << "Number.cc-1534" << endl;
		}
	}
	if(i_value) nr.setImaginaryPart(i_value->uncertainty());
	return nr;
}
Number Number::relativeUncertainty() const {
	if(!isInterval()) return Number();
	if(mpfr_inf_p(fl_value) || mpfr_inf_p(fu_value)) {
		Number nr;std::cout << "Number.cc-1543" << endl;
		nr.setPlusInfinity();std::cout << "Number.cc-1544" << endl;
		return nr;
	}
	mpfr_clear_flags();std::cout << "Number.cc-1547" << endl;
	mpfr_t f_mid, f_diff;
	mpfr_inits2(BIT_PRECISION, f_mid, f_diff, NULL);
	mpfr_sub(f_diff, fu_value, fl_value, MPFR_RNDU);
	mpfr_div_ui(f_diff, f_diff, 2, MPFR_RNDU);
	mpfr_add(f_mid, fl_value, f_diff, MPFR_RNDN);
	mpfr_abs(f_mid, f_mid, MPFR_RNDN);std::cout << "Number.cc-1553" << endl;
	mpfr_div(f_mid, f_diff, f_mid, MPFR_RNDN);
	Number nr;std::cout << "Number.cc-1555" << endl;
	nr.setInternal(f_mid);std::cout << "Number.cc-1556" << endl;
	mpfr_clears(f_mid, f_diff, NULL);
	nr.testFloatResult();std::cout << "Number.cc-1558" << endl;
	return nr;
}

double Number::floatValue() const {
	if(n_type == NUMBER_TYPE_RATIONAL) {
		return mpq_get_d(r_value);
	} else if(n_type == NUMBER_TYPE_FLOAT) {
		return mpfr_get_d(fu_value, MPFR_RNDN) / 2.0 + mpfr_get_d(fl_value, MPFR_RNDN) / 2.0;
	}
	return 0.0;
}
int Number::intValue(bool *overflow) const {
	if(includesInfinity()) return 0;
	if(n_type == NUMBER_TYPE_RATIONAL) {
		if(mpz_fits_sint_p(mpq_numref(r_value)) == 0) {
			if(overflow) *overflow = true;
			if(mpz_sgn(mpq_numref(r_value)) == -1) return INT_MIN;
			return INT_MAX;
		}
		return (int) mpz_get_si(mpq_numref(r_value));
	} else {
		Number nr;std::cout << "Number.cc-1580" << endl;
		nr.set(*this, false, true);std::cout << "Number.cc-1581" << endl;
		nr.round();std::cout << "Number.cc-1582" << endl;
		return nr.intValue(overflow);
	}
}
unsigned int Number::uintValue(bool *overflow) const {
	if(includesInfinity()) return 0;
	if(n_type == NUMBER_TYPE_RATIONAL) {
		if(mpz_fits_uint_p(mpq_numref(r_value)) == 0) {
			if(overflow) *overflow = true;
			if(mpz_sgn(mpq_numref(r_value)) == -1) return 0;
			return UINT_MAX;
		}
		return (unsigned int) mpz_get_ui(mpq_numref(r_value));
	} else {
		Number nr;std::cout << "Number.cc-1596" << endl;
		nr.set(*this, false, true);std::cout << "Number.cc-1597" << endl;
		nr.round();std::cout << "Number.cc-1598" << endl;
		return nr.uintValue(overflow);
	}
}
long int Number::lintValue(bool *overflow) const {
	if(includesInfinity()) return 0;
	if(n_type == NUMBER_TYPE_RATIONAL) {
		if(mpz_fits_slong_p(mpq_numref(r_value)) == 0) {
			if(overflow) *overflow = true;
			if(mpz_sgn(mpq_numref(r_value)) == -1) return LONG_MIN;
			return LONG_MAX;
		}
		return mpz_get_si(mpq_numref(r_value));
	} else {
		Number nr;std::cout << "Number.cc-1612" << endl;
		nr.set(*this, false, true);std::cout << "Number.cc-1613" << endl;
		nr.round();std::cout << "Number.cc-1614" << endl;
		return nr.lintValue(overflow);
	}
}
long long int Number::llintValue() const {
	if(includesInfinity()) return 0;
	if(n_type == NUMBER_TYPE_RATIONAL) {
		long long result = 0;std::cout << "Number.cc-1621" << endl;
		mpz_export(&result, 0, -1, sizeof result, 0, 0, mpq_numref(r_value));std::cout << "Number.cc-1622" << endl;
		if(mpq_sgn(r_value) < 0) return -result;
		return result;
	} else {
		Number nr;std::cout << "Number.cc-1626" << endl;
		nr.set(*this, false, true);std::cout << "Number.cc-1627" << endl;
		nr.round();std::cout << "Number.cc-1628" << endl;
		return nr.llintValue();
	}
}
unsigned long int Number::ulintValue(bool *overflow) const {
	if(includesInfinity()) return 0;
	if(n_type == NUMBER_TYPE_RATIONAL) {
		if(mpz_fits_ulong_p(mpq_numref(r_value)) == 0) {
			if(overflow) *overflow = true;
			if(mpz_sgn(mpq_numref(r_value)) == -1) return 0;
			return ULONG_MAX;
		}
		return mpz_get_ui(mpq_numref(r_value));
	} else {
		Number nr;std::cout << "Number.cc-1642" << endl;
		nr.set(*this, false, true);std::cout << "Number.cc-1643" << endl;
		nr.round();std::cout << "Number.cc-1644" << endl;
		return nr.ulintValue(overflow);
	}
}

bool Number::isApproximate() const {
	return b_approx;
}
bool Number::isFloatingPoint() const {
	return (n_type == NUMBER_TYPE_FLOAT);
}
bool Number::isInterval(bool ignore_imag) const {
	return (n_type == NUMBER_TYPE_FLOAT && !mpfr_equal_p(fl_value, fu_value)) || (!ignore_imag && i_value && i_value->isInterval());
}
bool Number::imaginaryPartIsInterval() const {
	return i_value && i_value->isInterval();
}
void Number::setApproximate(bool is_approximate) {
	if(is_approximate != isApproximate()) {
		if(is_approximate) {
			//i_precision = PRECISION;std::cout << "Number.cc-1664" << endl;
			b_approx = true;std::cout << "Number.cc-1665" << endl;
		} else {
			i_precision = -1;std::cout << "Number.cc-1667" << endl;
			b_approx = false;std::cout << "Number.cc-1668" << endl;
		}
	}
}

int Number::precision(int calculate_from_interval) const {
	if(calculate_from_interval < 0) {
		int iv_prec = precision(1);std::cout << "Number.cc-1675" << endl;
		if(i_precision < 0 || iv_prec < i_precision) return iv_prec;
	} else if(calculate_from_interval > 0) {
		if(n_type == NUMBER_TYPE_FLOAT && !mpfr_equal_p(fl_value, fu_value)) {
			mpfr_clear_flags();std::cout << "Number.cc-1679" << endl;
			mpfr_t f_diff, f_mid;
			mpfr_inits2(mpfr_get_prec(fl_value), f_diff, f_mid, NULL);
			mpfr_sub(f_diff, fu_value, fl_value, MPFR_RNDN);
			mpfr_div_ui(f_diff, f_diff, 2, MPFR_RNDN);
			mpfr_add(f_mid, fl_value, f_diff, MPFR_RNDN);
			mpfr_mul_ui(f_diff, f_diff, 2, MPFR_RNDN);
			mpfr_div(f_diff, f_mid, f_diff, MPFR_RNDN);
			mpfr_abs(f_diff, f_diff, MPFR_RNDN);
			if(!mpfr_zero_p(f_diff)) mpfr_log10(f_diff, f_diff, MPFR_RNDN);
			int i_prec = -1;std::cout << "Number.cc-1689" << endl;
			if(mpfr_sgn(f_diff) <= 0 || testErrors(0)) {
				i_prec = 0;std::cout << "Number.cc-1691" << endl;
			} else if(mpfr_fits_sint_p(f_diff, MPFR_RNDU)) {
				i_prec = mpfr_get_si(f_diff, MPFR_RNDD) + 1;
			}
			if(i_value && i_prec != 0) {
				int imag_prec = i_value->precision(1);std::cout << "Number.cc-1696" << endl;
				if(imag_prec >= 0 && (i_prec < 0 || imag_prec < i_prec)) i_prec = imag_prec;
			}
			mpfr_clears(f_diff, f_mid, NULL);
			return i_prec;

		} else if(i_value) return i_value->precision(1);
		return -1;
	}
	return i_precision;
}
void Number::setPrecision(int prec) {
	i_precision = prec;std::cout << "Number.cc-1708" << endl;
	if(i_precision >= 0) b_approx = true;
}

bool Number::isUndefined() const {
	return false;
}
bool Number::isInfinite(bool ignore_imag) const {
	return n_type >= NUMBER_TYPE_PLUS_INFINITY && (ignore_imag || !i_value || i_value->isZero());
}
bool Number::isPlusInfinity(bool ignore_imag) const {
	return n_type == NUMBER_TYPE_PLUS_INFINITY && (ignore_imag || !i_value || i_value->isZero());
}
bool Number::isMinusInfinity(bool ignore_imag) const {
	return n_type == NUMBER_TYPE_MINUS_INFINITY && (ignore_imag || !i_value || i_value->isZero());
}
bool Number::includesInfinity(bool ignore_imag) const {
	return n_type >= NUMBER_TYPE_PLUS_INFINITY || (n_type == NUMBER_TYPE_FLOAT && (mpfr_inf_p(fl_value) || mpfr_inf_p(fu_value))) || (!ignore_imag && i_value && i_value->includesInfinity());
}
bool Number::includesPlusInfinity() const {
	return n_type == NUMBER_TYPE_PLUS_INFINITY || (n_type == NUMBER_TYPE_FLOAT && (mpfr_inf_p(fu_value) && mpfr_sgn(fu_value) >= 0));
}
bool Number::includesMinusInfinity() const {
	return n_type == NUMBER_TYPE_MINUS_INFINITY || (n_type == NUMBER_TYPE_FLOAT && (mpfr_inf_p(fl_value) && mpfr_sgn(fl_value) < 0));
}

Number Number::realPart() const {
	Number real_part;std::cout << "Number.cc-1735" << endl;
	real_part.set(*this, true, true);std::cout << "Number.cc-1736" << endl;
	return real_part;
}
Number Number::imaginaryPart() const {
	if(!i_value) return Number();
	return *i_value;
}
Number Number::lowerEndPoint(bool include_imag) const {
	if(i_value && !i_value->isZero()) {
		if(include_imag) {
			if(!isInterval(false)) return *this;
			Number nr;std::cout << "Number.cc-1747" << endl;
			if(isInterval(true)) nr.setInternal(fl_value);
			else nr.set(realPart());std::cout << "Number.cc-1749" << endl;
			nr.setImaginaryPart(i_value->lowerEndPoint());std::cout << "Number.cc-1750" << endl;
			nr.setPrecisionAndApproximateFrom(*this);std::cout << "Number.cc-1751" << endl;
			return nr;
		}
		if(!isInterval()) return realPart();
	} else if(!isInterval()) return *this;
	Number nr;std::cout << "Number.cc-1756" << endl;
	nr.setInternal(fl_value);std::cout << "Number.cc-1757" << endl;
	nr.setPrecisionAndApproximateFrom(*this);std::cout << "Number.cc-1758" << endl;
	return nr;
}
Number Number::upperEndPoint(bool include_imag) const {
	if(i_value && !i_value->isZero()) {
		if(include_imag) {
			if(!isInterval(false)) return *this;
			Number nr;std::cout << "Number.cc-1765" << endl;
			if(isInterval(true)) nr.setInternal(fu_value);
			else nr.set(realPart());std::cout << "Number.cc-1767" << endl;
			nr.setImaginaryPart(i_value->upperEndPoint());std::cout << "Number.cc-1768" << endl;
			nr.setPrecisionAndApproximateFrom(*this);std::cout << "Number.cc-1769" << endl;
			return nr;
		}
		if(!isInterval()) return realPart();
	} else if(!isInterval()) return *this;
	Number nr;std::cout << "Number.cc-1774" << endl;
	nr.setInternal(fu_value);std::cout << "Number.cc-1775" << endl;
	nr.setPrecisionAndApproximateFrom(*this);std::cout << "Number.cc-1776" << endl;
	return nr;
}
Number Number::numerator() const {
	Number num;std::cout << "Number.cc-1780" << endl;
	num.setInternal(mpq_numref(r_value));std::cout << "Number.cc-1781" << endl;
	return num;
}
Number Number::denominator() const {
	Number den;std::cout << "Number.cc-1785" << endl;
	den.setInternal(mpq_denref(r_value));std::cout << "Number.cc-1786" << endl;
	return den;
}
Number Number::complexNumerator() const {
	Number num;std::cout << "Number.cc-1790" << endl;
	if(hasImaginaryPart()) num.setInternal(mpq_numref(i_value->internalRational()));
	return num;
}
Number Number::complexDenominator() const {
	Number den(1, 0);std::cout << "Number.cc-1795" << endl;
	if(hasImaginaryPart()) den.setInternal(mpq_denref(i_value->internalRational()));
	return den;
}

void Number::operator = (const Number &o) {set(o);}
void Number::operator = (long int i) {set(i, 1);}
void Number::operator -- (int) {
	if(n_type == NUMBER_TYPE_RATIONAL) {
		mpz_sub(mpq_numref(r_value), mpq_numref(r_value), mpq_denref(r_value));std::cout << "Number.cc-1804" << endl;
	} else if(n_type == NUMBER_TYPE_FLOAT) {
		if(!CREATE_INTERVAL && !isInterval()) {
			mpfr_sub_ui(fl_value, fl_value, 1, MPFR_RNDN);std::cout << "Number.cc-1807" << endl;
			mpfr_set(fu_value, fl_value, MPFR_RNDN);std::cout << "Number.cc-1808" << endl;
		} else {
			mpfr_sub_ui(fu_value, fu_value, 1, MPFR_RNDU);std::cout << "Number.cc-1810" << endl;
			mpfr_sub_ui(fl_value, fl_value, 1, MPFR_RNDD);std::cout << "Number.cc-1811" << endl;
		}
	}
}
void Number::operator ++ (int) {
	if(n_type == NUMBER_TYPE_RATIONAL) {
		mpz_add(mpq_numref(r_value), mpq_numref(r_value), mpq_denref(r_value));std::cout << "Number.cc-1817" << endl;
	} else if(n_type == NUMBER_TYPE_FLOAT) {
		if(!CREATE_INTERVAL && !isInterval()) {
			mpfr_add_ui(fl_value, fl_value, 1, MPFR_RNDN);std::cout << "Number.cc-1820" << endl;
			mpfr_set(fu_value, fl_value, MPFR_RNDN);std::cout << "Number.cc-1821" << endl;
		} else {
			mpfr_add_ui(fu_value, fu_value, 1, MPFR_RNDU);std::cout << "Number.cc-1823" << endl;
			mpfr_add_ui(fl_value, fl_value, 1, MPFR_RNDD);std::cout << "Number.cc-1824" << endl;
		}
	}
}
Number Number::operator - () const {Number o(*this); o.negate(); return o;}
Number Number::operator * (const Number &o) const {Number o2(*this); o2.multiply(o); return o2;}
Number Number::operator / (const Number &o) const {Number o2(*this); o2.divide(o); return o2;}
Number Number::operator + (const Number &o) const {Number o2(*this); o2.add(o); return o2;}
Number Number::operator - (const Number &o) const {Number o2(*this); o2.subtract(o); return o2;}
Number Number::operator ^ (const Number &o) const {Number o2(*this); o2.raise(o); return o2;}
Number Number::operator * (long int i) const {Number o2(*this); o2.multiply(i); return o2;}
Number Number::operator / (long int i) const {Number o2(*this); o2.divide(i); return o2;}
Number Number::operator + (long int i) const {Number o2(*this); o2.add(i); return o2;}
Number Number::operator - (long int i) const {Number o2(*this); o2.subtract(i); return o2;}
Number Number::operator ^ (long int i) const {Number o2(*this); o2.raise(i); return o2;}
Number Number::operator && (const Number &o) const {Number o2(*this); o2.add(o, OPERATION_LOGICAL_AND); return o2;}
Number Number::operator || (const Number &o) const {Number o2(*this); o2.add(o, OPERATION_LOGICAL_OR); return o2;}
Number Number::operator ! () const {Number o(*this); o.setLogicalNot(); return o;}

void Number::operator *= (const Number &o) {multiply(o);}
void Number::operator /= (const Number &o) {divide(o);}
void Number::operator += (const Number &o) {add(o);}
void Number::operator -= (const Number &o) {subtract(o);}
void Number::operator ^= (const Number &o) {raise(o);}
void Number::operator *= (long int i) {multiply(i);}
void Number::operator /= (long int i) {divide(i);}
void Number::operator += (long int i) {add(i);}
void Number::operator -= (long int i) {subtract(i);}
void Number::operator ^= (long int i) {raise(i);}

bool Number::operator == (const Number &o) const {return equals(o);}
bool Number::operator != (const Number &o) const {return !equals(o);}
bool Number::operator < (const Number &o) const {return isLessThan(o);}
bool Number::operator <= (const Number &o) const {return isLessThanOrEqualTo(o);}
bool Number::operator > (const Number &o) const {return isGreaterThan(o);}
bool Number::operator >= (const Number &o) const {return isGreaterThanOrEqualTo(o);}
bool Number::operator == (long int i) const {return equals(i);}
bool Number::operator != (long int i) const {return !equals(i);}
bool Number::operator < (long int i) const {return isLessThan(i);}
bool Number::operator <= (long int i) const {return isLessThanOrEqualTo(i);}
bool Number::operator > (long int i) const {return isGreaterThan(i);}
bool Number::operator >= (long int i) const {return isGreaterThanOrEqualTo(i);}

bool Number::bitAnd(const Number &o) {
	if(!o.isInteger() || !isInteger()) return false;
	mpz_and(mpq_numref(r_value), mpq_numref(r_value), mpq_numref(o.internalRational()));std::cout << "Number.cc-1869" << endl;
	setPrecisionAndApproximateFrom(o);std::cout << "Number.cc-1870" << endl;
	return true;
}
bool Number::bitOr(const Number &o) {
	if(!o.isInteger() || !isInteger()) return false;
	mpz_ior(mpq_numref(r_value), mpq_numref(r_value), mpq_numref(o.internalRational()));std::cout << "Number.cc-1875" << endl;
	setPrecisionAndApproximateFrom(o);std::cout << "Number.cc-1876" << endl;
	return true;
}
bool Number::bitXor(const Number &o) {
	if(!o.isInteger() || !isInteger()) return false;
	mpz_xor(mpq_numref(r_value), mpq_numref(r_value), mpq_numref(o.internalRational()));std::cout << "Number.cc-1881" << endl;
	setPrecisionAndApproximateFrom(o);std::cout << "Number.cc-1882" << endl;
	return true;
}
bool Number::bitNot() {
	if(!isInteger()) return false;
	mpz_com(mpq_numref(r_value), mpq_numref(r_value));std::cout << "Number.cc-1887" << endl;
	return true;
}
bool Number::bitCmp(unsigned int bits) {
	if(!isInteger()) return false;
	if(isNegative()) {
		return negate() && subtract(1);
	}
	for(unsigned int i = 0; i < bits; i++) {
		mpz_combit(mpq_numref(r_value), i);std::cout << "Number.cc-1896" << endl;
	}
	return true;
}
bool Number::bitEqv(const Number &o) {
	if(!o.isInteger() || !isInteger()) return false;
	bitXor(o);std::cout << "Number.cc-1902" << endl;
	bitNot();std::cout << "Number.cc-1903" << endl;
	setPrecisionAndApproximateFrom(o);std::cout << "Number.cc-1904" << endl;
	return true;
}
bool Number::shiftLeft(const Number &o) {
	if(!o.isInteger() || !isInteger() || o.isNegative()) return false;
	bool overflow = false;std::cout << "Number.cc-1909" << endl;
	long int y = o.lintValue(&overflow);std::cout << "Number.cc-1910" << endl;
	if(overflow) return false;
	mpz_mul_2exp(mpq_numref(r_value), mpq_numref(r_value), (unsigned long int) y);std::cout << "Number.cc-1912" << endl;
	setPrecisionAndApproximateFrom(o);std::cout << "Number.cc-1913" << endl;
	return true;
}
bool Number::shiftRight(const Number &o) {
	if(!o.isInteger() || !isInteger() || o.isNegative()) return false;
	bool overflow = false;std::cout << "Number.cc-1918" << endl;
	long int y = o.lintValue(&overflow);std::cout << "Number.cc-1919" << endl;
	if(overflow) return false;
	mpz_fdiv_q_2exp(mpq_numref(r_value), mpq_numref(r_value), (unsigned long int) y);std::cout << "Number.cc-1921" << endl;
	setPrecisionAndApproximateFrom(o);std::cout << "Number.cc-1922" << endl;
	return true;
}
bool Number::shift(const Number &o) {
	if(!o.isInteger() || !isInteger()) return false;
	bool overflow = false;std::cout << "Number.cc-1927" << endl;
	long int y = o.lintValue(&overflow);std::cout << "Number.cc-1928" << endl;
	if(overflow) return false;
	if(y < 0) mpz_fdiv_q_2exp(mpq_numref(r_value), mpq_numref(r_value), (unsigned long int) -y);
	else mpz_mul_2exp(mpq_numref(r_value), mpq_numref(r_value), (unsigned long int) y);std::cout << "Number.cc-1931" << endl;
	setPrecisionAndApproximateFrom(o);std::cout << "Number.cc-1932" << endl;
	return true;
}

bool Number::hasRealPart() const {
	if(isInfinite(true)) return true;
	if(n_type == NUMBER_TYPE_RATIONAL) return mpq_sgn(r_value) != 0;
	return !mpfr_zero_p(fu_value) || !mpfr_zero_p(fl_value);
}
bool Number::hasImaginaryPart() const {
	return i_value && !i_value->isZero();
}
bool Number::testErrors(int error_level) const {
	if(mpfr_underflow_p()) {if(error_level) CALCULATOR->error(error_level > 1, _("Floating point underflow"), NULL); return true;}
	if(mpfr_overflow_p()) {if(error_level) CALCULATOR->error(error_level > 1, _("Floating point overflow"), NULL); return true;}
	if(mpfr_divby0_p()) {if(error_level) CALCULATOR->error(error_level > 1, _("Floating point division by zero exception"), NULL); return true;}
	if(mpfr_nanflag_p()) {if(error_level) CALCULATOR->error(error_level > 1, _("Floating point not a number exception"), NULL); return true;}
	if(mpfr_erangeflag_p()) {if(error_level) CALCULATOR->error(error_level > 1, _("Floating point range exception"), NULL); return true;}
	return false;
}
bool testComplexZero(const Number *this_nr, const Number *i_nr) {
	if(!i_nr) return false;
	if(!this_nr->isInfinite(true) && !i_nr->isInfinite(true) && !i_nr->isZero() && !this_nr->isZero()) {
		if(i_nr->isFloatingPoint() && (!i_nr->isInterval() || !i_nr->isNonZero())) {
			mpfr_t thisf, testf;std::cout << "Number.cc-1956" << endl;
			mpfr_inits2(BIT_PRECISION - 10, thisf, testf, NULL);std::cout << "Number.cc-1957" << endl;
			bool b = true, b2 = false;std::cout << "Number.cc-1958" << endl;
			if(!this_nr->isInterval() || (!mpfr_zero_p(this_nr->internalLowerFloat()) && !mpfr_inf_p(this_nr->internalLowerFloat()))) {
				b2 = true;std::cout << "Number.cc-1960" << endl;
				if(this_nr->isFloatingPoint()) {
					mpfr_set(thisf, this_nr->internalLowerFloat(), MPFR_RNDN);std::cout << "Number.cc-1962" << endl;
				} else {
					mpfr_set_q(thisf, this_nr->internalRational(), MPFR_RNDN);std::cout << "Number.cc-1964" << endl;
				}
				mpfr_add(testf, thisf, i_nr->internalLowerFloat(), MPFR_RNDN);std::cout << "Number.cc-1966" << endl;
				b = mpfr_equal_p(thisf, testf);std::cout << "Number.cc-1967" << endl;
				if(b) {
					mpfr_add(testf, thisf, i_nr->internalUpperFloat(), MPFR_RNDN);std::cout << "Number.cc-1969" << endl;
					b = mpfr_equal_p(thisf, testf);std::cout << "Number.cc-1970" << endl;
				}
			}
			if(b && this_nr->isInterval() && !mpfr_zero_p(this_nr->internalUpperFloat()) && !mpfr_inf_p(this_nr->internalUpperFloat())) {
				b2 = true;std::cout << "Number.cc-1974" << endl;
				mpfr_set(thisf, this_nr->internalUpperFloat(), MPFR_RNDN);std::cout << "Number.cc-1975" << endl;
				mpfr_add(testf, thisf, i_nr->internalLowerFloat(), MPFR_RNDN);std::cout << "Number.cc-1976" << endl;
				b = mpfr_equal_p(thisf, testf);std::cout << "Number.cc-1977" << endl;
				if(b) {
					mpfr_add(testf, thisf, i_nr->internalUpperFloat(), MPFR_RNDN);std::cout << "Number.cc-1979" << endl;
					b = mpfr_equal_p(thisf, testf);std::cout << "Number.cc-1980" << endl;
				}
			}
			mpfr_clears(thisf, testf, NULL);std::cout << "Number.cc-1983" << endl;
			if(b && b2) {
				return true;
			}
		}
	}
	return false;
}

bool testComplex(Number *this_nr, Number *i_nr) {
	if(!i_nr) return false;
	if(!this_nr->isInfinite(true) && !i_nr->isInfinite(true) && !i_nr->isZero() && !this_nr->isZero()) {
		if(i_nr->isFloatingPoint() && (!i_nr->isInterval() || !i_nr->isNonZero())) {
			mpfr_t thisf, testf;std::cout << "Number.cc-1996" << endl;
			mpfr_inits2(BIT_PRECISION - 10, thisf, testf, NULL);std::cout << "Number.cc-1997" << endl;
			bool b = true, b2 = false;std::cout << "Number.cc-1998" << endl;
			if(!this_nr->isInterval() || (!mpfr_zero_p(this_nr->internalLowerFloat()) && !mpfr_inf_p(this_nr->internalLowerFloat()))) {
				b2 = true;std::cout << "Number.cc-2000" << endl;
				if(this_nr->isFloatingPoint()) {
					mpfr_set(thisf, this_nr->internalLowerFloat(), MPFR_RNDN);std::cout << "Number.cc-2002" << endl;
				} else {
					mpfr_set_q(thisf, this_nr->internalRational(), MPFR_RNDN);std::cout << "Number.cc-2004" << endl;
				}
				mpfr_add(testf, thisf, i_nr->internalLowerFloat(), MPFR_RNDN);std::cout << "Number.cc-2006" << endl;
				b = mpfr_equal_p(thisf, testf);std::cout << "Number.cc-2007" << endl;
				if(b) {
					mpfr_add(testf, thisf, i_nr->internalUpperFloat(), MPFR_RNDN);std::cout << "Number.cc-2009" << endl;
					b = mpfr_equal_p(thisf, testf);std::cout << "Number.cc-2010" << endl;
				}
			}
			if(b && this_nr->isInterval() && !mpfr_zero_p(this_nr->internalUpperFloat()) && !mpfr_inf_p(this_nr->internalUpperFloat())) {
				b2 = true;std::cout << "Number.cc-2014" << endl;
				mpfr_set(thisf, this_nr->internalUpperFloat(), MPFR_RNDN);std::cout << "Number.cc-2015" << endl;
				mpfr_add(testf, thisf, i_nr->internalLowerFloat(), MPFR_RNDN);std::cout << "Number.cc-2016" << endl;
				b = mpfr_equal_p(thisf, testf);std::cout << "Number.cc-2017" << endl;
				if(b) {
					mpfr_add(testf, thisf, i_nr->internalUpperFloat(), MPFR_RNDN);std::cout << "Number.cc-2019" << endl;
					b = mpfr_equal_p(thisf, testf);std::cout << "Number.cc-2020" << endl;
				}
			}
			mpfr_clears(thisf, testf, NULL);std::cout << "Number.cc-2023" << endl;
			if(b && b2) {
				i_nr->clear(true);std::cout << "Number.cc-2025" << endl;
				return true;
			}
		}
		if(this_nr->isFloatingPoint() && (!this_nr->isInterval() || !this_nr->realPartIsNonZero())) {
			mpfr_t thisf, testf;std::cout << "Number.cc-2030" << endl;
			mpfr_inits2(BIT_PRECISION - 10, thisf, testf, NULL);std::cout << "Number.cc-2031" << endl;
			bool b = true, b2 = false;std::cout << "Number.cc-2032" << endl;
			if(!this_nr->isInterval() || (!mpfr_zero_p(i_nr->internalLowerFloat()) && !mpfr_inf_p(i_nr->internalLowerFloat()))) {
				b2 = true;std::cout << "Number.cc-2034" << endl;
				if(i_nr->isFloatingPoint()) {
					mpfr_set(thisf, i_nr->internalLowerFloat(), MPFR_RNDN);std::cout << "Number.cc-2036" << endl;
				} else {
					mpfr_set_q(thisf, i_nr->internalRational(), MPFR_RNDN);std::cout << "Number.cc-2038" << endl;
				}
				mpfr_add(testf, thisf, this_nr->internalLowerFloat(), MPFR_RNDN);std::cout << "Number.cc-2040" << endl;
				b = mpfr_equal_p(thisf, testf);std::cout << "Number.cc-2041" << endl;
				if(b) {
					mpfr_add(testf, thisf, this_nr->internalUpperFloat(), MPFR_RNDN);std::cout << "Number.cc-2043" << endl;
					b = mpfr_equal_p(thisf, testf);std::cout << "Number.cc-2044" << endl;
				}
			}
			if(b && i_nr->isInterval() && !mpfr_zero_p(i_nr->internalUpperFloat()) && !mpfr_inf_p(i_nr->internalUpperFloat())) {
				b2 = true;std::cout << "Number.cc-2048" << endl;
				mpfr_set(thisf, i_nr->internalUpperFloat(), MPFR_RNDN);std::cout << "Number.cc-2049" << endl;
				mpfr_add(testf, thisf, this_nr->internalLowerFloat(), MPFR_RNDN);std::cout << "Number.cc-2050" << endl;
				b = mpfr_equal_p(thisf, testf);std::cout << "Number.cc-2051" << endl;
				if(b) {
					mpfr_add(testf, thisf, this_nr->internalUpperFloat(), MPFR_RNDN);std::cout << "Number.cc-2053" << endl;
					b = mpfr_equal_p(thisf, testf);std::cout << "Number.cc-2054" << endl;
				}
			}
			mpfr_clears(thisf, testf, NULL);std::cout << "Number.cc-2057" << endl;
			if(b && b2) {
				this_nr->clearReal();std::cout << "Number.cc-2059" << endl;
				return true;
			}
		}
	}
	return false;
}
bool Number::testFloatResult(bool allow_infinite_result, int error_level, bool test_integer) {
	if(mpfr_underflow_p()) {if(error_level) CALCULATOR->error(error_level > 1, _("Floating point underflow"), NULL); return false;}
	if(mpfr_overflow_p()) {if(error_level) CALCULATOR->error(error_level > 1, _("Floating point overflow"), NULL); return false;}
	if(mpfr_divby0_p()) {if(error_level) CALCULATOR->error(error_level > 1, _("Floating point division by zero exception"), NULL); return false;}
	if(mpfr_erangeflag_p()) {if(error_level) CALCULATOR->error(error_level > 1, _("Floating point range exception"), NULL); return false;}
	if(mpfr_nan_p(fu_value) || mpfr_nan_p(fl_value)) return false;
	if(mpfr_nanflag_p()) {if(error_level) CALCULATOR->error(error_level > 1, _("Floating point not a number exception"), NULL); return false;}
	if(mpfr_inexflag_p()) {
		b_approx = true;std::cout << "Number.cc-2074" << endl;
		if(!CREATE_INTERVAL && !isInterval() && (i_precision < 0 || i_precision > FROM_BIT_PRECISION(BIT_PRECISION))) i_precision = FROM_BIT_PRECISION(BIT_PRECISION);
	}
	mpfr_clear_flags();std::cout << "Number.cc-2077" << endl;
	if(mpfr_inf_p(fl_value) && mpfr_inf_p(fu_value) && mpfr_sgn(fl_value) == mpfr_sgn(fu_value)) {
		if(!allow_infinite_result) return false;
		int sign = mpfr_sgn(fl_value);std::cout << "Number.cc-2080" << endl;
		if(sign >= 0) n_type = NUMBER_TYPE_PLUS_INFINITY;
		else if(sign < 0) n_type = NUMBER_TYPE_MINUS_INFINITY;
		mpfr_clears(fl_value, fu_value, NULL);std::cout << "Number.cc-2083" << endl;
	} else if(mpfr_inf_p(fl_value) || mpfr_inf_p(fu_value)) {
		if(!allow_infinite_result) return false;
	} else if(mpfr_cmp(fl_value, fu_value) > 0) {
		mpfr_swap(fl_value, fu_value);std::cout << "Number.cc-2087" << endl;
	}
	if(test_integer) testInteger();
	if(!b_imag) testComplex(this, i_value);
	return true;
}
void Number::testInteger() {
	if(isFloatingPoint()) {
		if(mpfr_equal_p(fu_value, fl_value)) {
			if(mpfr_integer_p(fl_value) && mpfr_integer_p(fu_value)) {
				mpfr_get_z(mpq_numref(r_value), fl_value, MPFR_RNDN);std::cout << "Number.cc-2097" << endl;
				mpfr_clears(fl_value, fu_value, NULL);std::cout << "Number.cc-2098" << endl;
				n_type = NUMBER_TYPE_RATIONAL;std::cout << "Number.cc-2099" << endl;
			}
		} else if(mpfr_zero_p(fu_value) && mpfr_zero_p(fl_value)) {
			mpfr_clears(fl_value, fu_value, NULL);std::cout << "Number.cc-2102" << endl;
			n_type = NUMBER_TYPE_RATIONAL;std::cout << "Number.cc-2103" << endl;
		}
	}
	if(i_value) i_value->testInteger();
}
void Number::setPrecisionAndApproximateFrom(const Number &o) {
	if(o.precision() >= 0 && (i_precision < 0 || o.precision() < i_precision)) i_precision = o.precision();
	if(o.isApproximate()) b_approx = true;
}

bool Number::isComplex() const {
	return i_value && i_value->isNonZero();
}
Number Number::integer() const {
	if(isInteger()) return *this;
	Number nr(*this);std::cout << "Number.cc-2118" << endl;
	nr.round();std::cout << "Number.cc-2119" << endl;
	return nr;
}
bool Number::isInteger(IntegerType integer_type) const {
	if(n_type != NUMBER_TYPE_RATIONAL || hasImaginaryPart()) return false;
	if(mpz_cmp_ui(mpq_denref(r_value), 1) != 0) return false;
	switch(integer_type) {
		case INTEGER_TYPE_NONE: {return true;}
		case INTEGER_TYPE_SIZE: {}
		case INTEGER_TYPE_UINT: {return mpz_fits_uint_p(mpq_numref(r_value)) != 0;}
		case INTEGER_TYPE_SINT: {return mpz_fits_sint_p(mpq_numref(r_value)) != 0;}
		case INTEGER_TYPE_ULONG: {return mpz_fits_ulong_p(mpq_numref(r_value)) != 0;}
		case INTEGER_TYPE_SLONG: {return mpz_fits_slong_p(mpq_numref(r_value)) != 0;}
	}
	return true;
}
bool Number::isRational() const {
	return n_type == NUMBER_TYPE_RATIONAL && (!i_value || i_value->isZero());
}
bool Number::realPartIsRational() const {
	return n_type == NUMBER_TYPE_RATIONAL;
}
bool Number::isReal() const {
	return !includesInfinity() && !hasImaginaryPart();
}
bool Number::isFraction() const {
	if(hasImaginaryPart()) return false;
	if(n_type == NUMBER_TYPE_RATIONAL) {
		return mpz_cmpabs(mpq_denref(r_value), mpq_numref(r_value)) > 0;
	} else if(n_type == NUMBER_TYPE_FLOAT) {
		bool frac_u = mpfr_cmp_ui(fu_value, 1) < 0 && mpfr_cmp_si(fu_value, -1) > 0;std::cout << "Number.cc-2149" << endl;
		bool frac_l = mpfr_cmp_ui(fl_value, 1) < 0 && mpfr_cmp_si(fl_value, -1) > 0;std::cout << "Number.cc-2150" << endl;
		return frac_u && frac_l;
	}
	return false;
}
bool Number::isZero() const {
	if(i_value && !i_value->isZero()) return false;
	if(n_type == NUMBER_TYPE_FLOAT) return mpfr_zero_p(fu_value) && mpfr_zero_p(fl_value);
	else if(n_type == NUMBER_TYPE_RATIONAL) return mpz_sgn(mpq_numref(r_value)) == 0;
	return false;
}
bool Number::isNonZero() const {
	if(i_value && i_value->isNonZero()) return true;
	if(n_type == NUMBER_TYPE_FLOAT) return !mpfr_zero_p(fu_value) && mpfr_sgn(fu_value) == mpfr_sgn(fl_value);
	else if(n_type == NUMBER_TYPE_RATIONAL) return mpz_sgn(mpq_numref(r_value)) != 0;
	return true;
}
bool Number::isOne() const {
	if(!isReal()) return false;
	if(n_type == NUMBER_TYPE_FLOAT) {return mpfr_cmp_ui(fu_value, 1) == 0 && mpfr_cmp_ui(fl_value, 1) == 0;}
	return mpz_cmp(mpq_denref(r_value), mpq_numref(r_value)) == 0;
}
bool Number::isTwo() const {
	if(!isReal()) return false;
	if(n_type == NUMBER_TYPE_FLOAT) {return mpfr_cmp_ui(fu_value, 2) == 0 && mpfr_cmp_ui(fl_value, 2) == 0;}
	return mpq_cmp_si(r_value, 2, 1) == 0;
}
bool Number::isI() const {
	if(!i_value || !i_value->isOne()) return false;
	if(n_type == NUMBER_TYPE_FLOAT) return mpfr_zero_p(fu_value) && mpfr_zero_p(fl_value);
	else if(n_type == NUMBER_TYPE_RATIONAL) return mpz_sgn(mpq_numref(r_value)) == 0;
	return false;
}
bool Number::isMinusOne() const {
	if(!isReal()) return false;
	if(n_type == NUMBER_TYPE_FLOAT) {return mpfr_cmp_si(fu_value, -1) == 0 && mpfr_cmp_si(fl_value, -1) == 0;}
	return mpq_cmp_si(r_value, -1, 1) == 0;
}
bool Number::isMinusI() const {
	if(!i_value || !i_value->isMinusOne()) return false;
	if(n_type == NUMBER_TYPE_FLOAT) return mpfr_zero_p(fu_value) && mpfr_zero_p(fl_value);
	else if(n_type == NUMBER_TYPE_RATIONAL) return mpz_sgn(mpq_numref(r_value)) == 0;
	return false;
}
bool Number::isNegative() const {
	if(hasImaginaryPart()) return false;
	if(n_type == NUMBER_TYPE_FLOAT) return mpfr_sgn(fu_value) < 0;
	else if(n_type == NUMBER_TYPE_RATIONAL) return mpz_sgn(mpq_numref(r_value)) < 0;
	else if(n_type == NUMBER_TYPE_MINUS_INFINITY) return true;
	return false;
}
bool Number::isNonNegative() const {
	if(hasImaginaryPart()) return false;
	if(n_type == NUMBER_TYPE_FLOAT) {return mpfr_sgn(fl_value) >= 0;}
	else if(n_type == NUMBER_TYPE_RATIONAL) return mpz_sgn(mpq_numref(r_value)) >= 0;
	else if(n_type == NUMBER_TYPE_PLUS_INFINITY) return true;
	return false;
}
bool Number::isPositive() const {
	if(hasImaginaryPart()) return false;
	if(n_type == NUMBER_TYPE_FLOAT) {return mpfr_sgn(fl_value) > 0;}
	else if(n_type == NUMBER_TYPE_RATIONAL) return mpz_sgn(mpq_numref(r_value)) > 0;
	else if(n_type == NUMBER_TYPE_PLUS_INFINITY) return true;
	return false;
}
bool Number::isNonPositive() const {
	if(hasImaginaryPart()) return false;
	if(n_type == NUMBER_TYPE_FLOAT) {return mpfr_sgn(fu_value) <= 0;}
	else if(n_type == NUMBER_TYPE_RATIONAL) return mpz_sgn(mpq_numref(r_value)) <= 0;
	else if(n_type == NUMBER_TYPE_MINUS_INFINITY) return true;
	return false;
}
bool Number::realPartIsNegative() const {
	if(n_type == NUMBER_TYPE_FLOAT) return mpfr_sgn(fu_value) < 0;
	else if(n_type == NUMBER_TYPE_RATIONAL) return mpz_sgn(mpq_numref(r_value)) < 0;
	else if(n_type == NUMBER_TYPE_MINUS_INFINITY) return true;
	return false;
}
bool Number::realPartIsPositive() const {
	if(n_type == NUMBER_TYPE_FLOAT) return mpfr_sgn(fl_value) > 0;
	else if(n_type == NUMBER_TYPE_RATIONAL) return mpz_sgn(mpq_numref(r_value)) > 0;
	else if(n_type == NUMBER_TYPE_PLUS_INFINITY) return true;
	return false;
}
bool Number::realPartIsNonNegative() const {
	if(n_type == NUMBER_TYPE_FLOAT) {return mpfr_sgn(fl_value) >= 0;}
	else if(n_type == NUMBER_TYPE_RATIONAL) return mpz_sgn(mpq_numref(r_value)) >= 0;
	else if(n_type == NUMBER_TYPE_PLUS_INFINITY) return true;
	return false;
}
bool Number::realPartIsNonZero() const {
	if(n_type == NUMBER_TYPE_FLOAT) return !mpfr_zero_p(fu_value) && mpfr_sgn(fu_value) == mpfr_sgn(fl_value);
	else if(n_type == NUMBER_TYPE_RATIONAL) return mpz_sgn(mpq_numref(r_value)) != 0;
	return true;
}
bool Number::imaginaryPartIsNegative() const {
	return i_value && i_value->isNegative();
}
bool Number::imaginaryPartIsPositive() const {
	return i_value && i_value->isPositive();
}
bool Number::imaginaryPartIsNonNegative() const {
	return i_value && i_value->isNonNegative();
}
bool Number::imaginaryPartIsNonPositive() const {
	return i_value && i_value->isNonPositive();
}
bool Number::imaginaryPartIsNonZero() const {
	return i_value && i_value->isNonZero();
}
bool Number::hasNegativeSign() const {
	if(hasRealPart()) return realPartIsNegative();
	return imaginaryPartIsNegative();
}
bool Number::hasPositiveSign() const {
	if(hasRealPart()) return realPartIsPositive();
	return imaginaryPartIsPositive();
}
bool Number::equalsZero() const {
	return isZero();
}
bool Number::equals(const Number &o, bool allow_interval, bool allow_infinite) const {
	if(!allow_infinite && (includesInfinity() || o.includesInfinity())) return false;
	if(o.hasImaginaryPart()) {
		if(!i_value || !i_value->equals(*o.internalImaginary(), allow_interval, allow_infinite)) return false;
	} else if(hasImaginaryPart()) {
		return false;
	}
	if(allow_infinite) {
		if(o.isPlusInfinity()) return isPlusInfinity();
		if(o.isMinusInfinity()) return isMinusInfinity();
	}
	if(o.isFloatingPoint() && n_type != NUMBER_TYPE_FLOAT) {
		return mpfr_cmp_q(o.internalLowerFloat(), r_value) == 0 && mpfr_cmp_q(o.internalUpperFloat(), r_value) == 0;
	} else if(n_type == NUMBER_TYPE_FLOAT) {
		if(o.isFloatingPoint()) return (allow_interval || mpfr_equal_p(fu_value, fl_value)) && mpfr_equal_p(fl_value, o.internalLowerFloat()) && mpfr_equal_p(fu_value, o.internalUpperFloat());
		else return mpfr_cmp_q(fu_value, o.internalRational()) == 0 && mpfr_cmp_q(fl_value, o.internalRational()) == 0;
	}
	return mpq_cmp(r_value, o.internalRational()) == 0;
}
bool Number::equals(long int i) const {
	if(hasImaginaryPart()) return false;
	if(n_type == NUMBER_TYPE_FLOAT) return mpfr_cmp_si(fl_value, i) == 0 && mpfr_cmp_si(fu_value, i) == 0;
	else if(n_type == NUMBER_TYPE_RATIONAL) return mpq_cmp_si(r_value, i, 1) == 0;
	return false;
}
int Number::equalsApproximately(const Number &o, int prec) const {
	if(includesInfinity() || o.includesInfinity()) return false;
	if(o.isInterval() && !isInterval()) return o.equalsApproximately(*this, prec);
	if(equals(o)) return true;
	int b = 1;std::cout << "Number.cc-2300" << endl;
	if(o.hasImaginaryPart()) {
		if(i_value) {
			b = i_value->equalsApproximately(*o.internalImaginary(), prec);std::cout << "Number.cc-2303" << endl;
			if(b == 0) return b;
		} else {
			b = o.internalImaginary()->equalsApproximately(nr_zero, prec);std::cout << "Number.cc-2306" << endl;
			if(b == 0) return b;
		}
	} else if(hasImaginaryPart()) {
		b = i_value->equalsApproximately(nr_zero, prec);std::cout << "Number.cc-2310" << endl;
		if(b == 0) return b;
	}
	bool prec_choosen = prec >= 0;std::cout << "Number.cc-2313" << endl;
	if(prec == EQUALS_PRECISION_LOWEST) {
		prec = PRECISION;std::cout << "Number.cc-2315" << endl;
		if(i_precision >= 0 && i_precision < prec) prec = i_precision;
		if(o.precision() >= 0 && o.precision() < prec) prec = o.precision();
	} else if(prec == EQUALS_PRECISION_HIGHEST) {
		prec = i_precision;std::cout << "Number.cc-2319" << endl;
		if(o.precision() >= 0 && o.precision() > prec) prec = o.precision();
		if(prec < 0) prec = PRECISION;
	} else if(prec == EQUALS_PRECISION_DEFAULT) {
		prec = PRECISION;std::cout << "Number.cc-2323" << endl;
	}
	if(prec_choosen || isApproximate() || o.isApproximate()) {
		int b2 = 0;std::cout << "Number.cc-2326" << endl;
		if(isInterval()) {
			if(o.isInterval()) {
				mpfr_t test1, test2, test3, test4;std::cout << "Number.cc-2329" << endl;
				mpfr_inits2(::ceil(prec * 3.3219281), test1, test2, test3, test4, NULL);std::cout << "Number.cc-2330" << endl;
				if(!isNonZero() || !o.isNonZero()) {
					mpfr_add_ui(test1, fl_value, 1, MPFR_RNDN);std::cout << "Number.cc-2332" << endl;
					mpfr_add_ui(test2, fu_value, 1, MPFR_RNDN);std::cout << "Number.cc-2333" << endl;
					mpfr_add_ui(test3, o.internalLowerFloat(), 1, MPFR_RNDN);std::cout << "Number.cc-2334" << endl;
					mpfr_add_ui(test4, o.internalUpperFloat(), 1, MPFR_RNDN);std::cout << "Number.cc-2335" << endl;
				} else {
					mpfr_set(test1, fl_value, MPFR_RNDN);std::cout << "Number.cc-2337" << endl;
					mpfr_set(test2, fu_value, MPFR_RNDN);std::cout << "Number.cc-2338" << endl;
					mpfr_set(test3, o.internalLowerFloat(), MPFR_RNDN);std::cout << "Number.cc-2339" << endl;
					mpfr_set(test4, o.internalUpperFloat(), MPFR_RNDN);std::cout << "Number.cc-2340" << endl;
				}
				if(mpfr_equal_p(test1, test2) && mpfr_equal_p(test2, test3) && mpfr_equal_p(test3, test4)) b2 = 1;
				else if(mpfr_cmp(test1, test4) > 0 || mpfr_cmp(test2, test3) < 0) b2 = 0;
				else b2 = -1;std::cout << "Number.cc-2344" << endl;
				mpfr_clears(test1, test2, test3, test4, NULL);std::cout << "Number.cc-2345" << endl;
			} else if(!isNonZero() && o.isZero()) {
				mpfr_t test1, test2;std::cout << "Number.cc-2347" << endl;
				mpfr_inits2(::ceil(prec * 3.3219281), test1, test2, NULL);std::cout << "Number.cc-2348" << endl;
				mpfr_add_ui(test1, fl_value, 1, MPFR_RNDN);std::cout << "Number.cc-2349" << endl;
				mpfr_add_ui(test2, fu_value, 1, MPFR_RNDN);std::cout << "Number.cc-2350" << endl;
				if(mpfr_equal_p(test1, test2)) b2 = true;
				mpfr_clears(test1, test2, NULL);std::cout << "Number.cc-2352" << endl;
			} else {
				mpfr_t test1, test2, test3;std::cout << "Number.cc-2354" << endl;
				mpfr_inits2(::ceil(prec * 3.3219281), test1, test2, test3, NULL);std::cout << "Number.cc-2355" << endl;
				mpfr_set(test1, fl_value, MPFR_RNDN);std::cout << "Number.cc-2356" << endl;
				mpfr_set(test2, fu_value, MPFR_RNDN);std::cout << "Number.cc-2357" << endl;
				if(o.isFloatingPoint()) {
					mpfr_set(test3, o.internalLowerFloat(), MPFR_RNDN);std::cout << "Number.cc-2359" << endl;
				} else {
					mpfr_set_q(test3, o.internalRational(), MPFR_RNDN);std::cout << "Number.cc-2361" << endl;
				}
				if(mpfr_equal_p(test1, test2) && mpfr_equal_p(test2, test3)) b2 = 1;
				else if(mpfr_cmp(test3, test1) < 0 || mpfr_cmp(test3, test2) > 0) b2 = 0;
				else b2 = -1;std::cout << "Number.cc-2365" << endl;
				mpfr_clears(test1, test2, test3, NULL);std::cout << "Number.cc-2366" << endl;
			}
		} else {
			mpfr_t test1, test2;std::cout << "Number.cc-2369" << endl;
			mpfr_inits2(::ceil(prec * 3.3219281), test1, test2, NULL);std::cout << "Number.cc-2370" << endl;
			if(n_type == NUMBER_TYPE_FLOAT) {
				mpfr_set(test1, fl_value, MPFR_RNDN);std::cout << "Number.cc-2372" << endl;
			} else {
				mpfr_set_q(test1, r_value, MPFR_RNDN);std::cout << "Number.cc-2374" << endl;
			}
			if(o.isFloatingPoint()) {
				mpfr_set(test2, o.internalLowerFloat(), MPFR_RNDN);std::cout << "Number.cc-2377" << endl;
			} else {
				mpfr_set_q(test2, o.internalRational(), MPFR_RNDN);std::cout << "Number.cc-2379" << endl;
			}
			if(mpfr_equal_p(test1, test2)) {
				b2 = 1;std::cout << "Number.cc-2382" << endl;
			} else {
				mpfr_add_ui(test1, test1, 1, MPFR_RNDN);std::cout << "Number.cc-2384" << endl;
				mpfr_add_ui(test2, test2, 1, MPFR_RNDN);std::cout << "Number.cc-2385" << endl;
				if(mpfr_equal_p(test1, test2)) b2 = -1;
			}
			mpfr_clears(test1, test2, NULL);std::cout << "Number.cc-2388" << endl;
		}
		if(b2 < 0) b = -1;
		if(b2 == 0) b = 0;
		return b;
	}
	if(b == 1) b = 0;
	return b;
}
ComparisonResult Number::compare(long int i) const {return compare(Number(i, 1));}
ComparisonResult Number::compare(const Number &o, bool ignore_imag) const {
	if(isPlusInfinity()) {
		if((!ignore_imag && o.hasImaginaryPart()) || o.includesPlusInfinity()) return COMPARISON_RESULT_UNKNOWN;
		else return COMPARISON_RESULT_LESS;
	}
	if(isMinusInfinity()) {
		if((!ignore_imag && o.hasImaginaryPart()) || o.includesMinusInfinity()) return COMPARISON_RESULT_UNKNOWN;
		else return COMPARISON_RESULT_GREATER;
	}
	if(o.isPlusInfinity()) {
		if((!ignore_imag && hasImaginaryPart()) || includesPlusInfinity()) return COMPARISON_RESULT_UNKNOWN;
		return COMPARISON_RESULT_GREATER;
	}
	if(o.isMinusInfinity()) {
		if((!ignore_imag && hasImaginaryPart()) || includesMinusInfinity()) return COMPARISON_RESULT_UNKNOWN;
		return COMPARISON_RESULT_LESS;
	}
	if(!ignore_imag && equals(o)) return COMPARISON_RESULT_EQUAL;
	if(ignore_imag || (!hasImaginaryPart() && !o.hasImaginaryPart())) {
		int i = 0, i2 = 0;std::cout << "Number.cc-2417" << endl;
		if(o.isFloatingPoint() && n_type != NUMBER_TYPE_FLOAT) {
			i = mpfr_cmp_q(o.internalLowerFloat(), r_value);std::cout << "Number.cc-2419" << endl;
			i2 = mpfr_cmp_q(o.internalUpperFloat(), r_value);std::cout << "Number.cc-2420" << endl;
			if(i != i2) return COMPARISON_RESULT_CONTAINS;
		} else if(n_type == NUMBER_TYPE_FLOAT) {
			if(o.isFloatingPoint()) {
				i = mpfr_cmp(o.internalUpperFloat(), fl_value);std::cout << "Number.cc-2424" << endl;
				i2 = mpfr_cmp(o.internalLowerFloat(), fu_value);std::cout << "Number.cc-2425" << endl;
				if(i != i2 && i2 <= 0 && i >= 0) {
					i = mpfr_cmp(o.internalLowerFloat(), fl_value);std::cout << "Number.cc-2427" << endl;
					i2 = mpfr_cmp(o.internalUpperFloat(), fu_value);std::cout << "Number.cc-2428" << endl;
					if(i > 0) {
						if(i2 <= 0) return COMPARISON_RESULT_CONTAINED;
						else return COMPARISON_RESULT_OVERLAPPING_GREATER;
					} else if(i < 0) {
						if(i2 >= 0) return COMPARISON_RESULT_CONTAINS;
						else return COMPARISON_RESULT_OVERLAPPING_LESS;
					} else {
						if(i2 == 0) return COMPARISON_RESULT_EQUAL_LIMITS;
						else if(i2 > 0) return COMPARISON_RESULT_CONTAINS;
						else return COMPARISON_RESULT_CONTAINED;
					}
				}
			} else {
				i = -mpfr_cmp_q(fl_value, o.internalRational());std::cout << "Number.cc-2442" << endl;
				i2 = -mpfr_cmp_q(fu_value, o.internalRational());std::cout << "Number.cc-2443" << endl;
				if(i != i2) return COMPARISON_RESULT_CONTAINED;
			}
		} else {
			i = mpq_cmp(o.internalRational(), r_value);std::cout << "Number.cc-2447" << endl;
			i2 = i;std::cout << "Number.cc-2448" << endl;
		}
		if(i2 == 0 || i == 0) {
			if(i == 0) i = i2;
			if(i > 0) return COMPARISON_RESULT_EQUAL_OR_GREATER;
			else if(i < 0) return COMPARISON_RESULT_EQUAL_OR_LESS;
		} else if(i2 != i) {
			return COMPARISON_RESULT_UNKNOWN;
		}
		if(i == 0) return COMPARISON_RESULT_EQUAL;
		else if(i > 0) return COMPARISON_RESULT_GREATER;
		else return COMPARISON_RESULT_LESS;
	} else {
		if(hasImaginaryPart()) {
			if(o.hasImaginaryPart()) {
				ComparisonResult cr = realPart().compare(o.realPart());std::cout << "Number.cc-2463" << endl;
				if(COMPARISON_IS_NOT_EQUAL(cr)) return COMPARISON_RESULT_NOT_EQUAL;
				if(cr == COMPARISON_RESULT_EQUAL || COMPARISON_MIGHT_BE_EQUAL(cr)) {
					cr = imaginaryPart().compare(o.imaginaryPart());std::cout << "Number.cc-2466" << endl;
					if(COMPARISON_IS_NOT_EQUAL(cr)) return COMPARISON_RESULT_NOT_EQUAL;
					return COMPARISON_RESULT_UNKNOWN;
				}
			} else if(!i_value->isNonZero()) {
				ComparisonResult cr = realPart().compare(o.realPart());std::cout << "Number.cc-2471" << endl;
				if(COMPARISON_IS_NOT_EQUAL(cr)) return COMPARISON_RESULT_NOT_EQUAL;
				return COMPARISON_RESULT_UNKNOWN;
			}
		} else if(!o.imaginaryPartIsNonZero()) {
			ComparisonResult cr = realPart().compare(o.realPart());std::cout << "Number.cc-2476" << endl;
			if(COMPARISON_IS_NOT_EQUAL(cr)) return COMPARISON_RESULT_NOT_EQUAL;
			return COMPARISON_RESULT_UNKNOWN;
		}
		return COMPARISON_RESULT_NOT_EQUAL;
	}
}
ComparisonResult Number::compareAbsolute(const Number &o, bool ignore_imag) const {
	if(isPositive()) {
		if(o.isPositive()) return compare(o, ignore_imag);
		Number nr(o);std::cout << "Number.cc-2486" << endl;
		nr.negate();std::cout << "Number.cc-2487" << endl;
		return compare(nr, ignore_imag);
	} else if(o.isPositive()) {
		Number nr(*this);std::cout << "Number.cc-2490" << endl;
		nr.negate();std::cout << "Number.cc-2491" << endl;
		return nr.compare(o, ignore_imag);
	}
	if(!ignore_imag && (hasImaginaryPart() || o.hasImaginaryPart())) {
		Number nr1(*this);std::cout << "Number.cc-2495" << endl;
		nr1.negate();std::cout << "Number.cc-2496" << endl;
		Number nr2(o);std::cout << "Number.cc-2497" << endl;
		nr2.negate();std::cout << "Number.cc-2498" << endl;
		return nr1.compare(nr2, ignore_imag);
	}
	return o.compare(*this, ignore_imag);
}
ComparisonResult Number::compareApproximately(const Number &o, int prec) const {
	if(isPlusInfinity()) {
		if(o.hasImaginaryPart() || o.includesPlusInfinity()) return COMPARISON_RESULT_UNKNOWN;
		else return COMPARISON_RESULT_LESS;
	}
	if(isMinusInfinity()) {
		if(o.hasImaginaryPart() || o.includesMinusInfinity()) return COMPARISON_RESULT_UNKNOWN;
		else return COMPARISON_RESULT_GREATER;
	}
	if(o.isPlusInfinity()) {
		if(hasImaginaryPart() || includesPlusInfinity()) return COMPARISON_RESULT_UNKNOWN;
		return COMPARISON_RESULT_GREATER;
	}
	if(o.isMinusInfinity()) {
		if(hasImaginaryPart() || includesMinusInfinity()) return COMPARISON_RESULT_UNKNOWN;
		return COMPARISON_RESULT_LESS;
	}
	int b = equalsApproximately(o, prec);std::cout << "Number.cc-2520" << endl;
	if(b > 0) return COMPARISON_RESULT_EQUAL;
	else if(b < 0) return COMPARISON_RESULT_UNKNOWN;
	if(!hasImaginaryPart() && !o.hasImaginaryPart()) {
		int i = 0, i2 = 0;std::cout << "Number.cc-2524" << endl;
		if(o.isFloatingPoint() && n_type != NUMBER_TYPE_FLOAT) {
			i = mpfr_cmp_q(o.internalLowerFloat(), r_value);std::cout << "Number.cc-2526" << endl;
			i2 = mpfr_cmp_q(o.internalUpperFloat(), r_value);std::cout << "Number.cc-2527" << endl;
			if(i != i2) return COMPARISON_RESULT_CONTAINS;
		} else if(n_type == NUMBER_TYPE_FLOAT) {
			if(o.isFloatingPoint()) {
				i = mpfr_cmp(o.internalUpperFloat(), fl_value);std::cout << "Number.cc-2531" << endl;
				i2 = mpfr_cmp(o.internalLowerFloat(), fu_value);std::cout << "Number.cc-2532" << endl;
				if(i != i2 && i2 <= 0 && i >= 0) {
					i = mpfr_cmp(o.internalLowerFloat(), fl_value);std::cout << "Number.cc-2534" << endl;
					i2 = mpfr_cmp(o.internalUpperFloat(), fu_value);std::cout << "Number.cc-2535" << endl;
					if(i > 0) {
						if(i2 <= 0) return COMPARISON_RESULT_CONTAINED;
						else return COMPARISON_RESULT_OVERLAPPING_GREATER;
					} else if(i < 0) {
						if(i2 >= 0) return COMPARISON_RESULT_CONTAINS;
						else return COMPARISON_RESULT_OVERLAPPING_LESS;
					} else {
						if(i2 == 0) return COMPARISON_RESULT_EQUAL_LIMITS;
						else if(i2 > 0) return COMPARISON_RESULT_CONTAINS;
						else return COMPARISON_RESULT_CONTAINED;
					}
				}
			} else {
				i = -mpfr_cmp_q(fl_value, o.internalRational());std::cout << "Number.cc-2549" << endl;
				i2 = -mpfr_cmp_q(fu_value, o.internalRational());std::cout << "Number.cc-2550" << endl;
				if(i != i2) return COMPARISON_RESULT_CONTAINED;
			}
		} else {
			i = mpq_cmp(o.internalRational(), r_value);std::cout << "Number.cc-2554" << endl;
			i2 = i;std::cout << "Number.cc-2555" << endl;
		}
		if(i2 == 0 || i == 0) {
			if(i == 0) i = i2;
			if(i > 0) return COMPARISON_RESULT_EQUAL_OR_GREATER;
			else if(i < 0) return COMPARISON_RESULT_EQUAL_OR_LESS;
		} else if(i2 != i) {
			return COMPARISON_RESULT_UNKNOWN;
		}
		if(i == 0) return COMPARISON_RESULT_EQUAL;
		else if(i > 0) return COMPARISON_RESULT_GREATER;
		else return COMPARISON_RESULT_LESS;
	} else {
		if(hasImaginaryPart()) {
			if(o.hasImaginaryPart()) {
				ComparisonResult cr = realPart().compareApproximately(o.realPart());std::cout << "Number.cc-2570" << endl;
				if(COMPARISON_IS_NOT_EQUAL(cr)) return COMPARISON_RESULT_NOT_EQUAL;
				if(cr == COMPARISON_RESULT_EQUAL || COMPARISON_MIGHT_BE_EQUAL(cr)) {
					cr = imaginaryPart().compareApproximately(o.imaginaryPart());std::cout << "Number.cc-2573" << endl;
					if(COMPARISON_IS_NOT_EQUAL(cr)) return COMPARISON_RESULT_NOT_EQUAL;
					return COMPARISON_RESULT_UNKNOWN;
				}
			} else if(!i_value->isNonZero()) {
				ComparisonResult cr = realPart().compareApproximately(o.realPart());std::cout << "Number.cc-2578" << endl;
				if(COMPARISON_IS_NOT_EQUAL(cr)) return COMPARISON_RESULT_NOT_EQUAL;
				return COMPARISON_RESULT_UNKNOWN;
			}
		} else if(!o.imaginaryPartIsNonZero()) {
			ComparisonResult cr = realPart().compareApproximately(o.realPart());std::cout << "Number.cc-2583" << endl;
			if(COMPARISON_IS_NOT_EQUAL(cr)) return COMPARISON_RESULT_NOT_EQUAL;
			return COMPARISON_RESULT_UNKNOWN;
		}
		return COMPARISON_RESULT_NOT_EQUAL;
	}
}
ComparisonResult Number::compareImaginaryParts(const Number &o) const {
	if(o.hasImaginaryPart()) {
		if(!i_value) {
			if(o.imaginaryPartIsNonZero()) return COMPARISON_RESULT_NOT_EQUAL;
			return COMPARISON_RESULT_UNKNOWN;
		}
		return i_value->compareRealParts(*o.internalImaginary());
	} else if(hasImaginaryPart()) {
		if(i_value->isNonZero()) return COMPARISON_RESULT_NOT_EQUAL;
		return COMPARISON_RESULT_NOT_EQUAL;
	}
	return COMPARISON_RESULT_EQUAL;
}
ComparisonResult Number::compareRealParts(const Number &o) const {
	return compare(o, true);
}
bool Number::isGreaterThan(const Number &o) const {
	if(n_type == NUMBER_TYPE_MINUS_INFINITY || o.isPlusInfinity()) return false;
	if(o.isMinusInfinity()) return true;
	if(n_type == NUMBER_TYPE_PLUS_INFINITY) return true;
	if(hasImaginaryPart() || o.hasImaginaryPart()) return false;
	if(o.isFloatingPoint() && n_type != NUMBER_TYPE_FLOAT) {
		return mpfr_cmp_q(o.internalUpperFloat(), r_value) < 0;
	} else if(n_type == NUMBER_TYPE_FLOAT) {
		if(o.isFloatingPoint()) return mpfr_greater_p(fl_value, o.internalUpperFloat());
		else return mpfr_cmp_q(fl_value, o.internalRational()) > 0;
	}
	return mpq_cmp(r_value, o.internalRational()) > 0;
}
bool Number::isLessThan(const Number &o) const {
	if(o.isMinusInfinity() || n_type == NUMBER_TYPE_PLUS_INFINITY) return false;
	if(n_type == NUMBER_TYPE_MINUS_INFINITY || o.isPlusInfinity()) return true;
	if(hasImaginaryPart() || o.hasImaginaryPart()) return false;
	if(o.isFloatingPoint() && n_type != NUMBER_TYPE_FLOAT) {
		return mpfr_cmp_q(o.internalLowerFloat(), r_value) > 0;
	} else if(n_type == NUMBER_TYPE_FLOAT) {
		if(o.isFloatingPoint()) return mpfr_less_p(fu_value, o.internalLowerFloat());
		else return mpfr_cmp_q(fu_value, o.internalRational()) < 0;
	}
	return mpq_cmp(r_value, o.internalRational()) < 0;
}
bool Number::isGreaterThanOrEqualTo(const Number &o) const {
	if(n_type == NUMBER_TYPE_MINUS_INFINITY || o.isPlusInfinity()) return false;
	if(o.isMinusInfinity()) return true;
	if(n_type == NUMBER_TYPE_PLUS_INFINITY) return true;
	if(!hasImaginaryPart() && !o.hasImaginaryPart()) {
		if(o.isFloatingPoint() && n_type != NUMBER_TYPE_FLOAT) {
			return mpfr_cmp_q(o.internalUpperFloat(), r_value) <= 0;
		} else if(n_type == NUMBER_TYPE_FLOAT) {
			if(o.isFloatingPoint()) return mpfr_greaterequal_p(fl_value, o.internalUpperFloat());
			else return mpfr_cmp_q(fl_value, o.internalRational()) >= 0;
		}
		return mpq_cmp(r_value, o.internalRational()) >= 0;
	}
	return false;
}
bool Number::isLessThanOrEqualTo(const Number &o) const {
	if(o.isMinusInfinity() || n_type == NUMBER_TYPE_PLUS_INFINITY) return false;
	if(n_type == NUMBER_TYPE_MINUS_INFINITY || o.isPlusInfinity()) return true;
	if(!hasImaginaryPart() && !o.hasImaginaryPart()) {
		if(o.isFloatingPoint() && n_type != NUMBER_TYPE_FLOAT) {
			return mpfr_cmp_q(o.internalLowerFloat(), r_value) >= 0;
		} else if(n_type == NUMBER_TYPE_FLOAT) {
			if(o.isFloatingPoint()) return mpfr_lessequal_p(fu_value, o.internalLowerFloat());
			else return mpfr_cmp_q(fu_value, o.internalRational()) <= 0;
		}
		return mpq_cmp(r_value, o.internalRational()) <= 0;
	}
	return false;
}
bool Number::isGreaterThan(long int i) const {
	if(n_type == NUMBER_TYPE_MINUS_INFINITY) return false;
	if(n_type == NUMBER_TYPE_PLUS_INFINITY) return true;
	if(hasImaginaryPart()) return false;
	if(n_type == NUMBER_TYPE_FLOAT) {
		return mpfr_cmp_si(fl_value, i) > 0;
	}
	return mpq_cmp_si(r_value, i, 1) > 0;
}
bool Number::isLessThan(long int i) const {
	if(n_type == NUMBER_TYPE_PLUS_INFINITY) return false;
	if(n_type == NUMBER_TYPE_MINUS_INFINITY) return true;
	if(hasImaginaryPart()) return false;
	if(n_type == NUMBER_TYPE_FLOAT) {
		return mpfr_cmp_si(fu_value, i) < 0;
	}
	return mpq_cmp_si(r_value, i, 1) < 0;
}
bool Number::isGreaterThanOrEqualTo(long int i) const {
	if(n_type == NUMBER_TYPE_MINUS_INFINITY) return false;
	if(n_type == NUMBER_TYPE_PLUS_INFINITY) return true;
	if(hasImaginaryPart()) return false;
	if(n_type == NUMBER_TYPE_FLOAT) {
		return mpfr_cmp_si(fl_value, i) >= 0;
	}
	return mpq_cmp_si(r_value, i, 1) >= 0;
}
bool Number::isLessThanOrEqualTo(long int i) const {
	if(n_type == NUMBER_TYPE_PLUS_INFINITY) return false;
	if(n_type == NUMBER_TYPE_MINUS_INFINITY) return true;
	if(hasImaginaryPart()) return false;
	if(n_type == NUMBER_TYPE_FLOAT) {
		return mpfr_cmp_si(fu_value, i) <= 0;
	}
	return mpq_cmp_si(r_value, i, 1) <= 0;
}
bool Number::numeratorIsGreaterThan(long int i) const {
	if(!isRational()) return false;
	return mpz_cmp_si(mpq_numref(r_value), i) > 0;
}
bool Number::numeratorIsLessThan(long int i) const {
	if(!isRational()) return false;
	return mpz_cmp_si(mpq_numref(r_value), i) < 0;
}
bool Number::numeratorEquals(long int i) const {
	if(!isRational()) return false;
	return mpz_cmp_si(mpq_numref(r_value), i) == 0;
}
bool Number::denominatorIsGreaterThan(long int i) const {
	if(!isRational()) return false;
	return mpz_cmp_si(mpq_denref(r_value), i) > 0;
}
bool Number::denominatorIsLessThan(long int i) const {
	if(!isRational()) return false;
	return mpz_cmp_si(mpq_denref(r_value), i) < 0;
}
bool Number::denominatorEquals(long int i) const {
	if(!isRational()) return false;
	return mpz_cmp_si(mpq_denref(r_value), i) == 0;
}
bool Number::denominatorIsGreater(const Number &o) const {
	if(!isRational() || !o.isRational()) return false;
	return mpz_cmp(mpq_denref(r_value), mpq_denref(o.internalRational())) > 0;
}
bool Number::denominatorIsLess(const Number &o) const {
	if(!isRational() || !o.isRational()) return false;
	return mpz_cmp(mpq_denref(r_value), mpq_denref(o.internalRational())) < 0;
}
bool Number::denominatorIsEqual(const Number &o) const {
	if(!isRational() || !o.isRational()) return false;
	return mpz_cmp(mpq_denref(r_value), mpq_denref(o.internalRational())) == 0;
}
bool Number::isEven() const {
	return isInteger() && mpz_even_p(mpq_numref(r_value));
}
bool Number::denominatorIsEven() const {
	return !hasImaginaryPart() && n_type == NUMBER_TYPE_RATIONAL && mpz_even_p(mpq_denref(r_value));
}
bool Number::denominatorIsTwo() const {
	return !hasImaginaryPart() && n_type == NUMBER_TYPE_RATIONAL && mpz_cmp_si(mpq_denref(r_value), 2) == 0;
}
bool Number::numeratorIsEven() const {
	return !hasImaginaryPart() && n_type == NUMBER_TYPE_RATIONAL && mpz_even_p(mpq_numref(r_value));
}
bool Number::numeratorIsOne() const {
	return !hasImaginaryPart() && n_type == NUMBER_TYPE_RATIONAL && mpz_cmp_si(mpq_numref(r_value), 1) == 0;
}
bool Number::numeratorIsMinusOne() const {
	return !hasImaginaryPart() && n_type == NUMBER_TYPE_RATIONAL && mpz_cmp_si(mpq_numref(r_value), -1) == 0;
}
bool Number::isOdd() const {
	return isInteger() && mpz_odd_p(mpq_numref(r_value));
}

int Number::integerLength() const {
	if(isInteger()) return mpz_sizeinbase(mpq_numref(r_value), 2);
	return 0;
}


bool Number::add(const Number &o) {
	if(n_type == NUMBER_TYPE_RATIONAL && o.realPartIsRational()) {
		if(o.hasImaginaryPart()) {
			if(!i_value) {i_value = new Number(*o.internalImaginary()); i_value->markAsImaginaryPart();}
			else if(!i_value->add(*o.internalImaginary())) return false;
			setPrecisionAndApproximateFrom(*i_value);std::cout << "Number.cc-2765" << endl;
		}
		mpq_add(r_value, r_value, o.internalRational());std::cout << "Number.cc-2767" << endl;
		setPrecisionAndApproximateFrom(o);std::cout << "Number.cc-2768" << endl;
		return true;
	}
	Number nr_bak(*this);std::cout << "Number.cc-2771" << endl;
	if(o.hasImaginaryPart()) {
		if(!i_value) {i_value = new Number(*o.internalImaginary()); i_value->markAsImaginaryPart();}
		else if(!i_value->add(*o.internalImaginary())) return false;
		setPrecisionAndApproximateFrom(*i_value);std::cout << "Number.cc-2775" << endl;
	}
	if(includesMinusInfinity() && o.includesPlusInfinity()) return false;
	if(includesPlusInfinity() && o.includesMinusInfinity()) return false;
	if(isInfinite(true)) {
		setPrecisionAndApproximateFrom(o);std::cout << "Number.cc-2780" << endl;
		return true;
	}
	if(o.isPlusInfinity(true)) {
		setPlusInfinity(true, true);std::cout << "Number.cc-2784" << endl;
		setPrecisionAndApproximateFrom(o);std::cout << "Number.cc-2785" << endl;
		return true;
	}
	if(o.isMinusInfinity(true)) {
		setMinusInfinity(true, true);std::cout << "Number.cc-2789" << endl;
		setPrecisionAndApproximateFrom(o);std::cout << "Number.cc-2790" << endl;
		return true;
	}
	if(o.isFloatingPoint() || n_type == NUMBER_TYPE_FLOAT) {
		mpfr_clear_flags();std::cout << "Number.cc-2794" << endl;
		if(n_type != NUMBER_TYPE_FLOAT) {
			mpfr_init2(fu_value, BIT_PRECISION);std::cout << "Number.cc-2796" << endl;
			mpfr_init2(fl_value, BIT_PRECISION);std::cout << "Number.cc-2797" << endl;
			n_type = NUMBER_TYPE_FLOAT;std::cout << "Number.cc-2798" << endl;
			if(!CREATE_INTERVAL && !o.isInterval()) {
				mpfr_add_q(fl_value, o.internalLowerFloat(), r_value, MPFR_RNDN);std::cout << "Number.cc-2800" << endl;
				mpfr_set(fu_value, fl_value, MPFR_RNDN);std::cout << "Number.cc-2801" << endl;
			} else {
				mpfr_add_q(fu_value, o.internalUpperFloat(), r_value, MPFR_RNDU);std::cout << "Number.cc-2803" << endl;
				mpfr_add_q(fl_value, o.internalLowerFloat(), r_value, MPFR_RNDD);std::cout << "Number.cc-2804" << endl;
			}
			mpq_set_ui(r_value, 0, 1);std::cout << "Number.cc-2806" << endl;
		} else if(n_type == NUMBER_TYPE_FLOAT) {
			if(o.isFloatingPoint()) {
				if(!CREATE_INTERVAL && !isInterval() && !o.isInterval()) {
					mpfr_add(fl_value, fl_value, o.internalLowerFloat(), MPFR_RNDN);std::cout << "Number.cc-2810" << endl;
					mpfr_set(fu_value, fl_value, MPFR_RNDN);std::cout << "Number.cc-2811" << endl;
				} else {
					mpfr_add(fu_value, fu_value, o.internalUpperFloat(), MPFR_RNDU);std::cout << "Number.cc-2813" << endl;
					mpfr_add(fl_value, fl_value, o.internalLowerFloat(), MPFR_RNDD);std::cout << "Number.cc-2814" << endl;
				}
			} else {
				if(!CREATE_INTERVAL && !isInterval()) {
					mpfr_add_q(fl_value, fl_value, o.internalRational(), MPFR_RNDN);std::cout << "Number.cc-2818" << endl;
					mpfr_set(fu_value, fl_value, MPFR_RNDN);std::cout << "Number.cc-2819" << endl;
				} else {
					mpfr_add_q(fu_value, fu_value, o.internalRational(), MPFR_RNDU);std::cout << "Number.cc-2821" << endl;
					mpfr_add_q(fl_value, fl_value, o.internalRational(), MPFR_RNDD);std::cout << "Number.cc-2822" << endl;
				}
			}
		}
		if(!testFloatResult(true)) {
			set(nr_bak);std::cout << "Number.cc-2827" << endl;
			return false;
		}
	}
	setPrecisionAndApproximateFrom(o);std::cout << "Number.cc-2831" << endl;
	return true;
}
bool Number::add(long int i) {
	if(i == 0) return true;
	if(isInfinite(true)) return true;
	if(n_type == NUMBER_TYPE_FLOAT) {
		Number nr_bak(*this);std::cout << "Number.cc-2838" << endl;
		mpfr_clear_flags();std::cout << "Number.cc-2839" << endl;
		if(!CREATE_INTERVAL && !isInterval()) {
			mpfr_add_si(fl_value, fl_value, i, MPFR_RNDN);std::cout << "Number.cc-2841" << endl;
			mpfr_set(fu_value, fl_value, MPFR_RNDN);std::cout << "Number.cc-2842" << endl;
		} else {
			mpfr_add_si(fu_value, fu_value, i, MPFR_RNDU);std::cout << "Number.cc-2844" << endl;
			mpfr_add_si(fl_value, fl_value, i, MPFR_RNDD);std::cout << "Number.cc-2845" << endl;
		}
		if(!testFloatResult(true)) {
			set(nr_bak);std::cout << "Number.cc-2848" << endl;
			return false;
		}
	} else {
		if(i < 0) mpz_submul_ui(mpq_numref(r_value), mpq_denref(r_value), (unsigned int) (-i));
		else mpz_addmul_ui(mpq_numref(r_value), mpq_denref(r_value), (unsigned int) i);std::cout << "Number.cc-2853" << endl;
	}
	return true;
}

bool Number::subtract(const Number &o) {
	if(n_type == NUMBER_TYPE_RATIONAL && o.realPartIsRational()) {
		if(o.hasImaginaryPart()) {
			if(!i_value) {i_value = new Number(); i_value->markAsImaginaryPart();}
			if(!i_value->subtract(*o.internalImaginary())) return false;
			setPrecisionAndApproximateFrom(*i_value);std::cout << "Number.cc-2863" << endl;
		}
		mpq_sub(r_value, r_value, o.internalRational());std::cout << "Number.cc-2865" << endl;
		setPrecisionAndApproximateFrom(o);std::cout << "Number.cc-2866" << endl;
		return true;
	}
	Number nr_bak(*this);std::cout << "Number.cc-2869" << endl;
	if(includesPlusInfinity() && o.includesPlusInfinity()) return false;
	if(includesMinusInfinity() && o.includesMinusInfinity()) return false;
	if(o.hasImaginaryPart()) {
		if(!i_value) {i_value = new Number(); i_value->markAsImaginaryPart();}
		if(!i_value->subtract(*o.internalImaginary())) return false;
		setPrecisionAndApproximateFrom(*i_value);std::cout << "Number.cc-2875" << endl;
	}
	if(isInfinite()) {
		setPrecisionAndApproximateFrom(o);std::cout << "Number.cc-2878" << endl;
		return true;
	}
	if(o.isPlusInfinity(true)) {
		setMinusInfinity(true, true);std::cout << "Number.cc-2882" << endl;
		setPrecisionAndApproximateFrom(o);std::cout << "Number.cc-2883" << endl;
		return true;
	}
	if(o.isMinusInfinity(true)) {
		setPlusInfinity(true, true);std::cout << "Number.cc-2887" << endl;
		setPrecisionAndApproximateFrom(o);std::cout << "Number.cc-2888" << endl;
		return true;
	}
	if(o.isFloatingPoint() || n_type == NUMBER_TYPE_FLOAT) {
		mpfr_clear_flags();std::cout << "Number.cc-2892" << endl;
		if(n_type != NUMBER_TYPE_FLOAT) {
			mpfr_init2(fu_value, BIT_PRECISION);std::cout << "Number.cc-2894" << endl;
			mpfr_init2(fl_value, BIT_PRECISION);std::cout << "Number.cc-2895" << endl;
			n_type = NUMBER_TYPE_FLOAT;std::cout << "Number.cc-2896" << endl;
			if(!CREATE_INTERVAL && !o.isInterval()) {
				mpfr_sub_q(fl_value, o.internalLowerFloat(), r_value, MPFR_RNDN);std::cout << "Number.cc-2898" << endl;
				mpfr_neg(fl_value, fl_value, MPFR_RNDN);std::cout << "Number.cc-2899" << endl;
				mpfr_set(fu_value, fl_value, MPFR_RNDN);std::cout << "Number.cc-2900" << endl;
			} else {
				mpfr_sub_q(fu_value, o.internalLowerFloat(), r_value, MPFR_RNDD);std::cout << "Number.cc-2902" << endl;
				mpfr_neg(fu_value, fu_value, MPFR_RNDU);std::cout << "Number.cc-2903" << endl;
				mpfr_sub_q(fl_value, o.internalUpperFloat(), r_value, MPFR_RNDU);std::cout << "Number.cc-2904" << endl;
				mpfr_neg(fl_value, fl_value, MPFR_RNDD);std::cout << "Number.cc-2905" << endl;
			}
			mpq_set_ui(r_value, 0, 1);std::cout << "Number.cc-2907" << endl;
		} else if(n_type == NUMBER_TYPE_FLOAT) {
			if(o.isFloatingPoint()) {
				if(!CREATE_INTERVAL && !isInterval() && !o.isInterval()) {
					mpfr_sub(fl_value, fl_value, o.internalLowerFloat(), MPFR_RNDN);std::cout << "Number.cc-2911" << endl;
					mpfr_set(fu_value, fl_value, MPFR_RNDN);std::cout << "Number.cc-2912" << endl;
				} else {
					mpfr_sub(fu_value, fu_value, o.internalLowerFloat(), MPFR_RNDU);std::cout << "Number.cc-2914" << endl;
					mpfr_sub(fl_value, fl_value, o.internalUpperFloat(), MPFR_RNDD);std::cout << "Number.cc-2915" << endl;
				}
			} else {
				if(!CREATE_INTERVAL && !isInterval()) {
					mpfr_sub_q(fl_value, fl_value, o.internalRational(), MPFR_RNDN);std::cout << "Number.cc-2919" << endl;
					mpfr_set(fu_value, fl_value, MPFR_RNDN);std::cout << "Number.cc-2920" << endl;
				} else {
					mpfr_sub_q(fu_value, fu_value, o.internalRational(), MPFR_RNDU);std::cout << "Number.cc-2922" << endl;
					mpfr_sub_q(fl_value, fl_value, o.internalRational(), MPFR_RNDD);std::cout << "Number.cc-2923" << endl;
				}
			}
		}
		if(!testFloatResult(true)) {
			set(nr_bak);std::cout << "Number.cc-2928" << endl;
			return false;
		}
	}
	setPrecisionAndApproximateFrom(o);std::cout << "Number.cc-2932" << endl;
	return true;
}
bool Number::subtract(long int i) {
	if(i == 0) return true;
	if(isInfinite(true)) return true;
	if(n_type == NUMBER_TYPE_FLOAT) {
		Number nr_bak(*this);std::cout << "Number.cc-2939" << endl;
		mpfr_clear_flags();std::cout << "Number.cc-2940" << endl;
		if(!CREATE_INTERVAL && !isInterval()) {
			mpfr_sub_si(fl_value, fl_value, i, MPFR_RNDN);std::cout << "Number.cc-2942" << endl;
			mpfr_set(fu_value, fl_value, MPFR_RNDN);std::cout << "Number.cc-2943" << endl;
		} else {
			mpfr_sub_si(fu_value, fu_value, i, MPFR_RNDU);std::cout << "Number.cc-2945" << endl;
			mpfr_sub_si(fl_value, fl_value, i, MPFR_RNDD);std::cout << "Number.cc-2946" << endl;
		}
		if(!testFloatResult(true)) {
			set(nr_bak);std::cout << "Number.cc-2949" << endl;
			return false;
		}
	} else {
		if(i < 0) mpz_addmul_ui(mpq_numref(r_value), mpq_denref(r_value), (unsigned int) (-i));
		else mpz_submul_ui(mpq_numref(r_value), mpq_denref(r_value), (unsigned int) i);std::cout << "Number.cc-2954" << endl;
	}
	return true;
}

bool Number::multiply(const Number &o) {
	if(o.hasImaginaryPart()) {
		if(o.hasRealPart()) {
			Number nr_copy;std::cout << "Number.cc-2962" << endl;
			if(hasImaginaryPart()) {
				if(hasRealPart()) {
					Number nr_real;std::cout << "Number.cc-2965" << endl;
					nr_real.set(*this, false, true);std::cout << "Number.cc-2966" << endl;
					nr_copy.set(*i_value);std::cout << "Number.cc-2967" << endl;
					if(!nr_real.multiply(o.realPart()) || !nr_copy.multiply(o.imaginaryPart()) || !nr_copy.negate() || !nr_real.add(nr_copy)) return false;
					Number nr_imag(*i_value);std::cout << "Number.cc-2969" << endl;
					nr_copy.set(*this, false, true);std::cout << "Number.cc-2970" << endl;
					if(!nr_copy.multiply(o.imaginaryPart()) || !nr_imag.multiply(o.realPart()) || !nr_imag.add(nr_copy)) return false;
					set(nr_real, true, true);std::cout << "Number.cc-2972" << endl;
					i_value->set(nr_imag, true);std::cout << "Number.cc-2973" << endl;
					setPrecisionAndApproximateFrom(*i_value);std::cout << "Number.cc-2974" << endl;
					testComplex(this, i_value);std::cout << "Number.cc-2975" << endl;
					return true;
				}
				nr_copy.set(*i_value);std::cout << "Number.cc-2978" << endl;
				nr_copy.negate();std::cout << "Number.cc-2979" << endl;
			} else if(hasRealPart()) {
				nr_copy.setImaginaryPart(*this);std::cout << "Number.cc-2981" << endl;
			}
			Number nr_bak(*this);std::cout << "Number.cc-2983" << endl;
			if(!nr_copy.multiply(o.imaginaryPart())) return false;
			if(!multiply(o.realPart())) return false;
			if(!add(nr_copy)) {
				set(nr_bak);std::cout << "Number.cc-2987" << endl;
				return false;
			}
			return true;
		} else if(hasImaginaryPart()) {
			Number nr_copy(*i_value);std::cout << "Number.cc-2992" << endl;
			nr_copy.negate();std::cout << "Number.cc-2993" << endl;
			if(hasRealPart()) {
				nr_copy.setImaginaryPart(*this);std::cout << "Number.cc-2995" << endl;
			}
			if(!nr_copy.multiply(o.imaginaryPart())) return false;
			set(nr_copy);std::cout << "Number.cc-2998" << endl;
			return true;
		}
		if(!multiply(*o.internalImaginary())) return false;
		if(!i_value) {i_value = new Number(); i_value->markAsImaginaryPart();}
		i_value->set(*this, true, true);std::cout << "Number.cc-3003" << endl;
		clearReal();std::cout << "Number.cc-3004" << endl;
		return true;
	}
	if(o.includesInfinity() && !isNonZero()) return false;
	if(includesInfinity() && !o.isNonZero()) return false;
	if(n_type == NUMBER_TYPE_MINUS_INFINITY || n_type == NUMBER_TYPE_PLUS_INFINITY) {
		if(hasImaginaryPart()) {
			if(!i_value->multiply(o)) return false;
			setPrecisionAndApproximateFrom(*i_value);std::cout << "Number.cc-3012" << endl;
		}
		if(o.isNegative()) {
			if(n_type == NUMBER_TYPE_MINUS_INFINITY) {
				n_type = NUMBER_TYPE_PLUS_INFINITY;std::cout << "Number.cc-3016" << endl;
			} else {
				n_type = NUMBER_TYPE_MINUS_INFINITY;std::cout << "Number.cc-3018" << endl;
			}
			setPrecisionAndApproximateFrom(o);std::cout << "Number.cc-3020" << endl;
		}
		return true;
	}
	if(o.isPlusInfinity()) {
		if(hasRealPart() && !realPartIsNonZero()) return false;
		if(hasImaginaryPart()) {
			if(!i_value->multiply(o)) return false;
			setPrecisionAndApproximateFrom(*i_value);std::cout << "Number.cc-3028" << endl;
		}
		if(hasRealPart()) {
			if(isNegative()) setMinusInfinity(true, true);
			else setPlusInfinity(true, true);std::cout << "Number.cc-3032" << endl;
			setPrecisionAndApproximateFrom(o);std::cout << "Number.cc-3033" << endl;
		}
		return true;
	}
	if(o.isMinusInfinity()) {
		if(hasRealPart() && !realPartIsNonZero()) return false;
		if(hasImaginaryPart()) {
			if(!i_value->multiply(o)) return false;
			setPrecisionAndApproximateFrom(*i_value);std::cout << "Number.cc-3041" << endl;
		}
		if(hasRealPart()) {
			if(isNegative()) setPlusInfinity(true, true);
			else setMinusInfinity(true, true);std::cout << "Number.cc-3045" << endl;
		}
		setPrecisionAndApproximateFrom(o);std::cout << "Number.cc-3047" << endl;
		return true;
	}
	if(isZero()) return true;
	if(o.isZero()) {
		clear();std::cout << "Number.cc-3052" << endl;
		setPrecisionAndApproximateFrom(o);std::cout << "Number.cc-3053" << endl;
		return true;
	}
	if(o.isFloatingPoint() || n_type == NUMBER_TYPE_FLOAT) {
		Number nr_bak(*this);std::cout << "Number.cc-3057" << endl;
		if(hasImaginaryPart()) {
			if(!i_value->multiply(o)) return false;
			setPrecisionAndApproximateFrom(*i_value);std::cout << "Number.cc-3060" << endl;
		}
		if(hasRealPart()) {
			mpfr_clear_flags();std::cout << "Number.cc-3063" << endl;
			if(n_type != NUMBER_TYPE_FLOAT) {
				mpfr_init2(fu_value, BIT_PRECISION);std::cout << "Number.cc-3065" << endl;
				mpfr_init2(fl_value, BIT_PRECISION);std::cout << "Number.cc-3066" << endl;
				n_type = NUMBER_TYPE_FLOAT;std::cout << "Number.cc-3067" << endl;
				if(!CREATE_INTERVAL && !o.isInterval()) {
					mpfr_mul_q(fl_value, o.internalLowerFloat(), r_value, MPFR_RNDN);std::cout << "Number.cc-3069" << endl;
					mpfr_set(fu_value, fl_value, MPFR_RNDN);std::cout << "Number.cc-3070" << endl;
				} else {
					if(mpq_sgn(r_value) < 0) {
						mpfr_mul_q(fu_value, o.internalLowerFloat(), r_value, MPFR_RNDU);std::cout << "Number.cc-3073" << endl;
						mpfr_mul_q(fl_value, o.internalUpperFloat(), r_value, MPFR_RNDD);std::cout << "Number.cc-3074" << endl;
					} else {
						mpfr_mul_q(fu_value, o.internalUpperFloat(), r_value, MPFR_RNDU);std::cout << "Number.cc-3076" << endl;
						mpfr_mul_q(fl_value, o.internalLowerFloat(), r_value, MPFR_RNDD);std::cout << "Number.cc-3077" << endl;
					}
				}
				mpq_set_ui(r_value, 0, 1);std::cout << "Number.cc-3080" << endl;
			} else if(n_type == NUMBER_TYPE_FLOAT) {
				if(o.isFloatingPoint()) {
					if(!CREATE_INTERVAL && !isInterval() && !o.isInterval()) {
						mpfr_mul(fl_value, fl_value, o.internalLowerFloat(), MPFR_RNDN);std::cout << "Number.cc-3084" << endl;
						mpfr_set(fu_value, fl_value, MPFR_RNDN);std::cout << "Number.cc-3085" << endl;
					} else {
						int sgn_l = mpfr_sgn(fl_value), sgn_u = mpfr_sgn(fu_value), sgn_ol = mpfr_sgn(o.internalLowerFloat()), sgn_ou = mpfr_sgn(o.internalUpperFloat());std::cout << "Number.cc-3087" << endl;
						if((sgn_l < 0) != (sgn_u < 0)) {
							if((sgn_ol < 0) != (sgn_ou < 0)) {
								mpfr_t fu_value2, fl_value2;std::cout << "Number.cc-3090" << endl;
								mpfr_init2(fu_value2, BIT_PRECISION);std::cout << "Number.cc-3091" << endl;
								mpfr_init2(fl_value2, BIT_PRECISION);std::cout << "Number.cc-3092" << endl;
								mpfr_mul(fu_value2, fl_value, o.internalLowerFloat(), MPFR_RNDU);std::cout << "Number.cc-3093" << endl;
								mpfr_mul(fl_value2, fu_value, o.internalLowerFloat(), MPFR_RNDD);std::cout << "Number.cc-3094" << endl;
								mpfr_mul(fu_value, fu_value, o.internalUpperFloat(), MPFR_RNDU);std::cout << "Number.cc-3095" << endl;
								mpfr_mul(fl_value, fl_value, o.internalUpperFloat(), MPFR_RNDD);std::cout << "Number.cc-3096" << endl;
								if(mpfr_cmp(fu_value, fu_value2) < 0) mpfr_set(fu_value, fu_value2, MPFR_RNDU);
								if(mpfr_cmp(fl_value, fl_value2) > 0) mpfr_set(fl_value, fl_value2, MPFR_RNDD);
								mpfr_clears(fu_value2, fl_value2, NULL);std::cout << "Number.cc-3099" << endl;
							} else if(sgn_ol < 0) {
								mpfr_mul(fl_value, fl_value, o.internalLowerFloat(), MPFR_RNDU);std::cout << "Number.cc-3101" << endl;
								mpfr_mul(fu_value, fu_value, o.internalLowerFloat(), MPFR_RNDD);std::cout << "Number.cc-3102" << endl;
								mpfr_swap(fu_value, fl_value);std::cout << "Number.cc-3103" << endl;
							} else {
								mpfr_mul(fu_value, fu_value, o.internalUpperFloat(), MPFR_RNDU);std::cout << "Number.cc-3105" << endl;
								mpfr_mul(fl_value, fl_value, o.internalUpperFloat(), MPFR_RNDD);std::cout << "Number.cc-3106" << endl;
							}
						} else if((sgn_ol < 0) != (sgn_ou < 0)) {
							if(sgn_l < 0) {
								mpfr_mul(fu_value, fl_value, o.internalLowerFloat(), MPFR_RNDU);std::cout << "Number.cc-3110" << endl;
								mpfr_mul(fl_value, fl_value, o.internalUpperFloat(), MPFR_RNDD);std::cout << "Number.cc-3111" << endl;
							} else {
								mpfr_mul(fl_value, fu_value, o.internalLowerFloat(), MPFR_RNDD);std::cout << "Number.cc-3113" << endl;
								mpfr_mul(fu_value, fu_value, o.internalUpperFloat(), MPFR_RNDU);std::cout << "Number.cc-3114" << endl;
							}
						} else if(sgn_l < 0) {
							if(sgn_ol < 0) {
								mpfr_mul(fl_value, fl_value, o.internalLowerFloat(), MPFR_RNDU);std::cout << "Number.cc-3118" << endl;
								mpfr_mul(fu_value, fu_value, o.internalUpperFloat(), MPFR_RNDD);std::cout << "Number.cc-3119" << endl;
								mpfr_swap(fl_value, fu_value);std::cout << "Number.cc-3120" << endl;
							} else {
								mpfr_mul(fu_value, fu_value, o.internalLowerFloat(), MPFR_RNDU);std::cout << "Number.cc-3122" << endl;
								mpfr_mul(fl_value, fl_value, o.internalUpperFloat(), MPFR_RNDD);std::cout << "Number.cc-3123" << endl;
							}
						} else if(sgn_ol < 0) {
							mpfr_mul(fl_value, fl_value, o.internalUpperFloat(), MPFR_RNDU);std::cout << "Number.cc-3126" << endl;
							mpfr_mul(fu_value, fu_value, o.internalLowerFloat(), MPFR_RNDD);std::cout << "Number.cc-3127" << endl;
							mpfr_swap(fu_value, fl_value);std::cout << "Number.cc-3128" << endl;
						} else {
							mpfr_mul(fu_value, fu_value, o.internalUpperFloat(), MPFR_RNDU);std::cout << "Number.cc-3130" << endl;
							mpfr_mul(fl_value, fl_value, o.internalLowerFloat(), MPFR_RNDD);std::cout << "Number.cc-3131" << endl;
						}
					}
				} else {
					if(!CREATE_INTERVAL && !isInterval()) {
						mpfr_mul_q(fl_value, fl_value, o.internalRational(), MPFR_RNDN);std::cout << "Number.cc-3136" << endl;
						mpfr_set(fu_value, fl_value, MPFR_RNDN);std::cout << "Number.cc-3137" << endl;
					} else if(mpq_sgn(o.internalRational()) < 0) {
						mpfr_mul_q(fu_value, fu_value, o.internalRational(), MPFR_RNDD);std::cout << "Number.cc-3139" << endl;
						mpfr_mul_q(fl_value, fl_value, o.internalRational(), MPFR_RNDU);std::cout << "Number.cc-3140" << endl;
						mpfr_swap(fu_value, fl_value);std::cout << "Number.cc-3141" << endl;
					} else {
						mpfr_mul_q(fu_value, fu_value, o.internalRational(), MPFR_RNDU);std::cout << "Number.cc-3143" << endl;
						mpfr_mul_q(fl_value, fl_value, o.internalRational(), MPFR_RNDD);std::cout << "Number.cc-3144" << endl;
					}
				}
			}
			if(!testFloatResult(true)) {
				set(nr_bak);std::cout << "Number.cc-3149" << endl;
				return false;
			}
		}
	} else {
		if(hasImaginaryPart()) {
			if(!i_value->multiply(o)) return false;
			setPrecisionAndApproximateFrom(*i_value);std::cout << "Number.cc-3156" << endl;
		}
		mpq_mul(r_value, r_value, o.internalRational());std::cout << "Number.cc-3158" << endl;
	}
	setPrecisionAndApproximateFrom(o);std::cout << "Number.cc-3160" << endl;
	return true;
}
bool Number::multiply(long int i) {
	if(i == 0 && includesInfinity()) return false;
	if(n_type == NUMBER_TYPE_MINUS_INFINITY || n_type == NUMBER_TYPE_PLUS_INFINITY) {
		if(hasImaginaryPart()) {
			if(!i_value->multiply(i)) return false;
			setPrecisionAndApproximateFrom(*i_value);std::cout << "Number.cc-3168" << endl;
		}
		if(i < 0) {
			if(n_type == NUMBER_TYPE_MINUS_INFINITY) {
				n_type = NUMBER_TYPE_PLUS_INFINITY;std::cout << "Number.cc-3172" << endl;
			} else {
				n_type = NUMBER_TYPE_MINUS_INFINITY;std::cout << "Number.cc-3174" << endl;
			}
		}
		return true;
	}
	if(isZero()) return true;
	if(i == 0) {
		clear();std::cout << "Number.cc-3181" << endl;
		return true;
	}
	if(n_type == NUMBER_TYPE_FLOAT) {
		Number nr_bak(*this);std::cout << "Number.cc-3185" << endl;
		if(hasImaginaryPart()) {
			if(!i_value->multiply(i)) return false;
			setPrecisionAndApproximateFrom(*i_value);std::cout << "Number.cc-3188" << endl;
		}
		mpfr_clear_flags();std::cout << "Number.cc-3190" << endl;

		if(!CREATE_INTERVAL && !isInterval()) {
			mpfr_mul_si(fl_value, fl_value, i, MPFR_RNDN);std::cout << "Number.cc-3193" << endl;
			mpfr_set(fu_value, fl_value, MPFR_RNDN);std::cout << "Number.cc-3194" << endl;
		} else {
			mpfr_mul_si(fu_value, fu_value, i, MPFR_RNDU);std::cout << "Number.cc-3196" << endl;
			mpfr_mul_si(fl_value, fl_value, i, MPFR_RNDD);std::cout << "Number.cc-3197" << endl;
			if(i < 0) mpfr_swap(fu_value, fl_value);
		}

		if(!testFloatResult(true)) {
			set(nr_bak);std::cout << "Number.cc-3202" << endl;
			return false;
		}
	} else {
		if(hasImaginaryPart()) {
			if(!i_value->multiply(i)) return false;
			setPrecisionAndApproximateFrom(*i_value);std::cout << "Number.cc-3208" << endl;
		}
		mpq_t r_i;std::cout << "Number.cc-3210" << endl;
		mpq_init(r_i);std::cout << "Number.cc-3211" << endl;
		mpz_set_si(mpq_numref(r_i), i);std::cout << "Number.cc-3212" << endl;
		mpq_mul(r_value, r_value, r_i);std::cout << "Number.cc-3213" << endl;
		mpq_clear(r_i);std::cout << "Number.cc-3214" << endl;
	}
	return true;
}

bool Number::divide(const Number &o) {
	if(isInfinite() || o.isInfinite() || o.hasImaginaryPart() || o.isFloatingPoint() || n_type == NUMBER_TYPE_FLOAT) {
		Number oinv(o);std::cout << "Number.cc-3221" << endl;
		if(!oinv.recip()) return false;
		return multiply(oinv);
	}
	if(!o.isNonZero()) {
		if(isZero()) return false;
		return false;
	}
	if(isZero()) {
		setPrecisionAndApproximateFrom(o);std::cout << "Number.cc-3230" << endl;
		return true;
	}
	if(hasImaginaryPart()) {
		if(!i_value->divide(o)) return false;
		setPrecisionAndApproximateFrom(*i_value);std::cout << "Number.cc-3235" << endl;
	}
	mpq_div(r_value, r_value, o.internalRational());std::cout << "Number.cc-3237" << endl;
	setPrecisionAndApproximateFrom(o);std::cout << "Number.cc-3238" << endl;
	return true;
}
bool Number::divide(long int i) {
	if(includesInfinity() && i == 0) return false;
	if(isInfinite(true)) {
		if(hasImaginaryPart()) {
			if(!i_value->divide(i)) return false;
			setPrecisionAndApproximateFrom(*i_value);std::cout << "Number.cc-3246" << endl;
		}
		if(i < 0) {
			if(n_type == NUMBER_TYPE_PLUS_INFINITY) {
				n_type = NUMBER_TYPE_MINUS_INFINITY;std::cout << "Number.cc-3250" << endl;
			} else if(n_type == NUMBER_TYPE_MINUS_INFINITY) {
				n_type = NUMBER_TYPE_PLUS_INFINITY;std::cout << "Number.cc-3252" << endl;
			}
		}
		return true;
	}
	if(i == 0) return false;
	if(isZero()) return true;
	if(n_type == NUMBER_TYPE_FLOAT) {
		Number oinv(i < 0 ? -1 : 1, i < 0 ? -i : i, 0);std::cout << "Number.cc-3260" << endl;
		return multiply(oinv);
	}
	if(hasImaginaryPart()) {
		if(!i_value->divide(i)) return false;
		setPrecisionAndApproximateFrom(*i_value);std::cout << "Number.cc-3265" << endl;
	}
	mpq_t r_i;std::cout << "Number.cc-3267" << endl;
	mpq_init(r_i);std::cout << "Number.cc-3268" << endl;
	mpz_set_si(mpq_numref(r_i), i);std::cout << "Number.cc-3269" << endl;
	mpq_div(r_value, r_value, r_i);std::cout << "Number.cc-3270" << endl;
	mpq_clear(r_i);std::cout << "Number.cc-3271" << endl;
	return true;
}

bool Number::recip() {
	if(!isNonZero()) {
		return false;
	}
	if(isInfinite(false)) {
		clear();std::cout << "Number.cc-3280" << endl;
		return true;
	}
	if(hasImaginaryPart()) {
		if(hasRealPart()) {
			if(isInterval(false)) {

				// 1/(x+yi) = (x-yi)/(x^2+y^2);std::cout << "Number.cc-3287" << endl; max/min x/(x^2+y^2): x=abs(y), x=-abs(y)
				Number nr_bak(*this);std::cout << "Number.cc-3288" << endl;
				if(!setToFloatingPoint()) return false;
				if(!i_value->setToFloatingPoint()) return false;
				mpfr_t abs_il, abs_iu, absm_iu, absm_il, abs_rl, abs_ru, absm_ru, absm_rl, ftmp1, ftmp2, fu_tmp, fl_tmp;std::cout << "Number.cc-3291" << endl;
				mpfr_inits2(BIT_PRECISION, abs_il, abs_iu, absm_iu, absm_il, abs_rl, abs_ru, absm_ru, absm_rl, ftmp1, ftmp2, fu_tmp, fl_tmp, NULL);std::cout << "Number.cc-3292" << endl;

				if(mpfr_sgn(fl_value) != mpfr_sgn(fu_value)) mpfr_set_zero(abs_rl, 0);
				else if(mpfr_cmpabs(fu_value, fl_value) < 0) mpfr_abs(abs_rl, fu_value, MPFR_RNDD);
				else mpfr_abs(abs_rl, fl_value, MPFR_RNDD);std::cout << "Number.cc-3296" << endl;
				if(mpfr_cmpabs(fl_value, fu_value) > 0) mpfr_abs(abs_ru, fl_value, MPFR_RNDU);
				else mpfr_abs(abs_ru, fu_value, MPFR_RNDU);std::cout << "Number.cc-3298" << endl;
				mpfr_neg(absm_ru, abs_ru, MPFR_RNDD);std::cout << "Number.cc-3299" << endl;
				mpfr_neg(absm_rl, abs_rl, MPFR_RNDD);std::cout << "Number.cc-3300" << endl;

				if(mpfr_sgn(i_value->internalLowerFloat()) != mpfr_sgn(i_value->internalUpperFloat())) mpfr_set_zero(abs_il, 0);
				else if(mpfr_cmpabs(i_value->internalUpperFloat(), i_value->internalLowerFloat()) < 0) mpfr_abs(abs_il, i_value->internalUpperFloat(), MPFR_RNDD);
				else mpfr_abs(abs_il, i_value->internalLowerFloat(), MPFR_RNDD);std::cout << "Number.cc-3304" << endl;
				if(mpfr_cmpabs(i_value->internalLowerFloat(), i_value->internalUpperFloat()) > 0) mpfr_abs(abs_iu, i_value->internalLowerFloat(), MPFR_RNDU);
				else mpfr_abs(abs_iu, i_value->internalUpperFloat(), MPFR_RNDU);std::cout << "Number.cc-3306" << endl;
				mpfr_neg(absm_iu, abs_iu, MPFR_RNDD);std::cout << "Number.cc-3307" << endl;
				mpfr_neg(absm_il, abs_il, MPFR_RNDD);std::cout << "Number.cc-3308" << endl;

				for(size_t i = 0; i < 2; i++) {
					if(i == 0) {
						mpfr_swap(fu_tmp, fu_value);std::cout << "Number.cc-3312" << endl;
						mpfr_swap(fl_tmp, fl_value);std::cout << "Number.cc-3313" << endl;
					} else {
						mpfr_swap(fu_tmp, i_value->internalUpperFloat());std::cout << "Number.cc-3315" << endl;
						mpfr_swap(fl_tmp, i_value->internalLowerFloat());std::cout << "Number.cc-3316" << endl;
						mpfr_swap(abs_il, abs_rl);std::cout << "Number.cc-3317" << endl;
						mpfr_swap(abs_iu, abs_ru);std::cout << "Number.cc-3318" << endl;
						mpfr_swap(absm_iu, absm_ru);std::cout << "Number.cc-3319" << endl;
						mpfr_swap(absm_il, absm_rl);std::cout << "Number.cc-3320" << endl;
						mpfr_neg(fu_tmp, fu_tmp, MPFR_RNDU);std::cout << "Number.cc-3321" << endl;
						mpfr_neg(fl_tmp, fl_tmp, MPFR_RNDD);std::cout << "Number.cc-3322" << endl;
						mpfr_swap(fl_tmp, fu_tmp);std::cout << "Number.cc-3323" << endl;
					}

					bool neg = mpfr_sgn(fu_tmp) < 0;std::cout << "Number.cc-3326" << endl;
					if(neg) {
						mpfr_neg(fu_tmp, fu_tmp, MPFR_RNDD);std::cout << "Number.cc-3328" << endl;
						mpfr_neg(fl_tmp, fl_tmp, MPFR_RNDU);std::cout << "Number.cc-3329" << endl;
						mpfr_swap(fl_tmp, fu_tmp);std::cout << "Number.cc-3330" << endl;
					}
					if(mpfr_cmp(fl_tmp, abs_il) <= 0) {
						if(mpfr_cmp(fu_tmp, abs_il) >= 0) {
							mpfr_sqr(ftmp1, abs_il, MPFR_RNDD);std::cout << "Number.cc-3334" << endl;
							mpfr_mul_ui(ftmp1, ftmp1, 2, MPFR_RNDD);std::cout << "Number.cc-3335" << endl;
							mpfr_div(fu_tmp, abs_il, ftmp1, MPFR_RNDU);std::cout << "Number.cc-3336" << endl;
						} else {
							mpfr_sqr(ftmp1, fu_tmp, MPFR_RNDD);std::cout << "Number.cc-3338" << endl;
							mpfr_sqr(ftmp2, abs_il, MPFR_RNDD);std::cout << "Number.cc-3339" << endl;
							mpfr_add(ftmp1, ftmp1, ftmp2, MPFR_RNDD);std::cout << "Number.cc-3340" << endl;
							mpfr_div(fu_tmp, fu_tmp, ftmp1, MPFR_RNDU);std::cout << "Number.cc-3341" << endl;
						}
					} else {
						mpfr_sqr(ftmp1, fl_tmp, MPFR_RNDD);std::cout << "Number.cc-3344" << endl;
						mpfr_sqr(ftmp2, abs_il, MPFR_RNDD);std::cout << "Number.cc-3345" << endl;
						mpfr_add(ftmp1, ftmp1, ftmp2, MPFR_RNDD);std::cout << "Number.cc-3346" << endl;
						mpfr_div(fu_tmp, fl_tmp, ftmp1, MPFR_RNDU);std::cout << "Number.cc-3347" << endl;
					}
					if(mpfr_sgn(fl_tmp) < 0) {
						if(mpfr_cmp(fl_tmp, absm_il) <= 0) {
							mpfr_sqr(ftmp1, abs_il, MPFR_RNDU);std::cout << "Number.cc-3351" << endl;
							mpfr_mul_ui(ftmp1, ftmp1, 2, MPFR_RNDU);std::cout << "Number.cc-3352" << endl;
							mpfr_div(fl_tmp, absm_il, ftmp1, MPFR_RNDD);std::cout << "Number.cc-3353" << endl;
						} else {
							mpfr_sqr(ftmp1, fl_tmp, MPFR_RNDD);std::cout << "Number.cc-3355" << endl;
							mpfr_sqr(ftmp2, abs_il, MPFR_RNDD);std::cout << "Number.cc-3356" << endl;
							mpfr_add(ftmp1, ftmp1, ftmp2, MPFR_RNDD);std::cout << "Number.cc-3357" << endl;
							mpfr_div(fl_tmp, fl_tmp, ftmp1, MPFR_RNDU);std::cout << "Number.cc-3358" << endl;
						}
					} else if(mpfr_cmp(fl_tmp, absm_iu) <= 0) {
						if(mpfr_cmp(abs_ru, absm_iu) >= 0) {
							mpfr_sqr(ftmp1, abs_iu, MPFR_RNDU);std::cout << "Number.cc-3362" << endl;
							mpfr_mul_ui(ftmp1, ftmp1, 2, MPFR_RNDU);std::cout << "Number.cc-3363" << endl;
							mpfr_div(fl_tmp, absm_iu, ftmp1, MPFR_RNDD);std::cout << "Number.cc-3364" << endl;
						} else {
							mpfr_sqr(ftmp1, abs_ru, MPFR_RNDU);std::cout << "Number.cc-3366" << endl;
							mpfr_sqr(ftmp2, abs_iu, MPFR_RNDU);std::cout << "Number.cc-3367" << endl;
							mpfr_add(ftmp1, ftmp1, ftmp2, MPFR_RNDU);std::cout << "Number.cc-3368" << endl;
							mpfr_div(fl_tmp, abs_ru, ftmp1, MPFR_RNDD);std::cout << "Number.cc-3369" << endl;
						}
					} else {
						if(mpfr_cmp(fl_tmp, abs_iu) > 0) {
							mpfr_sqr(ftmp1, abs_ru, MPFR_RNDU);std::cout << "Number.cc-3373" << endl;
							mpfr_sqr(ftmp2, abs_iu, MPFR_RNDU);std::cout << "Number.cc-3374" << endl;
							mpfr_add(ftmp1, ftmp1, ftmp2, MPFR_RNDU);std::cout << "Number.cc-3375" << endl;
							mpfr_div(fl_tmp, abs_ru, ftmp1, MPFR_RNDD);std::cout << "Number.cc-3376" << endl;
						} else {
							mpfr_sqr(ftmp1, abs_rl, MPFR_RNDU);std::cout << "Number.cc-3378" << endl;
							mpfr_sqr(ftmp2, abs_iu, MPFR_RNDU);std::cout << "Number.cc-3379" << endl;
							mpfr_add(ftmp1, ftmp1, ftmp2, MPFR_RNDU);std::cout << "Number.cc-3380" << endl;
							mpfr_div(fl_tmp, fl_tmp, ftmp1, MPFR_RNDD);std::cout << "Number.cc-3381" << endl;
							if(mpfr_cmp(abs_ru, abs_iu) > 0) {
								mpfr_sqr(ftmp1, abs_ru, MPFR_RNDU);std::cout << "Number.cc-3383" << endl;
								mpfr_sqr(ftmp2, abs_iu, MPFR_RNDU);std::cout << "Number.cc-3384" << endl;
								mpfr_add(ftmp1, ftmp1, ftmp2, MPFR_RNDU);std::cout << "Number.cc-3385" << endl;
								mpfr_div(ftmp1, abs_ru, ftmp1, MPFR_RNDD);std::cout << "Number.cc-3386" << endl;
								if(mpfr_cmp(ftmp1, fl_tmp) < 0) {
									mpfr_swap(ftmp1, fl_tmp);std::cout << "Number.cc-3388" << endl;
								}
							}
						}
					}
					if(neg) {
						mpfr_neg(fu_tmp, fu_tmp, MPFR_RNDD);std::cout << "Number.cc-3394" << endl;
						mpfr_neg(fl_tmp, fl_tmp, MPFR_RNDU);std::cout << "Number.cc-3395" << endl;
						mpfr_swap(fl_tmp, fu_tmp);std::cout << "Number.cc-3396" << endl;
					}

					if(i == 0) {
						mpfr_swap(fu_tmp, fu_value);std::cout << "Number.cc-3400" << endl;
						mpfr_swap(fl_tmp, fl_value);std::cout << "Number.cc-3401" << endl;
					} else {
						mpfr_swap(fu_tmp, i_value->internalUpperFloat());std::cout << "Number.cc-3403" << endl;
						mpfr_swap(fl_tmp, i_value->internalLowerFloat());std::cout << "Number.cc-3404" << endl;
					}
				}
				mpfr_clears(abs_il, abs_iu, absm_iu, absm_il, abs_rl, abs_ru, absm_ru, absm_rl, ftmp1, ftmp2, fu_tmp, fl_tmp, NULL);std::cout << "Number.cc-3407" << endl;
				if(!i_value->testFloatResult(true) || !testFloatResult(true)) {
					set(nr_bak);std::cout << "Number.cc-3409" << endl;
					return false;
				}
				return true;
			} else {
				Number den1(*i_value);std::cout << "Number.cc-3414" << endl;
				Number den2;std::cout << "Number.cc-3415" << endl;
				den2.set(*this, false, true);std::cout << "Number.cc-3416" << endl;
				Number num_r(den2), num_i(den1);std::cout << "Number.cc-3417" << endl;
				if(!den1.square() || !num_i.negate() || !den2.square() || !den1.add(den2) || !num_r.divide(den1) || !num_i.divide(den1)) return false;
				set(num_r);std::cout << "Number.cc-3419" << endl;
				setImaginaryPart(num_i);std::cout << "Number.cc-3420" << endl;
			}
			return true;
		}
		if(!i_value->recip() || !i_value->negate()) return false;
		setPrecisionAndApproximateFrom(*i_value);std::cout << "Number.cc-3425" << endl;
		return true;
	}
	if(n_type == NUMBER_TYPE_FLOAT) {
		Number nr_bak(*this);std::cout << "Number.cc-3429" << endl;
		if(!CREATE_INTERVAL && !isInterval()) {
			mpfr_ui_div(fl_value, 1, fl_value, MPFR_RNDN);std::cout << "Number.cc-3431" << endl;
			mpfr_set(fu_value, fl_value, MPFR_RNDN);std::cout << "Number.cc-3432" << endl;
		} else {
			mpfr_ui_div(fu_value, 1, fu_value, MPFR_RNDD);std::cout << "Number.cc-3434" << endl;
			mpfr_ui_div(fl_value, 1, fl_value, MPFR_RNDU);std::cout << "Number.cc-3435" << endl;
			mpfr_swap(fu_value, fl_value);std::cout << "Number.cc-3436" << endl;
		}
		if(!testFloatResult(true)) {
			set(nr_bak);std::cout << "Number.cc-3439" << endl;
			return false;
		}
	} else {
		mpq_inv(r_value, r_value);std::cout << "Number.cc-3443" << endl;
	}
	return true;
}
bool Number::raise(const Number &o, bool try_exact) {
	if(o.isTwo()) return square();
	if(o.isMinusOne()) {
		if(!recip()) return false;
		setPrecisionAndApproximateFrom(o);std::cout << "Number.cc-3451" << endl;
		return true;
	}
	if((includesInfinity(true) || o.includesInfinity(true)) && !hasImaginaryPart() && !o.hasImaginaryPart()) {
		if(isInfinite(false)) {
			if(o.isNegative()) {
				clear(true);std::cout << "Number.cc-3457" << endl;
				return true;
			}
			if(!o.isNonZero() || o.hasImaginaryPart()) {
				return false;
			}
			if(isMinusInfinity()) {
				if(o.isEven()) {
					n_type = NUMBER_TYPE_PLUS_INFINITY;std::cout << "Number.cc-3465" << endl;
				} else if(!o.isInteger()) {
					return false;
				}
			}
			setPrecisionAndApproximateFrom(o);std::cout << "Number.cc-3470" << endl;
			return true;
		}
		if(includesInfinity()) {
			if(hasImaginaryPart() || !o.isNonZero() || o.hasImaginaryPart()) {
				return false;
			}
		}
		if(o.includesMinusInfinity()) {
			if(o.isFloatingPoint() && o.includesPlusInfinity()) return false;
			if(!isNonZero()) {
				return false;
			} else if(isNegative()) {
				if(!isLessThan(-1)) return false;
				if(!o.isFloatingPoint()) clear(true);
			} else if(hasImaginaryPart()) {
				if(hasRealPart()) return false;
				if(!o.isFloatingPoint() && !i_value->raise(o)) return false;
			} else if(isGreaterThan(1)) {
				if(!o.isFloatingPoint()) clear(true);
			} else if(isPositive() && isLessThan(1)) {
				setPlusInfinity();std::cout << "Number.cc-3491" << endl;
			} else {
				return false;
			}
			if(!o.isFloatingPoint()) {
				setPrecisionAndApproximateFrom(o);std::cout << "Number.cc-3496" << endl;
				return true;
			}
		} else if(o.includesPlusInfinity()) {
			if(!isNonZero()) {
				return false;
			} else if(isNegative()) {
				if(!isGreaterThan(-1)) return false;
				if(!o.isFloatingPoint()) clear(true);
			} else if(hasImaginaryPart()) {
				if(hasRealPart()) return false;
				if(!o.isFloatingPoint() && !i_value->raise(o)) return false;
			} else if(isGreaterThan(1)) {
				setPlusInfinity();std::cout << "Number.cc-3509" << endl;
			} else if(isPositive() && isLessThan(1)) {
				if(!o.isFloatingPoint()) clear(true);
			} else {
				return false;
			}
			if(!o.isFloatingPoint()) {
				setPrecisionAndApproximateFrom(o);std::cout << "Number.cc-3516" << endl;
				return true;
			}
		}
	}
	if(isZero() && o.isNegative()) {
		CALCULATOR->error(true, _("Division by zero."), NULL);
		return false;
	}
	if(isZero()) {
		if(o.isZero()) {
			//0^0
			CALCULATOR->error(false, _("0^0 might be considered undefined"), NULL);
			set(1, 1, 0, true);std::cout << "Number.cc-3529" << endl;
			setPrecisionAndApproximateFrom(o);std::cout << "Number.cc-3530" << endl;
			return true;
		} else if(!o.realPartIsNonNegative()) {
			return false;
		} else if(o.hasImaginaryPart()) {
			CALCULATOR->error(false, _("The result of 0^i is possibly undefined"), NULL);
		}
		return true;
	}

	if(o.isZero()) {
		if(hasImaginaryPart() && i_value->includesInfinity()) return false;
		set(1, 1, 0, false);std::cout << "Number.cc-3542" << endl;
		setPrecisionAndApproximateFrom(o);std::cout << "Number.cc-3543" << endl;
		return true;
	}
	if(o.isOne()) {
		setPrecisionAndApproximateFrom(o);std::cout << "Number.cc-3547" << endl;
		return true;
	}
	if(isOne() && !o.includesInfinity()) {
		return true;
	}
	if(o.hasImaginaryPart()) {
		if(b_imag) return false;
		Number nr_a, nr_b, nr_c, nr_d;std::cout << "Number.cc-3555" << endl;
		if(hasImaginaryPart()) {
			if(hasRealPart()) nr_a = realPart();
			nr_b = imaginaryPart();std::cout << "Number.cc-3558" << endl;
		} else {
			nr_a.set(*this);std::cout << "Number.cc-3560" << endl;
		}
		if(o.hasRealPart()) nr_c = o.realPart();
		nr_d = o.imaginaryPart();std::cout << "Number.cc-3563" << endl;
		Number a2b2c2(1, 1);std::cout << "Number.cc-3564" << endl;
		Number a2b2(nr_a);std::cout << "Number.cc-3565" << endl;
		Number b2(nr_b);std::cout << "Number.cc-3566" << endl;
		if(!a2b2.square() || !b2.square() || !a2b2.add(b2)) return false;
		if(!nr_c.isZero()) {
			Number chalf(nr_c);std::cout << "Number.cc-3569" << endl;
			a2b2c2 = a2b2;std::cout << "Number.cc-3570" << endl;
			if(!chalf.multiply(nr_half) || !a2b2c2.raise(chalf)) return false;
		}
		Number nr_arg(nr_b);std::cout << "Number.cc-3573" << endl;
		if(!nr_arg.atan2(nr_a, true)) return false;
		Number eraised, nexp(nr_d);std::cout << "Number.cc-3575" << endl;
		eraised.e();std::cout << "Number.cc-3576" << endl;
		if(!nexp.negate() || !nexp.multiply(nr_arg) || !eraised.raise(nexp, false)) return false;

		if(!nr_arg.multiply(nr_c) || !nr_d.multiply(nr_half) || !a2b2.ln() || !nr_d.multiply(a2b2) || !nr_arg.add(nr_d)) return false;
		Number nr_cos(nr_arg);std::cout << "Number.cc-3580" << endl;
		Number nr_sin(nr_arg);std::cout << "Number.cc-3581" << endl;
		if(!nr_cos.cos() || !nr_sin.sin() || !nr_sin.multiply(nr_one_i) || !nr_cos.add(nr_sin)) return false;
		if(!eraised.multiply(a2b2c2) || !eraised.multiply(nr_cos)) return false;
		if((hasImaginaryPart() || o.hasRealPart()) && eraised.isInterval(false) && eraised.precision(1) <= PRECISION + 20) CALCULATOR->error(false, MESSAGE_CATEGORY_WIDE_INTERVAL, _("Interval calculated wide."), NULL);
		set(eraised);std::cout << "Number.cc-3585" << endl;
		setPrecisionAndApproximateFrom(o);std::cout << "Number.cc-3586" << endl;
		return true;
	}
	if(hasImaginaryPart()) {
		if(o.isNegative()) {
			if(o.isMinusOne()) return recip();
			Number ninv(*this), opos(o);std::cout << "Number.cc-3592" << endl;
			if(!ninv.recip() ||!opos.negate() || !ninv.raise(opos, try_exact)) return false;
			set(ninv);std::cout << "Number.cc-3594" << endl;
			return true;
		}// else if(!o.isNonNegative()) return false;
		if(hasRealPart() || !o.isInteger()) {
			if(try_exact && !isFloatingPoint() && !i_value->isFloatingPoint() && o.isInteger() && o.isPositive() && o.isLessThan(100)) {
				int i = o.intValue();std::cout << "Number.cc-3599" << endl;
				Number nr_init(*this);std::cout << "Number.cc-3600" << endl;
				while(i > 1) {
					if(i > 5 && CALCULATOR->aborted()) return false;
					if(!multiply(nr_init)) {
						set(nr_init);std::cout << "Number.cc-3604" << endl;
						return false;
					}
					i--;std::cout << "Number.cc-3607" << endl;
				}
				setPrecisionAndApproximateFrom(o);std::cout << "Number.cc-3609" << endl;
				return true;
			}
			Number nbase, nexp(*this);std::cout << "Number.cc-3612" << endl;
			nbase.e();std::cout << "Number.cc-3613" << endl;
			if(!nexp.ln() || !nexp.multiply(o) || !nbase.raise(nexp, false)) return false;
			set(nbase);std::cout << "Number.cc-3615" << endl;
			return true;
		}
		if(!i_value->raise(o, try_exact)) return false;
		Number ibneg(o);std::cout << "Number.cc-3619" << endl;
		if(!ibneg.iquo(2)) return false;
		if(!ibneg.isEven() && !i_value->negate()) return false;
		if(o.isEven()) {
			set(*i_value, true, true);std::cout << "Number.cc-3623" << endl;
			clearImaginary();std::cout << "Number.cc-3624" << endl;
		} else {
			setPrecisionAndApproximateFrom(*i_value);std::cout << "Number.cc-3626" << endl;
		}
		setPrecisionAndApproximateFrom(o);std::cout << "Number.cc-3628" << endl;
		return true;
	}


	if(isMinusOne() && o.isRational()) {
		if(o.isInteger()) {
			if(o.isEven()) set(1, 1, 0, true);
			setPrecisionAndApproximateFrom(o);std::cout << "Number.cc-3636" << endl;
			return true;
		} else if(o.denominatorIsTwo()) {
			if(b_imag) return false;
			clear(true);std::cout << "Number.cc-3640" << endl;
			if(!i_value) {i_value = new Number(); i_value->markAsImaginaryPart();}
			if(o.numeratorIsOne()) {
				i_value->set(1, 1, 0);std::cout << "Number.cc-3643" << endl;
			} else {
				mpz_t zrem;std::cout << "Number.cc-3645" << endl;
				mpz_init(zrem);std::cout << "Number.cc-3646" << endl;
				mpz_tdiv_r_ui(zrem, mpq_numref(o.internalRational()), 4);std::cout << "Number.cc-3647" << endl;
				if(mpz_cmp_ui(zrem, 1) == 0 || mpz_cmp_si(zrem, -3) == 0) {
					i_value->set(1, 1, 0);std::cout << "Number.cc-3649" << endl;
				} else {
					i_value->set(-1, 1, 0);std::cout << "Number.cc-3651" << endl;
				}
			}
			setPrecisionAndApproximateFrom(o);std::cout << "Number.cc-3654" << endl;
			return true;
		}
	}

	if(n_type == NUMBER_TYPE_RATIONAL && !o.isFloatingPoint()) {
		bool success = false;std::cout << "Number.cc-3660" << endl;
		if(mpz_fits_slong_p(mpq_numref(o.internalRational())) != 0 && mpz_fits_ulong_p(mpq_denref(o.internalRational())) != 0) {
			long int i_pow = mpz_get_si(mpq_numref(o.internalRational()));std::cout << "Number.cc-3662" << endl;
			unsigned long int i_root = mpz_get_ui(mpq_denref(o.internalRational()));std::cout << "Number.cc-3663" << endl;
			size_t length1 = mpz_sizeinbase(mpq_numref(r_value), 10);std::cout << "Number.cc-3664" << endl;
			size_t length2 = mpz_sizeinbase(mpq_denref(r_value), 10);std::cout << "Number.cc-3665" << endl;
			if(length2 > length1) length1 = length2;
			if((i_root <= 2  || mpq_sgn(r_value) > 0) && ((!try_exact && i_root <= 3 && (long long int) labs(i_pow) * length1 < 1000) || (try_exact && (long long int) labs(i_pow) * length1 < 1000000LL && i_root < 1000000L))) {
				bool complex_result = false;std::cout << "Number.cc-3668" << endl;
				if(i_root != 1) {
					mpq_t r_test;std::cout << "Number.cc-3670" << endl;
					mpq_init(r_test);std::cout << "Number.cc-3671" << endl;
					bool b_neg = i_pow < 0;std::cout << "Number.cc-3672" << endl;
					if(b_neg) {
						mpq_inv(r_test, r_value);std::cout << "Number.cc-3674" << endl;
						i_pow = -i_pow;std::cout << "Number.cc-3675" << endl;
					} else {
						mpq_set(r_test, r_value);std::cout << "Number.cc-3677" << endl;
					}
					if(mpz_cmp_ui(mpq_denref(r_test), 1) == 0) {
						if(i_pow != 1) mpz_pow_ui(mpq_numref(r_test), mpq_numref(r_test), (unsigned long int) i_pow);
						if(i_root % 2 == 0 && mpq_sgn(r_test) < 0) {
							if(b_imag) {mpq_clear(r_test); return false;}
							if(i_root == 2) {
								mpq_neg(r_test, r_test);std::cout << "Number.cc-3684" << endl;
								success = mpz_root(mpq_numref(r_test), mpq_numref(r_test), i_root);std::cout << "Number.cc-3685" << endl;
								complex_result = true;std::cout << "Number.cc-3686" << endl;
							}
						} else {
							success = mpz_root(mpq_numref(r_test), mpq_numref(r_test), i_root);std::cout << "Number.cc-3689" << endl;
						}
					} else {
						if(i_pow != 1) {
							mpz_pow_ui(mpq_numref(r_test), mpq_numref(r_test), (unsigned long int) i_pow);std::cout << "Number.cc-3693" << endl;
							mpz_pow_ui(mpq_denref(r_test), mpq_denref(r_test), (unsigned long int) i_pow);std::cout << "Number.cc-3694" << endl;
						}
						if(i_root % 2 == 0 && mpq_sgn(r_test) < 0) {
							if(b_imag) {mpq_clear(r_test); return false;}
							if(i_root == 2) {
								mpq_neg(r_test, r_test);std::cout << "Number.cc-3699" << endl;
								success = mpz_root(mpq_numref(r_test), mpq_numref(r_test), i_root) && mpz_root(mpq_denref(r_test), mpq_denref(r_test), i_root);std::cout << "Number.cc-3700" << endl;
								complex_result = true;std::cout << "Number.cc-3701" << endl;
							}
						} else {
							success = mpz_root(mpq_numref(r_test), mpq_numref(r_test), i_root) && mpz_root(mpq_denref(r_test), mpq_denref(r_test), i_root);std::cout << "Number.cc-3704" << endl;
						}
					}
					if(success) {
						if(complex_result) {
							if(b_neg) mpq_neg(r_test, r_test);
							if(!i_value) {i_value = new Number(); i_value->markAsImaginaryPart();}
							i_value->setInternal(r_test, false, true);std::cout << "Number.cc-3711" << endl;
							if(i_pow % 4 == 3) i_value->negate();
							mpq_set_ui(r_value, 0, 1);std::cout << "Number.cc-3713" << endl;
						} else {
							mpq_set(r_value, r_test);std::cout << "Number.cc-3715" << endl;
						}
					}
					mpq_clear(r_test);std::cout << "Number.cc-3718" << endl;
				} else if(i_pow != 1) {
					if(i_pow < 0) {
						mpq_inv(r_value, r_value);std::cout << "Number.cc-3721" << endl;
						i_pow = -i_pow;std::cout << "Number.cc-3722" << endl;
					}
					if(i_pow != 1) {
						mpz_pow_ui(mpq_numref(r_value), mpq_numref(r_value), (unsigned long int) i_pow);std::cout << "Number.cc-3725" << endl;
						if(mpz_cmp_ui(mpq_denref(r_value), 1) != 0) mpz_pow_ui(mpq_denref(r_value), mpq_denref(r_value), (unsigned long int) i_pow);
					}
					success = true;std::cout << "Number.cc-3728" << endl;
				} else {
					success = true;std::cout << "Number.cc-3730" << endl;
				}
			}
		}
		if(success) {
			setPrecisionAndApproximateFrom(o);std::cout << "Number.cc-3735" << endl;
			return true;
		}
	}
	Number nr_bak(*this);std::cout << "Number.cc-3739" << endl;

	if(!setToFloatingPoint()) return false;
	mpfr_clear_flags();std::cout << "Number.cc-3742" << endl;

	int sgn_l = mpfr_sgn(fl_value), sgn_u = mpfr_sgn(fu_value);std::cout << "Number.cc-3744" << endl;

	bool try_complex = false;std::cout << "Number.cc-3746" << endl;

	if(o.isFloatingPoint()) {
		int sgn_ol = mpfr_sgn(o.internalLowerFloat()), sgn_ou = mpfr_sgn(o.internalUpperFloat());std::cout << "Number.cc-3749" << endl;
		if(sgn_ol < 0 && (sgn_l == 0 || sgn_l != sgn_u)) {
			//CALCULATOR->error(true, _("Division by zero."), NULL);
			set(nr_bak);std::cout << "Number.cc-3752" << endl;
			return false;
		}
		if(sgn_l < 0) {
			try_complex = true;std::cout << "Number.cc-3756" << endl;
		} else {
			if(!isInterval() && !o.isInterval()) {
				if(!CREATE_INTERVAL) {
					mpfr_pow(fl_value, fl_value, o.internalLowerFloat(), MPFR_RNDN);std::cout << "Number.cc-3760" << endl;
					mpfr_set(fu_value, fl_value, MPFR_RNDN);std::cout << "Number.cc-3761" << endl;
				} else {
					mpfr_pow(fl_value, fl_value, o.internalLowerFloat(), MPFR_RNDD);std::cout << "Number.cc-3763" << endl;
					mpfr_pow(fu_value, fu_value, o.internalUpperFloat(), MPFR_RNDU);std::cout << "Number.cc-3764" << endl;
				}
			} else {
				mpfr_t f_value1, f_value2, f_value3, f_value4;std::cout << "Number.cc-3767" << endl;
				mpfr_inits2(BIT_PRECISION, f_value1, f_value2, f_value3, f_value4, NULL);std::cout << "Number.cc-3768" << endl;

				mpfr_pow(f_value1, fu_value, o.internalUpperFloat(), MPFR_RNDN);std::cout << "Number.cc-3770" << endl;
				mpfr_pow(f_value2, fl_value, o.internalLowerFloat(), MPFR_RNDN);std::cout << "Number.cc-3771" << endl;
				mpfr_pow(f_value3, fu_value, o.internalLowerFloat(), MPFR_RNDN);std::cout << "Number.cc-3772" << endl;
				mpfr_pow(f_value4, fl_value, o.internalUpperFloat(), MPFR_RNDN);std::cout << "Number.cc-3773" << endl;

				int i_upper = 1, i_lower = 1;std::cout << "Number.cc-3775" << endl;

				if(mpfr_cmp(f_value2, f_value1) > 0) {
					if(mpfr_cmp(f_value3, f_value2) > 0) {
						if(mpfr_cmp(f_value4, f_value3) > 0) i_upper = 4;
						else i_upper = 3;std::cout << "Number.cc-3780" << endl;
					} else if(mpfr_cmp(f_value4, f_value2) > 0) i_upper = 4;
					else i_upper = 2;std::cout << "Number.cc-3782" << endl;
				} else if(mpfr_cmp(f_value3, f_value1) > 0) {
					if(mpfr_cmp(f_value4, f_value3) > 0) i_upper = 4;
					else i_upper = 3;std::cout << "Number.cc-3785" << endl;
				} else if(mpfr_cmp(f_value4, f_value1) > 0) i_upper = 4;

				if(mpfr_cmp(f_value2, f_value1) < 0) {
					if(mpfr_cmp(f_value3, f_value2) < 0) {
						if(mpfr_cmp(f_value4, f_value3) < 0) i_lower = 4;
						else i_lower = 3;std::cout << "Number.cc-3791" << endl;
					} else if(mpfr_cmp(f_value4, f_value2) < 0) i_lower = 4;
					else i_lower = 2;std::cout << "Number.cc-3793" << endl;
				} else if(mpfr_cmp(f_value3, f_value1) < 0) {
					if(mpfr_cmp(f_value4, f_value3) < 0) i_lower = 4;
					else i_lower = 3;std::cout << "Number.cc-3796" << endl;
				} else if(mpfr_cmp(f_value4, f_value1) < 0) i_lower = 4;

				switch(i_upper) {
					case 1: {mpfr_pow(f_value1, fu_value, o.internalUpperFloat(), MPFR_RNDU); break;}
					case 2: {mpfr_pow(f_value1, fl_value, o.internalLowerFloat(), MPFR_RNDU); break;}
					case 3: {mpfr_pow(f_value1, fu_value, o.internalLowerFloat(), MPFR_RNDU); break;}
					case 4: {mpfr_pow(f_value1, fl_value, o.internalUpperFloat(), MPFR_RNDU); break;}
				}
				switch(i_lower) {
					case 1: {mpfr_pow(fl_value, fu_value, o.internalUpperFloat(), MPFR_RNDD); break;}
					case 2: {mpfr_pow(fl_value, fl_value, o.internalLowerFloat(), MPFR_RNDD); break;}
					case 3: {mpfr_pow(fl_value, fu_value, o.internalLowerFloat(), MPFR_RNDD); break;}
					case 4: {mpfr_pow(fl_value, fl_value, o.internalUpperFloat(), MPFR_RNDD); break;}
				}
				mpfr_set(fu_value, f_value1, MPFR_RNDU);std::cout << "Number.cc-3811" << endl;

				if(sgn_ou != sgn_ol) {
					if(mpfr_cmp_ui(fl_value, 1) > 0) mpfr_set_ui(fl_value, 1, MPFR_RNDD);
					else if(mpfr_cmp_ui(fu_value, 1) < 0) mpfr_set_ui(fu_value, 1, MPFR_RNDU);
				}

				mpfr_clears(f_value1, f_value2, f_value3, f_value4, NULL);std::cout << "Number.cc-3818" << endl;
			}
		}
	} else {
		if(!o.isNonNegative() && (sgn_l == 0 || sgn_l != sgn_u)) {
			//CALCULATOR->error(true, _("Division by zero."), NULL);
			set(nr_bak);std::cout << "Number.cc-3824" << endl;
			return false;
		}
		if(o.isInteger() && o.integerLength() < 1000000L) {
			if(!CREATE_INTERVAL && !isInterval()) {
				mpfr_pow_z(fl_value, fl_value, mpq_numref(o.internalRational()), MPFR_RNDN);std::cout << "Number.cc-3829" << endl;
				mpfr_set(fu_value, fl_value, MPFR_RNDN);std::cout << "Number.cc-3830" << endl;
			} else if(o.isEven() && sgn_l < 0 && sgn_u >= 0) {
				if(mpfr_cmpabs(fu_value, fl_value) < 0) {
					mpfr_pow_z(fu_value, fl_value, mpq_numref(o.internalRational()), MPFR_RNDU);std::cout << "Number.cc-3833" << endl;
				} else {
					mpfr_pow_z(fu_value, fu_value, mpq_numref(o.internalRational()), MPFR_RNDU);std::cout << "Number.cc-3835" << endl;
				}
				mpfr_set_ui(fl_value, 0, MPFR_RNDD);std::cout << "Number.cc-3837" << endl;
			} else {
				bool b_reverse = o.isEven() && sgn_l < 0;std::cout << "Number.cc-3839" << endl;
				if(o.isNegative()) {
					if(b_reverse) {
						b_reverse = false;std::cout << "Number.cc-3842" << endl;
					} else {
						b_reverse = true;std::cout << "Number.cc-3844" << endl;
					}
				}
				if(b_reverse) {
					mpfr_pow_z(fu_value, fu_value, mpq_numref(o.internalRational()), MPFR_RNDD);std::cout << "Number.cc-3848" << endl;
					mpfr_pow_z(fl_value, fl_value, mpq_numref(o.internalRational()), MPFR_RNDU);std::cout << "Number.cc-3849" << endl;
					mpfr_swap(fu_value, fl_value);std::cout << "Number.cc-3850" << endl;
				} else {
					mpfr_pow_z(fu_value, fu_value, mpq_numref(o.internalRational()), MPFR_RNDU);std::cout << "Number.cc-3852" << endl;
					mpfr_pow_z(fl_value, fl_value, mpq_numref(o.internalRational()), MPFR_RNDD);std::cout << "Number.cc-3853" << endl;
				}
			}
		} else {
			if(sgn_l < 0) {
				if(sgn_u < 0 && mpz_cmp_ui(mpq_denref(o.internalRational()), 2) == 0) {
					if(b_imag) {set(nr_bak); return false;}
					if(!i_value) {i_value = new Number(); i_value->markAsImaginaryPart();}
					i_value->set(*this, false, true);std::cout << "Number.cc-3861" << endl;
					if(!i_value->negate() || !i_value->raise(o)) {
						set(nr_bak);std::cout << "Number.cc-3863" << endl;
						return false;
					}
					if(!o.numeratorIsOne()) {
						mpz_t zrem;std::cout << "Number.cc-3867" << endl;
						mpz_init(zrem);std::cout << "Number.cc-3868" << endl;
						mpz_tdiv_r_ui(zrem, mpq_numref(o.internalRational()), 4);std::cout << "Number.cc-3869" << endl;
						if(mpz_cmp_ui(zrem, 1) != 0 && mpz_cmp_si(zrem, -3) != 0) i_value->negate();
					}
					clearReal();std::cout << "Number.cc-3872" << endl;
					setPrecisionAndApproximateFrom(*i_value);std::cout << "Number.cc-3873" << endl;
					return true;
				} else {
					try_complex = true;std::cout << "Number.cc-3876" << endl;
				}
			} else {
				if(o.numeratorIsOne() && mpz_fits_ulong_p(mpq_denref(o.internalRational()))) {
					unsigned long int i_root = mpz_get_ui(mpq_denref(o.internalRational()));std::cout << "Number.cc-3880" << endl;
					if(!CREATE_INTERVAL && !isInterval()) {
#if MPFR_VERSION_MAJOR < 4
						mpfr_root(fl_value, fl_value, i_root, MPFR_RNDN);std::cout << "Number.cc-3883" << endl;
#else
						mpfr_rootn_ui(fl_value, fl_value, i_root, MPFR_RNDN);std::cout << "Number.cc-3885" << endl;
#endif
						mpfr_set(fu_value, fl_value, MPFR_RNDN);std::cout << "Number.cc-3887" << endl;
					} else {
#if MPFR_VERSION_MAJOR < 4
						mpfr_root(fu_value, fu_value, i_root, MPFR_RNDU);std::cout << "Number.cc-3890" << endl;
						mpfr_root(fl_value, fl_value, i_root, MPFR_RNDD);std::cout << "Number.cc-3891" << endl;
#else
						mpfr_rootn_ui(fu_value, fu_value, i_root, MPFR_RNDU);std::cout << "Number.cc-3893" << endl;
						mpfr_rootn_ui(fl_value, fl_value, i_root, MPFR_RNDD);std::cout << "Number.cc-3894" << endl;
#endif
					}
				} else if(!CREATE_INTERVAL && !isInterval()) {
					mpfr_t f_pow;std::cout << "Number.cc-3898" << endl;
					mpfr_init2(f_pow, BIT_PRECISION);std::cout << "Number.cc-3899" << endl;
					mpfr_set_q(f_pow, o.internalRational(), MPFR_RNDN);std::cout << "Number.cc-3900" << endl;
					mpfr_pow(fl_value, fl_value, f_pow, MPFR_RNDN);std::cout << "Number.cc-3901" << endl;
					mpfr_set(fu_value, fl_value, MPFR_RNDN);std::cout << "Number.cc-3902" << endl;
					mpfr_clears(f_pow, NULL);std::cout << "Number.cc-3903" << endl;
				} else {
					mpfr_t f_pow_u, f_pow_l;std::cout << "Number.cc-3905" << endl;
					mpfr_init2(f_pow_u, BIT_PRECISION);std::cout << "Number.cc-3906" << endl;
					mpfr_init2(f_pow_l, BIT_PRECISION);std::cout << "Number.cc-3907" << endl;
					mpfr_set_q(f_pow_u, o.internalRational(), MPFR_RNDU);std::cout << "Number.cc-3908" << endl;
					mpfr_set_q(f_pow_l, o.internalRational(), MPFR_RNDD);std::cout << "Number.cc-3909" << endl;
					if(mpfr_equal_p(fu_value, fl_value) || mpfr_equal_p(f_pow_u, f_pow_l)) {
						if(mpfr_equal_p(fu_value, fl_value) && mpfr_equal_p(f_pow_u, f_pow_l)) {
							mpfr_pow(fu_value, fu_value, f_pow_u, MPFR_RNDU);std::cout << "Number.cc-3912" << endl;
							mpfr_pow(fl_value, fl_value, f_pow_l, MPFR_RNDD);std::cout << "Number.cc-3913" << endl;
						} else if(mpfr_cmp_ui(fl_value, 1) < 0) {
							if(o.isNegative()) {
								mpfr_pow(fu_value, fu_value, f_pow_u, MPFR_RNDD);std::cout << "Number.cc-3916" << endl;
								mpfr_pow(fl_value, fl_value, f_pow_l, MPFR_RNDU);std::cout << "Number.cc-3917" << endl;
								mpfr_swap(fu_value, fl_value);std::cout << "Number.cc-3918" << endl;
							} else {
								mpfr_pow(fu_value, fu_value, f_pow_l, MPFR_RNDU);std::cout << "Number.cc-3920" << endl;
								mpfr_pow(fl_value, fl_value, f_pow_u, MPFR_RNDD);std::cout << "Number.cc-3921" << endl;
							}
						} else {
							if(o.isNegative()) {
								mpfr_pow(fu_value, fu_value, f_pow_l, MPFR_RNDD);std::cout << "Number.cc-3925" << endl;
								mpfr_pow(fl_value, fl_value, f_pow_u, MPFR_RNDU);std::cout << "Number.cc-3926" << endl;
								mpfr_swap(fu_value, fl_value);std::cout << "Number.cc-3927" << endl;
							} else {
								mpfr_pow(fu_value, fu_value, f_pow_u, MPFR_RNDU);std::cout << "Number.cc-3929" << endl;
								mpfr_pow(fl_value, fl_value, f_pow_l, MPFR_RNDD);std::cout << "Number.cc-3930" << endl;
							}
						}
					} else {
						mpfr_t f_value1, f_value2, f_value3, f_value4;std::cout << "Number.cc-3934" << endl;
						mpfr_inits2(BIT_PRECISION, f_value1, f_value2, f_value3, f_value4, NULL);std::cout << "Number.cc-3935" << endl;

						mpfr_pow(f_value1, fu_value, f_pow_u, MPFR_RNDN);std::cout << "Number.cc-3937" << endl;
						mpfr_pow(f_value2, fl_value, f_pow_l, MPFR_RNDN);std::cout << "Number.cc-3938" << endl;
						mpfr_pow(f_value3, fu_value, f_pow_l, MPFR_RNDN);std::cout << "Number.cc-3939" << endl;
						mpfr_pow(f_value4, fl_value, f_pow_u, MPFR_RNDN);std::cout << "Number.cc-3940" << endl;

						int i_upper = 1, i_lower = 1;std::cout << "Number.cc-3942" << endl;

						if(mpfr_cmp(f_value2, f_value1) > 0) {
							if(mpfr_cmp(f_value3, f_value2) > 0) {
								if(mpfr_cmp(f_value4, f_value3) > 0) i_upper = 4;
								else i_upper = 3;std::cout << "Number.cc-3947" << endl;
							} else if(mpfr_cmp(f_value4, f_value2) > 0) i_upper = 4;
							else i_upper = 2;std::cout << "Number.cc-3949" << endl;
						} else if(mpfr_cmp(f_value3, f_value1) > 0) {
							if(mpfr_cmp(f_value4, f_value3) > 0) i_upper = 4;
							else i_upper = 3;std::cout << "Number.cc-3952" << endl;
						} else if(mpfr_cmp(f_value4, f_value1) > 0) i_upper = 4;

						if(mpfr_cmp(f_value2, f_value1) < 0) {
							if(mpfr_cmp(f_value3, f_value2) < 0) {
								if(mpfr_cmp(f_value4, f_value3) < 0) i_lower = 4;
								else i_lower = 3;std::cout << "Number.cc-3958" << endl;
							} else if(mpfr_cmp(f_value4, f_value2) < 0) i_lower = 4;
							else i_lower = 2;std::cout << "Number.cc-3960" << endl;
						} else if(mpfr_cmp(f_value3, f_value1) < 0) {
							if(mpfr_cmp(f_value4, f_value3) < 0) i_lower = 4;
							else i_lower = 3;std::cout << "Number.cc-3963" << endl;
						} else if(mpfr_cmp(f_value4, f_value1) < 0) i_lower = 4;

						switch(i_upper) {
							case 1: {mpfr_pow(f_value1, fu_value, f_pow_u, MPFR_RNDU); break;}
							case 2: {mpfr_pow(f_value1, fl_value, f_pow_l, MPFR_RNDU); break;}
							case 3: {mpfr_pow(f_value1, fu_value, f_pow_l, MPFR_RNDU); break;}
							case 4: {mpfr_pow(f_value1, fl_value, f_pow_u, MPFR_RNDU); break;}
						}
						switch(i_lower) {
							case 1: {mpfr_pow(fl_value, fu_value, f_pow_u, MPFR_RNDD); break;}
							case 2: {mpfr_pow(fl_value, fl_value, f_pow_l, MPFR_RNDD); break;}
							case 3: {mpfr_pow(fl_value, fu_value, f_pow_l, MPFR_RNDD); break;}
							case 4: {mpfr_pow(fl_value, fl_value, f_pow_u, MPFR_RNDD); break;}
						}
						mpfr_set(fu_value, f_value1, MPFR_RNDU);std::cout << "Number.cc-3978" << endl;

						mpfr_clears(f_value1, f_value2, f_value3, f_value4, NULL);std::cout << "Number.cc-3980" << endl;
					}
					mpfr_clears(f_pow_u, f_pow_l, NULL);std::cout << "Number.cc-3982" << endl;
				}
			}
		}
	}
	if(try_complex) {
		set(nr_bak);std::cout << "Number.cc-3988" << endl;
		if(sgn_l < 0 && sgn_u >= 0) {
			Number nr_neg(lowerEndPoint());std::cout << "Number.cc-3990" << endl;
			if(!nr_neg.raise(o) || (b_imag && nr_neg.hasImaginaryPart())) return false;
			Number nr_pos(upperEndPoint());std::cout << "Number.cc-3992" << endl;
			if(!nr_pos.raise(o)) return false;
			if(!nr_neg.setInterval(nr_zero, nr_neg) || !nr_pos.setInterval(nr_zero, nr_pos)) return false;
			if(!setInterval(nr_neg, nr_pos)) return false;
		} else {
			Number nbase, nexp(*this);std::cout << "Number.cc-3997" << endl;
			nbase.e();std::cout << "Number.cc-3998" << endl;
			if(!nexp.ln()) return false;
			if(!nexp.multiply(o)) return false;
			if(!nbase.raise(nexp, false)) return false;
			if(b_imag && nbase.hasImaginaryPart()) return false;
			set(nbase);std::cout << "Number.cc-4003" << endl;
		}
		return true;
	}
	if(!testFloatResult(true) || (includesInfinity() && !nr_bak.includesInfinity() && !o.includesInfinity())) {
		set(nr_bak);std::cout << "Number.cc-4008" << endl;
		return false;
	}
	setPrecisionAndApproximateFrom(o);std::cout << "Number.cc-4011" << endl;
	return true;
}
bool Number::sqrt() {
	if(hasImaginaryPart()) return raise(Number(1, 2, 0), true);
	if(isNegative()) {
		if(b_imag) return false;
		if(!i_value) {i_value = new Number(); i_value->markAsImaginaryPart();}
		i_value->set(*this, false, true);std::cout << "Number.cc-4019" << endl;
		if(!i_value->negate() || !i_value->sqrt()) {
			i_value->clear();std::cout << "Number.cc-4021" << endl;
			return false;
		}
		clearReal();std::cout << "Number.cc-4024" << endl;
		setPrecisionAndApproximateFrom(*i_value);std::cout << "Number.cc-4025" << endl;
		return true;
	}
	if(n_type == NUMBER_TYPE_RATIONAL) {
		if(mpz_perfect_square_p(mpq_numref(r_value)) && mpz_perfect_square_p(mpq_denref(r_value))) {
			mpz_sqrt(mpq_numref(r_value), mpq_numref(r_value));std::cout << "Number.cc-4030" << endl;
			mpz_sqrt(mpq_denref(r_value), mpq_denref(r_value));std::cout << "Number.cc-4031" << endl;
			return true;
		}
	}
	Number nr_bak(*this);std::cout << "Number.cc-4035" << endl;
	if(!setToFloatingPoint()) return false;
	mpfr_clear_flags();std::cout << "Number.cc-4037" << endl;

	if(!CREATE_INTERVAL && !isInterval()) {
		mpfr_sqrt(fl_value, fl_value, MPFR_RNDN);std::cout << "Number.cc-4040" << endl;
		mpfr_set(fu_value, fl_value, MPFR_RNDN);std::cout << "Number.cc-4041" << endl;
	} else {
		if(mpfr_sgn(fl_value) < 0) {
			if(b_imag) {
				set(nr_bak);std::cout << "Number.cc-4045" << endl;
				return false;
			} else {
				if(!i_value) {i_value = new Number(); i_value->markAsImaginaryPart();}
				if(mpfr_sgn(fu_value) > 0) {
					i_value->setInterval(lowerEndPoint(), nr_zero);std::cout << "Number.cc-4050" << endl;
				} else {
					i_value->set(*this, false, true);std::cout << "Number.cc-4052" << endl;
				}
				if(!i_value->abs() || !i_value->sqrt()) {set(nr_bak); return false;}
			}
			mpfr_sqrt(fu_value, fu_value, MPFR_RNDU);std::cout << "Number.cc-4056" << endl;
			mpfr_set_zero(fl_value, 0);std::cout << "Number.cc-4057" << endl;
		} else {
			mpfr_sqrt(fu_value, fu_value, MPFR_RNDU);std::cout << "Number.cc-4059" << endl;
			mpfr_sqrt(fl_value, fl_value, MPFR_RNDD);std::cout << "Number.cc-4060" << endl;
		}
	}

	if(!testFloatResult(true)) {
		set(nr_bak);std::cout << "Number.cc-4065" << endl;
		return false;
	}
	return true;
}
bool Number::cbrt() {
	if(hasImaginaryPart()) return raise(Number(1, 3, 0), true);
	if(isOne() || isMinusOne() || isZero()) return true;
	Number nr_bak(*this);std::cout << "Number.cc-4073" << endl;
	if(n_type == NUMBER_TYPE_RATIONAL) {
		if(mpz_root(mpq_numref(r_value), mpq_numref(r_value), 3) && mpz_root(mpq_denref(r_value), mpq_denref(r_value), 3)) {
			return true;
		}
		set(nr_bak);std::cout << "Number.cc-4078" << endl;
	}
	if(!setToFloatingPoint()) return false;
	mpfr_clear_flags();std::cout << "Number.cc-4081" << endl;

	if(!CREATE_INTERVAL && !isInterval()) {
		mpfr_cbrt(fl_value, fl_value, MPFR_RNDN);std::cout << "Number.cc-4084" << endl;
		mpfr_set(fu_value, fl_value, MPFR_RNDN);std::cout << "Number.cc-4085" << endl;
	} else {
		mpfr_cbrt(fu_value, fu_value, MPFR_RNDU);std::cout << "Number.cc-4087" << endl;
		mpfr_cbrt(fl_value, fl_value, MPFR_RNDD);std::cout << "Number.cc-4088" << endl;
	}

	if(!testFloatResult(true)) {
		set(nr_bak);std::cout << "Number.cc-4092" << endl;
		return false;
	}
	return true;
}
bool Number::root(const Number &o) {
	if(!o.isInteger() || !o.isPositive() || hasImaginaryPart() || (o.isEven() && !isNonNegative())) return false;
	if(isOne() || o.isOne() || isZero() || isPlusInfinity()) return true;
	if(o.isTwo()) return sqrt();
	/*if(o.isEven() && (!isReal() || isNegative())) {
		Number o_odd_factor(o);std::cout << "Number.cc-4102" << endl;
		Number o_even_factor(1, 1, 0);std::cout << "Number.cc-4103" << endl;
		while(o_odd_factor.isEven() && !o_odd_factor.isTwo()) {
			if(!o_odd_factor.multiply(nr_half) || !o_even_factor.multiply(2)) return false;
			if(CALCULATOR->aborted()) return false;
		}
		if(!o_even_factor.recip()) return false;
		Number nr_bak(*this);std::cout << "Number.cc-4109" << endl;
		if(!root(o_odd_factor)) return false;
		if(!raise(o_even_factor)) {set(nr_bak); return false;}
		return true;
	}
	if(!isReal()) {
		if(!hasRealPart()) {
			Number nr_o(o);std::cout << "Number.cc-4116" << endl;
			if(!nr_o.irem(4) || !i_value->root(o)) return false;
			if(!nr_o.isOne()) i_value->negate();
			return true;
		}
		return false;
	}*/
	if(isMinusOne()) return true;
	Number nr_bak(*this);std::cout << "Number.cc-4124" << endl;
	if(!mpz_fits_ulong_p(mpq_numref(o.internalRational()))) {

		if(!setToFloatingPoint()) return false;

		Number o_inv(o);std::cout << "Number.cc-4129" << endl;
		o_inv.recip();std::cout << "Number.cc-4130" << endl;

		mpfr_t f_pow_u, f_pow_l;std::cout << "Number.cc-4132" << endl;

		mpfr_init2(f_pow_u, BIT_PRECISION);std::cout << "Number.cc-4134" << endl;
		mpfr_init2(f_pow_l, BIT_PRECISION);std::cout << "Number.cc-4135" << endl;

		if(!CREATE_INTERVAL && !isInterval()) {
			mpfr_set_q(f_pow_l, o.internalRational(), MPFR_RNDN);std::cout << "Number.cc-4138" << endl;
			int sgn_l = mpfr_sgn(fl_value);std::cout << "Number.cc-4139" << endl;
			if(sgn_l < 0) mpfr_neg(fl_value, fl_value, MPFR_RNDN);
			mpfr_pow(fl_value, fl_value, f_pow_l, MPFR_RNDN);std::cout << "Number.cc-4141" << endl;
			if(sgn_l < 0) mpfr_neg(fl_value, fl_value, MPFR_RNDN);
			mpfr_set(fu_value, fl_value, MPFR_RNDN);std::cout << "Number.cc-4143" << endl;
		} else {
			mpfr_set_q(f_pow_u, o.internalRational(), MPFR_RNDU);std::cout << "Number.cc-4145" << endl;
			mpfr_set_q(f_pow_l, o.internalRational(), MPFR_RNDD);std::cout << "Number.cc-4146" << endl;

			int sgn_l = mpfr_sgn(fl_value), sgn_u = mpfr_sgn(fu_value);std::cout << "Number.cc-4148" << endl;

			if(sgn_u < 0) mpfr_neg(fu_value, fu_value, MPFR_RNDD);
			if(sgn_l < 0) mpfr_neg(fl_value, fl_value, MPFR_RNDU);

			mpfr_pow(fu_value, fu_value, f_pow_u, MPFR_RNDU);std::cout << "Number.cc-4153" << endl;
			mpfr_pow(fl_value, fl_value, f_pow_l, MPFR_RNDD);std::cout << "Number.cc-4154" << endl;

			if(sgn_u < 0) mpfr_neg(fu_value, fu_value, MPFR_RNDU);
			if(sgn_l < 0) mpfr_neg(fl_value, fl_value, MPFR_RNDD);
		}

		mpfr_clears(f_pow_u, f_pow_l, NULL);std::cout << "Number.cc-4160" << endl;
	} else {
		unsigned long int i_root = mpz_get_ui(mpq_numref(o.internalRational()));std::cout << "Number.cc-4162" << endl;
		if(n_type == NUMBER_TYPE_RATIONAL) {
			if(mpz_root(mpq_numref(r_value), mpq_numref(r_value), i_root) && mpz_root(mpq_denref(r_value), mpq_denref(r_value), i_root)) {
				return true;
			}
			set(nr_bak);std::cout << "Number.cc-4167" << endl;
			if(!setToFloatingPoint()) return false;
		}
		mpfr_clear_flags();std::cout << "Number.cc-4170" << endl;

		if(!CREATE_INTERVAL && !isInterval()) {
#if MPFR_VERSION_MAJOR < 4
			mpfr_root(fl_value, fl_value, i_root, MPFR_RNDN);std::cout << "Number.cc-4174" << endl;
#else
			mpfr_rootn_ui(fl_value, fl_value, i_root, MPFR_RNDN);std::cout << "Number.cc-4176" << endl;
#endif
			mpfr_set(fu_value, fl_value, MPFR_RNDN);std::cout << "Number.cc-4178" << endl;
		} else {
#if MPFR_VERSION_MAJOR < 4
			mpfr_root(fu_value, fu_value, i_root, MPFR_RNDU);std::cout << "Number.cc-4181" << endl;
			mpfr_root(fl_value, fl_value, i_root, MPFR_RNDD);std::cout << "Number.cc-4182" << endl;
#else
			mpfr_rootn_ui(fu_value, fu_value, i_root, MPFR_RNDU);std::cout << "Number.cc-4184" << endl;
			mpfr_rootn_ui(fl_value, fl_value, i_root, MPFR_RNDD);std::cout << "Number.cc-4185" << endl;
#endif
		}
	}

	if(!testFloatResult(true)) {
		set(nr_bak);std::cout << "Number.cc-4191" << endl;
		return false;
	}
	return true;
}
bool Number::allroots(const Number &o, vector<Number> &roots) {
	if(!o.isInteger() || !o.isPositive()) return false;
	if(isOne() || o.isOne() || isZero()) {
		roots.push_back(*this);std::cout << "Number.cc-4199" << endl;
		return true;
	}
	if(o.isTwo()) {
		Number nr(*this);std::cout << "Number.cc-4203" << endl;
		if(!nr.sqrt()) return false;
		roots.push_back(nr);std::cout << "Number.cc-4205" << endl;
		if(!nr.negate()) return false;
		roots.push_back(nr);std::cout << "Number.cc-4207" << endl;
		return true;
	}
	if(isInfinite()) return false;
	Number o_inv(o);std::cout << "Number.cc-4211" << endl;
	if(!o_inv.recip()) return false;
	Number nr_arg;std::cout << "Number.cc-4213" << endl;
	nr_arg.set(*this, false, true);std::cout << "Number.cc-4214" << endl;
	if(!nr_arg.atan2(*i_value)) return false;
	Number nr_pi2;std::cout << "Number.cc-4216" << endl;
	nr_pi2.pi();std::cout << "Number.cc-4217" << endl;
	nr_pi2 *= 2;std::cout << "Number.cc-4218" << endl;
	Number nr_i;std::cout << "Number.cc-4219" << endl;
	Number nr_re;std::cout << "Number.cc-4220" << endl;
	nr_re.set(*this, false, true);std::cout << "Number.cc-4221" << endl;
	Number nr_im(*i_value);std::cout << "Number.cc-4222" << endl;
	if(!nr_re.square() || !nr_im.square() || !nr_re.add(nr_im) || !nr_re.sqrt() || !nr_re.raise(o_inv)) return false;
	while(nr_i.isLessThan(o)) {
		if(CALCULATOR->aborted()) return false;
		Number nr(nr_pi2);std::cout << "Number.cc-4226" << endl;
		if(!nr.multiply(nr_i) || !nr.add(nr_arg) || !nr.multiply(nr_one_i) || !nr.multiply(o_inv) || !nr.exp() || !nr.multiply(nr_re)) return false;
		roots.push_back(nr);std::cout << "Number.cc-4228" << endl;
		nr_i++;std::cout << "Number.cc-4229" << endl;
	}
	return true;
}
bool Number::exp10(const Number &o) {
	if(isZero()) return true;
	if(o.isZero()) {
		setPrecisionAndApproximateFrom(o);std::cout << "Number.cc-4236" << endl;
		return true;
	}
	Number ten(10, 1);std::cout << "Number.cc-4239" << endl;
	if(!ten.raise(o)) {
		return false;
	}
	multiply(ten);std::cout << "Number.cc-4243" << endl;
	return true;
}
bool Number::exp10() {
	if(isZero()) {
		set(1, 1);std::cout << "Number.cc-4248" << endl;
		return true;
	}
	Number ten(10, 1);std::cout << "Number.cc-4251" << endl;
	if(!ten.raise(*this)) {
		return false;
	}
	set(ten);std::cout << "Number.cc-4255" << endl;
	return true;
}
bool Number::exp2(const Number &o) {
	if(isZero()) return true;
	if(o.isZero()) {
		setPrecisionAndApproximateFrom(o);std::cout << "Number.cc-4261" << endl;
		return true;
	}
	Number two(2, 1);std::cout << "Number.cc-4264" << endl;
	if(!two.raise(o)) {
		return false;
	}
	multiply(two);std::cout << "Number.cc-4268" << endl;
	return true;
}
bool Number::exp2() {
	if(isZero()) {
		set(1, 1);std::cout << "Number.cc-4273" << endl;
		return true;
	}
	Number two(2, 1);std::cout << "Number.cc-4276" << endl;
	if(!two.raise(*this)) {
		return false;
	}
	set(two);std::cout << "Number.cc-4280" << endl;
	return true;
}
bool Number::square() {
	if(isInfinite()) {
		n_type = NUMBER_TYPE_PLUS_INFINITY;std::cout << "Number.cc-4285" << endl;
		return true;
	}
	if(hasImaginaryPart()) {
		if(!hasRealPart()) {
			if(i_value->isFloatingPoint() && (CREATE_INTERVAL || i_value->isInterval())) {
				Number nr_bak(*this);std::cout << "Number.cc-4291" << endl;
				if(!i_value->setToFloatingPoint()) return false;
				Number *i_copy = i_value;std::cout << "Number.cc-4293" << endl;
				i_value = NULL;std::cout << "Number.cc-4294" << endl;
				set(*i_copy, true);std::cout << "Number.cc-4295" << endl;
				delete i_copy;std::cout << "Number.cc-4296" << endl;
				if(!square() || !negate()) {set(nr_bak); return false;}
				return true;
			}
		} else if((n_type == NUMBER_TYPE_FLOAT || i_value->isFloatingPoint()) && (CREATE_INTERVAL || isInterval(false))) {
			Number nr_bak(*this);std::cout << "Number.cc-4301" << endl;
			if(!setToFloatingPoint()) return false;
			if(!i_value->setToFloatingPoint()) {set(nr_bak); return false;}
			mpfr_t f_ru, f_rl, f_iu, f_il, f_tmp;std::cout << "Number.cc-4304" << endl;
			mpfr_inits2(BIT_PRECISION, f_ru, f_rl, f_iu, f_il, f_tmp, NULL);std::cout << "Number.cc-4305" << endl;
			if(mpfr_sgn(fl_value) < 0 && mpfr_sgn(fu_value) > 0) {
				mpfr_set_zero(f_rl, 0);std::cout << "Number.cc-4307" << endl;
				if(mpfr_cmpabs(fl_value, fu_value) > 0) {
					mpfr_sqr(f_ru, fl_value, MPFR_RNDU);std::cout << "Number.cc-4309" << endl;
				} else {
					mpfr_sqr(f_ru, fu_value, MPFR_RNDU);std::cout << "Number.cc-4311" << endl;
				}
			} else {
				if(mpfr_cmpabs(fl_value, fu_value) > 0) {
					mpfr_sqr(f_ru, fl_value, MPFR_RNDU);std::cout << "Number.cc-4315" << endl;
					mpfr_sqr(f_rl, fu_value, MPFR_RNDD);std::cout << "Number.cc-4316" << endl;
				} else {
					mpfr_sqr(f_rl, fl_value, MPFR_RNDD);std::cout << "Number.cc-4318" << endl;
					mpfr_sqr(f_ru, fu_value, MPFR_RNDU);std::cout << "Number.cc-4319" << endl;
				}
			}
			if(mpfr_sgn(i_value->internalLowerFloat()) < 0 && mpfr_sgn(i_value->internalUpperFloat()) > 0) {
				if(mpfr_cmpabs(i_value->internalLowerFloat(), i_value->internalUpperFloat()) > 0) {
					mpfr_sqr(f_tmp, i_value->internalLowerFloat(), MPFR_RNDU);std::cout << "Number.cc-4324" << endl;
					mpfr_sub(f_rl, f_rl, f_tmp, MPFR_RNDD);std::cout << "Number.cc-4325" << endl;
				} else {
					mpfr_sqr(f_tmp, i_value->internalUpperFloat(), MPFR_RNDU);std::cout << "Number.cc-4327" << endl;
					mpfr_sub(f_rl, f_rl, f_tmp, MPFR_RNDD);std::cout << "Number.cc-4328" << endl;
				}
			} else if(mpfr_cmpabs(i_value->internalLowerFloat(), i_value->internalUpperFloat()) > 0) {
				mpfr_sqr(f_tmp, i_value->internalLowerFloat(), MPFR_RNDU);std::cout << "Number.cc-4331" << endl;
				mpfr_sub(f_rl, f_rl, f_tmp, MPFR_RNDD);std::cout << "Number.cc-4332" << endl;
				mpfr_sqr(f_tmp, i_value->internalUpperFloat(), MPFR_RNDD);std::cout << "Number.cc-4333" << endl;
				mpfr_sub(f_ru, f_ru, f_tmp, MPFR_RNDU);std::cout << "Number.cc-4334" << endl;
			} else {
				mpfr_sqr(f_tmp, i_value->internalUpperFloat(), MPFR_RNDU);std::cout << "Number.cc-4336" << endl;
				mpfr_sub(f_rl, f_rl, f_tmp, MPFR_RNDD);std::cout << "Number.cc-4337" << endl;
				mpfr_sqr(f_tmp, i_value->internalLowerFloat(), MPFR_RNDD);std::cout << "Number.cc-4338" << endl;
				mpfr_sub(f_ru, f_ru, f_tmp, MPFR_RNDU);std::cout << "Number.cc-4339" << endl;
			}

			bool neg_rl = mpfr_sgn(fl_value) < 0, neg_ru = mpfr_sgn(fu_value) < 0, neg_il = mpfr_sgn(i_value->internalLowerFloat()) < 0, neg_iu = mpfr_sgn(i_value->internalUpperFloat()) < 0;std::cout << "Number.cc-4342" << endl;
			if(neg_rl && !neg_ru && !neg_il) {
				mpfr_mul(f_il, fl_value, i_value->internalUpperFloat(), MPFR_RNDD);std::cout << "Number.cc-4344" << endl;
				mpfr_mul(f_iu, fu_value, i_value->internalUpperFloat(), MPFR_RNDU);std::cout << "Number.cc-4345" << endl;
			} else if(!neg_rl && neg_il && !neg_iu) {
				mpfr_mul(f_il, fu_value, i_value->internalLowerFloat(), MPFR_RNDD);std::cout << "Number.cc-4347" << endl;
				mpfr_mul(f_iu, fu_value, i_value->internalUpperFloat(), MPFR_RNDU);std::cout << "Number.cc-4348" << endl;
			} else if(neg_rl && neg_ru && !neg_il) {
				mpfr_mul(f_il, fl_value, i_value->internalUpperFloat(), MPFR_RNDD);std::cout << "Number.cc-4350" << endl;
				mpfr_mul(f_iu, fu_value, i_value->internalLowerFloat(), MPFR_RNDU);std::cout << "Number.cc-4351" << endl;
			} else if(!neg_rl && neg_il && neg_iu) {
				mpfr_mul(f_il, fu_value, i_value->internalLowerFloat(), MPFR_RNDD);std::cout << "Number.cc-4353" << endl;
				mpfr_mul(f_iu, fl_value, i_value->internalUpperFloat(), MPFR_RNDU);std::cout << "Number.cc-4354" << endl;
			} else if(neg_rl && neg_ru && neg_il && !neg_iu) {
				mpfr_mul(f_il, fl_value, i_value->internalUpperFloat(), MPFR_RNDD);std::cout << "Number.cc-4356" << endl;
				mpfr_mul(f_iu, fl_value, i_value->internalLowerFloat(), MPFR_RNDU);std::cout << "Number.cc-4357" << endl;
			} else if(neg_il && neg_iu && neg_rl && !neg_ru) {
				mpfr_mul(f_il, fu_value, i_value->internalLowerFloat(), MPFR_RNDD);std::cout << "Number.cc-4359" << endl;
				mpfr_mul(f_iu, fl_value, i_value->internalLowerFloat(), MPFR_RNDU);std::cout << "Number.cc-4360" << endl;
			} else if(neg_rl && !neg_ru && neg_il && !neg_iu) {
				mpfr_mul(f_il, fu_value, i_value->internalLowerFloat(), MPFR_RNDD);std::cout << "Number.cc-4362" << endl;
				mpfr_mul(f_tmp, fl_value, i_value->internalUpperFloat(), MPFR_RNDD);std::cout << "Number.cc-4363" << endl;
				if(mpfr_cmp(f_tmp, f_il) < 0) mpfr_swap(f_il, f_tmp);
				mpfr_mul(f_iu, fu_value, i_value->internalUpperFloat(), MPFR_RNDU);std::cout << "Number.cc-4365" << endl;
				mpfr_mul(f_tmp, fl_value, i_value->internalLowerFloat(), MPFR_RNDU);std::cout << "Number.cc-4366" << endl;
				if(mpfr_cmp(f_tmp, f_iu) > 0) mpfr_swap(f_iu, f_tmp);
			} else if(neg_rl && neg_ru && neg_il && neg_iu) {
				mpfr_mul(f_iu, fl_value, i_value->internalLowerFloat(), MPFR_RNDU);std::cout << "Number.cc-4369" << endl;
				mpfr_mul(f_il, fu_value, i_value->internalUpperFloat(), MPFR_RNDD);std::cout << "Number.cc-4370" << endl;
			} else {
				mpfr_mul(f_il, fl_value, i_value->internalLowerFloat(), MPFR_RNDD);std::cout << "Number.cc-4372" << endl;
				mpfr_mul(f_iu, fu_value, i_value->internalUpperFloat(), MPFR_RNDU);std::cout << "Number.cc-4373" << endl;
			}
			mpfr_mul_ui(f_il, f_il, 2, MPFR_RNDD);std::cout << "Number.cc-4375" << endl;
			mpfr_mul_ui(f_iu, f_iu, 2, MPFR_RNDU);std::cout << "Number.cc-4376" << endl;
			mpfr_swap(f_rl, fl_value);std::cout << "Number.cc-4377" << endl;
			mpfr_swap(f_ru, fu_value);std::cout << "Number.cc-4378" << endl;
			mpfr_swap(f_il, i_value->internalLowerFloat());std::cout << "Number.cc-4379" << endl;
			mpfr_swap(f_iu, i_value->internalUpperFloat());std::cout << "Number.cc-4380" << endl;
			mpfr_clears(f_ru, f_rl, f_iu, f_il, f_tmp, NULL);std::cout << "Number.cc-4381" << endl;
			if(!i_value->testFloatResult(true) || !testFloatResult(true)) {
				set(nr_bak);std::cout << "Number.cc-4383" << endl;
				return false;
			}
			return true;
		}
		Number nr(*this);std::cout << "Number.cc-4388" << endl;
		return multiply(nr);
	}
	if(n_type == NUMBER_TYPE_RATIONAL) {
		mpq_mul(r_value, r_value, r_value);std::cout << "Number.cc-4392" << endl;
	} else {
		if(!CREATE_INTERVAL && !isInterval()) {
			mpfr_sqr(fl_value, fl_value, MPFR_RNDN);std::cout << "Number.cc-4395" << endl;
			mpfr_set(fu_value, fl_value, MPFR_RNDN);std::cout << "Number.cc-4396" << endl;
		} else if(mpfr_sgn(fl_value) < 0) {
			if(mpfr_sgn(fu_value) < 0) {
				mpfr_sqr(fu_value, fu_value, MPFR_RNDD);std::cout << "Number.cc-4399" << endl;
				mpfr_sqr(fl_value, fl_value, MPFR_RNDU);std::cout << "Number.cc-4400" << endl;
				mpfr_swap(fu_value, fl_value);std::cout << "Number.cc-4401" << endl;
			} else {
				if(mpfr_cmpabs(fu_value, fl_value) < 0) mpfr_sqr(fu_value, fl_value, MPFR_RNDU);
				else mpfr_sqr(fu_value, fu_value, MPFR_RNDU);std::cout << "Number.cc-4404" << endl;
				mpfr_set_zero(fl_value, 0);std::cout << "Number.cc-4405" << endl;
			}
		} else {
			mpfr_sqr(fu_value, fu_value, MPFR_RNDU);std::cout << "Number.cc-4408" << endl;
			mpfr_sqr(fl_value, fl_value, MPFR_RNDD);std::cout << "Number.cc-4409" << endl;
		}
		testFloatResult(true, 2);std::cout << "Number.cc-4411" << endl;
	}
	return true;
}

bool Number::negate() {
	if(i_value) i_value->negate();
	switch(n_type) {
		case NUMBER_TYPE_PLUS_INFINITY: {
			n_type = NUMBER_TYPE_MINUS_INFINITY;std::cout << "Number.cc-4420" << endl;
			break;
		}
		case NUMBER_TYPE_MINUS_INFINITY: {
			n_type = NUMBER_TYPE_PLUS_INFINITY;std::cout << "Number.cc-4424" << endl;
			break;
		}
		case NUMBER_TYPE_RATIONAL: {
			mpq_neg(r_value, r_value);std::cout << "Number.cc-4428" << endl;
			break;
		}
		case NUMBER_TYPE_FLOAT: {
			mpfr_clear_flags();std::cout << "Number.cc-4432" << endl;
			if(!CREATE_INTERVAL && !isInterval()) {
				mpfr_neg(fl_value, fl_value, MPFR_RNDN);std::cout << "Number.cc-4434" << endl;
				mpfr_set(fu_value, fl_value, MPFR_RNDN);std::cout << "Number.cc-4435" << endl;
			} else {
				mpfr_neg(fu_value, fu_value, MPFR_RNDD);std::cout << "Number.cc-4437" << endl;
				mpfr_neg(fl_value, fl_value, MPFR_RNDU);std::cout << "Number.cc-4438" << endl;
				mpfr_swap(fu_value, fl_value);std::cout << "Number.cc-4439" << endl;
			}
			testFloatResult(true, 2);std::cout << "Number.cc-4441" << endl;
			break;
		}
		default: {break;}
	}
	return true;
}
void Number::setNegative(bool is_negative) {
	switch(n_type) {
		case NUMBER_TYPE_PLUS_INFINITY: {
			if(is_negative) n_type = NUMBER_TYPE_MINUS_INFINITY;
			break;
		}
		case NUMBER_TYPE_MINUS_INFINITY: {
			if(!is_negative) n_type = NUMBER_TYPE_PLUS_INFINITY;
			break;
		}
		case NUMBER_TYPE_RATIONAL: {
			if(is_negative != (mpq_sgn(r_value) < 0)) mpq_neg(r_value, r_value);
			break;
		}
		case NUMBER_TYPE_FLOAT: {
			mpfr_clear_flags();std::cout << "Number.cc-4463" << endl;
			if(mpfr_sgn(fl_value) != mpfr_sgn(fu_value)) {
				if(is_negative) {
					mpfr_neg(fu_value, fu_value, MPFR_RNDU);std::cout << "Number.cc-4466" << endl;
					if(mpfr_cmp(fl_value, fu_value) < 0) mpfr_swap(fu_value, fl_value);
					mpfr_set_zero(fu_value, 0);std::cout << "Number.cc-4468" << endl;
				} else {
					mpfr_abs(fl_value, fl_value, MPFR_RNDU);std::cout << "Number.cc-4470" << endl;
					if(mpfr_cmp(fl_value, fu_value) > 0) mpfr_swap(fu_value, fl_value);
					mpfr_set_zero(fl_value, 0);std::cout << "Number.cc-4472" << endl;
				}
			} else if(is_negative != (mpfr_sgn(fl_value) < 0)) {
				if(!CREATE_INTERVAL && !isInterval()) {
					mpfr_neg(fl_value, fl_value, MPFR_RNDN);std::cout << "Number.cc-4476" << endl;
					mpfr_set(fu_value, fl_value, MPFR_RNDN);std::cout << "Number.cc-4477" << endl;
				} else {
					mpfr_neg(fu_value, fu_value, MPFR_RNDD);std::cout << "Number.cc-4479" << endl;
					mpfr_neg(fl_value, fl_value, MPFR_RNDU);std::cout << "Number.cc-4480" << endl;
					mpfr_swap(fu_value, fl_value);std::cout << "Number.cc-4481" << endl;
				}
				testFloatResult(true, 2);std::cout << "Number.cc-4483" << endl;
			}
			break;
		}
		default: {break;}
	}
}
bool Number::abs() {
	if(hasImaginaryPart()) {
		if(hasRealPart()) {
			Number nr_bak(*this);std::cout << "Number.cc-4493" << endl;
			if(!i_value->square()) return false;
			Number *i_v = i_value;std::cout << "Number.cc-4495" << endl;
			i_value = NULL;std::cout << "Number.cc-4496" << endl;
			if(!square() || !add(*i_v)) {
				set(nr_bak);std::cout << "Number.cc-4498" << endl;
				return false;
			}
			i_v->clear();std::cout << "Number.cc-4501" << endl;
			i_value = i_v;std::cout << "Number.cc-4502" << endl;
			if(!raise(nr_half)) {
				set(nr_bak);std::cout << "Number.cc-4504" << endl;
				return false;
			}
			return true;
		}
		set(*i_value, true, true);std::cout << "Number.cc-4509" << endl;
		clearImaginary();std::cout << "Number.cc-4510" << endl;
	}
	if(isInfinite()) {
		n_type = NUMBER_TYPE_PLUS_INFINITY;std::cout << "Number.cc-4513" << endl;
		return true;
	}
	if(n_type == NUMBER_TYPE_RATIONAL) {
		mpq_abs(r_value, r_value);std::cout << "Number.cc-4517" << endl;
	} else {
		if(mpfr_sgn(fl_value) != mpfr_sgn(fu_value)) {
			mpfr_abs(fl_value, fl_value, MPFR_RNDU);std::cout << "Number.cc-4520" << endl;
			if(mpfr_cmp(fl_value, fu_value) > 0) mpfr_swap(fu_value, fl_value);
			mpfr_set_zero(fl_value, 0);std::cout << "Number.cc-4522" << endl;
		} else if(mpfr_sgn(fl_value) < 0) {
			if(!CREATE_INTERVAL && !isInterval()) {
				mpfr_neg(fl_value, fl_value, MPFR_RNDN);std::cout << "Number.cc-4525" << endl;
				mpfr_set(fu_value, fl_value, MPFR_RNDN);std::cout << "Number.cc-4526" << endl;
			} else {
				mpfr_neg(fu_value, fu_value, MPFR_RNDD);std::cout << "Number.cc-4528" << endl;
				mpfr_neg(fl_value, fl_value, MPFR_RNDU);std::cout << "Number.cc-4529" << endl;
				mpfr_swap(fu_value, fl_value);std::cout << "Number.cc-4530" << endl;
			}
			testFloatResult(true, 2);std::cout << "Number.cc-4532" << endl;
		}
	}
	return true;
}
bool Number::signum() {
	if(isZero()) return true;
	if(hasImaginaryPart()) {
		if(hasRealPart()) {
			Number nabs(*this);std::cout << "Number.cc-4541" << endl;
			if(!nabs.abs() || !nabs.recip()) return false;
			return multiply(nabs);
		}
		return i_value->signum();
	}
	if(isPositive()) {set(1, 1); return true;}
	if(isNegative()) {set(-1, 1); return true;}
	return false;
}
bool Number::round(bool halfway_to_even) {
	if(includesInfinity() || hasImaginaryPart()) return false;
	if(n_type == NUMBER_TYPE_RATIONAL) {
		if(!isInteger()) {
			mpz_t i_rem;std::cout << "Number.cc-4555" << endl;
			mpz_init(i_rem);std::cout << "Number.cc-4556" << endl;
			mpz_mul_ui(mpq_numref(r_value), mpq_numref(r_value), 2);std::cout << "Number.cc-4557" << endl;
			mpz_add(mpq_numref(r_value), mpq_numref(r_value), mpq_denref(r_value));std::cout << "Number.cc-4558" << endl;
			mpz_mul_ui(mpq_denref(r_value), mpq_denref(r_value), 2);std::cout << "Number.cc-4559" << endl;
			mpz_fdiv_qr(mpq_numref(r_value), i_rem, mpq_numref(r_value), mpq_denref(r_value));std::cout << "Number.cc-4560" << endl;
			mpz_set_ui(mpq_denref(r_value), 1);std::cout << "Number.cc-4561" << endl;
			if(mpz_sgn(i_rem) == 0 && (!halfway_to_even || mpz_odd_p(mpq_numref(r_value)))) {
				if(halfway_to_even) mpz_sub(mpq_numref(r_value), mpq_numref(r_value), mpq_denref(r_value));
				else if(mpz_sgn(mpq_numref(r_value)) <= 0) mpz_sub(mpq_numref(r_value), mpq_numref(r_value), mpq_denref(r_value));
			}
			mpz_clear(i_rem);std::cout << "Number.cc-4566" << endl;
		}
	} else {
		mpz_set_ui(mpq_denref(r_value), 1);std::cout << "Number.cc-4569" << endl;
		intervalToMidValue();std::cout << "Number.cc-4570" << endl;
		if(isRational()) return true;
		if(!halfway_to_even) mpfr_rint_round(fl_value, fl_value, MPFR_RNDN);
		mpfr_get_z(mpq_numref(r_value), fl_value, MPFR_RNDN);std::cout << "Number.cc-4573" << endl;
		n_type = NUMBER_TYPE_RATIONAL;std::cout << "Number.cc-4574" << endl;
		mpfr_clears(fl_value, fu_value, NULL);std::cout << "Number.cc-4575" << endl;
	}
	return true;
}
bool Number::floor() {
	if(isInfinite() || hasImaginaryPart()) return false;
	//if(b_approx && !isInteger()) b_approx = false;
	if(n_type == NUMBER_TYPE_RATIONAL) {
		if(!isInteger()) {
			mpz_fdiv_q(mpq_numref(r_value), mpq_numref(r_value), mpq_denref(r_value));std::cout << "Number.cc-4584" << endl;
			mpz_set_ui(mpq_denref(r_value), 1);std::cout << "Number.cc-4585" << endl;
		}
	} else {
		if(mpfr_inf_p(fl_value)) return false;
		mpz_set_ui(mpq_denref(r_value), 1);std::cout << "Number.cc-4589" << endl;
		mpfr_get_z(mpq_numref(r_value), fl_value, MPFR_RNDD);std::cout << "Number.cc-4590" << endl;
		n_type = NUMBER_TYPE_RATIONAL;std::cout << "Number.cc-4591" << endl;
		mpfr_clears(fl_value, fu_value, NULL);std::cout << "Number.cc-4592" << endl;
	}
	return true;
}
bool Number::ceil() {
	if(isInfinite() || hasImaginaryPart()) return false;
	//if(b_approx && !isInteger()) b_approx = false;
	if(n_type == NUMBER_TYPE_RATIONAL) {
		if(!isInteger()) {
			mpz_cdiv_q(mpq_numref(r_value), mpq_numref(r_value), mpq_denref(r_value));std::cout << "Number.cc-4601" << endl;
			mpz_set_ui(mpq_denref(r_value), 1);std::cout << "Number.cc-4602" << endl;
		}
	} else {
		if(mpfr_inf_p(fu_value)) return false;
		mpz_set_ui(mpq_denref(r_value), 1);std::cout << "Number.cc-4606" << endl;
		mpfr_get_z(mpq_numref(r_value), fu_value, MPFR_RNDU);std::cout << "Number.cc-4607" << endl;
		n_type = NUMBER_TYPE_RATIONAL;std::cout << "Number.cc-4608" << endl;
		mpfr_clears(fl_value, fu_value, NULL);std::cout << "Number.cc-4609" << endl;
	}
	return true;
}
bool Number::trunc() {
	if(isInfinite() || hasImaginaryPart()) return false;
	//if(b_approx && !isInteger()) b_approx = false;
	if(n_type == NUMBER_TYPE_RATIONAL) {
		if(!isInteger()) {
			mpz_tdiv_q(mpq_numref(r_value), mpq_numref(r_value), mpq_denref(r_value));std::cout << "Number.cc-4618" << endl;
			mpz_set_ui(mpq_denref(r_value), 1);std::cout << "Number.cc-4619" << endl;
		}
	} else {
		if(mpfr_inf_p(fl_value) && mpfr_inf_p(fu_value)) return false;
		mpz_set_ui(mpq_denref(r_value), 1);std::cout << "Number.cc-4623" << endl;
		if(mpfr_sgn(fu_value) <= 0) mpfr_get_z(mpq_numref(r_value), fu_value, MPFR_RNDU);
		else if(mpfr_sgn(fl_value) >= 0) mpfr_get_z(mpq_numref(r_value), fl_value, MPFR_RNDD);
		else mpz_set_ui(mpq_numref(r_value), 0);std::cout << "Number.cc-4626" << endl;
		n_type = NUMBER_TYPE_RATIONAL;std::cout << "Number.cc-4627" << endl;
		mpfr_clears(fl_value, fu_value, NULL);std::cout << "Number.cc-4628" << endl;
	}
	return true;
}
bool Number::round(const Number &o, bool halfway_to_even) {
	if(isInfinite() || o.isInfinite()) {
		return divide(o) && round();
	}
	if(hasImaginaryPart()) return false;
	if(o.hasImaginaryPart()) return false;
	return divide(o) && round(halfway_to_even);
}
bool Number::floor(const Number &o) {
	if(isInfinite() || o.isInfinite()) {
		return divide(o) && floor();
	}
	if(hasImaginaryPart()) return false;
	if(o.hasImaginaryPart()) return false;
	return divide(o) && floor();
}
bool Number::ceil(const Number &o) {
	if(isInfinite() || o.isInfinite()) {
		return divide(o) && ceil();
	}
	if(hasImaginaryPart()) return false;
	if(o.hasImaginaryPart()) return false;
	return divide(o) && ceil();
}
bool Number::trunc(const Number &o) {
	if(isInfinite() || o.isInfinite()) {
		return divide(o) && trunc();
	}
	if(hasImaginaryPart()) return false;
	if(o.hasImaginaryPart()) return false;
	return divide(o) && trunc();
}
bool Number::mod(const Number &o) {
	if(includesInfinity() || o.includesInfinity()) return false;
	if(hasImaginaryPart() || o.hasImaginaryPart()) return false;
	if(o.isZero()) return false;
	if(isRational() && o.isRational()) {
		if(isInteger() && o.isInteger()) {
			mpz_fdiv_r(mpq_numref(r_value), mpq_numref(r_value), mpq_numref(o.internalRational()));std::cout << "Number.cc-4670" << endl;
		} else {
			mpq_div(r_value, r_value, o.internalRational());std::cout << "Number.cc-4672" << endl;
			mpz_fdiv_r(mpq_numref(r_value), mpq_numref(r_value), mpq_denref(r_value));std::cout << "Number.cc-4673" << endl;
			mpq_mul(r_value, r_value, o.internalRational());std::cout << "Number.cc-4674" << endl;
		}
	} else {
		// TODO: Interval too wide when o is interval
		if(!divide(o) || !frac()) return false;
		if(isNegative()) {
			(*this)++;std::cout << "Number.cc-4680" << endl;
			testFloatResult(true, 2);std::cout << "Number.cc-4681" << endl;
		}
		return multiply(o);
	}
	setPrecisionAndApproximateFrom(o);std::cout << "Number.cc-4685" << endl;
	return true;
}
bool Number::frac() {
	if(includesInfinity() || hasImaginaryPart()) return false;
	if(n_type == NUMBER_TYPE_RATIONAL) {
		if(isInteger()) {
			clear();std::cout << "Number.cc-4692" << endl;
		} else {
			mpz_tdiv_r(mpq_numref(r_value), mpq_numref(r_value), mpq_denref(r_value));std::cout << "Number.cc-4694" << endl;
		}
	} else {
		mpfr_clear_flags();std::cout << "Number.cc-4697" << endl;
		if(!CREATE_INTERVAL && !isInterval()) {
			mpfr_frac(fl_value, fl_value, MPFR_RNDN);std::cout << "Number.cc-4699" << endl;
			mpfr_set(fu_value, fl_value, MPFR_RNDN);std::cout << "Number.cc-4700" << endl;
		} else if(!isInterval()) {
			mpfr_frac(fl_value, fl_value, MPFR_RNDD);std::cout << "Number.cc-4702" << endl;
			mpfr_frac(fu_value, fu_value, MPFR_RNDU);std::cout << "Number.cc-4703" << endl;
		} else {
			mpfr_t testf, testu;std::cout << "Number.cc-4705" << endl;
			mpfr_inits2(mpfr_get_prec(fl_value), testf, testu, NULL);std::cout << "Number.cc-4706" << endl;
			mpfr_trunc(testf, fl_value);std::cout << "Number.cc-4707" << endl;
			mpfr_trunc(testu, fu_value);std::cout << "Number.cc-4708" << endl;
			if(!mpfr_equal_p(testf, testu)) {
				mpfr_set_zero(fl_value, 0);std::cout << "Number.cc-4710" << endl;
				mpfr_set_ui(fu_value, 1, MPFR_RNDU);std::cout << "Number.cc-4711" << endl;
			} else {
				mpfr_frac(testf, fl_value, MPFR_RNDU);std::cout << "Number.cc-4713" << endl;
				mpfr_frac(testu, fu_value, MPFR_RNDU);std::cout << "Number.cc-4714" << endl;
				if(mpfr_cmp(testf, testu) > 0) {
					mpfr_frac(fu_value, fl_value, MPFR_RNDU);std::cout << "Number.cc-4716" << endl;
					mpfr_frac(fl_value, fu_value, MPFR_RNDD);std::cout << "Number.cc-4717" << endl;
				} else {
					mpfr_frac(fl_value, fl_value, MPFR_RNDD);std::cout << "Number.cc-4719" << endl;
					mpfr_frac(fu_value, fu_value, MPFR_RNDU);std::cout << "Number.cc-4720" << endl;
				}
			}
			mpfr_clears(testf, testu, NULL);std::cout << "Number.cc-4723" << endl;
		}
		testFloatResult(true, 2);std::cout << "Number.cc-4725" << endl;
	}
	return true;
}
bool Number::rem(const Number &o) {
	if(includesInfinity() || o.includesInfinity()) return false;
	if(hasImaginaryPart() || o.hasImaginaryPart()) return false;
	if(o.isZero()) return false;
	if(isRational() && o.isRational()) {
		if(isInteger() && o.isInteger()) {
			mpz_tdiv_r(mpq_numref(r_value), mpq_numref(r_value), mpq_numref(o.internalRational()));std::cout << "Number.cc-4735" << endl;
		} else {
			mpq_div(r_value, r_value, o.internalRational());std::cout << "Number.cc-4737" << endl;
			mpz_tdiv_r(mpq_numref(r_value), mpq_numref(r_value), mpq_denref(r_value));std::cout << "Number.cc-4738" << endl;
			mpq_mul(r_value, r_value, o.internalRational());std::cout << "Number.cc-4739" << endl;
		}
	} else {
		// TODO: Interval too wide when o is interval
		return divide(o) && frac() && multiply(o);
	}
	setPrecisionAndApproximateFrom(o);std::cout << "Number.cc-4745" << endl;
	return true;
}

bool Number::smod(const Number &o) {
	if(!isInteger() || !o.isInteger()) return false;
	mpz_t b2;std::cout << "Number.cc-4751" << endl;
	mpz_init(b2);std::cout << "Number.cc-4752" << endl;
	mpz_div_ui(b2, mpq_numref(o.internalRational()), 2);std::cout << "Number.cc-4753" << endl;
	mpz_sub_ui(b2, b2, 1);std::cout << "Number.cc-4754" << endl;
	mpz_add(mpq_numref(r_value), mpq_numref(r_value), b2);std::cout << "Number.cc-4755" << endl;
	mpz_fdiv_r(mpq_numref(r_value), mpq_numref(r_value), mpq_numref(o.internalRational()));std::cout << "Number.cc-4756" << endl;
	mpz_sub(mpq_numref(r_value), mpq_numref(r_value), b2);std::cout << "Number.cc-4757" << endl;
	mpz_clear(b2);std::cout << "Number.cc-4758" << endl;
	setPrecisionAndApproximateFrom(o);std::cout << "Number.cc-4759" << endl;
	return true;
}
bool Number::irem(const Number &o) {
	if(o.isZero()) return false;
	if(!isInteger() || !o.isInteger()) return false;
	mpz_tdiv_r(mpq_numref(r_value), mpq_numref(r_value), mpq_numref(o.internalRational()));std::cout << "Number.cc-4765" << endl;
	return true;
}
bool Number::irem(const Number &o, Number &q) {
	if(o.isZero()) return false;
	if(!isInteger() || !o.isInteger()) return false;
	q.set(1, 0);std::cout << "Number.cc-4771" << endl;
	mpz_tdiv_qr(mpq_numref(q.internalRational()), mpq_numref(r_value), mpq_numref(r_value), mpq_numref(o.internalRational()));std::cout << "Number.cc-4772" << endl;
	return true;
}
bool Number::iquo(const Number &o) {
	if(o.isZero()) return false;
	if(!isInteger() || !o.isInteger()) return false;
	mpz_tdiv_q(mpq_numref(r_value), mpq_numref(r_value), mpq_numref(o.internalRational()));std::cout << "Number.cc-4778" << endl;
	return true;
}
bool Number::iquo(unsigned long int i) {
	if(i == 0) return false;
	if(!isInteger()) return false;
	mpz_tdiv_q_ui(mpq_numref(r_value), mpq_numref(r_value), i);std::cout << "Number.cc-4784" << endl;
	return true;
}
bool Number::iquo(const Number &o, Number &r) {
	if(o.isZero()) return false;
	if(!isInteger() || !o.isInteger()) return false;
	r.set(1, 0);std::cout << "Number.cc-4790" << endl;
	mpz_tdiv_qr(mpq_numref(r_value), mpq_numref(r.internalRational()), mpq_numref(r_value), mpq_numref(o.internalRational()));std::cout << "Number.cc-4791" << endl;
	return true;
}
bool Number::isIntegerDivisible(const Number &o) const {
	if(!isInteger() || !o.isInteger()) return false;
	return mpz_divisible_p(mpq_numref(r_value), mpq_numref(o.internalRational()));
}
bool Number::isqrt() {
	if(isInteger()) {
		if(mpz_sgn(mpq_numref(r_value)) < 0) return false;
		mpz_sqrt(mpq_numref(r_value), mpq_numref(r_value));std::cout << "Number.cc-4801" << endl;
		return true;
	}
	return false;
}
bool Number::isPerfectSquare() const {
	if(isInteger()) {
		if(mpz_sgn(mpq_numref(r_value)) < 0) return false;
		return mpz_perfect_square_p(mpq_numref(r_value)) != 0;
	}
	return false;
}

int Number::getBoolean() const {
	if(isNonZero()) {
		return 1;
	} else if(isZero()) {
		return 0;
	}
	return -1;
}
void Number::toBoolean() {
	setTrue(isNonZero());std::cout << "Number.cc-4823" << endl;
}
void Number::setTrue(bool is_true) {
	if(is_true) {
		set(1, 0);std::cout << "Number.cc-4827" << endl;
	} else {
		clear();std::cout << "Number.cc-4829" << endl;
	}
}
void Number::setFalse() {
	setTrue(false);std::cout << "Number.cc-4833" << endl;
}
void Number::setLogicalNot() {
	setTrue(!isNonZero());std::cout << "Number.cc-4836" << endl;
}

void Number::e(bool use_cached_number) {
	if(use_cached_number) {
		if(nr_e.isZero() || CREATE_INTERVAL != nr_e.isInterval() || mpfr_get_prec(nr_e.internalLowerFloat()) < BIT_PRECISION) {
			nr_e.e(false);std::cout << "Number.cc-4842" << endl;
		}
		set(nr_e);std::cout << "Number.cc-4844" << endl;
	} else {
		if(n_type != NUMBER_TYPE_FLOAT) {
			mpfr_init2(fu_value, BIT_PRECISION);std::cout << "Number.cc-4847" << endl;
			mpfr_init2(fl_value, BIT_PRECISION);std::cout << "Number.cc-4848" << endl;
			mpq_set_ui(r_value, 0, 1);std::cout << "Number.cc-4849" << endl;
		} else {
			if(mpfr_get_prec(fu_value) < BIT_PRECISION) mpfr_set_prec(fu_value, BIT_PRECISION);
			if(mpfr_get_prec(fl_value) < BIT_PRECISION) mpfr_set_prec(fl_value, BIT_PRECISION);
		}
		n_type = NUMBER_TYPE_FLOAT;std::cout << "Number.cc-4854" << endl;
		if(!CREATE_INTERVAL) {
			mpfr_set_ui(fl_value, 1, MPFR_RNDN);std::cout << "Number.cc-4856" << endl;
			mpfr_exp(fl_value, fl_value, MPFR_RNDN);std::cout << "Number.cc-4857" << endl;
			mpfr_set(fu_value, fl_value, MPFR_RNDN);std::cout << "Number.cc-4858" << endl;
			i_precision = FROM_BIT_PRECISION(BIT_PRECISION);std::cout << "Number.cc-4859" << endl;
		} else {
			mpfr_set_ui(fl_value, 1, MPFR_RNDD);std::cout << "Number.cc-4861" << endl;
			mpfr_set_ui(fu_value, 1, MPFR_RNDU);std::cout << "Number.cc-4862" << endl;
			mpfr_exp(fu_value, fu_value, MPFR_RNDU);std::cout << "Number.cc-4863" << endl;
			mpfr_exp(fl_value, fl_value, MPFR_RNDD);std::cout << "Number.cc-4864" << endl;
		}
	}
	b_approx = true;std::cout << "Number.cc-4867" << endl;
}
void Number::pi() {
	if(n_type != NUMBER_TYPE_FLOAT) {
		mpfr_init2(fu_value, BIT_PRECISION);std::cout << "Number.cc-4871" << endl;
		mpfr_init2(fl_value, BIT_PRECISION);std::cout << "Number.cc-4872" << endl;
		mpq_set_ui(r_value, 0, 1);std::cout << "Number.cc-4873" << endl;
	} else {
		if(mpfr_get_prec(fu_value) < BIT_PRECISION) mpfr_set_prec(fu_value, BIT_PRECISION);
		if(mpfr_get_prec(fl_value) < BIT_PRECISION) mpfr_set_prec(fl_value, BIT_PRECISION);
	}
	n_type = NUMBER_TYPE_FLOAT;std::cout << "Number.cc-4878" << endl;
	if(!CREATE_INTERVAL) {
		mpfr_const_pi(fl_value, MPFR_RNDN);std::cout << "Number.cc-4880" << endl;
		mpfr_set(fu_value, fl_value, MPFR_RNDN);std::cout << "Number.cc-4881" << endl;
		i_precision = FROM_BIT_PRECISION(BIT_PRECISION);std::cout << "Number.cc-4882" << endl;
	} else {
		mpfr_const_pi(fu_value, MPFR_RNDU);std::cout << "Number.cc-4884" << endl;
		mpfr_const_pi(fl_value, MPFR_RNDD);std::cout << "Number.cc-4885" << endl;
	}
	b_approx = true;std::cout << "Number.cc-4887" << endl;
}
void Number::catalan() {
	if(n_type != NUMBER_TYPE_FLOAT) {
		mpfr_init2(fu_value, BIT_PRECISION);std::cout << "Number.cc-4891" << endl;
		mpfr_init2(fl_value, BIT_PRECISION);std::cout << "Number.cc-4892" << endl;
		mpq_set_ui(r_value, 0, 1);std::cout << "Number.cc-4893" << endl;
	} else {
		if(mpfr_get_prec(fu_value) < BIT_PRECISION) mpfr_set_prec(fu_value, BIT_PRECISION);
		if(mpfr_get_prec(fl_value) < BIT_PRECISION) mpfr_set_prec(fl_value, BIT_PRECISION);
	}
	n_type = NUMBER_TYPE_FLOAT;std::cout << "Number.cc-4898" << endl;
	if(!CREATE_INTERVAL) {
		mpfr_const_catalan(fl_value, MPFR_RNDN);std::cout << "Number.cc-4900" << endl;
		mpfr_set(fu_value, fl_value, MPFR_RNDN);std::cout << "Number.cc-4901" << endl;
		i_precision = FROM_BIT_PRECISION(BIT_PRECISION);std::cout << "Number.cc-4902" << endl;
	} else {
		mpfr_const_catalan(fu_value, MPFR_RNDU);std::cout << "Number.cc-4904" << endl;
		mpfr_const_catalan(fl_value, MPFR_RNDD);std::cout << "Number.cc-4905" << endl;
	}
	b_approx = true;std::cout << "Number.cc-4907" << endl;
}
void Number::euler() {
	if(n_type != NUMBER_TYPE_FLOAT) {
		mpfr_init2(fu_value, BIT_PRECISION);std::cout << "Number.cc-4911" << endl;
		mpfr_init2(fl_value, BIT_PRECISION);std::cout << "Number.cc-4912" << endl;
		mpq_set_ui(r_value, 0, 1);std::cout << "Number.cc-4913" << endl;
	} else {
		if(mpfr_get_prec(fu_value) < BIT_PRECISION) mpfr_set_prec(fu_value, BIT_PRECISION);
		if(mpfr_get_prec(fl_value) < BIT_PRECISION) mpfr_set_prec(fl_value, BIT_PRECISION);
	}
	n_type = NUMBER_TYPE_FLOAT;std::cout << "Number.cc-4918" << endl;
	if(!CREATE_INTERVAL) {
		mpfr_const_euler(fl_value, MPFR_RNDN);std::cout << "Number.cc-4920" << endl;
		mpfr_set(fu_value, fl_value, MPFR_RNDN);std::cout << "Number.cc-4921" << endl;
		i_precision = FROM_BIT_PRECISION(BIT_PRECISION);std::cout << "Number.cc-4922" << endl;
	} else {
		mpfr_const_euler(fu_value, MPFR_RNDU);std::cout << "Number.cc-4924" << endl;
		mpfr_const_euler(fl_value, MPFR_RNDD);std::cout << "Number.cc-4925" << endl;
	}
	b_approx = true;std::cout << "Number.cc-4927" << endl;
}
bool Number::zeta() {
	if(!isInteger() || !isPositive() || isOne()) {
		return false;
	}
	bool overflow = false;std::cout << "Number.cc-4933" << endl;
	long int i = lintValue(&overflow);std::cout << "Number.cc-4934" << endl;
	if(overflow) {
		CALCULATOR->error(true, _("Cannot handle an argument (s) that large for Riemann Zeta."), NULL);
		return false;
	}

	Number nr_bak(*this);std::cout << "Number.cc-4940" << endl;
	if(!setToFloatingPoint()) return false;
	mpfr_clear_flags();std::cout << "Number.cc-4942" << endl;

	if(!CREATE_INTERVAL && !isInterval()) {
		mpfr_zeta_ui(fl_value, (unsigned long int) i, MPFR_RNDN);std::cout << "Number.cc-4945" << endl;
		mpfr_set(fu_value, fl_value, MPFR_RNDN);std::cout << "Number.cc-4946" << endl;
	} else {
		mpfr_zeta_ui(fu_value, (unsigned long int) i, MPFR_RNDU);std::cout << "Number.cc-4948" << endl;
		mpfr_zeta_ui(fl_value, (unsigned long int) i, MPFR_RNDD);std::cout << "Number.cc-4949" << endl;
	}

	mpq_set_ui(r_value, 0, 1);std::cout << "Number.cc-4952" << endl;
	n_type = NUMBER_TYPE_FLOAT;std::cout << "Number.cc-4953" << endl;

	if(!testFloatResult()) {
		set(nr_bak);std::cout << "Number.cc-4956" << endl;
		return false;
	}

	return true;
}
bool Number::gamma() {
	if(isPlusInfinity()) return true;
	if(!isReal()) return false;
	if(!isNonZero()) return false;
	Number nr_bak(*this);std::cout << "Number.cc-4966" << endl;
	if(!setToFloatingPoint()) return false;
	mpfr_clear_flags();std::cout << "Number.cc-4968" << endl;
	if(!CREATE_INTERVAL && !isInterval()) {
		mpfr_gamma(fl_value, fl_value, MPFR_RNDN);std::cout << "Number.cc-4970" << endl;
		mpfr_set(fu_value, fl_value, MPFR_RNDN);std::cout << "Number.cc-4971" << endl;
	} else {
		if(mpfr_sgn(fl_value) > 0) {
			if(mpfr_cmp_d(fl_value, 1.5) < 0) {
				mpfr_t f_gamma_minx;std::cout << "Number.cc-4975" << endl;
				mpfr_init2(f_gamma_minx, BIT_PRECISION);std::cout << "Number.cc-4976" << endl;
				mpfr_set_str(f_gamma_minx, "1.46163214496836234126265954232572132846819620400644635129598840859878644035380181024307499273372559", 10, MPFR_RNDN);
				if(mpfr_cmp(fl_value, f_gamma_minx) < 0) {
					if(mpfr_cmp(fu_value, f_gamma_minx) < 0) {
						mpfr_gamma(fu_value, fu_value, MPFR_RNDD);std::cout << "Number.cc-4980" << endl;
						mpfr_gamma(fl_value, fl_value, MPFR_RNDU);std::cout << "Number.cc-4981" << endl;
						mpfr_swap(fl_value, fu_value);std::cout << "Number.cc-4982" << endl;
					} else {
						mpfr_gamma(fu_value, fu_value, MPFR_RNDU);std::cout << "Number.cc-4984" << endl;
						mpfr_gamma(fl_value, fl_value, MPFR_RNDU);std::cout << "Number.cc-4985" << endl;
						if(mpfr_cmp(fl_value, fu_value) > 0) mpfr_swap(fl_value, fu_value);
						mpfr_set_str(fl_value, "0.88560319441088870027881590058258873320795153366990344887120016587513622741739634666479828021420359", 10, MPFR_RNDD);
					}
				} else {
					mpfr_gamma(fu_value, fu_value, MPFR_RNDU);std::cout << "Number.cc-4990" << endl;
					mpfr_gamma(fl_value, fl_value, MPFR_RNDD);std::cout << "Number.cc-4991" << endl;
				}
				mpfr_clear(f_gamma_minx);std::cout << "Number.cc-4993" << endl;
			} else {
				mpfr_gamma(fu_value, fu_value, MPFR_RNDU);std::cout << "Number.cc-4995" << endl;
				mpfr_gamma(fl_value, fl_value, MPFR_RNDD);std::cout << "Number.cc-4996" << endl;
			}
		} else if(mpfr_sgn(fu_value) >= 0) {
			set(nr_bak);std::cout << "Number.cc-4999" << endl;
			return false;
		} else {
			mpfr_t fu_test, fl_test;std::cout << "Number.cc-5002" << endl;
			mpfr_init2(fu_test, BIT_PRECISION);std::cout << "Number.cc-5003" << endl;
			mpfr_init2(fl_test, BIT_PRECISION);std::cout << "Number.cc-5004" << endl;
			mpfr_floor(fu_test, fu_value);std::cout << "Number.cc-5005" << endl;
			mpfr_floor(fl_test, fl_value);std::cout << "Number.cc-5006" << endl;
			if(!mpfr_equal_p(fu_test, fl_test) || mpfr_equal_p(fl_test, fl_value)) {set(nr_bak); return false;}
			mpfr_gamma(fu_value, fu_value, MPFR_RNDN);std::cout << "Number.cc-5008" << endl;
			mpfr_gamma(fl_value, fl_value, MPFR_RNDN);std::cout << "Number.cc-5009" << endl;
			if(mpfr_cmp(fl_value, fu_value) > 0) mpfr_swap(fl_value, fu_value);
			if(nr_bak.isInterval() && nr_bak.precision(1) <= PRECISION + 20) CALCULATOR->error(false, _("%s() lacks proper support interval arithmetic."), CALCULATOR->f_gamma->name().c_str(), NULL);
			mpfr_clears(fu_test, fl_test, NULL);std::cout << "Number.cc-5012" << endl;
		}
	}
	if(!testFloatResult()) {
		set(nr_bak);std::cout << "Number.cc-5016" << endl;
		return false;
	}
	return true;
}
bool Number::digamma() {
	if(!isReal()) return false;
	if(!isNonZero()) return false;
	Number nr_bak(*this);std::cout << "Number.cc-5024" << endl;
	if(!setToFloatingPoint()) return false;
	mpfr_clear_flags();std::cout << "Number.cc-5026" << endl;
	if(!CREATE_INTERVAL && !isInterval()) {
		mpfr_digamma(fl_value, fl_value, MPFR_RNDN);std::cout << "Number.cc-5028" << endl;
		mpfr_set(fu_value, fl_value, MPFR_RNDN);std::cout << "Number.cc-5029" << endl;
	} else {
		if(mpfr_sgn(fl_value) > 0) {
			mpfr_digamma(fu_value, fu_value, MPFR_RNDU);std::cout << "Number.cc-5032" << endl;
			mpfr_digamma(fl_value, fl_value, MPFR_RNDD);std::cout << "Number.cc-5033" << endl;
		} else if(mpfr_sgn(fu_value) >= 0) {
			set(nr_bak);std::cout << "Number.cc-5035" << endl;
			return false;
		} else {
			mpfr_t fu_test, fl_test;std::cout << "Number.cc-5038" << endl;
			mpfr_init2(fu_test, BIT_PRECISION);std::cout << "Number.cc-5039" << endl;
			mpfr_init2(fl_test, BIT_PRECISION);std::cout << "Number.cc-5040" << endl;
			mpfr_floor(fu_test, fu_value);std::cout << "Number.cc-5041" << endl;
			mpfr_floor(fl_test, fl_value);std::cout << "Number.cc-5042" << endl;
			if(!mpfr_equal_p(fu_test, fl_test) || mpfr_equal_p(fl_test, fl_value)) {set(nr_bak); return false;}
			mpfr_digamma(fu_value, fu_value, MPFR_RNDU);std::cout << "Number.cc-5044" << endl;
			mpfr_digamma(fl_value, fl_value, MPFR_RNDD);std::cout << "Number.cc-5045" << endl;
			mpfr_clears(fu_test, fl_test, NULL);std::cout << "Number.cc-5046" << endl;
		}
	}
	if(!testFloatResult()) {
		set(nr_bak);std::cout << "Number.cc-5050" << endl;
		return false;
	}
	return true;
}
bool Number::erf() {
	if(hasImaginaryPart()) return false;
	if(isPlusInfinity()) {set(1, 1, 0, true); return true;}
	if(isMinusInfinity()) {set(-1, 1, 0, true); return true;}
	Number nr_bak(*this);std::cout << "Number.cc-5059" << endl;
	if(!setToFloatingPoint()) return false;
	mpfr_clear_flags();std::cout << "Number.cc-5061" << endl;
	if(!CREATE_INTERVAL && !isInterval()) {
		mpfr_erf(fl_value, fl_value, MPFR_RNDN);std::cout << "Number.cc-5063" << endl;
		mpfr_set(fu_value, fl_value, MPFR_RNDN);std::cout << "Number.cc-5064" << endl;
	} else {
		mpfr_erf(fu_value, fu_value, MPFR_RNDU);std::cout << "Number.cc-5066" << endl;
		mpfr_erf(fl_value, fl_value, MPFR_RNDD);std::cout << "Number.cc-5067" << endl;
	}
	if(!testFloatResult()) {
		set(nr_bak);std::cout << "Number.cc-5070" << endl;
		return false;
	}
	return true;
}
bool Number::erfc() {
	if(hasImaginaryPart()) return false;
	if(isPlusInfinity()) {clear(true); return true;}
	if(isMinusInfinity()) {set(2, 1, 0, true); return true;}
	Number nr_bak(*this);std::cout << "Number.cc-5079" << endl;
	if(!setToFloatingPoint()) return false;
	mpfr_clear_flags();std::cout << "Number.cc-5081" << endl;
	if(!CREATE_INTERVAL && !isInterval()) {
		mpfr_erfc(fl_value, fl_value, MPFR_RNDN);std::cout << "Number.cc-5083" << endl;
		mpfr_set(fu_value, fl_value, MPFR_RNDN);std::cout << "Number.cc-5084" << endl;
	} else {
		mpfr_erfc(fu_value, fu_value, MPFR_RNDD);std::cout << "Number.cc-5086" << endl;
		mpfr_erfc(fl_value, fl_value, MPFR_RNDU);std::cout << "Number.cc-5087" << endl;
		mpfr_swap(fu_value, fl_value);std::cout << "Number.cc-5088" << endl;
	}
	if(!testFloatResult()) {
		set(nr_bak);std::cout << "Number.cc-5091" << endl;
		return false;
	}
	return true;
}
bool Number::airy() {
	if(!isReal()) return false;
	if(!isLessThanOrEqualTo(500) || !isGreaterThanOrEqualTo(-500)) return false;
	Number nr_bak(*this);std::cout << "Number.cc-5099" << endl;
	if(!setToFloatingPoint()) return false;
	mpfr_clear_flags();std::cout << "Number.cc-5101" << endl;
	if(!CREATE_INTERVAL && !isInterval()) {
		mpfr_ai(fl_value, fl_value, MPFR_RNDN);std::cout << "Number.cc-5103" << endl;
		mpfr_set(fu_value, fl_value, MPFR_RNDN);std::cout << "Number.cc-5104" << endl;
	} else if(mpfr_cmp_si(fl_value, -1) >= 0) {
		mpfr_ai(fl_value, fl_value, MPFR_RNDU);std::cout << "Number.cc-5106" << endl;
		mpfr_ai(fu_value, fu_value, MPFR_RNDD);std::cout << "Number.cc-5107" << endl;
		mpfr_swap(fl_value, fu_value);std::cout << "Number.cc-5108" << endl;
	} else {
		mpfr_ai(fl_value, fl_value, MPFR_RNDN);std::cout << "Number.cc-5110" << endl;
		mpfr_ai(fu_value, fu_value, MPFR_RNDN);std::cout << "Number.cc-5111" << endl;
		if(mpfr_cmp(fl_value, fu_value) > 0) mpfr_swap(fl_value, fu_value);
		if(nr_bak.isInterval() && nr_bak.precision(1) <= PRECISION + 20) CALCULATOR->error(false, _("%s() lacks proper support interval arithmetic."), CALCULATOR->f_airy->name().c_str(), NULL);
	}
	if(!testFloatResult()) {
		set(nr_bak);std::cout << "Number.cc-5116" << endl;
		return false;
	}
	return true;
}
bool Number::besselj(const Number &o) {
	if(hasImaginaryPart() || !o.isInteger()) return false;
	if(isZero()) return true;
	if(isInfinite()) {
		clear(true);std::cout << "Number.cc-5125" << endl;
		return true;
	}
	if(!mpz_fits_slong_p(mpq_numref(o.internalRational()))) return false;
	long int n = mpz_get_si(mpq_numref(o.internalRational()));std::cout << "Number.cc-5129" << endl;
	Number nr_bak(*this);std::cout << "Number.cc-5130" << endl;
	if(!setToFloatingPoint()) return false;
	if(mpfr_get_exp(fl_value) > 2000000L) {
		set(nr_bak);std::cout << "Number.cc-5133" << endl;
		return false;
	}
	mpfr_clear_flags();std::cout << "Number.cc-5136" << endl;
	if(!CREATE_INTERVAL && !isInterval()) {
		mpfr_jn(fl_value, n, fl_value, MPFR_RNDN);std::cout << "Number.cc-5138" << endl;
		mpfr_set(fu_value, fl_value, MPFR_RNDN);std::cout << "Number.cc-5139" << endl;
	} else {
		mpfr_jn(fl_value, n, fl_value, MPFR_RNDN);std::cout << "Number.cc-5141" << endl;
		mpfr_jn(fu_value, n, fu_value, MPFR_RNDN);std::cout << "Number.cc-5142" << endl;
		if(mpfr_cmp(fl_value, fu_value) > 0) mpfr_swap(fl_value, fu_value);
		if(nr_bak.isInterval() && nr_bak.precision(1) <= PRECISION + 20) CALCULATOR->error(false, _("%s() lacks proper support interval arithmetic."), CALCULATOR->f_besselj->name().c_str(), NULL);
	}
	if(!testFloatResult()) {
		set(nr_bak);std::cout << "Number.cc-5147" << endl;
		return false;
	}
	return true;
}
bool Number::bessely(const Number &o) {
	if(hasImaginaryPart() || !isNonNegative() || !o.isInteger() || isZero()) return false;
	if(isPlusInfinity()) {
		clear(true);std::cout << "Number.cc-5155" << endl;
		return true;
	}
	if(isMinusInfinity()) return false;
	if(!mpz_fits_slong_p(mpq_numref(o.internalRational()))) return false;
	long int n = mpz_get_si(mpq_numref(o.internalRational()));std::cout << "Number.cc-5160" << endl;
	Number nr_bak(*this);std::cout << "Number.cc-5161" << endl;
	if(!setToFloatingPoint()) return false;
	if(mpfr_get_exp(fl_value) > 2000000L) {
		set(nr_bak);std::cout << "Number.cc-5164" << endl;
		return false;
	}
	mpfr_clear_flags();std::cout << "Number.cc-5167" << endl;
	if(!CREATE_INTERVAL && !isInterval()) {
		mpfr_yn(fl_value, n, fl_value, MPFR_RNDN);std::cout << "Number.cc-5169" << endl;
		mpfr_set(fu_value, fl_value, MPFR_RNDN);std::cout << "Number.cc-5170" << endl;
	} else {
		mpfr_yn(fl_value, n, fl_value, MPFR_RNDN);std::cout << "Number.cc-5172" << endl;
		mpfr_yn(fu_value, n, fu_value, MPFR_RNDN);std::cout << "Number.cc-5173" << endl;
		if(mpfr_cmp(fl_value, fu_value) > 0) mpfr_swap(fl_value, fu_value);
		if(nr_bak.isInterval() && nr_bak.precision(1) <= PRECISION + 20) CALCULATOR->error(false, _("%s() lacks proper support interval arithmetic."), CALCULATOR->f_bessely->name().c_str(), NULL);
	}
	if(!testFloatResult()) {
		set(nr_bak);std::cout << "Number.cc-5178" << endl;
		return false;
	}
	return true;
}

bool Number::sin() {
	if(includesInfinity()) {
		if(!hasImaginaryPart() && isInterval() && (mpfr_sgn(fl_value) != mpfr_sgn(fu_value) || !mpfr_inf_p(fl_value) || !mpfr_inf_p(fu_value))) {
			setInterval(nr_minus_one, nr_one, true);std::cout << "Number.cc-5187" << endl;
			return true;
		}
		return false;
	}
	if(isZero()) return true;
	if(hasImaginaryPart()) {
		if(hasRealPart()) {
			Number t1a, t1b, t2a, t2b;std::cout << "Number.cc-5195" << endl;
			t1a.set(*this, false, true);std::cout << "Number.cc-5196" << endl;
			t1b.set(*i_value, false, true);std::cout << "Number.cc-5197" << endl;
			t2a.set(t1a);std::cout << "Number.cc-5198" << endl;
			t2b.set(t1b);std::cout << "Number.cc-5199" << endl;
			if(!t1a.sin() || !t1b.cosh() || !t2a.cos() || !t2b.sinh() || !t1a.multiply(t1b) || !t2a.multiply(t2b)) return false;
			if(!t1a.isReal() || !t2a.isReal()) return false;
			set(t1a, true, true);std::cout << "Number.cc-5202" << endl;
			i_value->set(t2a, true, true);std::cout << "Number.cc-5203" << endl;
			setPrecisionAndApproximateFrom(*i_value);std::cout << "Number.cc-5204" << endl;
			return true;
		} else {
			if(!i_value->sinh()) return false;
			setPrecisionAndApproximateFrom(*i_value);std::cout << "Number.cc-5208" << endl;
			return true;
		}
	}
	Number nr_bak(*this);std::cout << "Number.cc-5212" << endl;
	bool do_pi = true;std::cout << "Number.cc-5213" << endl;
	if(n_type == NUMBER_TYPE_RATIONAL) {
		if(mpz_cmp_ui(mpq_denref(r_value), 1000000L) < 0) do_pi = false;
		if(!setToFloatingPoint()) return false;
	}
	if(mpfr_get_exp(fl_value) > BIT_PRECISION || mpfr_get_exp(fu_value) > BIT_PRECISION) {
		set(nr_bak);std::cout << "Number.cc-5219" << endl;
		return false;
	}
	mpfr_clear_flags();std::cout << "Number.cc-5222" << endl;
	if(!CREATE_INTERVAL && !isInterval()) {
		if(do_pi) {
			mpfr_t f_pi, f_quo;std::cout << "Number.cc-5225" << endl;
			mpz_t f_int;std::cout << "Number.cc-5226" << endl;
			mpz_init(f_int);std::cout << "Number.cc-5227" << endl;
			mpfr_init2(f_pi, BIT_PRECISION);std::cout << "Number.cc-5228" << endl;
			mpfr_init2(f_quo, BIT_PRECISION - 30);std::cout << "Number.cc-5229" << endl;
			mpfr_const_pi(f_pi, MPFR_RNDN);std::cout << "Number.cc-5230" << endl;
			mpfr_div(f_quo, fl_value, f_pi, MPFR_RNDN);std::cout << "Number.cc-5231" << endl;
			mpfr_get_z(f_int, f_quo, MPFR_RNDD);std::cout << "Number.cc-5232" << endl;
			mpfr_frac(f_quo, f_quo, MPFR_RNDN);std::cout << "Number.cc-5233" << endl;
			if(mpfr_zero_p(f_quo)) {
				clear(true);std::cout << "Number.cc-5235" << endl;
				b_approx = true;std::cout << "Number.cc-5236" << endl;
				if(i_precision < 0 || FROM_BIT_PRECISION(BIT_PRECISION - 30) < i_precision) i_precision = FROM_BIT_PRECISION(BIT_PRECISION - 30);
				mpfr_clears(f_pi, f_quo, NULL);std::cout << "Number.cc-5238" << endl;
				mpz_clear(f_int);std::cout << "Number.cc-5239" << endl;
				return true;
			}
			mpfr_abs(f_quo, f_quo, MPFR_RNDN);std::cout << "Number.cc-5242" << endl;
			mpfr_mul_ui(f_quo, f_quo, 2, MPFR_RNDN);std::cout << "Number.cc-5243" << endl;
			mpfr_sub_ui(f_quo, f_quo, 1, MPFR_RNDN);std::cout << "Number.cc-5244" << endl;
			if(mpfr_zero_p(f_quo)) {
				if(mpz_odd_p(f_int)) set(-1, 1, 0, true);
				else set(1, 1, 0, true);std::cout << "Number.cc-5247" << endl;
				b_approx = true;std::cout << "Number.cc-5248" << endl;
				if(i_precision < 0 || FROM_BIT_PRECISION(BIT_PRECISION - 30) < i_precision) i_precision = FROM_BIT_PRECISION(BIT_PRECISION - 30);
				mpfr_clears(f_pi, f_quo, NULL);std::cout << "Number.cc-5250" << endl;
				mpz_clear(f_int);std::cout << "Number.cc-5251" << endl;
				return true;
			}
			mpfr_clears(f_pi, f_quo, NULL);std::cout << "Number.cc-5254" << endl;
			mpz_clear(f_int);std::cout << "Number.cc-5255" << endl;
		}
		mpfr_sin(fl_value, fl_value, MPFR_RNDN);std::cout << "Number.cc-5257" << endl;
		mpfr_set(fu_value, fl_value, MPFR_RNDN);std::cout << "Number.cc-5258" << endl;
	} else {
		mpfr_t fl_pi, fu_pi, fu_quo, fl_quo;std::cout << "Number.cc-5260" << endl;
		mpfr_inits2(BIT_PRECISION, fl_pi, fu_pi, fl_quo, fu_quo, NULL);std::cout << "Number.cc-5261" << endl;
		mpz_t fl_int, fu_int, f_diff;
		mpz_inits(fl_int, fu_int, f_diff, NULL);
		mpfr_const_pi(fl_pi, MPFR_RNDD);std::cout << "Number.cc-5264" << endl;
		mpfr_const_pi(fu_pi, MPFR_RNDU);std::cout << "Number.cc-5265" << endl;
		mpfr_div(fl_quo, fl_value, fu_pi, MPFR_RNDD);std::cout << "Number.cc-5266" << endl;
		mpfr_div(fu_quo, fu_value, fl_pi, MPFR_RNDU);std::cout << "Number.cc-5267" << endl;
		mpfr_sub_q(fl_quo, fl_quo, nr_half.internalRational(), MPFR_RNDD);std::cout << "Number.cc-5268" << endl;
		mpfr_sub_q(fu_quo, fu_quo, nr_half.internalRational(), MPFR_RNDU);std::cout << "Number.cc-5269" << endl;
		mpfr_get_z(fl_int, fl_quo, MPFR_RNDD);std::cout << "Number.cc-5270" << endl;
		mpfr_get_z(fu_int, fu_quo, MPFR_RNDD);std::cout << "Number.cc-5271" << endl;
		mpfr_sub_z(fl_quo, fl_quo, fl_int, MPFR_RNDD);std::cout << "Number.cc-5272" << endl;
		mpfr_sub_z(fu_quo, fu_quo, fu_int, MPFR_RNDU);std::cout << "Number.cc-5273" << endl;
		if(mpz_cmp(fl_int, fu_int) != 0) {
			mpz_sub(f_diff, fu_int, fl_int);
			if(mpz_cmp_ui(f_diff, 2) >= 0) {
				mpfr_set_si(fl_value, -1, MPFR_RNDD);std::cout << "Number.cc-5277" << endl;
				mpfr_set_si(fu_value, 1, MPFR_RNDU);std::cout << "Number.cc-5278" << endl;
			} else {
				if(mpz_even_p(fl_int)) {
					mpfr_sin(fl_value, fl_value, MPFR_RNDU);std::cout << "Number.cc-5281" << endl;
					mpfr_sin(fu_value, fu_value, MPFR_RNDU);std::cout << "Number.cc-5282" << endl;
					if(mpfr_cmp(fl_value, fu_value) > 0) mpfr_swap(fl_value, fu_value);
					mpfr_set_si(fl_value, -1, MPFR_RNDD);std::cout << "Number.cc-5284" << endl;
				} else {
					mpfr_sin(fl_value, fl_value, MPFR_RNDD);std::cout << "Number.cc-5286" << endl;
					mpfr_sin(fu_value, fu_value, MPFR_RNDD);std::cout << "Number.cc-5287" << endl;
					if(mpfr_cmp(fu_value, fl_value) < 0) mpfr_swap(fl_value, fu_value);
					mpfr_set_si(fu_value, 1, MPFR_RNDU);std::cout << "Number.cc-5289" << endl;
				}
			}
		} else {
			if(mpz_even_p(fl_int)) {
				mpfr_sin(fl_value, fl_value, MPFR_RNDU);std::cout << "Number.cc-5294" << endl;
				mpfr_sin(fu_value, fu_value, MPFR_RNDD);std::cout << "Number.cc-5295" << endl;
				mpfr_swap(fl_value, fu_value);std::cout << "Number.cc-5296" << endl;
			} else {
				mpfr_sin(fl_value, fl_value, MPFR_RNDD);std::cout << "Number.cc-5298" << endl;
				mpfr_sin(fu_value, fu_value, MPFR_RNDU);std::cout << "Number.cc-5299" << endl;
			}
		}
		mpfr_clears(fl_pi, fu_pi, fl_quo, fu_quo, NULL);std::cout << "Number.cc-5302" << endl;
		mpz_clears(fl_int, fu_int, f_diff, NULL);
	}

	if(!testFloatResult()) {
		set(nr_bak);std::cout << "Number.cc-5307" << endl;
		return false;
	}
	return true;
}
bool Number::asin() {
	if(includesInfinity()) return false;
	if(isZero()) return true;
	if(isOne()) {
		pi();std::cout << "Number.cc-5316" << endl;
		divide(2);std::cout << "Number.cc-5317" << endl;
		return true;
	}
	if(isMinusOne()) {
		pi();std::cout << "Number.cc-5321" << endl;
		divide(-2);std::cout << "Number.cc-5322" << endl;
		return true;
	}
	if(hasImaginaryPart() || !isFraction()) {
		if(b_imag) return false;
		if(hasImaginaryPart() && !hasRealPart()) {
			Number nri(*i_value);std::cout << "Number.cc-5328" << endl;
			if(!nri.asinh() || !nri.multiply(nr_one_i)) return false;
			set(nri, true);std::cout << "Number.cc-5330" << endl;
			return true;
		}
		if(isInterval(false)) {
			Number nr1(lowerEndPoint(true));std::cout << "Number.cc-5334" << endl;
			Number nr2(upperEndPoint(true));std::cout << "Number.cc-5335" << endl;
			if(!nr1.asin() || !nr2.asin()) return false;
			if(!hasImaginaryPart()) {
				if(!setInterval(nr1, nr2, true)) return false;
				return true;
			}
			Number nr;std::cout << "Number.cc-5341" << endl;
			if(!nr.setInterval(nr1, nr2, true)) return false;
			if(isInterval(true) && imaginaryPartIsInterval()) {
				Number nr3(lowerEndPoint(false));std::cout << "Number.cc-5344" << endl;
				Number nr4(upperEndPoint(false));std::cout << "Number.cc-5345" << endl;
				nr3.setImaginaryPart(i_value->upperEndPoint());std::cout << "Number.cc-5346" << endl;
				nr4.setImaginaryPart(i_value->lowerEndPoint());std::cout << "Number.cc-5347" << endl;
				if(!nr3.asin() || !nr4.asin()) return false;
				if(!nr.setInterval(nr, nr3, true)) return false;
				if(!nr.setInterval(nr, nr4, true)) return false;
			}
			if(hasRealPart() && !realPartIsNonZero()) {
				nr1 = lowerEndPoint(true);std::cout << "Number.cc-5353" << endl;
				nr2 = upperEndPoint(true);std::cout << "Number.cc-5354" << endl;
				nr1.clearReal();std::cout << "Number.cc-5355" << endl;
				nr2.clearReal();std::cout << "Number.cc-5356" << endl;
				if(!nr1.asin() || !nr2.asin()) return false;
				if(!nr.setInterval(nr, nr1, true)) return false;
				if(!nr.setInterval(nr, nr2, true)) return false;
			}
			if(hasImaginaryPart() && !imaginaryPartIsNonZero()) {
				nr1 = lowerEndPoint(false);std::cout << "Number.cc-5362" << endl;
				nr2 = upperEndPoint(false);std::cout << "Number.cc-5363" << endl;
				if(!nr1.asin() || !nr2.asin()) return false;
				if(!nr.setInterval(nr, nr1, true)) return false;
				if(!nr.setInterval(nr, nr2, true)) return false;
			}
			set(nr, true);std::cout << "Number.cc-5368" << endl;
			return true;
		}
		Number z_sqln(*this);std::cout << "Number.cc-5371" << endl;
		Number i_z(*this);std::cout << "Number.cc-5372" << endl;
		bool b_neg = false;std::cout << "Number.cc-5373" << endl;
		if(hasImaginaryPart()) {
			b_neg = (realPartIsNegative() && !imaginaryPartIsNegative()) || (realPartIsPositive() && imaginaryPartIsPositive());std::cout << "Number.cc-5375" << endl;
		} else {
			b_neg = isNegative();std::cout << "Number.cc-5377" << endl;
		}
		if(b_neg && (!z_sqln.negate() || !i_z.negate())) return false;
		if(!i_z.multiply(nr_one_i)) return false;
		if(!z_sqln.square() || !z_sqln.negate() || !z_sqln.add(1) || !z_sqln.raise(nr_half) || !z_sqln.add(i_z) || !z_sqln.ln() || !z_sqln.multiply(nr_minus_i)) return false;
		if(b_neg && !z_sqln.negate()) return false;
		if(hasImaginaryPart() && z_sqln.isInterval(false) && z_sqln.precision(1) <= PRECISION + 20) CALCULATOR->error(false, MESSAGE_CATEGORY_WIDE_INTERVAL, _("Interval calculated wide."), NULL);
		set(z_sqln);std::cout << "Number.cc-5384" << endl;
		return true;
	}
	Number nr_bak(*this);std::cout << "Number.cc-5387" << endl;
	if(!setToFloatingPoint()) return false;
	mpfr_clear_flags();std::cout << "Number.cc-5389" << endl;
	if(!CREATE_INTERVAL && !isInterval()) {
		mpfr_asin(fl_value, fl_value, MPFR_RNDN);std::cout << "Number.cc-5391" << endl;
		mpfr_set(fu_value, fl_value, MPFR_RNDN);std::cout << "Number.cc-5392" << endl;
	} else {
		mpfr_asin(fl_value, fl_value, MPFR_RNDD);std::cout << "Number.cc-5394" << endl;
		mpfr_asin(fu_value, fu_value, MPFR_RNDU);std::cout << "Number.cc-5395" << endl;
	}
	if(!testFloatResult()) {
		set(nr_bak);std::cout << "Number.cc-5398" << endl;
		return false;
	}
	return true;
}
bool Number::sinh() {
	if(isInfinite()) return true;
	if(isZero()) return true;
	if(hasImaginaryPart()) {
		if(hasRealPart()) {
			Number t1a, t1b, t2a, t2b;std::cout << "Number.cc-5408" << endl;
			t1a.set(*this, false, true);std::cout << "Number.cc-5409" << endl;
			t1b.set(*i_value, false, true);std::cout << "Number.cc-5410" << endl;
			t2a.set(t1a);std::cout << "Number.cc-5411" << endl;
			t2b.set(t1b);std::cout << "Number.cc-5412" << endl;
			if(!t1a.sinh() || !t1b.cos() || !t2a.cosh() || !t2b.sin() || !t1a.multiply(t1b) || !t2a.multiply(t2b)) return false;
			if(!t1a.isReal() || !t2a.isReal()) return false;
			set(t1a, true, true);std::cout << "Number.cc-5415" << endl;
			i_value->set(t2a, true, true);std::cout << "Number.cc-5416" << endl;
			setPrecisionAndApproximateFrom(*i_value);std::cout << "Number.cc-5417" << endl;
			return true;
		} else {
			if(!i_value->sin()) return false;
			setPrecisionAndApproximateFrom(*i_value);std::cout << "Number.cc-5421" << endl;
			return true;
		}
	}
	Number nr_bak(*this);std::cout << "Number.cc-5425" << endl;
	if(!setToFloatingPoint()) return false;
	if(mpfr_get_exp(fl_value) > 28 || mpfr_get_exp(fu_value) > 28) {
		set(nr_bak);std::cout << "Number.cc-5428" << endl;
		return false;
	}
	mpfr_clear_flags();std::cout << "Number.cc-5431" << endl;
	if(!CREATE_INTERVAL && !isInterval()) {
		mpfr_sinh(fl_value, fl_value, MPFR_RNDN);std::cout << "Number.cc-5433" << endl;
		mpfr_set(fu_value, fl_value, MPFR_RNDN);std::cout << "Number.cc-5434" << endl;
	} else {
		mpfr_sinh(fl_value, fl_value, MPFR_RNDD);std::cout << "Number.cc-5436" << endl;
		mpfr_sinh(fu_value, fu_value, MPFR_RNDU);std::cout << "Number.cc-5437" << endl;
	}
	if(!testFloatResult()) {
		set(nr_bak);std::cout << "Number.cc-5440" << endl;
		return false;
	}
	return true;
}
bool Number::asinh() {
	if(isInfinite()) return true;
	if(isZero()) return true;
	if(hasImaginaryPart()) {
		 if(!hasRealPart()) {
			Number inr(*i_value);std::cout << "Number.cc-5450" << endl;
			if(!inr.asin() || !inr.multiply(nr_one_i)) return false;
			set(inr, true);std::cout << "Number.cc-5452" << endl;
			return true;
		}
		if(isInterval(false)) {
			Number nr1(lowerEndPoint(true));std::cout << "Number.cc-5456" << endl;
			Number nr2(upperEndPoint(true));std::cout << "Number.cc-5457" << endl;
			if(!nr1.asinh() || !nr2.asinh()) return false;
			Number nr;std::cout << "Number.cc-5459" << endl;
			if(!nr.setInterval(nr1, nr2, true)) return false;
			if(isInterval(true) && imaginaryPartIsInterval()) {
				Number nr3(lowerEndPoint(false));std::cout << "Number.cc-5462" << endl;
				Number nr4(upperEndPoint(false));std::cout << "Number.cc-5463" << endl;
				nr3.setImaginaryPart(i_value->upperEndPoint());std::cout << "Number.cc-5464" << endl;
				nr4.setImaginaryPart(i_value->lowerEndPoint());std::cout << "Number.cc-5465" << endl;
				if(!nr3.asinh() || !nr4.asinh()) return false;
				if(!nr.setInterval(nr, nr3, true)) return false;
				if(!nr.setInterval(nr, nr4, true)) return false;
			}
			if(hasRealPart() && !realPartIsNonZero()) {
				nr1 = lowerEndPoint(true);std::cout << "Number.cc-5471" << endl;
				nr2 = upperEndPoint(true);std::cout << "Number.cc-5472" << endl;
				nr1.clearReal();std::cout << "Number.cc-5473" << endl;
				nr2.clearReal();std::cout << "Number.cc-5474" << endl;
				if(!nr1.asinh() || !nr2.asinh()) return false;
				if(!nr.setInterval(nr, nr1, true)) return false;
				if(!nr.setInterval(nr, nr2, true)) return false;
			}
			if(hasImaginaryPart() && !imaginaryPartIsNonZero()) {
				nr1 = lowerEndPoint(false);std::cout << "Number.cc-5480" << endl;
				nr2 = upperEndPoint(false);std::cout << "Number.cc-5481" << endl;
				if(!nr1.asinh() || !nr2.asinh()) return false;
				if(!nr.setInterval(nr, nr1, true)) return false;
				if(!nr.setInterval(nr, nr2, true)) return false;
			}
			set(nr, true);std::cout << "Number.cc-5486" << endl;
			return true;
		}
		Number z_sqln(*this);std::cout << "Number.cc-5489" << endl;
		if(!z_sqln.square() || !z_sqln.add(1) || !z_sqln.raise(nr_half) || !z_sqln.add(*this)) return false;
		//If zero, it means that the precision is too low (since infinity is not the correct value). Happens with number less than -(10^1000)i
		if(z_sqln.isZero()) return false;
		if(!z_sqln.ln()) return false;
		if(hasImaginaryPart() && z_sqln.isInterval(false) && z_sqln.precision(1) <= PRECISION + 20) CALCULATOR->error(false, MESSAGE_CATEGORY_WIDE_INTERVAL, _("Interval calculated wide."), NULL);
		set(z_sqln);std::cout << "Number.cc-5495" << endl;
		return true;
	}
	Number nr_bak(*this);std::cout << "Number.cc-5498" << endl;
	if(!setToFloatingPoint()) return false;
	mpfr_clear_flags();std::cout << "Number.cc-5500" << endl;
	if(!CREATE_INTERVAL && !isInterval()) {
		mpfr_asinh(fl_value, fl_value, MPFR_RNDN);std::cout << "Number.cc-5502" << endl;
		mpfr_set(fu_value, fl_value, MPFR_RNDN);std::cout << "Number.cc-5503" << endl;
	} else {
		mpfr_asinh(fl_value, fl_value, MPFR_RNDD);std::cout << "Number.cc-5505" << endl;
		mpfr_asinh(fu_value, fu_value, MPFR_RNDU);std::cout << "Number.cc-5506" << endl;
	}
	if(!testFloatResult()) {
		set(nr_bak);std::cout << "Number.cc-5509" << endl;
		return false;
	}
	return true;
}
bool Number::cos() {
	if(includesInfinity()) {
		if(!hasImaginaryPart() && isInterval() && (mpfr_sgn(fl_value) != mpfr_sgn(fu_value) || !mpfr_inf_p(fl_value) || !mpfr_inf_p(fu_value))) {
			setInterval(nr_minus_one, nr_one, true);std::cout << "Number.cc-5517" << endl;
			return true;
		}
		return false;
	}
	if(isZero()) {
		set(1, 1, 0, true);std::cout << "Number.cc-5523" << endl;
		return true;
	}
	if(hasImaginaryPart()) {
		if(hasRealPart()) {
			Number t1a, t1b, t2a, t2b;std::cout << "Number.cc-5528" << endl;
			t1a.set(*this, false, true);std::cout << "Number.cc-5529" << endl;
			t1b.set(*i_value, false, true);std::cout << "Number.cc-5530" << endl;
			t2a.set(t1a);std::cout << "Number.cc-5531" << endl;
			t2b.set(t1b);std::cout << "Number.cc-5532" << endl;
			if(!t1a.cos() || !t1b.cosh() || !t2a.sin() || !t2b.sinh() || !t1a.multiply(t1b) || !t2a.multiply(t2b) || !t2a.negate()) return false;
			if(!t1a.isReal() || !t2a.isReal()) return false;
			set(t1a, true, true);std::cout << "Number.cc-5535" << endl;
			i_value->set(t2a, true, true);std::cout << "Number.cc-5536" << endl;
			setPrecisionAndApproximateFrom(*i_value);std::cout << "Number.cc-5537" << endl;
			testComplex(this, i_value);std::cout << "Number.cc-5538" << endl;
			return true;
		} else {
			if(!i_value->cosh()) return false;
			set(*i_value, true, true);std::cout << "Number.cc-5542" << endl;
			i_value->clear();std::cout << "Number.cc-5543" << endl;
			return true;
		}
	}
	Number nr_bak(*this);std::cout << "Number.cc-5547" << endl;
	bool do_pi = true;std::cout << "Number.cc-5548" << endl;
	if(n_type == NUMBER_TYPE_RATIONAL) {
		if(mpz_cmp_ui(mpq_denref(r_value), 1000000L) < 0) do_pi = false;
		if(!setToFloatingPoint()) return false;
	}
	if(mpfr_get_exp(fl_value) > BIT_PRECISION || mpfr_get_exp(fu_value) > BIT_PRECISION) {
		set(nr_bak);std::cout << "Number.cc-5554" << endl;
		return false;
	}
	mpfr_clear_flags();std::cout << "Number.cc-5557" << endl;
	if(!CREATE_INTERVAL && !isInterval()) {
		if(do_pi) {
			mpfr_t f_pi, f_quo;std::cout << "Number.cc-5560" << endl;
			mpz_t f_int;std::cout << "Number.cc-5561" << endl;
			mpz_init(f_int);std::cout << "Number.cc-5562" << endl;
			mpfr_init2(f_pi, BIT_PRECISION);std::cout << "Number.cc-5563" << endl;
			mpfr_init2(f_quo, BIT_PRECISION - 30);std::cout << "Number.cc-5564" << endl;
			mpfr_const_pi(f_pi, MPFR_RNDN);std::cout << "Number.cc-5565" << endl;
			mpfr_div(f_quo, fl_value, f_pi, MPFR_RNDN);std::cout << "Number.cc-5566" << endl;
			// ?: was MPFR_RNDF
			mpfr_get_z(f_int, f_quo, MPFR_RNDD);std::cout << "Number.cc-5568" << endl;
			mpfr_frac(f_quo, f_quo, MPFR_RNDN);std::cout << "Number.cc-5569" << endl;
			if(mpfr_zero_p(f_quo)) {
				if(mpz_odd_p(f_int)) set(-1, 1, 0, true);
				else set(1, 1, 0, true);std::cout << "Number.cc-5572" << endl;
				b_approx = true;std::cout << "Number.cc-5573" << endl;
				if(i_precision < 0 || FROM_BIT_PRECISION(BIT_PRECISION - 30) < i_precision) i_precision = FROM_BIT_PRECISION(BIT_PRECISION - 30);
				mpfr_clears(f_pi, f_quo, NULL);std::cout << "Number.cc-5575" << endl;
				mpz_clear(f_int);std::cout << "Number.cc-5576" << endl;
				return true;
			}
			mpfr_abs(f_quo, f_quo, MPFR_RNDN);std::cout << "Number.cc-5579" << endl;
			mpfr_mul_ui(f_quo, f_quo, 2, MPFR_RNDN);std::cout << "Number.cc-5580" << endl;
			mpfr_sub_ui(f_quo, f_quo, 1, MPFR_RNDN);std::cout << "Number.cc-5581" << endl;
			if(mpfr_zero_p(f_quo)) {
				clear(true);std::cout << "Number.cc-5583" << endl;
				b_approx = true;std::cout << "Number.cc-5584" << endl;
				if(i_precision < 0 || FROM_BIT_PRECISION(BIT_PRECISION - 30) < i_precision) i_precision = FROM_BIT_PRECISION(BIT_PRECISION - 30);
				mpfr_clears(f_pi, f_quo, NULL);std::cout << "Number.cc-5586" << endl;
				mpz_clear(f_int);std::cout << "Number.cc-5587" << endl;
				return true;
			}
			mpfr_clears(f_pi, f_quo, NULL);std::cout << "Number.cc-5590" << endl;
			mpz_clear(f_int);std::cout << "Number.cc-5591" << endl;
		}
		mpfr_cos(fl_value, fl_value, MPFR_RNDN);std::cout << "Number.cc-5593" << endl;
		mpfr_set(fu_value, fl_value, MPFR_RNDN);std::cout << "Number.cc-5594" << endl;
	} else {
		mpfr_t fl_pi, fu_pi, fu_quo, fl_quo;std::cout << "Number.cc-5596" << endl;
		mpfr_inits2(BIT_PRECISION, fl_pi, fu_pi, fl_quo, fu_quo, NULL);std::cout << "Number.cc-5597" << endl;
		mpz_t fl_int, fu_int, f_diff;
		mpz_inits(fl_int, fu_int, f_diff, NULL);
		mpfr_const_pi(fl_pi, MPFR_RNDD);std::cout << "Number.cc-5600" << endl;
		mpfr_const_pi(fu_pi, MPFR_RNDU);std::cout << "Number.cc-5601" << endl;
		mpfr_div(fl_quo, fl_value, fu_pi, MPFR_RNDD);std::cout << "Number.cc-5602" << endl;
		mpfr_div(fu_quo, fu_value, fl_pi, MPFR_RNDU);std::cout << "Number.cc-5603" << endl;
		mpfr_get_z(fl_int, fl_quo, MPFR_RNDD);std::cout << "Number.cc-5604" << endl;
		mpfr_get_z(fu_int, fu_quo, MPFR_RNDD);std::cout << "Number.cc-5605" << endl;
		mpfr_sub_z(fl_quo, fl_quo, fl_int, MPFR_RNDD);std::cout << "Number.cc-5606" << endl;
		mpfr_sub_z(fu_quo, fu_quo, fu_int, MPFR_RNDU);std::cout << "Number.cc-5607" << endl;
		if(mpz_cmp(fl_int, fu_int) != 0) {
			mpz_sub(f_diff, fu_int, fl_int);
			if(mpz_cmp_ui(f_diff, 2) >= 0) {
				mpfr_set_si(fl_value, -1, MPFR_RNDD);std::cout << "Number.cc-5611" << endl;
				mpfr_set_si(fu_value, 1, MPFR_RNDU);std::cout << "Number.cc-5612" << endl;
			} else {
				if(mpz_even_p(fl_int)) {
					mpfr_cos(fl_value, fl_value, MPFR_RNDU);std::cout << "Number.cc-5615" << endl;
					mpfr_cos(fu_value, fu_value, MPFR_RNDU);std::cout << "Number.cc-5616" << endl;
					if(mpfr_cmp(fl_value, fu_value) > 0) mpfr_swap(fl_value, fu_value);
					mpfr_set_si(fl_value, -1, MPFR_RNDD);std::cout << "Number.cc-5618" << endl;
				} else {
					mpfr_cos(fl_value, fl_value, MPFR_RNDD);std::cout << "Number.cc-5620" << endl;
					mpfr_cos(fu_value, fu_value, MPFR_RNDD);std::cout << "Number.cc-5621" << endl;
					if(mpfr_cmp(fu_value, fl_value) < 0) mpfr_swap(fl_value, fu_value);
					mpfr_set_si(fu_value, 1, MPFR_RNDU);std::cout << "Number.cc-5623" << endl;
				}
			}
		} else {
			if(mpz_even_p(fl_int)) {
				mpfr_cos(fl_value, fl_value, MPFR_RNDU);std::cout << "Number.cc-5628" << endl;
				mpfr_cos(fu_value, fu_value, MPFR_RNDD);std::cout << "Number.cc-5629" << endl;
				mpfr_swap(fl_value, fu_value);std::cout << "Number.cc-5630" << endl;
			} else {
				mpfr_cos(fl_value, fl_value, MPFR_RNDD);std::cout << "Number.cc-5632" << endl;
				mpfr_cos(fu_value, fu_value, MPFR_RNDU);std::cout << "Number.cc-5633" << endl;
			}
		}
		mpfr_clears(fl_pi, fu_pi, fl_quo, fu_quo, NULL);std::cout << "Number.cc-5636" << endl;
		mpz_clears(fl_int, fu_int, f_diff, NULL);
	}

	if(!testFloatResult()) {
		set(nr_bak);std::cout << "Number.cc-5641" << endl;
		return false;
	}
	return true;
}
bool Number::acos() {
	if(includesInfinity()) return false;
	if(isOne()) {
		clear(true);std::cout << "Number.cc-5649" << endl;
		return true;
	}
	if(isZero()) {
		pi();std::cout << "Number.cc-5653" << endl;
		divide(2);std::cout << "Number.cc-5654" << endl;
		return true;
	}
	if(isMinusOne()) {
		pi();std::cout << "Number.cc-5658" << endl;
		return true;
	}
	if(hasImaginaryPart() || !isFraction()) {
		if(b_imag) return false;
		//acos(x)=(pi-2*asin(x))/2
		Number nr(*this);std::cout << "Number.cc-5664" << endl;
		Number nr_pi;std::cout << "Number.cc-5665" << endl;
		nr_pi.pi();std::cout << "Number.cc-5666" << endl;
		if(!nr.asin() || !nr.multiply(2) || !nr.negate() || !nr.add(nr_pi) || !nr.multiply(nr_half)) return false;
		set(nr);std::cout << "Number.cc-5668" << endl;
		return true;
	}
	Number nr_bak(*this);std::cout << "Number.cc-5671" << endl;
	if(!setToFloatingPoint()) return false;
	mpfr_clear_flags();std::cout << "Number.cc-5673" << endl;

	if(!CREATE_INTERVAL && !isInterval()) {
		mpfr_acos(fl_value, fl_value, MPFR_RNDN);std::cout << "Number.cc-5676" << endl;
		mpfr_set(fu_value, fl_value, MPFR_RNDN);std::cout << "Number.cc-5677" << endl;
	} else {
		mpfr_acos(fl_value, fl_value, MPFR_RNDU);std::cout << "Number.cc-5679" << endl;
		mpfr_acos(fu_value, fu_value, MPFR_RNDD);std::cout << "Number.cc-5680" << endl;
		mpfr_swap(fl_value, fu_value);std::cout << "Number.cc-5681" << endl;
	}
	if(!testFloatResult()) {
		set(nr_bak);std::cout << "Number.cc-5684" << endl;
		return false;
	}
	return true;
}
bool Number::cosh() {
	if(isInfinite()) {
		setPlusInfinity();std::cout << "Number.cc-5691" << endl;
		return true;
	}
	if(isZero()) {
		set(1, 1, 0, true);std::cout << "Number.cc-5695" << endl;
		return true;
	}
	if(hasImaginaryPart()) {
		if(hasRealPart()) {
			Number t1a, t1b, t2a, t2b;std::cout << "Number.cc-5700" << endl;
			t1a.set(*this, false, true);std::cout << "Number.cc-5701" << endl;
			t1b.set(*i_value, false, true);std::cout << "Number.cc-5702" << endl;
			t2a.set(t1a);std::cout << "Number.cc-5703" << endl;
			t2b.set(t1b);std::cout << "Number.cc-5704" << endl;
			if(!t1a.cosh() || !t1b.cos() || !t2a.sinh() || !t2b.sin() || !t1a.multiply(t1b) || !t2a.multiply(t2b)) return false;
			if(!t1a.isReal() || !t2a.isReal()) return false;
			set(t1a, true, true);std::cout << "Number.cc-5707" << endl;
			i_value->set(t2a, true, true);std::cout << "Number.cc-5708" << endl;
			setPrecisionAndApproximateFrom(*i_value);std::cout << "Number.cc-5709" << endl;
			testComplex(this, i_value);std::cout << "Number.cc-5710" << endl;
			return true;
		} else {
			if(!i_value->cos()) return false;
			set(*i_value, true, true);std::cout << "Number.cc-5714" << endl;
			i_value->clear();std::cout << "Number.cc-5715" << endl;
			return true;
		}
	}
	Number nr_bak(*this);std::cout << "Number.cc-5719" << endl;
	if(!setToFloatingPoint()) return false;
	if(mpfr_get_exp(fl_value) > 28 || mpfr_get_exp(fu_value) > 28) {
		set(nr_bak);std::cout << "Number.cc-5722" << endl;
		return false;
	}
	mpfr_clear_flags();std::cout << "Number.cc-5725" << endl;
	if(!CREATE_INTERVAL && !isInterval()) {
		mpfr_cosh(fl_value, fl_value, MPFR_RNDN);std::cout << "Number.cc-5727" << endl;
		mpfr_set(fu_value, fl_value, MPFR_RNDN);std::cout << "Number.cc-5728" << endl;
	} else {
		if(mpfr_sgn(fl_value) < 0) {
			if(mpfr_sgn(fu_value) == 0) {
				mpfr_cosh(fu_value, fl_value, MPFR_RNDU);std::cout << "Number.cc-5732" << endl;
				mpfr_set_ui(fl_value, 1, MPFR_RNDD);std::cout << "Number.cc-5733" << endl;
			} else if(mpfr_sgn(fu_value) > 0) {
				mpfr_cosh(fl_value, fl_value, MPFR_RNDU);std::cout << "Number.cc-5735" << endl;
				mpfr_cosh(fu_value, fu_value, MPFR_RNDD);std::cout << "Number.cc-5736" << endl;
				if(mpfr_cmp(fl_value, fu_value) > 0) mpfr_swap(fl_value, fu_value);
				mpfr_set_ui(fl_value, 1, MPFR_RNDD);std::cout << "Number.cc-5738" << endl;
			} else {
				mpfr_cosh(fl_value, fl_value, MPFR_RNDU);std::cout << "Number.cc-5740" << endl;
				mpfr_cosh(fu_value, fu_value, MPFR_RNDD);std::cout << "Number.cc-5741" << endl;
				mpfr_swap(fl_value, fu_value);std::cout << "Number.cc-5742" << endl;
			}
		} else {
			mpfr_cosh(fu_value, fu_value, MPFR_RNDU);std::cout << "Number.cc-5745" << endl;
			if(mpfr_sgn(fl_value) == 0) mpfr_set_ui(fl_value, 1, MPFR_RNDD);
			else mpfr_cosh(fl_value, fl_value, MPFR_RNDD);std::cout << "Number.cc-5747" << endl;
		}
	}
	if(!testFloatResult()) {
		set(nr_bak);std::cout << "Number.cc-5751" << endl;
		return false;
	}
	return true;
}
bool Number::acosh() {
	if(isPlusInfinity()) return true;
	if(isMinusInfinity()) return false;
	if(n_type == NUMBER_TYPE_FLOAT && (mpfr_inf_p(fl_value) && mpfr_sgn(fl_value) < 0)) return false;
	if(isOne()) {
		clear(true);std::cout << "Number.cc-5761" << endl;
		return true;
	}
	if(hasImaginaryPart() || !isGreaterThanOrEqualTo(nr_one)) {
		if(b_imag) return false;
		if(isInterval(false)) {
			Number nr1(lowerEndPoint(true));std::cout << "Number.cc-5767" << endl;
			Number nr2(upperEndPoint(true));std::cout << "Number.cc-5768" << endl;
			if(!nr1.acosh() || !nr2.acosh()) return false;
			Number nr;std::cout << "Number.cc-5770" << endl;
			if(!nr.setInterval(nr1, nr2, true)) return false;
			if(isInterval(true) && imaginaryPartIsInterval()) {
				Number nr3(lowerEndPoint(false));std::cout << "Number.cc-5773" << endl;
				Number nr4(upperEndPoint(false));std::cout << "Number.cc-5774" << endl;
				nr3.setImaginaryPart(i_value->upperEndPoint());std::cout << "Number.cc-5775" << endl;
				nr4.setImaginaryPart(i_value->lowerEndPoint());std::cout << "Number.cc-5776" << endl;
				if(!nr3.acosh() || !nr4.acosh()) return false;
				if(!nr.setInterval(nr, nr3, true)) return false;
				if(!nr.setInterval(nr, nr4, true)) return false;
			}
			if(hasRealPart() && !realPartIsNonZero()) {
				nr1 = lowerEndPoint(true);std::cout << "Number.cc-5782" << endl;
				nr2 = upperEndPoint(true);std::cout << "Number.cc-5783" << endl;
				nr1.clearReal();std::cout << "Number.cc-5784" << endl;
				nr2.clearReal();std::cout << "Number.cc-5785" << endl;
				if(!nr1.acosh() || !nr2.acosh()) return false;
				if(!nr.setInterval(nr, nr1, true)) return false;
				if(!nr.setInterval(nr, nr2, true)) return false;
			}
			if(hasImaginaryPart() && !imaginaryPartIsNonZero()) {
				nr1 = lowerEndPoint(false);std::cout << "Number.cc-5791" << endl;
				nr2 = upperEndPoint(false);std::cout << "Number.cc-5792" << endl;
				if(!nr1.acosh() || !nr2.acosh()) return false;
				Number nr_pi;std::cout << "Number.cc-5794" << endl;
				nr_pi.pi();std::cout << "Number.cc-5795" << endl;
				nr1.setImaginaryPart(nr_pi);std::cout << "Number.cc-5796" << endl;
				nr_pi.negate();std::cout << "Number.cc-5797" << endl;
				nr2.setImaginaryPart(nr_pi);std::cout << "Number.cc-5798" << endl;
				if(!nr.setInterval(nr, nr1, true)) return false;
				if(!nr.setInterval(nr, nr2, true)) return false;
			}
			set(nr, true);std::cout << "Number.cc-5802" << endl;
			return true;
		}
		if(CREATE_INTERVAL && !hasImaginaryPart()) {
			Number ipz(lowerEndPoint()), imz(ipz);std::cout << "Number.cc-5806" << endl;
			if(!ipz.add(1) || !imz.subtract(1)) return false;
			if(!ipz.raise(nr_half) || !imz.raise(nr_half) || !ipz.multiply(imz) || !ipz.add(lowerEndPoint())) return false;
			Number ipz2(upperEndPoint()), imz2(ipz2);std::cout << "Number.cc-5809" << endl;
			if(!ipz2.add(1) || !imz2.subtract(1)) return false;
			if(!ipz2.raise(nr_half) || !imz2.raise(nr_half) || !ipz2.multiply(imz2) || !ipz2.add(upperEndPoint())) return false;
			Number nriv;std::cout << "Number.cc-5812" << endl;
			nriv.setInterval(ipz, ipz2);std::cout << "Number.cc-5813" << endl;
			if(!nriv.ln()) return false;
			if(isGreaterThanOrEqualTo(nr_minus_one)) {
				nriv.clearReal();std::cout << "Number.cc-5816" << endl;
			} else {
				if(nriv.isInterval(false) && nriv.precision(1) <= PRECISION + 20) CALCULATOR->error(false, MESSAGE_CATEGORY_WIDE_INTERVAL, _("Interval calculated wide."), NULL);
			}
			set(nriv);std::cout << "Number.cc-5820" << endl;
			return true;
		}
		Number ipz(*this), imz(*this);std::cout << "Number.cc-5823" << endl;
		if(!ipz.add(1) || !imz.subtract(1)) return false;
		if(!ipz.raise(nr_half) || !imz.raise(nr_half) || !ipz.multiply(imz) || !ipz.add(*this) || !ipz.ln()) return false;
		if(hasImaginaryPart() && ipz.isInterval(false) && ipz.precision(1) <= PRECISION + 20) CALCULATOR->error(false, MESSAGE_CATEGORY_WIDE_INTERVAL, _("Interval calculated wide."), NULL);
		set(ipz);std::cout << "Number.cc-5827" << endl;
		return true;
	}
	Number nr_bak(*this);std::cout << "Number.cc-5830" << endl;
	if(!setToFloatingPoint()) return false;
	mpfr_clear_flags();std::cout << "Number.cc-5832" << endl;
	if(!CREATE_INTERVAL && !isInterval()) {
		mpfr_acosh(fl_value, fl_value, MPFR_RNDN);std::cout << "Number.cc-5834" << endl;
		mpfr_set(fu_value, fl_value, MPFR_RNDN);std::cout << "Number.cc-5835" << endl;
	} else {
		mpfr_acosh(fl_value, fl_value, MPFR_RNDD);std::cout << "Number.cc-5837" << endl;
		mpfr_acosh(fu_value, fu_value, MPFR_RNDU);std::cout << "Number.cc-5838" << endl;
	}
	if(!testFloatResult()) {
		set(nr_bak);std::cout << "Number.cc-5841" << endl;
		return false;
	}
	return true;
}
bool Number::tan() {
	if(includesInfinity()) return false;
	if(isZero()) return true;
	if(hasImaginaryPart()) {
		if(hasRealPart()) {
			Number t1a, t1b, t2a, t2b;std::cout << "Number.cc-5851" << endl;
			t1a.set(*this, false, true);std::cout << "Number.cc-5852" << endl;
			t1b.set(*i_value, false, true);std::cout << "Number.cc-5853" << endl;
			if(!t1a.multiply(2) || !t1b.multiply(2)) return false;
			t2a.set(t1a);std::cout << "Number.cc-5855" << endl;
			t2b.set(t1b);std::cout << "Number.cc-5856" << endl;
			if(!t1a.sin() || !t1b.sinh() || !t2a.cos() || !t2b.cosh() || !t2a.add(t2b) || !t1a.divide(t2a) ||  !t1b.divide(t2a)) return false;
			if(!t1a.isReal() || !t1b.isReal()) return false;
			if(t1a.isInterval(false) && t1a.precision(1) <= PRECISION + 20) CALCULATOR->error(false, MESSAGE_CATEGORY_WIDE_INTERVAL, _("Interval calculated wide."), NULL);
			set(t1a, true, true);std::cout << "Number.cc-5860" << endl;
			i_value->set(t1b, true, true);std::cout << "Number.cc-5861" << endl;
			setPrecisionAndApproximateFrom(*i_value);std::cout << "Number.cc-5862" << endl;
			testComplex(this, i_value);std::cout << "Number.cc-5863" << endl;
			return true;
		} else {
			if(!i_value->tanh()) return false;
			setPrecisionAndApproximateFrom(*i_value);std::cout << "Number.cc-5867" << endl;
			return true;
		}
	}
	Number nr_bak(*this);std::cout << "Number.cc-5871" << endl;
	bool do_pi = true;std::cout << "Number.cc-5872" << endl;
	if(n_type == NUMBER_TYPE_RATIONAL) {
		if(mpz_cmp_ui(mpq_denref(r_value), 1000000L) < 0) do_pi = false;
		if(!setToFloatingPoint()) return false;
	}
	if(mpfr_get_exp(fl_value) > BIT_PRECISION || mpfr_get_exp(fu_value) > BIT_PRECISION) {
		set(nr_bak);std::cout << "Number.cc-5878" << endl;
		return false;
	}

	mpfr_clear_flags();std::cout << "Number.cc-5882" << endl;

	if(!CREATE_INTERVAL && !isInterval()) {
		if(do_pi) {
			mpfr_t f_pi, f_quo;std::cout << "Number.cc-5886" << endl;
			mpfr_init2(f_pi, BIT_PRECISION);std::cout << "Number.cc-5887" << endl;
			mpfr_init2(f_quo, BIT_PRECISION - 30);std::cout << "Number.cc-5888" << endl;
			mpfr_const_pi(f_pi, MPFR_RNDN);std::cout << "Number.cc-5889" << endl;
			mpfr_div(f_quo, fl_value, f_pi, MPFR_RNDN);std::cout << "Number.cc-5890" << endl;
			mpfr_frac(f_quo, f_quo, MPFR_RNDN);std::cout << "Number.cc-5891" << endl;
			if(mpfr_zero_p(f_quo)) {
				clear(true);std::cout << "Number.cc-5893" << endl;
				b_approx = true;std::cout << "Number.cc-5894" << endl;
				if(i_precision < 0 || FROM_BIT_PRECISION(BIT_PRECISION - 30) < i_precision) i_precision = FROM_BIT_PRECISION(BIT_PRECISION - 30);
				mpfr_clears(f_pi, f_quo, NULL);std::cout << "Number.cc-5896" << endl;
				return true;
			}
			mpfr_clears(f_pi, f_quo, NULL);std::cout << "Number.cc-5899" << endl;
		}
		mpfr_tan(fl_value, fl_value, MPFR_RNDN);std::cout << "Number.cc-5901" << endl;
		mpfr_set(fu_value, fl_value, MPFR_RNDN);std::cout << "Number.cc-5902" << endl;
	} else {
		mpfr_t fl_pi, fu_pi, fu_quo, fl_quo, f_diff;
		mpfr_inits2(BIT_PRECISION, fl_pi, fu_pi, f_diff, fl_quo, fu_quo, NULL);
		mpfr_const_pi(fl_pi, MPFR_RNDD);std::cout << "Number.cc-5906" << endl;
		mpfr_const_pi(fu_pi, MPFR_RNDU);std::cout << "Number.cc-5907" << endl;
		bool b_neg1 = mpfr_sgn(fl_value) < 0;std::cout << "Number.cc-5908" << endl;
		bool b_neg2 = mpfr_sgn(fu_value) < 0;std::cout << "Number.cc-5909" << endl;
		mpfr_div(fl_quo, fl_value, b_neg1 ? fl_pi : fu_pi, MPFR_RNDD);std::cout << "Number.cc-5910" << endl;
		mpfr_div(fu_quo, fu_value, b_neg2 ? fu_pi : fl_pi, MPFR_RNDU);std::cout << "Number.cc-5911" << endl;
		mpfr_sub(f_diff, fu_quo, fl_quo, MPFR_RNDU);
		if(mpfr_cmp_ui(f_diff, 1) >= 0) {
			mpfr_clears(f_diff, fl_pi, fu_pi, fl_quo, fu_quo, NULL);
			set(nr_bak);std::cout << "Number.cc-5915" << endl;
			return false;
		}
		mpfr_frac(fl_quo, fl_quo, MPFR_RNDD);std::cout << "Number.cc-5918" << endl;
		mpfr_frac(fu_quo, fu_quo, MPFR_RNDU);std::cout << "Number.cc-5919" << endl;
		if(b_neg1) mpfr_neg(fl_quo, fl_quo, MPFR_RNDU);
		if(b_neg2) mpfr_neg(fu_quo, fu_quo, MPFR_RNDD);
		int c1 = mpfr_cmp_ui_2exp(fl_quo, 1, -1);std::cout << "Number.cc-5922" << endl;
		int c2 = mpfr_cmp_ui_2exp(fu_quo, 1, -1);std::cout << "Number.cc-5923" << endl;
		if(b_neg1) c1 = -c1;
		if(b_neg2) c2 = -c2;
		if((c1 != c2 && c1 <= 0 && c2 >= 0) || (c1 == c2 && mpfr_cmp_ui_2exp(f_diff, 1, -1) >= 0)) {
			mpfr_clears(f_diff, fl_pi, fu_pi, fl_quo, fu_quo, NULL);
			set(nr_bak);std::cout << "Number.cc-5928" << endl;
			return false;
		}
		mpfr_clears(f_diff, fl_pi, fu_pi, fl_quo, fu_quo, NULL);
		mpfr_tan(fl_value, fl_value, MPFR_RNDD);std::cout << "Number.cc-5932" << endl;
		mpfr_tan(fu_value, fu_value, MPFR_RNDU);std::cout << "Number.cc-5933" << endl;
	}

	if(!testFloatResult()) {
		set(nr_bak);std::cout << "Number.cc-5937" << endl;
		return false;
	}
	return true;
}
bool Number::atan() {
	if(isZero()) return true;
	if(isInfinite(false)) {
		bool b_neg = isMinusInfinity();std::cout << "Number.cc-5945" << endl;
		pi();std::cout << "Number.cc-5946" << endl;
		divide(2);std::cout << "Number.cc-5947" << endl;
		if(b_neg) negate();
		return true;
	}
	if(hasImaginaryPart()) {
		if(hasRealPart()) {
			Number ipz(*this), imz(*this);std::cout << "Number.cc-5953" << endl;
			if(!ipz.multiply(nr_one_i) || !imz.multiply(nr_minus_i) || !ipz.add(1) || !imz.add(1)) return false;
			if(!ipz.ln() || !imz.ln() || !imz.subtract(ipz) || !imz.multiply(nr_one_i) || !imz.divide(2)) return false;
			if(imz.isInterval(false) && imz.precision(1) <= PRECISION + 20) CALCULATOR->error(false, MESSAGE_CATEGORY_WIDE_INTERVAL, _("Interval calculated wide."), NULL);
			set(imz);std::cout << "Number.cc-5957" << endl;
			return true;
		} else {
			Number nri(*i_value);std::cout << "Number.cc-5960" << endl;
			if(!nri.atanh() || !nri.multiply(nr_one_i)) return false;
			set(nri, true);std::cout << "Number.cc-5962" << endl;
			return true;
		}
	}
	Number nr_bak(*this);std::cout << "Number.cc-5966" << endl;
	if(!setToFloatingPoint()) return false;
	mpfr_clear_flags();std::cout << "Number.cc-5968" << endl;
	if(!CREATE_INTERVAL && !isInterval()) {
		mpfr_atan(fl_value, fl_value, MPFR_RNDN);std::cout << "Number.cc-5970" << endl;
		mpfr_set(fu_value, fl_value, MPFR_RNDN);std::cout << "Number.cc-5971" << endl;
	} else {
		mpfr_atan(fl_value, fl_value, MPFR_RNDD);std::cout << "Number.cc-5973" << endl;
		mpfr_atan(fu_value, fu_value, MPFR_RNDU);std::cout << "Number.cc-5974" << endl;
	}
	if(!testFloatResult()) {
		set(nr_bak);std::cout << "Number.cc-5977" << endl;
		return false;
	}
	return true;
}
bool Number::atan2(const Number &o, bool allow_zero) {
	if(hasImaginaryPart() || o.hasImaginaryPart()) return false;
	if(isZero()) {
		if(allow_zero && o.isNonNegative()) {
			clear();std::cout << "Number.cc-5986" << endl;
			setPrecisionAndApproximateFrom(o);std::cout << "Number.cc-5987" << endl;
			return true;
		}
		if(o.isZero()) return false;
		if(o.isPositive()) {
			clear();std::cout << "Number.cc-5992" << endl;
			setPrecisionAndApproximateFrom(o);std::cout << "Number.cc-5993" << endl;
			return true;
		}
	}
	Number nr_bak(*this);std::cout << "Number.cc-5997" << endl;
	if(!setToFloatingPoint()) return false;
	mpfr_clear_flags();std::cout << "Number.cc-5999" << endl;
	if(o.isFloatingPoint()) {
		if(!CREATE_INTERVAL && !isInterval() && !o.isInterval()) {
			mpfr_atan2(fl_value, fl_value, o.internalLowerFloat(), MPFR_RNDN);std::cout << "Number.cc-6002" << endl;
			mpfr_set(fu_value, fl_value, MPFR_RNDN);std::cout << "Number.cc-6003" << endl;
		} else {
			int sgn_l = mpfr_sgn(fl_value);std::cout << "Number.cc-6005" << endl;
			int sgn_u = mpfr_sgn(fu_value);std::cout << "Number.cc-6006" << endl;
			if(!allow_zero && !o.isNonZero() && (sgn_l != sgn_u || sgn_l == 0)) {set(nr_bak); return false;}
			int sgn_lo = mpfr_sgn(o.internalLowerFloat());std::cout << "Number.cc-6008" << endl;
			int sgn_uo = mpfr_sgn(o.internalUpperFloat());std::cout << "Number.cc-6009" << endl;
			if(sgn_lo < 0) {
				if(sgn_u >= 0) {
					if(sgn_l < 0) {
						mpfr_const_pi(fu_value, MPFR_RNDD);std::cout << "Number.cc-6013" << endl;
						mpfr_neg(fu_value, fu_value, MPFR_RNDD);std::cout << "Number.cc-6014" << endl;
					} else if(sgn_uo >= 0) {
						if(sgn_l == 0) mpfr_set_zero(fu_value, 0);
						else mpfr_atan2(fu_value, fl_value, o.internalUpperFloat(), MPFR_RNDD);std::cout << "Number.cc-6017" << endl;
					} else {
						mpfr_atan2(fu_value, fu_value, o.internalUpperFloat(), MPFR_RNDD);std::cout << "Number.cc-6019" << endl;
					}
					if(sgn_l <= 0) mpfr_const_pi(fl_value, MPFR_RNDU);
					else mpfr_atan2(fl_value, fl_value, o.internalLowerFloat(), MPFR_RNDU);std::cout << "Number.cc-6022" << endl;
					mpfr_swap(fl_value, fu_value);std::cout << "Number.cc-6023" << endl;
				} else {
					mpfr_atan2(fl_value, fl_value, o.internalUpperFloat(), MPFR_RNDU);std::cout << "Number.cc-6025" << endl;
					mpfr_atan2(fu_value, fu_value, o.internalLowerFloat(), MPFR_RNDD);std::cout << "Number.cc-6026" << endl;
					mpfr_swap(fl_value, fu_value);std::cout << "Number.cc-6027" << endl;
				}
			} else {
				if(sgn_u >= 0) {
					if(sgn_u == 0) mpfr_set_zero(fu_value, 0);
					else mpfr_atan2(fu_value, fu_value, o.internalLowerFloat(), MPFR_RNDU);std::cout << "Number.cc-6032" << endl;
					if(sgn_l == 0) mpfr_set_zero(fl_value, 0);
					else if(sgn_l < 0) mpfr_atan2(fl_value, fl_value, o.internalLowerFloat(), MPFR_RNDD);
					else mpfr_atan2(fl_value, fl_value, o.internalUpperFloat(), MPFR_RNDD);std::cout << "Number.cc-6035" << endl;
				} else {
					mpfr_atan2(fl_value, fl_value, o.internalLowerFloat(), MPFR_RNDD);std::cout << "Number.cc-6037" << endl;
					mpfr_atan2(fu_value, fu_value, o.internalUpperFloat(), MPFR_RNDU);std::cout << "Number.cc-6038" << endl;
				}
			}
		}
	} else {
		if(!CREATE_INTERVAL && !isInterval()) {
			mpfr_t of_value;std::cout << "Number.cc-6044" << endl;
			mpfr_init2(of_value, BIT_PRECISION);std::cout << "Number.cc-6045" << endl;
			if(o.isPlusInfinity()) mpfr_set_inf(of_value, 1);
			else if(o.isMinusInfinity()) mpfr_set_inf(of_value, -1);
			else mpfr_set_q(of_value, o.internalRational(), MPFR_RNDN);std::cout << "Number.cc-6048" << endl;
			mpfr_atan2(fl_value, fl_value, of_value, MPFR_RNDN);std::cout << "Number.cc-6049" << endl;
			mpfr_set(fu_value, fl_value, MPFR_RNDN);std::cout << "Number.cc-6050" << endl;
			mpfr_clear(of_value);std::cout << "Number.cc-6051" << endl;
		} else {
			Number nr_o(o);std::cout << "Number.cc-6053" << endl;
			if(!nr_o.setToFloatingPoint() || !atan2(nr_o)) return false;
			return true;
		}
	}
	if(!testFloatResult()) {
		set(nr_bak);std::cout << "Number.cc-6059" << endl;
		return false;
	}
	return true;
}
bool Number::arg() {
	if(!isNonZero()) return false;
	if(!hasImaginaryPart()) {
		if(isNegative()) {
			pi();std::cout << "Number.cc-6068" << endl;
		} else {
			clear(true);std::cout << "Number.cc-6070" << endl;
		}
		return true;
	}
	if(!hasRealPart()) {
		bool b_neg = i_value->isNegative();std::cout << "Number.cc-6075" << endl;
		pi();std::cout << "Number.cc-6076" << endl;
		multiply(nr_half);std::cout << "Number.cc-6077" << endl;
		if(b_neg) negate();
		return true;
	}
	Number *i_value2 = i_value;std::cout << "Number.cc-6081" << endl;
	i_value = NULL;std::cout << "Number.cc-6082" << endl;
	if(!i_value2->atan2(*this)) {
		i_value = i_value2;std::cout << "Number.cc-6084" << endl;
		return false;
	}
	set(*i_value2);std::cout << "Number.cc-6087" << endl;
	delete i_value2;std::cout << "Number.cc-6088" << endl;
	return true;
}
bool Number::tanh() {
	if(isPlusInfinity()) {set(1, 1, 0, true); return true;}
	if(isMinusInfinity()) {set(-1, 1, 0, true); return true;}
	if(isZero()) return true;
	if(hasImaginaryPart()) {
		if(hasRealPart()) {
			Number t1a, t1b, t2a, t2b;std::cout << "Number.cc-6097" << endl;
			t1a.set(*this, false, true);std::cout << "Number.cc-6098" << endl;
			t1b.set(*this, false);std::cout << "Number.cc-6099" << endl;
			t1b.clearReal();std::cout << "Number.cc-6100" << endl;
			if(!t1a.tanh() || !t1b.tanh()) return false;
			t2a.set(t1a);std::cout << "Number.cc-6102" << endl;
			t2b.set(t1b);std::cout << "Number.cc-6103" << endl;
			if(!t1a.add(t1b) || !t2a.multiply(t2b) || !t2a.add(1) || !t1a.divide(t2a)) return false;
			if(t1a.isInterval(false) && t1a.precision(1) <= PRECISION + 20) CALCULATOR->error(false, MESSAGE_CATEGORY_WIDE_INTERVAL, _("Interval calculated wide."), NULL);
			set(t1a, true);std::cout << "Number.cc-6106" << endl;
			return true;
		} else {
			if(!i_value->tan()) return false;
			setPrecisionAndApproximateFrom(*i_value);std::cout << "Number.cc-6110" << endl;
			return true;
		}
	}
	Number nr_bak(*this);std::cout << "Number.cc-6114" << endl;
	if(!setToFloatingPoint()) return false;
	mpfr_clear_flags();std::cout << "Number.cc-6116" << endl;
	if(!CREATE_INTERVAL && !isInterval()) {
		mpfr_tanh(fl_value, fl_value, MPFR_RNDN);std::cout << "Number.cc-6118" << endl;
		mpfr_set(fu_value, fl_value, MPFR_RNDN);std::cout << "Number.cc-6119" << endl;
	} else {
		mpfr_tanh(fl_value, fl_value, MPFR_RNDD);std::cout << "Number.cc-6121" << endl;
		mpfr_tanh(fu_value, fu_value, MPFR_RNDU);std::cout << "Number.cc-6122" << endl;
	}
	if(!testFloatResult()) {
		set(nr_bak);std::cout << "Number.cc-6125" << endl;
		return false;
	}
	return true;
}
bool Number::atanh() {
	if(isZero()) return true;
	if(isOne()) {
		if(b_imag) return false;
		setPlusInfinity();std::cout << "Number.cc-6134" << endl;
		return true;
	}
	if(isMinusOne()) {
		if(b_imag) return false;
		setMinusInfinity();std::cout << "Number.cc-6139" << endl;
		return true;
	}
	if(hasImaginaryPart() || !isLessThanOrEqualTo(1) || !isGreaterThanOrEqualTo(-1)) {
		if(b_imag) return false;
		if(!hasImaginaryPart()) {
			Number nr_bak(*this);std::cout << "Number.cc-6145" << endl;
			if(!setToFloatingPoint()) return false;
			mpfr_clear_flags();std::cout << "Number.cc-6147" << endl;
			Number i_nr;std::cout << "Number.cc-6148" << endl;
			i_nr.markAsImaginaryPart();std::cout << "Number.cc-6149" << endl;
			int cmp1u = mpfr_cmp_si(fu_value, 1);std::cout << "Number.cc-6150" << endl;
			int cmp1l = mpfr_cmp_si(fl_value, 1);std::cout << "Number.cc-6151" << endl;
			int cmp0u = mpfr_sgn(fu_value);std::cout << "Number.cc-6152" << endl;
			int cmp0l = mpfr_sgn(fl_value);std::cout << "Number.cc-6153" << endl;
			int cmpm1u = mpfr_cmp_si(fu_value, -1);std::cout << "Number.cc-6154" << endl;
			int cmpm1l = mpfr_cmp_si(fl_value, -1);std::cout << "Number.cc-6155" << endl;
			if(cmp1u > 0) {
				i_nr.pi();std::cout << "Number.cc-6157" << endl;
				if(!i_nr.multiply(nr_minus_half)) {set(nr_bak); return false;}
				if(cmpm1l < 0) {
					Number nr;std::cout << "Number.cc-6160" << endl;
					nr.pi();std::cout << "Number.cc-6161" << endl;
					if(!nr.multiply(nr_half) || !i_nr.setInterval(i_nr, nr)) {set(nr_bak); return false;}
				} else if(cmp1l <= 0) {
					if(!i_nr.setInterval(i_nr, nr_zero)) {set(nr_bak); return false;}
				}
			} else {
				i_nr.pi();std::cout << "Number.cc-6167" << endl;
				if(!i_nr.multiply(nr_half)) {set(nr_bak); return false;}
				if(cmp1u > 0) {
					Number nr;std::cout << "Number.cc-6170" << endl;
					nr.pi();std::cout << "Number.cc-6171" << endl;
					if(!nr.multiply(nr_minus_half) || !i_nr.setInterval(nr, i_nr)) {set(nr_bak); return false;}
				} else if(cmpm1u >= 0) {
					if(!i_nr.setInterval(nr_zero, i_nr)) {set(nr_bak); return false;}
				}
			}
			if((cmp1u >= 0 && cmp1l >= 0) || (cmpm1u <= 0 && cmpm1l <= 0)) {
				if(!recip() || !atanh()) {set(nr_bak); return false;}
			} else if(cmp1u >= 0 && cmpm1l <= 0) {
				mpfr_set_inf(fl_value, -1);std::cout << "Number.cc-6180" << endl;
				mpfr_set_inf(fu_value, 1);std::cout << "Number.cc-6181" << endl;
			} else if(cmp1u <= 0) {
				if(cmp0u < 0) {
					mpfr_ui_div(fl_value, 1, fl_value, MPFR_RNDU);std::cout << "Number.cc-6184" << endl;
					if(mpfr_cmp(fl_value, fu_value) > 0) {
						mpfr_swap(fl_value, fu_value);std::cout << "Number.cc-6186" << endl;
					}
				}
				mpfr_atanh(fu_value, fu_value, MPFR_RNDU);std::cout << "Number.cc-6189" << endl;
				mpfr_set_inf(fl_value, -1);std::cout << "Number.cc-6190" << endl;
			} else {
				if(cmp0l > 0) {
					mpfr_ui_div(fu_value, 1, fu_value, MPFR_RNDD);std::cout << "Number.cc-6193" << endl;
					if(mpfr_cmp(fu_value, fl_value) < 0) {
						mpfr_swap(fl_value, fu_value);std::cout << "Number.cc-6195" << endl;
					}
				}
				mpfr_atanh(fl_value, fl_value, MPFR_RNDD);std::cout << "Number.cc-6198" << endl;
				mpfr_set_inf(fu_value, 1);std::cout << "Number.cc-6199" << endl;
			}
			if(!i_value) {i_value = new Number(i_nr); i_value->markAsImaginaryPart();}
			else i_value->set(i_nr, true);std::cout << "Number.cc-6202" << endl;
			setPrecisionAndApproximateFrom(*i_value);std::cout << "Number.cc-6203" << endl;
			if(!testFloatResult()) {
				set(nr_bak);std::cout << "Number.cc-6205" << endl;
				return false;
			}
			return true;
		} else if(!hasRealPart()) {
			Number inr(*i_value);std::cout << "Number.cc-6210" << endl;
			if(!inr.atan() || !inr.multiply(nr_one_i)) return false;
			set(inr, true);std::cout << "Number.cc-6212" << endl;
			return true;
		}
		Number ipz(nr_one), imz(nr_one);std::cout << "Number.cc-6215" << endl;
		if(!ipz.add(*this) || !imz.subtract(*this) || !ipz.ln() || !imz.ln() || !imz.negate() || !ipz.add(imz) || !ipz.divide(2)) return false;
		if(ipz.isInterval(false) && ipz.precision(1) <= PRECISION + 20) CALCULATOR->error(false, MESSAGE_CATEGORY_WIDE_INTERVAL, _("Interval calculated wide."), NULL);
		set(ipz);std::cout << "Number.cc-6218" << endl;
		return true;
	}
	Number nr_bak(*this);std::cout << "Number.cc-6221" << endl;
	if(!setToFloatingPoint()) return false;
	mpfr_clear_flags();std::cout << "Number.cc-6223" << endl;
	if(!CREATE_INTERVAL && !isInterval()) {
		if(mpfr_cmp_si(fl_value, -1) == 0) mpfr_set_inf(fl_value, -1);
		else if(mpfr_cmp_si(fl_value, 1) == 0) mpfr_set_inf(fl_value, 1);
		else mpfr_atanh(fl_value, fl_value, MPFR_RNDN);std::cout << "Number.cc-6227" << endl;
		mpfr_set(fu_value, fl_value, MPFR_RNDN);std::cout << "Number.cc-6228" << endl;
	} else {
		if(mpfr_cmp_si(fl_value, -1) == 0) mpfr_set_inf(fl_value, -1);
		else if(mpfr_cmp_si(fl_value, 1) == 0) mpfr_set_inf(fl_value, 1);
		else mpfr_atanh(fl_value, fl_value, MPFR_RNDD);std::cout << "Number.cc-6232" << endl;
		if(mpfr_cmp_si(fu_value, -1) == 0) mpfr_set_inf(fu_value, -1);
		else if(mpfr_cmp_si(fu_value, 1) == 0) mpfr_set_inf(fu_value, 1);
		else mpfr_atanh(fu_value, fu_value, MPFR_RNDU);std::cout << "Number.cc-6235" << endl;
	}
	if(!testFloatResult()) {
		set(nr_bak);std::cout << "Number.cc-6238" << endl;
		return false;
	}
	return true;
}
bool Number::ln() {

	if(isPlusInfinity()) return true;
	if(isMinusInfinity()) {
		n_type = NUMBER_TYPE_PLUS_INFINITY;std::cout << "Number.cc-6247" << endl;
		if(!i_value) {i_value = new Number(); i_value->markAsImaginaryPart();}
		i_value->pi();std::cout << "Number.cc-6249" << endl;
		return true;
	}

	if(isOne()) {
		clear(true);std::cout << "Number.cc-6254" << endl;
		return true;
	}

	if(isZero()) {
		if(b_imag) return false;
		setMinusInfinity();std::cout << "Number.cc-6260" << endl;
		return true;
	}


	if(hasImaginaryPart()) {
		Number new_i(*i_value);std::cout << "Number.cc-6266" << endl;
		Number new_r(*this);std::cout << "Number.cc-6267" << endl;
		Number this_r;std::cout << "Number.cc-6268" << endl;
		this_r.set(*this, false, true);std::cout << "Number.cc-6269" << endl;
		if(!new_i.atan2(this_r, true) || !new_r.abs() || new_i.hasImaginaryPart()) return false;
		if(new_r.hasImaginaryPart() || !new_r.ln()) return false;
		set(new_r);std::cout << "Number.cc-6272" << endl;
		setImaginaryPart(new_i);std::cout << "Number.cc-6273" << endl;
		testComplex(this, i_value);std::cout << "Number.cc-6274" << endl;
		return true;
	} else if(isNonPositive()) {
		if(b_imag) return false;
		Number new_r(*this);std::cout << "Number.cc-6278" << endl;
		if(!new_r.abs() || !new_r.ln()) return false;
		set(new_r);std::cout << "Number.cc-6280" << endl;
		if(!i_value) {i_value = new Number(); i_value->markAsImaginaryPart();}
		i_value->pi();std::cout << "Number.cc-6282" << endl;
		testComplex(this, i_value);std::cout << "Number.cc-6283" << endl;
		return true;
	}

	Number nr_bak(*this);std::cout << "Number.cc-6287" << endl;
	if(!setToFloatingPoint()) return false;
	mpfr_clear_flags();std::cout << "Number.cc-6289" << endl;

	if(!CREATE_INTERVAL && !isInterval()) {
		mpfr_log(fl_value, fl_value, MPFR_RNDN);std::cout << "Number.cc-6292" << endl;
		mpfr_set(fu_value, fl_value, MPFR_RNDN);std::cout << "Number.cc-6293" << endl;
	} else {
		if(mpfr_sgn(fl_value) < 0) {
			if(mpfr_cmpabs(fl_value, fu_value) > 0) {
				mpfr_neg(fu_value, fl_value, MPFR_RNDU);std::cout << "Number.cc-6297" << endl;
			}
			mpfr_set_inf(fl_value, -1);std::cout << "Number.cc-6299" << endl;
			mpfr_log(fu_value, fu_value, MPFR_RNDU);std::cout << "Number.cc-6300" << endl;
			if(!i_value) {i_value = new Number(); i_value->markAsImaginaryPart();}
			i_value->pi();std::cout << "Number.cc-6302" << endl;
			i_value->setInterval(nr_zero, *i_value);std::cout << "Number.cc-6303" << endl;
		} else {
			if(mpfr_zero_p(fl_value)) mpfr_set_inf(fl_value, -1);
			else mpfr_log(fl_value, fl_value, MPFR_RNDD);std::cout << "Number.cc-6306" << endl;
			mpfr_log(fu_value, fu_value, MPFR_RNDU);std::cout << "Number.cc-6307" << endl;
		}
	}

	if(!testFloatResult(true)) {
		set(nr_bak);std::cout << "Number.cc-6312" << endl;
		return false;
	}

	return true;
}
bool Number::log(const Number &o) {
	if(isOne() && !o.isGreaterThan(1)) {
		clear(true);std::cout << "Number.cc-6320" << endl;
		setPrecisionAndApproximateFrom(o);std::cout << "Number.cc-6321" << endl;
		return true;
	}
	if(o.isOne()) return false;
	if(o.isRational() && o == 2 && isReal() && isPositive()) {
		Number nr_bak(*this);std::cout << "Number.cc-6326" << endl;
		if(!setToFloatingPoint()) return false;
		mpfr_clear_flags();std::cout << "Number.cc-6328" << endl;
		if(!CREATE_INTERVAL && !isInterval()) {
			mpfr_log2(fl_value, fl_value, MPFR_RNDN);std::cout << "Number.cc-6330" << endl;
			mpfr_set(fu_value, fl_value, MPFR_RNDN);std::cout << "Number.cc-6331" << endl;
		} else {
			mpfr_log2(fl_value, fl_value, MPFR_RNDD);std::cout << "Number.cc-6333" << endl;
			mpfr_log2(fu_value, fu_value, MPFR_RNDU);std::cout << "Number.cc-6334" << endl;
		}
		if(!testFloatResult(true)) {
			set(nr_bak);std::cout << "Number.cc-6337" << endl;
			return false;
		}
		setPrecisionAndApproximateFrom(o);std::cout << "Number.cc-6340" << endl;
		return true;
	} else if(o.isRational() && o == 10 && isReal() && isPositive()) {
		Number nr_bak(*this);std::cout << "Number.cc-6343" << endl;
		if(!setToFloatingPoint()) return false;
		mpfr_clear_flags();std::cout << "Number.cc-6345" << endl;
		if(!CREATE_INTERVAL && !isInterval()) {
			mpfr_log10(fl_value, fl_value, MPFR_RNDN);std::cout << "Number.cc-6347" << endl;
			mpfr_set(fu_value, fl_value, MPFR_RNDN);std::cout << "Number.cc-6348" << endl;
		} else {
			mpfr_log10(fl_value, fl_value, MPFR_RNDD);std::cout << "Number.cc-6350" << endl;
			mpfr_log10(fu_value, fu_value, MPFR_RNDU);std::cout << "Number.cc-6351" << endl;
		}
		if(!testFloatResult(true)) {
			set(nr_bak);std::cout << "Number.cc-6354" << endl;
			return false;
		}
		setPrecisionAndApproximateFrom(o);std::cout << "Number.cc-6357" << endl;
		return true;
	}
	Number num(*this);std::cout << "Number.cc-6360" << endl;
	Number den(o);std::cout << "Number.cc-6361" << endl;
	if(!num.ln() || !den.ln() || !den.recip() || !num.multiply(den)) return false;
	if(b_imag && num.hasImaginaryPart()) return false;
	set(num);std::cout << "Number.cc-6364" << endl;
	return true;
}
bool Number::exp() {
	if(isPlusInfinity()) return true;
	if(isMinusInfinity()) {
		clear();std::cout << "Number.cc-6370" << endl;
		return true;
	}
	if(hasImaginaryPart()) {
		Number e_base;std::cout << "Number.cc-6374" << endl;
		e_base.e();std::cout << "Number.cc-6375" << endl;
		if(!e_base.raise(*this)) return false;
		set(e_base);std::cout << "Number.cc-6377" << endl;
		return true;
	}
	Number nr_bak(*this);std::cout << "Number.cc-6380" << endl;
	if(!setToFloatingPoint()) return false;
	mpfr_clear_flags();std::cout << "Number.cc-6382" << endl;

	if(!CREATE_INTERVAL && !isInterval()) {
		mpfr_exp(fl_value, fl_value, MPFR_RNDN);std::cout << "Number.cc-6385" << endl;
		mpfr_set(fu_value, fl_value, MPFR_RNDN);std::cout << "Number.cc-6386" << endl;
	} else {
		mpfr_exp(fu_value, fu_value, MPFR_RNDU);std::cout << "Number.cc-6388" << endl;
		mpfr_exp(fl_value, fl_value, MPFR_RNDD);std::cout << "Number.cc-6389" << endl;
	}
	if(!testFloatResult(true)) {
		set(nr_bak);std::cout << "Number.cc-6392" << endl;
		return false;
	}
	return true;
}
bool Number::lambertW() {

	if(!isReal()) return false;
	if(isZero()) return true;

	if(isInterval()) {
		Number nr_l, nr_u;std::cout << "Number.cc-6403" << endl;
		nr_l.setInternal(fl_value);std::cout << "Number.cc-6404" << endl;
		nr_u.setInternal(fu_value);std::cout << "Number.cc-6405" << endl;
		nr_l.intervalToPrecision();std::cout << "Number.cc-6406" << endl;
		nr_u.intervalToPrecision();std::cout << "Number.cc-6407" << endl;
		if(!nr_l.lambertW() || !nr_u.lambertW()) return false;
		setPrecisionAndApproximateFrom(nr_l);std::cout << "Number.cc-6409" << endl;
		setPrecisionAndApproximateFrom(nr_u);std::cout << "Number.cc-6410" << endl;
		return setInterval(nr_l, nr_u, true);
	}

	Number nr_bak(*this);std::cout << "Number.cc-6414" << endl;
	mpfr_clear_flags();std::cout << "Number.cc-6415" << endl;

	mpfr_t x, m1_div_exp1;std::cout << "Number.cc-6417" << endl;
	mpfr_inits2(BIT_PRECISION, x, m1_div_exp1, NULL);std::cout << "Number.cc-6418" << endl;
	if(n_type == NUMBER_TYPE_RATIONAL) mpfr_set_q(x, r_value, MPFR_RNDN);
	else mpfr_set(x, fl_value, MPFR_RNDN);std::cout << "Number.cc-6420" << endl;
	mpfr_set_ui(m1_div_exp1, 1, MPFR_RNDN);std::cout << "Number.cc-6421" << endl;
	mpfr_exp(m1_div_exp1, m1_div_exp1, MPFR_RNDN);std::cout << "Number.cc-6422" << endl;
	mpfr_ui_div(m1_div_exp1, 1, m1_div_exp1, MPFR_RNDN);std::cout << "Number.cc-6423" << endl;
	mpfr_neg(m1_div_exp1, m1_div_exp1, MPFR_RNDN);std::cout << "Number.cc-6424" << endl;
	int cmp = mpfr_cmp(x, m1_div_exp1);std::cout << "Number.cc-6425" << endl;
	if(cmp == 0) {
		mpfr_clears(x, m1_div_exp1, NULL);std::cout << "Number.cc-6427" << endl;
		if(!CREATE_INTERVAL) {
			set(-1, 1);std::cout << "Number.cc-6429" << endl;
			b_approx = true;std::cout << "Number.cc-6430" << endl;
			i_precision = PRECISION;std::cout << "Number.cc-6431" << endl;
			return true;
		} else {
			mpfr_set_ui(fl_value, -1, MPFR_RNDD);std::cout << "Number.cc-6434" << endl;
			mpfr_set_ui(fu_value, -1, MPFR_RNDU);std::cout << "Number.cc-6435" << endl;
		}
	} else if(cmp < 0) {
		mpfr_clears(x, m1_div_exp1, NULL);std::cout << "Number.cc-6438" << endl;
		set(nr_bak);std::cout << "Number.cc-6439" << endl;
		return false;
	} else {
		mpfr_t w;std::cout << "Number.cc-6442" << endl;
		mpfr_init2(w, BIT_PRECISION);std::cout << "Number.cc-6443" << endl;
		mpfr_set_zero(w, 0);std::cout << "Number.cc-6444" << endl;
		cmp = mpfr_cmp_ui(x, 10);std::cout << "Number.cc-6445" << endl;
		if(cmp > 0) {
			mpfr_log(w, x, MPFR_RNDN);std::cout << "Number.cc-6447" << endl;
			mpfr_t wln;std::cout << "Number.cc-6448" << endl;
			mpfr_init2(wln, BIT_PRECISION);std::cout << "Number.cc-6449" << endl;
			mpfr_log(wln, w, MPFR_RNDN);std::cout << "Number.cc-6450" << endl;
			mpfr_sub(w, w, wln, MPFR_RNDN);std::cout << "Number.cc-6451" << endl;
			mpfr_clear(wln);std::cout << "Number.cc-6452" << endl;
		}

		mpfr_t wPrec, wTimesExpW, wPlusOneTimesExpW, testXW, tmp1, tmp2;std::cout << "Number.cc-6455" << endl;
		mpfr_inits2(BIT_PRECISION, wPrec, wTimesExpW, wPlusOneTimesExpW, testXW, tmp1, tmp2, NULL);std::cout << "Number.cc-6456" << endl;
		mpfr_set_si(wPrec, -(BIT_PRECISION - 30), MPFR_RNDN);std::cout << "Number.cc-6457" << endl;
		mpfr_exp2(wPrec, wPrec, MPFR_RNDN);std::cout << "Number.cc-6458" << endl;
		while(true) {
			if(CALCULATOR->aborted() || testErrors()) {
				mpfr_clears(x, m1_div_exp1, w, wPrec, wTimesExpW, wPlusOneTimesExpW, testXW, tmp1, tmp2, NULL);std::cout << "Number.cc-6461" << endl;
				set(nr_bak);std::cout << "Number.cc-6462" << endl;
				return false;
			}
			mpfr_exp(wTimesExpW, w, MPFR_RNDN);std::cout << "Number.cc-6465" << endl;
			mpfr_set(wPlusOneTimesExpW, wTimesExpW, MPFR_RNDN);std::cout << "Number.cc-6466" << endl;
			mpfr_mul(wTimesExpW, wTimesExpW, w, MPFR_RNDN);std::cout << "Number.cc-6467" << endl;
			mpfr_add(wPlusOneTimesExpW, wPlusOneTimesExpW, wTimesExpW, MPFR_RNDN);std::cout << "Number.cc-6468" << endl;
			mpfr_sub(testXW, x, wTimesExpW, MPFR_RNDN);std::cout << "Number.cc-6469" << endl;
			mpfr_div(testXW, testXW, wPlusOneTimesExpW, MPFR_RNDN);std::cout << "Number.cc-6470" << endl;
			mpfr_abs(testXW, testXW, MPFR_RNDN);std::cout << "Number.cc-6471" << endl;
			if(mpfr_cmp(wPrec, testXW) > 0) {
				break;
			}
			mpfr_sub(wTimesExpW, wTimesExpW, x, MPFR_RNDN);std::cout << "Number.cc-6475" << endl;
			mpfr_add_ui(tmp1, w, 2, MPFR_RNDN);std::cout << "Number.cc-6476" << endl;
			mpfr_mul(tmp2, wTimesExpW, tmp1, MPFR_RNDN);std::cout << "Number.cc-6477" << endl;
			mpfr_mul_ui(tmp1, w, 2, MPFR_RNDN);std::cout << "Number.cc-6478" << endl;
			mpfr_add_ui(tmp1, tmp1, 2, MPFR_RNDN);std::cout << "Number.cc-6479" << endl;
			mpfr_div(tmp2, tmp2, tmp1, MPFR_RNDN);std::cout << "Number.cc-6480" << endl;
			mpfr_sub(wPlusOneTimesExpW, wPlusOneTimesExpW, tmp2, MPFR_RNDN);std::cout << "Number.cc-6481" << endl;
			mpfr_div(wTimesExpW, wTimesExpW, wPlusOneTimesExpW, MPFR_RNDN);std::cout << "Number.cc-6482" << endl;
			mpfr_sub(w, w, wTimesExpW, MPFR_RNDN);std::cout << "Number.cc-6483" << endl;
		}
		if(n_type == NUMBER_TYPE_RATIONAL) {
			mpfr_init2(fl_value, BIT_PRECISION);std::cout << "Number.cc-6486" << endl;
			mpfr_init2(fu_value, BIT_PRECISION);std::cout << "Number.cc-6487" << endl;
			n_type = NUMBER_TYPE_FLOAT;std::cout << "Number.cc-6488" << endl;
			mpq_set_ui(r_value, 0, 1);std::cout << "Number.cc-6489" << endl;
		}
		mpfr_set(fl_value, w, MPFR_RNDN);std::cout << "Number.cc-6491" << endl;
		mpfr_set(fu_value, fl_value, MPFR_RNDN);std::cout << "Number.cc-6492" << endl;
		mpfr_clears(x, m1_div_exp1, w, wPrec, wTimesExpW, wPlusOneTimesExpW, testXW, tmp1, tmp2, NULL);std::cout << "Number.cc-6493" << endl;
		if(i_precision < 0 || i_precision > PRECISION) i_precision = FROM_BIT_PRECISION(BIT_PRECISION - 30);
		if(CREATE_INTERVAL) {
			precisionToInterval();std::cout << "Number.cc-6496" << endl;
		}
	}
	if(!testFloatResult(true)) {
		set(nr_bak);std::cout << "Number.cc-6500" << endl;
		return false;
	}
	b_approx = true;std::cout << "Number.cc-6503" << endl;
	return true;

}
bool Number::gcd(const Number &o) {
	if(!isInteger() || !o.isInteger()) {
		return false;
	}
	if(isZero() && o.isZero()) {
		clear();std::cout << "Number.cc-6512" << endl;
		return true;
	}
	mpz_gcd(mpq_numref(r_value), mpq_numref(r_value), mpq_numref(o.internalRational()));std::cout << "Number.cc-6515" << endl;
	setPrecisionAndApproximateFrom(o);std::cout << "Number.cc-6516" << endl;
	return true;
}
bool Number::lcm(const Number &o) {
	if(isInteger() && o.isInteger()) {
		mpz_lcm(mpq_numref(r_value), mpq_numref(r_value), mpq_numref(o.internalRational()));std::cout << "Number.cc-6521" << endl;
		return true;
	}
	return multiply(o);
}

bool Number::polylog(const Number &o) {
	if(isZero()) return true;
	if(!o.isTwo() || !isLessThanOrEqualTo(1)) return false;
	Number nr_bak(*this);std::cout << "Number.cc-6530" << endl;
	if(!setToFloatingPoint()) return false;
	mpfr_clear_flags();std::cout << "Number.cc-6532" << endl;
	if(!CREATE_INTERVAL && !isInterval()) {
		mpfr_li2(fl_value, fl_value, MPFR_RNDN);std::cout << "Number.cc-6534" << endl;
		mpfr_set(fu_value, fl_value, MPFR_RNDN);std::cout << "Number.cc-6535" << endl;
	} else {
		mpfr_li2(fl_value, fl_value, MPFR_RNDD);std::cout << "Number.cc-6537" << endl;
		mpfr_li2(fu_value, fu_value, MPFR_RNDU);std::cout << "Number.cc-6538" << endl;
	}
	if(!testFloatResult()) {
		set(nr_bak);std::cout << "Number.cc-6541" << endl;
		return false;
	}
	return true;
}
bool Number::igamma(const Number &o) {
#if MPFR_VERSION_MAJOR < 4
	return false;
#else
	if(!isReal() || (!o.isNonZero() && !isNonZero())) return false;
	Number nr_bak(*this);std::cout << "Number.cc-6551" << endl;
	if(!setToFloatingPoint()) return false;
	Number o_float(o);std::cout << "Number.cc-6553" << endl;
	if(!o_float.setToFloatingPoint()) return false;
	mpfr_clear_flags();std::cout << "Number.cc-6555" << endl;
	if(!CREATE_INTERVAL && !isInterval() && !o_float.isInterval()) {
		mpfr_gamma_inc(fl_value, fl_value, o_float.internalLowerFloat(), MPFR_RNDN);std::cout << "Number.cc-6557" << endl;
		mpfr_set(fu_value, fl_value, MPFR_RNDN);std::cout << "Number.cc-6558" << endl;
	} else {
		mpfr_gamma_inc(fl_value, fl_value, o_float.internalUpperFloat(), MPFR_RNDD);std::cout << "Number.cc-6560" << endl;
		mpfr_gamma_inc(fu_value, fu_value, o_float.internalLowerFloat(), MPFR_RNDU);std::cout << "Number.cc-6561" << endl;
		if(!o.isGreaterThanOrEqualTo(1) && !nr_bak.isGreaterThan(2) && nr_bak.isInterval() && nr_bak.precision(1) <= PRECISION + 20) CALCULATOR->error(false, _("%s() lacks proper support interval arithmetic."), CALCULATOR->f_igamma->name().c_str(), NULL);
	}
	if(!testFloatResult()) {
		set(nr_bak);std::cout << "Number.cc-6565" << endl;
		return false;
	}
	return true;
#endif
}
bool Number::expint() {
	if(hasImaginaryPart()) return false;
	if(isInfinite()) {clear(); return true;}
	Number nr_bak(*this);std::cout << "Number.cc-6574" << endl;
	if(!setToFloatingPoint()) return false;
	mpfr_clear_flags();std::cout << "Number.cc-6576" << endl;
	if(!CREATE_INTERVAL && !isInterval()) {
		mpfr_eint(fl_value, fl_value, MPFR_RNDN);std::cout << "Number.cc-6578" << endl;
		mpfr_set(fu_value, fl_value, MPFR_RNDN);std::cout << "Number.cc-6579" << endl;
	} else {
		if(mpfr_sgn(fl_value) < 0) {
			if(mpfr_sgn(fu_value) > 0) {
				mpfr_eint(fl_value, fl_value, MPFR_RNDU);std::cout << "Number.cc-6583" << endl;
				mpfr_eint(fu_value, fu_value, MPFR_RNDU);std::cout << "Number.cc-6584" << endl;
				if(mpfr_cmp(fl_value, fu_value) > 0) mpfr_swap(fl_value, fu_value);
				mpfr_set_inf(fl_value, -1);std::cout << "Number.cc-6586" << endl;
			} else {
				mpfr_eint(fl_value, fl_value, MPFR_RNDU);std::cout << "Number.cc-6588" << endl;
				mpfr_eint(fu_value, fu_value, MPFR_RNDD);std::cout << "Number.cc-6589" << endl;
				mpfr_swap(fl_value, fu_value);std::cout << "Number.cc-6590" << endl;
			}
		} else {
			mpfr_eint(fl_value, fl_value, MPFR_RNDD);std::cout << "Number.cc-6593" << endl;
			mpfr_eint(fu_value, fu_value, MPFR_RNDU);std::cout << "Number.cc-6594" << endl;
		}
	}
	if(!testFloatResult()) {
		set(nr_bak);std::cout << "Number.cc-6598" << endl;
		return false;
	}
	return true;
}
bool Number::logint() {
	if(isZero()) return true;
	Number nr_bak(*this);std::cout << "Number.cc-6605" << endl;
	if(!ln() || !expint()) {
		set(nr_bak);std::cout << "Number.cc-6607" << endl;
		return false;
	}
	return true;
}
bool Number::sinint() {
	if(isPlusInfinity()) {pi(); multiply(2); return true;}
	if(isMinusInfinity()) {pi(); multiply(-2); return true;}
	if(isZero()) return true;
	if(hasImaginaryPart()) {
		if(hasRealPart()) return false;
		if(!i_value->sinhint()) return false;
		setPrecisionAndApproximateFrom(*i_value);std::cout << "Number.cc-6619" << endl;
		return true;
	}
	Number nr_bak(*this);std::cout << "Number.cc-6622" << endl;
	if(isNegative()) {
		if(!negate() || !sinint() || !negate()) {set(nr_bak); return false;}
		return true;
	}
	if(!setToFloatingPoint()) return false;
	Number nr_round(*this);std::cout << "Number.cc-6628" << endl;
	nr_round.round();std::cout << "Number.cc-6629" << endl;
	if(isInterval()) {
		Number nr_lower(lowerEndPoint());std::cout << "Number.cc-6631" << endl;
		Number nr_upper(upperEndPoint());std::cout << "Number.cc-6632" << endl;
		if(nr_lower.isNegative() && nr_upper.isPositive()) {
			nr_lower.setInterval(nr_lower, nr_zero);std::cout << "Number.cc-6634" << endl;
			nr_upper.setInterval(nr_zero, nr_upper);std::cout << "Number.cc-6635" << endl;
			if(!nr_lower.sinint() || !nr_upper.sinint()) {
				set(nr_bak);std::cout << "Number.cc-6637" << endl;
				return false;
			}
			setInterval(nr_lower, nr_upper);std::cout << "Number.cc-6640" << endl;
			return true;
		}
		Number nr_mid1;std::cout << "Number.cc-6643" << endl;
		Number nr_mid2;std::cout << "Number.cc-6644" << endl;
		bool b1 = false, b2 = false, b = true;std::cout << "Number.cc-6645" << endl;
		if(mpfr_cmp_si(fl_value, 1) > 0) {
			mpfr_t f_pi, f_mid1, f_mid2;std::cout << "Number.cc-6647" << endl;
			if(mpfr_get_exp(fl_value) > 1000 * PRECISION) {
				pi();std::cout << "Number.cc-6649" << endl;
				divide(2);std::cout << "Number.cc-6650" << endl;
				return true;
			}
			if(mpfr_get_exp(fl_value) > 100000) {
				set(nr_bak);std::cout << "Number.cc-6654" << endl;
				return false;
			}
			mpfr_prec_t prec = mpfr_get_prec(fl_value) + mpfr_get_exp(fl_value);std::cout << "Number.cc-6657" << endl;
			if(nr_round > 100 + PRECISION * 5) prec += nr_round.intValue() * 5;
			else prec += 10;std::cout << "Number.cc-6659" << endl;
			mpfr_inits2(mpfr_get_prec(fl_value) + mpfr_get_exp(fl_value), f_pi, f_mid1, f_mid2, NULL);std::cout << "Number.cc-6660" << endl;
			mpfr_const_pi(f_pi, MPFR_RNDN);std::cout << "Number.cc-6661" << endl;
			mpfr_div(f_mid1, nr_lower.internalLowerFloat(), f_pi, MPFR_RNDN);std::cout << "Number.cc-6662" << endl;
			mpfr_ceil(f_mid1, f_mid1);std::cout << "Number.cc-6663" << endl;
			mpfr_add_ui(f_mid2, f_mid1, 1, MPFR_RNDN);std::cout << "Number.cc-6664" << endl;
			mpfr_mul(f_mid1, f_mid1, f_pi, MPFR_RNDN);std::cout << "Number.cc-6665" << endl;
			if(mpfr_cmp(f_mid1, nr_upper.internalLowerFloat()) < 0) {
				mpfr_mul(f_mid2, f_mid2, f_pi, MPFR_RNDN);std::cout << "Number.cc-6667" << endl;
				b1 = true;std::cout << "Number.cc-6668" << endl;
				b2 = mpfr_cmp(f_mid2, nr_upper.internalLowerFloat()) < 0;std::cout << "Number.cc-6669" << endl;
			}
			mpfr_clear_flags();std::cout << "Number.cc-6671" << endl;
			for(size_t i2 = 0; b && ((i2 == 0 && b1) || (i2 == 1 && b2)); i2++) {
				b = false;std::cout << "Number.cc-6673" << endl;
				mpfr_t f_x, f_xi, f_y;std::cout << "Number.cc-6674" << endl;
				mpz_t z_i, z_fac;std::cout << "Number.cc-6675" << endl;
				mpz_inits(z_i, z_fac, NULL);std::cout << "Number.cc-6676" << endl;
				if(nr_round > 100 + PRECISION * 5) {
					mpfr_t f_sin, f_cos, f_y1, f_y2, f_yt, f_pi;std::cout << "Number.cc-6678" << endl;
					mpfr_inits2(mpfr_get_prec(i2 == 0 ? f_mid1 : f_mid2), f_x, f_xi, f_y, NULL);std::cout << "Number.cc-6679" << endl;
					mpfr_set(f_x, i2 == 0 ? f_mid1 : f_mid2, MPFR_RNDN);std::cout << "Number.cc-6680" << endl;
					mpfr_inits2(mpfr_get_prec(f_x), f_pi, f_sin, f_cos, f_y1, f_y2, f_yt, NULL);std::cout << "Number.cc-6681" << endl;
					mpfr_const_pi(f_pi, MPFR_RNDN);std::cout << "Number.cc-6682" << endl;
					mpfr_div_ui(f_pi, f_pi, 2, MPFR_RNDN);std::cout << "Number.cc-6683" << endl;
					mpfr_set(f_cos, f_x, MPFR_RNDN);std::cout << "Number.cc-6684" << endl;
					mpfr_set(f_sin, f_x, MPFR_RNDN);std::cout << "Number.cc-6685" << endl;
					mpfr_cos(f_cos, f_cos, MPFR_RNDN);std::cout << "Number.cc-6686" << endl;
					mpfr_div(f_cos, f_cos, f_x, MPFR_RNDN);std::cout << "Number.cc-6687" << endl;
					mpfr_sin(f_sin, f_sin, MPFR_RNDN);std::cout << "Number.cc-6688" << endl;
					mpfr_div(f_sin, f_sin, f_x, MPFR_RNDN);std::cout << "Number.cc-6689" << endl;
					mpfr_set_ui(f_y1, 1, MPFR_RNDN);std::cout << "Number.cc-6690" << endl;
					mpfr_ui_div(f_y2, 1, f_x, MPFR_RNDN);std::cout << "Number.cc-6691" << endl;
					mpz_set_ui(z_i, 1);std::cout << "Number.cc-6692" << endl;
					mpz_set_ui(z_fac, 1);std::cout << "Number.cc-6693" << endl;
					for(size_t i = 0; i < 100; i++) {
						if(CALCULATOR->aborted()) {set(nr_bak); return false;}
						mpz_add_ui(z_i, z_i, 1);std::cout << "Number.cc-6696" << endl;
						mpz_mul(z_fac, z_fac, z_i);std::cout << "Number.cc-6697" << endl;
						mpfr_set(f_xi, f_x, MPFR_RNDN);std::cout << "Number.cc-6698" << endl;
						mpfr_pow_z(f_xi, f_xi, z_i, MPFR_RNDN);std::cout << "Number.cc-6699" << endl;
						mpfr_ui_div(f_xi, 1, f_xi, MPFR_RNDN);std::cout << "Number.cc-6700" << endl;
						mpfr_mul_z(f_xi, f_xi, z_fac, MPFR_RNDN);std::cout << "Number.cc-6701" << endl;
						if(i % 2 == 0) mpfr_sub(f_y1, f_y1, f_xi, MPFR_RNDN);
						else mpfr_add(f_y1, f_y1, f_xi, MPFR_RNDN);std::cout << "Number.cc-6703" << endl;
						mpz_add_ui(z_i, z_i, 1);std::cout << "Number.cc-6704" << endl;
						mpz_mul(z_fac, z_fac, z_i);std::cout << "Number.cc-6705" << endl;
						mpfr_set(f_xi, f_x, MPFR_RNDN);std::cout << "Number.cc-6706" << endl;
						mpfr_pow_z(f_xi, f_xi, z_i, MPFR_RNDN);std::cout << "Number.cc-6707" << endl;
						mpfr_ui_div(f_xi, 1, f_xi, MPFR_RNDN);std::cout << "Number.cc-6708" << endl;
						mpfr_mul_z(f_xi, f_xi, z_fac, MPFR_RNDN);std::cout << "Number.cc-6709" << endl;
						if(i % 2 == 0) mpfr_sub(f_y2, f_y2, f_xi, MPFR_RNDN);
						else mpfr_add(f_y2, f_y2, f_xi, MPFR_RNDN);std::cout << "Number.cc-6711" << endl;
						mpfr_mul(f_yt, f_y1, f_cos, MPFR_RNDN);std::cout << "Number.cc-6712" << endl;
						mpfr_sub(f_y, f_pi, f_yt, MPFR_RNDN);std::cout << "Number.cc-6713" << endl;
						mpfr_mul(f_yt, f_y2, f_sin, MPFR_RNDN);std::cout << "Number.cc-6714" << endl;
						mpfr_sub(f_y, f_y, f_yt, MPFR_RNDN);std::cout << "Number.cc-6715" << endl;
						mpfr_set(fl_value, f_y, CREATE_INTERVAL ? MPFR_RNDD : MPFR_RNDN);std::cout << "Number.cc-6716" << endl;
						if(i > 0 && mpfr_equal_p(fu_value, fl_value)) {b = true; break;}
						mpfr_set(fu_value, f_y, CREATE_INTERVAL ? MPFR_RNDD : MPFR_RNDN);std::cout << "Number.cc-6718" << endl;
					}
					mpfr_clears(f_sin, f_cos, f_y1, f_y2, f_yt, f_pi, NULL);std::cout << "Number.cc-6720" << endl;
				} else {
					mpfr_inits2(mpfr_get_prec(i2 == 0 ? f_mid1 : f_mid2), f_x, f_xi, f_y, NULL);std::cout << "Number.cc-6722" << endl;
					mpfr_set(f_x, i2 == 0 ? f_mid1 : f_mid2, MPFR_RNDN);std::cout << "Number.cc-6723" << endl;
					mpfr_set(f_y, i2 == 0 ? f_mid1 : f_mid2, MPFR_RNDN);std::cout << "Number.cc-6724" << endl;
					mpz_set_ui(z_i, 1);std::cout << "Number.cc-6725" << endl;
					mpz_set_ui(z_fac, 1);std::cout << "Number.cc-6726" << endl;
					for(size_t i = 0; i < 10000; i++) {
						if(CALCULATOR->aborted()) {set(nr_bak); return false;}
						mpz_add_ui(z_i, z_i, 1);std::cout << "Number.cc-6729" << endl;
						mpz_mul(z_fac, z_fac, z_i);std::cout << "Number.cc-6730" << endl;
						mpz_add_ui(z_i, z_i, 1);std::cout << "Number.cc-6731" << endl;
						mpz_mul(z_fac, z_fac, z_i);std::cout << "Number.cc-6732" << endl;
						mpz_mul(z_fac, z_fac, z_i);std::cout << "Number.cc-6733" << endl;
						mpfr_set(f_xi, f_x, MPFR_RNDN);std::cout << "Number.cc-6734" << endl;
						mpfr_pow_z(f_xi, f_xi, z_i, MPFR_RNDN);std::cout << "Number.cc-6735" << endl;
						mpfr_div_z(f_xi, f_xi, z_fac, MPFR_RNDN);std::cout << "Number.cc-6736" << endl;
						mpz_divexact(z_fac, z_fac, z_i);std::cout << "Number.cc-6737" << endl;
						if(i % 2 == 0) mpfr_sub(f_y, f_y, f_xi, MPFR_RNDN);
						else mpfr_add(f_y, f_y, f_xi, MPFR_RNDN);std::cout << "Number.cc-6739" << endl;
						mpfr_set(fl_value, f_y, CREATE_INTERVAL ? MPFR_RNDD : MPFR_RNDN);std::cout << "Number.cc-6740" << endl;
						if(i > 0 && mpfr_equal_p(fu_value, fl_value)) {b = true; break;}
						mpfr_set(fu_value, f_y, CREATE_INTERVAL ? MPFR_RNDD : MPFR_RNDN);std::cout << "Number.cc-6742" << endl;
					}
				}
				if(b) {
					if(i2 == 0) {
						nr_mid1.setInternal(fl_value);std::cout << "Number.cc-6747" << endl;
						mpfr_set(nr_mid1.internalUpperFloat(), f_y, MPFR_RNDU);std::cout << "Number.cc-6748" << endl;
						if(!nr_mid1.testFloatResult()) b = false;
					} else {
						nr_mid2.setInternal(fl_value);std::cout << "Number.cc-6751" << endl;
						mpfr_set(nr_mid2.internalUpperFloat(), f_y, MPFR_RNDU);std::cout << "Number.cc-6752" << endl;
						if(!nr_mid2.testFloatResult()) b = false;
					}
				}
				mpfr_clears(f_x, f_xi, f_y, NULL);std::cout << "Number.cc-6756" << endl;
				mpz_clears(z_i, z_fac, NULL);std::cout << "Number.cc-6757" << endl;
			}
			mpfr_clears(f_pi, f_mid1, f_mid2, NULL);std::cout << "Number.cc-6759" << endl;
		}
		if(!b || !nr_lower.sinint() || !nr_upper.sinint()) {
			set(nr_bak);std::cout << "Number.cc-6762" << endl;
			return false;
		}
		setInterval(nr_lower, nr_upper);std::cout << "Number.cc-6765" << endl;
		if(b1) setInterval(*this, nr_mid1);
		if(b2) setInterval(*this, nr_mid2);
		return true;
	}
	mpfr_clear_flags();std::cout << "Number.cc-6770" << endl;
	mpfr_t f_x, f_xi, f_y;std::cout << "Number.cc-6771" << endl;
	mpz_t z_i, z_fac;std::cout << "Number.cc-6772" << endl;
	mpz_inits(z_i, z_fac, NULL);std::cout << "Number.cc-6773" << endl;
	bool b = false;std::cout << "Number.cc-6774" << endl;
	if(nr_round > 100 + PRECISION * 5) {
		mpfr_t f_sin, f_cos, f_y1, f_y2, f_yt, f_pi;std::cout << "Number.cc-6776" << endl;
		mpfr_inits2(mpfr_get_prec(fl_value) + 10, f_x, f_xi, f_y, NULL);std::cout << "Number.cc-6777" << endl;
		mpfr_set(f_x, fl_value, MPFR_RNDN);std::cout << "Number.cc-6778" << endl;
		mpfr_inits2(mpfr_get_prec(f_x), f_pi, f_sin, f_cos, f_y1, f_y2, f_yt, NULL);std::cout << "Number.cc-6779" << endl;
		mpfr_const_pi(f_pi, MPFR_RNDN);std::cout << "Number.cc-6780" << endl;
		mpfr_div_ui(f_pi, f_pi, 2, MPFR_RNDN);std::cout << "Number.cc-6781" << endl;
		mpfr_set(f_cos, f_x, MPFR_RNDN);std::cout << "Number.cc-6782" << endl;
		mpfr_set(f_sin, f_x, MPFR_RNDN);std::cout << "Number.cc-6783" << endl;
		mpfr_cos(f_cos, f_cos, MPFR_RNDN);std::cout << "Number.cc-6784" << endl;
		mpfr_div(f_cos, f_cos, f_x, MPFR_RNDN);std::cout << "Number.cc-6785" << endl;
		mpfr_sin(f_sin, f_sin, MPFR_RNDN);std::cout << "Number.cc-6786" << endl;
		mpfr_div(f_sin, f_sin, f_x, MPFR_RNDN);std::cout << "Number.cc-6787" << endl;
		mpfr_set_ui(f_y1, 1, MPFR_RNDN);std::cout << "Number.cc-6788" << endl;
		mpfr_ui_div(f_y2, 1, f_x, MPFR_RNDN);std::cout << "Number.cc-6789" << endl;
		mpz_set_ui(z_i, 1);std::cout << "Number.cc-6790" << endl;
		mpz_set_ui(z_fac, 1);std::cout << "Number.cc-6791" << endl;
		for(size_t i = 0; i < 100; i++) {
			if(CALCULATOR->aborted()) {set(nr_bak); return false;}
			mpz_add_ui(z_i, z_i, 1);std::cout << "Number.cc-6794" << endl;
			mpz_mul(z_fac, z_fac, z_i);std::cout << "Number.cc-6795" << endl;
			mpfr_set(f_xi, f_x, MPFR_RNDN);std::cout << "Number.cc-6796" << endl;
			mpfr_pow_z(f_xi, f_xi, z_i, MPFR_RNDN);std::cout << "Number.cc-6797" << endl;
			mpfr_ui_div(f_xi, 1, f_xi, MPFR_RNDN);std::cout << "Number.cc-6798" << endl;
			mpfr_mul_z(f_xi, f_xi, z_fac, MPFR_RNDN);std::cout << "Number.cc-6799" << endl;
			if(i % 2 == 0) mpfr_sub(f_y1, f_y1, f_xi, MPFR_RNDN);
			else mpfr_add(f_y1, f_y1, f_xi, MPFR_RNDN);std::cout << "Number.cc-6801" << endl;
			mpz_add_ui(z_i, z_i, 1);std::cout << "Number.cc-6802" << endl;
			mpz_mul(z_fac, z_fac, z_i);std::cout << "Number.cc-6803" << endl;
			mpfr_set(f_xi, f_x, MPFR_RNDN);std::cout << "Number.cc-6804" << endl;
			mpfr_pow_z(f_xi, f_xi, z_i, MPFR_RNDN);std::cout << "Number.cc-6805" << endl;
			mpfr_ui_div(f_xi, 1, f_xi, MPFR_RNDN);std::cout << "Number.cc-6806" << endl;
			mpfr_mul_z(f_xi, f_xi, z_fac, MPFR_RNDN);std::cout << "Number.cc-6807" << endl;
			if(i % 2 == 0) mpfr_sub(f_y2, f_y2, f_xi, MPFR_RNDN);
			else mpfr_add(f_y2, f_y2, f_xi, MPFR_RNDN);std::cout << "Number.cc-6809" << endl;
			mpfr_mul(f_yt, f_y1, f_cos, MPFR_RNDN);std::cout << "Number.cc-6810" << endl;
			mpfr_sub(f_y, f_pi, f_yt, MPFR_RNDN);std::cout << "Number.cc-6811" << endl;
			mpfr_mul(f_yt, f_y2, f_sin, MPFR_RNDN);std::cout << "Number.cc-6812" << endl;
			mpfr_sub(f_y, f_y, f_yt, MPFR_RNDN);std::cout << "Number.cc-6813" << endl;
			mpfr_set(fl_value, f_y, CREATE_INTERVAL ? MPFR_RNDD : MPFR_RNDN);std::cout << "Number.cc-6814" << endl;
			if(i > 0 && mpfr_equal_p(fu_value, fl_value)) {b = true; break;}
			mpfr_set(fu_value, f_y, CREATE_INTERVAL ? MPFR_RNDD : MPFR_RNDN);std::cout << "Number.cc-6816" << endl;
		}
		mpfr_clears(f_sin, f_cos, f_y1, f_y2, f_yt, f_pi, NULL);std::cout << "Number.cc-6818" << endl;
	} else {
		mpfr_inits2(mpfr_get_prec(fl_value) + nr_round.intValue() * 5, f_x, f_xi, f_y, NULL);std::cout << "Number.cc-6820" << endl;
		mpfr_set(f_x, fl_value, MPFR_RNDN);std::cout << "Number.cc-6821" << endl;
		mpfr_set(f_y, fl_value, MPFR_RNDN);std::cout << "Number.cc-6822" << endl;
		mpz_set_ui(z_i, 1);std::cout << "Number.cc-6823" << endl;
		mpz_set_ui(z_fac, 1);std::cout << "Number.cc-6824" << endl;
		for(size_t i = 0; i < 10000; i++) {
			if(CALCULATOR->aborted()) {set(nr_bak); return false;}
			mpz_add_ui(z_i, z_i, 1);std::cout << "Number.cc-6827" << endl;
			mpz_mul(z_fac, z_fac, z_i);std::cout << "Number.cc-6828" << endl;
			mpz_add_ui(z_i, z_i, 1);std::cout << "Number.cc-6829" << endl;
			mpz_mul(z_fac, z_fac, z_i);std::cout << "Number.cc-6830" << endl;
			mpz_mul(z_fac, z_fac, z_i);std::cout << "Number.cc-6831" << endl;
			mpfr_set(f_xi, f_x, MPFR_RNDN);std::cout << "Number.cc-6832" << endl;
			mpfr_pow_z(f_xi, f_xi, z_i, MPFR_RNDN);std::cout << "Number.cc-6833" << endl;
			mpfr_div_z(f_xi, f_xi, z_fac, MPFR_RNDN);std::cout << "Number.cc-6834" << endl;
			mpz_divexact(z_fac, z_fac, z_i);std::cout << "Number.cc-6835" << endl;
			if(i % 2 == 0) mpfr_sub(f_y, f_y, f_xi, MPFR_RNDN);
			else mpfr_add(f_y, f_y, f_xi, MPFR_RNDN);std::cout << "Number.cc-6837" << endl;
			mpfr_set(fl_value, f_y, CREATE_INTERVAL ? MPFR_RNDD : MPFR_RNDN);std::cout << "Number.cc-6838" << endl;
			if(i > 0 && mpfr_equal_p(fu_value, fl_value)) {b = true; break;}
			mpfr_set(fu_value, f_y, CREATE_INTERVAL ? MPFR_RNDD : MPFR_RNDN);std::cout << "Number.cc-6840" << endl;
		}
	}
	if(!b) {
		set(nr_bak);std::cout << "Number.cc-6844" << endl;
		return false;
	}
	if(CREATE_INTERVAL) mpfr_set(fu_value, f_y, MPFR_RNDU);
	else mpfr_set(fu_value, fl_value, MPFR_RNDN);std::cout << "Number.cc-6848" << endl;
	mpfr_clears(f_x, f_xi, f_y, NULL);std::cout << "Number.cc-6849" << endl;
	mpz_clears(z_i, z_fac, NULL);std::cout << "Number.cc-6850" << endl;
	if(!testFloatResult()) {
		set(nr_bak);std::cout << "Number.cc-6852" << endl;
		return false;
	}
	b_approx = true;std::cout << "Number.cc-6855" << endl;
	return true;
}
bool Number::sinhint() {
	if(isPlusInfinity()) {return true;}
	if(isMinusInfinity()) {return true;}
	if(isZero()) return true;
	if(hasImaginaryPart()) {
		if(hasRealPart()) return false;
		if(!i_value->sinhint()) return false;
		setPrecisionAndApproximateFrom(*i_value);std::cout << "Number.cc-6865" << endl;
		return true;
	}
	Number nr_bak(*this);std::cout << "Number.cc-6868" << endl;
	if(isNegative()) {
		if(!negate() || !sinhint() || !negate()) {set(nr_bak); return false;}
		return true;
	}
	if(isGreaterThan(1000)) return false;
	if(!setToFloatingPoint()) return false;
	if(isInterval()) {
		Number nr_lower(lowerEndPoint());std::cout << "Number.cc-6876" << endl;
		Number nr_upper(upperEndPoint());std::cout << "Number.cc-6877" << endl;
		if(!nr_lower.sinhint() || !nr_upper.sinhint()) {
			set(nr_bak);std::cout << "Number.cc-6879" << endl;
			return false;
		}
		setInterval(nr_lower, nr_upper);std::cout << "Number.cc-6882" << endl;
		return true;
	}
	mpfr_clear_flags();std::cout << "Number.cc-6885" << endl;
	mpfr_t f_x, f_xi, f_y;std::cout << "Number.cc-6886" << endl;
	mpz_t z_i, z_fac;std::cout << "Number.cc-6887" << endl;
	mpz_inits(z_i, z_fac, NULL);std::cout << "Number.cc-6888" << endl;
	Number nr_round(*this);std::cout << "Number.cc-6889" << endl;
	nr_round.round();std::cout << "Number.cc-6890" << endl;
	mpfr_inits2(mpfr_get_prec(fl_value) + nr_round.intValue(), f_x, f_xi, f_y, NULL);std::cout << "Number.cc-6891" << endl;
	mpfr_set(f_x, fl_value, MPFR_RNDN);std::cout << "Number.cc-6892" << endl;
	mpfr_set(f_y, fl_value, MPFR_RNDN);std::cout << "Number.cc-6893" << endl;
	mpz_set_ui(z_i, 1);std::cout << "Number.cc-6894" << endl;
	mpz_set_ui(z_fac, 1);std::cout << "Number.cc-6895" << endl;
	bool b = false;std::cout << "Number.cc-6896" << endl;
	for(size_t i = 0; i < 10000; i++) {
		if(CALCULATOR->aborted()) {set(nr_bak); return false;}
		mpz_add_ui(z_i, z_i, 1);std::cout << "Number.cc-6899" << endl;
		mpz_mul(z_fac, z_fac, z_i);std::cout << "Number.cc-6900" << endl;
		mpz_add_ui(z_i, z_i, 1);std::cout << "Number.cc-6901" << endl;
		mpz_mul(z_fac, z_fac, z_i);std::cout << "Number.cc-6902" << endl;
		mpz_mul(z_fac, z_fac, z_i);std::cout << "Number.cc-6903" << endl;
		mpfr_set(f_xi, f_x, MPFR_RNDN);std::cout << "Number.cc-6904" << endl;
		mpfr_pow_z(f_xi, f_xi, z_i, MPFR_RNDN);std::cout << "Number.cc-6905" << endl;
		mpfr_div_z(f_xi, f_xi, z_fac, MPFR_RNDN);std::cout << "Number.cc-6906" << endl;
		mpz_divexact(z_fac, z_fac, z_i);std::cout << "Number.cc-6907" << endl;
		mpfr_add(f_y, f_y, f_xi, MPFR_RNDN);std::cout << "Number.cc-6908" << endl;
		mpfr_set(fl_value, f_y, CREATE_INTERVAL ? MPFR_RNDD : MPFR_RNDN);std::cout << "Number.cc-6909" << endl;
		if(i > 0 && mpfr_equal_p(fu_value, fl_value)) {b = true; break;}
		mpfr_set(fu_value, f_y, CREATE_INTERVAL ? MPFR_RNDD : MPFR_RNDN);std::cout << "Number.cc-6911" << endl;
	}
	if(!b) {
		set(nr_bak);std::cout << "Number.cc-6914" << endl;
		return false;
	}
	if(CREATE_INTERVAL) mpfr_set(fu_value, f_y, MPFR_RNDU);
	else mpfr_set(fu_value, fl_value, MPFR_RNDN);std::cout << "Number.cc-6918" << endl;
	mpfr_clears(f_x, f_xi, f_y, NULL);std::cout << "Number.cc-6919" << endl;
	mpz_clears(z_i, z_fac, NULL);std::cout << "Number.cc-6920" << endl;
	if(!testFloatResult()) {
		set(nr_bak);std::cout << "Number.cc-6922" << endl;
		return false;
	}
	b_approx = true;std::cout << "Number.cc-6925" << endl;
	return true;
}
bool Number::cosint() {
	if(isPlusInfinity()) {clear(true); return true;}
	if(isZero()) {setMinusInfinity(true); return true;}
	if(hasImaginaryPart() && !hasRealPart()) {
		if(i_value->isNegative()) {
			set(*i_value, true);std::cout << "Number.cc-6933" << endl;
			negate();std::cout << "Number.cc-6934" << endl;
			if(!coshint()) return false;
			Number nr_i;std::cout << "Number.cc-6936" << endl;
			nr_i.pi();std::cout << "Number.cc-6937" << endl;
			nr_i.divide(-2);std::cout << "Number.cc-6938" << endl;
			setImaginaryPart(nr_i);std::cout << "Number.cc-6939" << endl;
			return true;
		} else if(i_value->isPositive()) {
			set(*i_value, true);std::cout << "Number.cc-6942" << endl;
			if(!coshint()) return false;
			Number nr_i;std::cout << "Number.cc-6944" << endl;
			nr_i.pi();std::cout << "Number.cc-6945" << endl;
			nr_i.divide(2);std::cout << "Number.cc-6946" << endl;
			setImaginaryPart(nr_i);std::cout << "Number.cc-6947" << endl;
			return true;
		} else {
			set(*i_value, true);std::cout << "Number.cc-6950" << endl;
			if(!coshint()) return false;
			Number nr_i;std::cout << "Number.cc-6952" << endl;
			nr_i.pi();std::cout << "Number.cc-6953" << endl;
			nr_i.divide(2);std::cout << "Number.cc-6954" << endl;
			Number nr_low(nr_i);std::cout << "Number.cc-6955" << endl;
			nr_low.negate();std::cout << "Number.cc-6956" << endl;
			nr_i.setInterval(nr_low, nr_i);std::cout << "Number.cc-6957" << endl;
			setImaginaryPart(nr_i);std::cout << "Number.cc-6958" << endl;
			return true;
		}
	}
	if(!isReal()) return false;
	Number nr_bak(*this);std::cout << "Number.cc-6963" << endl;
	if(isNegative()) {
		if(!negate() || !cosint() || !negate()) {set(nr_bak); return false;}
		if(!i_value) {i_value = new Number(); i_value->markAsImaginaryPart();}
		i_value->pi();std::cout << "Number.cc-6967" << endl;
		setPrecisionAndApproximateFrom(*i_value);std::cout << "Number.cc-6968" << endl;
		return true;
	}
	if(!setToFloatingPoint()) return false;
	Number nr_round(*this);std::cout << "Number.cc-6972" << endl;
	nr_round.round();std::cout << "Number.cc-6973" << endl;
	if(isInterval()) {
		Number nr_lower(lowerEndPoint());std::cout << "Number.cc-6975" << endl;
		Number nr_upper(upperEndPoint());std::cout << "Number.cc-6976" << endl;
		if(nr_lower.isNegative() && nr_upper.isPositive()) {
			nr_lower.setInterval(nr_lower, nr_zero);std::cout << "Number.cc-6978" << endl;
			nr_upper.setInterval(nr_zero, nr_upper);std::cout << "Number.cc-6979" << endl;
			if(!nr_lower.cosint() || !nr_upper.cosint()) {
				set(nr_bak);std::cout << "Number.cc-6981" << endl;
				return false;
			}
			setInterval(nr_lower, nr_upper);std::cout << "Number.cc-6984" << endl;
			return true;
		}
		Number nr_mid1;std::cout << "Number.cc-6987" << endl;
		Number nr_mid2;std::cout << "Number.cc-6988" << endl;
		bool b1 = false, b2 = false, b = true;std::cout << "Number.cc-6989" << endl;
		if(mpfr_cmp_si(fl_value, 1) > 0) {
			mpfr_t f_pi, f_mid1, f_mid2;std::cout << "Number.cc-6991" << endl;
			if(mpfr_get_exp(fl_value) > 100000) {
				set(nr_bak);std::cout << "Number.cc-6993" << endl;
				return false;
			}
			mpfr_prec_t prec = mpfr_get_prec(fl_value) + mpfr_get_exp(fl_value);std::cout << "Number.cc-6996" << endl;
			if(nr_round > 100 + PRECISION * 5) prec += nr_round.intValue() * 5;
			else prec += 10;std::cout << "Number.cc-6998" << endl;
			mpfr_inits2(mpfr_get_prec(fl_value) + mpfr_get_exp(fl_value), f_pi, f_mid1, f_mid2, NULL);std::cout << "Number.cc-6999" << endl;
			mpfr_const_pi(f_pi, MPFR_RNDN);std::cout << "Number.cc-7000" << endl;
			mpfr_div(f_mid1, nr_lower.internalLowerFloat(), f_pi, MPFR_RNDN);std::cout << "Number.cc-7001" << endl;
			mpfr_floor(f_mid1, f_mid1);std::cout << "Number.cc-7002" << endl;
			mpfr_mul_ui(f_mid1, f_mid1, 2, MPFR_RNDN);std::cout << "Number.cc-7003" << endl;
			mpfr_add_ui(f_mid1, f_mid1, 1, MPFR_RNDN);std::cout << "Number.cc-7004" << endl;
			mpfr_div_ui(f_mid1, f_mid1, 2, MPFR_RNDN);std::cout << "Number.cc-7005" << endl;
			mpfr_add_ui(f_mid2, f_mid1, 1, MPFR_RNDN);std::cout << "Number.cc-7006" << endl;
			mpfr_mul(f_mid1, f_mid1, f_pi, MPFR_RNDN);std::cout << "Number.cc-7007" << endl;
			if(mpfr_cmp(f_mid1, nr_lower.internalLowerFloat()) < 0) {
				mpfr_add(f_mid1, f_mid1, f_pi, MPFR_RNDN);std::cout << "Number.cc-7009" << endl;
				mpfr_add_ui(f_mid2, f_mid2, 1, MPFR_RNDN);std::cout << "Number.cc-7010" << endl;
			}
			if(mpfr_cmp(f_mid1, nr_upper.internalLowerFloat()) < 0) {
				mpfr_mul(f_mid2, f_mid2, f_pi, MPFR_RNDN);std::cout << "Number.cc-7013" << endl;
				b1 = true;std::cout << "Number.cc-7014" << endl;
				b2 = mpfr_cmp(f_mid2, nr_upper.internalLowerFloat()) < 0;std::cout << "Number.cc-7015" << endl;
			}
			mpfr_clear_flags();std::cout << "Number.cc-7017" << endl;
			for(size_t i2 = 0; b && ((i2 == 0 && b1) || (i2 == 1 && b2)); i2++) {
				b = false;std::cout << "Number.cc-7019" << endl;
				mpfr_t f_x, f_xi, f_y;std::cout << "Number.cc-7020" << endl;
				mpz_t z_i, z_fac;std::cout << "Number.cc-7021" << endl;
				mpz_inits(z_i, z_fac, NULL);std::cout << "Number.cc-7022" << endl;
				if(nr_round > 100 + PRECISION * 5) {
					mpfr_t f_sin, f_cos, f_y1, f_y2, f_yt;std::cout << "Number.cc-7024" << endl;
					mpfr_inits2(mpfr_get_prec(i2 == 0 ? f_mid1 : f_mid2), f_x, f_xi, f_y, NULL);std::cout << "Number.cc-7025" << endl;
					mpfr_set(f_x, i2 == 0 ? f_mid1 : f_mid2, MPFR_RNDN);std::cout << "Number.cc-7026" << endl;
					mpfr_inits2(mpfr_get_prec(f_x), f_sin, f_cos, f_y1, f_y2, f_yt, NULL);std::cout << "Number.cc-7027" << endl;
					mpfr_set(f_cos, f_x, MPFR_RNDN);std::cout << "Number.cc-7028" << endl;
					mpfr_set(f_sin, f_x, MPFR_RNDN);std::cout << "Number.cc-7029" << endl;
					mpfr_cos(f_cos, f_cos, MPFR_RNDN);std::cout << "Number.cc-7030" << endl;
					mpfr_div(f_cos, f_cos, f_x, MPFR_RNDN);std::cout << "Number.cc-7031" << endl;
					mpfr_sin(f_sin, f_sin, MPFR_RNDN);std::cout << "Number.cc-7032" << endl;
					mpfr_div(f_sin, f_sin, f_x, MPFR_RNDN);std::cout << "Number.cc-7033" << endl;
					mpfr_set_ui(f_y1, 1, MPFR_RNDN);std::cout << "Number.cc-7034" << endl;
					mpfr_ui_div(f_y2, 1, f_x, MPFR_RNDN);std::cout << "Number.cc-7035" << endl;
					mpz_set_ui(z_i, 1);std::cout << "Number.cc-7036" << endl;
					mpz_set_ui(z_fac, 1);std::cout << "Number.cc-7037" << endl;
					for(size_t i = 0; i < 100; i++) {
						if(CALCULATOR->aborted()) {set(nr_bak); return false;}
						mpz_add_ui(z_i, z_i, 1);std::cout << "Number.cc-7040" << endl;
						mpz_mul(z_fac, z_fac, z_i);std::cout << "Number.cc-7041" << endl;
						mpfr_set(f_xi, f_x, MPFR_RNDN);std::cout << "Number.cc-7042" << endl;
						mpfr_pow_z(f_xi, f_xi, z_i, MPFR_RNDN);std::cout << "Number.cc-7043" << endl;
						mpfr_ui_div(f_xi, 1, f_xi, MPFR_RNDN);std::cout << "Number.cc-7044" << endl;
						mpfr_mul_z(f_xi, f_xi, z_fac, MPFR_RNDN);std::cout << "Number.cc-7045" << endl;
						if(i % 2 == 0) mpfr_sub(f_y1, f_y1, f_xi, MPFR_RNDN);
						else mpfr_add(f_y1, f_y1, f_xi, MPFR_RNDN);std::cout << "Number.cc-7047" << endl;
						mpz_add_ui(z_i, z_i, 1);std::cout << "Number.cc-7048" << endl;
						mpz_mul(z_fac, z_fac, z_i);std::cout << "Number.cc-7049" << endl;
						mpfr_set(f_xi, f_x, MPFR_RNDN);std::cout << "Number.cc-7050" << endl;
						mpfr_pow_z(f_xi, f_xi, z_i, MPFR_RNDN);std::cout << "Number.cc-7051" << endl;
						mpfr_ui_div(f_xi, 1, f_xi, MPFR_RNDN);std::cout << "Number.cc-7052" << endl;
						mpfr_mul_z(f_xi, f_xi, z_fac, MPFR_RNDN);std::cout << "Number.cc-7053" << endl;
						if(i % 2 == 0) mpfr_sub(f_y2, f_y2, f_xi, MPFR_RNDN);
						else mpfr_add(f_y2, f_y2, f_xi, MPFR_RNDN);std::cout << "Number.cc-7055" << endl;
						mpfr_mul(f_y, f_y1, f_sin, MPFR_RNDN);std::cout << "Number.cc-7056" << endl;
						mpfr_mul(f_yt, f_y2, f_cos, MPFR_RNDN);std::cout << "Number.cc-7057" << endl;
						mpfr_sub(f_y, f_y, f_yt, MPFR_RNDN);std::cout << "Number.cc-7058" << endl;
						mpfr_set(fl_value, f_y, CREATE_INTERVAL ? MPFR_RNDD : MPFR_RNDN);std::cout << "Number.cc-7059" << endl;
						if(i > 0 && mpfr_equal_p(fu_value, fl_value)) {b = true; break;}
						mpfr_set(fu_value, f_y, CREATE_INTERVAL ? MPFR_RNDD : MPFR_RNDN);std::cout << "Number.cc-7061" << endl;
					}
					mpfr_clears(f_sin, f_cos, f_y1, f_y2, f_yt, NULL);std::cout << "Number.cc-7063" << endl;
				} else {
					mpfr_t f_euler;std::cout << "Number.cc-7065" << endl;
					mpfr_inits2(mpfr_get_prec(i2 == 0 ? f_mid1 : f_mid2), f_x, f_xi, f_y, f_euler, NULL);std::cout << "Number.cc-7066" << endl;
					mpfr_set(f_x, i2 == 0 ? f_mid1 : f_mid2, MPFR_RNDN);std::cout << "Number.cc-7067" << endl;
					mpfr_const_euler(f_euler, MPFR_RNDN);std::cout << "Number.cc-7068" << endl;
					mpfr_set(f_y, i2 == 0 ? f_mid1 : f_mid2, MPFR_RNDN);std::cout << "Number.cc-7069" << endl;
					mpfr_log(f_y, f_y, MPFR_RNDN);std::cout << "Number.cc-7070" << endl;
					mpfr_add(f_y, f_y, f_euler, MPFR_RNDN);std::cout << "Number.cc-7071" << endl;
					mpz_set_ui(z_i, 0);std::cout << "Number.cc-7072" << endl;
					mpz_set_ui(z_fac, 1);std::cout << "Number.cc-7073" << endl;
					for(size_t i = 0; i < 10000; i++) {
						if(CALCULATOR->aborted()) {set(nr_bak); return false;}
						mpz_add_ui(z_i, z_i, 1);std::cout << "Number.cc-7076" << endl;
						mpz_mul(z_fac, z_fac, z_i);std::cout << "Number.cc-7077" << endl;
						mpz_add_ui(z_i, z_i, 1);std::cout << "Number.cc-7078" << endl;
						mpz_mul(z_fac, z_fac, z_i);std::cout << "Number.cc-7079" << endl;
						mpz_mul(z_fac, z_fac, z_i);std::cout << "Number.cc-7080" << endl;
						mpfr_set(f_xi, f_x, MPFR_RNDN);std::cout << "Number.cc-7081" << endl;
						mpfr_pow_z(f_xi, f_xi, z_i, MPFR_RNDN);std::cout << "Number.cc-7082" << endl;
						mpfr_div_z(f_xi, f_xi, z_fac, MPFR_RNDN);std::cout << "Number.cc-7083" << endl;
						mpz_divexact(z_fac, z_fac, z_i);std::cout << "Number.cc-7084" << endl;
						if(i % 2 == 0) mpfr_sub(f_y, f_y, f_xi, MPFR_RNDN);
						else mpfr_add(f_y, f_y, f_xi, MPFR_RNDN);std::cout << "Number.cc-7086" << endl;
						mpfr_set(fl_value, f_y, CREATE_INTERVAL ? MPFR_RNDD : MPFR_RNDN);std::cout << "Number.cc-7087" << endl;
						if(i > 0 && mpfr_equal_p(fu_value, fl_value)) {b = true; break;}
						mpfr_set(fu_value, f_y, CREATE_INTERVAL ? MPFR_RNDD : MPFR_RNDN);std::cout << "Number.cc-7089" << endl;
					}
					mpfr_clear(f_euler);std::cout << "Number.cc-7091" << endl;
				}
				if(b) {
					if(i2 == 0) {
						nr_mid1.setInternal(fl_value);std::cout << "Number.cc-7095" << endl;
						mpfr_set(nr_mid1.internalUpperFloat(), f_y, MPFR_RNDU);std::cout << "Number.cc-7096" << endl;
						if(!nr_mid1.testFloatResult()) b = false;
					} else {
						nr_mid2.setInternal(fl_value);std::cout << "Number.cc-7099" << endl;
						mpfr_set(nr_mid2.internalUpperFloat(), f_y, MPFR_RNDU);std::cout << "Number.cc-7100" << endl;
						if(!nr_mid2.testFloatResult()) b = false;
					}
				}
				mpfr_clears(f_x, f_xi, f_y, NULL);std::cout << "Number.cc-7104" << endl;
				mpz_clears(z_i, z_fac, NULL);std::cout << "Number.cc-7105" << endl;
			}
			mpfr_clears(f_pi, f_mid1, f_mid2, NULL);std::cout << "Number.cc-7107" << endl;
		}
		if(!b || !nr_lower.cosint() || !nr_upper.cosint()) {
			set(nr_bak);std::cout << "Number.cc-7110" << endl;
			return false;
		}
		setInterval(nr_lower, nr_upper);std::cout << "Number.cc-7113" << endl;
		if(b1) setInterval(*this, nr_mid1);
		if(b2) setInterval(*this, nr_mid2);
		return true;
	}
	mpfr_clear_flags();std::cout << "Number.cc-7118" << endl;
	mpfr_t f_x, f_xi, f_y;std::cout << "Number.cc-7119" << endl;
	mpz_t z_i, z_fac;std::cout << "Number.cc-7120" << endl;
	mpz_inits(z_i, z_fac, NULL);std::cout << "Number.cc-7121" << endl;
	bool b = false;std::cout << "Number.cc-7122" << endl;
	if(nr_round > 100 + PRECISION * 5) {
		mpfr_t f_sin, f_cos, f_y1, f_y2, f_yt;std::cout << "Number.cc-7124" << endl;
		mpfr_inits2(mpfr_get_prec(fl_value) + 10, f_x, f_xi, f_y, NULL);std::cout << "Number.cc-7125" << endl;
		mpfr_set(f_x, fl_value, MPFR_RNDN);std::cout << "Number.cc-7126" << endl;
		mpfr_inits2(mpfr_get_prec(f_x), f_sin, f_cos, f_y1, f_y2, f_yt, NULL);std::cout << "Number.cc-7127" << endl;
		mpfr_set(f_cos, f_x, MPFR_RNDN);std::cout << "Number.cc-7128" << endl;
		mpfr_set(f_sin, f_x, MPFR_RNDN);std::cout << "Number.cc-7129" << endl;
		mpfr_cos(f_cos, f_cos, MPFR_RNDN);std::cout << "Number.cc-7130" << endl;
		mpfr_div(f_cos, f_cos, f_x, MPFR_RNDN);std::cout << "Number.cc-7131" << endl;
		mpfr_sin(f_sin, f_sin, MPFR_RNDN);std::cout << "Number.cc-7132" << endl;
		mpfr_div(f_sin, f_sin, f_x, MPFR_RNDN);std::cout << "Number.cc-7133" << endl;
		mpfr_set_ui(f_y1, 1, MPFR_RNDN);std::cout << "Number.cc-7134" << endl;
		mpfr_ui_div(f_y2, 1, f_x, MPFR_RNDN);std::cout << "Number.cc-7135" << endl;
		mpz_set_ui(z_i, 1);std::cout << "Number.cc-7136" << endl;
		mpz_set_ui(z_fac, 1);std::cout << "Number.cc-7137" << endl;
		for(size_t i = 0; i < 100; i++) {
			if(CALCULATOR->aborted()) {set(nr_bak); return false;}
			mpz_add_ui(z_i, z_i, 1);std::cout << "Number.cc-7140" << endl;
			mpz_mul(z_fac, z_fac, z_i);std::cout << "Number.cc-7141" << endl;
			mpfr_set(f_xi, f_x, MPFR_RNDN);std::cout << "Number.cc-7142" << endl;
			mpfr_pow_z(f_xi, f_xi, z_i, MPFR_RNDN);std::cout << "Number.cc-7143" << endl;
			mpfr_ui_div(f_xi, 1, f_xi, MPFR_RNDN);std::cout << "Number.cc-7144" << endl;
			mpfr_mul_z(f_xi, f_xi, z_fac, MPFR_RNDN);std::cout << "Number.cc-7145" << endl;
			if(i % 2 == 0) mpfr_sub(f_y1, f_y1, f_xi, MPFR_RNDN);
			else mpfr_add(f_y1, f_y1, f_xi, MPFR_RNDN);std::cout << "Number.cc-7147" << endl;
			mpz_add_ui(z_i, z_i, 1);std::cout << "Number.cc-7148" << endl;
			mpz_mul(z_fac, z_fac, z_i);std::cout << "Number.cc-7149" << endl;
			mpfr_set(f_xi, f_x, MPFR_RNDN);std::cout << "Number.cc-7150" << endl;
			mpfr_pow_z(f_xi, f_xi, z_i, MPFR_RNDN);std::cout << "Number.cc-7151" << endl;
			mpfr_ui_div(f_xi, 1, f_xi, MPFR_RNDN);std::cout << "Number.cc-7152" << endl;
			mpfr_mul_z(f_xi, f_xi, z_fac, MPFR_RNDN);std::cout << "Number.cc-7153" << endl;
			if(i % 2 == 0) mpfr_sub(f_y2, f_y2, f_xi, MPFR_RNDN);
			else mpfr_add(f_y2, f_y2, f_xi, MPFR_RNDN);std::cout << "Number.cc-7155" << endl;
			mpfr_mul(f_y, f_y1, f_sin, MPFR_RNDN);std::cout << "Number.cc-7156" << endl;
			mpfr_mul(f_yt, f_y2, f_cos, MPFR_RNDN);std::cout << "Number.cc-7157" << endl;
			mpfr_sub(f_y, f_y, f_yt, MPFR_RNDN);std::cout << "Number.cc-7158" << endl;
			mpfr_set(fl_value, f_y, CREATE_INTERVAL ? MPFR_RNDD : MPFR_RNDN);std::cout << "Number.cc-7159" << endl;
			if(i > 0 && mpfr_equal_p(fu_value, fl_value)) {b = true; break;}
			mpfr_set(fu_value, f_y, CREATE_INTERVAL ? MPFR_RNDD : MPFR_RNDN);std::cout << "Number.cc-7161" << endl;
		}
		mpfr_clears(f_sin, f_cos, f_y1, f_y2, f_yt, NULL);std::cout << "Number.cc-7163" << endl;
	} else {
		mpfr_t f_euler;std::cout << "Number.cc-7165" << endl;
		mpfr_inits2(mpfr_get_prec(fl_value) + nr_round.intValue() * 5, f_x, f_xi, f_y, f_euler, NULL);std::cout << "Number.cc-7166" << endl;
		mpfr_set(f_x, fl_value, MPFR_RNDN);std::cout << "Number.cc-7167" << endl;
		mpfr_const_euler(f_euler, MPFR_RNDN);std::cout << "Number.cc-7168" << endl;
		mpfr_set(f_y, fl_value, MPFR_RNDN);std::cout << "Number.cc-7169" << endl;
		mpfr_log(f_y, f_y, MPFR_RNDN);std::cout << "Number.cc-7170" << endl;
		mpfr_add(f_y, f_y, f_euler, MPFR_RNDN);std::cout << "Number.cc-7171" << endl;
		mpz_set_ui(z_i, 0);std::cout << "Number.cc-7172" << endl;
		mpz_set_ui(z_fac, 1);std::cout << "Number.cc-7173" << endl;
		for(size_t i = 0; i < 10000; i++) {
			if(CALCULATOR->aborted()) {set(nr_bak); return false;}
			mpz_add_ui(z_i, z_i, 1);std::cout << "Number.cc-7176" << endl;
			mpz_mul(z_fac, z_fac, z_i);std::cout << "Number.cc-7177" << endl;
			mpz_add_ui(z_i, z_i, 1);std::cout << "Number.cc-7178" << endl;
			mpz_mul(z_fac, z_fac, z_i);std::cout << "Number.cc-7179" << endl;
			mpz_mul(z_fac, z_fac, z_i);std::cout << "Number.cc-7180" << endl;
			mpfr_set(f_xi, f_x, MPFR_RNDN);std::cout << "Number.cc-7181" << endl;
			mpfr_pow_z(f_xi, f_xi, z_i, MPFR_RNDN);std::cout << "Number.cc-7182" << endl;
			mpfr_div_z(f_xi, f_xi, z_fac, MPFR_RNDN);std::cout << "Number.cc-7183" << endl;
			mpz_divexact(z_fac, z_fac, z_i);std::cout << "Number.cc-7184" << endl;
			if(i % 2 == 0) mpfr_sub(f_y, f_y, f_xi, MPFR_RNDN);
			else mpfr_add(f_y, f_y, f_xi, MPFR_RNDN);std::cout << "Number.cc-7186" << endl;
			mpfr_set(fl_value, f_y, CREATE_INTERVAL ? MPFR_RNDD : MPFR_RNDN);std::cout << "Number.cc-7187" << endl;
			if(i > 0 && mpfr_equal_p(fu_value, fl_value)) {b = true; break;}
			mpfr_set(fu_value, f_y, CREATE_INTERVAL ? MPFR_RNDD : MPFR_RNDN);std::cout << "Number.cc-7189" << endl;
		}
		mpfr_clear(f_euler);std::cout << "Number.cc-7191" << endl;
	}
	if(!b) {
		set(nr_bak);std::cout << "Number.cc-7194" << endl;
		return false;
	}
	if(CREATE_INTERVAL) mpfr_set(fu_value, f_y, MPFR_RNDU);
	else mpfr_set(fu_value, fl_value, MPFR_RNDN);std::cout << "Number.cc-7198" << endl;
	mpfr_clears(f_x, f_xi, f_y, NULL);std::cout << "Number.cc-7199" << endl;
	mpz_clears(z_i, z_fac, NULL);std::cout << "Number.cc-7200" << endl;
	if(!testFloatResult()) {
		set(nr_bak);std::cout << "Number.cc-7202" << endl;
		return false;
	}
	b_approx = true;std::cout << "Number.cc-7205" << endl;
	return true;
}
bool Number::coshint() {
	if(isPlusInfinity()) {return true;}
	if(isZero()) {setMinusInfinity(true); return true;}
	if(hasImaginaryPart() && !hasRealPart()) {
		if(i_value->isNegative()) {
			set(*i_value, true);std::cout << "Number.cc-7213" << endl;
			negate();std::cout << "Number.cc-7214" << endl;
			if(!cosint()) return false;
			Number nr_i;std::cout << "Number.cc-7216" << endl;
			nr_i.pi();std::cout << "Number.cc-7217" << endl;
			nr_i.divide(-2);std::cout << "Number.cc-7218" << endl;
			setImaginaryPart(nr_i);std::cout << "Number.cc-7219" << endl;
			return true;
		} else if(i_value->isPositive()) {
			set(*i_value, true);std::cout << "Number.cc-7222" << endl;
			if(!cosint()) return false;
			Number nr_i;std::cout << "Number.cc-7224" << endl;
			nr_i.pi();std::cout << "Number.cc-7225" << endl;
			nr_i.divide(2);std::cout << "Number.cc-7226" << endl;
			setImaginaryPart(nr_i);std::cout << "Number.cc-7227" << endl;
			return true;
		} else {
			set(*i_value, true);std::cout << "Number.cc-7230" << endl;
			if(!cosint()) return false;
			Number nr_i;std::cout << "Number.cc-7232" << endl;
			nr_i.pi();std::cout << "Number.cc-7233" << endl;
			nr_i.divide(2);std::cout << "Number.cc-7234" << endl;
			Number nr_low(nr_i);std::cout << "Number.cc-7235" << endl;
			nr_low.negate();std::cout << "Number.cc-7236" << endl;
			nr_i.setInterval(nr_low, nr_i);std::cout << "Number.cc-7237" << endl;
			setImaginaryPart(nr_i);std::cout << "Number.cc-7238" << endl;
			return true;
		}
	}
	if(!isReal()) return false;
	Number nr_bak(*this);std::cout << "Number.cc-7243" << endl;
	if(isNegative()) {
		if(!negate() || !coshint() || !negate()) {set(nr_bak); return false;}
		if(!i_value) {i_value = new Number(); i_value->markAsImaginaryPart();}
		i_value->pi();std::cout << "Number.cc-7247" << endl;
		setPrecisionAndApproximateFrom(*i_value);std::cout << "Number.cc-7248" << endl;
		return true;
	}
	if(isGreaterThan(1000)) return false;
	if(!setToFloatingPoint()) return false;
	if(isInterval()) {
		Number nr_lower(lowerEndPoint());std::cout << "Number.cc-7254" << endl;
		Number nr_upper(upperEndPoint());std::cout << "Number.cc-7255" << endl;
		if(!nr_lower.coshint() || !nr_upper.coshint()) {
			set(nr_bak);std::cout << "Number.cc-7257" << endl;
			return false;
		}
		if(!isNonZero()) {
			setInterval(nr_lower, nr_upper);std::cout << "Number.cc-7261" << endl;
			setInterval(nr_minus_inf, *this);std::cout << "Number.cc-7262" << endl;
		} else {
			setInterval(nr_lower, nr_upper);std::cout << "Number.cc-7264" << endl;
		}
		return true;
	}
	mpfr_clear_flags();std::cout << "Number.cc-7268" << endl;
	mpfr_t f_x, f_xi, f_y, f_euler;std::cout << "Number.cc-7269" << endl;
	mpz_t z_i, z_fac;std::cout << "Number.cc-7270" << endl;
	mpz_inits(z_i, z_fac, NULL);std::cout << "Number.cc-7271" << endl;
	Number nr_round(*this);std::cout << "Number.cc-7272" << endl;
	nr_round.round();std::cout << "Number.cc-7273" << endl;
	mpfr_inits2(mpfr_get_prec(fl_value) + nr_round.intValue(), f_x, f_xi, f_y, f_euler, NULL);std::cout << "Number.cc-7274" << endl;
	mpfr_set(f_x, fl_value, MPFR_RNDN);std::cout << "Number.cc-7275" << endl;
	mpfr_const_euler(f_euler, MPFR_RNDN);std::cout << "Number.cc-7276" << endl;
	mpfr_set(f_y, fl_value, MPFR_RNDN);std::cout << "Number.cc-7277" << endl;
	mpfr_log(f_y, f_y, MPFR_RNDN);std::cout << "Number.cc-7278" << endl;
	mpfr_add(f_y, f_y, f_euler, MPFR_RNDN);std::cout << "Number.cc-7279" << endl;
	mpz_set_ui(z_i, 0);std::cout << "Number.cc-7280" << endl;
	mpz_set_ui(z_fac, 1);std::cout << "Number.cc-7281" << endl;
	bool b = false;std::cout << "Number.cc-7282" << endl;
	for(size_t i = 0; i < 10000; i++) {
		if(CALCULATOR->aborted()) {set(nr_bak); return false;}
		mpz_add_ui(z_i, z_i, 1);std::cout << "Number.cc-7285" << endl;
		mpz_mul(z_fac, z_fac, z_i);std::cout << "Number.cc-7286" << endl;
		mpz_add_ui(z_i, z_i, 1);std::cout << "Number.cc-7287" << endl;
		mpz_mul(z_fac, z_fac, z_i);std::cout << "Number.cc-7288" << endl;
		mpz_mul(z_fac, z_fac, z_i);std::cout << "Number.cc-7289" << endl;
		mpfr_set(f_xi, f_x, MPFR_RNDN);std::cout << "Number.cc-7290" << endl;
		mpfr_pow_z(f_xi, f_xi, z_i, MPFR_RNDN);std::cout << "Number.cc-7291" << endl;
		mpfr_div_z(f_xi, f_xi, z_fac, MPFR_RNDN);std::cout << "Number.cc-7292" << endl;
		mpz_divexact(z_fac, z_fac, z_i);std::cout << "Number.cc-7293" << endl;
		mpfr_add(f_y, f_y, f_xi, MPFR_RNDN);std::cout << "Number.cc-7294" << endl;
		mpfr_set(fl_value, f_y, CREATE_INTERVAL ? MPFR_RNDD : MPFR_RNDN);std::cout << "Number.cc-7295" << endl;
		if(i > 0 && mpfr_equal_p(fu_value, fl_value)) {b = true; break;}
		mpfr_set(fu_value, f_y, CREATE_INTERVAL ? MPFR_RNDD : MPFR_RNDN);std::cout << "Number.cc-7297" << endl;
	}
	if(!b) {
		set(nr_bak);std::cout << "Number.cc-7300" << endl;
		return false;
	}
	if(CREATE_INTERVAL) mpfr_set(fu_value, f_y, MPFR_RNDU);
	else mpfr_set(fu_value, fl_value, MPFR_RNDN);std::cout << "Number.cc-7304" << endl;
	mpfr_clears(f_x, f_xi, f_y, f_euler, NULL);std::cout << "Number.cc-7305" << endl;
	mpz_clears(z_i, z_fac, NULL);std::cout << "Number.cc-7306" << endl;
	if(!testFloatResult()) {
		set(nr_bak);std::cout << "Number.cc-7308" << endl;
		return false;
	}
	b_approx = true;std::cout << "Number.cc-7311" << endl;
	return true;
}

bool recfact(mpz_ptr ret, long int start, long int n) {
	long int i;std::cout << "Number.cc-7316" << endl;
	if(n <= 16) {
		mpz_set_si(ret, start);std::cout << "Number.cc-7318" << endl;
		for(i = start + 1; i < start + n; i++) mpz_mul_si(ret, ret, i);
		return true;
	}
	if(CALCULATOR->aborted()) return false;
	i = n / 2;std::cout << "Number.cc-7323" << endl;
	if(!recfact(ret, start, i)) return false;
	mpz_t retmul;std::cout << "Number.cc-7325" << endl;
	mpz_init(retmul);std::cout << "Number.cc-7326" << endl;
	if(!recfact(retmul, start + i, n - i)) return false;
	mpz_mul(ret, ret, retmul);std::cout << "Number.cc-7328" << endl;
	mpz_clear(retmul);std::cout << "Number.cc-7329" << endl;
	return true;
}
bool recfact2(mpz_ptr ret, long int start, long int n) {
	long int i;std::cout << "Number.cc-7333" << endl;
	if(n <= 32) {
		mpz_set_si(ret, start + n - 1);std::cout << "Number.cc-7335" << endl;
		for(i = start + n - 3; i >= start; i -= 2) mpz_mul_si(ret, ret, i);
		return true;
	}
	if(CALCULATOR->aborted()) return false;
	i = n / 2;std::cout << "Number.cc-7340" << endl;
	if(n % 2 != i % 2) i--;
	if(!recfact2(ret, start, i)) return false;
	mpz_t retmul;std::cout << "Number.cc-7343" << endl;
	mpz_init(retmul);std::cout << "Number.cc-7344" << endl;
	if(!recfact2(retmul, start + i, n - i)) return false;
	mpz_mul(ret, ret, retmul);std::cout << "Number.cc-7346" << endl;
	mpz_clear(retmul);std::cout << "Number.cc-7347" << endl;
	return true;
}
bool recfactm(mpz_ptr ret, long int start, long int n, long int m) {
	long int i;std::cout << "Number.cc-7351" << endl;
	if(n <= 16 * m) {
		mpz_set_si(ret, start + n - 1);std::cout << "Number.cc-7353" << endl;
		for(i = start + n - 1 - m; i >= start; i -= m) mpz_mul_si(ret, ret, i);
		return true;
	}
	if(CALCULATOR->aborted()) return false;
	i = n / 2;std::cout << "Number.cc-7358" << endl;
	i -= ((i % m) - (n % m));std::cout << "Number.cc-7359" << endl;
	if(!recfactm(ret, start, i, m)) return false;
	mpz_t retmul;std::cout << "Number.cc-7361" << endl;
	mpz_init(retmul);std::cout << "Number.cc-7362" << endl;
	if(!recfactm(retmul, start + i, n - i, m)) return false;
	mpz_mul(ret, ret, retmul);std::cout << "Number.cc-7364" << endl;
	mpz_clear(retmul);std::cout << "Number.cc-7365" << endl;
	return true;
}

bool Number::factorial() {
	if(!isInteger()) {
		return false;
	}
	if(isNegative()) {
		/*if(b_imag) return false;
		setPlusInfinity();std::cout << "Number.cc-7375" << endl;
		return true;*/
		return false;
	}
	if(isZero()) {
		set(1);std::cout << "Number.cc-7380" << endl;
		return true;
	} else if(isOne()) {
		return true;
	} else if(isNegative()) {
		return false;
	}
	if(!mpz_fits_slong_p(mpq_numref(r_value))) return false;
	long int n = mpz_get_si(mpq_numref(r_value));std::cout << "Number.cc-7388" << endl;
	if(!recfact(mpq_numref(r_value), 1, n)) {
		mpz_set_si(mpq_numref(r_value), n);std::cout << "Number.cc-7390" << endl;
		return false;
	}
	return true;
}
bool Number::multiFactorial(const Number &o) {
	if(!isInteger() || !o.isInteger() || !o.isPositive()) {
		return false;
	}
	if(isZero()) {
		set(1, 1);std::cout << "Number.cc-7400" << endl;
		return true;
	} else if(isOne()) {
		return true;
	} else if(isNegative()) {
		return false;
	}
	if(!mpz_fits_slong_p(mpq_numref(r_value)) || !mpz_fits_slong_p(mpq_numref(o.internalRational()))) return false;
	long int n = mpz_get_si(mpq_numref(r_value));std::cout << "Number.cc-7408" << endl;
	long int m = mpz_get_si(mpq_numref(o.internalRational()));std::cout << "Number.cc-7409" << endl;
	if(!recfactm(mpq_numref(r_value), 1, n, m)) {
		mpz_set_si(mpq_numref(r_value), n);std::cout << "Number.cc-7411" << endl;
		return false;
	}
	return true;
}
bool Number::doubleFactorial() {
	if(!isInteger()) {
		return false;
	}
	if(isZero() || isMinusOne()) {
		set(1, 1);std::cout << "Number.cc-7421" << endl;
		return true;
	} else if(isOne()) {
		return true;
	} else if(isNegative()) {
		return false;
	}
	if(!mpz_fits_slong_p(mpq_numref(r_value))) return false;
	unsigned long int n = mpz_get_si(mpq_numref(r_value));std::cout << "Number.cc-7429" << endl;
	if(!recfact2(mpq_numref(r_value), 1, n)) {
		mpz_set_si(mpq_numref(r_value), n);std::cout << "Number.cc-7431" << endl;
		return false;
	}
	return true;
}
bool Number::binomial(const Number &m, const Number &k) {
	if(!m.isInteger() || !k.isInteger()) return false;
	if(m.isNegative()) {
		if(k.isNegative()) return false;
		Number m2(m);std::cout << "Number.cc-7440" << endl;
		if(!m2.negate() || !m2.add(k) || !m2.add(nr_minus_one) || !binomial(m2, k)) return false;
		if(k.isOdd()) negate();
		return true;
	}
	if(k.isNegative() || k.isGreaterThan(m)) {
		clear();std::cout << "Number.cc-7446" << endl;
		return true;
	}
	if(m.isZero() || m.equals(k)) {
		set(1, 1, 0);std::cout << "Number.cc-7450" << endl;
		return true;
	}
	if(!mpz_fits_ulong_p(mpq_numref(k.internalRational()))) return false;
	clear();std::cout << "Number.cc-7454" << endl;
	mpz_bin_ui(mpq_numref(r_value), mpq_numref(m.internalRational()), mpz_get_ui(mpq_numref(k.internalRational())));std::cout << "Number.cc-7455" << endl;
	return true;
}

bool Number::factorize(vector<Number> &factors) {
	if(isZero() || !isInteger()) return false;
	if(mpz_cmp_si(mpq_numref(r_value), 1) == 0) {
		factors.push_back(nr_one);std::cout << "Number.cc-7462" << endl;
		return true;
	}
	if(mpz_cmp_si(mpq_numref(r_value), -1) == 0) {
		factors.push_back(nr_minus_one);std::cout << "Number.cc-7466" << endl;
		return true;
	}
	mpz_t inr, last_prime, facmax;std::cout << "Number.cc-7469" << endl;
	mpz_inits(inr, last_prime, facmax, NULL);std::cout << "Number.cc-7470" << endl;
	mpz_set(inr, mpq_numref(r_value));std::cout << "Number.cc-7471" << endl;
	if(mpz_sgn(inr) < 0) {
		mpz_neg(inr, inr);std::cout << "Number.cc-7473" << endl;
		factors.push_back(nr_minus_one);std::cout << "Number.cc-7474" << endl;
	}
	size_t prime_index = 0;std::cout << "Number.cc-7476" << endl;
	bool b = true;std::cout << "Number.cc-7477" << endl;
	while(b) {
		if(CALCULATOR->aborted()) {mpz_clears(inr, last_prime, facmax, NULL); return false;}
		b = false;std::cout << "Number.cc-7480" << endl;
		mpz_sqrt(facmax, inr);std::cout << "Number.cc-7481" << endl;
		for(; prime_index < NR_OF_PRIMES && mpz_cmp_si(facmax, PRIMES[prime_index]) >= 0; prime_index++) {
			if(mpz_divisible_ui_p(inr, (unsigned long int) PRIMES[prime_index])) {
				mpz_divexact_ui(inr, inr, (unsigned long int) PRIMES[prime_index]);std::cout << "Number.cc-7484" << endl;
				Number fac(PRIMES[prime_index], 1);std::cout << "Number.cc-7485" << endl;;std::cout << "Number.cc-7485" << endl;
				factors.push_back(fac);std::cout << "Number.cc-7486" << endl;
				b = true;std::cout << "Number.cc-7487" << endl;
				break;
			}
		}
		if(prime_index == NR_OF_PRIMES) {
			mpz_set_si(last_prime, PRIMES[NR_OF_PRIMES - 1] + 2);std::cout << "Number.cc-7492" << endl;
			prime_index++;std::cout << "Number.cc-7493" << endl;
		}
		if(!b && prime_index > NR_OF_PRIMES) {
			while(!b && mpz_cmp(facmax, last_prime) >= 0) {
				if(CALCULATOR->aborted()) {mpz_clears(inr, last_prime, facmax, NULL); return false;}
				if(mpz_divisible_p(inr, last_prime)) {
					mpz_divexact(inr, inr, last_prime);std::cout << "Number.cc-7499" << endl;
					b = true;std::cout << "Number.cc-7500" << endl;
					Number fac;std::cout << "Number.cc-7501" << endl;
					fac.setInternal(last_prime);std::cout << "Number.cc-7502" << endl;
					factors.push_back(fac);std::cout << "Number.cc-7503" << endl;
					break;
				}
				mpz_add_ui(last_prime, last_prime, 2);std::cout << "Number.cc-7506" << endl;
			}
		}
	}
	if(mpz_cmp_si(mpq_numref(r_value), 1) != 0) {
		Number fac;std::cout << "Number.cc-7511" << endl;
		fac.setInternal(inr);std::cout << "Number.cc-7512" << endl;
		factors.push_back(fac);std::cout << "Number.cc-7513" << endl;
	}
	mpz_clears(inr, last_prime, facmax, NULL);std::cout << "Number.cc-7515" << endl;
	return true;
}

void Number::rand() {
	if(n_type != NUMBER_TYPE_FLOAT) {
		mpfr_inits2(BIT_PRECISION, fl_value, fu_value, NULL);std::cout << "Number.cc-7521" << endl;
		mpq_set_ui(r_value, 0, 1);std::cout << "Number.cc-7522" << endl;
		n_type = NUMBER_TYPE_FLOAT;std::cout << "Number.cc-7523" << endl;
	}
	mpfr_urandom(fu_value, randstate, MPFR_RNDN);std::cout << "Number.cc-7525" << endl;
	mpfr_set(fl_value, fu_value, MPFR_RNDN);std::cout << "Number.cc-7526" << endl;
	b_approx = false;std::cout << "Number.cc-7527" << endl;
	i_precision = -1;std::cout << "Number.cc-7528" << endl;
}
void Number::randn() {
#if MPFR_VERSION_MAJOR < 4
	Number nr_u, nr_v, nr_r2;std::cout << "Number.cc-7532" << endl;
	do {
		nr_u.rand();std::cout << "Number.cc-7534" << endl; nr_u *= 2;std::cout << "Number.cc-7534" << endl; nr_u -= 1;std::cout << "Number.cc-7534" << endl;
		nr_v.rand();std::cout << "Number.cc-7535" << endl; nr_v *= 2;std::cout << "Number.cc-7535" << endl; nr_v -= 1;std::cout << "Number.cc-7535" << endl;
		nr_r2 = (nr_u ^ 2) + (nr_v ^ 2);std::cout << "Number.cc-7536" << endl;
	} while(nr_r2 > 1 || nr_r2.isZero());
	set(nr_r2);std::cout << "Number.cc-7538" << endl;
	ln();std::cout << "Number.cc-7539" << endl;
	divide(nr_r2);std::cout << "Number.cc-7540" << endl;
	multiply(-2);std::cout << "Number.cc-7541" << endl;
	sqrt();std::cout << "Number.cc-7542" << endl;
	multiply(nr_u);std::cout << "Number.cc-7543" << endl;
#else
	if(n_type != NUMBER_TYPE_FLOAT) {
		mpfr_inits2(BIT_PRECISION, fl_value, fu_value, NULL);std::cout << "Number.cc-7546" << endl;
		mpq_set_ui(r_value, 0, 1);std::cout << "Number.cc-7547" << endl;
		n_type = NUMBER_TYPE_FLOAT;std::cout << "Number.cc-7548" << endl;
	}
	mpfr_nrandom(fu_value, randstate, MPFR_RNDN);std::cout << "Number.cc-7550" << endl;
	mpfr_set(fl_value, fu_value, MPFR_RNDN);std::cout << "Number.cc-7551" << endl;
#endif
	b_approx = false;std::cout << "Number.cc-7553" << endl;
	i_precision = -1;std::cout << "Number.cc-7554" << endl;
}
void Number::intRand(const Number &ceil) {
	clear();std::cout << "Number.cc-7557" << endl;
	if(!ceil.isInteger() || !ceil.isPositive()) return;
	mpz_urandomm(mpq_numref(r_value), randstate, mpq_numref(ceil.internalRational()));std::cout << "Number.cc-7559" << endl;
}

bool Number::add(const Number &o, MathOperation op) {
	switch(op) {
		case OPERATION_SUBTRACT: {
			return subtract(o);
		}
		case OPERATION_ADD: {
			return add(o);
		}
		case OPERATION_MULTIPLY: {
			return multiply(o);
		}
		case OPERATION_DIVIDE: {
			return divide(o);
		}
		case OPERATION_RAISE: {
			return raise(o);
		}
		case OPERATION_EXP10: {
			return exp10(o);
		}
		case OPERATION_BITWISE_AND: {
			return bitAnd(o);
		}
		case OPERATION_BITWISE_OR: {
			return bitOr(o);
		}
		case OPERATION_BITWISE_XOR: {
			return bitXor(o);
		}
		case OPERATION_LOGICAL_OR: {
			Number nr;std::cout << "Number.cc-7592" << endl;
			ComparisonResult i1 = compare(nr);std::cout << "Number.cc-7593" << endl;
			ComparisonResult i2 = o.compare(nr);std::cout << "Number.cc-7594" << endl;
			if(i1 >= COMPARISON_RESULT_UNKNOWN || i1 == COMPARISON_RESULT_EQUAL_OR_LESS || i1 == COMPARISON_RESULT_EQUAL_OR_GREATER) i1 = COMPARISON_RESULT_UNKNOWN;
			if(i2 >= COMPARISON_RESULT_UNKNOWN || i2 == COMPARISON_RESULT_EQUAL_OR_LESS || i2 == COMPARISON_RESULT_EQUAL_OR_GREATER) i2 = COMPARISON_RESULT_UNKNOWN;
			if(i1 >= COMPARISON_RESULT_UNKNOWN && !COMPARISON_IS_NOT_EQUAL(i2)) return false;
			if(i2 >= COMPARISON_RESULT_UNKNOWN && !COMPARISON_IS_NOT_EQUAL(i1)) return false;
			setTrue(COMPARISON_IS_NOT_EQUAL(i1) || COMPARISON_IS_NOT_EQUAL(i2));std::cout << "Number.cc-7599" << endl;
			return true;
		}
		case OPERATION_LOGICAL_XOR: {
			Number nr;std::cout << "Number.cc-7603" << endl;
			ComparisonResult i1 = compare(nr);std::cout << "Number.cc-7604" << endl;
			ComparisonResult i2 = o.compare(nr);std::cout << "Number.cc-7605" << endl;
			if(i1 >= COMPARISON_RESULT_UNKNOWN || i1 == COMPARISON_RESULT_EQUAL_OR_LESS || i1 == COMPARISON_RESULT_EQUAL_OR_GREATER) return false;
			if(i2 >= COMPARISON_RESULT_UNKNOWN || i2 == COMPARISON_RESULT_EQUAL_OR_LESS || i2 == COMPARISON_RESULT_EQUAL_OR_GREATER) return false;
			if(COMPARISON_IS_NOT_EQUAL(i1)) setTrue(i2 == COMPARISON_RESULT_EQUAL);
			else setTrue(COMPARISON_IS_NOT_EQUAL(i2));std::cout << "Number.cc-7609" << endl;
			return true;
		}
		case OPERATION_LOGICAL_AND: {
			Number nr;std::cout << "Number.cc-7613" << endl;
			ComparisonResult i1 = compare(nr);std::cout << "Number.cc-7614" << endl;
			ComparisonResult i2 = o.compare(nr);std::cout << "Number.cc-7615" << endl;
			if(i1 >= COMPARISON_RESULT_UNKNOWN || i1 == COMPARISON_RESULT_EQUAL_OR_LESS || i1 == COMPARISON_RESULT_EQUAL_OR_GREATER) i1 = COMPARISON_RESULT_UNKNOWN;
			if(i2 >= COMPARISON_RESULT_UNKNOWN || i2 == COMPARISON_RESULT_EQUAL_OR_LESS || i2 == COMPARISON_RESULT_EQUAL_OR_GREATER) i2 = COMPARISON_RESULT_UNKNOWN;
			if(i1 >= COMPARISON_RESULT_UNKNOWN && (i2 == COMPARISON_RESULT_UNKNOWN || COMPARISON_IS_NOT_EQUAL(i2))) return false;
			if(i2 >= COMPARISON_RESULT_UNKNOWN && COMPARISON_IS_NOT_EQUAL(i1)) return false;
			setTrue(COMPARISON_IS_NOT_EQUAL(i1) && COMPARISON_IS_NOT_EQUAL(i2));std::cout << "Number.cc-7620" << endl;
			return true;
		}
		case OPERATION_EQUALS: {
			ComparisonResult i = compare(o);std::cout << "Number.cc-7624" << endl;
			if(i >= COMPARISON_RESULT_UNKNOWN || i == COMPARISON_RESULT_EQUAL_OR_GREATER || i == COMPARISON_RESULT_EQUAL_OR_LESS) return false;
			setTrue(i == COMPARISON_RESULT_EQUAL);std::cout << "Number.cc-7626" << endl;
			return true;
		}
		case OPERATION_GREATER: {
			ComparisonResult i = compare(o);std::cout << "Number.cc-7630" << endl;
			switch(i) {
				case COMPARISON_RESULT_LESS: {
					setTrue();std::cout << "Number.cc-7633" << endl;
					return true;
				}
				case COMPARISON_RESULT_GREATER: {}
				case COMPARISON_RESULT_EQUAL_OR_GREATER: {}
				case COMPARISON_RESULT_EQUAL: {
					setFalse();std::cout << "Number.cc-7639" << endl;
					return true;
				}
				default: {
					return false;
				}
			}
		}
		case OPERATION_LESS: {
			ComparisonResult i = compare(o);std::cout << "Number.cc-7648" << endl;
			switch(i) {
				case COMPARISON_RESULT_GREATER: {
					setTrue();std::cout << "Number.cc-7651" << endl;
					return true;
				}
				case COMPARISON_RESULT_LESS: {}
				case COMPARISON_RESULT_EQUAL_OR_LESS: {}
				case COMPARISON_RESULT_EQUAL: {
					setFalse();std::cout << "Number.cc-7657" << endl;
					return true;
				}
				default: {
					return false;
				}
			}
		}
		case OPERATION_EQUALS_GREATER: {
			ComparisonResult i = compare(o);std::cout << "Number.cc-7666" << endl;
			switch(i) {
				case COMPARISON_RESULT_EQUAL_OR_LESS: {}
				case COMPARISON_RESULT_EQUAL: {}
				case COMPARISON_RESULT_LESS: {
					setTrue();std::cout << "Number.cc-7671" << endl;
					return true;
				}
				case COMPARISON_RESULT_GREATER: {
					setFalse();std::cout << "Number.cc-7675" << endl;
					return true;
				}
				default: {
					return false;
				}
			}
			return false;
		}
		case OPERATION_EQUALS_LESS: {
			ComparisonResult i = compare(o);std::cout << "Number.cc-7685" << endl;
			switch(i) {
				case COMPARISON_RESULT_EQUAL_OR_GREATER: {}
				case COMPARISON_RESULT_EQUAL: {}
				case COMPARISON_RESULT_GREATER: {
					setTrue();std::cout << "Number.cc-7690" << endl;
					return true;
				}
				case COMPARISON_RESULT_LESS: {
					setFalse();std::cout << "Number.cc-7694" << endl;
					return true;
				}
				default: {
					return false;
				}
			}
			return false;
		}
		case OPERATION_NOT_EQUALS: {
			ComparisonResult i = compare(o);std::cout << "Number.cc-7704" << endl;
			if(i >= COMPARISON_RESULT_UNKNOWN || i == COMPARISON_RESULT_EQUAL_OR_GREATER || i == COMPARISON_RESULT_EQUAL_OR_LESS) return false;
			setTrue(i == COMPARISON_RESULT_NOT_EQUAL || i == COMPARISON_RESULT_GREATER || i == COMPARISON_RESULT_LESS);std::cout << "Number.cc-7706" << endl;
			return true;
		}
	}
	return false;
}
string Number::printNumerator(int base, bool display_sign, BaseDisplay base_display, bool lower_case) const {
	return format_number_string(printMPZ(mpq_numref(r_value), base, false, lower_case), base, base_display, display_sign);
}
string Number::printDenominator(int base, bool display_sign, BaseDisplay base_display, bool lower_case) const {
	return format_number_string(printMPZ(mpq_denref(r_value), base, false, lower_case), base, base_display, display_sign);
}
string Number::printImaginaryNumerator(int base, bool display_sign, BaseDisplay base_display, bool lower_case) const {
	return format_number_string(printMPZ(mpq_numref(i_value ? i_value->internalRational() : nr_zero.internalRational()), base, false, lower_case), base, base_display, display_sign);
}
string Number::printImaginaryDenominator(int base, bool display_sign, BaseDisplay base_display, bool lower_case) const {
	return format_number_string(printMPZ(mpq_denref(i_value ? i_value->internalRational() : nr_zero.internalRational()), base, false, lower_case), base, base_display, display_sign);
}

ostream& operator << (ostream &os, const Number &nr) {
	os << nr.print();std::cout << "Number.cc-7726" << endl;
	return os;
}

string Number::print(const PrintOptions &po, const InternalPrintStruct &ips) const {
	if(CALCULATOR->aborted()) return CALCULATOR->abortedMessage();
	if(ips.minus) *ips.minus = false;
	if(ips.exp_minus) *ips.exp_minus = false;
	if(ips.num) *ips.num = "";
	if(ips.den) *ips.den = "";
	if(ips.exp) *ips.exp = "";
	if(ips.re) *ips.re = "";
	if(ips.im) *ips.im = "";
	if(ips.iexp) *ips.iexp = 0;
	if(po.is_approximate && isApproximate()) *po.is_approximate = true;
	if(po.base == BASE_BIJECTIVE_26 && isReal()) {
		Number nr(*this);std::cout << "Number.cc-7742" << endl;
		if(!nr.isInteger()) {
			if(po.is_approximate) *po.is_approximate = true;
			nr.round(po.round_halfway_to_even);std::cout << "Number.cc-7745" << endl;
		}
		if(nr.isZero()) return "";
		bool neg = nr.isNegative();std::cout << "Number.cc-7748" << endl;
		if(neg) nr.negate();
		Number nri, nra;std::cout << "Number.cc-7750" << endl;
		string str;std::cout << "Number.cc-7751" << endl;
		do {
			nri = nr;std::cout << "Number.cc-7753" << endl;
			nri /= 26;std::cout << "Number.cc-7754" << endl;
			nri.ceil();std::cout << "Number.cc-7755" << endl;
			nri--;std::cout << "Number.cc-7756" << endl;
			nra = nri;std::cout << "Number.cc-7757" << endl;
			nra *= 26;std::cout << "Number.cc-7758" << endl;
			nra = nr - nra;std::cout << "Number.cc-7759" << endl;
			nr = nri;std::cout << "Number.cc-7760" << endl;
			str.insert((size_t) 0, 1, ('A' + nra.intValue() - 1));std::cout << "Number.cc-7761" << endl;
		} while(!nr.isZero());
		if(ips.minus) {
			*ips.minus = neg;std::cout << "Number.cc-7764" << endl;
		} else if(neg) {
			if(po.use_unicode_signs && (!po.can_display_unicode_string_function || (*po.can_display_unicode_string_function) (SIGN_MINUS, po.can_display_unicode_string_arg))) str.insert(0, SIGN_MINUS);
			else str.insert(0, "-");
		}
		return str;
	}
	if(((po.base < BASE_CUSTOM && po.base != BASE_BIJECTIVE_26) || (po.base == BASE_CUSTOM && (!CALCULATOR->customOutputBase().isInteger() || CALCULATOR->customOutputBase() > 62 || CALCULATOR->customOutputBase() < 2))) && isReal()) {
		Number base;std::cout << "Number.cc-7772" << endl;
		switch(po.base) {
			case BASE_GOLDEN_RATIO: {
				base.set(5);std::cout << "Number.cc-7775" << endl;
				base.sqrt();std::cout << "Number.cc-7776" << endl;
				base.add(1);std::cout << "Number.cc-7777" << endl;
				base.divide(2);std::cout << "Number.cc-7778" << endl;
				break;
			}
			case BASE_SUPER_GOLDEN_RATIO: {
				base.set(93);std::cout << "Number.cc-7782" << endl;
				base.sqrt();std::cout << "Number.cc-7783" << endl;
				base.multiply(3);std::cout << "Number.cc-7784" << endl;
				Number b2(base);std::cout << "Number.cc-7785" << endl;
				b2.negate();std::cout << "Number.cc-7786" << endl;
				b2.add(29);std::cout << "Number.cc-7787" << endl;
				b2.divide(2);std::cout << "Number.cc-7788" << endl;
				b2.cbrt();std::cout << "Number.cc-7789" << endl;
				base.add(29);std::cout << "Number.cc-7790" << endl;
				base.divide(2);std::cout << "Number.cc-7791" << endl;
				base.cbrt();std::cout << "Number.cc-7792" << endl;
				base.add(b2);std::cout << "Number.cc-7793" << endl;
				base.add(1);std::cout << "Number.cc-7794" << endl;
				base.divide(3);std::cout << "Number.cc-7795" << endl;
				break;
			}
			case BASE_PI: {base.pi(); break;}
			case BASE_E: {base.e(); break;}
			case BASE_SQRT2: {base.set(2); base.sqrt(); break;}
			case BASE_UNICODE: {base.set(1114112L); break;}
			default: {base = CALCULATOR->customOutputBase();std::cout << "Number.cc-7802" << endl;}
		}
		if(base.isInteger() && base >= 2 && base <= 36) {
			PrintOptions po2 = po;std::cout << "Number.cc-7805" << endl;
			po2.base = base.intValue();std::cout << "Number.cc-7806" << endl;
			return print(po2, ips);
		}
		if(!base.isReal() || (base.isNegative() && !base.isInteger()) || !(base > 1 || base < -1)) {
			CALCULATOR->error(true, _("Unsupported base"), NULL);
			PrintOptions po2 = po;std::cout << "Number.cc-7811" << endl;
			po2.base = BASE_DECIMAL;std::cout << "Number.cc-7812" << endl;
			return print(po2, ips);
		}
		if((base.isNegative() && isInterval()) || (isInterval() && precision(true) < 1)) {
			Number nr(*this);std::cout << "Number.cc-7816" << endl;
			if(!nr.intervalToPrecision()) {
				nr.intervalToMidValue();std::cout << "Number.cc-7818" << endl;
				nr.setPrecision(1);std::cout << "Number.cc-7819" << endl;
			}
			return nr.print(po, ips);
		}
		if(base.isInterval() && base.precision(true) < 1) {
			if(!base.intervalToPrecision()) {
				base.intervalToMidValue();std::cout << "Number.cc-7825" << endl;
				base.setPrecision(1);std::cout << "Number.cc-7826" << endl;
			}
		}
		Number abs_base(base);std::cout << "Number.cc-7829" << endl;
		abs_base.abs();std::cout << "Number.cc-7830" << endl;
		bool b_uni = (abs_base == 1114112L);std::cout << "Number.cc-7831" << endl;
		bool b_num = abs_base > 62;std::cout << "Number.cc-7832" << endl;
		bool b_case = !b_num && abs_base > 36;std::cout << "Number.cc-7833" << endl;

		if(po.is_approximate && base.isApproximate()) *po.is_approximate = true;
		long int precision = PRECISION;std::cout << "Number.cc-7836" << endl;
		if(b_approx && i_precision >= 0 && i_precision < precision) precision = i_precision;
		if(base.isApproximate() && base.precision() >= 0 && base.precision() < precision) precision = base.precision();
		if(po.restrict_to_parent_precision && ips.parent_precision >= 0 && ips.parent_precision < precision) precision = ips.parent_precision;
		long int precision_base = precision;std::cout << "Number.cc-7840" << endl;
		Number precmax(10);std::cout << "Number.cc-7841" << endl;
		precmax.raise(precision_base);std::cout << "Number.cc-7842" << endl;
		precmax--;std::cout << "Number.cc-7843" << endl;
		precmax.log(abs_base < 2 ? 2 : abs_base);std::cout << "Number.cc-7844" << endl;
		precmax.floor();std::cout << "Number.cc-7845" << endl;
		precision_base = precmax.lintValue();std::cout << "Number.cc-7846" << endl;

		string str;std::cout << "Number.cc-7848" << endl;

		if(isZero()) {
			if(b_num) str += '\\';
			str += '0';std::cout << "Number.cc-7852" << endl;
			if(po.show_ending_zeroes && isApproximate()) {
				str += po.decimalpoint();std::cout << "Number.cc-7854" << endl;
				while(precision_base > 1) {
					precision_base--;std::cout << "Number.cc-7856" << endl;
					if(b_num) str += '\\';
					str += '0';std::cout << "Number.cc-7858" << endl;
				}
			}
			if(ips.num) *ips.num = str;
			return str;
		}

		bool exact = false;std::cout << "Number.cc-7865" << endl;
		bool b_dp = false;std::cout << "Number.cc-7866" << endl;
		bool neg = false;std::cout << "Number.cc-7867" << endl;
		long int i_dp = 0;std::cout << "Number.cc-7868" << endl;
		Number nr(*this);std::cout << "Number.cc-7869" << endl;
		Number nr_log(*this);std::cout << "Number.cc-7870" << endl;
		nr_log.abs();std::cout << "Number.cc-7871" << endl;
		nr_log.log(abs_base);std::cout << "Number.cc-7872" << endl;
		if(nr_log.isInterval() && nr_log.precision(true) > PRECISION) {
			Number nr_log_upper(nr_log.upperEndPoint());std::cout << "Number.cc-7874" << endl;
			nr_log.intervalToMidValue();std::cout << "Number.cc-7875" << endl;
			nr_log.floor();std::cout << "Number.cc-7876" << endl;
			nr_log++;std::cout << "Number.cc-7877" << endl;
			if(nr_log_upper < nr_log) {
				nr_log--;std::cout << "Number.cc-7879" << endl;
			}
		} else {
			nr_log.intervalToMidValue();std::cout << "Number.cc-7882" << endl;
			nr_log.floor();std::cout << "Number.cc-7883" << endl;
		}
		if(nr_log < -1000 || nr_log > 1000) {
			PrintOptions po2 = po;std::cout << "Number.cc-7886" << endl;
			po2.base = BASE_DECIMAL;std::cout << "Number.cc-7887" << endl;
			return print(po2, ips);
		}
		vector<long int> digits;std::cout << "Number.cc-7890" << endl;
		Number nr_digit;std::cout << "Number.cc-7891" << endl;
		if(base.isNegative()) {
			CALCULATOR->beginTemporaryStopIntervalArithmetic();std::cout << "Number.cc-7893" << endl;
			nr_log++;std::cout << "Number.cc-7894" << endl;
			if(nr_log < precision_base && (!po.use_max_decimals || po.max_decimals != 0)) {
				nr_log.subtract(precision_base);std::cout << "Number.cc-7896" << endl;
				nr_log.negate();std::cout << "Number.cc-7897" << endl;
				b_dp = true;std::cout << "Number.cc-7898" << endl;
				if(po.use_max_decimals && po.max_decimals >= 0 && nr_log > po.max_decimals) nr_log = po.max_decimals;
				i_dp = nr_log.lintValue();std::cout << "Number.cc-7900" << endl;
				Number nr_mul(base);std::cout << "Number.cc-7901" << endl;
				nr_mul.raise(nr_log);std::cout << "Number.cc-7902" << endl;
				nr.multiply(nr_mul);std::cout << "Number.cc-7903" << endl;
			}
			Number nr_frac;std::cout << "Number.cc-7905" << endl;
			while(true) {
				nr_digit = nr;std::cout << "Number.cc-7907" << endl;
				nr.divide(base);std::cout << "Number.cc-7908" << endl;
				nr_digit.mod(base);std::cout << "Number.cc-7909" << endl;
				nr.floor();std::cout << "Number.cc-7910" << endl;
				if(nr_digit.isNegative()) {
					nr_digit += abs_base;std::cout << "Number.cc-7912" << endl;
					nr++;std::cout << "Number.cc-7913" << endl;
				}
				if(digits.empty()) {
					nr_frac = nr_digit;std::cout << "Number.cc-7916" << endl;
					nr_frac.frac();std::cout << "Number.cc-7917" << endl;
				}
				nr_digit.floor();std::cout << "Number.cc-7919" << endl;
				digits.insert(digits.begin(), nr_digit.lintValue());std::cout << "Number.cc-7920" << endl;
				if(nr.isZero()) {
					if(nr_frac.isZero()) {
						exact = true;std::cout << "Number.cc-7923" << endl;
						break;
					}
					nr_frac *= 2;std::cout << "Number.cc-7926" << endl;
					if(nr_frac.isGreaterThan(1) || (nr_frac.isOne() && (!po.round_halfway_to_even || digits[digits.size() - 1] % 2 == 1))) {
						size_t i = digits.size();std::cout << "Number.cc-7928" << endl;
						while(i > 0) {
							i--;std::cout << "Number.cc-7930" << endl;
							digits[i]++;std::cout << "Number.cc-7931" << endl;
							if(abs_base <= digits[i]) {
								digits[i] = 0;std::cout << "Number.cc-7933" << endl;
								if(i == 0) {
									digits.insert(digits.begin(), 1L);std::cout << "Number.cc-7935" << endl;
									digits.erase(digits.end() - 1);std::cout << "Number.cc-7936" << endl;
								}
							} else {
								break;
							}
						}
					}
					break;
				}
			}
			i_dp = digits.size() - i_dp;std::cout << "Number.cc-7946" << endl;
			if(b_dp && (!po.show_ending_zeroes || ((exact || base.isApproximate()) && !isApproximate()))) {
				while(digits[digits.size() - 1] == 0) {
					digits.erase(digits.end() - 1);std::cout << "Number.cc-7949" << endl;
					if((long int) digits.size() == i_dp) {
						b_dp = false;std::cout << "Number.cc-7951" << endl;
						break;
					}
				}
			}
			CALCULATOR->endTemporaryStopIntervalArithmetic();std::cout << "Number.cc-7956" << endl;
		} else {
			if(nr_log.isNegative()) {
				i_dp = nr_log.lintValue() + 1;std::cout << "Number.cc-7959" << endl;
				b_dp = true;std::cout << "Number.cc-7960" << endl;
			}
			if(nr.isNegative()) {
				nr.negate();std::cout << "Number.cc-7963" << endl;
				neg = true;std::cout << "Number.cc-7964" << endl;
			}
			Number base_pow;std::cout << "Number.cc-7966" << endl;
			while(true) {
				base_pow = base;std::cout << "Number.cc-7968" << endl;
				base_pow.raise(nr_log);std::cout << "Number.cc-7969" << endl;
				if(CALCULATOR->aborted()) {
					return CALCULATOR->abortedMessage();
				}
				bool do_break = (b_dp && ((long int) digits.size() == precision_base || (po.use_max_decimals && po.max_decimals >= 0 && po.max_decimals == (long int) digits.size() - i_dp)));
				if(!b_dp && nr_log.isNegative()) {
					if((long int) digits.size() >= precision_base || (po.use_max_decimals && po.max_decimals == 0)) {
						do_break = true;
					} else {
						i_dp = digits.size();std::cout << "Number.cc-7978" << endl;
						b_dp = true;std::cout << "Number.cc-7979" << endl;
					}
				}
				if(b_dp && nr.isInterval() && nr.precision(true) < 1) {
					do_break = true;
					precision_base = 0;std::cout << "Number.cc-7984" << endl;
					if(i_dp == (long int) digits.size()) b_dp = false;
				}
				if(do_break) {
					nr.divide(base);std::cout << "Number.cc-7988" << endl;
					nr.multiply(2);std::cout << "Number.cc-7989" << endl;
					nr.intervalToMidValue();std::cout << "Number.cc-7990" << endl;
					base_pow.intervalToMidValue();std::cout << "Number.cc-7991" << endl;
					if(nr.isGreaterThan(base_pow) || (nr == base_pow && (!po.round_halfway_to_even || digits[digits.size() - 1] % 2 == 1))) {
						size_t i = digits.size();std::cout << "Number.cc-7993" << endl;
						while(i > 0) {
							i--;std::cout << "Number.cc-7995" << endl;
							digits[i]++;std::cout << "Number.cc-7996" << endl;
							if(base <= digits[i]) {
								digits[i] = 0;std::cout << "Number.cc-7998" << endl;
								if(i == 0) {
									digits.insert(digits.begin(), 1L);std::cout << "Number.cc-8000" << endl;
									if(b_dp) {
										i_dp++;std::cout << "Number.cc-8002" << endl;
										if((long int) digits.size() == i_dp) {
											b_dp = false;std::cout << "Number.cc-8004" << endl;
										} else {
											digits.erase(digits.end() - 1);std::cout << "Number.cc-8006" << endl;
											if((long int) digits.size() == i_dp) b_dp = false;
										}
									}
								}
							} else {
								break;
							}
						}
					}
					break;
				}
				if(nr == base_pow) {
					digits.push_back(1L);std::cout << "Number.cc-8019" << endl;
					exact = true;std::cout << "Number.cc-8020" << endl;
					nr.clear();std::cout << "Number.cc-8021" << endl;
				} else {
					nr_digit = nr;std::cout << "Number.cc-8023" << endl;
					nr_digit.divide(base_pow);std::cout << "Number.cc-8024" << endl;
					if(nr_digit.isInterval() && nr_digit.precision(true) > PRECISION) {
						Number nr_digit_upper(nr_digit.upperEndPoint());std::cout << "Number.cc-8026" << endl;
						nr_digit.intervalToMidValue();std::cout << "Number.cc-8027" << endl;
						nr_digit.floor();std::cout << "Number.cc-8028" << endl;
						nr_digit++;std::cout << "Number.cc-8029" << endl;
						if(nr_digit_upper < nr_digit) {
							nr_digit--;std::cout << "Number.cc-8031" << endl;
						}
					} else {
						nr_digit.intervalToMidValue();std::cout << "Number.cc-8034" << endl;
						nr_digit.trunc();std::cout << "Number.cc-8035" << endl;
					}
					digits.push_back(nr_digit.lintValue());std::cout << "Number.cc-8037" << endl;
					nr_digit.multiply(base_pow);std::cout << "Number.cc-8038" << endl;
					nr -= nr_digit;std::cout << "Number.cc-8039" << endl;
				}
				if(nr.isZero()) {
					while(nr_log > 0) {digits.push_back(0L); nr_log--;}
					exact = true;std::cout << "Number.cc-8043" << endl;
					break;
				}
				nr_log--;std::cout << "Number.cc-8046" << endl;
			}
			if(po.show_ending_zeroes && (isApproximate() || (!exact && !base.isApproximate())) && digits.size() != 1 && (long int) digits.size() < precision_base) {
				if(digits.empty()) digits.push_back(0L);
				if(!b_dp) {b_dp = true; i_dp = digits.size();}
				while((long int) digits.size() < precision_base) {
					digits.push_back(0L);std::cout << "Number.cc-8052" << endl;
				}
			} else if(b_dp && (!po.show_ending_zeroes || ((exact || base.isApproximate()) && !isApproximate()))) {
				while(digits[digits.size() - 1] == 0) {
					digits.erase(digits.end() - 1);std::cout << "Number.cc-8056" << endl;
					if((long int) digits.size() == i_dp) {
						b_dp = false;std::cout << "Number.cc-8058" << endl;
						break;
					}
				}
			}
		}
		if(po.is_approximate && !exact) *po.is_approximate = true;
		if(b_dp && i_dp < 0) {
			b_dp = false;std::cout << "Number.cc-8066" << endl;
			if(b_num) str += '\\';
			str += '0';std::cout << "Number.cc-8068" << endl;
			if(b_num) str += '\\';
			str += po.decimalpoint();std::cout << "Number.cc-8070" << endl;
			while(i_dp < 0) {
				i_dp++;std::cout << "Number.cc-8072" << endl;
				if(b_num) str += '\\';
				str += '0';std::cout << "Number.cc-8074" << endl;
			}
		}
		bool prev_esc = false;std::cout << "Number.cc-8077" << endl;
		for(size_t index = 0; index < digits.size(); index++) {
			long int c = digits[index];std::cout << "Number.cc-8079" << endl;
			if(b_dp && (size_t) i_dp == index) {
				if(str.empty()) {
					if(b_num) str += '\\';
					str += '0';std::cout << "Number.cc-8083" << endl;
				}
				if(b_num) str += '\\';
				str += po.decimalpoint();std::cout << "Number.cc-8086" << endl;
				b_dp = false;std::cout << "Number.cc-8087" << endl;
				if(b_num) prev_esc = true;
			}
			if(b_num) {
				if(!b_uni || c <= 32 || (!po.use_unicode_signs && c > 0x7f) || c >= 1114112L) {
					str += '\\';std::cout << "Number.cc-8092" << endl;
					str += i2s(c);std::cout << "Number.cc-8093" << endl;
					prev_esc = true;std::cout << "Number.cc-8094" << endl;
				} else if(prev_esc && c >= '0' && c <= '9') {
					str += '\\';std::cout << "Number.cc-8096" << endl;
					str += i2s(c);std::cout << "Number.cc-8097" << endl;
					prev_esc = true;std::cout << "Number.cc-8098" << endl;
				} else if(c <= 0x7f) {
					if(c == '\\' && index < digits.size() - 1 && ((digits[index + 1] >= '0' && digits[index + 1] <= '9') || (digits[index + 1] == 'x' && index < digits.size() - 2 && ((digits[index + 2] >= '0' && digits[index + 2] <= '9') || (digits[index + 2] >= 'A' && digits[index + 2] <= 'F') || (digits[index + 2] >= 'a' && digits[index + 2] <= 'f'))))) str += '\\';
					str += (char) c;std::cout << "Number.cc-8101" << endl;
					prev_esc = false;std::cout << "Number.cc-8102" << endl;
				} else {
					string str_c;std::cout << "Number.cc-8104" << endl;
					if(c <= 0x7ff) {
						if(c > 0xa0) {
							str_c += (char) ((c >> 6) | 0xc0);std::cout << "Number.cc-8107" << endl;
							str_c += (char) ((c & 0x3f) | 0x80);std::cout << "Number.cc-8108" << endl;
						}
					} else if((c <= 0xd7ff || (0xe000 <= c && c <= 0xffff))) {
						str_c += (char) ((c >> 12) | 0xe0);std::cout << "Number.cc-8111" << endl;
						str_c += (char) (((c >> 6) & 0x3f) | 0x80);std::cout << "Number.cc-8112" << endl;
						str_c += (char) ((c & 0x3f) | 0x80);std::cout << "Number.cc-8113" << endl;
					} else if(0xffff < c && c <= 0x10ffff) {
						str_c += (char) ((c >> 18) | 0xf0);std::cout << "Number.cc-8115" << endl;
						str_c += (char) (((c >> 12) & 0x3f) | 0x80);std::cout << "Number.cc-8116" << endl;
						str_c += (char) (((c >> 6) & 0x3f) | 0x80);std::cout << "Number.cc-8117" << endl;
						str_c += (char) ((c & 0x3f) | 0x80);std::cout << "Number.cc-8118" << endl;
					}
					if(str_c.empty()) {
						str += '\\';std::cout << "Number.cc-8121" << endl;
						str += i2s(c);std::cout << "Number.cc-8122" << endl;
						prev_esc = true;std::cout << "Number.cc-8123" << endl;
					} else {
						str += str_c;std::cout << "Number.cc-8125" << endl;
						prev_esc = false;std::cout << "Number.cc-8126" << endl;
					}
				}
			} else {
				if(c <= 9) {
					str += '0' + c;std::cout << "Number.cc-8131" << endl;
				} else if(b_case) {
					if(c < 36) str += 'A' + (c - 10);
					else str += 'a' + (c - 36);std::cout << "Number.cc-8134" << endl;
				} else if(po.lower_case_numbers) {
					str += 'a' + (c - 10);std::cout << "Number.cc-8136" << endl;
				} else {
					str += 'A' + (c - 10);std::cout << "Number.cc-8138" << endl;
				}
				prev_esc = false;std::cout << "Number.cc-8140" << endl;
			}
		}
		if(str.empty()) {
			if(b_num) str += '\\';
			str += '0';std::cout << "Number.cc-8145" << endl;
		}
		if(ips.minus) {
			*ips.minus = neg;std::cout << "Number.cc-8148" << endl;
		} else if(neg) {
			if(b_num) str.insert(0, "\\-");
			else if(po.use_unicode_signs && (!po.can_display_unicode_string_function || (*po.can_display_unicode_string_function) (SIGN_MINUS, po.can_display_unicode_string_arg))) str.insert(0, SIGN_MINUS);
			else str.insert(0, "-");
		}
		if(ips.num) *ips.num = str;
		return str;
	}
	if((po.base == BASE_SEXAGESIMAL || po.base == BASE_TIME) && isReal()) {
		Number nr(*this);std::cout << "Number.cc-8158" << endl;
		bool neg = nr.isNegative();std::cout << "Number.cc-8159" << endl;
		nr.setNegative(false);std::cout << "Number.cc-8160" << endl;
		nr.trunc();std::cout << "Number.cc-8161" << endl;
		PrintOptions po2 = po;std::cout << "Number.cc-8162" << endl;
		po2.base = 10;std::cout << "Number.cc-8163" << endl;
		po2.number_fraction_format = FRACTION_FRACTIONAL;
		string str = nr.print(po2);std::cout << "Number.cc-8165" << endl;
		if(po.base == BASE_SEXAGESIMAL) {
			if(po.use_unicode_signs && (!po.can_display_unicode_string_function || (*po.can_display_unicode_string_function) (SIGN_DEGREE, po.can_display_unicode_string_arg))) {
				str += SIGN_DEGREE;std::cout << "Number.cc-8168" << endl;
			} else {
				str += "o";
			}
		}
		nr = *this;std::cout << "Number.cc-8173" << endl;
		nr.frac();std::cout << "Number.cc-8174" << endl;
		nr *= 60;std::cout << "Number.cc-8175" << endl;
		Number nr2(nr);std::cout << "Number.cc-8176" << endl;
		nr.trunc();std::cout << "Number.cc-8177" << endl;
		if(po.base == BASE_TIME) {
			str += ":";
			if(nr.isLessThan(10)) {
				str += "0";
			}
		}
		str += nr.printNumerator(10, false);std::cout << "Number.cc-8184" << endl;
		if(po.base == BASE_SEXAGESIMAL) {
			if(po.use_unicode_signs && (!po.can_display_unicode_string_function || (*po.can_display_unicode_string_function) ("′", po.can_display_unicode_string_arg))) {
				str += "′";
			} else {
				str += "'";
			}
		}
		nr2.frac();std::cout << "Number.cc-8192" << endl;
		if(!nr2.isZero() || po.base == BASE_SEXAGESIMAL) {
			nr2.multiply(60);std::cout << "Number.cc-8194" << endl;
			nr = nr2;std::cout << "Number.cc-8195" << endl;
			nr.trunc();std::cout << "Number.cc-8196" << endl;
			nr2.frac();std::cout << "Number.cc-8197" << endl;
			if(!nr2.isZero()) {
				if(po.is_approximate) *po.is_approximate = true;
				if(nr2.isGreaterThanOrEqualTo(nr_half)) {
					nr.add(1);std::cout << "Number.cc-8201" << endl;
				}
			}
			if(po.base == BASE_TIME) {
				str += ":";
				if(nr.isLessThan(10)) {
					str += "0";
				}
			}
			str += nr.printNumerator(10, false);std::cout << "Number.cc-8210" << endl;
			if(po.base == BASE_SEXAGESIMAL) {
				if(po.use_unicode_signs && (!po.can_display_unicode_string_function || (*po.can_display_unicode_string_function) ("″", po.can_display_unicode_string_arg))) {
					str += "″";
				} else {
					str += "\"";
				}
			}
		}
		if(ips.minus) {
			*ips.minus = neg;std::cout << "Number.cc-8220" << endl;
		} else if(neg) {
			if(po.use_unicode_signs && (!po.can_display_unicode_string_function || (*po.can_display_unicode_string_function) (SIGN_MINUS, po.can_display_unicode_string_arg))) str.insert(0, SIGN_MINUS);
			else str.insert(0, "-");
		}
		if(ips.num) *ips.num = str;

		return str;
	}
	string str;std::cout << "Number.cc-8229" << endl;
	int base;std::cout << "Number.cc-8230" << endl;
	long int min_decimals = 0;std::cout << "Number.cc-8231" << endl;
	if(po.use_min_decimals && po.min_decimals > 0) min_decimals = po.min_decimals;
	if((int) min_decimals > po.max_decimals && po.use_max_decimals && po.max_decimals >= 0) {
		min_decimals = po.max_decimals;std::cout << "Number.cc-8234" << endl;
	}
	if(po.base == BASE_CUSTOM) base = CALCULATOR->customOutputBase().intValue();
	else if(po.base <= 1 && po.base != BASE_ROMAN_NUMERALS && po.base != BASE_TIME) base = 10;
	else if(po.base > 36 && po.base != BASE_SEXAGESIMAL) base = 36;
	else base = po.base;std::cout << "Number.cc-8239" << endl;
	if(po.base == BASE_ROMAN_NUMERALS) {
		if(!isRational()) {
			CALCULATOR->error(false, _("Can only display rational numbers as roman numerals."), NULL);
			base = 10;std::cout << "Number.cc-8243" << endl;
		} else if(mpz_cmpabs_ui(mpq_numref(r_value), 9999) > 0 || mpz_cmp_ui(mpq_denref(r_value), 9999) > 0) {
			CALCULATOR->error(false, _("Cannot display numbers greater than 9999 or less than -9999 as roman numerals."), NULL);
			base = 10;std::cout << "Number.cc-8246" << endl;
		}
	}

	if(hasImaginaryPart()) {
		if(i_value->isZero()) {
			Number nr;std::cout << "Number.cc-8252" << endl;
			nr.set(*this, false, true);std::cout << "Number.cc-8253" << endl;
			return nr.print(po, ips);
		}
		string str_i = (CALCULATOR ? CALCULATOR->v_i->preferredDisplayName(po.abbreviate_names, po.use_unicode_signs, false, po.use_reference_names, po.can_display_unicode_string_function, po.can_display_unicode_string_arg).name : "i");
		bool bre = hasRealPart();std::cout << "Number.cc-8257" << endl;
		if(bre) {
			Number r_nr(*this);std::cout << "Number.cc-8259" << endl;
			r_nr.clearImaginary();std::cout << "Number.cc-8260" << endl;
			str = r_nr.print(po, ips);std::cout << "Number.cc-8261" << endl;
			if(ips.re) *ips.re = str;
			InternalPrintStruct ips_n = ips;std::cout << "Number.cc-8263" << endl;
			bool neg = false;std::cout << "Number.cc-8264" << endl;
			ips_n.minus = &neg;std::cout << "Number.cc-8265" << endl;
			string str2 = i_value->print(po, ips_n);std::cout << "Number.cc-8266" << endl;
			if(ips.im) *ips.im = str2;
			if(!po.short_multiplication && (str2 != "1" || po.base == BASE_UNICODE)) {
				if(po.spacious) {
					str2 += " * ";
				} else {
					str2 += "*";
				}
			}
			if(str2 == "1" && po.base != BASE_UNICODE) str2 = str_i;
			else if(str_i == "j") str2.insert(0, str_i);
			else str2 += str_i;std::cout << "Number.cc-8277" << endl;
			if(*ips_n.minus) {
				str += " - ";
			} else {
				str += " + ";
			}
			str += str2;std::cout << "Number.cc-8283" << endl;
		} else {
			str = i_value->print(po, ips);std::cout << "Number.cc-8285" << endl;
			if(ips.im) *ips.im = str;
			if(!po.short_multiplication && (str != "1" || po.base == BASE_UNICODE)) {
				if(po.spacious) {
					str += " * ";
				} else {
					str += "*";
				}
			}
			if(str == "1" && po.base != BASE_UNICODE) str = str_i;
			else if(str_i == "j") str.insert(0, str_i);
			else str += str_i;std::cout << "Number.cc-8296" << endl;
		}
		if(ips.num) *ips.num = str;
		return str;
	}

	long int precision = PRECISION;std::cout << "Number.cc-8302" << endl;
	if(b_approx && i_precision >= 0 && (po.preserve_precision || po.preserve_format || i_precision < PRECISION)) precision = i_precision;
	else if(b_approx && i_precision < 0 && po.preserve_precision && FROM_BIT_PRECISION(BIT_PRECISION) > precision) precision = FROM_BIT_PRECISION(BIT_PRECISION);
	else if(b_approx && i_precision < 0 && po.preserve_format && FROM_BIT_PRECISION(BIT_PRECISION) - 1 > precision) precision = FROM_BIT_PRECISION(BIT_PRECISION) - 1;
	if(po.restrict_to_parent_precision && ips.parent_precision >= 0 && ips.parent_precision < precision) precision = ips.parent_precision;
	long int precision_base = precision;std::cout << "Number.cc-8307" << endl;
	if(base != 10 && ((base >= 2 && base <= 36) || po.base == BASE_CUSTOM)) {
		Number precmax(10);std::cout << "Number.cc-8309" << endl;
		precmax.raise(precision_base);std::cout << "Number.cc-8310" << endl;
		precmax--;std::cout << "Number.cc-8311" << endl;
		precmax.log(base);std::cout << "Number.cc-8312" << endl;
		precmax.floor();std::cout << "Number.cc-8313" << endl;
		precision_base = precmax.lintValue();std::cout << "Number.cc-8314" << endl;
	}
	long int i_precision_base = precision_base;std::cout << "Number.cc-8316" << endl;
	if((i_precision < 0 && FROM_BIT_PRECISION(BIT_PRECISION) > precision) || i_precision > precision) {
		if(i_precision < 0) i_precision_base = FROM_BIT_PRECISION(BIT_PRECISION);
		else i_precision_base = i_precision;std::cout << "Number.cc-8319" << endl;
		if(po.restrict_to_parent_precision && ips.parent_precision >= 0 && ips.parent_precision < i_precision_base) i_precision_base = ips.parent_precision;
		if(base != 10 && ((base >= 2 && base <= 36) || po.base == BASE_CUSTOM)) {
			Number precmax(10);std::cout << "Number.cc-8322" << endl;
			precmax.raise(i_precision_base);std::cout << "Number.cc-8323" << endl;
			precmax--;std::cout << "Number.cc-8324" << endl;
			precmax.log(base);std::cout << "Number.cc-8325" << endl;
			precmax.floor();std::cout << "Number.cc-8326" << endl;
			i_precision_base = precmax.lintValue();std::cout << "Number.cc-8327" << endl;
		}
	}
	bool approx = isApproximate() || (ips.parent_approximate && po.restrict_to_parent_precision);std::cout << "Number.cc-8330" << endl;

	if(isInteger()) {

		long int length = mpz_sizeinbase(mpq_numref(r_value), base);std::cout << "Number.cc-8334" << endl;
		if(precision_base + min_decimals + 1000 + ::abs(po.min_exp) < length && ((approx || (base == 10 && po.min_exp != 0 && (po.restrict_fraction_length || po.number_fraction_format == FRACTION_DECIMAL || po.number_fraction_format == FRACTION_DECIMAL_EXACT))) || length > (po.base == 10 ? 1000000L : 100000L))) {
			Number nr(*this);std::cout << "Number.cc-8336" << endl;
			CALCULATOR->beginTemporaryStopMessages();std::cout << "Number.cc-8337" << endl;
			PrintOptions po2 = po;std::cout << "Number.cc-8338" << endl;
			po2.interval_display = INTERVAL_DISPLAY_MIDPOINT;std::cout << "Number.cc-8339" << endl;
			if(nr.setToFloatingPoint()) {
				CALCULATOR->endTemporaryStopMessages(true);std::cout << "Number.cc-8341" << endl;
				return nr.print(po2, ips);
			} else {
				length--;std::cout << "Number.cc-8344" << endl;
				mpz_t ivalue;std::cout << "Number.cc-8345" << endl;
				mpz_init(ivalue);std::cout << "Number.cc-8346" << endl;
				mpz_ui_pow_ui(ivalue, base, length);std::cout << "Number.cc-8347" << endl;
				Number nrexp;std::cout << "Number.cc-8348" << endl;
				nrexp.setInternal(ivalue);std::cout << "Number.cc-8349" << endl;
				if(nr.divide(nrexp)) {
					CALCULATOR->endTemporaryStopMessages();std::cout << "Number.cc-8351" << endl;
					str = nr.print(po2, ips);std::cout << "Number.cc-8352" << endl;
					if(base == 10) {
						if(ips.iexp) *ips.iexp = length;
						if(ips.exp) {
							if(ips.exp_minus) *ips.exp_minus = false;
							*ips.exp = i2s(length);std::cout << "Number.cc-8357" << endl;
						} else {
							if(po.lower_case_e) str += "e";
							else str += "E";
							str += i2s(length);std::cout << "Number.cc-8361" << endl;
						}
					} else {
						Number nrl(length);std::cout << "Number.cc-8364" << endl;
						po2.twos_complement = false;std::cout << "Number.cc-8365" << endl;
						po2.hexadecimal_twos_complement = false;std::cout << "Number.cc-8366" << endl;
						po2.binary_bits = 0;std::cout << "Number.cc-8367" << endl;
						string str_bexp = nrl.print(po2);std::cout << "Number.cc-8368" << endl;
						if(ips.exp) {
							if(ips.exp_minus) *ips.exp_minus = false;
							*ips.exp = str_bexp;std::cout << "Number.cc-8371" << endl;
						} else {
							if(po.spacious) str += " ";
							if(po.use_unicode_signs && po.multiplication_sign == MULTIPLICATION_SIGN_DOT && (!po.can_display_unicode_string_function || (*po.can_display_unicode_string_function) (SIGN_MULTIDOT, po.can_display_unicode_string_arg))) str += SIGN_MULTIDOT;
							else if(po.use_unicode_signs && (po.multiplication_sign == MULTIPLICATION_SIGN_DOT || po.multiplication_sign == MULTIPLICATION_SIGN_ALTDOT) && (!po.can_display_unicode_string_function || (*po.can_display_unicode_string_function) (SIGN_MIDDLEDOT, po.can_display_unicode_string_arg))) str += SIGN_MIDDLEDOT;
							else if(po.use_unicode_signs && po.multiplication_sign == MULTIPLICATION_SIGN_X && (!po.can_display_unicode_string_function || (*po.can_display_unicode_string_function) (SIGN_MULTIPLICATION, po.can_display_unicode_string_arg))) str += SIGN_MULTIPLICATION;
							else str += "*";
							if(po.spacious) str += " ";
							str += i2s(base);std::cout << "Number.cc-8379" << endl;
							str += "^";
							str += str_bexp;std::cout << "Number.cc-8381" << endl;
							if(ips.depth > 0) {
								str.insert(0, "(");
								str += ")";
							}
						}
					}
					return str;
				}
				CALCULATOR->endTemporaryStopMessages(true);std::cout << "Number.cc-8390" << endl;
				return "(floating point error)";
			}
		}

		if(po.base == 2 || (po.base == 16 && po.hexadecimal_twos_complement)) {
			if((po.base == 16 || po.twos_complement) && isNegative()) {
				Number nr;std::cout << "Number.cc-8397" << endl;
				unsigned int bits = po.binary_bits;std::cout << "Number.cc-8398" << endl;
				if(bits == 0) {
					nr = *this;std::cout << "Number.cc-8400" << endl;
					nr.floor();std::cout << "Number.cc-8401" << endl;
					nr++;std::cout << "Number.cc-8402" << endl;
					bits = nr.integerLength() + 1;std::cout << "Number.cc-8403" << endl;
					if(bits <= 8) bits = 8;
					else if(bits <= 16) bits = 16;
					else if(bits <= 32) bits = 32;
					else if(bits <= 64) bits = 64;
					else if(bits <= 128) bits = 128;
					else {
						bits = (unsigned int) ::ceil(::log2(bits));std::cout << "Number.cc-8410" << endl;
						bits = ::pow(2, bits);std::cout << "Number.cc-8411" << endl;
					}
				}
				nr = bits;std::cout << "Number.cc-8414" << endl;
				nr.exp2();std::cout << "Number.cc-8415" << endl;
				nr += *this;std::cout << "Number.cc-8416" << endl;
				PrintOptions po2 = po;std::cout << "Number.cc-8417" << endl;
				po2.twos_complement = false;std::cout << "Number.cc-8418" << endl;
				if(!nr.isInteger() && po2.number_fraction_format == FRACTION_DECIMAL) {
					string str = print(po2);std::cout << "Number.cc-8420" << endl;
					size_t i = str.find(po2.decimalpoint());std::cout << "Number.cc-8421" << endl;
					if(i != string::npos) {
						po2.min_decimals = str.length() - (i + po2.decimalpoint().length());std::cout << "Number.cc-8423" << endl;
						po2.max_decimals = po2.min_decimals;std::cout << "Number.cc-8424" << endl;
						po2.use_max_decimals = true;std::cout << "Number.cc-8425" << endl;
						po2.use_min_decimals = true;std::cout << "Number.cc-8426" << endl;
					} else {
						po2.max_decimals = 0;std::cout << "Number.cc-8428" << endl;
						po2.use_max_decimals = true;std::cout << "Number.cc-8429" << endl;
					}
				}
				po2.binary_bits = bits;std::cout << "Number.cc-8432" << endl;
				return nr.print(po2, ips);
			} else if(po.binary_bits == 0) {
				Number nr(*this);std::cout << "Number.cc-8435" << endl;
				nr.ceil();std::cout << "Number.cc-8436" << endl;
				unsigned int bits = nr.integerLength() + 1;std::cout << "Number.cc-8437" << endl;
				if(bits <= 8) bits = 8;
				else if(bits <= 16) bits = 16;
				else if(bits <= 32) bits = 32;
				else if(bits <= 64) bits = 64;
				else if(bits <= 128) bits = 128;
				else {
					bits = (unsigned int) ::ceil(::log2(bits));std::cout << "Number.cc-8444" << endl;
					bits = ::pow(2, bits);std::cout << "Number.cc-8445" << endl;
				}
				PrintOptions po2 = po;std::cout << "Number.cc-8447" << endl;
				po2.binary_bits = bits;std::cout << "Number.cc-8448" << endl;
				return print(po2, ips);
			}
		}

		mpz_t ivalue;std::cout << "Number.cc-8453" << endl;
		mpz_init_set(ivalue, mpq_numref(r_value));std::cout << "Number.cc-8454" << endl;
		bool neg = (mpz_sgn(ivalue) < 0);std::cout << "Number.cc-8455" << endl;
		bool rerun = false;std::cout << "Number.cc-8456" << endl;
		bool exact = true;std::cout << "Number.cc-8457" << endl;

		integer_rerun:

		string mpz_str = printMPZ(ivalue, base, false, base != BASE_ROMAN_NUMERALS && po.lower_case_numbers);std::cout << "Number.cc-8461" << endl;

		if(CALCULATOR->aborted()) return CALCULATOR->abortedMessage();

		length = mpz_str.length();std::cout << "Number.cc-8465" << endl;
		long int expo = 0;std::cout << "Number.cc-8466" << endl;
		if(base == 10 && !po.preserve_format) {
			if(length == 1 && mpz_str[0] == '0') {
				expo = 0;std::cout << "Number.cc-8469" << endl;
			} else if(length > 0 && (po.restrict_fraction_length || po.number_fraction_format == FRACTION_DECIMAL || po.number_fraction_format == FRACTION_DECIMAL_EXACT)) {
				if(po.number_fraction_format == FRACTION_FRACTIONAL) {
					long int precexp = i_precision_base;std::cout << "Number.cc-8472" << endl;
					if(precision < 8 && precexp > precision + 2) precexp = precision + 2;
					else if(precexp > precision + 3) precexp = precision + 3;
					if(exact && ((expo >= 0 && length - 1 < precexp) || (expo < 0 && expo > -PRECISION))) expo = 0;
					else expo = length - 1;std::cout << "Number.cc-8476" << endl;
				} else {
					expo = length - 1;std::cout << "Number.cc-8478" << endl;
				}
			} else if(length > 0) {
				for(long int i = length - 1; i >= 0; i--) {
					if(mpz_str[i] != '0') {
						break;
					}
					expo++;std::cout << "Number.cc-8485" << endl;
				}
			}
			if(po.min_exp == EXP_PRECISION || (po.min_exp == EXP_NONE && (expo > 100000L || expo < -100000L))) {
				long int precexp = i_precision_base;std::cout << "Number.cc-8489" << endl;
				if(precision < 8 && precexp > precision + 2) precexp = precision + 2;
				else if(precexp > precision + 3) precexp = precision + 3;
				if(exact && ((expo >= 0 && length - 1 < precexp) || (expo < 0 && expo > -PRECISION))) {
					if(precision_base < length) precision_base = length;
					expo = 0;std::cout << "Number.cc-8494" << endl;
				}
			} else if(po.min_exp < -1) {
				expo -= expo % (-po.min_exp);std::cout << "Number.cc-8497" << endl;
				if(expo < 0) expo = 0;
			} else if(po.min_exp != 0) {
				if((long int) expo > -po.min_exp && (long int) expo < po.min_exp) {
					expo = 0;std::cout << "Number.cc-8501" << endl;
				}
			} else {
				expo = 0;std::cout << "Number.cc-8504" << endl;
			}
		}
		long int decimals = expo;std::cout << "Number.cc-8507" << endl;
		long int nondecimals = length - decimals;std::cout << "Number.cc-8508" << endl;

		bool dp_added = false;std::cout << "Number.cc-8510" << endl;

		if(!rerun && mpz_sgn(ivalue) != 0) {
			long int precision2 = precision_base;std::cout << "Number.cc-8513" << endl;
			if(min_decimals > 0 && min_decimals + nondecimals > precision_base) {
				precision2 = min_decimals + nondecimals;std::cout << "Number.cc-8515" << endl;
				if(approx && precision2 > i_precision_base) precision2 = i_precision_base;
			}
			if(po.use_max_decimals && po.max_decimals >= 0 && decimals > po.max_decimals && (!approx || po.max_decimals + nondecimals < precision2) && (base == 10 && (po.restrict_fraction_length || po.number_fraction_format == FRACTION_DECIMAL || po.number_fraction_format == FRACTION_DECIMAL_EXACT))) {
				mpz_t i_rem, i_quo, i_div;std::cout << "Number.cc-8519" << endl;
				mpz_inits(i_rem, i_quo, i_div, NULL);std::cout << "Number.cc-8520" << endl;
				mpz_ui_pow_ui(i_div, (unsigned long int) base, (unsigned long int) -(po.max_decimals - expo));std::cout << "Number.cc-8521" << endl;
				mpz_fdiv_qr(i_quo, i_rem, ivalue, i_div);std::cout << "Number.cc-8522" << endl;
				if(mpz_sgn(i_rem) != 0) {
					mpz_set(ivalue, i_quo);std::cout << "Number.cc-8524" << endl;
					mpq_t q_rem, q_base_half;std::cout << "Number.cc-8525" << endl;
					mpq_inits(q_rem, q_base_half, NULL);std::cout << "Number.cc-8526" << endl;
					mpz_set(mpq_numref(q_rem), i_rem);std::cout << "Number.cc-8527" << endl;
					mpz_set(mpq_denref(q_rem), i_div);std::cout << "Number.cc-8528" << endl;
					mpz_set_si(mpq_numref(q_base_half), base);std::cout << "Number.cc-8529" << endl;
					mpq_mul(q_rem, q_rem, q_base_half);std::cout << "Number.cc-8530" << endl;
					mpz_set_ui(mpq_denref(q_base_half), 2);std::cout << "Number.cc-8531" << endl;
					int i_sign = mpq_cmp(q_rem, q_base_half);std::cout << "Number.cc-8532" << endl;
					if(po.round_halfway_to_even && mpz_even_p(ivalue)) {
						if(i_sign > 0) mpz_add_ui(ivalue, ivalue, 1);
					} else {
						if(i_sign >= 0) mpz_add_ui(ivalue, ivalue, 1);
					}
					mpq_clears(q_base_half, q_rem, NULL);std::cout << "Number.cc-8538" << endl;
					mpz_mul(ivalue, ivalue, i_div);std::cout << "Number.cc-8539" << endl;
					exact = false;std::cout << "Number.cc-8540" << endl;
					rerun = true;std::cout << "Number.cc-8541" << endl;
					mpz_clears(i_rem, i_quo, i_div, NULL);std::cout << "Number.cc-8542" << endl;
					goto integer_rerun;std::cout << "Number.cc-8543" << endl;
				}
				mpz_clears(i_rem, i_quo, i_div, NULL);std::cout << "Number.cc-8545" << endl;
			} else if(precision2 < length && (approx || (base == 10 && expo != 0 && (po.restrict_fraction_length || po.number_fraction_format == FRACTION_DECIMAL || po.number_fraction_format == FRACTION_DECIMAL_EXACT)))) {

				mpq_t qvalue;std::cout << "Number.cc-8548" << endl;
				mpq_init(qvalue);std::cout << "Number.cc-8549" << endl;
				mpz_set(mpq_numref(qvalue), ivalue);std::cout << "Number.cc-8550" << endl;

				precision2 = length - precision2;std::cout << "Number.cc-8552" << endl;

				long int p2_cd = precision2;std::cout << "Number.cc-8554" << endl;

				mpq_t q_exp;std::cout << "Number.cc-8556" << endl;
				mpq_init(q_exp);std::cout << "Number.cc-8557" << endl;

				long int p2_cd_min = 10000;std::cout << "Number.cc-8559" << endl;
				while(p2_cd_min >= 1000) {
					if(p2_cd > p2_cd_min) {
						mpz_ui_pow_ui(mpq_numref(q_exp), (unsigned long int) base, (unsigned long int) p2_cd_min);std::cout << "Number.cc-8562" << endl;
						while(p2_cd > p2_cd_min) {
							mpq_div(qvalue, qvalue, q_exp);std::cout << "Number.cc-8564" << endl;
							p2_cd -= p2_cd_min;std::cout << "Number.cc-8565" << endl;
							if(CALCULATOR->aborted()) {mpq_clears(q_exp, qvalue, NULL); mpz_clear(ivalue); return CALCULATOR->abortedMessage();}
						}
					}
					p2_cd_min = p2_cd_min / 10;std::cout << "Number.cc-8569" << endl;
				}

				mpz_ui_pow_ui(mpq_numref(q_exp), (unsigned long int) base, (unsigned long int) p2_cd);std::cout << "Number.cc-8572" << endl;
				mpq_div(qvalue, qvalue, q_exp);std::cout << "Number.cc-8573" << endl;

				mpz_t i_rem, i_quo;std::cout << "Number.cc-8575" << endl;
				mpz_inits(i_rem, i_quo, NULL);std::cout << "Number.cc-8576" << endl;
				mpz_fdiv_qr(i_quo, i_rem, mpq_numref(qvalue), mpq_denref(qvalue));std::cout << "Number.cc-8577" << endl;
				if(mpz_sgn(i_rem) != 0) {
					mpz_set(ivalue, i_quo);std::cout << "Number.cc-8579" << endl;
					mpq_t q_rem, q_base_half;std::cout << "Number.cc-8580" << endl;
					mpq_inits(q_rem, q_base_half, NULL);std::cout << "Number.cc-8581" << endl;
					mpz_set(mpq_numref(q_rem), i_rem);std::cout << "Number.cc-8582" << endl;
					mpz_set(mpq_denref(q_rem), mpq_denref(qvalue));std::cout << "Number.cc-8583" << endl;
					mpz_set_si(mpq_numref(q_base_half), base);std::cout << "Number.cc-8584" << endl;
					mpq_mul(q_rem, q_rem, q_base_half);std::cout << "Number.cc-8585" << endl;
					mpz_set_ui(mpq_denref(q_base_half), 2);std::cout << "Number.cc-8586" << endl;
					int i_sign = mpq_cmp(q_rem, q_base_half);std::cout << "Number.cc-8587" << endl;
					if(po.round_halfway_to_even && mpz_even_p(ivalue)) {
						if(i_sign > 0) mpz_add_ui(ivalue, ivalue, 1);
					} else {
						if(i_sign >= 0) mpz_add_ui(ivalue, ivalue, 1);
					}
					mpq_clears(q_base_half, q_rem, NULL);std::cout << "Number.cc-8593" << endl;
					mpz_ui_pow_ui(i_quo, (unsigned long int) base, (unsigned long int) precision2);std::cout << "Number.cc-8594" << endl;
					mpz_mul(ivalue, ivalue, i_quo);std::cout << "Number.cc-8595" << endl;
					exact = false;std::cout << "Number.cc-8596" << endl;
					rerun = true;std::cout << "Number.cc-8597" << endl;
					mpz_clears(i_rem, i_quo, NULL);std::cout << "Number.cc-8598" << endl;
					mpq_clears(q_exp, qvalue, NULL);std::cout << "Number.cc-8599" << endl;
					goto integer_rerun;std::cout << "Number.cc-8600" << endl;
				}
				mpz_clears(i_rem, i_quo, NULL);std::cout << "Number.cc-8602" << endl;
				mpq_clears(q_exp, qvalue, NULL);std::cout << "Number.cc-8603" << endl;
			}
		}

		mpz_clear(ivalue);std::cout << "Number.cc-8607" << endl;

		decimals = 0;std::cout << "Number.cc-8609" << endl;
		if(expo > 0) {
			if(po.restrict_fraction_length || po.number_fraction_format == FRACTION_DECIMAL || po.number_fraction_format == FRACTION_DECIMAL_EXACT) {
				mpz_str.insert(mpz_str.length() - expo, po.decimalpoint());std::cout << "Number.cc-8612" << endl;
				dp_added = true;std::cout << "Number.cc-8613" << endl;
				decimals = expo;std::cout << "Number.cc-8614" << endl;
			} else {
				mpz_str = mpz_str.substr(0, mpz_str.length() - expo);std::cout << "Number.cc-8616" << endl;
			}
		}

		if(base != BASE_ROMAN_NUMERALS && (po.restrict_fraction_length || po.number_fraction_format == FRACTION_DECIMAL || po.number_fraction_format == FRACTION_DECIMAL_EXACT)) {
			int pos = mpz_str.length() - 1;std::cout << "Number.cc-8621" << endl;
			for(; pos >= (int) mpz_str.length() + min_decimals - decimals; pos--) {
				if(mpz_str[pos] != '0') {
					break;
				}
			}
			if(pos + 1 < (int) mpz_str.length()) {
				decimals -= mpz_str.length() - (pos + 1);std::cout << "Number.cc-8628" << endl;
				mpz_str = mpz_str.substr(0, pos + 1);std::cout << "Number.cc-8629" << endl;
			}
			if(exact && min_decimals > decimals) {
				if(decimals <= 0) {
					mpz_str += po.decimalpoint();std::cout << "Number.cc-8633" << endl;
					dp_added = true;std::cout << "Number.cc-8634" << endl;
				}
				while(min_decimals > decimals) {
					decimals++;std::cout << "Number.cc-8637" << endl;
					mpz_str += "0";
				}
			}
			if(mpz_str[mpz_str.length() - 1] == po.decimalpoint()[0]) {
				mpz_str.erase(mpz_str.end() - 1);std::cout << "Number.cc-8642" << endl;
				dp_added = false;std::cout << "Number.cc-8643" << endl;
			}
		}

		if(!exact && po.is_approximate) *po.is_approximate = true;

		if(base != BASE_ROMAN_NUMERALS && po.show_ending_zeroes && (mpz_str.length() > 1 || mpz_str == "0") && (!exact || approx) && (!po.use_max_decimals || po.max_decimals < 0 || po.max_decimals > decimals)) {
			precision = precision_base;std::cout << "Number.cc-8650" << endl;
			precision -= mpz_str.length();std::cout << "Number.cc-8651" << endl;
			if(dp_added) {
				precision += 1;std::cout << "Number.cc-8653" << endl;
			} else if(precision > 0) {
				mpz_str += po.decimalpoint();std::cout << "Number.cc-8655" << endl;
			}
			for(; precision > 0 && (!po.use_max_decimals || po.max_decimals < 0 || po.max_decimals > decimals); precision--) {
				decimals++;std::cout << "Number.cc-8658" << endl;
				mpz_str += "0";
			}
		}

		if(ips.minus) *ips.minus = neg;
		str = format_number_string(mpz_str, base, po.base_display, !ips.minus && neg, true, po);

		if(expo != 0) {
			if(ips.iexp) *ips.iexp = expo;
			if(ips.exp) {
				if(ips.exp_minus) {
					*ips.exp_minus = expo < 0;std::cout << "Number.cc-8670" << endl;
					if(expo < 0) expo = -expo;
				}
				*ips.exp = i2s(expo);std::cout << "Number.cc-8673" << endl;
			} else {
				if(po.lower_case_e) str += "e";
				else str += "E";
				str += i2s(expo);std::cout << "Number.cc-8677" << endl;
			}
		}
		if(ips.num) *ips.num = str;

	} else if(isPlusInfinity()) {
		str += "+";
		if(po.use_unicode_signs && (!po.can_display_unicode_string_function || (*po.can_display_unicode_string_function) (SIGN_INFINITY, po.can_display_unicode_string_arg))) {
			str += SIGN_INFINITY;std::cout << "Number.cc-8685" << endl;
		} else {
			str += _("infinity");
		}
	} else if(isMinusInfinity()) {
		if(po.use_unicode_signs && (!po.can_display_unicode_string_function || (*po.can_display_unicode_string_function) (SIGN_MINUS, po.can_display_unicode_string_arg))) str += SIGN_MINUS;
		else str += "-";
		if(po.use_unicode_signs && (!po.can_display_unicode_string_function || (*po.can_display_unicode_string_function) (SIGN_INFINITY, po.can_display_unicode_string_arg))) {
			str += SIGN_INFINITY;std::cout << "Number.cc-8693" << endl;
		} else {
			str += _("infinity");
		}
	} else if(n_type == NUMBER_TYPE_FLOAT) {

		bool rerun = false;std::cout << "Number.cc-8699" << endl;
		if((base < 2 || base > 36) && po.base != BASE_CUSTOM) base = 10;
		mpfr_clear_flags();std::cout << "Number.cc-8701" << endl;

		mpfr_t f_diff, f_mid;

		bool is_interval = !mpfr_equal_p(fl_value, fu_value);std::cout << "Number.cc-8705" << endl;

		if(!is_interval) {
			if(mpfr_inf_p(fl_value)) {
				Number nr;std::cout << "Number.cc-8709" << endl;
				if(mpfr_sgn(fl_value) < 0) nr.setMinusInfinity();
				else nr.setPlusInfinity();std::cout << "Number.cc-8711" << endl;
				nr.setPrecisionAndApproximateFrom(*this);std::cout << "Number.cc-8712" << endl;
				return nr.print(po, ips);
			}
			mpfr_init2(f_mid, mpfr_get_prec(fl_value));std::cout << "Number.cc-8715" << endl;
			mpfr_set(f_mid, fl_value, MPFR_RNDN);std::cout << "Number.cc-8716" << endl;
		} else if(po.interval_display == INTERVAL_DISPLAY_INTERVAL) {
			PrintOptions po2 = po;std::cout << "Number.cc-8718" << endl;
			InternalPrintStruct ips2;std::cout << "Number.cc-8719" << endl;
			ips2.parent_approximate = ips.parent_approximate;std::cout << "Number.cc-8720" << endl;
			ips2.parent_precision = ips.parent_precision;std::cout << "Number.cc-8721" << endl;
			po2.interval_display = INTERVAL_DISPLAY_LOWER;std::cout << "Number.cc-8722" << endl;
			string str1 = print(po2, ips2);std::cout << "Number.cc-8723" << endl;
			po2.interval_display = INTERVAL_DISPLAY_UPPER;std::cout << "Number.cc-8724" << endl;
			string str2 = print(po2, ips2);std::cout << "Number.cc-8725" << endl;
			if(str1 == str2) return print(po2, ips);
			str = CALCULATOR->f_interval->preferredDisplayName(po.abbreviate_names, po.use_unicode_signs, false, po.use_reference_names, po.can_display_unicode_string_function, po.can_display_unicode_string_arg).name;std::cout << "Number.cc-8727" << endl;
			str += LEFT_PARENTHESIS;std::cout << "Number.cc-8728" << endl;
			str += str1;std::cout << "Number.cc-8729" << endl;
			str += po.comma();std::cout << "Number.cc-8730" << endl;
			str += SPACE;std::cout << "Number.cc-8731" << endl;
			str += str2;std::cout << "Number.cc-8732" << endl;
			str += RIGHT_PARENTHESIS;std::cout << "Number.cc-8733" << endl;
			if(ips.minus) *ips.minus = false;
			if(ips.num) *ips.num = str;
			return str;
		} else if(po.interval_display == INTERVAL_DISPLAY_PLUSMINUS) {
			if(mpfr_inf_p(fl_value) || mpfr_inf_p(fu_value)) {
				PrintOptions po2 = po;std::cout << "Number.cc-8739" << endl;
				po2.interval_display = INTERVAL_DISPLAY_INTERVAL;std::cout << "Number.cc-8740" << endl;
				return print(po2, ips);
			}
			mpfr_inits2(mpfr_get_prec(fl_value), f_diff, f_mid, NULL);
			mpfr_sub(f_diff, fu_value, fl_value, MPFR_RNDN);
			mpfr_div_ui(f_diff, f_diff, 2, MPFR_RNDN);
			mpfr_add(f_mid, fl_value, f_diff, MPFR_RNDN);
			mpfr_clear(f_diff);
			if(po.is_approximate) *po.is_approximate = true;
		} else if(po.interval_display == INTERVAL_DISPLAY_MIDPOINT) {
			if(mpfr_inf_p(fl_value) || mpfr_inf_p(fu_value)) {
				PrintOptions po2 = po;std::cout << "Number.cc-8751" << endl;
				po2.interval_display = INTERVAL_DISPLAY_INTERVAL;std::cout << "Number.cc-8752" << endl;
				return print(po2, ips);
			}
			mpfr_inits2(mpfr_get_prec(fl_value), f_diff, f_mid, NULL);
			mpfr_sub(f_diff, fu_value, fl_value, MPFR_RNDN);
			mpfr_div_ui(f_diff, f_diff, 2, MPFR_RNDN);
			mpfr_add(f_mid, fl_value, f_diff, MPFR_RNDN);
			mpfr_clear(f_diff);
			if(po.is_approximate) *po.is_approximate = true;
		} else if(po.interval_display == INTERVAL_DISPLAY_LOWER) {
			if(mpfr_inf_p(fl_value)) {
				Number nr;std::cout << "Number.cc-8763" << endl;
				if(mpfr_sgn(fl_value) < 0) nr.setMinusInfinity();
				else nr.setPlusInfinity();std::cout << "Number.cc-8765" << endl;
				nr.setPrecisionAndApproximateFrom(*this);std::cout << "Number.cc-8766" << endl;
				return nr.print(po, ips);
			}
			mpfr_init2(f_mid, mpfr_get_prec(fl_value));std::cout << "Number.cc-8769" << endl;
			mpfr_set(f_mid, fl_value, MPFR_RNDD);std::cout << "Number.cc-8770" << endl;
		} else if(po.interval_display == INTERVAL_DISPLAY_UPPER) {
			if(mpfr_inf_p(fu_value)) {
				Number nr;std::cout << "Number.cc-8773" << endl;
				if(mpfr_sgn(fu_value) < 0) nr.setMinusInfinity();
				else nr.setPlusInfinity();std::cout << "Number.cc-8775" << endl;
				nr.setPrecisionAndApproximateFrom(*this);std::cout << "Number.cc-8776" << endl;
				return nr.print(po, ips);
			}
			mpfr_init2(f_mid, mpfr_get_prec(fu_value));std::cout << "Number.cc-8779" << endl;
			mpfr_set(f_mid, fu_value, MPFR_RNDU);std::cout << "Number.cc-8780" << endl;
		} else {
			if(mpfr_inf_p(fl_value) || mpfr_inf_p(fu_value)) {
				PrintOptions po2 = po;std::cout << "Number.cc-8783" << endl;
				po2.interval_display = INTERVAL_DISPLAY_INTERVAL;std::cout << "Number.cc-8784" << endl;
				return print(po2, ips);
			}
			mpfr_t vl, vu, f_logl, f_base, f_log_base, f_logu;std::cout << "Number.cc-8787" << endl;
			mpfr_inits2(mpfr_get_prec(fl_value), vl, vu, f_logl, f_logu, f_base, f_log_base, NULL);std::cout << "Number.cc-8788" << endl;

			mpfr_set_si(f_base, base, MPFR_RNDN);std::cout << "Number.cc-8790" << endl;
			mpfr_log(f_log_base, f_base, MPFR_RNDN);std::cout << "Number.cc-8791" << endl;

			mpq_t base_half;std::cout << "Number.cc-8793" << endl;
			mpq_init(base_half);std::cout << "Number.cc-8794" << endl;
			mpq_set_ui(base_half, base, 2);std::cout << "Number.cc-8795" << endl;
			mpq_canonicalize(base_half);std::cout << "Number.cc-8796" << endl;

			if(mpfr_sgn(fl_value) != mpfr_sgn(fu_value)) {
				long int ilogl = i_precision_base, ilogu = i_precision_base;std::cout << "Number.cc-8799" << endl;
				if(mpfr_sgn(fl_value) < 0) {
					mpfr_neg(f_logl, fl_value, MPFR_RNDU);std::cout << "Number.cc-8801" << endl;
					mpfr_log(f_logl, f_logl, MPFR_RNDU);std::cout << "Number.cc-8802" << endl;
					mpfr_div(f_logl, f_logl, f_log_base, MPFR_RNDU);std::cout << "Number.cc-8803" << endl;
					ilogl = -mpfr_get_si(f_logl, MPFR_RNDU);std::cout << "Number.cc-8804" << endl;
					if(ilogl >= 0) {
						mpfr_ui_pow_ui(f_logl, (unsigned long int) base, (unsigned long int) ilogl + 1, MPFR_RNDU);std::cout << "Number.cc-8806" << endl;
						mpfr_mul(vl, fl_value, f_logl, MPFR_RNDD);std::cout << "Number.cc-8807" << endl;
						mpfr_neg(vl, vl, MPFR_RNDU);std::cout << "Number.cc-8808" << endl;
						if(mpfr_cmp_q(vl, base_half) <= 0) ilogl++;
					}
				}
				if(mpfr_sgn(fu_value) > 0) {
					mpfr_log(f_logl, fu_value, MPFR_RNDU);std::cout << "Number.cc-8813" << endl;
					mpfr_div(f_logl, f_logl, f_log_base, MPFR_RNDU);std::cout << "Number.cc-8814" << endl;
					ilogu = -mpfr_get_si(f_logl, MPFR_RNDU);std::cout << "Number.cc-8815" << endl;
					if(ilogu >= 0) {
						mpfr_ui_pow_ui(f_logl, (unsigned long int) base, (unsigned long int) ilogu + 1, MPFR_RNDU);std::cout << "Number.cc-8817" << endl;
						mpfr_mul(vu, fu_value, f_logl, MPFR_RNDU);std::cout << "Number.cc-8818" << endl;
						if(mpfr_cmp_q(vu, base_half) <= 0) ilogu++;
					}
				}
				mpfr_clears(vu, vl, f_logl, f_logu, f_base, f_log_base, NULL);std::cout << "Number.cc-8822" << endl;
				mpq_clear(base_half);std::cout << "Number.cc-8823" << endl;
				if(ilogu < ilogl) ilogl = ilogu;
				if(ilogl <= 1) {
					PrintOptions po2 = po;std::cout << "Number.cc-8826" << endl;
					po2.interval_display = INTERVAL_DISPLAY_PLUSMINUS;std::cout << "Number.cc-8827" << endl;
					return print(po2, ips);
				} else {
					i_precision_base = ilogl;std::cout << "Number.cc-8830" << endl;
				}
				Number nr_zero;std::cout << "Number.cc-8832" << endl;
				nr_zero.setApproximate(true);std::cout << "Number.cc-8833" << endl;
				PrintOptions po2 = po;std::cout << "Number.cc-8834" << endl;
				if(!po.use_max_decimals || po2.max_decimals < 0 || po.max_decimals > i_precision_base - 1) {

					po2.max_decimals = i_precision_base - 1;std::cout << "Number.cc-8837" << endl;
					po2.use_max_decimals = true;std::cout << "Number.cc-8838" << endl;
				}
				return nr_zero.print(po2, ips);
			}

			string str_bexp;std::cout << "Number.cc-8843" << endl;
			bool neg_bexp = false;std::cout << "Number.cc-8844" << endl;

			long int i_log_mod = 0;std::cout << "Number.cc-8846" << endl;

			float_interval_prec_rerun:

			if(CALCULATOR->aborted()) return CALCULATOR->abortedMessage();

			mpfr_set(vl, fl_value, MPFR_RNDN);std::cout << "Number.cc-8852" << endl;
			mpfr_set(vu, fu_value, MPFR_RNDN);std::cout << "Number.cc-8853" << endl;
			bool neg = (mpfr_sgn(vl) < 0);std::cout << "Number.cc-8854" << endl;
			if(neg) {
				mpfr_neg(vl, vl, MPFR_RNDN);std::cout << "Number.cc-8856" << endl;
				mpfr_neg(vu, vu, MPFR_RNDN);std::cout << "Number.cc-8857" << endl;
				mpfr_swap(vl, vu);std::cout << "Number.cc-8858" << endl;
			}

			if(base != 10 && (mpfr_get_exp(vu) > 100000L || mpfr_get_exp(vu) < -100000L)) {
				if(base == 2) {
					mpfr_log2(f_logl, vu, MPFR_RNDN);std::cout << "Number.cc-8863" << endl;
				} else {
					mpfr_log(f_logl, vu, MPFR_RNDN);std::cout << "Number.cc-8865" << endl;
					mpfr_div(f_logl, f_logl, f_log_base, MPFR_RNDN);std::cout << "Number.cc-8866" << endl;
				}
				mpfr_floor(f_logl, f_logl);std::cout << "Number.cc-8868" << endl;
				mpz_t z_exp;std::cout << "Number.cc-8869" << endl;
				mpz_init(z_exp);std::cout << "Number.cc-8870" << endl;
				mpfr_get_z(z_exp, f_logl, MPFR_RNDN);std::cout << "Number.cc-8871" << endl;
				if(mpz_sgn(z_exp) < 0) {
					mpz_neg(z_exp, z_exp);std::cout << "Number.cc-8873" << endl;
					neg_bexp = true;std::cout << "Number.cc-8874" << endl;
				}
				Number nr_bexp;std::cout << "Number.cc-8876" << endl;
				nr_bexp.setInternal(z_exp);std::cout << "Number.cc-8877" << endl;
				PrintOptions po2 = po;std::cout << "Number.cc-8878" << endl;
				po2.twos_complement = false;std::cout << "Number.cc-8879" << endl;
				po2.hexadecimal_twos_complement = false;std::cout << "Number.cc-8880" << endl;
				po2.binary_bits = 0;std::cout << "Number.cc-8881" << endl;
				str_bexp = nr_bexp.print(po2);std::cout << "Number.cc-8882" << endl;
				mpfr_ui_pow(f_logu, base, f_logl, MPFR_RNDU);std::cout << "Number.cc-8883" << endl;
				mpfr_ui_pow(f_logl, base, f_logl, MPFR_RNDD);std::cout << "Number.cc-8884" << endl;
				mpfr_div(vu, vu, f_logl, MPFR_RNDU);std::cout << "Number.cc-8885" << endl;
				mpfr_div(vl, vl, f_logu, MPFR_RNDD);std::cout << "Number.cc-8886" << endl;
			}

			if(base == 2) {
				mpfr_log2(f_logl, vu, MPFR_RNDN);std::cout << "Number.cc-8890" << endl;
			} else {
				mpfr_log(f_logl, vu, MPFR_RNDN);std::cout << "Number.cc-8892" << endl;
				mpfr_div(f_logl, f_logl, f_log_base, MPFR_RNDN);std::cout << "Number.cc-8893" << endl;
			}
			if(i_log_mod != 0) mpfr_add_si(f_logl, f_logl, i_log_mod, MPFR_RNDN);
			mpfr_floor(f_logl, f_logl);std::cout << "Number.cc-8896" << endl;

			if(precision_base > i_precision_base) precision_base = i_precision_base;

			long int expo = 0;std::cout << "Number.cc-8900" << endl;
			long int i_log = mpfr_get_si(f_logl, MPFR_RNDN);std::cout << "Number.cc-8901" << endl;
			if(!po.preserve_format && base == 10) {
				expo = i_log;std::cout << "Number.cc-8903" << endl;
				precision = precision_base;std::cout << "Number.cc-8904" << endl;
				if(po.min_exp == EXP_PRECISION || (po.min_exp == EXP_NONE && (expo > 100000L || expo < -100000L))) {
					long int precexp = i_precision_base;std::cout << "Number.cc-8906" << endl;
					if(precision < 8 && precexp > precision + 2) precexp = precision + 2;
					else if(precexp > precision + 3) precexp = precision + 3;
					if((expo > 0 && expo < precexp) || (expo < 0 && expo > -PRECISION)) {
						if(expo >= i_precision_base) i_precision_base = expo + 1;
						if(expo >= precision_base) precision_base = expo + 1;
						if(expo >= precision) precision = expo + 1;
						expo = 0;std::cout << "Number.cc-8913" << endl;
					}
				} else if(po.min_exp < -1) {
					if(expo < 0) {
						int expo_rem = (-expo) % (-po.min_exp);std::cout << "Number.cc-8917" << endl;
						if(expo_rem > 0) expo_rem = (-po.min_exp) - expo_rem;
						expo -= expo_rem;std::cout << "Number.cc-8919" << endl;
						if(expo > 0) expo = 0;
					} else if(expo > 0) {
						expo -= expo % (-po.min_exp);std::cout << "Number.cc-8922" << endl;
						if(expo < 0) expo = 0;
					}
				} else if(po.min_exp != 0) {
					if(expo > -po.min_exp && expo < po.min_exp) {
						expo = 0;std::cout << "Number.cc-8927" << endl;
					}
				} else {
					expo = 0;std::cout << "Number.cc-8930" << endl;
				}
			}
			if(po.use_max_decimals && po.max_decimals >= 0 && po.use_max_decimals && i_log - expo + po.max_decimals + 1 < i_precision_base) {
				precision_base = i_log - expo + po.max_decimals + 1;std::cout << "Number.cc-8934" << endl;
			} else {
				if(po.use_min_decimals && po.min_decimals > 0 && i_log - expo + po.min_decimals + 1 > precision_base) {
					precision_base = i_log - expo + po.min_decimals + 1;std::cout << "Number.cc-8937" << endl;
					if(precision_base > i_precision_base) precision_base = i_precision_base;
				}
				if(i_log - expo + 1 > precision_base) {
					precision_base = i_log - expo + 1;std::cout << "Number.cc-8941" << endl;
					if(precision_base > i_precision_base) precision_base = i_precision_base;
				}
			}

			mpfr_sub_si(f_logu, f_logl, precision_base - 1, MPFR_RNDU);std::cout << "Number.cc-8946" << endl;
			mpfr_sub_si(f_logl, f_logl, precision_base - 1, MPFR_RNDD);std::cout << "Number.cc-8947" << endl;
			mpfr_pow(f_logl, f_base, f_logl, MPFR_RNDD);std::cout << "Number.cc-8948" << endl;
			mpfr_pow(f_logu, f_base, f_logu, MPFR_RNDU);std::cout << "Number.cc-8949" << endl;
			mpfr_div(vl, vl, f_logu, MPFR_RNDD);std::cout << "Number.cc-8950" << endl;
			mpfr_div(vu, vu, f_logl, MPFR_RNDU);std::cout << "Number.cc-8951" << endl;
			if(mpfr_cmp(vl, vu) > 0) mpfr_swap(vl, vu);
			if(po.round_halfway_to_even) {
				mpfr_rint(vl, vl, MPFR_RNDN);std::cout << "Number.cc-8954" << endl;
				mpfr_rint(vu, vu, MPFR_RNDN);std::cout << "Number.cc-8955" << endl;
			} else {
				mpfr_round(vl, vl);std::cout << "Number.cc-8957" << endl;
				mpfr_round(vu, vu);std::cout << "Number.cc-8958" << endl;
			}
			mpz_t ivalue;std::cout << "Number.cc-8960" << endl;
			mpz_init(ivalue);std::cout << "Number.cc-8961" << endl;
			mpfr_get_z(ivalue, vu, MPFR_RNDN);std::cout << "Number.cc-8962" << endl;
			string str_u = printMPZ(ivalue, base, false, po.lower_case_numbers);std::cout << "Number.cc-8963" << endl;
			if(str_u.length() != (size_t) precision_base) {
				if(i_log_mod != 0) {
					precision_base--;std::cout << "Number.cc-8966" << endl;
					if(precision_base <= 1) {
						mpz_clear(ivalue);std::cout << "Number.cc-8968" << endl;
						mpfr_clears(vu, vl, f_logl, f_logu, f_base, f_log_base, NULL);std::cout << "Number.cc-8969" << endl;
						mpq_clear(base_half);std::cout << "Number.cc-8970" << endl;
						PrintOptions po2 = po;std::cout << "Number.cc-8971" << endl;
						po2.interval_display = INTERVAL_DISPLAY_PLUSMINUS;std::cout << "Number.cc-8972" << endl;
						return print(po2, ips);
					}
				} else {
					i_log_mod += str_u.length() - precision_base;std::cout << "Number.cc-8976" << endl;
					mpz_clear(ivalue);std::cout << "Number.cc-8977" << endl;
					goto float_interval_prec_rerun;std::cout << "Number.cc-8978" << endl;
				}
			}
			mpfr_get_z(ivalue, vl, MPFR_RNDN);std::cout << "Number.cc-8981" << endl;
			string str_l = printMPZ(ivalue, base, false, po.lower_case_numbers);std::cout << "Number.cc-8982" << endl;
			mpz_clear(ivalue);std::cout << "Number.cc-8983" << endl;

			i_precision_base = precision_base;std::cout << "Number.cc-8985" << endl;

			i_log_mod = 0;std::cout << "Number.cc-8987" << endl;

			if(str_u.length() > str_l.length()) {
				str_l.insert(0, str_u.length() - str_l.length(), '0');std::cout << "Number.cc-8990" << endl;
			}
			for(size_t i = 0; i < str_l.size(); i++) {
				if(str_l[i] != str_u[i]) {
					if(char2val(str_l[i], base) + 1 == char2val(str_u[i], base)) {
						i++;std::cout << "Number.cc-8995" << endl;
						bool do_rerun = false;std::cout << "Number.cc-8996" << endl;
						for(; i < str_l.size(); i++) {
							if(char2val(str_l[i], base) == base - 1) {
								do_rerun = true;std::cout << "Number.cc-8999" << endl;
							} else if(char2val(str_l[i], base) >= base / 2) {
								do_rerun = true;std::cout << "Number.cc-9001" << endl;
								break;
							} else {
								i--;std::cout << "Number.cc-9004" << endl;
								break;
							}
						}
						if(do_rerun) {
							if(i_precision_base == 0 || i == 0) {i_precision_base = 0; break;}
							else if(i >= (size_t) i_precision_base) i_precision_base--;
							else i_precision_base = i;std::cout << "Number.cc-9011" << endl;
							goto float_interval_prec_rerun;std::cout << "Number.cc-9012" << endl;
						}
					}
					if(i == 0 || i_precision_base <= 0) {
						i_precision_base = 0;std::cout << "Number.cc-9016" << endl;
						break;
						if(i_precision_base < 0) break;
						mpfr_mul(vu, vu, f_logl, MPFR_RNDN);std::cout << "Number.cc-9019" << endl;
						if(mpfr_cmp_ui(fu_value, 1) >= 0) {i_precision_base = 0; break;}
						i_precision_base = -1;std::cout << "Number.cc-9021" << endl;
					} else if(i >= (size_t) i_precision_base) i_precision_base--;
					else i_precision_base = i;std::cout << "Number.cc-9023" << endl;
					goto float_interval_prec_rerun;std::cout << "Number.cc-9024" << endl;
				}
			}

			if(precision_base <= 1) {
				mpfr_clears(vu, vl, f_logl, f_logu, f_base, f_log_base, NULL);std::cout << "Number.cc-9029" << endl;
				mpq_clear(base_half);std::cout << "Number.cc-9030" << endl;
				PrintOptions po2 = po;std::cout << "Number.cc-9031" << endl;
				po2.interval_display = INTERVAL_DISPLAY_PLUSMINUS;std::cout << "Number.cc-9032" << endl;
				return print(po2, ips);
			}

			if(i_precision_base <= 0) {
				if(neg) {
					mpfr_neg(vl, fl_value, MPFR_RNDN);std::cout << "Number.cc-9038" << endl;
					mpfr_neg(vu, fu_value, MPFR_RNDN);std::cout << "Number.cc-9039" << endl;
				} else {
					mpfr_set(vl, fl_value, MPFR_RNDN);std::cout << "Number.cc-9041" << endl;
					mpfr_set(vu, fu_value, MPFR_RNDN);std::cout << "Number.cc-9042" << endl;
				}
				mpfr_log(f_logl, vl, MPFR_RNDU);std::cout << "Number.cc-9044" << endl;
				mpfr_div(f_logl, f_logl, f_log_base, MPFR_RNDU);std::cout << "Number.cc-9045" << endl;
				long int ilogl = -mpfr_get_si(f_logl, MPFR_RNDU);std::cout << "Number.cc-9046" << endl;
				if(ilogl >= 0) {
					mpfr_ui_pow_ui(f_logl, (unsigned long int) base, (unsigned long int) ilogl + 1, MPFR_RNDU);std::cout << "Number.cc-9048" << endl;
					mpfr_mul(vl, fl_value, f_logl, MPFR_RNDD);std::cout << "Number.cc-9049" << endl;
					mpfr_neg(vl, vl, MPFR_RNDU);std::cout << "Number.cc-9050" << endl;
					if(mpfr_cmp_q(vl, base_half) <= 0) ilogl++;
				}
				mpfr_log(f_logl, vu, MPFR_RNDU);std::cout << "Number.cc-9053" << endl;
				mpfr_div(f_logl, f_logl, f_log_base, MPFR_RNDU);std::cout << "Number.cc-9054" << endl;
				long int ilogu = -mpfr_get_si(f_logl, MPFR_RNDU);std::cout << "Number.cc-9055" << endl;
				if(ilogu >= 0) {
					mpfr_ui_pow_ui(f_logl, (unsigned long int) base, (unsigned long int) ilogu + 1, MPFR_RNDU);std::cout << "Number.cc-9057" << endl;
					mpfr_mul(vu, fu_value, f_logl, MPFR_RNDU);std::cout << "Number.cc-9058" << endl;
					if(mpfr_cmp_q(vu, base_half) <= 0) ilogu++;
				}
				mpfr_clears(vu, vl, f_logl, f_logu, f_base, f_log_base, NULL);std::cout << "Number.cc-9061" << endl;
				mpq_clear(base_half);std::cout << "Number.cc-9062" << endl;
				if(ilogu < ilogl) ilogl = ilogu;
				if(ilogl <= 0) {
					PrintOptions po2 = po;std::cout << "Number.cc-9065" << endl;
					po2.interval_display = INTERVAL_DISPLAY_PLUSMINUS;std::cout << "Number.cc-9066" << endl;
					return print(po2, ips);
				} else {
					i_precision_base = ilogl;std::cout << "Number.cc-9069" << endl;
				}
				Number nr_zero;std::cout << "Number.cc-9071" << endl;
				nr_zero.setApproximate(true);std::cout << "Number.cc-9072" << endl;
				PrintOptions po2 = po;std::cout << "Number.cc-9073" << endl;
				if(!po.use_max_decimals || po2.max_decimals < 0 || po.max_decimals > i_precision_base - 1) {
					po2.max_decimals = i_precision_base - 1;std::cout << "Number.cc-9075" << endl;
					po2.use_max_decimals = true;std::cout << "Number.cc-9076" << endl;
				}
				return nr_zero.print(po2, ips);
			}

			str = str_u;std::cout << "Number.cc-9081" << endl;

			mpq_clear(base_half);std::cout << "Number.cc-9083" << endl;

			mpfr_clears(vl, vu, f_logl, f_logu, f_base, f_log_base, NULL);std::cout << "Number.cc-9085" << endl;

			if(po.is_approximate) *po.is_approximate = true;

			long int l10 = str.length() - i_log + expo - 1;std::cout << "Number.cc-9089" << endl;
			if(l10 > 0) {
				l10 = str.length() - l10;std::cout << "Number.cc-9091" << endl;
				if(l10 < 1) {
					str.insert(str.begin(), 1 - l10, '0');std::cout << "Number.cc-9093" << endl;
					l10 = 1;std::cout << "Number.cc-9094" << endl;
				}
				str.insert(l10, po.decimalpoint());std::cout << "Number.cc-9096" << endl;
				int l2 = 0;std::cout << "Number.cc-9097" << endl;
				while(str[str.length() - 1 - l2] == '0') {
					l2++;std::cout << "Number.cc-9099" << endl;
				}
				if(l2 > 0 && !po.show_ending_zeroes) {
					if(min_decimals > 0) {
						int decimals = str.length() - l10 - 1;std::cout << "Number.cc-9103" << endl;
						if(decimals - min_decimals < l2) l2 = decimals - min_decimals;
					}
					if(l2 > 0) str = str.substr(0, str.length() - l2);
				}
				if(str[str.length() - 1] == po.decimalpoint()[0]) {
					str.erase(str.end() - 1);std::cout << "Number.cc-9109" << endl;
				}
			} else if(l10 < 0) {
				while(l10 < 0) {
					l10++;std::cout << "Number.cc-9113" << endl;
					str += "0";
				}
			}

			if(str.empty()) {
				str = "0";
			}
			if(str[str.length() - 1] == po.decimalpoint()[0]) {
				str.erase(str.end() - 1);std::cout << "Number.cc-9122" << endl;
			}

			if(!str_bexp.empty()) {
				PrintOptions po2 = po;std::cout << "Number.cc-9126" << endl;
				po2.binary_bits = 0;std::cout << "Number.cc-9127" << endl;
				str = format_number_string(str, base, po.base_display, !ips.minus && neg, true, po2);
			} else {
				str = format_number_string(str, base, po.base_display, !ips.minus && neg, true, po);
			}

			if(expo != 0) {
				if(ips.iexp) *ips.iexp = expo;
				if(ips.exp) {
					if(ips.exp_minus) {
						*ips.exp_minus = expo < 0;std::cout << "Number.cc-9137" << endl;
						if(expo < 0) expo = -expo;
					}
					*ips.exp = i2s(expo);std::cout << "Number.cc-9140" << endl;
				} else {
					if(base == 10) {
						if(po.lower_case_e) str += "e";
						else str += "E";
						str += i2s(expo);std::cout << "Number.cc-9145" << endl;
					} else {
						if(po.spacious) str += " ";
						if(po.use_unicode_signs && po.multiplication_sign == MULTIPLICATION_SIGN_DOT && (!po.can_display_unicode_string_function || (*po.can_display_unicode_string_function) (SIGN_MULTIDOT, po.can_display_unicode_string_arg))) str += SIGN_MULTIDOT;
						else if(po.use_unicode_signs && (po.multiplication_sign == MULTIPLICATION_SIGN_DOT || po.multiplication_sign == MULTIPLICATION_SIGN_ALTDOT) && (!po.can_display_unicode_string_function || (*po.can_display_unicode_string_function) (SIGN_MIDDLEDOT, po.can_display_unicode_string_arg))) str += SIGN_MIDDLEDOT;
						else if(po.use_unicode_signs && po.multiplication_sign == MULTIPLICATION_SIGN_X && (!po.can_display_unicode_string_function || (*po.can_display_unicode_string_function) (SIGN_MULTIPLICATION, po.can_display_unicode_string_arg))) str += SIGN_MULTIPLICATION;
						else str += "*";
						if(po.spacious) str += " ";
						str += i2s(base);std::cout << "Number.cc-9153" << endl;
						str += "^";
						str += i2s(expo);std::cout << "Number.cc-9155" << endl;
						if(ips.depth > 0) {
							str.insert(0, "(");
							str += ")";
						}
					}
				}
			}

			if(!str_bexp.empty()) {
				if(ips.exp) {
					if(ips.exp_minus) *ips.exp_minus = neg_bexp;
					else if(neg_bexp) str_bexp.insert(0, "-");
					*ips.exp = str_bexp;std::cout << "Number.cc-9168" << endl;
				} else {
					if(po.spacious) str += " ";
					if(po.use_unicode_signs && po.multiplication_sign == MULTIPLICATION_SIGN_DOT && (!po.can_display_unicode_string_function || (*po.can_display_unicode_string_function) (SIGN_MULTIDOT, po.can_display_unicode_string_arg))) str += SIGN_MULTIDOT;
					else if(po.use_unicode_signs && (po.multiplication_sign == MULTIPLICATION_SIGN_DOT || po.multiplication_sign == MULTIPLICATION_SIGN_ALTDOT) && (!po.can_display_unicode_string_function || (*po.can_display_unicode_string_function) (SIGN_MIDDLEDOT, po.can_display_unicode_string_arg))) str += SIGN_MIDDLEDOT;
					else if(po.use_unicode_signs && po.multiplication_sign == MULTIPLICATION_SIGN_X && (!po.can_display_unicode_string_function || (*po.can_display_unicode_string_function) (SIGN_MULTIPLICATION, po.can_display_unicode_string_arg))) str += SIGN_MULTIPLICATION;
					else str += "*";
					if(po.spacious) str += " ";
					str += i2s(base);std::cout << "Number.cc-9176" << endl;
					str += "^";
					if(neg_bexp) {
						if(po.use_unicode_signs && (!po.can_display_unicode_string_function || (*po.can_display_unicode_string_function) (SIGN_MINUS, po.can_display_unicode_string_arg))) str += SIGN_MINUS;
						else str = "-";
					}
					str += str_bexp;std::cout << "Number.cc-9182" << endl;
					if(ips.depth > 0) {
						str.insert(0, "(");
						str += ")";
					}
				}
			}
			if(ips.minus) *ips.minus = neg;
			if(ips.num) *ips.num = str;
			if(po.is_approximate && mpfr_inexflag_p()) *po.is_approximate = true;
			testErrors(2);std::cout << "Number.cc-9192" << endl;
			return str;

		}

		precision = precision_base;std::cout << "Number.cc-9197" << endl;

		bool b_pm_zero = false;std::cout << "Number.cc-9199" << endl;

		if(mpfr_zero_p(f_mid)) {
			if(po.interval_display == INTERVAL_DISPLAY_PLUSMINUS && is_interval) {
				mpfr_t f_lunc, f_unc;std::cout << "Number.cc-9203" << endl;
				mpfr_inits2(mpfr_get_prec(f_mid), f_lunc, f_unc, NULL);std::cout << "Number.cc-9204" << endl;
				mpfr_sub(f_lunc, f_mid, fl_value, MPFR_RNDU);std::cout << "Number.cc-9205" << endl;
				mpfr_sub(f_unc, fu_value, f_mid, MPFR_RNDU);std::cout << "Number.cc-9206" << endl;
				if(mpfr_cmp(f_lunc, f_unc) > 0) mpfr_swap(f_lunc, f_unc);
				if(!mpfr_zero_p(f_unc)) {
					b_pm_zero = true;std::cout << "Number.cc-9209" << endl;
					mpfr_swap(f_unc, f_mid);std::cout << "Number.cc-9210" << endl;
				}
				mpfr_clears(f_lunc, f_unc, NULL);std::cout << "Number.cc-9212" << endl;
			}
			if(!b_pm_zero) {
				Number nr_zero;std::cout << "Number.cc-9215" << endl;
				nr_zero.setApproximate(true);std::cout << "Number.cc-9216" << endl;
				PrintOptions po2 = po;std::cout << "Number.cc-9217" << endl;
				if(!po.use_max_decimals || po2.max_decimals < 0 || po.max_decimals < i_precision_base - 1) {
					po2.max_decimals = i_precision_base - 1;std::cout << "Number.cc-9219" << endl;
					po2.use_max_decimals = true;std::cout << "Number.cc-9220" << endl;
				}
				mpfr_clear(f_mid);std::cout << "Number.cc-9222" << endl;
				return nr_zero.print(po2, ips);
			}
		}

		bool use_max_idp = false;std::cout << "Number.cc-9227" << endl;

		float_rerun:

		string str_bexp;std::cout << "Number.cc-9231" << endl;
		bool neg_bexp = false;std::cout << "Number.cc-9232" << endl;

		mpfr_t f_log, f_base, f_log_base;std::cout << "Number.cc-9234" << endl;
		mpfr_inits2(mpfr_get_prec(f_mid), f_log, f_base, f_log_base, NULL);std::cout << "Number.cc-9235" << endl;
		mpfr_set_si(f_base, base, MPFR_RNDN);std::cout << "Number.cc-9236" << endl;
		mpfr_log(f_log_base, f_base, MPFR_RNDN);std::cout << "Number.cc-9237" << endl;

		if(base != 10 && (mpfr_get_exp(f_mid) > 100000L || mpfr_get_exp(f_mid) < -100000L)) {
			bool b_neg = mpfr_sgn(f_mid) < 0;std::cout << "Number.cc-9240" << endl;
			if(b_neg) mpfr_neg(f_mid, f_mid, MPFR_RNDN);
			if(base == 2) {
				mpfr_log2(f_log, f_mid, MPFR_RNDN);std::cout << "Number.cc-9243" << endl;
			} else {
				mpfr_log(f_log, f_mid, MPFR_RNDN);std::cout << "Number.cc-9245" << endl;
				mpfr_div(f_log, f_log, f_log_base, MPFR_RNDN);std::cout << "Number.cc-9246" << endl;
			}
			mpfr_floor(f_log, f_log);std::cout << "Number.cc-9248" << endl;
			mpz_t z_exp;std::cout << "Number.cc-9249" << endl;
			mpz_init(z_exp);std::cout << "Number.cc-9250" << endl;
			mpfr_get_z(z_exp, f_log, MPFR_RNDN);std::cout << "Number.cc-9251" << endl;
			if(mpz_sgn(z_exp) < 0) {
				mpz_neg(z_exp, z_exp);std::cout << "Number.cc-9253" << endl;
				neg_bexp = true;std::cout << "Number.cc-9254" << endl;
			}
			Number nr_bexp;std::cout << "Number.cc-9256" << endl;
			nr_bexp.setInternal(z_exp);std::cout << "Number.cc-9257" << endl;
			PrintOptions po2 = po;std::cout << "Number.cc-9258" << endl;
			po2.twos_complement = false;std::cout << "Number.cc-9259" << endl;
			po2.hexadecimal_twos_complement = false;std::cout << "Number.cc-9260" << endl;
			po2.binary_bits = 0;std::cout << "Number.cc-9261" << endl;
			str_bexp = nr_bexp.print(po2);std::cout << "Number.cc-9262" << endl;
			mpfr_ui_pow(f_log, base, f_log, MPFR_RNDN);std::cout << "Number.cc-9263" << endl;
			mpfr_div(f_mid, f_mid, f_log, MPFR_RNDN);std::cout << "Number.cc-9264" << endl;
			if(b_neg) mpfr_neg(f_mid, f_mid, MPFR_RNDN);
		}
		mpfr_t v;std::cout << "Number.cc-9267" << endl;
		mpfr_init2(v, mpfr_get_prec(f_mid));std::cout << "Number.cc-9268" << endl;
		long int expo = 0;std::cout << "Number.cc-9269" << endl;
		long int l10 = 0;std::cout << "Number.cc-9270" << endl;
		mpfr_set(v, f_mid, MPFR_RNDN);std::cout << "Number.cc-9271" << endl;
		bool neg = (mpfr_sgn(v) < 0);std::cout << "Number.cc-9272" << endl;
		if(neg) mpfr_neg(v, v, MPFR_RNDN);
		mpfr_log(f_log, v, MPFR_RNDN);std::cout << "Number.cc-9274" << endl;
		mpfr_div(f_log, f_log, f_log_base, MPFR_RNDN);std::cout << "Number.cc-9275" << endl;
		mpfr_floor(f_log, f_log);std::cout << "Number.cc-9276" << endl;
		long int i_log = mpfr_get_si(f_log, MPFR_RNDN);std::cout << "Number.cc-9277" << endl;
		if((base == 10 || (isInterval() && po.interval_display == INTERVAL_DISPLAY_MIDPOINT && i_log > 0 && i_log > precision)) && (!po.preserve_format || (is_interval && po.interval_display == INTERVAL_DISPLAY_PLUSMINUS))) {
			expo = i_log;std::cout << "Number.cc-9279" << endl;
			if(po.min_exp == EXP_PRECISION || (po.min_exp == EXP_NONE && (expo > 100000L || expo < -100000L))) {
				long int precexp = i_precision_base;std::cout << "Number.cc-9281" << endl;
				if(precision < 8 && precexp > precision + 2) precexp = precision + 2;
				else if(precexp > precision + 3) precexp = precision + 3;
				if((expo > 0 && expo < precexp) || (expo < 0 && expo > -PRECISION)) {
					if(expo >= i_precision_base) i_precision_base = expo + 1;
					if(expo >= precision_base) precision_base = expo + 1;
					if(expo >= precision) precision = expo + 1;
					expo = 0;std::cout << "Number.cc-9288" << endl;
				}
			} else if(po.min_exp < -1) {
				if(expo < 0) {
					int expo_rem = (-expo) % (-po.min_exp);std::cout << "Number.cc-9292" << endl;
					if(expo_rem > 0) expo_rem = (-po.min_exp) - expo_rem;
					expo -= expo_rem;std::cout << "Number.cc-9294" << endl;
					if(expo > 0) expo = 0;
				} else if(expo > 0) {
					expo -= expo % (-po.min_exp);std::cout << "Number.cc-9297" << endl;
					if(expo < 0) expo = 0;
				}
			} else if(po.min_exp != 0) {
				if(expo > -po.min_exp && expo < po.min_exp) {
					expo = 0;std::cout << "Number.cc-9302" << endl;
				}
			} else {
				expo = 0;std::cout << "Number.cc-9305" << endl;
			}
		}
		if(!rerun && i_precision_base > precision_base && min_decimals > 0 && (po.interval_display != INTERVAL_DISPLAY_PLUSMINUS || !is_interval)) {
			if(min_decimals > precision - 1 - (i_log - expo)) {
				precision = min_decimals + 1 + (i_log - expo);std::cout << "Number.cc-9310" << endl;
				if(precision > i_precision_base) precision = i_precision_base;
				mpfr_clears(v, f_log, f_base, f_log_base, NULL);std::cout << "Number.cc-9312" << endl;
				rerun = true;std::cout << "Number.cc-9313" << endl;
				goto float_rerun;std::cout << "Number.cc-9314" << endl;
			}
		}
		if(expo == 0 && i_log > precision) {
			precision = (i_precision_base > i_log + 1) ? i_log + 1 : i_precision_base;std::cout << "Number.cc-9318" << endl;
		}
		mpfr_sub_si(f_log, f_log, ((use_max_idp || po.interval_display != INTERVAL_DISPLAY_PLUSMINUS || !is_interval) && po.use_max_decimals && po.max_decimals >= 0 && precision > po.max_decimals + i_log - expo) ? po.max_decimals + i_log - expo: precision - 1, MPFR_RNDN);std::cout << "Number.cc-9320" << endl;
		l10 = expo - mpfr_get_si(f_log, MPFR_RNDN);std::cout << "Number.cc-9321" << endl;
		mpfr_pow(f_log, f_base, f_log, MPFR_RNDN);std::cout << "Number.cc-9322" << endl;
		if((!neg && po.interval_display == INTERVAL_DISPLAY_LOWER) || (neg && po.interval_display == INTERVAL_DISPLAY_UPPER)) {
			mpfr_div(v, v, f_log, MPFR_RNDU);std::cout << "Number.cc-9324" << endl;
			mpfr_floor(v, v);std::cout << "Number.cc-9325" << endl;
		} else if((neg && po.interval_display == INTERVAL_DISPLAY_LOWER) || (!neg && po.interval_display == INTERVAL_DISPLAY_UPPER)) {
			mpfr_div(v, v, f_log, MPFR_RNDD);std::cout << "Number.cc-9327" << endl;
			mpfr_ceil(v, v);std::cout << "Number.cc-9328" << endl;
		} else if(po.round_halfway_to_even) {
			mpfr_div(v, v, f_log, MPFR_RNDN);std::cout << "Number.cc-9330" << endl;
			mpfr_rint(v, v, MPFR_RNDN);std::cout << "Number.cc-9331" << endl;
		} else {
			mpfr_div(v, v, f_log, MPFR_RNDN);std::cout << "Number.cc-9333" << endl;
			mpfr_round(v, v);std::cout << "Number.cc-9334" << endl;
		}
		mpz_t ivalue;std::cout << "Number.cc-9336" << endl;
		mpz_init(ivalue);std::cout << "Number.cc-9337" << endl;
		mpfr_get_z(ivalue, v, MPFR_RNDN);std::cout << "Number.cc-9338" << endl;

		str = printMPZ(ivalue, base, false, po.lower_case_numbers);std::cout << "Number.cc-9340" << endl;

		bool show_ending_zeroes = po.show_ending_zeroes;std::cout << "Number.cc-9342" << endl;

		string str_unc;std::cout << "Number.cc-9344" << endl;

		if(b_pm_zero) {
			if(!rerun && !po.preserve_precision && l10 > 0 && str.length() > 2) {
				precision = str.length() - l10;std::cout << "Number.cc-9348" << endl;
				if(precision < 2) precision = 2;
				mpfr_clears(v, f_log, f_base, f_log_base, NULL);std::cout << "Number.cc-9350" << endl;
				mpz_clear(ivalue);std::cout << "Number.cc-9351" << endl;
				rerun = true;std::cout << "Number.cc-9352" << endl;
				goto float_rerun;std::cout << "Number.cc-9353" << endl;
			}
			if(!po.preserve_precision) show_ending_zeroes = l10 > 0;
			str_unc = str;std::cout << "Number.cc-9356" << endl;
			str = "0";
		} else if(po.interval_display == INTERVAL_DISPLAY_PLUSMINUS && is_interval) {
			mpfr_t f_lunc, f_unc;std::cout << "Number.cc-9359" << endl;
			mpfr_inits2(mpfr_get_prec(f_mid), f_lunc, f_unc, NULL);std::cout << "Number.cc-9360" << endl;
			mpfr_div(f_lunc, fl_value, f_log, MPFR_RNDD);std::cout << "Number.cc-9361" << endl;
			mpfr_div(f_unc, fu_value, f_log, MPFR_RNDU);std::cout << "Number.cc-9362" << endl;
			if(neg) mpfr_neg(v, v, MPFR_RNDN);
			mpfr_sub(f_lunc, v, f_lunc, MPFR_RNDU);std::cout << "Number.cc-9364" << endl;
			mpfr_sub(f_unc, f_unc, v, MPFR_RNDU);std::cout << "Number.cc-9365" << endl;
			if(mpfr_cmp(f_lunc, f_unc) > 0) mpfr_swap(f_lunc, f_unc);
			if(po.round_halfway_to_even) mpfr_rint(f_unc, f_unc, MPFR_RNDN);
			else mpfr_round(f_unc, f_unc);std::cout << "Number.cc-9368" << endl;
			if(!mpfr_zero_p(f_unc)) {
				mpfr_get_z(ivalue, f_unc, MPFR_RNDN);std::cout << "Number.cc-9370" << endl;
				str_unc = printMPZ(ivalue, base, false, po.lower_case_numbers);std::cout << "Number.cc-9371" << endl;
				if(!po.preserve_precision) show_ending_zeroes = str.length() > str_unc.length() || precision == 2;
			}
			if(!rerun) {
				if(str_unc.empty() && po.use_max_decimals && po.max_decimals >= 0) {
					use_max_idp = true;std::cout << "Number.cc-9376" << endl;
					rerun = true;std::cout << "Number.cc-9377" << endl;
					goto float_rerun;std::cout << "Number.cc-9378" << endl;
				} else if(str_unc.length() > str.length()) {
					precision -= str_unc.length() - str.length();std::cout << "Number.cc-9380" << endl;
					if(precision <= 0) {
						PrintOptions po2 = po;std::cout << "Number.cc-9382" << endl;
						po2.interval_display = INTERVAL_DISPLAY_INTERVAL;std::cout << "Number.cc-9383" << endl;
						mpfr_clears(f_mid, f_lunc, f_unc, v, f_log, f_base, f_log_base, NULL);std::cout << "Number.cc-9384" << endl;
						mpz_clear(ivalue);std::cout << "Number.cc-9385" << endl;
						return print(po2, ips);
					}
					use_max_idp = true;std::cout << "Number.cc-9388" << endl;
					rerun = true;std::cout << "Number.cc-9389" << endl;
					goto float_rerun;std::cout << "Number.cc-9390" << endl;
				} else if(!po.preserve_precision && l10 > 0 && str_unc.length() > 2) {
					precision = str.length() - l10;std::cout << "Number.cc-9392" << endl;
					if(precision < (long int) str.length() - (long int) str_unc.length() + 2) precision = str.length() - str_unc.length() + 2;
					mpfr_clears(f_lunc, f_unc, v, f_log, f_base, f_log_base, NULL);std::cout << "Number.cc-9394" << endl;
					mpz_clear(ivalue);std::cout << "Number.cc-9395" << endl;
					rerun = true;std::cout << "Number.cc-9396" << endl;
					goto float_rerun;std::cout << "Number.cc-9397" << endl;
				}
			}
		}

		if(l10 > 0) {
			if(!str_unc.empty()) {
				long int l10unc = str_unc.length() - l10;std::cout << "Number.cc-9404" << endl;
				if(l10unc < 1) {
					str_unc.insert(str_unc.begin(), 1 - l10unc, '0');std::cout << "Number.cc-9406" << endl;
					l10unc = 1;std::cout << "Number.cc-9407" << endl;
				}
				str_unc.insert(l10unc, po.decimalpoint());std::cout << "Number.cc-9409" << endl;
				int l2unc = 0;std::cout << "Number.cc-9410" << endl;
				while(str_unc[str_unc.length() - 1 - l2unc] == '0') {
					l2unc++;std::cout << "Number.cc-9412" << endl;
				}
				if(l2unc > 0 && !show_ending_zeroes) {
					if(min_decimals > 0) {
						int decimals = str_unc.length() - l10unc - 1;std::cout << "Number.cc-9416" << endl;
						if(decimals - min_decimals < l2unc) l2unc = decimals - min_decimals;
					}
					if(l2unc > 0) str_unc = str_unc.substr(0, str_unc.length() - l2unc);
				}
				if(str_unc[str_unc.length() - 1] == po.decimalpoint()[0]) {
					str_unc.erase(str_unc.end() - 1);std::cout << "Number.cc-9422" << endl;
				}
			}
			l10 = str.length() - l10;std::cout << "Number.cc-9425" << endl;
			if(l10 < 1) {
				str.insert(str.begin(), 1 - l10, '0');std::cout << "Number.cc-9427" << endl;
				l10 = 1;std::cout << "Number.cc-9428" << endl;
			}
			str.insert(l10, po.decimalpoint());std::cout << "Number.cc-9430" << endl;
			int l2 = 0;std::cout << "Number.cc-9431" << endl;
			while(str[str.length() - 1 - l2] == '0') {
				l2++;std::cout << "Number.cc-9433" << endl;
			}
			if(l2 > 0 && !show_ending_zeroes) {
				if(min_decimals > 0) {
					int decimals = str.length() - l10 - 1;std::cout << "Number.cc-9437" << endl;
					if(decimals - min_decimals < l2) l2 = decimals - min_decimals;
				}
				if(l2 > 0) str = str.substr(0, str.length() - l2);
			}
			if(str[str.length() - 1] == po.decimalpoint()[0]) {
				str.erase(str.end() - 1);std::cout << "Number.cc-9443" << endl;
			}
		} else if(l10 < 0) {
			while(l10 < 0) {
				l10++;std::cout << "Number.cc-9447" << endl;
				str += "0";
				if(!str_unc.empty()) str_unc += "0";
			}
		}

		if(str.empty()) {
			str = "0";
		}
		if(str[str.length() - 1] == po.decimalpoint()[0]) {
			str.erase(str.end() - 1);std::cout << "Number.cc-9457" << endl;
		}

		if(!str_bexp.empty()) {
			PrintOptions po2 = po;std::cout << "Number.cc-9461" << endl;
			po2.binary_bits = 0;std::cout << "Number.cc-9462" << endl;
			str = format_number_string(str, base, po.base_display, !ips.minus && neg, true, po2);
			if(!str_unc.empty()) str_unc = format_number_string(str_unc, base, po.base_display, false, true, po2);
		} else {
			str = format_number_string(str, base, po.base_display, !ips.minus && neg, true, po);
			if(!str_unc.empty()) str_unc = format_number_string(str_unc, base, po.base_display, false, true, po);
		}

		if(expo != 0) {
			if(ips.iexp) *ips.iexp = expo;
			if(ips.exp) {
				if(ips.exp_minus) {
					*ips.exp_minus = expo < 0;std::cout << "Number.cc-9474" << endl;
					if(expo < 0) expo = -expo;
				}
				*ips.exp = i2s(expo);std::cout << "Number.cc-9477" << endl;
			} else {

				if(!b_pm_zero) {
					if(base == 10) {
						if(po.lower_case_e) str += "e";
						else str += "E";
						str += i2s(expo);std::cout << "Number.cc-9484" << endl;
					} else {
						if(po.spacious) str += " ";
						if(po.use_unicode_signs && po.multiplication_sign == MULTIPLICATION_SIGN_DOT && (!po.can_display_unicode_string_function || (*po.can_display_unicode_string_function) (SIGN_MULTIDOT, po.can_display_unicode_string_arg))) str += SIGN_MULTIDOT;
						else if(po.use_unicode_signs && (po.multiplication_sign == MULTIPLICATION_SIGN_DOT || po.multiplication_sign == MULTIPLICATION_SIGN_ALTDOT) && (!po.can_display_unicode_string_function || (*po.can_display_unicode_string_function) (SIGN_MIDDLEDOT, po.can_display_unicode_string_arg))) str += SIGN_MIDDLEDOT;
						else if(po.use_unicode_signs && po.multiplication_sign == MULTIPLICATION_SIGN_X && (!po.can_display_unicode_string_function || (*po.can_display_unicode_string_function) (SIGN_MULTIPLICATION, po.can_display_unicode_string_arg))) str += SIGN_MULTIPLICATION;
						else str += "*";
						if(po.spacious) str += " ";
						str += i2s(base);std::cout << "Number.cc-9492" << endl;
						str += "^";
						str += i2s(expo);std::cout << "Number.cc-9494" << endl;
						if(ips.depth > 0) {
							str.insert(0, "(");
							str += ")";
						}
					}
				}
				if(!str_unc.empty()) {
					if(base == 10) {
						if(po.lower_case_e) str_unc += "e";
						else str_unc += "E";
						str_unc += i2s(expo);std::cout << "Number.cc-9505" << endl;
					} else {
						if(po.spacious) str += " ";
						if(po.use_unicode_signs && po.multiplication_sign == MULTIPLICATION_SIGN_DOT && (!po.can_display_unicode_string_function || (*po.can_display_unicode_string_function) (SIGN_MULTIDOT, po.can_display_unicode_string_arg))) str += SIGN_MULTIDOT;
						else if(po.use_unicode_signs && (po.multiplication_sign == MULTIPLICATION_SIGN_DOT || po.multiplication_sign == MULTIPLICATION_SIGN_ALTDOT) && (!po.can_display_unicode_string_function || (*po.can_display_unicode_string_function) (SIGN_MIDDLEDOT, po.can_display_unicode_string_arg))) str += SIGN_MIDDLEDOT;
						else if(po.use_unicode_signs && po.multiplication_sign == MULTIPLICATION_SIGN_X && (!po.can_display_unicode_string_function || (*po.can_display_unicode_string_function) (SIGN_MULTIPLICATION, po.can_display_unicode_string_arg))) str += SIGN_MULTIPLICATION;
						else str += "*";
						if(po.spacious) str += " ";
						str += i2s(base);std::cout << "Number.cc-9513" << endl;
						str += "^";
						str += i2s(expo);std::cout << "Number.cc-9515" << endl;
						if(ips.depth > 0) {
							str.insert(0, "(");
							str += ")";
						}
					}
				}
			}
		}

		if(!str_unc.empty()) {
			str += SIGN_PLUSMINUS;std::cout << "Number.cc-9526" << endl;
			str += str_unc;std::cout << "Number.cc-9527" << endl;
		}
		if(!str_bexp.empty()) {
			if(ips.exp) {
				if(ips.exp_minus) *ips.exp_minus = neg_bexp;
				else if(neg_bexp) str_bexp.insert(0, "-");
				*ips.exp = str_bexp;std::cout << "Number.cc-9533" << endl;
			} else {
				if(po.spacious) str += " ";
				if(po.use_unicode_signs && po.multiplication_sign == MULTIPLICATION_SIGN_DOT && (!po.can_display_unicode_string_function || (*po.can_display_unicode_string_function) (SIGN_MULTIDOT, po.can_display_unicode_string_arg))) str += SIGN_MULTIDOT;
				else if(po.use_unicode_signs && (po.multiplication_sign == MULTIPLICATION_SIGN_DOT || po.multiplication_sign == MULTIPLICATION_SIGN_ALTDOT) && (!po.can_display_unicode_string_function || (*po.can_display_unicode_string_function) (SIGN_MIDDLEDOT, po.can_display_unicode_string_arg))) str += SIGN_MIDDLEDOT;
				else if(po.use_unicode_signs && po.multiplication_sign == MULTIPLICATION_SIGN_X && (!po.can_display_unicode_string_function || (*po.can_display_unicode_string_function) (SIGN_MULTIPLICATION, po.can_display_unicode_string_arg))) str += SIGN_MULTIPLICATION;
				else str += "*";
				if(po.spacious) str += " ";
				str += i2s(base);std::cout << "Number.cc-9541" << endl;
				str += "^";
				if(neg_bexp) {
					if(po.use_unicode_signs && (!po.can_display_unicode_string_function || (*po.can_display_unicode_string_function) (SIGN_MINUS, po.can_display_unicode_string_arg))) str += SIGN_MINUS;
					else str = "-";
				}
				str += str_bexp;std::cout << "Number.cc-9547" << endl;
				if(ips.depth > 0) {
					str.insert(0, "(");
					str += ")";
				}
			}
		}
		if(ips.minus) *ips.minus = neg;
		if(ips.num) *ips.num = str;
		mpfr_clears(f_mid, v, f_log, f_base, f_log_base, NULL);std::cout << "Number.cc-9556" << endl;
		mpz_clear(ivalue);std::cout << "Number.cc-9557" << endl;
		if(po.is_approximate && mpfr_inexflag_p()) *po.is_approximate = true;
		testErrors(2);std::cout << "Number.cc-9559" << endl;

	} else if(base != BASE_ROMAN_NUMERALS && (po.number_fraction_format == FRACTION_DECIMAL || po.number_fraction_format == FRACTION_DECIMAL_EXACT)) {

		long int numlength = mpz_sizeinbase(mpq_numref(r_value), base);std::cout << "Number.cc-9563" << endl;
		long int denlength = mpz_sizeinbase(mpq_denref(r_value), base);std::cout << "Number.cc-9564" << endl;
		if(precision_base + min_decimals + 1000 + ::abs(po.min_exp) < labs(numlength - denlength) && (approx || (po.min_exp != 0 && base == 10) || labs(numlength - denlength) > (po.base == 10 ? 1000000L : 100000L))) {
			long int length = labs(numlength - denlength);std::cout << "Number.cc-9566" << endl;
			Number nr(*this);std::cout << "Number.cc-9567" << endl;
			PrintOptions po2 = po;std::cout << "Number.cc-9568" << endl;
			po2.interval_display = INTERVAL_DISPLAY_MIDPOINT;std::cout << "Number.cc-9569" << endl;
			CALCULATOR->beginTemporaryStopMessages();std::cout << "Number.cc-9570" << endl;
			if(nr.setToFloatingPoint()) {
				CALCULATOR->endTemporaryStopMessages(true);std::cout << "Number.cc-9572" << endl;
				return nr.print(po2, ips);
			} else {
				mpz_t ivalue;std::cout << "Number.cc-9575" << endl;
				mpz_init(ivalue);std::cout << "Number.cc-9576" << endl;
				mpz_ui_pow_ui(ivalue, base, length);std::cout << "Number.cc-9577" << endl;
				Number nrexp;std::cout << "Number.cc-9578" << endl;
				nrexp.setInternal(ivalue);std::cout << "Number.cc-9579" << endl;
				if(nr.divide(nrexp)) {
					CALCULATOR->endTemporaryStopMessages();std::cout << "Number.cc-9581" << endl;
					str = nr.print(po2, ips);std::cout << "Number.cc-9582" << endl;
					if(base == 10) {
						if(ips.iexp) *ips.iexp = length;
						if(ips.exp) {
							if(ips.exp_minus) *ips.exp_minus = false;
							*ips.exp = i2s(length);std::cout << "Number.cc-9587" << endl;
						} else {
							if(po.lower_case_e) str += "e";
							else str += "E";
							str += i2s(length);std::cout << "Number.cc-9591" << endl;
						}
					} else {
						Number nrl(length);std::cout << "Number.cc-9594" << endl;
						po2.twos_complement = false;std::cout << "Number.cc-9595" << endl;
						po2.hexadecimal_twos_complement = false;std::cout << "Number.cc-9596" << endl;
						po2.binary_bits = 0;std::cout << "Number.cc-9597" << endl;
						string str_bexp = nrl.print(po2);std::cout << "Number.cc-9598" << endl;
						if(ips.exp) {
							if(ips.exp_minus) *ips.exp_minus = false;
							*ips.exp = str_bexp;std::cout << "Number.cc-9601" << endl;
						} else {
							if(po.spacious) str += " ";
							if(po.use_unicode_signs && po.multiplication_sign == MULTIPLICATION_SIGN_DOT && (!po.can_display_unicode_string_function || (*po.can_display_unicode_string_function) (SIGN_MULTIDOT, po.can_display_unicode_string_arg))) str += SIGN_MULTIDOT;
							else if(po.use_unicode_signs && (po.multiplication_sign == MULTIPLICATION_SIGN_DOT || po.multiplication_sign == MULTIPLICATION_SIGN_ALTDOT) && (!po.can_display_unicode_string_function || (*po.can_display_unicode_string_function) (SIGN_MIDDLEDOT, po.can_display_unicode_string_arg))) str += SIGN_MIDDLEDOT;
							else if(po.use_unicode_signs && po.multiplication_sign == MULTIPLICATION_SIGN_X && (!po.can_display_unicode_string_function || (*po.can_display_unicode_string_function) (SIGN_MULTIPLICATION, po.can_display_unicode_string_arg))) str += SIGN_MULTIPLICATION;
							else str += "*";
							if(po.spacious) str += " ";
							str += i2s(base);std::cout << "Number.cc-9609" << endl;
							str += "^";
							str += str_bexp;std::cout << "Number.cc-9611" << endl;
							if(ips.depth > 0) {
								str.insert(0, "(");
								str += ")";
							}
						}
					}
					return str;
				}
				CALCULATOR->endTemporaryStopMessages(true);std::cout << "Number.cc-9620" << endl;
				return "(floating point error)";
			}
		}

		mpz_t num, d, remainder, remainder2, exp;std::cout << "Number.cc-9625" << endl;
		mpz_inits(num, d, remainder, remainder2, exp, NULL);std::cout << "Number.cc-9626" << endl;
		mpz_set(d, mpq_denref(r_value));std::cout << "Number.cc-9627" << endl;
		mpz_set(num, mpq_numref(r_value));std::cout << "Number.cc-9628" << endl;
		bool neg = (mpz_sgn(num) < 0);std::cout << "Number.cc-9629" << endl;
		if(neg) mpz_neg(num, num);
		mpz_tdiv_qr(num, remainder, num, d);std::cout << "Number.cc-9631" << endl;
		bool exact = (mpz_sgn(remainder) == 0);std::cout << "Number.cc-9632" << endl;
		vector<mpz_t*> remainders;std::cout << "Number.cc-9633" << endl;
		bool started = false;std::cout << "Number.cc-9634" << endl;
		long int expo = 0;std::cout << "Number.cc-9635" << endl;
		long int precision2 = precision_base;std::cout << "Number.cc-9636" << endl;
		int num_sign = mpz_sgn(num);std::cout << "Number.cc-9637" << endl;
		long int min_l10 = 0, max_l10 = 0;std::cout << "Number.cc-9638" << endl;
		bool applied_expo = false;std::cout << "Number.cc-9639" << endl;
		bool try_infinite_series = po.indicate_infinite_series && po.number_fraction_format != FRACTION_DECIMAL_EXACT && !approx && (!po.use_max_decimals || po.max_decimals < 0 || po.max_decimals >= 3);
		long int min_decimals_bak = min_decimals;std::cout << "Number.cc-9641" << endl;

		if(num_sign != 0) {

			str = printMPZ(num, base, false);std::cout << "Number.cc-9645" << endl;

			if(CALCULATOR->aborted()) {mpz_clears(num, d, remainder, remainder2, exp, NULL); return CALCULATOR->abortedMessage();}

			long int length = str.length();std::cout << "Number.cc-9649" << endl;
			if(base != 10 || po.preserve_format) {
				expo = 0;std::cout << "Number.cc-9651" << endl;
			} else {
				expo = length - 1;std::cout << "Number.cc-9653" << endl;
				if(po.min_exp == EXP_PRECISION) {
					long int precexp = i_precision_base;std::cout << "Number.cc-9655" << endl;
					if(precision < 8 && precexp > precision + 2) precexp = precision + 2;
					else if(precexp > precision + 3) precexp = precision + 3;
					if((expo > 0 && expo < precexp) || (expo < 0 && expo > -PRECISION)) {
						if(expo >= precision) precision = expo + 1;
						if(expo >= precision_base) precision_base = expo + 1;
						if(expo >= precision2) precision2 = expo + 1;
						expo = 0;std::cout << "Number.cc-9662" << endl;
					}
				} else if(po.min_exp < -1) {
					expo -= expo % (-po.min_exp);std::cout << "Number.cc-9665" << endl;
					if(expo < 0) expo = 0;
				} else if(po.min_exp != 0) {
					if(expo > -po.min_exp && expo < po.min_exp) {
						expo = 0;std::cout << "Number.cc-9669" << endl;
					}
				} else {
					expo = 0;std::cout << "Number.cc-9672" << endl;
				}
			}
			long int decimals = expo;std::cout << "Number.cc-9675" << endl;
			long int nondecimals = length - decimals;std::cout << "Number.cc-9676" << endl;

			if(approx && min_decimals + nondecimals > i_precision_base) min_decimals = i_precision_base - nondecimals;

			precision2 -= length;std::cout << "Number.cc-9680" << endl;
			if(po.preserve_format) {
				precision2 += 100;std::cout << "Number.cc-9682" << endl;
			} else if(min_decimals) {
				min_l10 = (min_decimals + nondecimals) - length;std::cout << "Number.cc-9684" << endl;
				if(min_l10 > precision2) precision2 = min_l10;
				min_l10 = 0;std::cout << "Number.cc-9686" << endl;
			}

			int do_div = 0;std::cout << "Number.cc-9689" << endl;

			if(po.use_max_decimals && po.max_decimals >= 0 && decimals > po.max_decimals && (!approx || po.max_decimals - decimals < precision2)) {
				do_div = 1;std::cout << "Number.cc-9692" << endl;
			} else if(precision2 < 0 && (approx || decimals > min_decimals)) {
				do_div = 2;std::cout << "Number.cc-9694" << endl;
			}
			if(try_infinite_series && expo > 0 && do_div != 2) {
				mpz_t i_div;std::cout << "Number.cc-9697" << endl;
				mpz_init(i_div);std::cout << "Number.cc-9698" << endl;
				mpz_ui_pow_ui(i_div, 10, expo);std::cout << "Number.cc-9699" << endl;
				mpz_mul(d, d, i_div);std::cout << "Number.cc-9700" << endl;
				mpz_set(num, mpq_numref(r_value));std::cout << "Number.cc-9701" << endl;
				bool neg = (mpz_sgn(num) < 0);std::cout << "Number.cc-9702" << endl;
				if(neg) mpz_neg(num, num);
				mpz_tdiv_qr(num, remainder, num, d);std::cout << "Number.cc-9704" << endl;
				mpz_clear(i_div);std::cout << "Number.cc-9705" << endl;
				exact = false;std::cout << "Number.cc-9706" << endl;
				do_div = 0;std::cout << "Number.cc-9707" << endl;
				precision2 = precision - nondecimals;std::cout << "Number.cc-9708" << endl;
				if(min_decimals > 0) {
					if(precision2 < min_decimals) precision2 = min_decimals;
					min_l10 = min_decimals;std::cout << "Number.cc-9711" << endl;
				}
				if(po.use_max_decimals && po.max_decimals >= 0) {
					if(precision2 > po.max_decimals) precision2 = po.max_decimals;
					max_l10 = po.max_decimals;std::cout << "Number.cc-9715" << endl;
				}
				applied_expo = true;std::cout << "Number.cc-9717" << endl;
			} else if(do_div) {
				mpz_t i_rem, i_quo, i_div, i_div_pre;std::cout << "Number.cc-9719" << endl;
				mpz_inits(i_rem, i_quo, i_div, i_div_pre, NULL);std::cout << "Number.cc-9720" << endl;
				mpz_ui_pow_ui(i_div_pre, (unsigned long int) base, do_div == 1 ? (unsigned long int) -(po.max_decimals - decimals) : (unsigned long int) -precision2);std::cout << "Number.cc-9721" << endl;
				mpz_mul(i_div, i_div_pre, mpq_denref(r_value));std::cout << "Number.cc-9722" << endl;
				mpz_fdiv_qr(i_quo, i_rem, mpq_numref(r_value), i_div);std::cout << "Number.cc-9723" << endl;
				if(mpz_sgn(i_rem) != 0) {
					mpz_set(num, i_quo);std::cout << "Number.cc-9725" << endl;
					mpq_t q_rem, q_base_half;std::cout << "Number.cc-9726" << endl;
					mpq_inits(q_rem, q_base_half, NULL);std::cout << "Number.cc-9727" << endl;
					mpz_set(mpq_numref(q_rem), i_rem);std::cout << "Number.cc-9728" << endl;
					mpz_set(mpq_denref(q_rem), i_div);std::cout << "Number.cc-9729" << endl;
					mpz_set_si(mpq_numref(q_base_half), base);std::cout << "Number.cc-9730" << endl;
					mpq_mul(q_rem, q_rem, q_base_half);std::cout << "Number.cc-9731" << endl;
					mpz_set_ui(mpq_denref(q_base_half), 2);std::cout << "Number.cc-9732" << endl;
					int i_sign = mpq_cmp(q_rem, q_base_half);std::cout << "Number.cc-9733" << endl;
					if(po.round_halfway_to_even && mpz_even_p(num)) {
						if(i_sign > 0) mpz_add_ui(num, num, 1);
					} else {
						if(i_sign >= 0) mpz_add_ui(num, num, 1);
					}
					mpq_clears(q_base_half, q_rem, NULL);std::cout << "Number.cc-9739" << endl;
					mpz_mul(num, num, i_div_pre);std::cout << "Number.cc-9740" << endl;
					exact = false;std::cout << "Number.cc-9741" << endl;
					if(neg) mpz_neg(num, num);
				}
				mpz_clears(i_rem, i_quo, i_div, i_div_pre, NULL);std::cout << "Number.cc-9744" << endl;
				mpz_set_ui(remainder, 0);std::cout << "Number.cc-9745" << endl;
			}
			started = true;std::cout << "Number.cc-9747" << endl;
			if(!applied_expo && po.use_max_decimals && po.max_decimals >= 0 && precision2 > po.max_decimals - decimals) precision2 = po.max_decimals - decimals;
		}

		bool b_bak = false;std::cout << "Number.cc-9751" << endl;
		mpz_t remainder_bak, num_bak;std::cout << "Number.cc-9752" << endl;
		if(num_sign == 0 && (try_infinite_series || (po.use_max_decimals && po.max_decimals >= 0) || min_decimals > 0)) {
			mpz_init_set(remainder_bak, remainder);std::cout << "Number.cc-9754" << endl;
			mpz_init_set(num_bak, num);std::cout << "Number.cc-9755" << endl;
			b_bak = true;std::cout << "Number.cc-9756" << endl;
		}
		bool rerun = false;std::cout << "Number.cc-9758" << endl;
		int first_rem_check = 0;std::cout << "Number.cc-9759" << endl;

		rational_rerun:

		int infinite_series = 0;std::cout << "Number.cc-9763" << endl;
		long int l10 = 0;std::cout << "Number.cc-9764" << endl;
		if(rerun) {
			mpz_set(num, num_bak);std::cout << "Number.cc-9766" << endl;
			mpz_set(remainder, remainder_bak);std::cout << "Number.cc-9767" << endl;
		}

		if(po.preserve_format) precision2 += 100;
		long int prec2_begin = precision2;std::cout << "Number.cc-9771" << endl;
		mpz_t *remcopy;std::cout << "Number.cc-9772" << endl;
		while(!exact && precision2 > 0) {
			if(try_infinite_series) {
				remcopy = (mpz_t*) malloc(sizeof(mpz_t));std::cout << "Number.cc-9775" << endl;
				mpz_init_set(*remcopy, remainder);std::cout << "Number.cc-9776" << endl;
			}
			mpz_mul_si(remainder, remainder, base);std::cout << "Number.cc-9778" << endl;
			mpz_tdiv_qr(remainder, remainder2, remainder, d);std::cout << "Number.cc-9779" << endl;
			exact = (mpz_sgn(remainder2) == 0);std::cout << "Number.cc-9780" << endl;
			if(!started) {
				started = (mpz_sgn(remainder) != 0);std::cout << "Number.cc-9782" << endl;
			}
			if(started) {
				mpz_mul_si(num, num, base);std::cout << "Number.cc-9785" << endl;
				mpz_add(num, num, remainder);std::cout << "Number.cc-9786" << endl;
			}
			if(try_infinite_series) {
				if(started && first_rem_check == 0) {
					remainders.push_back(remcopy);std::cout << "Number.cc-9790" << endl;
				} else {
					if(started) first_rem_check--;
					mpz_clear(*remcopy);std::cout << "Number.cc-9793" << endl;
					free(remcopy);std::cout << "Number.cc-9794" << endl;
				}
			}
			if(CALCULATOR->aborted()) {
				if(b_bak) mpz_clears(num_bak, remainder_bak, NULL);
				mpz_clears(num, d, remainder, remainder2, exp, NULL);std::cout << "Number.cc-9799" << endl;
				return CALCULATOR->abortedMessage();
			}
			l10++;std::cout << "Number.cc-9802" << endl;
			mpz_set(remainder, remainder2);std::cout << "Number.cc-9803" << endl;
			if(try_infinite_series && !exact && started) {
				for(size_t i = 0; i < remainders.size(); i++) {
					if(CALCULATOR->aborted()) {mpz_clears(num, d, remainder, remainder2, exp, NULL); return CALCULATOR->abortedMessage();}
					if(!mpz_cmp(*remainders[i], remainder)) {
						infinite_series = remainders.size() - i;std::cout << "Number.cc-9808" << endl;
						try_infinite_series = false;std::cout << "Number.cc-9809" << endl;
						long int min_prec2_diff = precision2 - (prec2_begin - min_l10);
						long int max_prec2_diff = precision2 - (prec2_begin - max_l10);
						if(infinite_series == 1) precision2 = 3;
						else precision2 = infinite_series + 1;std::cout << "Number.cc-9813" << endl;
						if(min_l10 > 0 && precision2 < min_prec2_diff) {
							precision2 = min_prec2_diff;
							if(max_l10 > 0 && precision2 > max_prec2_diff) precision2 = max_prec2_diff;
						} else if(max_l10 > 0 && precision2 > max_prec2_diff) {
							precision2 = max_prec2_diff;
							infinite_series = 0;std::cout << "Number.cc-9819" << endl;
						}
						break;
					}
				}
			}
			if(started) {
				precision2--;std::cout << "Number.cc-9826" << endl;
			}
		}

		for(size_t i = 0; i < remainders.size(); i++) {
			mpz_clear(*remainders[i]);std::cout << "Number.cc-9831" << endl;
			free(remainders[i]);std::cout << "Number.cc-9832" << endl;
		}
		remainders.clear();std::cout << "Number.cc-9834" << endl;
		if(!exact && !infinite_series) {
			mpz_mul_si(remainder, remainder, base);std::cout << "Number.cc-9836" << endl;
			mpz_tdiv_qr(remainder, remainder2, remainder, d);std::cout << "Number.cc-9837" << endl;
			mpq_t q_rem, q_base_half;std::cout << "Number.cc-9838" << endl;
			mpq_inits(q_rem, q_base_half, NULL);std::cout << "Number.cc-9839" << endl;
			mpz_set(mpq_numref(q_rem), remainder);std::cout << "Number.cc-9840" << endl;
			mpz_set_si(mpq_numref(q_base_half), base);std::cout << "Number.cc-9841" << endl;
			mpz_set_ui(mpq_denref(q_base_half), 2);std::cout << "Number.cc-9842" << endl;
			int i_sign = mpq_cmp(q_rem, q_base_half);std::cout << "Number.cc-9843" << endl;
			if(po.round_halfway_to_even && mpz_sgn(remainder2) == 0 && mpz_even_p(num)) {
				if(i_sign > 0) mpz_add_ui(num, num, 1);
			} else {
				if(i_sign >= 0) mpz_add_ui(num, num, 1);
			}
			mpq_clears(q_base_half, q_rem, NULL);std::cout << "Number.cc-9849" << endl;
		}
		if(!exact && !infinite_series && po.number_fraction_format == FRACTION_DECIMAL_EXACT && !approx) {
			PrintOptions po2 = po;std::cout << "Number.cc-9852" << endl;
			po2.number_fraction_format = FRACTION_FRACTIONAL;
			po2.restrict_fraction_length = true;std::cout << "Number.cc-9854" << endl;
			if(b_bak) mpz_clears(num_bak, remainder_bak, NULL);
			mpz_clears(num, d, remainder, remainder2, exp, NULL);std::cout << "Number.cc-9856" << endl;
			return print(po2, ips);
		}

		str = printMPZ(num, base, false, po.lower_case_numbers);std::cout << "Number.cc-9860" << endl;
		if(base == 10 && !rerun && !po.preserve_format && !applied_expo) {
			expo = str.length() - l10 - 1;std::cout << "Number.cc-9862" << endl;
			if(po.min_exp == EXP_PRECISION || (po.min_exp == EXP_NONE && (expo > 100000L || expo < -100000L))) {
				long int precexp = i_precision_base;std::cout << "Number.cc-9864" << endl;
				if(precision < 8 && precexp > precision + 2) precexp = precision + 2;
				else if(precexp > precision + 3) precexp = precision + 3;
				if((expo > 0 && expo < precexp) || (expo < 0 && expo > -PRECISION)) {
					if(expo >= precision) precision = expo + 1;
					if(expo >= precision2) precision2 = expo + 1;
					expo = 0;std::cout << "Number.cc-9870" << endl;
				}
			} else if(po.min_exp < -1) {
				if(expo < 0) {
					int expo_rem = (-expo) % (-po.min_exp);std::cout << "Number.cc-9874" << endl;
					if(expo_rem > 0) expo_rem = (-po.min_exp) - expo_rem;
					expo -= expo_rem;std::cout << "Number.cc-9876" << endl;
					if(expo > 0) expo = 0;
				} else if(expo > 0) {
					expo -= expo % (-po.min_exp);std::cout << "Number.cc-9879" << endl;
					if(expo < 0) expo = 0;
				}
			} else if(po.min_exp != 0) {
				if(expo > -po.min_exp && expo < po.min_exp) {
					expo = 0;std::cout << "Number.cc-9884" << endl;
				}
			} else {
				expo = 0;std::cout << "Number.cc-9887" << endl;
			}
			if(expo < 0 && infinite_series > 0) {
				first_rem_check = str.length() - l10 - expo;std::cout << "Number.cc-9890" << endl;
				precision2 = precision;std::cout << "Number.cc-9891" << endl;
				try_infinite_series = true;std::cout << "Number.cc-9892" << endl;
				if(min_decimals > 0) {
					min_l10 = min_decimals + first_rem_check;std::cout << "Number.cc-9894" << endl;
					if(precision2 < min_l10) precision2 = min_l10;
				}
				if(po.use_max_decimals && po.max_decimals >= 0) {
					max_l10 = po.max_decimals + first_rem_check;std::cout << "Number.cc-9898" << endl;
					if(precision2 > max_l10) precision2 = max_l10;
				}
				rerun = true;std::cout << "Number.cc-9901" << endl;
				started = false;std::cout << "Number.cc-9902" << endl;
				goto rational_rerun;std::cout << "Number.cc-9903" << endl;
			}
		}

		if(!rerun && num_sign == 0 && expo <= 0 && po.use_max_decimals && po.max_decimals >= 0 && l10 + expo > po.max_decimals) {
			precision2 = po.max_decimals + (str.length() - l10 - expo);std::cout << "Number.cc-9908" << endl;
			try_infinite_series = false;std::cout << "Number.cc-9909" << endl;
			rerun = true;std::cout << "Number.cc-9910" << endl;
			exact = false;std::cout << "Number.cc-9911" << endl;
			started = false;std::cout << "Number.cc-9912" << endl;
			goto rational_rerun;std::cout << "Number.cc-9913" << endl;
		}
		if(!rerun && !exact && num_sign == 0 && expo <= 0 && min_decimals_bak > 0 && l10 + expo < min_decimals_bak && (!approx || (long int) str.length() < i_precision_base)) {
			min_decimals = min_decimals_bak;std::cout << "Number.cc-9916" << endl;
			precision2 = min_decimals + (str.length() - l10 - expo);std::cout << "Number.cc-9917" << endl;
			if(approx && precision2 > i_precision_base) precision2 = i_precision_base;
			rerun = true;std::cout << "Number.cc-9919" << endl;
			started = false;std::cout << "Number.cc-9920" << endl;
			try_infinite_series = po.indicate_infinite_series && po.number_fraction_format != FRACTION_DECIMAL_EXACT && !approx && (!po.use_max_decimals || po.max_decimals < 0 || po.max_decimals >= 3);
			min_l10 = precision2;std::cout << "Number.cc-9922" << endl;
			goto rational_rerun;std::cout << "Number.cc-9923" << endl;
		}

		if(po.is_approximate && !exact && !infinite_series) *po.is_approximate = true;
		if(expo != 0 && !applied_expo) {
			l10 += expo;std::cout << "Number.cc-9928" << endl;
		}
		while(l10 < 0) {
			str += '0';std::cout << "Number.cc-9931" << endl;
			l10++;std::cout << "Number.cc-9932" << endl;
		}
		if(b_bak) mpz_clears(num_bak, remainder_bak, NULL);
		mpz_clears(num, d, remainder, remainder2, exp, NULL);std::cout << "Number.cc-9935" << endl;
		if(CALCULATOR->aborted()) return CALCULATOR->abortedMessage();
		if(l10 > 0) {
			l10 = str.length() - l10;std::cout << "Number.cc-9938" << endl;
			long int padd_begin = 0;std::cout << "Number.cc-9939" << endl;
			if(l10 < 1) {
				padd_begin = -l10 + 1;std::cout << "Number.cc-9941" << endl;
				str.insert(str.begin(), 1 - l10, '0');std::cout << "Number.cc-9942" << endl;
				l10 = 1;std::cout << "Number.cc-9943" << endl;
			}
			str.insert(l10, po.decimalpoint());std::cout << "Number.cc-9945" << endl;
			int l2 = 0;std::cout << "Number.cc-9946" << endl;
			while(str[str.length() - 1 - l2] == '0') {
				l2++;std::cout << "Number.cc-9948" << endl;
			}
			int decimals = str.length() - l10 - 1;std::cout << "Number.cc-9950" << endl;
			if((!exact || approx) && !infinite_series && po.show_ending_zeroes && (int) str.length() - precision_base - 1 - padd_begin < l2) {
				l2 = str.length() - precision_base - 1 - padd_begin;std::cout << "Number.cc-9952" << endl;
				if(po.use_max_decimals && po.max_decimals >= 0 && decimals - l2 > po.max_decimals) {
					l2 = decimals - po.max_decimals;std::cout << "Number.cc-9954" << endl;
				}
				while(l2 < 0) {
					l2++;std::cout << "Number.cc-9957" << endl;
					str += '0';std::cout << "Number.cc-9958" << endl;
				}
			}
			if(l2 > 0 && !infinite_series) {
				if(min_decimals > 0 && (!approx || (!po.show_ending_zeroes && (int) str.length() - i_precision_base - 1 < l2))) {
					if(decimals - min_decimals < l2) l2 = decimals - min_decimals;
					if(approx && (int) str.length() - i_precision_base - 1 > l2) l2 = str.length() - i_precision_base - 1;
				}
				if(l2 > 0) str = str.substr(0, str.length() - l2);
			}
			if(str[str.length() - 1] == po.decimalpoint()[0]) {
				str.erase(str.end() - 1);std::cout << "Number.cc-9969" << endl;
			}
		}

		int decimals = 0;std::cout << "Number.cc-9973" << endl;
		if(l10 > 0) {
			decimals = str.length() - l10 - 1;std::cout << "Number.cc-9975" << endl;
		}

		if(str.empty()) {
			str = "0";
		}
		if(!exact && str == "0" && po.show_ending_zeroes && po.use_max_decimals && po.max_decimals >= 0 && po.max_decimals < precision_base) {
			str += po.decimalpoint();std::cout << "Number.cc-9982" << endl;
			for(; decimals < po.max_decimals; decimals++) str += '0';
		}
		if(exact && min_decimals > decimals) {
			if(decimals <= 0) {
				str += po.decimalpoint();std::cout << "Number.cc-9987" << endl;
				decimals = 0;std::cout << "Number.cc-9988" << endl;
			}
			for(; decimals < min_decimals; decimals++) {
				str += "0";
			}
		}
		if(str[str.length() - 1] == po.decimalpoint()[0]) {
			str.erase(str.end() - 1);std::cout << "Number.cc-9995" << endl;
		}
		if(infinite_series) {
			size_t i_dp = str.find(po.decimalpoint());std::cout << "Number.cc-9998" << endl;
			if(i_dp != string::npos && ((infinite_series == 1 && i_dp + po.decimalpoint().length() + 2 < str.length() - infinite_series) || (infinite_series > 1 && i_dp + po.decimalpoint().length() < str.length() - infinite_series))) {
#ifdef _WIN32
				str.insert(str.length() - (infinite_series == 1 ? 3 : infinite_series), " ");
#else
				if(po.use_unicode_signs && (!po.can_display_unicode_string_function || (*po.can_display_unicode_string_function) (" ", po.can_display_unicode_string_arg))) str.insert(str.length() - (infinite_series == 1 ? 3 : infinite_series), " ");
				else str.insert(str.length() - (infinite_series == 1 ? 3 : infinite_series), " ");
#endif
			}
			if(po.use_unicode_signs && (!po.can_display_unicode_string_function || (*po.can_display_unicode_string_function) ("…", po.can_display_unicode_string_arg))) str += "…";
			else str += "...";
		}

		str = format_number_string(str, base, po.base_display, !ips.minus && neg, true, po);

		if(expo != 0) {
			if(ips.iexp) *ips.iexp = expo;
			if(ips.exp) {
				if(ips.exp_minus) {
					*ips.exp_minus = expo < 0;std::cout << "Number.cc-10017" << endl;
					if(expo < 0) expo = -expo;
				}
				*ips.exp = i2s(expo);std::cout << "Number.cc-10020" << endl;
			} else {
				if(po.lower_case_e) str += "e";
				else str += "E";
				str += i2s(expo);std::cout << "Number.cc-10024" << endl;
			}
		}
		if(ips.minus) *ips.minus = neg;
		if(ips.num) *ips.num = str;
	} else {
		if(approx && po.show_ending_zeroes && base != BASE_ROMAN_NUMERALS) {
			PrintOptions po2 = po;std::cout << "Number.cc-10031" << endl;
			po2.number_fraction_format = FRACTION_DECIMAL;
			return print(po2, ips);
		}
		Number num, den;std::cout << "Number.cc-10035" << endl;
		num.setInternal(mpq_numref(r_value));std::cout << "Number.cc-10036" << endl;
		den.setInternal(mpq_denref(r_value));std::cout << "Number.cc-10037" << endl;
		if(isApproximate()) {
			num.setApproximate();std::cout << "Number.cc-10039" << endl;
			den.setApproximate();std::cout << "Number.cc-10040" << endl;
		}
		bool approximately_displayed = false;std::cout << "Number.cc-10042" << endl;
		PrintOptions po2 = po;std::cout << "Number.cc-10043" << endl;
		po2.is_approximate = &approximately_displayed;std::cout << "Number.cc-10044" << endl;
		str = num.print(po2, ips);std::cout << "Number.cc-10045" << endl;
		if(approximately_displayed && base != BASE_ROMAN_NUMERALS) {
			po2 = po;std::cout << "Number.cc-10047" << endl;
			po2.number_fraction_format = FRACTION_DECIMAL;
			return print(po2, ips);
		}
		if(ips.num) *ips.num = str;
		if(po.spacious) str += " ";
		if(po.use_unicode_signs && po.division_sign == DIVISION_SIGN_DIVISION && (!po.can_display_unicode_string_function || (*po.can_display_unicode_string_function) (SIGN_DIVISION, po.can_display_unicode_string_arg))) {
			str += SIGN_DIVISION;std::cout << "Number.cc-10054" << endl;
		} else if(po.use_unicode_signs && po.division_sign == DIVISION_SIGN_DIVISION_SLASH && (!po.can_display_unicode_string_function || (*po.can_display_unicode_string_function) (SIGN_DIVISION_SLASH, po.can_display_unicode_string_arg))) {
			str += SIGN_DIVISION_SLASH;std::cout << "Number.cc-10056" << endl;
		} else {
			str += "/";
		}
		if(po.spacious) str += " ";
		InternalPrintStruct ips_n = ips;std::cout << "Number.cc-10061" << endl;
		ips_n.minus = NULL;std::cout << "Number.cc-10062" << endl;
		string str2 = den.print(po2, ips_n);std::cout << "Number.cc-10063" << endl;
		if(approximately_displayed && base != BASE_ROMAN_NUMERALS) {
			po2 = po;std::cout << "Number.cc-10065" << endl;
			po2.number_fraction_format = FRACTION_DECIMAL;
			return print(po2, ips);
		}
		if(ips.den) *ips.den = str2;
		str += str2;std::cout << "Number.cc-10070" << endl;
		if(po.is_approximate && approximately_displayed) *po.is_approximate = true;
	}
	return str;
}



#endif