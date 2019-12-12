#ifndef ANNO_PROTECT_549390516
#define ANNO_PROTECT_549390516
/*
    Qalculate

    Copyright (C) 2003-2007, 2008, 2016-2019  Hanna Knutsson (hanna.knutsson@protonmail.com)
    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.
*/

#include "support.h"

#include "Calculator.h"
#include "BuiltinFunctions.h"
#include "util.h"
#include "MathStructure.h"
#include "Unit.h"
#include "Variable.h"
#include "Function.h"
#include "DataSet.h"
#include "ExpressionItem.h"
#include "Prefix.h"
#include "Number.h"
#include "QalculateDateTime.h"

// #include <locale.h>
#include "../libxml/xmlmemory.h"
#include "../libxml/parser.h"
#include <unistd.h>
#include <time.h>
#include <utime.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <queue>
#include <iostream>
#include <sstream>
#include <fstream>

#ifdef HAVE_LIBCURL
#	include <curl/curl.h>
#endif
#ifdef HAVE_ICU
#	include <unicode/ucasemap.h>
#endif

#if HAVE_UNORDERED_MAP
#	include <unordered_map>
	using std::unordered_map;
#elif 	defined(__GNUC__)
#	ifndef __has_include
#	define __has_include(x) 0
#	endif

#	if (defined(__clang__) && __has_include(<tr1/unordered_map>)) || (__GNUC__ >= 4 && __GNUC_MINOR__ >= 3)#		include <tr1/unordered_map>
		namespace Sgi = std;
#		define unordered_map std::tr1::unordered_map
#	else
#		if __GNUC__ < 3
#			include <hash_map.h>
			namespace Sgi { using ::hash_map; }; // inherit globals
#		else
#			include <ext/hash_map>
#			if __GNUC__ == 3 && __GNUC_MINOR__ == 0
				namespace Sgi = std;               // GCC 3.0
#			else
				namespace Sgi = ::__gnu_cxx;       // GCC 3.1 and later
#			endif
#		endif
#		define unordered_map Sgi::hash_map
#	endif
#else      // ...  there are other compilers, right?
	namespace Sgi = std;
#	define unordered_map Sgi::hash_map
#endif

using std::string;
using std::cout;
using std::vector;
using std::ostream;
using std::ofstream;
using std::endl;
using std::ios;
using std::ifstream;
using std::iterator;
using std::list;
using std::queue;

#define XML_GET_PREC_FROM_PROP(node, i)		    	value = xmlGetProp(node, (xmlChar*) "precision"); if(value) {i = s2i((char*) value); xmlFree(value);} else {i = -1;}
#define XML_GET_APPROX_FROM_PROP(node, b)	    	value = xmlGetProp(node, (xmlChar*) "approximate"); if(value) {b = !xmlStrcmp(value, (const xmlChar*) "true");} else {value = xmlGetProp(node, (xmlChar*) "precise"); if(value) {b = xmlStrcmp(value, (const xmlChar*) "true");} else {b = false;}} if(value) xmlFree(value);
#define XML_GET_FALSE_FROM_PROP(node, name, b)		value = xmlGetProp(node, (xmlChar*) name); if(value && !xmlStrcmp(value, (const xmlChar*) "false")) {b = false;} else {b = true;} if(value) xmlFree(value);
#define XML_GET_TRUE_FROM_PROP(node, name, b)		value = xmlGetProp(node, (xmlChar*) name); if(value && !xmlStrcmp(value, (const xmlChar*) "true")) {b = true;} else {b = false;} if(value) xmlFree(value);
#define XML_GET_BOOL_FROM_PROP(node, name, b)		value = xmlGetProp(node, (xmlChar*) name); if(value && !xmlStrcmp(value, (const xmlChar*) "false")) {b = false;} else if(value && !xmlStrcmp(value, (const xmlChar*) "true")) {b = true;} if(value) xmlFree(value);
#define XML_GET_FALSE_FROM_TEXT(node, b)	    	value = xmlNodeListGetString(doc, node->xmlChildrenNode, 1); if(value && !xmlStrcmp(value, (const xmlChar*) "false")) {b = false;} else {b = true;} if(value) xmlFree(value);
#define XML_GET_TRUE_FROM_TEXT(node, b)		    	value = xmlNodeListGetString(doc, node->xmlChildrenNode, 1); if(value && !xmlStrcmp(value, (const xmlChar*) "true")) {b = true;} else {b = false;} if(value) xmlFree(value);
#define XML_GET_BOOL_FROM_TEXT(node, b)		    	value = xmlNodeListGetString(doc, node->xmlChildrenNode, 1); if(value && !xmlStrcmp(value, (const xmlChar*) "false")) {b = false;} else if(value && !xmlStrcmp(value, (const xmlChar*) "true")) {b = true;} if(value) xmlFree(value);
#define XML_GET_STRING_FROM_PROP(node, name, str)	value = xmlGetProp(node, (xmlChar*) name); if(value) {str = (char*) value; remove_blank_ends(str); xmlFree(value);} else str = "";
#define XML_GET_STRING_FROM_TEXT(node, str)	    	value = xmlNodeListGetString(doc, node->xmlChildrenNode, 1); if(value) {str = (char*) value; remove_blank_ends(str); xmlFree(value);} else str = "";
#define XML_DO_FROM_PROP(node, name, action)		value = xmlGetProp(node, (xmlChar*) name); if(value) action((char*) value); else action(""); if(value) xmlFree(value);
#define XML_DO_FROM_TEXT(node, action)		    	value = xmlNodeListGetString(doc, node->xmlChildrenNode, 1); if(value) {action((char*) value); xmlFree(value);} else action("");
#define XML_GET_INT_FROM_PROP(node, name, i)		value = xmlGetProp(node, (xmlChar*) name); if(value) {i = s2i((char*) value); xmlFree(value);}
#define XML_GET_INT_FROM_TEXT(node, i)		    	value = xmlNodeListGetString(doc, node->xmlChildrenNode, 1); if(value) {i = s2i((char*) value); xmlFree(value);}
#define XML_GET_LOCALE_STRING_FROM_TEXT(node, str, best, next_best)	    	value = xmlNodeListGetString(doc, node->xmlChildrenNode, 1); lang = xmlNodeGetLang(node); if(!best) {if(!lang) {if(!next_best) {if(value) {str = (char*) value; remove_blank_ends(str);} else str = ""; if(locale.empty()) {best = true;}}} else {if(locale == (char*) lang) {best = true; if(value) {str = (char*) value; remove_blank_ends(str);} else str = "";} else if(!next_best && strlen((char*) lang) >= 2 && fulfilled_translation == 0 && lang[0] == localebase[0] && lang[1] == localebase[1]) {next_best = true; if(value) {str = (char*) value; remove_blank_ends(str);} else str = "";} else if(!next_best && str.empty() && value) {str = (char*) value; remove_blank_ends(str);}}} if(value) xmlFree(value); if(lang) xmlFree(lang);
#define XML_GET_LOCALE_STRING_FROM_TEXT_REQ(node, str, best, next_best)		value = xmlNodeListGetString(doc, node->xmlChildrenNode, 1); lang = xmlNodeGetLang(node); if(!best) {if(!lang) {if(!next_best) {if(value) {str = (char*) value; remove_blank_ends(str);} else str = ""; if(locale.empty()) {best = true;}}} else {if(locale == (char*) lang) {best = true; if(value) {str = (char*) value; remove_blank_ends(str);} else str = "";} else if(!next_best && strlen((char*) lang) >= 2 && fulfilled_translation == 0 && lang[0] == localebase[0] && lang[1] == localebase[1]) {next_best = true; if(value) {str = (char*) value; remove_blank_ends(str);} else str = "";} else if(!next_best && str.empty() && value && !require_translation) {str = (char*) value; remove_blank_ends(str);}}} if(value) xmlFree(value); if(lang) xmlFree(lang);

PrintOptions::PrintOptions() :
    min_exp(EXP_PRECISION),
    base(BASE_DECIMAL),
    lower_case_numbers(false),
    lower_case_e(false),
    number_fraction_format(FRACTION_DECIMAL),
    indicate_infinite_series(false),
    show_ending_zeroes(true),
    abbreviate_names(true),
    use_reference_names(false), place_units_separately(true),
    use_unit_prefixes(true), use_prefixes_for_all_units(false),
    use_prefixes_for_currencies(false),
    use_all_prefixes(false), use_denominator_prefix(true),
    negative_exponents(false), short_multiplication(true),
    limit_implicit_multiplication(false),
    allow_non_usable(false), use_unicode_signs(false),
    multiplication_sign(MULTIPLICATION_SIGN_DOT),
    division_sign(DIVISION_SIGN_DIVISION_SLASH),
    spacious(true), excessive_parenthesis(false),
    halfexp_to_sqrt(true),
    min_decimals(0),
    max_decimals(-1),
    use_min_decimals(true),
    use_max_decimals(true),
    round_halfway_to_even(false),
    improve_division_multipliers(true),
    prefix(NULL), is_approximate(NULL),
    can_display_unicode_string_function(NULL),
    can_display_unicode_string_arg(NULL),
    hide_underscore_spaces(false),
    preserve_format(false), allow_factorization(false),
    spell_out_logical_operators(false),
    restrict_to_parent_precision(true),
    restrict_fraction_length(false),
    exp_to_root(false), preserve_precision(false),
    interval_display(INTERVAL_DISPLAY_INTERVAL),
    digit_grouping(DIGIT_GROUPING_NONE),
    date_time_format(DATE_TIME_FORMAT_ISO),
    time_zone(TIME_ZONE_LOCAL),
    custom_time_zone(0),
    twos_complement(true),
    hexadecimal_twos_complement(false),
    binary_bits(0) {}

const string &PrintOptions::comma() const {if(comma_sign.empty()) return CALCULATOR->getComma(); return comma_sign;}
const string &PrintOptions::decimalpoint() const {if(decimalpoint_sign.empty()) return CALCULATOR->getDecimalPoint(); return decimalpoint_sign;}

InternalPrintStruct::InternalPrintStruct() : depth(0), power_depth(0), division_depth(0), wrap(false), num(NULL), den(NULL), re(NULL), im(NULL), exp(NULL), minus(NULL), exp_minus(NULL), parent_approximate(false), parent_precision(-1), iexp(NULL) {}

ParseOptions::ParseOptions() : variables_enabled(true), functions_enabled(true), unknowns_enabled(true), units_enabled(true), rpn(false), base(BASE_DECIMAL), limit_implicit_multiplication(false), read_precision(DONT_READ_PRECISION), dot_as_separator(false), brackets_as_parentheses(false), angle_unit(ANGLE_UNIT_NONE), unended_function(NULL), preserve_format(false), default_dataset(NULL), parsing_mode(PARSING_MODE_ADAPTIVE), twos_complement(false), hexadecimal_twos_complement(false) {}

EvaluationOptions::EvaluationOptions() : approximation(APPROXIMATION_TRY_EXACT), sync_units(true), sync_nonlinear_unit_relations(true), keep_prefixes(false), calculate_variables(true), calculate_functions(true), test_comparisons(true), isolate_x(true), expand(true), combine_divisions(false), reduce_divisions(true), allow_complex(true), allow_infinite(true), assume_denominators_nonzero(true), warn_about_denominators_assumed_nonzero(false), split_squares(true), keep_zero_units(true), auto_post_conversion(POST_CONVERSION_OPTIMAL), mixed_units_conversion(MIXED_UNITS_CONVERSION_DEFAULT), structuring(STRUCTURING_SIMPLIFY), isolate_var(NULL), do_polynomial_division(true), protected_function(NULL), complex_number_form(COMPLEX_NUMBER_FORM_RECTANGULAR), local_currency_conversion(true), transform_trigonometric_functions(true), interval_calculation(INTERVAL_CALCULATION_VARIANCE_FORMULA) {}

/*#include <time.h>
#include <sys/time.h>

struct timeval tvtime;
long int usecs, secs, usecs2, usecs3;

#define PRINT_TIME(x) gettimeofday(&tvtime, NULL); usecs2 = tvtime.tv_usec - usecs + (tvtime.tv_sec - secs) * 1000000; printf("%s %li\n", x, usecs2);
#define PRINT_TIMEDIFF(x) gettimeofday(&tvtime, NULL); printf("%s %li\n", x, tvtime.tv_usec - usecs + (tvtime.tv_sec - secs) * 1000000 - usecs2); usecs2 = tvtime.tv_usec - usecs + (tvtime.tv_sec - secs) * 1000000;
#define ADD_TIME1 gettimeofday(&tvtime, NULL); usecs2 = tvtime.tv_usec - usecs + (tvtime.tv_sec - secs) * 1000000;
#define ADD_TIME2 gettimeofday(&tvtime, NULL); usecs3 += tvtime.tv_usec - usecs + (tvtime.tv_sec - secs) * 1000000 - usecs2; */

typedef void (*CREATEPLUG_PROC)();

PlotParameters::PlotParameters() {
	auto_y_min = true;std::cout << "176" << endl;
	auto_x_min = true;std::cout << "177" << endl;
	auto_y_max = true;std::cout << "178" << endl;
	auto_x_max = true;std::cout << "179" << endl;
	y_log = false;std::cout << "180" << endl;
	x_log = false;std::cout << "181" << endl;
	y_log_base = 10;std::cout << "182" << endl;
	x_log_base = 10;std::cout << "183" << endl;
	grid = false;std::cout << "184" << endl;
	color = true;std::cout << "185" << endl;
	linewidth = -1;std::cout << "186" << endl;
	show_all_borders = false;std::cout << "187" << endl;
	legend_placement = PLOT_LEGEND_TOP_RIGHT;std::cout << "188" << endl;
}
PlotDataParameters::PlotDataParameters() {
	yaxis2 = false;std::cout << "191" << endl;
	xaxis2 = false;std::cout << "192" << endl;
	style = PLOT_STYLE_LINES;std::cout << "193" << endl;
	smoothing = PLOT_SMOOTHING_NONE;std::cout << "194" << endl;
	test_continuous = false;std::cout << "195" << endl;
}

CalculatorMessage::CalculatorMessage(string message_, MessageType type_, int cat_, int stage_) {
	mtype = type_;std::cout << "199" << endl;
	i_stage = stage_;std::cout << "200" << endl;
	i_cat = cat_;std::cout << "201" << endl;
	smessage = message_;std::cout << "202" << endl;
}
CalculatorMessage::CalculatorMessage(const CalculatorMessage &e) {
	mtype = e.type();std::cout << "205" << endl;
	i_stage = e.stage();std::cout << "206" << endl;
	i_cat = e.category();std::cout << "207" << endl;
	smessage = e.message();std::cout << "208" << endl;
}
string CalculatorMessage::message() const {
	return smessage;
}
const char* CalculatorMessage::c_message() const {
	return smessage.c_str();
}
MessageType CalculatorMessage::type() const {
	return mtype;
}
int CalculatorMessage::stage() const {
	return i_stage;
}
int CalculatorMessage::category() const {
	return i_cat;
}

void Calculator::addStringAlternative(string replacement, string standard) {
	signs.push_back(replacement);std::cout << "227" << endl;
	real_signs.push_back(standard);std::cout << "228" << endl;
}
bool Calculator::delStringAlternative(string replacement, string standard) {
	for(size_t i = 0; i < signs.size(); i++) {
		if(signs[i] == replacement && real_signs[i] == standard) {
			signs.erase(signs.begin() + i);std::cout << "233" << endl;
			real_signs.erase(real_signs.begin() + i);std::cout << "234" << endl;
			return true;
		}
	}
	return false;
}
void Calculator::addDefaultStringAlternative(string replacement, string standard) {
	default_signs.push_back(replacement);std::cout << "241" << endl;
	default_real_signs.push_back(standard);std::cout << "242" << endl;
}
bool Calculator::delDefaultStringAlternative(string replacement, string standard) {
	for(size_t i = 0; i < default_signs.size(); i++) {
		if(default_signs[i] == replacement && default_real_signs[i] == standard) {
			default_signs.erase(default_signs.begin() + i);std::cout << "247" << endl;
			default_real_signs.erase(default_real_signs.begin() + i);std::cout << "248" << endl;
			return true;
		}
	}
	return false;
}

Calculator *calculator = NULL;

MathStructure m_undefined, m_empty_vector, m_empty_matrix, m_zero, m_one, m_minus_one, m_one_i;
Number nr_zero, nr_one, nr_two, nr_three, nr_minus_one, nr_one_i, nr_minus_i, nr_half, nr_minus_half, nr_plus_inf, nr_minus_inf;
EvaluationOptions no_evaluation;
ExpressionName empty_expression_name;
extern gmp_randstate_t randstate;
#ifdef HAVE_ICU
	extern UCaseMap *ucm;
#endif


enum {
	PROC_RPN_ADD,
	PROC_RPN_SET,
	PROC_RPN_OPERATION_1,
	PROC_RPN_OPERATION_2,
	PROC_RPN_OPERATION_F,
	PROC_NO_COMMAND
};

class CalculateThread : public Thread {
  protected:
	virtual void run();
};



void autoConvert(const MathStructure &morig, MathStructure &mconv, const EvaluationOptions &eo) {
	if(!morig.containsType(STRUCT_UNIT, true)) {
		if(&mconv != &morig) mconv.set(morig);
		return;
	}
	switch(eo.auto_post_conversion) {
		case POST_CONVERSION_OPTIMAL: {
			mconv.set(CALCULATOR->convertToOptimalUnit(morig, eo, false));std::cout << "290" << endl;
			break;
		}
		case POST_CONVERSION_BASE: {
			mconv.set(CALCULATOR->convertToBaseUnits(morig, eo));std::cout << "294" << endl;
			break;
		}
		case POST_CONVERSION_OPTIMAL_SI: {
			mconv.set(CALCULATOR->convertToOptimalUnit(morig, eo, true));std::cout << "298" << endl;
			break;
		}
		default: {
			if(&mconv != &morig) mconv.set(morig);
		}
	}
	if(eo.mixed_units_conversion != MIXED_UNITS_CONVERSION_NONE) mconv.set(CALCULATOR->convertToMixedUnits(mconv, eo));
}

void CalculateThread::run() {
	enableAsynchronousCancel();std::cout << "309" << endl;
	while(true) {
		bool b_parse = true;std::cout << "311" << endl;
		if(!read<bool>(&b_parse)) break;
		void *x = NULL;std::cout << "313" << endl;
		if(!read<void *>(&x) || !x) break;
		MathStructure *mstruct = (MathStructure*) x;std::cout << "315" << endl;
		CALCULATOR->startControl();std::cout << "316" << endl;
		if(b_parse) {
			mstruct->setAborted();std::cout << "318" << endl;
			if(CALCULATOR->tmp_parsedstruct) CALCULATOR->tmp_parsedstruct->setAborted();
			//if(CALCULATOR->tmp_tostruct) CALCULATOR->tmp_tostruct->setUndefined();
			mstruct->set(CALCULATOR->calculate(CALCULATOR->expression_to_calculate, CALCULATOR->tmp_evaluationoptions, CALCULATOR->tmp_parsedstruct, CALCULATOR->tmp_tostruct, CALCULATOR->tmp_maketodivision));std::cout << "321" << endl;
		} else {
			MathStructure meval(*mstruct);std::cout << "323" << endl;
			mstruct->setAborted();std::cout << "324" << endl;
			mstruct->set(CALCULATOR->calculate(meval, CALCULATOR->tmp_evaluationoptions));std::cout << "325" << endl;
		}
		switch(CALCULATOR->tmp_proc_command) {
			case PROC_RPN_ADD: {
				CALCULATOR->RPNStackEnter(mstruct, false);std::cout << "329" << endl;
				break;
			}
			case PROC_RPN_SET: {
				CALCULATOR->setRPNRegister(CALCULATOR->tmp_rpnindex, mstruct, false);std::cout << "333" << endl;
				break;
			}
			case PROC_RPN_OPERATION_1: {
				if(CALCULATOR->RPNStackSize() > 0) {
					CALCULATOR->setRPNRegister(1, mstruct, false);std::cout << "338" << endl;
				} else {
					CALCULATOR->RPNStackEnter(mstruct, false);std::cout << "340" << endl;
				}
				break;
			}
			case PROC_RPN_OPERATION_2: {
				if(CALCULATOR->RPNStackSize() > 1) {
					CALCULATOR->deleteRPNRegister(1);std::cout << "346" << endl;
				}
				if(CALCULATOR->RPNStackSize() > 0) {
					CALCULATOR->setRPNRegister(1, mstruct, false);std::cout << "349" << endl;
				} else {
					CALCULATOR->RPNStackEnter(mstruct, false);std::cout << "351" << endl;
				}
				break;
			}
			case PROC_RPN_OPERATION_F: {
				for(size_t i = 0; (CALCULATOR->tmp_proc_registers < 0 || (int) i < CALCULATOR->tmp_proc_registers - 1) && CALCULATOR->RPNStackSize() > 1; i++) {
					CALCULATOR->deleteRPNRegister(1);std::cout << "357" << endl;
				}
				if(CALCULATOR->RPNStackSize() > 0 && CALCULATOR->tmp_proc_registers != 0) {
					CALCULATOR->setRPNRegister(1, mstruct, false);std::cout << "360" << endl;
				} else {
					CALCULATOR->RPNStackEnter(mstruct, false);std::cout << "362" << endl;
				}
				break;
			}
			case PROC_NO_COMMAND: {}
		}
		CALCULATOR->stopControl();std::cout << "368" << endl;
		CALCULATOR->b_busy = false;std::cout << "369" << endl;
	}
}

class Calculator_p {
	public:
		unordered_map<size_t, MathStructure*> id_structs;
		unordered_map<size_t, bool> ids_p;
		vector<size_t> freed_ids;
		size_t ids_i;
		Number custom_input_base, custom_output_base;
		long int custom_input_base_i;
		Unit *local_currency;
		int use_binary_prefixes;
		MathFunction *f_cis;
};

bool is_not_number(char c, int base) {
	if(c >= '0' && c <= '9') return false;
	if(base == -1) return false;
	if(base == -12) return c != 'E' && c != 'X';
	if(base <= 10) return true;
	if(base <= 36) {
		if(c >= 'a' && c < 'a' + (base - 10)) return false;
		if(c >= 'A' && c < 'A' + (base - 10)) return false;
		return true;
	}
	if(base <= 62) {
		if(c >= 'a' && c < 'a' + (base - 36)) return false;
		if(c >= 'A' && c < 'Z') return false;
		return true;
	}
	return false;
}

#define BITWISE_XOR "⊻"

Calculator::Calculator() {
	b_ignore_locale = false;std::cout << "407" << endl;

#ifdef ENABLE_NLS
	if(!b_ignore_locale) {
		bindtextdomain(GETTEXT_PACKAGE, getPackageLocaleDir().c_str());std::cout << "411" << endl;
		bind_textdomain_codeset(GETTEXT_PACKAGE, "UTF-8");
	}
#endif

	if(b_ignore_locale) {
		char *current_lc_monetary = setlocale(LC_MONETARY, "");
		if(current_lc_monetary) saved_locale = strdup(current_lc_monetary);
		else saved_locale = NULL;std::cout << "419" << endl;
		setlocale(LC_ALL, "C");
		if(saved_locale) {
			setlocale(LC_MONETARY, saved_locale);std::cout << "422" << endl;
			free(saved_locale);std::cout << "423" << endl;
			saved_locale = NULL;std::cout << "424" << endl;
		}
	} else {
		setlocale(LC_ALL, "");
	}


	gmp_randinit_default(randstate);std::cout << "431" << endl;
	gmp_randseed_ui(randstate, (unsigned long int) time(NULL));std::cout << "432" << endl;

	priv = new Calculator_p;std::cout << "434" << endl;
	priv->custom_input_base_i = 0;std::cout << "435" << endl;
	priv->ids_i = 0;std::cout << "436" << endl;
	priv->local_currency = NULL;std::cout << "437" << endl;
	priv->use_binary_prefixes = 0;std::cout << "438" << endl;

#ifdef HAVE_ICU
	UErrorCode err = U_ZERO_ERROR;std::cout << "441" << endl;
	ucm = ucasemap_open(NULL, 0, &err);std::cout << "442" << endl;
#endif

	srand(time(NULL));std::cout << "445" << endl;

	exchange_rates_time[0] = 0;std::cout << "447" << endl;
	exchange_rates_time[1] = 0;std::cout << "448" << endl;
	exchange_rates_time[2] = 0;std::cout << "449" << endl;
	exchange_rates_check_time[0] = 0;std::cout << "450" << endl;
	exchange_rates_check_time[1] = 0;std::cout << "451" << endl;
	exchange_rates_check_time[2] = 0;std::cout << "452" << endl;
	b_exchange_rates_warning_enabled = true;std::cout << "453" << endl;
	b_exchange_rates_used = 0;std::cout << "454" << endl;

	i_aborted = 0;std::cout << "456" << endl;
	b_controlled = false;std::cout << "457" << endl;
	i_timeout = 0;std::cout << "458" << endl;

	setPrecision(DEFAULT_PRECISION);std::cout << "460" << endl;
	b_interval = true;std::cout << "461" << endl;
	i_stop_interval = 0;std::cout << "462" << endl;
	i_start_interval = 0;std::cout << "463" << endl;

	b_var_units = true;std::cout << "465" << endl;

	addStringAlternative(SIGN_DIVISION, DIVISION);
	addStringAlternative(SIGN_DIVISION_SLASH, DIVISION);
	addStringAlternative("⁄", DIVISION);
	addStringAlternative(SIGN_MULTIPLICATION, MULTIPLICATION);
	addStringAlternative(SIGN_MULTIDOT, MULTIPLICATION);
	addStringAlternative(SIGN_MIDDLEDOT, MULTIPLICATION);
	addStringAlternative(SIGN_MULTIBULLET, MULTIPLICATION);
	addStringAlternative(SIGN_SMALLCIRCLE, MULTIPLICATION);
	addStringAlternative(SIGN_MINUS, MINUS);
	addStringAlternative("–", MINUS);
	addStringAlternative(SIGN_PLUS, PLUS);
	addStringAlternative(SIGN_NOT_EQUAL, " " NOT EQUALS);
	addStringAlternative(SIGN_GREATER_OR_EQUAL, GREATER EQUALS);
	addStringAlternative(SIGN_LESS_OR_EQUAL, LESS EQUALS);
	addStringAlternative(";", COMMA);
	addStringAlternative("\t", SPACE);
	addStringAlternative("\n", SPACE);
	addStringAlternative(" ", SPACE);
	addStringAlternative(" ", SPACE);
	addStringAlternative(" ", SPACE);
	addStringAlternative("**", POWER);
	addStringAlternative("↊", "X");
	addStringAlternative("↋", "E");
	addStringAlternative("∧", BITWISE_AND);
	addStringAlternative("∨", BITWISE_OR);
	addStringAlternative("¬", BITWISE_NOT);
	addStringAlternative(SIGN_MICRO, "μ");

	per_str = _("per");
	per_str_len = per_str.length();std::cout << "496" << endl;
	times_str = _("times");
	times_str_len = times_str.length();std::cout << "498" << endl;
	plus_str = _("plus");
	plus_str_len = plus_str.length();std::cout << "500" << endl;
	minus_str = _("minus");
	minus_str_len = minus_str.length();std::cout << "502" << endl;
	and_str = _("and");
	if(and_str == "and") and_str = "";
	and_str_len = and_str.length();std::cout << "505" << endl;
	AND_str = "AND";
	AND_str_len = AND_str.length();std::cout << "507" << endl;
	or_str = _("or");
	if(or_str == "or") or_str = "";
	or_str_len = or_str.length();std::cout << "510" << endl;
	OR_str = "OR";
	OR_str_len = OR_str.length();std::cout << "512" << endl;
	XOR_str = "XOR";
	XOR_str_len = XOR_str.length();std::cout << "514" << endl;

	char *current_lc_numeric = setlocale(LC_NUMERIC, NULL);std::cout << "516" << endl;
	if(current_lc_numeric) saved_locale = strdup(current_lc_numeric);
	else saved_locale = NULL;std::cout << "518" << endl;
	struct lconv *lc = localeconv();std::cout << "519" << endl;
	if(!lc) {
		setlocale(LC_NUMERIC, "C");
		lc = localeconv();std::cout << "522" << endl;
	}
	place_currency_sign_before = lc->p_cs_precedes;
	place_currency_sign_before_negative = lc->n_cs_precedes;
#ifdef HAVE_STRUCT_LCONV_INT_P_CS_PRECEDES
 	place_currency_code_before = lc->int_p_cs_precedes;
#else
	place_currency_code_before = place_currency_sign_before;
#endif
#ifdef HAVE_STRUCT_LCONV_INT_N_CS_PRECEDES
	place_currency_code_before_negative = lc->int_n_cs_precedes;
#else
	place_currency_code_before_negative = place_currency_sign_before_negative;
#endif
	local_digit_group_separator = lc->thousands_sep;std::cout << "536" << endl;
	if((local_digit_group_separator.length() == 1 && local_digit_group_separator[0] < 0) || local_digit_group_separator == " ") local_digit_group_separator = " ";
	else if(local_digit_group_separator == " ") local_digit_group_separator = " ";
	local_digit_group_format = lc->grouping;
	remove_blank_ends(local_digit_group_format);
	default_dot_as_separator = (local_digit_group_separator == ".");
	if(strcmp(lc->decimal_point, ",") == 0) {
		DOT_STR = ",";
		DOT_S = ".,";
		COMMA_STR = ";";
		COMMA_S = ";";
	} else {
		DOT_STR = ".";
		DOT_S = ".";
		COMMA_STR = ",";
		COMMA_S = ",;";
	}
	setlocale(LC_NUMERIC, "C");

	NAME_NUMBER_PRE_S = "_#";
	NAME_NUMBER_PRE_STR = "_";

	//"to"-operator
	string str = _("to");
	local_to = (str != "to");

	decimal_null_prefix = new DecimalPrefix(0, "", "");
	binary_null_prefix = new BinaryPrefix(0, "", "");
	m_undefined.setUndefined();std::cout << "564" << endl;
	m_empty_vector.clearVector();std::cout << "565" << endl;
	m_empty_matrix.clearMatrix();std::cout << "566" << endl;
	m_zero.clear();std::cout << "567" << endl;
	m_one.set(1, 1, 0);std::cout << "568" << endl;
	m_minus_one.set(-1, 1, 0);std::cout << "569" << endl;
	nr_zero.clear();std::cout << "570" << endl;
	nr_one.set(1, 1, 0);std::cout << "571" << endl;
	nr_two.set(2, 1, 0);std::cout << "572" << endl;
	nr_three.set(3, 1, 0);std::cout << "573" << endl;
	nr_half.set(1, 2, 0);std::cout << "574" << endl;
	nr_minus_half.set(-1, 2, 0);std::cout << "575" << endl;
	nr_one_i.setImaginaryPart(1, 1, 0);std::cout << "576" << endl;
	nr_minus_i.setImaginaryPart(-1, 1, 0);std::cout << "577" << endl;
	m_one_i.set(nr_one_i);std::cout << "578" << endl;
	nr_minus_one.set(-1, 1, 0);std::cout << "579" << endl;
	nr_plus_inf.setPlusInfinity();std::cout << "580" << endl;
	nr_minus_inf.setMinusInfinity();std::cout << "581" << endl;
	no_evaluation.approximation = APPROXIMATION_EXACT;std::cout << "582" << endl;
	no_evaluation.structuring = STRUCTURING_NONE;std::cout << "583" << endl;
	no_evaluation.sync_units = false;std::cout << "584" << endl;

	save_printoptions.decimalpoint_sign = ".";
	save_printoptions.comma_sign = ",";
	save_printoptions.use_reference_names = true;std::cout << "588" << endl;
	save_printoptions.preserve_precision = true;std::cout << "589" << endl;
	save_printoptions.interval_display = INTERVAL_DISPLAY_INTERVAL;std::cout << "590" << endl;
	save_printoptions.limit_implicit_multiplication = true;std::cout << "591" << endl;
	save_printoptions.spacious = false;std::cout << "592" << endl;
	save_printoptions.number_fraction_format = FRACTION_FRACTIONAL;
	save_printoptions.short_multiplication = false;std::cout << "594" << endl;
	save_printoptions.show_ending_zeroes = false;std::cout << "595" << endl;
	save_printoptions.use_unit_prefixes = false;std::cout << "596" << endl;

	message_printoptions.interval_display = INTERVAL_DISPLAY_PLUSMINUS;std::cout << "598" << endl;
	message_printoptions.spell_out_logical_operators = true;std::cout << "599" << endl;
	message_printoptions.number_fraction_format = FRACTION_FRACTIONAL;

	default_user_evaluation_options.structuring = STRUCTURING_SIMPLIFY;std::cout << "602" << endl;

	default_assumptions = new Assumptions;std::cout << "604" << endl;
	default_assumptions->setType(ASSUMPTION_TYPE_REAL);std::cout << "605" << endl;
	default_assumptions->setSign(ASSUMPTION_SIGN_UNKNOWN);std::cout << "606" << endl;

	u_rad = NULL;std::cout << "608" << endl; u_gra = NULL;std::cout << "608" << endl; u_deg = NULL;std::cout << "608" << endl;

	b_save_called = false;std::cout << "610" << endl;

	ILLEGAL_IN_NAMES = "\a\b" + DOT_S + RESERVED OPERATORS SPACES PARENTHESISS VECTOR_WRAPS COMMAS;
	ILLEGAL_IN_NAMES_MINUS_SPACE_STR = "\a\b" + DOT_S + RESERVED OPERATORS PARENTHESISS VECTOR_WRAPS COMMAS;
	ILLEGAL_IN_UNITNAMES = ILLEGAL_IN_NAMES + NUMBERS;std::cout << "614" << endl;
	b_argument_errors = true;std::cout << "615" << endl;
	current_stage = MESSAGE_STAGE_UNSET;std::cout << "616" << endl;
	calculator = this;std::cout << "617" << endl;
	srand48(time(0));std::cout << "618" << endl;

	addBuiltinVariables();std::cout << "620" << endl;
	addBuiltinFunctions();std::cout << "621" << endl;
	addBuiltinUnits();std::cout << "622" << endl;

	disable_errors_ref = 0;std::cout << "624" << endl;
	b_busy = false;std::cout << "625" << endl;
	b_gnuplot_open = false;std::cout << "626" << endl;
	gnuplot_pipe = NULL;std::cout << "627" << endl;

	calculate_thread = new CalculateThread;std::cout << "629" << endl;
}
Calculator::Calculator(bool ignore_locale) {

	b_ignore_locale = ignore_locale;std::cout << "633" << endl;

#ifdef ENABLE_NLS
	if(!b_ignore_locale) {
		bindtextdomain(GETTEXT_PACKAGE, getPackageLocaleDir().c_str());std::cout << "637" << endl;
		bind_textdomain_codeset(GETTEXT_PACKAGE, "UTF-8");
	}
#endif

	if(b_ignore_locale) {
		char *current_lc_monetary = setlocale(LC_MONETARY, "");
		if(current_lc_monetary) saved_locale = strdup(current_lc_monetary);
		else saved_locale = NULL;std::cout << "645" << endl;
		setlocale(LC_ALL, "C");
		if(saved_locale) {
			setlocale(LC_MONETARY, saved_locale);std::cout << "648" << endl;
			free(saved_locale);std::cout << "649" << endl;
			saved_locale = NULL;std::cout << "650" << endl;
		}
	} else {
		setlocale(LC_ALL, "");
	}


	gmp_randinit_default(randstate);std::cout << "657" << endl;
	gmp_randseed_ui(randstate, (unsigned long int) time(NULL));std::cout << "658" << endl;

	priv = new Calculator_p;std::cout << "660" << endl;
	priv->custom_input_base_i = 0;std::cout << "661" << endl;
	priv->ids_i = 0;std::cout << "662" << endl;
	priv->local_currency = NULL;std::cout << "663" << endl;
	priv->use_binary_prefixes = 0;std::cout << "664" << endl;

#ifdef HAVE_ICU
	UErrorCode err = U_ZERO_ERROR;std::cout << "667" << endl;
	ucm = ucasemap_open(NULL, 0, &err);std::cout << "668" << endl;
#endif

	srand(time(NULL));std::cout << "671" << endl;

	exchange_rates_time[0] = 0;std::cout << "673" << endl;
	exchange_rates_time[1] = 0;std::cout << "674" << endl;
	exchange_rates_time[2] = 0;std::cout << "675" << endl;
	exchange_rates_check_time[0] = 0;std::cout << "676" << endl;
	exchange_rates_check_time[1] = 0;std::cout << "677" << endl;
	exchange_rates_check_time[2] = 0;std::cout << "678" << endl;
	b_exchange_rates_warning_enabled = true;std::cout << "679" << endl;
	b_exchange_rates_used = 0;std::cout << "680" << endl;

	i_aborted = 0;std::cout << "682" << endl;
	b_controlled = false;std::cout << "683" << endl;
	i_timeout = 0;std::cout << "684" << endl;

	setPrecision(DEFAULT_PRECISION);std::cout << "686" << endl;
	b_interval = true;std::cout << "687" << endl;
	i_stop_interval = 0;std::cout << "688" << endl;
	i_start_interval = 0;std::cout << "689" << endl;

	b_var_units = true;std::cout << "691" << endl;

	addStringAlternative(SIGN_DIVISION, DIVISION);
	addStringAlternative(SIGN_DIVISION_SLASH, DIVISION);
	addStringAlternative("⁄", DIVISION);
	addStringAlternative(SIGN_MULTIPLICATION, MULTIPLICATION);
	addStringAlternative(SIGN_MULTIDOT, MULTIPLICATION);
	addStringAlternative(SIGN_MIDDLEDOT, MULTIPLICATION);
	addStringAlternative(SIGN_MULTIBULLET, MULTIPLICATION);
	addStringAlternative(SIGN_SMALLCIRCLE, MULTIPLICATION);
	addStringAlternative(SIGN_MINUS, MINUS);
	addStringAlternative("–", MINUS);
	addStringAlternative(SIGN_PLUS, PLUS);
	addStringAlternative(SIGN_NOT_EQUAL, " " NOT EQUALS);
	addStringAlternative(SIGN_GREATER_OR_EQUAL, GREATER EQUALS);
	addStringAlternative(SIGN_LESS_OR_EQUAL, LESS EQUALS);
	addStringAlternative(";", COMMA);
	addStringAlternative("\t", SPACE);
	addStringAlternative("\n", SPACE);
	addStringAlternative(" ", SPACE);
	addStringAlternative(" ", SPACE);
	addStringAlternative(" ", SPACE);
	addStringAlternative("**", POWER);
	addStringAlternative("↊", "X");
	addStringAlternative("↋", "E");
	addStringAlternative("∧", BITWISE_AND);
	addStringAlternative("∨", BITWISE_OR);
	addStringAlternative("¬", BITWISE_NOT);
	addStringAlternative(SIGN_MICRO, "μ");

	per_str = _("per");
	per_str_len = per_str.length();std::cout << "722" << endl;
	times_str = _("times");
	times_str_len = times_str.length();std::cout << "724" << endl;
	plus_str = _("plus");
	plus_str_len = plus_str.length();std::cout << "726" << endl;
	minus_str = _("minus");
	minus_str_len = minus_str.length();std::cout << "728" << endl;
	and_str = _("and");
	if(and_str == "and") and_str = "";
	and_str_len = and_str.length();std::cout << "731" << endl;
	AND_str = "AND";
	AND_str_len = AND_str.length();std::cout << "733" << endl;
	or_str = _("or");
	if(or_str == "or") or_str = "";
	or_str_len = or_str.length();std::cout << "736" << endl;
	OR_str = "OR";
	OR_str_len = OR_str.length();std::cout << "738" << endl;
	XOR_str = "XOR";
	XOR_str_len = XOR_str.length();std::cout << "740" << endl;

	char *current_lc_numeric = setlocale(LC_NUMERIC, NULL);std::cout << "742" << endl;
	if(current_lc_numeric) saved_locale = strdup(current_lc_numeric);
	else saved_locale = NULL;std::cout << "744" << endl;
	struct lconv *lc = localeconv();std::cout << "745" << endl;
	if(!lc) {
		setlocale(LC_NUMERIC, "C");
		lc = localeconv();std::cout << "748" << endl;
	}
	place_currency_sign_before = lc->p_cs_precedes;
	place_currency_sign_before_negative = lc->n_cs_precedes;
#ifdef HAVE_STRUCT_LCONV_INT_P_CS_PRECEDES
 	place_currency_code_before = lc->int_p_cs_precedes;
#else
	place_currency_code_before = place_currency_sign_before;
#endif
#ifdef HAVE_STRUCT_LCONV_INT_N_CS_PRECEDES
	place_currency_code_before_negative = lc->int_n_cs_precedes;
#else
	place_currency_code_before_negative = place_currency_sign_before_negative;
#endif
	local_digit_group_separator = lc->thousands_sep;std::cout << "762" << endl;
	if((local_digit_group_separator.length() == 1 && local_digit_group_separator[0] < 0) || local_digit_group_separator == " ") local_digit_group_separator = " ";
	else if(local_digit_group_separator == " ") local_digit_group_separator = " ";
	local_digit_group_format = lc->grouping;
	remove_blank_ends(local_digit_group_format);
	default_dot_as_separator = (local_digit_group_separator == ".");
	if(strcmp(lc->decimal_point, ",") == 0) {
		DOT_STR = ",";
		DOT_S = ".,";
		COMMA_STR = ";";
		COMMA_S = ";";
	} else {
		DOT_STR = ".";
		DOT_S = ".";
		COMMA_STR = ",";
		COMMA_S = ",;";
	}
	setlocale(LC_NUMERIC, "C");

	NAME_NUMBER_PRE_S = "_#";
	NAME_NUMBER_PRE_STR = "_";

	//"to"-operator
	string str = _("to");
	local_to = (str != "to");

	decimal_null_prefix = new DecimalPrefix(0, "", "");
	binary_null_prefix = new BinaryPrefix(0, "", "");
	m_undefined.setUndefined();std::cout << "790" << endl;
	m_empty_vector.clearVector();std::cout << "791" << endl;
	m_empty_matrix.clearMatrix();std::cout << "792" << endl;
	m_zero.clear();std::cout << "793" << endl;
	m_one.set(1, 1, 0);std::cout << "794" << endl;
	m_minus_one.set(-1, 1, 0);std::cout << "795" << endl;
	nr_zero.clear();std::cout << "796" << endl;
	nr_one.set(1, 1, 0);std::cout << "797" << endl;
	nr_two.set(2, 1, 0);std::cout << "798" << endl;
	nr_three.set(3, 1, 0);std::cout << "799" << endl;
	nr_half.set(1, 2, 0);std::cout << "800" << endl;
	nr_minus_half.set(-1, 2, 0);std::cout << "801" << endl;
	nr_one_i.setImaginaryPart(1, 1, 0);std::cout << "802" << endl;
	nr_minus_i.setImaginaryPart(-1, 1, 0);std::cout << "803" << endl;
	m_one_i.set(nr_one_i);std::cout << "804" << endl;
	nr_minus_one.set(-1, 1, 0);std::cout << "805" << endl;
	nr_plus_inf.setPlusInfinity();std::cout << "806" << endl;
	nr_minus_inf.setMinusInfinity();std::cout << "807" << endl;
	no_evaluation.approximation = APPROXIMATION_EXACT;std::cout << "808" << endl;
	no_evaluation.structuring = STRUCTURING_NONE;std::cout << "809" << endl;
	no_evaluation.sync_units = false;std::cout << "810" << endl;

	save_printoptions.decimalpoint_sign = ".";
	save_printoptions.comma_sign = ",";
	save_printoptions.use_reference_names = true;std::cout << "814" << endl;
	save_printoptions.preserve_precision = true;std::cout << "815" << endl;
	save_printoptions.interval_display = INTERVAL_DISPLAY_INTERVAL;std::cout << "816" << endl;
	save_printoptions.limit_implicit_multiplication = true;std::cout << "817" << endl;
	save_printoptions.spacious = false;std::cout << "818" << endl;
	save_printoptions.number_fraction_format = FRACTION_FRACTIONAL;
	save_printoptions.short_multiplication = false;std::cout << "820" << endl;
	save_printoptions.show_ending_zeroes = false;std::cout << "821" << endl;
	save_printoptions.use_unit_prefixes = false;std::cout << "822" << endl;

	message_printoptions.interval_display = INTERVAL_DISPLAY_PLUSMINUS;std::cout << "824" << endl;
	message_printoptions.spell_out_logical_operators = true;std::cout << "825" << endl;
	message_printoptions.number_fraction_format = FRACTION_FRACTIONAL;

	default_user_evaluation_options.structuring = STRUCTURING_SIMPLIFY;std::cout << "828" << endl;

	default_assumptions = new Assumptions;std::cout << "830" << endl;
	default_assumptions->setType(ASSUMPTION_TYPE_REAL);std::cout << "831" << endl;
	default_assumptions->setSign(ASSUMPTION_SIGN_UNKNOWN);std::cout << "832" << endl;

	u_rad = NULL;std::cout << "834" << endl; u_gra = NULL;std::cout << "834" << endl; u_deg = NULL;std::cout << "834" << endl;

	b_save_called = false;std::cout << "836" << endl;

	ILLEGAL_IN_NAMES = "\a\b" + DOT_S + RESERVED OPERATORS SPACES PARENTHESISS VECTOR_WRAPS COMMAS;
	ILLEGAL_IN_NAMES_MINUS_SPACE_STR = "\a\b" + DOT_S + RESERVED OPERATORS PARENTHESISS VECTOR_WRAPS COMMAS;
	ILLEGAL_IN_UNITNAMES = ILLEGAL_IN_NAMES + NUMBERS;std::cout << "840" << endl;
	b_argument_errors = true;std::cout << "841" << endl;
	current_stage = MESSAGE_STAGE_UNSET;std::cout << "842" << endl;
	calculator = this;std::cout << "843" << endl;
	srand48(time(0));std::cout << "844" << endl;

	addBuiltinVariables();std::cout << "846" << endl;
	addBuiltinFunctions();std::cout << "847" << endl;
	addBuiltinUnits();std::cout << "848" << endl;

	disable_errors_ref = 0;std::cout << "850" << endl;
	b_busy = false;std::cout << "851" << endl;
	b_gnuplot_open = false;std::cout << "852" << endl;
	gnuplot_pipe = NULL;std::cout << "853" << endl;

	calculate_thread = new CalculateThread;std::cout << "855" << endl;
}
Calculator::~Calculator() {
	closeGnuplot();std::cout << "858" << endl;
	abort();std::cout << "859" << endl;
	terminateThreads();std::cout << "860" << endl;
	delete priv;std::cout << "861" << endl;
	delete calculate_thread;std::cout << "862" << endl;
	gmp_randclear(randstate);std::cout << "863" << endl;
#ifdef HAVE_ICU
	if(ucm) ucasemap_close(ucm);
#endif
}

Unit *Calculator::getGraUnit() {
	if(!u_gra) u_gra = getUnit("gra");
	if(!u_gra) {
		error(true, _("Gradians unit is missing. Creating one for this session."), NULL);
		u_gra = addUnit(new AliasUnit(_("Angle/Plane Angle"), "gra", "gradians", "gradian", "Gradian", getRadUnit(), "pi/200", 1, "", false, true, true));
	}
	return u_gra;
}
Unit *Calculator::getRadUnit() {
	if(!u_rad) u_rad = getUnit("rad");
	if(!u_rad) {
		error(true, _("Radians unit is missing. Creating one for this session."), NULL);
		u_rad = addUnit(new Unit(_("Angle/Plane Angle"), "rad", "radians", "radian", "Radian", false, true, true));
	}
	return u_rad;
}
Unit *Calculator::getDegUnit() {
	if(!u_deg) u_deg = getUnit("deg");
	if(!u_deg) {
		error(true, _("Degrees unit is missing. Creating one for this session."), NULL);
		u_deg = addUnit(new AliasUnit(_("Angle/Plane Angle"), "deg", "degrees", "degree", "Degree", getRadUnit(), "pi/180", 1, "", false, true, true));
	}
	return u_deg;
}

bool Calculator::utf8_pos_is_valid_in_name(char *pos) {
	if(is_in(ILLEGAL_IN_NAMES, pos[0])) {
		return false;
	}
	if((unsigned char) pos[0] >= 0xC0) {
		string str;std::cout << "899" << endl;
		str += pos[0];std::cout << "900" << endl;
		while((unsigned char) pos[1] >= 0x80 && (unsigned char) pos[1] < 0xC0) {
			str += pos[1];std::cout << "902" << endl;
			pos++;std::cout << "903" << endl;
		}
		return str != SIGN_DIVISION && str != SIGN_DIVISION_SLASH && str != SIGN_MULTIPLICATION && str != SIGN_MULTIDOT && str != SIGN_SMALLCIRCLE && str != SIGN_MULTIBULLET && str != SIGN_MINUS && str != SIGN_PLUS && str != SIGN_NOT_EQUAL && str != SIGN_GREATER_OR_EQUAL && str != SIGN_LESS_OR_EQUAL;
	}
	return true;
}

bool Calculator::showArgumentErrors() const {
	return b_argument_errors;
}
void Calculator::beginTemporaryStopMessages() {
	disable_errors_ref++;std::cout << "914" << endl;
	stopped_errors_count.push_back(0);std::cout << "915" << endl;
	stopped_warnings_count.push_back(0);std::cout << "916" << endl;
	stopped_messages_count.push_back(0);std::cout << "917" << endl;
	vector<CalculatorMessage> vcm;std::cout << "918" << endl;
	stopped_messages.push_back(vcm);std::cout << "919" << endl;
}
int Calculator::endTemporaryStopMessages(int *message_count, int *warning_count, int release_messages_if_no_equal_or_greater_than_message_type) {
	if(disable_errors_ref <= 0) return -1;
	disable_errors_ref--;std::cout << "923" << endl;
	int ret = stopped_errors_count[disable_errors_ref];std::cout << "924" << endl;
	bool release_messages = false;std::cout << "925" << endl;
	if(release_messages_if_no_equal_or_greater_than_message_type >= MESSAGE_INFORMATION) {
		if(ret > 0) release_messages = true;
		if(release_messages_if_no_equal_or_greater_than_message_type == MESSAGE_WARNING && stopped_warnings_count[disable_errors_ref] > 0) release_messages = true;
		else if(release_messages_if_no_equal_or_greater_than_message_type == MESSAGE_INFORMATION && stopped_messages_count[disable_errors_ref] > 0) release_messages = true;
	}
	if(message_count) *message_count = stopped_messages_count[disable_errors_ref];
	if(warning_count) *warning_count = stopped_warnings_count[disable_errors_ref];
	stopped_errors_count.pop_back();std::cout << "933" << endl;
	stopped_warnings_count.pop_back();std::cout << "934" << endl;
	stopped_messages_count.pop_back();std::cout << "935" << endl;
	if(release_messages) addMessages(&stopped_messages[disable_errors_ref]);
	stopped_messages.pop_back();std::cout << "937" << endl;
	return ret;
}
void Calculator::endTemporaryStopMessages(bool release_messages, vector<CalculatorMessage> *blocked_messages) {
	if(disable_errors_ref <= 0) return;
	disable_errors_ref--;std::cout << "942" << endl;
	stopped_errors_count.pop_back();std::cout << "943" << endl;
	stopped_warnings_count.pop_back();std::cout << "944" << endl;
	stopped_messages_count.pop_back();std::cout << "945" << endl;
	if(blocked_messages) *blocked_messages = stopped_messages[disable_errors_ref];
	if(release_messages) addMessages(&stopped_messages[disable_errors_ref]);
	stopped_messages.pop_back();std::cout << "948" << endl;
}
void Calculator::addMessages(vector<CalculatorMessage> *message_vector) {
	for(size_t i3 = 0; i3 < message_vector->size(); i3++) {
		string error_str = (*message_vector)[i3].message();std::cout << "952" << endl;
		bool dup_error = false;std::cout << "953" << endl;
		for(size_t i = 0; i < messages.size(); i++) {
			if(error_str == messages[i].message()) {
				dup_error = true;std::cout << "956" << endl;
				break;
			}
		}
		if(!dup_error) {
			if(disable_errors_ref > 0) {
				for(size_t i2 = 0; !dup_error && i2 < (size_t) disable_errors_ref; i2++) {
					for(size_t i = 0; i < stopped_messages[i2].size(); i++) {
						if(error_str == stopped_messages[i2][i].message()) {
							dup_error = true;std::cout << "965" << endl;
							break;
						}
					}
				}
				if(!dup_error) stopped_messages[disable_errors_ref - 1].push_back((*message_vector)[i3]);
			} else {
				messages.push_back((*message_vector)[i3]);std::cout << "972" << endl;
			}
		}
	}
}
const PrintOptions &Calculator::messagePrintOptions() const {return message_printoptions;}
void Calculator::setMessagePrintOptions(const PrintOptions &po) {message_printoptions = po;}

Variable *Calculator::getVariable(size_t index) const {
	if(index < variables.size()) {
		return variables[index];
	}
	return NULL;
}
bool Calculator::hasVariable(Variable *v) {
	for(size_t i = 0; i < variables.size(); i++) {
		if(variables[i] == v) return true;
	}
	return false;
}
bool Calculator::hasUnit(Unit *u) {
	for(size_t i = 0; i < units.size(); i++) {
		if(units[i] == u) return true;
	}
	return false;
}
bool Calculator::hasFunction(MathFunction *f) {
	for(size_t i = 0; i < functions.size(); i++) {
		if(functions[i] == f) return true;
	}
	return false;
}
bool Calculator::stillHasVariable(Variable *v) {
	for(vector<Variable*>::iterator it = deleted_variables.begin(); it != deleted_variables.end(); ++it) {
		if(*it == v) return false;
	}
	return true;
}
bool Calculator::stillHasUnit(Unit *u) {
	for(vector<Unit*>::iterator it = deleted_units.begin(); it != deleted_units.end(); ++it) {
		if(*it == u) return false;
	}
	return true;
}
bool Calculator::stillHasFunction(MathFunction *f) {
	for(vector<MathFunction*>::iterator it = deleted_functions.begin(); it != deleted_functions.end(); ++it) {
		if(*it == f) return false;
	}
	return true;
}
void Calculator::saveFunctionCalled() {
	b_save_called = true;std::cout << "1023" << endl;
}
bool Calculator::checkSaveFunctionCalled() {
	if(b_save_called) {
		b_save_called = false;std::cout << "1027" << endl;
		return true;
	}
	return false;
}
ExpressionItem *Calculator::getActiveExpressionItem(ExpressionItem *item) {
	if(!item) return NULL;
	for(size_t i = 1; i <= item->countNames(); i++) {
		ExpressionItem *item2 = getActiveExpressionItem(item->getName(i).name, item);std::cout << "1035" << endl;
		if(item2) {
			return item2;
		}
	}
	return NULL;
}
ExpressionItem *Calculator::getActiveExpressionItem(string name, ExpressionItem *item) {
	if(name.empty()) return NULL;
	for(size_t index = 0; index < variables.size(); index++) {
		if(variables[index] != item && variables[index]->isActive() && variables[index]->hasName(name)) {
			return variables[index];
		}
	}
	for(size_t index = 0; index < functions.size(); index++) {
		if(functions[index] != item && functions[index]->isActive() && functions[index]->hasName(name)) {
			return functions[index];
		}
	}
	for(size_t i = 0; i < units.size(); i++) {
		if(units[i] != item && units[i]->isActive() && units[i]->hasName(name)) {
			return units[i];
		}
	}
	return NULL;
}
ExpressionItem *Calculator::getInactiveExpressionItem(string name, ExpressionItem *item) {
	if(name.empty()) return NULL;
	for(size_t index = 0; index < variables.size(); index++) {
		if(variables[index] != item && !variables[index]->isActive() && variables[index]->hasName(name)) {
			return variables[index];
		}
	}
	for(size_t index = 0; index < functions.size(); index++) {
		if(functions[index] != item && !functions[index]->isActive() && functions[index]->hasName(name)) {
			return functions[index];
		}
	}
	for(size_t i = 0; i < units.size(); i++) {
		if(units[i] != item && !units[i]->isActive() && units[i]->hasName(name)) {
			return units[i];
		}
	}
	return NULL;
}
ExpressionItem *Calculator::getExpressionItem(string name, ExpressionItem *item) {
	if(name.empty()) return NULL;
	Variable *v = getVariable(name);std::cout << "1082" << endl;
	if(v && v != item) return v;
	MathFunction *f = getFunction(name);std::cout << "1084" << endl;
	if(f && f != item) return f;
	Unit *u = getUnit(name);std::cout << "1086" << endl;
	if(u && u != item) return u;
	u = getCompositeUnit(name);std::cout << "1088" << endl;
	if(u && u != item) return u;
	return NULL;
}
Unit *Calculator::getUnit(size_t index) const {
	if(index < units.size()) {
		return units[index];
	}
	return NULL;
}
MathFunction *Calculator::getFunction(size_t index) const {
	if(index < functions.size()) {
		return functions[index];
	}
	return NULL;
}

void Calculator::setDefaultAssumptions(Assumptions *ass) {
	if(default_assumptions) delete default_assumptions;
	default_assumptions = ass;std::cout << "1107" << endl;
}
Assumptions *Calculator::defaultAssumptions() {
	return default_assumptions;
}

Prefix *Calculator::getPrefix(size_t index) const {
	if(index < prefixes.size()) {
		return prefixes[index];
	}
	return NULL;
}
Prefix *Calculator::getPrefix(string name_) const {
	for(size_t i = 0; i < prefixes.size(); i++) {
		if(prefixes[i]->shortName(false) == name_ || prefixes[i]->longName(false) == name_ || prefixes[i]->unicodeName(false) == name_) {
			return prefixes[i];
		}
	}
	return NULL;
}
DecimalPrefix *Calculator::getExactDecimalPrefix(int exp10, int exp) const {
	for(size_t i = 0; i < decimal_prefixes.size(); i++) {
		if(decimal_prefixes[i]->exponent(exp) == exp10) {
			return decimal_prefixes[i];
		} else if(decimal_prefixes[i]->exponent(exp) > exp10) {
			break;
		}
	}
	return NULL;
}
BinaryPrefix *Calculator::getExactBinaryPrefix(int exp2, int exp) const {
	for(size_t i = 0; i < binary_prefixes.size(); i++) {
		if(binary_prefixes[i]->exponent(exp) == exp2) {
			return binary_prefixes[i];
		} else if(binary_prefixes[i]->exponent(exp) > exp2) {
			break;
		}
	}
	return NULL;
}
Prefix *Calculator::getExactPrefix(const Number &o, int exp) const {
	ComparisonResult c;std::cout << "1148" << endl;
	for(size_t i = 0; i < prefixes.size(); i++) {
		c = o.compare(prefixes[i]->value(exp));std::cout << "1150" << endl;
		if(c == COMPARISON_RESULT_EQUAL) {
			return prefixes[i];
		} else if(c == COMPARISON_RESULT_GREATER) {
			break;
		}
	}
	return NULL;
}
DecimalPrefix *Calculator::getNearestDecimalPrefix(int exp10, int exp) const {
	if(decimal_prefixes.size() <= 0) return NULL;
	int i = 0;std::cout << "1161" << endl;
	if(exp < 0) {
		i = decimal_prefixes.size() - 1;std::cout << "1163" << endl;
	}
	while((exp < 0 && i >= 0) || (exp >= 0 && i < (int) decimal_prefixes.size())) {
		if(decimal_prefixes[i]->exponent(exp) == exp10) {
			return decimal_prefixes[i];
		} else if(decimal_prefixes[i]->exponent(exp) > exp10) {
			if(i == 0) {
				return decimal_prefixes[i];
			} else if(exp10 - decimal_prefixes[i - 1]->exponent(exp) < decimal_prefixes[i]->exponent(exp) - exp10) {
				return decimal_prefixes[i - 1];
			} else {
				return decimal_prefixes[i];
			}
		}
		if(exp < 0) {
			i--;std::cout << "1178" << endl;
		} else {
			i++;std::cout << "1180" << endl;
		}
	}
	return decimal_prefixes[decimal_prefixes.size() - 1];
}
DecimalPrefix *Calculator::getOptimalDecimalPrefix(int exp10, int exp, bool all_prefixes) const {
	if(decimal_prefixes.size() <= 0 || exp10 == 0) return NULL;
	int i = 0;std::cout << "1187" << endl;
	if(exp < 0) {
		i = decimal_prefixes.size() - 1;std::cout << "1189" << endl;
	}
	DecimalPrefix *p = NULL, *p_prev = NULL;std::cout << "1191" << endl;
	int exp10_1, exp10_2;std::cout << "1192" << endl;
	while((exp < 0 && i >= 0) || (exp >= 0 && i < (int) decimal_prefixes.size())) {
		if(all_prefixes || decimal_prefixes[i]->exponent() % 3 == 0) {
			p = decimal_prefixes[i];std::cout << "1195" << endl;
			if(p_prev && (p_prev->exponent() >= 0) != (p->exponent() >= 0) && p_prev->exponent() != 0) {
				if(exp < 0) {
					i++;std::cout << "1198" << endl;
				} else {
					i--;std::cout << "1200" << endl;
				}
				p = decimal_null_prefix;std::cout << "1202" << endl;
			}
			if(p->exponent(exp) == exp10) {
				if(p == decimal_null_prefix) return NULL;
				return p;
			} else if(p->exponent(exp) > exp10) {
				if(i == 0) {
					if(p == decimal_null_prefix) return NULL;
					return p;
				}
				exp10_1 = exp10;std::cout << "1212" << endl;
				if(p_prev) {
					exp10_1 -= p_prev->exponent(exp);std::cout << "1214" << endl;
				}
				exp10_2 = p->exponent(exp);std::cout << "1216" << endl;
				exp10_2 -= exp10;std::cout << "1217" << endl;
				exp10_2 *= 2;std::cout << "1218" << endl;
				exp10_2 += 2;std::cout << "1219" << endl;
				if(exp10_1 < exp10_2) {
					if(p_prev == decimal_null_prefix) return NULL;
					return p_prev;
				} else {
					return p;
				}
			}
			p_prev = p;std::cout << "1227" << endl;
		}
		if(exp < 0) {
			i--;std::cout << "1230" << endl;
		} else {
			i++;std::cout << "1232" << endl;
		}
	}
	return p_prev;
}
DecimalPrefix *Calculator::getOptimalDecimalPrefix(const Number &exp10, const Number &exp, bool all_prefixes) const {
	if(decimal_prefixes.size() <= 0 || exp10.isZero()) return NULL;
	int i = 0;std::cout << "1239" << endl;
	ComparisonResult c;std::cout << "1240" << endl;
	if(exp.isNegative()) {
		i = decimal_prefixes.size() - 1;std::cout << "1242" << endl;
	}
	DecimalPrefix *p = NULL, *p_prev = NULL;std::cout << "1244" << endl;
	Number exp10_1, exp10_2;std::cout << "1245" << endl;
	while((exp.isNegative() && i >= 0) || (!exp.isNegative() && i < (int) decimal_prefixes.size())) {
		if(all_prefixes || decimal_prefixes[i]->exponent() % 3 == 0) {
			p = decimal_prefixes[i];std::cout << "1248" << endl;
			if(p_prev && (p_prev->exponent() >= 0) != (p->exponent() >= 0) && p_prev->exponent() != 0) {
				if(exp.isNegative()) {
					i++;std::cout << "1251" << endl;
				} else {
					i--;std::cout << "1253" << endl;
				}
				p = decimal_null_prefix;std::cout << "1255" << endl;
			}
			c = exp10.compare(p->exponent(exp));std::cout << "1257" << endl;
			if(c == COMPARISON_RESULT_EQUAL) {
				if(p == decimal_null_prefix) return NULL;
				return p;
			} else if(c == COMPARISON_RESULT_GREATER) {
				if(i == 0) {
					if(p == decimal_null_prefix) return NULL;
					return p;
				}
				exp10_1 = exp10;std::cout << "1266" << endl;
				if(p_prev) {
					exp10_1 -= p_prev->exponent(exp);std::cout << "1268" << endl;
				}
				exp10_2 = p->exponent(exp);std::cout << "1270" << endl;
				exp10_2 -= exp10;std::cout << "1271" << endl;
				exp10_2 *= 2;std::cout << "1272" << endl;
				exp10_2 += 2;std::cout << "1273" << endl;
				if(exp10_1.isLessThan(exp10_2)) {
					if(p_prev == decimal_null_prefix) return NULL;
					return p_prev;
				} else {
					return p;
				}
			}
			p_prev = p;std::cout << "1281" << endl;
		}
		if(exp.isNegative()) {
			i--;std::cout << "1284" << endl;
		} else {
			i++;std::cout << "1286" << endl;
		}
	}
	return p_prev;
}
int Calculator::usesBinaryPrefixes() const {
	return priv->use_binary_prefixes;
}
void Calculator::useBinaryPrefixes(int use_binary_prefixes) {
	priv->use_binary_prefixes = use_binary_prefixes;std::cout << "1295" << endl;
}
BinaryPrefix *Calculator::getNearestBinaryPrefix(int exp2, int exp) const {
	if(binary_prefixes.size() <= 0) return NULL;
	int i = 0;std::cout << "1299" << endl;
	if(exp < 0) {
		i = binary_prefixes.size() - 1;std::cout << "1301" << endl;
	}
	while((exp < 0 && i >= 0) || (exp >= 0 && i < (int) binary_prefixes.size())) {
		if(binary_prefixes[i]->exponent(exp) == exp2) {
			return binary_prefixes[i];
		} else if(binary_prefixes[i]->exponent(exp) > exp2) {
			if(i == 0) {
				return binary_prefixes[i];
			} else if(exp2 - binary_prefixes[i - 1]->exponent(exp) < binary_prefixes[i]->exponent(exp) - exp2) {
				return binary_prefixes[i - 1];
			} else {
				return binary_prefixes[i];
			}
		}
		if(exp < 0) {
			i--;std::cout << "1316" << endl;
		} else {
			i++;std::cout << "1318" << endl;
		}
	}
	return binary_prefixes[binary_prefixes.size() - 1];
}
BinaryPrefix *Calculator::getOptimalBinaryPrefix(int exp2, int exp) const {
	if(binary_prefixes.size() <= 0 || exp2 == 0) return NULL;
	int i = -1;std::cout << "1325" << endl;
	if(exp < 0) {
		i = binary_prefixes.size() - 1;std::cout << "1327" << endl;
	}
	BinaryPrefix *p = NULL, *p_prev = NULL;std::cout << "1329" << endl;
	int exp2_1, exp2_2;std::cout << "1330" << endl;
	while((exp < 0 && i >= -1) || (exp >= 0 && i < (int) binary_prefixes.size())) {
		if(i >= 0) p = binary_prefixes[i];
		else p = binary_null_prefix;std::cout << "1333" << endl;
		if(p_prev && (p_prev->exponent() >= 0) != (p->exponent() >= 0) && p_prev->exponent() != 0) {
			if(exp < 0) {
				i++;std::cout << "1336" << endl;
			} else {
				i--;std::cout << "1338" << endl;
			}
			p = binary_null_prefix;std::cout << "1340" << endl;
		}
		if(p->exponent(exp) == exp2) {
			if(p == binary_null_prefix) return NULL;
			return p;
		} else if(p->exponent(exp) > exp2) {
			exp2_1 = exp2;std::cout << "1346" << endl;
			if(p_prev) {
				exp2_1 -= p_prev->exponent(exp);std::cout << "1348" << endl;
			}
			exp2_2 = p->exponent(exp);std::cout << "1350" << endl;
			exp2_2 -= exp2;std::cout << "1351" << endl;
			exp2_2 += 9;std::cout << "1352" << endl;
			if(exp2_1 < exp2_2) {
				if(p_prev == binary_null_prefix) return NULL;
				return p_prev;
			} else {
				return p;
			}
		}
		p_prev = p;std::cout << "1360" << endl;
		if(exp < 0) {
			i--;std::cout << "1362" << endl;
		} else {
			i++;std::cout << "1364" << endl;
		}
	}
	return p_prev;
}
BinaryPrefix *Calculator::getOptimalBinaryPrefix(const Number &exp2, const Number &exp) const {
	if(binary_prefixes.size() <= 0 || exp2.isZero()) return NULL;
	int i = -1;std::cout << "1371" << endl;
	ComparisonResult c;std::cout << "1372" << endl;
	if(exp.isNegative()) {
		i = binary_prefixes.size() - 1;std::cout << "1374" << endl;
	}
	BinaryPrefix *p = NULL, *p_prev = NULL;std::cout << "1376" << endl;
	Number exp2_1, exp2_2;std::cout << "1377" << endl;
	while((exp.isNegative() && i >= -1) || (!exp.isNegative() && i < (int) binary_prefixes.size())) {
		if(i >= 0) p = binary_prefixes[i];
		else p = binary_null_prefix;std::cout << "1380" << endl;
		c = exp2.compare(p->exponent(exp));std::cout << "1381" << endl;
		if(c == COMPARISON_RESULT_EQUAL) {
			if(p == binary_null_prefix) return NULL;
			return p;
		} else if(c == COMPARISON_RESULT_GREATER) {
			exp2_1 = exp2;std::cout << "1386" << endl;
			if(p_prev) {
				exp2_1 -= p_prev->exponent(exp);std::cout << "1388" << endl;
			}
			exp2_2 = p->exponent(exp);std::cout << "1390" << endl;
			exp2_2 -= exp2;std::cout << "1391" << endl;
			exp2_2 += 9;std::cout << "1392" << endl;
			if(exp2_1.isLessThan(exp2_2)) {
				if(p_prev == binary_null_prefix) return NULL;
				return p_prev;
			} else {
				return p;
			}
		}
		p_prev = p;std::cout << "1400" << endl;
		if(exp.isNegative()) {
			i--;std::cout << "1402" << endl;
		} else {
			i++;std::cout << "1404" << endl;
		}
	}
	return p_prev;
}
Prefix *Calculator::addPrefix(Prefix *p) {
	if(p->type() == PREFIX_DECIMAL) {
		decimal_prefixes.push_back((DecimalPrefix*) p);std::cout << "1411" << endl;
	} else if(p->type() == PREFIX_BINARY) {
		binary_prefixes.push_back((BinaryPrefix*) p);std::cout << "1413" << endl;
	}
	prefixes.push_back(p);std::cout << "1415" << endl;
	prefixNameChanged(p, true);std::cout << "1416" << endl;
	return p;
}
void Calculator::prefixNameChanged(Prefix *p, bool new_item) {
	size_t l2;std::cout << "1420" << endl;
	if(!new_item) delPrefixUFV(p);
	if(!p->longName(false).empty()) {
		l2 = p->longName(false).length();std::cout << "1423" << endl;
		if(l2 > UFV_LENGTHS) {
			size_t i = 0, l;std::cout << "1425" << endl;
			for(vector<void*>::iterator it = ufvl.begin(); ; ++it) {
				l = 0;std::cout << "1427" << endl;
				if(it != ufvl.end()) {
					if(ufvl_t[i] == 'v')						l = ((Variable*) (*it))->getName(ufvl_i[i]).name.length();
					else if(ufvl_t[i] == 'f')						l = ((MathFunction*) (*it))->getName(ufvl_i[i]).name.length();
					else if(ufvl_t[i] == 'u')						l = ((Unit*) (*it))->getName(ufvl_i[i]).name.length();
					else if(ufvl_t[i] == 'p')						l = ((Prefix*) (*it))->shortName(false).length();
					else if(ufvl_t[i] == 'P')						l = ((Prefix*) (*it))->longName(false).length();
					else if(ufvl_t[i] == 'q')						l = ((Prefix*) (*it))->unicodeName(false).length();
				}
				if(it == ufvl.end()) {
					ufvl.push_back((void*) p);std::cout << "1437" << endl;
					ufvl_t.push_back('P');std::cout << "1438" << endl;
					ufvl_i.push_back(1);std::cout << "1439" << endl;
					break;
				} else if(l <= l2) {
					ufvl.insert(it, (void*) p);std::cout << "1442" << endl;
					ufvl_t.insert(ufvl_t.begin() + i, 'P');std::cout << "1443" << endl;
					ufvl_i.insert(ufvl_i.begin() + i, 1);std::cout << "1444" << endl;
					break;
				}
				i++;std::cout << "1447" << endl;
			}
		} else if(l2 > 0) {
			l2--;std::cout << "1450" << endl;
			ufv[0][l2].push_back((void*) p);std::cout << "1451" << endl;
			ufv_i[0][l2].push_back(1);std::cout << "1452" << endl;
		}
	}
	if(!p->shortName(false).empty()) {
		l2 = p->shortName(false).length();std::cout << "1456" << endl;
		if(l2 > UFV_LENGTHS) {
			size_t i = 0, l;std::cout << "1458" << endl;
			for(vector<void*>::iterator it = ufvl.begin(); ; ++it) {
				l = 0;std::cout << "1460" << endl;
				if(it != ufvl.end()) {
					if(ufvl_t[i] == 'v')						l = ((Variable*) (*it))->getName(ufvl_i[i]).name.length();
					else if(ufvl_t[i] == 'f')						l = ((MathFunction*) (*it))->getName(ufvl_i[i]).name.length();
					else if(ufvl_t[i] == 'u')						l = ((Unit*) (*it))->getName(ufvl_i[i]).name.length();
					else if(ufvl_t[i] == 'p')						l = ((Prefix*) (*it))->shortName(false).length();
					else if(ufvl_t[i] == 'P')						l = ((Prefix*) (*it))->longName(false).length();
					else if(ufvl_t[i] == 'q')						l = ((Prefix*) (*it))->unicodeName(false).length();
				}
				if(it == ufvl.end()) {
					ufvl.push_back((void*) p);std::cout << "1470" << endl;
					ufvl_t.push_back('p');std::cout << "1471" << endl;
					ufvl_i.push_back(1);std::cout << "1472" << endl;
					break;
				} else if(l <= l2) {
					ufvl.insert(it, (void*) p);std::cout << "1475" << endl;
					ufvl_t.insert(ufvl_t.begin() + i, 'p');std::cout << "1476" << endl;
					ufvl_i.insert(ufvl_i.begin() + i, 1);std::cout << "1477" << endl;
					break;
				}
				i++;std::cout << "1480" << endl;
			}
		} else if(l2 > 0) {
			l2--;std::cout << "1483" << endl;
			ufv[0][l2].push_back((void*) p);std::cout << "1484" << endl;
			ufv_i[0][l2].push_back(2);std::cout << "1485" << endl;
		}
	}
	if(!p->unicodeName(false).empty()) {
		l2 = p->unicodeName(false).length();std::cout << "1489" << endl;
		if(l2 > UFV_LENGTHS) {
			size_t i = 0, l;std::cout << "1491" << endl;
			for(vector<void*>::iterator it = ufvl.begin(); ; ++it) {
				l = 0;std::cout << "1493" << endl;
				if(it != ufvl.end()) {
					if(ufvl_t[i] == 'v')						l = ((Variable*) (*it))->getName(ufvl_i[i]).name.length();
					else if(ufvl_t[i] == 'f')						l = ((MathFunction*) (*it))->getName(ufvl_i[i]).name.length();
					else if(ufvl_t[i] == 'u')						l = ((Unit*) (*it))->getName(ufvl_i[i]).name.length();
					else if(ufvl_t[i] == 'p')						l = ((Prefix*) (*it))->shortName(false).length();
					else if(ufvl_t[i] == 'P')						l = ((Prefix*) (*it))->longName(false).length();
					else if(ufvl_t[i] == 'q')						l = ((Prefix*) (*it))->unicodeName(false).length();
				}
				if(it == ufvl.end()) {
					ufvl.push_back((void*) p);std::cout << "1503" << endl;
					ufvl_t.push_back('q');std::cout << "1504" << endl;
					ufvl_i.push_back(1);std::cout << "1505" << endl;
					break;
				} else if(l <= l2) {
					ufvl.insert(it, (void*) p);std::cout << "1508" << endl;
					ufvl_t.insert(ufvl_t.begin() + i, 'q');std::cout << "1509" << endl;
					ufvl_i.insert(ufvl_i.begin() + i, 1);std::cout << "1510" << endl;
					break;
				}
				i++;std::cout << "1513" << endl;
			}
		} else if(l2 > 0) {
			l2--;std::cout << "1516" << endl;
			ufv[0][l2].push_back((void*) p);std::cout << "1517" << endl;
			ufv_i[0][l2].push_back(3);std::cout << "1518" << endl;
		}
	}
}
#define PRECISION_TO_BITS(p) (((p) * 3.322) + 100)
void Calculator::setPrecision(int precision) {
	if(precision <= 0) precision = DEFAULT_PRECISION;
	i_precision = precision;std::cout << "1524" << endl;
	mpfr_set_default_prec(PRECISION_TO_BITS(i_precision));std::cout << "1525" << endl;
}
int Calculator::getPrecision() const {
	return i_precision;
}
void Calculator::useIntervalArithmetic(bool use_interval_arithmetic) {b_interval = use_interval_arithmetic;}
bool Calculator::usesIntervalArithmetic() const {return i_start_interval > 0 || (b_interval && i_stop_interval <= 0);}
void Calculator::beginTemporaryStopIntervalArithmetic() {
	i_stop_interval++;std::cout << "1533" << endl;
}
void Calculator::endTemporaryStopIntervalArithmetic() {
	i_stop_interval--;std::cout << "1536" << endl;
}
void Calculator::beginTemporaryEnableIntervalArithmetic() {
	i_start_interval++;std::cout << "1539" << endl;
}
void Calculator::endTemporaryEnableIntervalArithmetic() {
	i_start_interval--;std::cout << "1542" << endl;
}

void Calculator::setCustomInputBase(Number nr) {
	priv->custom_input_base = nr;std::cout << "1546" << endl;
	if(!nr.isReal()) {
		priv->custom_input_base_i = LONG_MAX;std::cout << "1548" << endl;
	} else {
		nr.abs();std::cout << "1550" << endl; nr.ceil();std::cout << "1550" << endl;
		priv->custom_input_base_i = nr.lintValue();std::cout << "1551" << endl;
		if(priv->custom_input_base_i < 2) priv->custom_input_base_i = 2;
	}
}
void Calculator::setCustomOutputBase(Number nr) {priv->custom_output_base = nr;}
const Number &Calculator::customInputBase() const {return priv->custom_input_base;}
const Number &Calculator::customOutputBase() const {return priv->custom_output_base;}

const string &Calculator::getDecimalPoint() const {return DOT_STR;}
const string &Calculator::getComma() const {return COMMA_STR;}
string Calculator::localToString(bool include_spaces) const {
	if(include_spaces) return string(SPACE) + string(_("to")) + SPACE;
	else return _("to");
}
string Calculator::localWhereString() const {
	return string(SPACE) + string(_("where")) + SPACE;
}
void Calculator::setLocale() {
	if(b_ignore_locale) return;
	if(saved_locale) setlocale(LC_NUMERIC, saved_locale);
	lconv *locale = localeconv();std::cout << "1571" << endl;
	if(strcmp(locale->decimal_point, ",") == 0) {
		DOT_STR = ",";
		DOT_S = ".,";
		COMMA_STR = ";";
		COMMA_S = ";";
	} else {
		DOT_STR = ".";
		DOT_S = ".";
		COMMA_STR = ",";
		COMMA_S = ",;";
	}
	setlocale(LC_NUMERIC, "C");
}
void Calculator::setIgnoreLocale() {
	if(saved_locale) {
		free(saved_locale);std::cout << "1587" << endl;
		saved_locale = NULL;std::cout << "1588" << endl;
	}
	char *current_lc_monetary = setlocale(LC_MONETARY, NULL);std::cout << "1590" << endl;
	if(current_lc_monetary) saved_locale = strdup(current_lc_monetary);
	else saved_locale = NULL;std::cout << "1592" << endl;
	setlocale(LC_ALL, "C");
#ifdef ENABLE_NLS
#	ifdef _WIN32
	bindtextdomain(GETTEXT_PACKAGE, "NULL");
	bind_textdomain_codeset(GETTEXT_PACKAGE, "UTF-8");
#	endif
#endif
	if(saved_locale) {
		setlocale(LC_MONETARY, saved_locale);std::cout << "1601" << endl;
		free(saved_locale);std::cout << "1602" << endl;
		saved_locale = NULL;std::cout << "1603" << endl;
	}
	b_ignore_locale = true;std::cout << "1605" << endl;
	per_str = "per";
	per_str_len = per_str.length();std::cout << "1607" << endl;
	times_str = "times";
	times_str_len = times_str.length();std::cout << "1609" << endl;
	plus_str = "plus";
	plus_str_len = plus_str.length();std::cout << "1611" << endl;
	minus_str = "minus";
	minus_str_len = minus_str.length();std::cout << "1613" << endl;
	and_str = "";
	and_str_len = 0;std::cout << "1615" << endl;
	or_str = "";
	or_str_len = 0;std::cout << "1617" << endl;
	local_to = false;std::cout << "1618" << endl;
	unsetLocale();std::cout << "1619" << endl;
}
bool Calculator::getIgnoreLocale() {
	return b_ignore_locale;
}
void Calculator::useDecimalComma() {
	DOT_STR = ",";
	DOT_S = ".,";
	COMMA_STR = ";";
	COMMA_S = ";";
}
void Calculator::useDecimalPoint(bool use_comma_as_separator) {
	DOT_STR = ".";
	DOT_S = ".";
	if(use_comma_as_separator) {
		COMMA_STR = ";";
		COMMA_S = ";";
	} else {
		COMMA_STR = ",";
		COMMA_S = ",;";
	}
}
void Calculator::unsetLocale() {
	COMMA_STR = ",";
	COMMA_S = ",;";
	DOT_STR = ".";
	DOT_S = ".";
}

size_t Calculator::addId(MathStructure *mstruct, bool persistent) {
	size_t id = 0;std::cout << "1649" << endl;
	if(priv->freed_ids.size() > 0) {
		id = priv->freed_ids.back();std::cout << "1651" << endl;
		priv->freed_ids.pop_back();std::cout << "1652" << endl;
	} else {
		priv->ids_i++;std::cout << "1654" << endl;
		id = priv->ids_i;std::cout << "1655" << endl;
	}
	priv->ids_p[id] = persistent;std::cout << "1657" << endl;
	priv->id_structs[id] = mstruct;std::cout << "1658" << endl;
	return id;
}
size_t Calculator::parseAddId(MathFunction *f, const string &str, const ParseOptions &po, bool persistent) {
	size_t id = 0;std::cout << "1662" << endl;
	if(priv->freed_ids.size() > 0) {
		id = priv->freed_ids.back();std::cout << "1664" << endl;
		priv->freed_ids.pop_back();std::cout << "1665" << endl;
	} else {
		priv->ids_i++;std::cout << "1667" << endl;
		id = priv->ids_i;std::cout << "1668" << endl;
	}
	priv->ids_p[id] = persistent;std::cout << "1670" << endl;
	priv->id_structs[id] = new MathStructure();std::cout << "1671" << endl;
	f->parse(*priv->id_structs[id], str, po);std::cout << "1672" << endl;
	return id;
}
size_t Calculator::parseAddIdAppend(MathFunction *f, const MathStructure &append_mstruct, const string &str, const ParseOptions &po, bool persistent) {
	size_t id = 0;std::cout << "1676" << endl;
	if(priv->freed_ids.size() > 0) {
		id = priv->freed_ids.back();std::cout << "1678" << endl;
		priv->freed_ids.pop_back();std::cout << "1679" << endl;
	} else {
		priv->ids_i++;std::cout << "1681" << endl;
		id = priv->ids_i;std::cout << "1682" << endl;
	}
	priv->ids_p[id] = persistent;std::cout << "1684" << endl;
	priv->id_structs[id] = new MathStructure();std::cout << "1685" << endl;
	f->parse(*priv->id_structs[id], str, po);std::cout << "1686" << endl;
	priv->id_structs[id]->addChild(append_mstruct);std::cout << "1687" << endl;
	return id;
}
size_t Calculator::parseAddVectorId(const string &str, const ParseOptions &po, bool persistent) {
	size_t id = 0;std::cout << "1691" << endl;
	if(priv->freed_ids.size() > 0) {
		id = priv->freed_ids.back();std::cout << "1693" << endl;
		priv->freed_ids.pop_back();std::cout << "1694" << endl;
	} else {
		priv->ids_i++;std::cout << "1696" << endl;
		id = priv->ids_i;std::cout << "1697" << endl;
	}
	priv->ids_p[id] = persistent;std::cout << "1699" << endl;
	priv->id_structs[id] = new MathStructure();std::cout << "1700" << endl;
	f_vector->args(str, *priv->id_structs[id], po);std::cout << "1701" << endl;
	return id;
}
MathStructure *Calculator::getId(size_t id) {
	if(priv->id_structs.find(id) != priv->id_structs.end()) {
		if(priv->ids_p[id]) {
			return new MathStructure(*priv->id_structs[id]);
		} else {
			MathStructure *mstruct = priv->id_structs[id];std::cout << "1709" << endl;
			priv->freed_ids.push_back(id);std::cout << "1710" << endl;
			priv->id_structs.erase(id);std::cout << "1711" << endl;
			priv->ids_p.erase(id);std::cout << "1712" << endl;
			return mstruct;
		}
	}
	return NULL;
}

void Calculator::delId(size_t id) {
	if(priv->ids_p.find(id) != priv->ids_p.end()) {
		priv->freed_ids.push_back(id);std::cout << "1721" << endl;
		priv->id_structs[id]->unref();std::cout << "1722" << endl;
		priv->id_structs.erase(id);std::cout << "1723" << endl;
		priv->ids_p.erase(id);std::cout << "1724" << endl;
	}
}

void Calculator::resetVariables() {
	variables.clear();std::cout << "1729" << endl;
	addBuiltinVariables();std::cout << "1730" << endl;
}
void Calculator::resetFunctions() {
	functions.clear();std::cout << "1733" << endl;
	addBuiltinFunctions();std::cout << "1734" << endl;
}
void Calculator::resetUnits() {
	units.clear();std::cout << "1737" << endl;
	addBuiltinUnits();std::cout << "1738" << endl;
}
void Calculator::reset() {
	resetVariables();std::cout << "1741" << endl;
	resetFunctions();std::cout << "1742" << endl;
	resetUnits();std::cout << "1743" << endl;
}

#ifdef __linux__
#	include <sys/sysinfo.h>
#endif

#if defined __linux__ || defined _WIN32
class UptimeVariable : public DynamicVariable {
  private:
	void calculate(MathStructure &m) const {
		Number nr;std::cout << "1754" << endl;
#	ifdef __linux__
		std::ifstream proc_uptime("/proc/uptime", std::ios::in);
		if(proc_uptime.is_open()) {
			string s_uptime;std::cout << "1758" << endl;
			getline(proc_uptime, s_uptime, ' ');std::cout << "1759" << endl;
			nr.set(s_uptime);std::cout << "1760" << endl;
		} else {
			struct sysinfo sf;std::cout << "1762" << endl;
			if(!sysinfo(&sf)) nr = (long int) sf.uptime;
		}
#	elif _WIN32
		ULONGLONG i_uptime = GetTickCount64();std::cout << "1766" << endl;
		nr.set((long int) (i_uptime % 1000), 1000);std::cout << "1767" << endl;
		nr += (long int) (i_uptime / 1000);std::cout << "1768" << endl;
#	endif
		m = nr;std::cout << "1770" << endl;
		Unit *u = CALCULATOR->getUnit("s");
		if(u) m *= u;
	}
  public:
	UptimeVariable() : DynamicVariable("", "uptime") {
		setApproximate(false);
		always_recalculate = true;
	}
	UptimeVariable(const UptimeVariable *variable) {set(variable);}
	ExpressionItem *copy() const {return new UptimeVariable(this);}
};
#endif

void Calculator::addBuiltinVariables() {

	v_e = (KnownVariable*) addVariable(new EVariable());
	v_pi = (KnownVariable*) addVariable(new PiVariable());
	Number nr(1, 1);
	MathStructure mstruct;
	mstruct.number().setImaginaryPart(nr);
	v_i = (KnownVariable*) addVariable(new KnownVariable("", "i", mstruct, "Imaginary i (sqrt(-1))", false, true));
	mstruct.number().setPlusInfinity();
	v_pinf = (KnownVariable*) addVariable(new KnownVariable("", "plus_infinity", mstruct, "+Infinity", false, true));
	mstruct.number().setMinusInfinity();
	v_minf = (KnownVariable*) addVariable(new KnownVariable("", "minus_infinity", mstruct, "-Infinity", false, true));
	mstruct.setUndefined();
	v_undef = (KnownVariable*) addVariable(new KnownVariable("", "undefined", mstruct, "Undefined", false, true));
	v_euler = (KnownVariable*) addVariable(new EulerVariable());
	v_catalan = (KnownVariable*) addVariable(new CatalanVariable());
	v_precision = (KnownVariable*) addVariable(new PrecisionVariable());
	v_percent = (KnownVariable*) addVariable(new KnownVariable("", "percent", MathStructure(1, 1, -2), "Percent", false, true));
	v_percent->addName("%");
	v_permille = (KnownVariable*) addVariable(new KnownVariable("", "permille", MathStructure(1, 1, -3), "Per Mille", false, true));
	v_permyriad = (KnownVariable*) addVariable(new KnownVariable("", "permyriad", MathStructure(1, 1, -4), "Per Myriad", false, true));
	v_x = (UnknownVariable*) addVariable(new UnknownVariable("", "x", "", true, false));
	v_y = (UnknownVariable*) addVariable(new UnknownVariable("", "y", "", true, false));
	v_z = (UnknownVariable*) addVariable(new UnknownVariable("", "z", "", true, false));
	v_C = new UnknownVariable("", "C", "", false, true);
	v_C->setAssumptions(new Assumptions());std::cout << "1809" << endl;
	v_n = (UnknownVariable*) addVariable(new UnknownVariable("", "n", "", false, true));
	v_n->setAssumptions(new Assumptions());std::cout << "1811" << endl;
	v_n->assumptions()->setType(ASSUMPTION_TYPE_INTEGER);std::cout << "1812" << endl;
	v_today = (KnownVariable*) addVariable(new TodayVariable());std::cout << "1813" << endl;
	v_yesterday = (KnownVariable*) addVariable(new YesterdayVariable());std::cout << "1814" << endl;
	v_tomorrow = (KnownVariable*) addVariable(new TomorrowVariable());std::cout << "1815" << endl;
	v_now = (KnownVariable*) addVariable(new NowVariable());std::cout << "1816" << endl;
#if defined __linux__ || defined _WIN32
	addVariable(new UptimeVariable());std::cout << "1818" << endl;
#endif

}

void Calculator::addBuiltinFunctions() {

	f_vector = addFunction(new VectorFunction());std::cout << "1825" << endl;
	f_sort = addFunction(new SortFunction());std::cout << "1826" << endl;
	f_rank = addFunction(new RankFunction());std::cout << "1827" << endl;
	f_limits = addFunction(new LimitsFunction());std::cout << "1828" << endl;
	//f_component = addFunction(new ComponentFunction());std::cout << "1829" << endl;
	f_dimension = addFunction(new DimensionFunction());std::cout << "1830" << endl;
	f_merge_vectors = addFunction(new MergeVectorsFunction());std::cout << "1831" << endl;
	f_matrix = addFunction(new MatrixFunction());std::cout << "1832" << endl;
	f_matrix_to_vector = addFunction(new MatrixToVectorFunction());std::cout << "1833" << endl;
	f_area = addFunction(new AreaFunction());std::cout << "1834" << endl;
	f_rows = addFunction(new RowsFunction());std::cout << "1835" << endl;
	f_columns = addFunction(new ColumnsFunction());std::cout << "1836" << endl;
	f_row = addFunction(new RowFunction());std::cout << "1837" << endl;
	f_column = addFunction(new ColumnFunction());std::cout << "1838" << endl;
	f_elements = addFunction(new ElementsFunction());std::cout << "1839" << endl;
	f_element = addFunction(new ElementFunction());std::cout << "1840" << endl;
	f_transpose = addFunction(new TransposeFunction());std::cout << "1841" << endl;
	f_identity = addFunction(new IdentityFunction());std::cout << "1842" << endl;
	f_determinant = addFunction(new DeterminantFunction());std::cout << "1843" << endl;
	f_permanent = addFunction(new PermanentFunction());std::cout << "1844" << endl;
	f_adjoint = addFunction(new AdjointFunction());std::cout << "1845" << endl;
	f_cofactor = addFunction(new CofactorFunction());std::cout << "1846" << endl;
	f_inverse = addFunction(new InverseFunction());std::cout << "1847" << endl;
	f_magnitude = addFunction(new MagnitudeFunction());std::cout << "1848" << endl;
	f_hadamard = addFunction(new HadamardFunction());std::cout << "1849" << endl;
	f_entrywise = addFunction(new EntrywiseFunction());std::cout << "1850" << endl;

	f_factorial = addFunction(new FactorialFunction());std::cout << "1852" << endl;
	f_factorial2 = addFunction(new DoubleFactorialFunction());std::cout << "1853" << endl;
	f_multifactorial = addFunction(new MultiFactorialFunction());
	f_binomial = addFunction(new BinomialFunction());std::cout << "1855" << endl;

	f_xor = addFunction(new XorFunction());std::cout << "1857" << endl;
	f_bitxor = addFunction(new BitXorFunction());std::cout << "1858" << endl;
	f_even = addFunction(new EvenFunction());std::cout << "1859" << endl;
	f_odd = addFunction(new OddFunction());std::cout << "1860" << endl;
	f_shift = addFunction(new ShiftFunction());
	f_bitcmp = addFunction(new BitCmpFunction());std::cout << "1862" << endl;
	addFunction(new CircularShiftFunction());

	f_abs = addFunction(new AbsFunction());std::cout << "1865" << endl;
	f_signum = addFunction(new SignumFunction());std::cout << "1866" << endl;
	f_heaviside = addFunction(new HeavisideFunction());std::cout << "1867" << endl;
	f_dirac = addFunction(new DiracFunction());std::cout << "1868" << endl;
	f_gcd = addFunction(new GcdFunction());std::cout << "1869" << endl;
	f_lcm = addFunction(new LcmFunction());std::cout << "1870" << endl;
	f_round = addFunction(new RoundFunction());std::cout << "1871" << endl;
	f_floor = addFunction(new FloorFunction());std::cout << "1872" << endl;
	f_ceil = addFunction(new CeilFunction());std::cout << "1873" << endl;
	f_trunc = addFunction(new TruncFunction());std::cout << "1874" << endl;
	f_int = addFunction(new IntFunction());std::cout << "1875" << endl;
	f_frac = addFunction(new FracFunction());std::cout << "1876" << endl;
	f_rem = addFunction(new RemFunction());std::cout << "1877" << endl;
	f_mod = addFunction(new ModFunction());std::cout << "1878" << endl;

	f_polynomial_unit = addFunction(new PolynomialUnitFunction());std::cout << "1880" << endl;
	f_polynomial_primpart = addFunction(new PolynomialPrimpartFunction());std::cout << "1881" << endl;
	f_polynomial_content = addFunction(new PolynomialContentFunction());std::cout << "1882" << endl;
	f_coeff = addFunction(new CoeffFunction());std::cout << "1883" << endl;
	f_lcoeff = addFunction(new LCoeffFunction());std::cout << "1884" << endl;
	f_tcoeff = addFunction(new TCoeffFunction());std::cout << "1885" << endl;
	f_degree = addFunction(new DegreeFunction());std::cout << "1886" << endl;
	f_ldegree = addFunction(new LDegreeFunction());std::cout << "1887" << endl;

	f_re = addFunction(new ReFunction());std::cout << "1889" << endl;
	f_im = addFunction(new ImFunction());std::cout << "1890" << endl;
	f_arg = addFunction(new ArgFunction());std::cout << "1891" << endl;
	f_numerator = addFunction(new NumeratorFunction());std::cout << "1892" << endl;
	f_denominator = addFunction(new DenominatorFunction());std::cout << "1893" << endl;

	f_interval = addFunction(new IntervalFunction());std::cout << "1895" << endl;
	f_uncertainty = addFunction(new UncertaintyFunction());std::cout << "1896" << endl;

	f_sqrt = addFunction(new SqrtFunction());std::cout << "1898" << endl;
	f_cbrt = addFunction(new CbrtFunction());std::cout << "1899" << endl;
	f_root = addFunction(new RootFunction());std::cout << "1900" << endl;
	f_sq = addFunction(new SquareFunction());std::cout << "1901" << endl;

	f_exp = addFunction(new ExpFunction());std::cout << "1903" << endl;

	f_ln = addFunction(new LogFunction());std::cout << "1905" << endl;
	f_logn = addFunction(new LognFunction());std::cout << "1906" << endl;

	f_lambert_w = addFunction(new LambertWFunction());std::cout << "1908" << endl;

	f_sin = addFunction(new SinFunction());std::cout << "1910" << endl;
	f_cos = addFunction(new CosFunction());std::cout << "1911" << endl;
	f_tan = addFunction(new TanFunction());std::cout << "1912" << endl;
	f_asin = addFunction(new AsinFunction());std::cout << "1913" << endl;
	f_acos = addFunction(new AcosFunction());std::cout << "1914" << endl;
	f_atan = addFunction(new AtanFunction());std::cout << "1915" << endl;
	f_sinh = addFunction(new SinhFunction());std::cout << "1916" << endl;
	f_cosh = addFunction(new CoshFunction());std::cout << "1917" << endl;
	f_tanh = addFunction(new TanhFunction());std::cout << "1918" << endl;
	f_asinh = addFunction(new AsinhFunction());std::cout << "1919" << endl;
	f_acosh = addFunction(new AcoshFunction());std::cout << "1920" << endl;
	f_atanh = addFunction(new AtanhFunction());std::cout << "1921" << endl;
	f_atan2 = addFunction(new Atan2Function());std::cout << "1922" << endl;
	f_sinc = addFunction(new SincFunction());std::cout << "1923" << endl;
	priv->f_cis = addFunction(new CisFunction());std::cout << "1924" << endl;
	f_radians_to_default_angle_unit = addFunction(new RadiansToDefaultAngleUnitFunction());std::cout << "1925" << endl;

	f_zeta = addFunction(new ZetaFunction());std::cout << "1927" << endl;
	f_gamma = addFunction(new GammaFunction());std::cout << "1928" << endl;
	f_digamma = addFunction(new DigammaFunction());std::cout << "1929" << endl;
	f_beta = addFunction(new BetaFunction());std::cout << "1930" << endl;
	f_airy = addFunction(new AiryFunction());std::cout << "1931" << endl;
	f_besselj = addFunction(new BesseljFunction());std::cout << "1932" << endl;
	f_bessely = addFunction(new BesselyFunction());std::cout << "1933" << endl;
	f_erf = addFunction(new ErfFunction());std::cout << "1934" << endl;
	f_erfc = addFunction(new ErfcFunction());std::cout << "1935" << endl;

	f_total = addFunction(new TotalFunction());std::cout << "1937" << endl;
	f_percentile = addFunction(new PercentileFunction());std::cout << "1938" << endl;
	f_min = addFunction(new MinFunction());std::cout << "1939" << endl;
	f_max = addFunction(new MaxFunction());std::cout << "1940" << endl;
	f_mode = addFunction(new ModeFunction());std::cout << "1941" << endl;
	f_rand = addFunction(new RandFunction());std::cout << "1942" << endl;
	addFunction(new RandnFunction());std::cout << "1943" << endl;
	addFunction(new RandPoissonFunction());std::cout << "1944" << endl;

	f_date = addFunction(new DateFunction());std::cout << "1946" << endl;
	f_datetime = addFunction(new DateTimeFunction());std::cout << "1947" << endl;
	f_timevalue = addFunction(new TimeValueFunction());std::cout << "1948" << endl;
	f_timestamp = addFunction(new TimestampFunction());std::cout << "1949" << endl;
	f_stamptodate = addFunction(new TimestampToDateFunction());std::cout << "1950" << endl;
	f_days = addFunction(new DaysFunction());std::cout << "1951" << endl;
	f_yearfrac = addFunction(new YearFracFunction());std::cout << "1952" << endl;
	f_week = addFunction(new WeekFunction());std::cout << "1953" << endl;
	f_weekday = addFunction(new WeekdayFunction());std::cout << "1954" << endl;
	f_month = addFunction(new MonthFunction());std::cout << "1955" << endl;
	f_day = addFunction(new DayFunction());std::cout << "1956" << endl;
	f_year = addFunction(new YearFunction());std::cout << "1957" << endl;
	f_yearday = addFunction(new YeardayFunction());std::cout << "1958" << endl;
	f_time = addFunction(new TimeFunction());std::cout << "1959" << endl;
	f_add_days = addFunction(new AddDaysFunction());std::cout << "1960" << endl;
	f_add_months = addFunction(new AddMonthsFunction());std::cout << "1961" << endl;
	f_add_years = addFunction(new AddYearsFunction());std::cout << "1962" << endl;

	f_lunarphase = addFunction(new LunarPhaseFunction());std::cout << "1964" << endl;
	f_nextlunarphase = addFunction(new NextLunarPhaseFunction());std::cout << "1965" << endl;

	f_base = addFunction(new BaseFunction());std::cout << "1967" << endl;
	f_bin = addFunction(new BinFunction());std::cout << "1968" << endl;
	f_oct = addFunction(new OctFunction());std::cout << "1969" << endl;
	addFunction(new DecFunction());std::cout << "1970" << endl;
	f_hex = addFunction(new HexFunction());std::cout << "1971" << endl;
	f_roman = addFunction(new RomanFunction());std::cout << "1972" << endl;
	addFunction(new BijectiveFunction());std::cout << "1973" << endl;

	f_ascii = addFunction(new AsciiFunction());std::cout << "1975" << endl;
	f_char = addFunction(new CharFunction());std::cout << "1976" << endl;

	f_length = addFunction(new LengthFunction());std::cout << "1978" << endl;
	f_concatenate = addFunction(new ConcatenateFunction());std::cout << "1979" << endl;

	f_replace = addFunction(new ReplaceFunction());std::cout << "1981" << endl;
	f_stripunits = addFunction(new StripUnitsFunction());std::cout << "1982" << endl;

	f_genvector = addFunction(new GenerateVectorFunction());std::cout << "1984" << endl;
	f_for = addFunction(new ForFunction());
	f_sum = addFunction(new SumFunction());std::cout << "1986" << endl;
	f_product = addFunction(new ProductFunction());std::cout << "1987" << endl;
	f_process = addFunction(new ProcessFunction());std::cout << "1988" << endl;
	f_process_matrix = addFunction(new ProcessMatrixFunction());std::cout << "1989" << endl;
	f_csum = addFunction(new CustomSumFunction());std::cout << "1990" << endl;
	f_function = addFunction(new FunctionFunction());std::cout << "1991" << endl;
	f_select = addFunction(new SelectFunction());std::cout << "1992" << endl;
	f_title = addFunction(new TitleFunction());std::cout << "1993" << endl;
	f_if = addFunction(new IFFunction());
	f_is_number = addFunction(new IsNumberFunction());std::cout << "1995" << endl;
	f_is_real = addFunction(new IsRealFunction());std::cout << "1996" << endl;
	f_is_rational = addFunction(new IsRationalFunction());std::cout << "1997" << endl;
	f_is_integer = addFunction(new IsIntegerFunction());std::cout << "1998" << endl;
	f_represents_number = addFunction(new RepresentsNumberFunction());std::cout << "1999" << endl;
	f_represents_real = addFunction(new RepresentsRealFunction());std::cout << "2000" << endl;
	f_represents_rational = addFunction(new RepresentsRationalFunction());std::cout << "2001" << endl;
	f_represents_integer = addFunction(new RepresentsIntegerFunction());std::cout << "2002" << endl;
	f_error = addFunction(new ErrorFunction());std::cout << "2003" << endl;
	f_warning = addFunction(new WarningFunction());std::cout << "2004" << endl;
	f_message = addFunction(new MessageFunction());std::cout << "2005" << endl;

	f_save = addFunction(new SaveFunction());std::cout << "2007" << endl;
	f_load = addFunction(new LoadFunction());std::cout << "2008" << endl;
	f_export = addFunction(new ExportFunction());std::cout << "2009" << endl;

	f_register = addFunction(new RegisterFunction());std::cout << "2011" << endl;
	f_stack = addFunction(new StackFunction());std::cout << "2012" << endl;

	f_diff = addFunction(new DeriveFunction());
	f_integrate = addFunction(new IntegrateFunction());std::cout << "2015" << endl;
	addFunction(new RombergFunction());std::cout << "2016" << endl;
	addFunction(new MonteCarloFunction());std::cout << "2017" << endl;
	f_solve = addFunction(new SolveFunction());std::cout << "2018" << endl;
	f_multisolve = addFunction(new SolveMultipleFunction());std::cout << "2019" << endl;
	f_dsolve = addFunction(new DSolveFunction());std::cout << "2020" << endl;
	f_limit = addFunction(new LimitFunction());std::cout << "2021" << endl;

	f_li = addFunction(new liFunction());std::cout << "2023" << endl;
	f_Li = addFunction(new LiFunction());std::cout << "2024" << endl;
	f_Ei = addFunction(new EiFunction());std::cout << "2025" << endl;
	f_Si = addFunction(new SiFunction());std::cout << "2026" << endl;
	f_Ci = addFunction(new CiFunction());std::cout << "2027" << endl;
	f_Shi = addFunction(new ShiFunction());std::cout << "2028" << endl;
	f_Chi = addFunction(new ChiFunction());std::cout << "2029" << endl;
	f_igamma = addFunction(new IGammaFunction());std::cout << "2030" << endl;

	if(canPlot()) f_plot = addFunction(new PlotFunction());

	/*void *plugin = dlopen("", RTLD_NOW);
	if(plugin) {
		CREATEPLUG_PROC createproc = (CREATEPLUG_PROC) dlsym(plugin, "createPlugin");
		if (dlerror() != NULL) {
			dlclose(plugin);std::cout << "2038" << endl;
			printf( "dlsym error\n");
		} else {
			createproc();std::cout << "2041" << endl;
		}
	} else {
		printf( "dlopen error\n");
	}*/

}
void Calculator::addBuiltinUnits() {
	u_euro = addUnit(new Unit(_("Currency"), "EUR", "euros", "euro", "European Euros", false, true, true));
	u_btc = addUnit(new AliasUnit(_("Currency"), "BTC", "bitcoins", "bitcoin", "Bitcoins", u_euro, "6512.84", 1, "", false, true, true));
	u_btc->setApproximate();std::cout << "2051" << endl;
	u_btc->setPrecision(-2);std::cout << "2052" << endl;
	u_btc->setChanged(false);std::cout << "2053" << endl;
	u_second = NULL;std::cout << "2054" << endl;
	u_minute = NULL;std::cout << "2055" << endl;
	u_hour = NULL;std::cout << "2056" << endl;
	u_day = NULL;std::cout << "2057" << endl;
	u_month = NULL;std::cout << "2058" << endl;
	u_year = NULL;std::cout << "2059" << endl;
}

void Calculator::setVariableUnitsEnabled(bool enable_variable_units) {
	b_var_units = enable_variable_units;std::cout << "2063" << endl;
}
bool Calculator::variableUnitsEnabled() const {
	return b_var_units;
}

void Calculator::error(bool critical, int message_category, const char *TEMPLATE, ...) {
	va_list ap;std::cout << "2070" << endl;
	va_start(ap, TEMPLATE);std::cout << "2071" << endl;
	message(critical ? MESSAGE_ERROR : MESSAGE_WARNING, message_category, TEMPLATE, ap);std::cout << "2072" << endl;
	va_end(ap);std::cout << "2073" << endl;
}
void Calculator::error(bool critical, const char *TEMPLATE, ...) {
	va_list ap;std::cout << "2076" << endl;
	va_start(ap, TEMPLATE);std::cout << "2077" << endl;
	message(critical ? MESSAGE_ERROR : MESSAGE_WARNING, MESSAGE_CATEGORY_NONE, TEMPLATE, ap);std::cout << "2078" << endl;
	va_end(ap);std::cout << "2079" << endl;
}
void Calculator::message(MessageType mtype, int message_category, const char *TEMPLATE, ...) {
	va_list ap;std::cout << "2082" << endl;
	va_start(ap, TEMPLATE);std::cout << "2083" << endl;
	message(mtype, message_category, TEMPLATE, ap);std::cout << "2084" << endl;
	va_end(ap);std::cout << "2085" << endl;
}
void Calculator::message(MessageType mtype, const char *TEMPLATE, ...) {
	va_list ap;std::cout << "2088" << endl;
	va_start(ap, TEMPLATE);std::cout << "2089" << endl;
	message(mtype, MESSAGE_CATEGORY_NONE, TEMPLATE, ap);std::cout << "2090" << endl;
	va_end(ap);std::cout << "2091" << endl;
}
void Calculator::message(MessageType mtype, int message_category, const char *TEMPLATE, va_list ap) {
	if(disable_errors_ref > 0) {
		stopped_messages_count[disable_errors_ref - 1]++;std::cout << "2095" << endl;
		if(mtype == MESSAGE_ERROR) {
			stopped_errors_count[disable_errors_ref - 1]++;std::cout << "2097" << endl;
		} else if(mtype == MESSAGE_WARNING) {
			stopped_warnings_count[disable_errors_ref - 1]++;std::cout << "2099" << endl;
		}
	}
	string error_str = TEMPLATE;std::cout << "2102" << endl;
	size_t i = 0;std::cout << "2103" << endl;
	while(true) {
		i = error_str.find("%", i);
		if(i == string::npos || i + 1 == error_str.length()) break;
		switch(error_str[i + 1]) {
			case 's': {
				const char *str = va_arg(ap, const char*);std::cout << "2109" << endl;
				if(!str) {
					i++;std::cout << "2111" << endl;
				} else {
					error_str.replace(i, 2, str);std::cout << "2113" << endl;
					i += strlen(str);std::cout << "2114" << endl;
				}
				break;
			}
			case 'c': {
				char c = (char) va_arg(ap, int);std::cout << "2119" << endl;
				if(c > 0) {
					error_str.replace(i, 2, 1, c);std::cout << "2121" << endl;
				}
				i++;std::cout << "2123" << endl;
				break;
			}
			default: {
				i++;std::cout << "2127" << endl;
				break;
			}
		}
	}
	bool dup_error = false;std::cout << "2132" << endl;
	for(i = 0; i < messages.size(); i++) {
		if(error_str == messages[i].message()) {
			dup_error = true;std::cout << "2135" << endl;
			break;
		}
	}
	if(disable_errors_ref > 0) {
		for(size_t i2 = 0; !dup_error && i2 < (size_t) disable_errors_ref; i2++) {
			for(i = 0; i < stopped_messages[i2].size(); i++) {
				if(error_str == stopped_messages[i2][i].message()) {
					dup_error = true;std::cout << "2143" << endl;
					break;
				}
			}
		}
	}
	if(!dup_error) {
		if(disable_errors_ref > 0) stopped_messages[disable_errors_ref - 1].push_back(CalculatorMessage(error_str, mtype, message_category, current_stage));
		else messages.push_back(CalculatorMessage(error_str, mtype, message_category, current_stage));std::cout << "2151" << endl;
	}
}
CalculatorMessage* Calculator::message() {
	if(!messages.empty()) {
		return &messages[0];
	}
	return NULL;
}
CalculatorMessage* Calculator::nextMessage() {
	if(!messages.empty()) {
		messages.erase(messages.begin());std::cout << "2162" << endl;
		if(!messages.empty()) {
			return &messages[0];
		}
	}
	return NULL;
}
void Calculator::clearMessages() {
	messages.clear();std::cout << "2170" << endl;
}
void Calculator::cleanMessages(const MathStructure &mstruct, size_t first_message) {
	if(first_message > 0) first_message--;
	if(messages.size() <= first_message) return;
	if(mstruct.containsInterval(true, false, false, -2, true) <= 0) {
		for(size_t i = messages.size() - 1; ; i--) {
			if(messages[i].category() == MESSAGE_CATEGORY_WIDE_INTERVAL) {
				messages.erase(messages.begin() + i);std::cout << "2178" << endl;
			}
			if(i == first_message) break;
		}
	}
}
void Calculator::deleteName(string name_, ExpressionItem *object) {
	Variable *v2 = getVariable(name_);std::cout << "2185" << endl;
	if(v2 == object) {
		return;
	}
	if(v2 != NULL) {
		v2->destroy();std::cout << "2190" << endl;
	} else {
		MathFunction *f2 = getFunction(name_);std::cout << "2192" << endl;
		if(f2 == object)			return;
		if(f2 != NULL) {
			f2->destroy();std::cout << "2195" << endl;
		}
	}
	deleteName(name_, object);std::cout << "2198" << endl;
}
void Calculator::deleteUnitName(string name_, Unit *object) {
	Unit *u2 = getUnit(name_);std::cout << "2201" << endl;
	if(u2) {
		if(u2 != object) {
			u2->destroy();std::cout << "2204" << endl;
		}
		return;
	}
	u2 = getCompositeUnit(name_);std::cout << "2208" << endl;
	if(u2) {
		if(u2 != object) {
			u2->destroy();std::cout << "2211" << endl;
		}
	}
	deleteUnitName(name_, object);std::cout << "2214" << endl;
}
void Calculator::saveState() {
}
void Calculator::restoreState() {
}
void Calculator::clearBuffers() {
	for(unordered_map<size_t, bool>::iterator it = priv->ids_p.begin(); it != priv->ids_p.end(); ++it) {
		if(!it->second) {
			priv->freed_ids.push_back(it->first);std::cout << "2223" << endl;
			priv->id_structs.erase(it->first);std::cout << "2224" << endl;
			priv->ids_p.erase(it);std::cout << "2225" << endl;
		}
	}
}
bool Calculator::abort() {
	i_aborted = 1;std::cout << "2230" << endl;
	if(!b_busy) return true;
	if(!calculate_thread->running) {
		b_busy = false;std::cout << "2233" << endl;
	} else {
		int msecs = 5000;std::cout << "2235" << endl;
		while(b_busy && msecs > 0) {
			sleep_ms(10);std::cout << "2237" << endl;
			msecs -= 10;std::cout << "2238" << endl;
		}
		if(b_busy) {
			calculate_thread->cancel();std::cout << "2241" << endl;
			stopControl();std::cout << "2242" << endl;
			stopped_messages_count.clear();std::cout << "2243" << endl;
			stopped_warnings_count.clear();std::cout << "2244" << endl;
			stopped_errors_count.clear();std::cout << "2245" << endl;
			stopped_messages.clear();std::cout << "2246" << endl;
			disable_errors_ref = 0;std::cout << "2247" << endl;
			if(tmp_rpn_mstruct) tmp_rpn_mstruct->unref();
			tmp_rpn_mstruct = NULL;std::cout << "2249" << endl;
			error(true, _("The calculation has been forcibly terminated. Please restart the application and report this as a bug."), NULL);
			b_busy = false;std::cout << "2251" << endl;
			calculate_thread->start();std::cout << "2252" << endl;
			return false;
		}
	}
	return true;
}
bool Calculator::busy() {
	return b_busy;
}
void Calculator::terminateThreads() {
	if(calculate_thread->running) {
		if(!calculate_thread->write(false) || !calculate_thread->write(NULL)) calculate_thread->cancel();
		for(size_t i = 0; i < 10 && calculate_thread->running; i++) {
			sleep_ms(1);std::cout << "2265" << endl;
		}
		if(calculate_thread->running) calculate_thread->cancel();
	}
}

string Calculator::localizeExpression(string str, const ParseOptions &po) const {
	if((DOT_STR == DOT && COMMA_STR == COMMA && !po.comma_as_separator) || po.base == BASE_UNICODE || (po.base == BASE_CUSTOM && priv->custom_input_base_i > 62)) return str;
	vector<size_t> q_begin;std::cout << "2273" << endl;
	vector<size_t> q_end;std::cout << "2274" << endl;
	size_t i3 = 0;std::cout << "2275" << endl;
	while(true) {
		i3 = str.find_first_of("\"\'", i3);
		if(i3 == string::npos) {
			break;
		}
		q_begin.push_back(i3);std::cout << "2281" << endl;
		i3 = str.find(str[i3], i3 + 1);std::cout << "2282" << endl;
		if(i3 == string::npos) {
			q_end.push_back(str.length() - 1);std::cout << "2284" << endl;
			break;
		}
		q_end.push_back(i3);std::cout << "2287" << endl;
		i3++;std::cout << "2288" << endl;
	}
	if(COMMA_STR != COMMA || po.comma_as_separator) {
		bool b_alt_comma = po.comma_as_separator && COMMA_STR == COMMA;std::cout << "2291" << endl;
		size_t ui = str.find(COMMA);std::cout << "2292" << endl;
		while(ui != string::npos) {
			bool b = false;std::cout << "2294" << endl;
			for(size_t ui2 = 0; ui2 < q_end.size(); ui2++) {
				if(ui <= q_end[ui2] && ui >= q_begin[ui2]) {
					ui = str.find(COMMA, q_end[ui2] + 1);std::cout << "2297" << endl;
					b = true;std::cout << "2298" << endl;
					break;
				}
			}
			if(!b) {
				str.replace(ui, strlen(COMMA), b_alt_comma ? ";" : COMMA_STR);
				ui = str.find(COMMA, ui + (b_alt_comma ? 1 : COMMA_STR.length()));std::cout << "2304" << endl;
			}
		}
	}
	if(DOT_STR != DOT) {
		size_t ui = str.find(DOT);std::cout << "2309" << endl;
		while(ui != string::npos) {
			bool b = false;std::cout << "2311" << endl;
			for(size_t ui2 = 0; ui2 < q_end.size(); ui2++) {
				if(ui <= q_end[ui2] && ui >= q_begin[ui2]) {
					ui = str.find(DOT, q_end[ui2] + 1);std::cout << "2314" << endl;
					b = true;std::cout << "2315" << endl;
					break;
				}
			}
			if(!b) {
				str.replace(ui, strlen(DOT), DOT_STR);std::cout << "2320" << endl;
				ui = str.find(DOT, ui + DOT_STR.length());std::cout << "2321" << endl;
			}
		}
	}
	return str;
}
string Calculator::unlocalizeExpression(string str, const ParseOptions &po) const {
	if((DOT_STR == DOT && COMMA_STR == COMMA && !po.comma_as_separator) || po.base == BASE_UNICODE || (po.base == BASE_CUSTOM && priv->custom_input_base_i > 62)) return str;
	int base = po.base;std::cout << "2329" << endl;
	if(base == BASE_CUSTOM) {
		base = (int) priv->custom_input_base_i;std::cout << "2331" << endl;
	} else if(base == BASE_BIJECTIVE_26) {
		base = 36;std::cout << "2333" << endl;
	} else if(base == BASE_GOLDEN_RATIO || base == BASE_SUPER_GOLDEN_RATIO || base == BASE_SQRT2) {
		base = 2;std::cout << "2335" << endl;
	} else if(base == BASE_PI) {
		base = 4;std::cout << "2337" << endl;
	} else if(base == BASE_E) {
		base = 3;std::cout << "2339" << endl;
	} else if(base == BASE_DUODECIMAL) {
		base = -12;std::cout << "2341" << endl;
	} else if(base < 2 || base > 36) {
		base = -1;std::cout << "2343" << endl;
	}
	vector<size_t> q_begin;std::cout << "2345" << endl;
	vector<size_t> q_end;std::cout << "2346" << endl;
	size_t i3 = 0;std::cout << "2347" << endl;
	while(true) {
		i3 = str.find_first_of("\"\'", i3);
		if(i3 == string::npos) {
			break;
		}
		q_begin.push_back(i3);std::cout << "2353" << endl;
		i3 = str.find(str[i3], i3 + 1);std::cout << "2354" << endl;
		if(i3 == string::npos) {
			q_end.push_back(str.length() - 1);std::cout << "2356" << endl;
			break;
		}
		q_end.push_back(i3);std::cout << "2359" << endl;
		i3++;std::cout << "2360" << endl;
	}
	if(DOT_STR != DOT) {
		if(DOT_STR == COMMA && str.find(COMMA_STR) == string::npos && base > 0 && base <= 10) {
			bool b_vector = (str.find(LEFT_VECTOR_WRAP) != string::npos && !po.dot_as_separator);std::cout << "2364" << endl;
			bool b_dot = (str.find(DOT) != string::npos);std::cout << "2365" << endl;
			size_t ui = str.find_first_of(b_vector ? DOT COMMA : COMMA);std::cout << "2366" << endl;
			size_t ui2 = 0;std::cout << "2367" << endl;
			while(ui != string::npos) {
				for(; ui2 < q_end.size(); ui2++) {
					if(ui >= q_begin[ui2]) {
						if(ui <= q_end[ui2]) {
							ui = str.find_first_of(b_vector ? DOT COMMA : COMMA, q_end[ui2] + 1);std::cout << "2372" << endl;
							if(ui == string::npos) break;
						}
					} else {
						break;
					}
				}
				if(ui == string::npos) break;
				if(ui > 0) {
					size_t ui3 = str.find_last_not_of(SPACES, ui - 1);std::cout << "2381" << endl;
					if(ui3 != string::npos && ((str[ui3] > 'a' && str[ui3] < 'z') || (str[ui3] > 'A' && str[ui3] < 'Z')) && is_not_number(str[ui3], base)) return str;
				}
				if(ui != str.length() - 1) {
					size_t ui3 = str.find_first_not_of(SPACES, ui + 1);std::cout << "2385" << endl;
					if(ui3 != string::npos && is_not_number(str[ui3], base)) return str;
					if(b_vector || !b_dot) {
						ui3 = str.find_first_not_of(SPACES NUMBERS, ui3 + 1);std::cout << "2388" << endl;
						if(ui3 != string::npos && (str[ui3] == COMMA_CH || (b_vector && str[ui3] == DOT_CH))) return str;
					}
				}
				ui = str.find(b_vector ? DOT COMMA : COMMA, ui + 1);std::cout << "2392" << endl;
			}
		}
		if(po.dot_as_separator) {
			size_t ui = str.find(DOT);std::cout << "2396" << endl;
			size_t ui2 = 0;std::cout << "2397" << endl;
			while(ui != string::npos) {
				for(; ui2 < q_end.size(); ui2++) {
					if(ui >= q_begin[ui2]) {
						if(ui <= q_end[ui2]) {
							ui = str.find(DOT, q_end[ui2] + 1);std::cout << "2402" << endl;
							if(ui == string::npos) break;
						}
					} else {
						break;
					}
				}
				if(ui == string::npos) break;
				str.replace(ui, strlen(DOT), SPACE);std::cout << "2410" << endl;
				ui = str.find(DOT, ui + strlen(SPACE));std::cout << "2411" << endl;
			}
		}
		size_t ui2 = 0;std::cout << "2414" << endl;
		size_t ui = str.find(DOT_STR);std::cout << "2415" << endl;
		while(ui != string::npos) {
			for(; ui2 < q_end.size(); ui2++) {
				if(ui >= q_begin[ui2]) {
					if(ui <= q_end[ui2]) {
						ui = str.find(DOT_STR, q_end[ui2] + 1);std::cout << "2420" << endl;
						if(ui == string::npos) break;
					}
				} else {
					break;
				}
			}
			if(ui == string::npos) break;
			str.replace(ui, DOT_STR.length(), DOT);std::cout << "2428" << endl;
			ui = str.find(DOT_STR, ui + strlen(DOT));std::cout << "2429" << endl;
		}
	}
	if(COMMA_STR != COMMA || po.comma_as_separator) {
		bool b_alt_comma = po.comma_as_separator && COMMA_STR == COMMA;std::cout << "2433" << endl;
		if(po.comma_as_separator) {
			size_t ui = str.find(COMMA);std::cout << "2435" << endl;
			size_t ui2 = 0;std::cout << "2436" << endl;
			while(ui != string::npos) {
				for(; ui2 < q_end.size(); ui2++) {
					if(ui >= q_begin[ui2]) {
						if(ui <= q_end[ui2]) {
							ui = str.find(COMMA, q_end[ui2] + 1);std::cout << "2441" << endl;
							if(ui == string::npos) break;
						}
					} else {
						break;
					}
				}
				if(ui == string::npos) break;
				str.erase(ui, strlen(COMMA));std::cout << "2449" << endl;
				ui = str.find(COMMA, ui);std::cout << "2450" << endl;
			}
		}
		size_t ui2 = 0;std::cout << "2453" << endl;
		size_t ui = str.find(b_alt_comma ? ";" : COMMA_STR);
		while(ui != string::npos) {
			for(; ui2 < q_end.size(); ui2++) {
				if(ui >= q_begin[ui2]) {
					if(ui <= q_end[ui2]) {
						ui = str.find(b_alt_comma ? ";" : COMMA_STR, q_end[ui2] + 1);
						if(ui == string::npos) break;
					}
				} else {
					break;
				}
			}
			if(ui == string::npos) break;
			str.replace(ui, b_alt_comma ? 1 : COMMA_STR.length(), COMMA);std::cout << "2467" << endl;
			ui = str.find(b_alt_comma ? ";" : COMMA_STR, ui + strlen(COMMA));
		}
	}
	return str;
}

bool Calculator::calculateRPNRegister(size_t index, int msecs, const EvaluationOptions &eo) {
	if(index <= 0 || index > rpn_stack.size()) return false;
	return calculateRPN(new MathStructure(*rpn_stack[rpn_stack.size() - index]), PROC_RPN_SET, index, msecs, eo);
}

bool Calculator::calculateRPN(MathStructure *mstruct, int command, size_t index, int msecs, const EvaluationOptions &eo, int function_arguments) {
	b_busy = true;std::cout << "2480" << endl;
	if(!calculate_thread->running && !calculate_thread->start()) {mstruct->setAborted(); return false;}
	bool had_msecs = msecs > 0;std::cout << "2482" << endl;
	tmp_evaluationoptions = eo;std::cout << "2483" << endl;
	tmp_proc_command = command;std::cout << "2484" << endl;
	tmp_rpnindex = index;std::cout << "2485" << endl;
	tmp_rpn_mstruct = mstruct;std::cout << "2486" << endl;
	tmp_proc_registers = function_arguments;std::cout << "2487" << endl;
	if(!calculate_thread->write(false)) {calculate_thread->cancel(); mstruct->setAborted(); return false;}
	if(!calculate_thread->write((void*) mstruct)) {calculate_thread->cancel(); mstruct->setAborted(); return false;}
	while(msecs > 0 && b_busy) {
		sleep_ms(10);std::cout << "2491" << endl;
		msecs -= 10;std::cout << "2492" << endl;
	}
	if(had_msecs && b_busy) {
		abort();std::cout << "2495" << endl;
		return false;
	}
	return true;
}
bool Calculator::calculateRPN(string str, int command, size_t index, int msecs, const EvaluationOptions &eo, MathStructure *parsed_struct, MathStructure *to_struct, bool make_to_division, int function_arguments) {
	MathStructure *mstruct = new MathStructure();std::cout << "2501" << endl;
	b_busy = true;std::cout << "2502" << endl;
	if(!calculate_thread->running && !calculate_thread->start()) {mstruct->setAborted(); return false;}
	bool had_msecs = msecs > 0;std::cout << "2504" << endl;
	expression_to_calculate = str;std::cout << "2505" << endl;
	tmp_evaluationoptions = eo;std::cout << "2506" << endl;
	tmp_proc_command = command;std::cout << "2507" << endl;
	tmp_rpnindex = index;std::cout << "2508" << endl;
	tmp_rpn_mstruct = mstruct;std::cout << "2509" << endl;
	tmp_parsedstruct = parsed_struct;std::cout << "2510" << endl;
	tmp_tostruct = to_struct;std::cout << "2511" << endl;
	tmp_maketodivision = make_to_division;std::cout << "2512" << endl;
	tmp_proc_registers = function_arguments;std::cout << "2513" << endl;
	if(!calculate_thread->write(true)) {calculate_thread->cancel(); mstruct->setAborted(); return false;}
	if(!calculate_thread->write((void*) mstruct)) {calculate_thread->cancel(); mstruct->setAborted(); return false;}
	while(msecs > 0 && b_busy) {
		sleep_ms(10);std::cout << "2517" << endl;
		msecs -= 10;std::cout << "2518" << endl;
	}
	if(had_msecs && b_busy) {
		abort();std::cout << "2521" << endl;
		return false;
	}
	return true;
}

bool Calculator::calculateRPN(MathOperation op, int msecs, const EvaluationOptions &eo, MathStructure *parsed_struct) {
	MathStructure *mstruct;std::cout << "2528" << endl;
	if(rpn_stack.size() == 0) {
		mstruct = new MathStructure();std::cout << "2530" << endl;
		mstruct->add(m_zero, op);std::cout << "2531" << endl;
		if(parsed_struct) parsed_struct->clear();
	} else if(rpn_stack.size() == 1) {
		if(parsed_struct) {
			parsed_struct->set(*rpn_stack.back());std::cout << "2535" << endl;
			if(op == OPERATION_SUBTRACT) {
				parsed_struct->transform(STRUCT_NEGATE);
			} else if(op == OPERATION_DIVIDE) {
				parsed_struct->transform(STRUCT_INVERSE);
			} else {
				parsed_struct->add(*rpn_stack.back(), op);std::cout << "2541" << endl;
			}
		}
		if(op == OPERATION_SUBTRACT) {
			mstruct = new MathStructure();std::cout << "2545" << endl;
		} else if(op == OPERATION_DIVIDE) {
			mstruct = new MathStructure(1, 1, 0);std::cout << "2547" << endl;
		} else {
			mstruct = new MathStructure(*rpn_stack.back());std::cout << "2549" << endl;
		}
		mstruct->add(*rpn_stack.back(), op);std::cout << "2551" << endl;
	} else {
		if(parsed_struct) {
			parsed_struct->set(*rpn_stack[rpn_stack.size() - 2]);std::cout << "2554" << endl;
			if(op == OPERATION_SUBTRACT) {
				parsed_struct->transform(STRUCT_ADDITION, *rpn_stack.back());
				(*parsed_struct)[1].transform(STRUCT_NEGATE);
			} else if(op == OPERATION_DIVIDE) {
				parsed_struct->transform(STRUCT_DIVISION, *rpn_stack.back());
			} else {
				parsed_struct->add(*rpn_stack.back(), op);std::cout << "2561" << endl;
			}
		}
		mstruct = new MathStructure(*rpn_stack[rpn_stack.size() - 2]);std::cout << "2564" << endl;
		mstruct->add(*rpn_stack.back(), op);std::cout << "2565" << endl;
	}
	return calculateRPN(mstruct, PROC_RPN_OPERATION_2, 0, msecs, eo);
}
bool Calculator::calculateRPN(MathFunction *f, int msecs, const EvaluationOptions &eo, MathStructure *parsed_struct) {
	MathStructure *mstruct = new MathStructure(f, NULL);std::cout << "2570" << endl;
	int iregs = 0;std::cout << "2571" << endl;
	if(f->args() != 0) {
		size_t i = f->minargs();std::cout << "2573" << endl;
		bool fill_vector = (i > 0 && f->getArgumentDefinition(i) && f->getArgumentDefinition(i)->type() == ARGUMENT_TYPE_VECTOR);std::cout << "2574" << endl;
		if(fill_vector && rpn_stack.size() < i) fill_vector = false;
		if(fill_vector && rpn_stack.size() > 0 && rpn_stack.back()->isVector()) fill_vector = false;
		if(fill_vector) {
			i = rpn_stack.size();std::cout << "2578" << endl;
		} else if(i < 1) {
			i = 1;std::cout << "2580" << endl;
		}
		for(; i > 0; i--) {
			if(i > rpn_stack.size()) {
				error(false, _("Stack is empty. Filling remaining function arguments with zeroes."), NULL);
				mstruct->addChild(m_zero);std::cout << "2585" << endl;
			} else {
				if(fill_vector && rpn_stack.size() - i == (size_t) f->minargs() - 1) mstruct->addChild(m_empty_vector);
				if(fill_vector && rpn_stack.size() - i >= (size_t) f->minargs() - 1) mstruct->getChild(f->minargs())->addChild(*rpn_stack[rpn_stack.size() - i]);
				else mstruct->addChild(*rpn_stack[rpn_stack.size() - i]);std::cout << "2589" << endl;
				iregs++;std::cout << "2590" << endl;
			}
			if(!fill_vector && f->getArgumentDefinition(i) && f->getArgumentDefinition(i)->type() == ARGUMENT_TYPE_ANGLE) {
				switch(eo.parse_options.angle_unit) {
					case ANGLE_UNIT_DEGREES: {
						(*mstruct)[i - 1].multiply(getDegUnit());std::cout << "2595" << endl;
						break;
					}
					case ANGLE_UNIT_GRADIANS: {
						(*mstruct)[i - 1].multiply(getGraUnit());std::cout << "2599" << endl;
						break;
					}
					case ANGLE_UNIT_RADIANS: {
						(*mstruct)[i - 1].multiply(getRadUnit());std::cout << "2603" << endl;
						break;
					}
					default: {}
				}
			}
		}
		if(fill_vector) mstruct->childrenUpdated();
		f->appendDefaultValues(*mstruct);std::cout << "2611" << endl;
	}
	if(parsed_struct) parsed_struct->set(*mstruct);
	return calculateRPN(mstruct, PROC_RPN_OPERATION_F, 0, msecs, eo, iregs);
}
bool Calculator::calculateRPNBitwiseNot(int msecs, const EvaluationOptions &eo, MathStructure *parsed_struct) {
	MathStructure *mstruct;std::cout << "2617" << endl;
	if(rpn_stack.size() == 0) {
		mstruct = new MathStructure();std::cout << "2619" << endl;
		mstruct->setBitwiseNot();std::cout << "2620" << endl;
	} else {
		mstruct = new MathStructure(*rpn_stack.back());std::cout << "2622" << endl;
		mstruct->setBitwiseNot();std::cout << "2623" << endl;
	}
	if(parsed_struct) parsed_struct->set(*mstruct);
	return calculateRPN(mstruct, PROC_RPN_OPERATION_1, 0, msecs, eo);
}
bool Calculator::calculateRPNLogicalNot(int msecs, const EvaluationOptions &eo, MathStructure *parsed_struct) {
	MathStructure *mstruct;std::cout << "2629" << endl;
	if(rpn_stack.size() == 0) {
		mstruct = new MathStructure();std::cout << "2631" << endl;
		mstruct->setLogicalNot();std::cout << "2632" << endl;
	} else {
		mstruct = new MathStructure(*rpn_stack.back());std::cout << "2634" << endl;
		mstruct->setLogicalNot();std::cout << "2635" << endl;
	}
	if(parsed_struct) parsed_struct->set(*rpn_stack.back());
	return calculateRPN(mstruct, PROC_RPN_OPERATION_1, 0, msecs, eo);
}
MathStructure *Calculator::calculateRPN(MathOperation op, const EvaluationOptions &eo, MathStructure *parsed_struct) {
	current_stage = MESSAGE_STAGE_PARSING;std::cout << "2641" << endl;
	MathStructure *mstruct;std::cout << "2642" << endl;
	if(rpn_stack.size() == 0) {
		mstruct = new MathStructure();std::cout << "2644" << endl;
		mstruct->add(m_zero, op);std::cout << "2645" << endl;
		if(parsed_struct) parsed_struct->clear();
	} else if(rpn_stack.size() == 1) {
		if(parsed_struct) {
			parsed_struct->clear();std::cout << "2649" << endl;
			if(op == OPERATION_SUBTRACT) {
				parsed_struct->transform(STRUCT_ADDITION, *rpn_stack.back());
				(*parsed_struct)[1].transform(STRUCT_NEGATE);
			} else if(op == OPERATION_DIVIDE) {
				parsed_struct->transform(STRUCT_DIVISION, *rpn_stack.back());
			} else {
				parsed_struct->add(*rpn_stack.back(), op);std::cout << "2656" << endl;
			}
		}
		mstruct = new MathStructure();std::cout << "2659" << endl;
		mstruct->add(*rpn_stack.back(), op);std::cout << "2660" << endl;
	} else {
		if(parsed_struct) {
			parsed_struct->set(*rpn_stack[rpn_stack.size() - 2]);std::cout << "2663" << endl;
			if(op == OPERATION_SUBTRACT) {
				parsed_struct->transform(STRUCT_ADDITION, *rpn_stack.back());
				(*parsed_struct)[1].transform(STRUCT_NEGATE);
			} else if(op == OPERATION_DIVIDE) {
				parsed_struct->transform(STRUCT_DIVISION, *rpn_stack.back());
			} else {
				parsed_struct->add(*rpn_stack.back(), op);std::cout << "2670" << endl;
			}
		}
		mstruct = new MathStructure(*rpn_stack[rpn_stack.size() - 2]);std::cout << "2673" << endl;
		mstruct->add(*rpn_stack.back(), op);std::cout << "2674" << endl;
	}
	current_stage = MESSAGE_STAGE_CALCULATION;std::cout << "2676" << endl;
	mstruct->eval(eo);std::cout << "2677" << endl;
	current_stage = MESSAGE_STAGE_CONVERSION;std::cout << "2678" << endl;
	autoConvert(*mstruct, *mstruct, eo);std::cout << "2679" << endl;
	current_stage = MESSAGE_STAGE_UNSET;std::cout << "2680" << endl;
	if(rpn_stack.size() > 1) {
		rpn_stack.back()->unref();std::cout << "2682" << endl;
		rpn_stack.erase(rpn_stack.begin() + (rpn_stack.size() - 1));std::cout << "2683" << endl;
	}
	if(rpn_stack.size() > 0) {
		rpn_stack.back()->unref();std::cout << "2686" << endl;
		rpn_stack.back() = mstruct;std::cout << "2687" << endl;
	} else {
		rpn_stack.push_back(mstruct);std::cout << "2689" << endl;
	}
	return rpn_stack.back();
}
MathStructure *Calculator::calculateRPN(MathFunction *f, const EvaluationOptions &eo, MathStructure *parsed_struct) {
	current_stage = MESSAGE_STAGE_PARSING;std::cout << "2694" << endl;
	MathStructure *mstruct = new MathStructure(f, NULL);std::cout << "2695" << endl;
	size_t iregs = 0;std::cout << "2696" << endl;
	if(f->args() != 0) {
		size_t i = f->minargs();std::cout << "2698" << endl;
		bool fill_vector = (i > 0 && f->getArgumentDefinition(i) && f->getArgumentDefinition(i)->type() == ARGUMENT_TYPE_VECTOR);std::cout << "2699" << endl;
		if(fill_vector && rpn_stack.size() < i) fill_vector = false;
		if(fill_vector && rpn_stack.size() > 0 && rpn_stack.back()->isVector()) fill_vector = false;
		if(fill_vector) {
			i = rpn_stack.size();std::cout << "2703" << endl;
		} else if(i < 1) {
			i = 1;std::cout << "2705" << endl;
		}
		for(; i > 0; i--) {
			if(i > rpn_stack.size()) {
				error(false, _("Stack is empty. Filling remaining function arguments with zeroes."), NULL);
				mstruct->addChild(m_zero);std::cout << "2710" << endl;
			} else {
				if(fill_vector && rpn_stack.size() - i == (size_t) f->minargs() - 1) mstruct->addChild(m_empty_vector);
				if(fill_vector && rpn_stack.size() - i >= (size_t) f->minargs() - 1) mstruct->getChild(f->minargs())->addChild(*rpn_stack[rpn_stack.size() - i]);
				else mstruct->addChild(*rpn_stack[rpn_stack.size() - i]);std::cout << "2714" << endl;
				iregs++;std::cout << "2715" << endl;
			}
			if(!fill_vector && f->getArgumentDefinition(i) && f->getArgumentDefinition(i)->type() == ARGUMENT_TYPE_ANGLE) {
				switch(eo.parse_options.angle_unit) {
					case ANGLE_UNIT_DEGREES: {
						(*mstruct)[i - 1].multiply(getDegUnit());std::cout << "2720" << endl;
						break;
					}
					case ANGLE_UNIT_GRADIANS: {
						(*mstruct)[i - 1].multiply(getGraUnit());std::cout << "2724" << endl;
						break;
					}
					case ANGLE_UNIT_RADIANS: {
						(*mstruct)[i - 1].multiply(getRadUnit());std::cout << "2728" << endl;
						break;
					}
					default: {}
				}
			}
		}
		if(fill_vector) mstruct->childrenUpdated();
		f->appendDefaultValues(*mstruct);std::cout << "2736" << endl;
	}
	if(parsed_struct) parsed_struct->set(*mstruct);
	current_stage = MESSAGE_STAGE_CALCULATION;std::cout << "2739" << endl;
	mstruct->eval(eo);std::cout << "2740" << endl;
	current_stage = MESSAGE_STAGE_CONVERSION;std::cout << "2741" << endl;
	autoConvert(*mstruct, *mstruct, eo);std::cout << "2742" << endl;
	current_stage = MESSAGE_STAGE_UNSET;std::cout << "2743" << endl;
	if(iregs == 0) {
		rpn_stack.push_back(mstruct);std::cout << "2745" << endl;
	} else {
		for(size_t i = 0; i < iregs - 1 && rpn_stack.size() > 1; i++) {
			rpn_stack.back()->unref();std::cout << "2748" << endl;
			rpn_stack.pop_back();std::cout << "2749" << endl;
			deleteRPNRegister(1);std::cout << "2750" << endl;
		}
		rpn_stack.back()->unref();std::cout << "2752" << endl;
		rpn_stack.back() = mstruct;std::cout << "2753" << endl;
	}
	return rpn_stack.back();
}
MathStructure *Calculator::calculateRPNBitwiseNot(const EvaluationOptions &eo, MathStructure *parsed_struct) {
	current_stage = MESSAGE_STAGE_PARSING;std::cout << "2758" << endl;
	MathStructure *mstruct;std::cout << "2759" << endl;
	if(rpn_stack.size() == 0) {
		mstruct = new MathStructure();std::cout << "2761" << endl;
		mstruct->setBitwiseNot();std::cout << "2762" << endl;
	} else {
		mstruct = new MathStructure(*rpn_stack.back());std::cout << "2764" << endl;
		mstruct->setBitwiseNot();std::cout << "2765" << endl;
	}
	if(parsed_struct) parsed_struct->set(*mstruct);
	current_stage = MESSAGE_STAGE_CALCULATION;std::cout << "2768" << endl;
	mstruct->eval(eo);std::cout << "2769" << endl;
	current_stage = MESSAGE_STAGE_CONVERSION;std::cout << "2770" << endl;
	autoConvert(*mstruct, *mstruct, eo);std::cout << "2771" << endl;
	current_stage = MESSAGE_STAGE_UNSET;std::cout << "2772" << endl;
	if(rpn_stack.size() == 0) {
		rpn_stack.push_back(mstruct);std::cout << "2774" << endl;
	} else {
		rpn_stack.back()->unref();std::cout << "2776" << endl;
		rpn_stack.back() = mstruct;std::cout << "2777" << endl;
	}
	return rpn_stack.back();
}
MathStructure *Calculator::calculateRPNLogicalNot(const EvaluationOptions &eo, MathStructure *parsed_struct) {
	current_stage = MESSAGE_STAGE_PARSING;std::cout << "2782" << endl;
	MathStructure *mstruct;std::cout << "2783" << endl;
	if(rpn_stack.size() == 0) {
		mstruct = new MathStructure();std::cout << "2785" << endl;
		mstruct->setLogicalNot();std::cout << "2786" << endl;
	} else {
		mstruct = new MathStructure(*rpn_stack.back());std::cout << "2788" << endl;
		mstruct->setLogicalNot();std::cout << "2789" << endl;
	}
	if(parsed_struct) parsed_struct->set(*mstruct);
	current_stage = MESSAGE_STAGE_CALCULATION;std::cout << "2792" << endl;
	mstruct->eval(eo);std::cout << "2793" << endl;
	current_stage = MESSAGE_STAGE_CONVERSION;std::cout << "2794" << endl;
	autoConvert(*mstruct, *mstruct, eo);std::cout << "2795" << endl;
	current_stage = MESSAGE_STAGE_UNSET;std::cout << "2796" << endl;
	if(rpn_stack.size() == 0) {
		rpn_stack.push_back(mstruct);std::cout << "2798" << endl;
	} else {
		rpn_stack.back()->unref();std::cout << "2800" << endl;
		rpn_stack.back() = mstruct;std::cout << "2801" << endl;
	}
	return rpn_stack.back();
}
bool Calculator::RPNStackEnter(MathStructure *mstruct, int msecs, const EvaluationOptions &eo) {
	return calculateRPN(mstruct, PROC_RPN_ADD, 0, msecs, eo);
}
bool Calculator::RPNStackEnter(string str, int msecs, const EvaluationOptions &eo, MathStructure *parsed_struct, MathStructure *to_struct, bool make_to_division) {
	remove_blank_ends(str);std::cout << "2809" << endl;
	if(str.empty() && rpn_stack.size() > 0) {
		rpn_stack.push_back(new MathStructure(*rpn_stack.back()));std::cout << "2811" << endl;
		return true;
	}
	return calculateRPN(str, PROC_RPN_ADD, 0, msecs, eo, parsed_struct, to_struct, make_to_division);
}
void Calculator::RPNStackEnter(MathStructure *mstruct, bool eval, const EvaluationOptions &eo) {
	if(eval) {
		current_stage = MESSAGE_STAGE_CALCULATION;std::cout << "2818" << endl;
		mstruct->eval(eo);std::cout << "2819" << endl;
		current_stage = MESSAGE_STAGE_CONVERSION;std::cout << "2820" << endl;
		autoConvert(*mstruct, *mstruct, eo);std::cout << "2821" << endl;
		current_stage = MESSAGE_STAGE_UNSET;std::cout << "2822" << endl;
	}
	rpn_stack.push_back(mstruct);std::cout << "2824" << endl;
}
void Calculator::RPNStackEnter(string str, const EvaluationOptions &eo, MathStructure *parsed_struct, MathStructure *to_struct, bool make_to_division) {
	remove_blank_ends(str);std::cout << "2827" << endl;
	if(str.empty() && rpn_stack.size() > 0) rpn_stack.push_back(new MathStructure(*rpn_stack.back()));
	else rpn_stack.push_back(new MathStructure(calculate(str, eo, parsed_struct, to_struct, make_to_division)));std::cout << "2829" << endl;
}
bool Calculator::setRPNRegister(size_t index, MathStructure *mstruct, int msecs, const EvaluationOptions &eo) {
	if(mstruct == NULL) {
		deleteRPNRegister(index);std::cout << "2833" << endl;
		return true;
	}
	if(index <= 0 || index > rpn_stack.size()) return false;
	return calculateRPN(mstruct, PROC_RPN_SET, index, msecs, eo);
}
bool Calculator::setRPNRegister(size_t index, string str, int msecs, const EvaluationOptions &eo, MathStructure *parsed_struct, MathStructure *to_struct, bool make_to_division) {
	if(index <= 0 || index > rpn_stack.size()) return false;
	return calculateRPN(str, PROC_RPN_SET, index, msecs, eo, parsed_struct, to_struct, make_to_division);
}
void Calculator::setRPNRegister(size_t index, MathStructure *mstruct, bool eval, const EvaluationOptions &eo) {
	if(mstruct == NULL) {
		deleteRPNRegister(index);std::cout << "2845" << endl;
		return;
	}
	if(eval) {
		current_stage = MESSAGE_STAGE_CALCULATION;std::cout << "2849" << endl;
		mstruct->eval(eo);std::cout << "2850" << endl;
		current_stage = MESSAGE_STAGE_CONVERSION;std::cout << "2851" << endl;
		autoConvert(*mstruct, *mstruct, eo);std::cout << "2852" << endl;
		current_stage = MESSAGE_STAGE_UNSET;std::cout << "2853" << endl;
	}
	if(index <= 0 || index > rpn_stack.size()) return;
	index = rpn_stack.size() - index;std::cout << "2856" << endl;
	rpn_stack[index]->unref();std::cout << "2857" << endl;
	rpn_stack[index] = mstruct;std::cout << "2858" << endl;
}
void Calculator::setRPNRegister(size_t index, string str, const EvaluationOptions &eo, MathStructure *parsed_struct, MathStructure *to_struct, bool make_to_division) {
	if(index <= 0 || index > rpn_stack.size()) return;
	index = rpn_stack.size() - index;std::cout << "2862" << endl;
	MathStructure *mstruct = new MathStructure(calculate(str, eo, parsed_struct, to_struct, make_to_division));std::cout << "2863" << endl;
	rpn_stack[index]->unref();std::cout << "2864" << endl;
	rpn_stack[index] = mstruct;std::cout << "2865" << endl;
}
void Calculator::deleteRPNRegister(size_t index) {
	if(index <= 0 || index > rpn_stack.size()) return;
	index = rpn_stack.size() - index;std::cout << "2869" << endl;
	rpn_stack[index]->unref();std::cout << "2870" << endl;
	rpn_stack.erase(rpn_stack.begin() + index);std::cout << "2871" << endl;
}
MathStructure *Calculator::getRPNRegister(size_t index) const {
	if(index > 0 && index <= rpn_stack.size()) {
		index = rpn_stack.size() - index;std::cout << "2875" << endl;
		return rpn_stack[index];
	}
	return NULL;
}
size_t Calculator::RPNStackSize() const {
	return rpn_stack.size();
}
void Calculator::clearRPNStack() {
	for(size_t i = 0; i < rpn_stack.size(); i++) {
		rpn_stack[i]->unref();std::cout << "2885" << endl;
	}
	rpn_stack.clear();std::cout << "2887" << endl;
}
void Calculator::moveRPNRegister(size_t old_index, size_t new_index) {
	if(old_index == new_index) return;
	if(old_index > 0 && old_index <= rpn_stack.size()) {
		old_index = rpn_stack.size() - old_index;std::cout << "2892" << endl;
		MathStructure *mstruct = rpn_stack[old_index];std::cout << "2893" << endl;
		if(new_index > rpn_stack.size()) {
			new_index = 0;std::cout << "2895" << endl;
		} else if(new_index <= 1) {
			rpn_stack.push_back(mstruct);std::cout << "2897" << endl;
			rpn_stack.erase(rpn_stack.begin() + old_index);std::cout << "2898" << endl;
			return;
		} else {
			new_index = rpn_stack.size() - new_index;std::cout << "2901" << endl;
		}
		if(new_index > old_index) {
			rpn_stack.erase(rpn_stack.begin() + old_index);std::cout << "2904" << endl;
			rpn_stack.insert(rpn_stack.begin() + new_index, mstruct);std::cout << "2905" << endl;
		} else if(new_index < old_index) {
			rpn_stack.insert(rpn_stack.begin() + new_index, mstruct);std::cout << "2907" << endl;
			rpn_stack.erase(rpn_stack.begin() + (old_index + 1));std::cout << "2908" << endl;
		}
	}
}
void Calculator::moveRPNRegisterUp(size_t index) {
	if(index > 1 && index <= rpn_stack.size()) {
		index = rpn_stack.size() - index;std::cout << "2914" << endl;
		MathStructure *mstruct = rpn_stack[index];std::cout << "2915" << endl;
		rpn_stack.erase(rpn_stack.begin() + index);std::cout << "2916" << endl;
		index++;std::cout << "2917" << endl;
		if(index == rpn_stack.size()) rpn_stack.push_back(mstruct);
		else rpn_stack.insert(rpn_stack.begin() + index, mstruct);std::cout << "2919" << endl;
	}
}
void Calculator::moveRPNRegisterDown(size_t index) {
	if(index > 0 && index < rpn_stack.size()) {
		index = rpn_stack.size() - index;std::cout << "2924" << endl;
		MathStructure *mstruct = rpn_stack[index];std::cout << "2925" << endl;
		rpn_stack.erase(rpn_stack.begin() + index);std::cout << "2926" << endl;
		index--;std::cout << "2927" << endl;
		rpn_stack.insert(rpn_stack.begin() + index, mstruct);std::cout << "2928" << endl;
	}
}

int has_information_unit(const MathStructure &m, bool top = true) {
	if(m.isUnit_exp()) {
		if(m.isUnit()) {
			if(m.unit()->baseUnit()->referenceName() == "bit") return 1;
		} else {
			if(m[0].unit()->baseUnit()->referenceName() == "bit") {
				if(m[1].isInteger() && m[1].number().isPositive()) return 1;
				return 2;
			}
		}
		return 0;
	}
	for(size_t i = 0; i < m.size(); i++) {
		int ret = has_information_unit(m[i], false);
		if(ret > 0) {
			if(ret == 1 && top && m.isMultiplication() && m[0].isNumber() && m[0].number().isFraction()) return 2;
			return ret;
		}
	}
	return 0;
}

#define EQUALS_IGNORECASE_AND_LOCAL(x,y,z)	(equalsIgnoreCase(x, y) || equalsIgnoreCase(x, z))
string Calculator::calculateAndPrint(string str, int msecs, const EvaluationOptions &eo, const PrintOptions &po) {
	if(msecs > 0) startControl(msecs);
	PrintOptions printops = po;std::cout << "2956" << endl;
	EvaluationOptions evalops = eo;std::cout << "2957" << endl;
	MathStructure mstruct;std::cout << "2958" << endl;
	bool do_bases = false, do_factors = false, do_fraction = false, do_pfe = false, do_calendars = false, do_expand = false, do_binary_prefixes = false, complex_angle_form = false;
	string from_str = str, to_str;std::cout << "2960" << endl;
	Number base_save;std::cout << "2961" << endl;
	if(printops.base == BASE_CUSTOM) base_save = customOutputBase();
	int save_bin = priv->use_binary_prefixes;std::cout << "2963" << endl;
	if(separateToExpression(from_str, to_str, evalops, true)) {
		remove_duplicate_blanks(to_str);std::cout << "2965" << endl;
		string to_str1, to_str2;std::cout << "2966" << endl;
		size_t ispace = to_str.find_first_of(SPACES);std::cout << "2967" << endl;
		if(ispace != string::npos) {
			to_str1 = to_str.substr(0, ispace);std::cout << "2969" << endl;
			remove_blank_ends(to_str1);std::cout << "2970" << endl;
			to_str2 = to_str.substr(ispace + 1);std::cout << "2971" << endl;
			remove_blank_ends(to_str2);std::cout << "2972" << endl;
		}
		if(equalsIgnoreCase(to_str, "hex") || EQUALS_IGNORECASE_AND_LOCAL(to_str, "hexadecimal", _("hexadecimal"))) {
			str = from_str;std::cout << "2975" << endl;
			printops.base = BASE_HEXADECIMAL;std::cout << "2976" << endl;
		} else if(equalsIgnoreCase(to_str, "bin") || EQUALS_IGNORECASE_AND_LOCAL(to_str, "binary", _("binary"))) {
			str = from_str;std::cout << "2978" << endl;
			printops.base = BASE_BINARY;std::cout << "2979" << endl;
		} else if(equalsIgnoreCase(to_str, "dec") || EQUALS_IGNORECASE_AND_LOCAL(to_str, "decimal", _("decimal"))) {
			str = from_str;std::cout << "2981" << endl;
			printops.base = BASE_DECIMAL;std::cout << "2982" << endl;
		} else if(equalsIgnoreCase(to_str, "oct") || EQUALS_IGNORECASE_AND_LOCAL(to_str, "octal", _("octal"))) {
			str = from_str;std::cout << "2984" << endl;
			printops.base = BASE_OCTAL;std::cout << "2985" << endl;
		} else if(equalsIgnoreCase(to_str, "duo") || EQUALS_IGNORECASE_AND_LOCAL(to_str, "duodecimal", _("duodecimal"))) {
			str = from_str;std::cout << "2987" << endl;
			printops.base = BASE_DUODECIMAL;std::cout << "2988" << endl;
		} else if(equalsIgnoreCase(to_str, "roman") || equalsIgnoreCase(to_str, _("roman"))) {
			str = from_str;std::cout << "2990" << endl;
			printops.base = BASE_ROMAN_NUMERALS;std::cout << "2991" << endl;
		} else if(equalsIgnoreCase(to_str, "bijective") || equalsIgnoreCase(to_str, _("bijective"))) {
			str = from_str;std::cout << "2993" << endl;
			printops.base = BASE_BIJECTIVE_26;std::cout << "2994" << endl;
		} else if(equalsIgnoreCase(to_str, "sexa") || equalsIgnoreCase(to_str, "sexagesimal") || equalsIgnoreCase(to_str, _("sexagesimal"))) {
			str = from_str;std::cout << "2996" << endl;
			printops.base = BASE_SEXAGESIMAL;std::cout << "2997" << endl;
		} else if(equalsIgnoreCase(to_str, "time") || equalsIgnoreCase(to_str, _("time"))) {
			str = from_str;std::cout << "2999" << endl;
			printops.base = BASE_TIME;std::cout << "3000" << endl;
		} else if(equalsIgnoreCase(to_str, "unicode")) {
			str = from_str;std::cout << "3002" << endl;
			printops.base = BASE_UNICODE;std::cout << "3003" << endl;
		} else if(equalsIgnoreCase(to_str, "utc") || equalsIgnoreCase(to_str, "gmt")) {
			str = from_str;std::cout << "3005" << endl;
			printops.time_zone = TIME_ZONE_UTC;std::cout << "3006" << endl;
		} else if(to_str.length() > 3 && (equalsIgnoreCase(to_str.substr(0, 3), "utc") || equalsIgnoreCase(to_str.substr(0, 3), "gmt"))) {
			to_str = to_str.substr(3);std::cout << "3008" << endl;
			remove_blanks(to_str);std::cout << "3009" << endl;
			bool b_minus = false;std::cout << "3010" << endl;
			if(to_str[0] == '+') {
				to_str.erase(0, 1);std::cout << "3012" << endl;
			} else if(to_str[0] == '-') {
				b_minus = true;std::cout << "3014" << endl;
				to_str.erase(0, 1);std::cout << "3015" << endl;
			} else if(to_str.find(SIGN_MINUS) == 0) {
				b_minus = true;std::cout << "3017" << endl;
				to_str.erase(0, strlen(SIGN_MINUS));std::cout << "3018" << endl;
			}
			unsigned int tzh = 0, tzm = 0;std::cout << "3020" << endl;
			int itz = 0;std::cout << "3021" << endl;
			if(!to_str.empty() && sscanf(to_str.c_str(), "%2u:%2u", &tzh, &tzm) > 0) {
				itz = tzh * 60 + tzm;std::cout << "3023" << endl;
				if(b_minus) itz = -itz;
			} else {
				error(true, _("Time zone parsing failed."), NULL);
			}
			printops.time_zone = TIME_ZONE_CUSTOM;std::cout << "3028" << endl;
			printops.custom_time_zone = itz;std::cout << "3029" << endl;
			str = from_str;std::cout << "3030" << endl;
		} else if(to_str == "CET") {
			printops.time_zone = TIME_ZONE_CUSTOM;std::cout << "3032" << endl;
			printops.custom_time_zone = 60;std::cout << "3033" << endl;
			str = from_str;std::cout << "3034" << endl;
		} else if(EQUALS_IGNORECASE_AND_LOCAL(to_str, "rectangular", _("rectangular")) || EQUALS_IGNORECASE_AND_LOCAL(to_str, "cartesian", _("cartesian")) || str == "rect") {
			str = from_str;std::cout << "3036" << endl;
			evalops.complex_number_form = COMPLEX_NUMBER_FORM_RECTANGULAR;
		} else if(EQUALS_IGNORECASE_AND_LOCAL(to_str, "exponential", _("exponential")) || to_str == "exp") {
			str = from_str;std::cout << "3039" << endl;
			evalops.complex_number_form = COMPLEX_NUMBER_FORM_EXPONENTIAL;
		} else if(EQUALS_IGNORECASE_AND_LOCAL(to_str, "polar", _("polar"))) {
			str = from_str;std::cout << "3042" << endl;
			evalops.complex_number_form = COMPLEX_NUMBER_FORM_POLAR;
		} else if(EQUALS_IGNORECASE_AND_LOCAL(to_str, "angle", _("angle")) || EQUALS_IGNORECASE_AND_LOCAL(to_str, "phasor", _("phasor"))) {
			str = from_str;std::cout << "3045" << endl;
			evalops.complex_number_form = COMPLEX_NUMBER_FORM_CIS;
			complex_angle_form = true;
		} else if(to_str == "cis") {
			str = from_str;std::cout << "3049" << endl;
			evalops.complex_number_form = COMPLEX_NUMBER_FORM_CIS;
		} else if(EQUALS_IGNORECASE_AND_LOCAL(to_str, "fraction", _("fraction"))) {
			str = from_str;std::cout << "3052" << endl;
			do_fraction = true;std::cout << "3053" << endl;
		} else if(EQUALS_IGNORECASE_AND_LOCAL(to_str, "factors", _("factors")) || equalsIgnoreCase(to_str, "factor")) {
			str = from_str;std::cout << "3055" << endl;
			evalops.structuring = STRUCTURING_FACTORIZE;std::cout << "3056" << endl;
			do_factors = true;std::cout << "3057" << endl;
		}  else if(equalsIgnoreCase(to_str, "partial fraction") || equalsIgnoreCase(to_str, _("partial fraction"))) {
			str = from_str;std::cout << "3059" << endl;
			do_pfe = true;std::cout << "3060" << endl;
		} else if(EQUALS_IGNORECASE_AND_LOCAL(to_str, "bases", _("bases"))) {
			do_bases = true;std::cout << "3062" << endl;
			str = from_str;std::cout << "3063" << endl;
		} else if(EQUALS_IGNORECASE_AND_LOCAL(to_str, "calendars", _("calendars"))) {
			do_calendars = true;std::cout << "3065" << endl;
			str = from_str;std::cout << "3066" << endl;
		} else if(EQUALS_IGNORECASE_AND_LOCAL(to_str, "optimal", _("optimal"))) {
			str = from_str;std::cout << "3068" << endl;
			evalops.parse_options.units_enabled = true;std::cout << "3069" << endl;
			evalops.auto_post_conversion = POST_CONVERSION_OPTIMAL_SI;std::cout << "3070" << endl;
		} else if(EQUALS_IGNORECASE_AND_LOCAL(to_str, "base", _("base"))) {
			str = from_str;std::cout << "3072" << endl;
			evalops.parse_options.units_enabled = true;std::cout << "3073" << endl;
			evalops.auto_post_conversion = POST_CONVERSION_BASE;std::cout << "3074" << endl;
		} else if(EQUALS_IGNORECASE_AND_LOCAL(to_str1, "base", _("base"))) {
			str = from_str;std::cout << "3076" << endl;
			if(to_str2 == "b26" || to_str2 == "B26") printops.base = BASE_BIJECTIVE_26;
			else if(equalsIgnoreCase(to_str2, "golden") || equalsIgnoreCase(to_str2, "golden ratio") || to_str2 == "φ") printops.base = BASE_GOLDEN_RATIO;
			else if(equalsIgnoreCase(to_str2, "unicode")) printops.base = BASE_UNICODE;
			else if(equalsIgnoreCase(to_str2, "supergolden") || equalsIgnoreCase(to_str2, "supergolden ratio") || to_str2 == "ψ") printops.base = BASE_SUPER_GOLDEN_RATIO;
			else if(equalsIgnoreCase(to_str2, "pi") || to_str2 == "π") printops.base = BASE_PI;
			else if(to_str2 == "e") printops.base = BASE_E;
			else if(to_str2 == "sqrt(2)" || to_str2 == "sqrt 2" || to_str2 == "sqrt2" || to_str2 == "√2") printops.base = BASE_SQRT2;
			else {
				EvaluationOptions eo = evalops;std::cout << "3085" << endl;
				eo.parse_options.base = 10;std::cout << "3086" << endl;
				MathStructure m = calculate(to_str2, eo);std::cout << "3087" << endl;
				if(m.isInteger() && m.number() >= 2 && m.number() <= 36) {
					printops.base = m.number().intValue();std::cout << "3089" << endl;
				} else {
					printops.base = BASE_CUSTOM;std::cout << "3091" << endl;
					base_save = customOutputBase();std::cout << "3092" << endl;
					setCustomOutputBase(m.number());std::cout << "3093" << endl;
				}
			}
		} else if(EQUALS_IGNORECASE_AND_LOCAL(to_str, "mixed", _("mixed"))) {
			str = from_str;std::cout << "3097" << endl;
			evalops.parse_options.units_enabled = true;std::cout << "3098" << endl;
			evalops.auto_post_conversion = POST_CONVERSION_NONE;std::cout << "3099" << endl;
			evalops.mixed_units_conversion = MIXED_UNITS_CONVERSION_FORCE_INTEGER;std::cout << "3100" << endl;
		} else {
			evalops.parse_options.units_enabled = true;std::cout << "3102" << endl;
			if(to_str[0] == '?' || (to_str.length() > 1 && to_str[1] == '?' && (to_str[0] == 'a' || to_str[0] == 'd'))) {
				printops.use_unit_prefixes = true;std::cout << "3104" << endl;
				printops.use_prefixes_for_currencies = true;
				printops.use_prefixes_for_all_units = true;
				if(to_str[0] == 'a') printops.use_all_prefixes = true;
				else if(to_str[0] == 'd') priv->use_binary_prefixes = 0;
			} else if(to_str.length() > 1 && to_str[1] == '?' && to_str[0] == 'b') {
				do_binary_prefixes = true;std::cout << "3110" << endl;
			}
		}
	} else {
		size_t i = str.find_first_of(SPACES LEFT_PARENTHESIS);std::cout << "3114" << endl;
		if(i != string::npos) {
			to_str = str.substr(0, i);std::cout << "3116" << endl;
			if(to_str == "factor" || EQUALS_IGNORECASE_AND_LOCAL(to_str, "factorize", _("factorize"))) {
				str = str.substr(i + 1);std::cout << "3118" << endl;
				do_factors = true;std::cout << "3119" << endl;
				evalops.structuring = STRUCTURING_FACTORIZE;std::cout << "3120" << endl;
			} else if(EQUALS_IGNORECASE_AND_LOCAL(to_str, "expand", _("expand"))) {
				str = str.substr(i + 1);std::cout << "3122" << endl;
				evalops.structuring = STRUCTURING_SIMPLIFY;std::cout << "3123" << endl;
				do_expand = true;std::cout << "3124" << endl;
			}
		}
	}

	mstruct = calculate(str, evalops);std::cout << "3129" << endl;

	if(do_factors) {
		mstruct.integerFactorize();std::cout << "3132" << endl;
	} else if(do_expand) {
		mstruct.expand(evalops, false);std::cout << "3134" << endl;
	}
	if(do_pfe) mstruct.expandPartialFractions(evalops);

	printops.allow_factorization = printops.allow_factorization || evalops.structuring == STRUCTURING_FACTORIZE || do_factors;std::cout << "3138" << endl;

	if(do_calendars && mstruct.isDateTime()) {
		str = "";
		bool b_fail;std::cout << "3142" << endl;
		long int y, m, d;std::cout << "3143" << endl;
#define PRINT_CALENDAR(x, c) if(!str.empty()) {str += "\n";} str += x; str += " ";b_fail = !dateToCalendar(*mstruct.datetime(), y, m, d, c); if(b_fail) {str += _("failed");} else {str += i2s(d); str += " "; str += monthName(m, c, true); str += " "; str += i2s(y);}
		PRINT_CALENDAR(string(_("Gregorian:")), CALENDAR_GREGORIAN);
		PRINT_CALENDAR(string(_("Hebrew:")), CALENDAR_HEBREW);
		PRINT_CALENDAR(string(_("Islamic:")), CALENDAR_ISLAMIC);
		PRINT_CALENDAR(string(_("Persian:")), CALENDAR_PERSIAN);
		PRINT_CALENDAR(string(_("Indian national:")), CALENDAR_INDIAN);
		PRINT_CALENDAR(string(_("Chinese:")), CALENDAR_CHINESE);
		long int cy, yc, st, br;std::cout << "3151" << endl;
		chineseYearInfo(y, cy, yc, st, br);std::cout << "3152" << endl;
		if(!b_fail) {str += " ("; str += chineseStemName(st); str += string(" "); str += chineseBranchName(br); str += ")";}
		PRINT_CALENDAR(string(_("Julian:")), CALENDAR_JULIAN);
		PRINT_CALENDAR(string(_("Revised julian:")), CALENDAR_MILANKOVIC);
		PRINT_CALENDAR(string(_("Coptic:")), CALENDAR_COPTIC);
		PRINT_CALENDAR(string(_("Ethiopian:")), CALENDAR_ETHIOPIAN);
		stopControl();std::cout << "3158" << endl;
		return str;
	} else if(do_bases) {
		printops.base = BASE_BINARY;std::cout << "3161" << endl;
		str = print(mstruct, 0, printops);std::cout << "3162" << endl;
		str += " = ";
		printops.base = BASE_OCTAL;std::cout << "3164" << endl;
		str += print(mstruct, 0, printops);std::cout << "3165" << endl;
		str += " = ";
		printops.base = BASE_DECIMAL;std::cout << "3167" << endl;
		str += print(mstruct, 0, printops);std::cout << "3168" << endl;
		str += " = ";
		printops.base = BASE_HEXADECIMAL;std::cout << "3170" << endl;
		str += print(mstruct, 0, printops);std::cout << "3171" << endl;
		stopControl();std::cout << "3172" << endl;
		return str;
	} else if(do_fraction) {
		if(mstruct.isNumber()) printops.number_fraction_format = FRACTION_COMBINED;
		else printops.number_fraction_format = FRACTION_FRACTIONAL;
	} else if(do_binary_prefixes) {
		int i = has_information_unit(mstruct);
		printops.use_unit_prefixes = true;std::cout << "3179" << endl;
		priv->use_binary_prefixes = (i > 0 ? 1 : 2);std::cout << "3180" << endl;
		if(i == 1) {
			printops.use_denominator_prefix = false;std::cout << "3182" << endl;
		} else if(i > 1) {
			printops.use_denominator_prefix = true;std::cout << "3184" << endl;
		} else {
			printops.use_prefixes_for_currencies = true;
			printops.use_prefixes_for_all_units = true;
		}
	}
	mstruct.removeDefaultAngleUnit(evalops);std::cout << "3190" << endl;
	mstruct.format(printops);
	str = mstruct.print(printops);std::cout << "3192" << endl;
	if(complex_angle_form) gsub(" cis ", "∠", str);
	stopControl();std::cout << "3194" << endl;
	if(printops.base == BASE_CUSTOM) setCustomOutputBase(base_save);
	priv->use_binary_prefixes = save_bin;std::cout << "3196" << endl;
	return str;
}
bool Calculator::calculate(MathStructure *mstruct, string str, int msecs, const EvaluationOptions &eo, MathStructure *parsed_struct, MathStructure *to_struct, bool make_to_division) {
	mstruct->set(string(_("calculating...")), false, true);
	b_busy = true;std::cout << "3201" << endl;
	if(!calculate_thread->running && !calculate_thread->start()) {mstruct->setAborted(); return false;}
	bool had_msecs = msecs > 0;std::cout << "3203" << endl;
	expression_to_calculate = str;std::cout << "3204" << endl;
	tmp_evaluationoptions = eo;std::cout << "3205" << endl;
	tmp_proc_command = PROC_NO_COMMAND;std::cout << "3206" << endl;
	tmp_rpn_mstruct = NULL;std::cout << "3207" << endl;
	tmp_parsedstruct = parsed_struct;std::cout << "3208" << endl;
	tmp_tostruct = to_struct;std::cout << "3209" << endl;
	tmp_maketodivision = make_to_division;std::cout << "3210" << endl;
	if(!calculate_thread->write(true)) {calculate_thread->cancel(); mstruct->setAborted(); return false;}
	if(!calculate_thread->write((void*) mstruct)) {calculate_thread->cancel(); mstruct->setAborted(); return false;}
	while(msecs > 0 && b_busy) {
		sleep_ms(10);std::cout << "3214" << endl;
		msecs -= 10;std::cout << "3215" << endl;
	}
	if(had_msecs && b_busy) {
		if(!abort()) mstruct->setAborted();
		return false;
	}
	return true;
}
bool Calculator::calculate(MathStructure *mstruct, int msecs, const EvaluationOptions &eo, string to_str) {
	b_busy = true;std::cout << "3224" << endl;
	if(!calculate_thread->running && !calculate_thread->start()) {mstruct->setAborted(); return false;}
	bool had_msecs = msecs > 0;std::cout << "3226" << endl;
	expression_to_calculate = "";
	tmp_evaluationoptions = eo;std::cout << "3228" << endl;
	tmp_proc_command = PROC_NO_COMMAND;std::cout << "3229" << endl;
	tmp_rpn_mstruct = NULL;std::cout << "3230" << endl;
	tmp_parsedstruct = NULL;std::cout << "3231" << endl;
	if(!to_str.empty()) tmp_tostruct = new MathStructure(to_str);
	else tmp_tostruct = NULL;std::cout << "3233" << endl;
	tmp_tostruct = NULL;std::cout << "3234" << endl;
	tmp_maketodivision = false;std::cout << "3235" << endl;
	if(!calculate_thread->write(false)) {calculate_thread->cancel(); mstruct->setAborted(); return false;}
	if(!calculate_thread->write((void*) mstruct)) {calculate_thread->cancel(); mstruct->setAborted(); return false;}
	while(msecs > 0 && b_busy) {
		sleep_ms(10);std::cout << "3239" << endl;
		msecs -= 10;std::cout << "3240" << endl;
	}
	if(had_msecs && b_busy) {
		if(!abort()) mstruct->setAborted();
		return false;
	}
	return true;
}
bool Calculator::hasToExpression(const string &str, bool allow_empty_from) const {
	if(str.empty()) return false;
	size_t i = str.length() - 1, i2 = i;std::cout << "3250" << endl;
	int l = 2;std::cout << "3251" << endl;
	while(i != 0) {
		i2 = str.rfind(_("to"), i - 1);
		i = str.rfind("to", i - 1);
		if(i2 != string::npos && (i == string::npos || i < i2)) {l = strlen(_("to")); i = i2;}
		else l = 2;std::cout << "3256" << endl;
		if(i == string::npos) break;
		if(((i > 0 && is_in(SPACES, str[i - 1])) || (allow_empty_from && i == 0)) && i + l < str.length() && is_in(SPACES, str[i + l])) return true;
	}
	return false;
}
bool Calculator::hasToExpression(const string &str, bool allow_empty_from, const EvaluationOptions &eo) const {
	if(eo.parse_options.base == BASE_UNICODE || (eo.parse_options.base == BASE_CUSTOM && priv->custom_input_base_i > 62)) return false;
	if(str.empty()) return false;
	size_t i = str.length() - 1, i2 = i;std::cout << "3265" << endl;
	int l = 2;std::cout << "3266" << endl;
	while(i != 0) {
		i2 = str.rfind(_("to"), i - 1);
		i = str.rfind("to", i - 1);
		if(i2 != string::npos && (i == string::npos || i < i2)) {l = strlen(_("to")); i = i2;}
		else l = 2;std::cout << "3271" << endl;
		if(i == string::npos) break;
		if(((i > 0 && is_in(SPACES, str[i - 1])) || (allow_empty_from && i == 0)) && i + l < str.length() && is_in(SPACES, str[i + l])) return true;
	}
	return false;
}
bool Calculator::separateToExpression(string &str, string &to_str, const EvaluationOptions &eo, bool keep_modifiers, bool allow_empty_from) const {
	if(eo.parse_options.base == BASE_UNICODE || (eo.parse_options.base == BASE_CUSTOM && priv->custom_input_base_i > 62)) return false;
	to_str = "";
	if(str.empty()) return false;
	size_t i = str.length() - 1, i2 = i;std::cout << "3281" << endl;
	int l = 2;std::cout << "3282" << endl;
	while(i != 0) {
		i2 = str.rfind(_("to"), i - 1);
		i = str.rfind("to", i - 1);
		if(i2 != string::npos && (i == string::npos || i < i2)) {l = strlen(_("to")); i = i2;}
		else l = 2;std::cout << "3287" << endl;
		if(i == string::npos) break;
		if(((i > 0 && is_in(SPACES, str[i - 1])) || (allow_empty_from && i == 0)) && i + l < str.length() && is_in(SPACES, str[i + l])) {
			to_str = str.substr(i + l , str.length() - i - l);std::cout << "3290" << endl;
			if(to_str.empty()) return false;
			remove_blank_ends(to_str);std::cout << "3292" << endl;
			if(!to_str.empty()) {
				if(to_str.rfind(SIGN_MINUS, 0) == 0) {
					to_str.replace(0, strlen(SIGN_MINUS), MINUS);std::cout << "3295" << endl;
				}
				if(!keep_modifiers && (to_str[0] == '0' || to_str[0] == '?' || to_str[0] == '+' || to_str[0] == '-')) {
					to_str = to_str.substr(1, str.length() - 1);std::cout << "3298" << endl;
					remove_blank_ends(to_str);std::cout << "3299" << endl;
				} else if(!keep_modifiers && to_str.length() > 1 && to_str[1] == '?' && (to_str[0] == 'b' || to_str[0] == 'a' || to_str[0] == 'd')) {
					to_str = to_str.substr(2, str.length() - 2);std::cout << "3301" << endl;
					remove_blank_ends(to_str);std::cout << "3302" << endl;
				}
			}
			str = str.substr(0, i);std::cout << "3305" << endl;
			return true;
		}
	}
	return false;
}
bool Calculator::hasWhereExpression(const string &str, const EvaluationOptions &eo) const {
	if(eo.parse_options.base == BASE_UNICODE || (eo.parse_options.base == BASE_CUSTOM && priv->custom_input_base_i > 62)) return false;
	if(str.empty()) return false;
	size_t i = str.length() - 1, i2 = i;std::cout << "3314" << endl;
	int l = 2;std::cout << "3315" << endl;
	while(i != 0) {
		//"where"-operator
		i2 = str.rfind(_("where"), i - 1);
		i = str.rfind("where", i - 1);
		if(i2 != string::npos && (i == string::npos || i < i2)) {l = strlen(_("where")); i = i2;}
		else l = 2;std::cout << "3321" << endl;
		if(i == string::npos) break;
		if(i > 0 && is_in(SPACES, str[i - 1]) && i + l < str.length() && is_in(SPACES, str[i + l])) return true;
	}
	if((i = str.rfind("/.", str.length() - 2)) != string::npos && eo.parse_options.base >= 2 && eo.parse_options.base <= 10 && (str[i + 2] < '0' || str[i + 2] > '9')) return true;
	return false;
}
bool Calculator::separateWhereExpression(string &str, string &to_str, const EvaluationOptions &eo) const {
	if(eo.parse_options.base == BASE_UNICODE || (eo.parse_options.base == BASE_CUSTOM && priv->custom_input_base_i > 62)) return false;
	to_str = "";
	size_t i = 0;std::cout << "3331" << endl;
	if((i = str.rfind("/.", str.length() - 2)) != string::npos && i != str.length() - 2 && eo.parse_options.base >= 2 && eo.parse_options.base <= 10 && (str[i + 2] < '0' || str[i + 2] > '9')) {
		to_str = str.substr(i + 2 , str.length() - i - 2);std::cout << "3333" << endl;
	} else {
		i = str.length() - 1;std::cout << "3335" << endl;
		size_t i2 = i;std::cout << "3336" << endl;
		int l = 5;std::cout << "3337" << endl;
		while(i != 0) {
			i2 = str.rfind(_("where"), i - 1);
			i = str.rfind("where", i - 1);
			if(i2 != string::npos && (i == string::npos || i < i2)) {l = strlen(_("where")); i = i2;}
			else l = 5;std::cout << "3342" << endl;
			if(i == string::npos) break;
			if(i > 0 && is_in(SPACES, str[i - 1]) && i + l < str.length() && is_in(SPACES, str[i + l])) {
				to_str = str.substr(i + l , str.length() - i - l);std::cout << "3345" << endl;
				break;
			}
		}
	}
	if(!to_str.empty()) {
		remove_blank_ends(to_str);std::cout << "3351" << endl;
		str = str.substr(0, i);std::cout << "3352" << endl;
		parseSigns(str);std::cout << "3353" << endl;
		if(str.find("&&") == string::npos) {
			int par = 0;std::cout << "3355" << endl;
			int bra = 0;std::cout << "3356" << endl;
			for(size_t i = 0; i < str.length(); i++) {
				switch(str[i]) {
					case '(': {par++; break;}
					case ')': {if(par > 0) par--; break;}
					case '[': {bra++; break;}
					case ']': {if(bra > 0) bra--; break;}
					case COMMA_CH: {
						if(par == 0 && bra == 0) {
							str.replace(i, 1, LOGICAL_AND);std::cout << "3365" << endl;
							i++;std::cout << "3366" << endl;
						}
						break;
					}
					default: {}
				}
			}
		}
		return true;
	}
	return false;
}
extern string format_and_print(const MathStructure &mstruct);
extern bool replace_function(MathStructure &m, MathFunction *f1, MathFunction *f2, const EvaluationOptions &eo);
extern bool replace_intervals_f(MathStructure &mstruct);
extern bool replace_f_interval(MathStructure &mstruct, const EvaluationOptions &eo);

bool calculate_rand(MathStructure &mstruct, const EvaluationOptions &eo) {
	if(mstruct.isFunction() && mstruct.function() == CALCULATOR->f_rand) {
		mstruct.unformat(eo);
		mstruct.calculateFunctions(eo, false);std::cout << "3386" << endl;
		return true;
	}
	bool ret = false;std::cout << "3389" << endl;
	for(size_t i = 0; i < mstruct.size(); i++) {
		if(calculate_rand(mstruct[i], eo)) {
			ret = true;std::cout << "3392" << endl;
			mstruct.childUpdated(i + 1);std::cout << "3393" << endl;
		}
	}
	return ret;
}
bool calculate_ans(MathStructure &mstruct, const EvaluationOptions &eo) {
	if(mstruct.isFunction() && (mstruct.function()->hasName("answer") || mstruct.function()->hasName("expression"))) {
		mstruct.unformat(eo);
		mstruct.calculateFunctions(eo, false);std::cout << "3401" << endl;
		return true;
	} else if(mstruct.isVariable() && mstruct.variable()->isKnown() && (mstruct.variable()->referenceName() == "ans" || (mstruct.variable()->referenceName().length() == 4 && mstruct.variable()->referenceName().substr(0, 3) == "ans" && is_in(NUMBERS, mstruct.variable()->referenceName()[3])))) {
		mstruct.set(((KnownVariable*) mstruct.variable())->get(), true);std::cout << "3404" << endl;
		return true;
	}
	bool ret = false;std::cout << "3407" << endl;
	for(size_t i = 0; i < mstruct.size(); i++) {
		if(calculate_ans(mstruct[i], eo)) {
			mstruct.childUpdated(i + 1);std::cout << "3410" << endl;
			ret = true;std::cout << "3411" << endl;
		}
	}
	return ret;
}
bool handle_where_expression(MathStructure &m, MathStructure &mstruct, const EvaluationOptions &eo, vector<UnknownVariable*>& vars, vector<MathStructure>& varms, bool empty_func, bool do_eval = true) {
	if(m.isComparison()) {
		if(m.comparisonType() == COMPARISON_EQUALS) {
			if(m[0].size() > 0 && do_eval) {
				MathStructure m2(m);std::cout << "3420" << endl;
				MathStructure xvar = m[0].find_x_var();std::cout << "3421" << endl;
				EvaluationOptions eo2 = eo;std::cout << "3422" << endl;
				eo2.isolate_x = true;std::cout << "3423" << endl;
				if(!xvar.isUndefined()) eo2.isolate_var = &xvar;
				m2.eval(eo2);std::cout << "3425" << endl;
				if(m2.isComparison()) return handle_where_expression(m2, mstruct, eo, vars, varms, false, false);
			}
			if(m[0].isFunction() && m[1].isFunction() && (m[0].size() == 0 || (empty_func && m[0].function()->minargs() == 0)) && (m[1].size() == 0 || (empty_func && m[1].function()->minargs() == 0))) {
				if(!replace_function(mstruct, m[0].function(), m[1].function(), eo)) CALCULATOR->error(false, _("Original value (%s) was not found."), (m[0].function()->name() + "()").c_str(), NULL);
			} else {
				calculate_rand(m[1], eo);std::cout << "3431" << endl;
				if(mstruct.countOccurrences(m[0]) > 1 && m[1].containsInterval(true, false, false, 0, true)) {
					MathStructure mv(m[1]);std::cout << "3433" << endl;
					replace_f_interval(mv, eo);std::cout << "3434" << endl;
					replace_intervals_f(mv);std::cout << "3435" << endl;
					if(!mstruct.replace(m[0], mv)) CALCULATOR->error(false, _("Original value (%s) was not found."), format_and_print(m[0]).c_str(), NULL);
				} else {
					if(!mstruct.replace(m[0], m[1])) CALCULATOR->error(false, _("Original value (%s) was not found."), format_and_print(m[0]).c_str(), NULL);
				}
			}
			return true;
		} else if(m[0].isSymbolic() || (m[0].isVariable() && !m[0].variable()->isKnown())) {
			if(!m[1].isNumber()) m[1].eval(eo);
			if(m[1].isNumber() && !m[1].number().hasImaginaryPart()) {
				Assumptions *ass = NULL;std::cout << "3445" << endl;
				for(size_t i = 0; i < varms.size(); i++) {
					if(varms[i] == m[0]) {
						ass = vars[0]->assumptions();std::cout << "3448" << endl;
						break;
					}
				}
				if((m.comparisonType() != COMPARISON_NOT_EQUALS || (!ass && m[1].isZero()))) {
					if(ass) {
						if(m.comparisonType() == COMPARISON_EQUALS_GREATER) {
							if(!ass->min() || (*ass->min() < m[1].number())) {
								ass->setMin(&m[1].number());std::cout << "3456" << endl; ass->setIncludeEqualsMin(true);std::cout << "3456" << endl;
								return true;
							} else if(*ass->min() >= m[1].number()) {
								return true;
							}
						} else if(m.comparisonType() == COMPARISON_EQUALS_LESS) {
							if(!ass->max() || (*ass->max() > m[1].number())) {
								ass->setMax(&m[1].number());std::cout << "3463" << endl; ass->setIncludeEqualsMax(true);std::cout << "3463" << endl;
								return true;
							} else if(*ass->max() <= m[1].number()) {
								return true;
							}
						} else if(m.comparisonType() == COMPARISON_GREATER) {
							if(!ass->min() || (ass->includeEqualsMin() && *ass->min() <= m[1].number()) || (!ass->includeEqualsMin() && *ass->min() < m[1].number())) {
								ass->setMin(&m[1].number());std::cout << "3470" << endl; ass->setIncludeEqualsMin(false);std::cout << "3470" << endl;
								return true;
							} else if((ass->includeEqualsMin() && *ass->min() > m[1].number()) || (!ass->includeEqualsMin() && *ass->min() >= m[1].number())) {
								return true;
							}
						} else if(m.comparisonType() == COMPARISON_LESS) {
							if(!ass->max() || (ass->includeEqualsMax() && *ass->max() >= m[1].number()) || (!ass->includeEqualsMax() && *ass->max() > m[1].number())) {
								ass->setMax(&m[1].number());std::cout << "3477" << endl; ass->setIncludeEqualsMax(false);std::cout << "3477" << endl;
								return true;
							} else if((ass->includeEqualsMax() && *ass->max() < m[1].number()) || (!ass->includeEqualsMax() && *ass->max() <= m[1].number())) {
								return true;
							}
						}
					} else {
						UnknownVariable *var = new UnknownVariable("", format_and_print(m[0]));
						ass = new Assumptions();std::cout << "3485" << endl;
						if(m[1].isZero()) {
							if(m.comparisonType() == COMPARISON_EQUALS_GREATER) ass->setSign(ASSUMPTION_SIGN_NONNEGATIVE);
							else if(m.comparisonType() == COMPARISON_EQUALS_LESS) ass->setSign(ASSUMPTION_SIGN_NONPOSITIVE);
							else if(m.comparisonType() == COMPARISON_GREATER) ass->setSign(ASSUMPTION_SIGN_POSITIVE);
							else if(m.comparisonType() == COMPARISON_LESS) ass->setSign(ASSUMPTION_SIGN_NEGATIVE);
							else if(m.comparisonType() == COMPARISON_NOT_EQUALS) ass->setSign(ASSUMPTION_SIGN_NONZERO);
						} else {
							if(m.comparisonType() == COMPARISON_EQUALS_GREATER) {ass->setMin(&m[1].number()); ass->setIncludeEqualsMin(true);}
							else if(m.comparisonType() == COMPARISON_EQUALS_LESS) {ass->setMax(&m[1].number()); ass->setIncludeEqualsMax(true);}
							else if(m.comparisonType() == COMPARISON_GREATER) {ass->setMin(&m[1].number()); ass->setIncludeEqualsMin(false);}
							else if(m.comparisonType() == COMPARISON_LESS) {ass->setMax(&m[1].number()); ass->setIncludeEqualsMax(false);}
						}
						var->setAssumptions(ass);std::cout << "3498" << endl;
						vars.push_back(var);std::cout << "3499" << endl;
						varms.push_back(m[0]);std::cout << "3500" << endl;
						MathStructure u_var(var);std::cout << "3501" << endl;
						if(!mstruct.replace(m[0], u_var)) CALCULATOR->error(false, _("Original value (%s) was not found."), format_and_print(m[0]).c_str(), NULL);
						return true;
					}
				}
			}
		} else if(do_eval) {
			MathStructure xvar = m[0].find_x_var();std::cout << "3508" << endl;
			EvaluationOptions eo2 = eo;std::cout << "3509" << endl;
			eo2.isolate_x = true;std::cout << "3510" << endl;
			if(!xvar.isUndefined()) eo2.isolate_var = &xvar;
			m.eval(eo2);std::cout << "3512" << endl;
			return handle_where_expression(m, mstruct, eo, vars, varms, false, false);
		}
	} else if(m.isLogicalAnd()) {
		bool ret = true;std::cout << "3516" << endl;
		for(size_t i = 0; i < m.size(); i++) {
			if(!handle_where_expression(m[i], mstruct, eo, vars, varms, empty_func, do_eval)) ret = false;
		}
		return ret;
	}
	CALCULATOR->error(true, _("Unhandled \"where\" expression: %s"), format_and_print(m).c_str(), NULL);
	return false;
}
MathStructure Calculator::calculate(string str, const EvaluationOptions &eo, MathStructure *parsed_struct, MathStructure *to_struct, bool make_to_division) {

	string str2, str_where;std::cout << "3527" << endl;

	if(make_to_division) separateToExpression(str, str2, eo, true);
	separateWhereExpression(str, str_where, eo);std::cout << "3530" << endl;

	Unit *u = NULL;std::cout << "3532" << endl;
	if(to_struct) {
		if(str2.empty()) {
			if(to_struct->isSymbolic() && !to_struct->symbol().empty()) {
				str2 = to_struct->symbol();std::cout << "3536" << endl;
				remove_blank_ends(str2);std::cout << "3537" << endl;
			} else if(to_struct->isUnit()) {
				u = to_struct->unit();std::cout << "3539" << endl;
			}
		}
		to_struct->setUndefined();std::cout << "3542" << endl;
	}

	// ok
	MathStructure mstruct;std::cout << "3546" << endl;

	current_stage = MESSAGE_STAGE_PARSING;std::cout << "3548" << endl;
	size_t n_messages = messages.size();std::cout << "3549" << endl;
	parse(&mstruct, str, eo.parse_options);std::cout << "3550" << endl;
	// not ok
	if(parsed_struct) {
		beginTemporaryStopMessages();std::cout << "3553" << endl;
		ParseOptions po = eo.parse_options;std::cout << "3554" << endl;
		po.preserve_format = true;
		parse(parsed_struct, str, po);std::cout << "3556" << endl;
		endTemporaryStopMessages();std::cout << "3557" << endl;
	}


	vector<UnknownVariable*> vars;std::cout << "3561" << endl;
	vector<MathStructure> varms;std::cout << "3562" << endl;
	if(!str_where.empty()) {
		MathStructure where_struct;std::cout << "3564" << endl;
		parse(&where_struct, str_where, eo.parse_options);std::cout << "3565" << endl;
		current_stage = MESSAGE_STAGE_CALCULATION;std::cout << "3566" << endl;
		calculate_ans(mstruct, eo);std::cout << "3567" << endl;
		string str_test = str_where;std::cout << "3568" << endl;
		remove_blanks(str_test);std::cout << "3569" << endl;
		bool empty_func = str_test.find("()=") != string::npos;
		if(mstruct.isComparison() || (mstruct.isFunction() && mstruct.function() == CALCULATOR->f_solve && mstruct.size() >= 1 && mstruct[0].isComparison())) {
			beginTemporaryStopMessages();std::cout << "3572" << endl;
			MathStructure mbak(mstruct);std::cout << "3573" << endl;
			if(handle_where_expression(where_struct, mstruct, eo, vars, varms, empty_func)) {
				endTemporaryStopMessages(true);std::cout << "3575" << endl;
			} else {
				endTemporaryStopMessages();std::cout << "3577" << endl;
				mstruct = mbak;std::cout << "3578" << endl;
				if(mstruct.isComparison()) mstruct.transform(STRUCT_LOGICAL_AND, where_struct);
				else {mstruct[0].transform(STRUCT_LOGICAL_AND, where_struct); mstruct.childUpdated(1);}
			}
		} else {
			if(eo.approximation == APPROXIMATION_EXACT) {
				EvaluationOptions eo2 = eo;std::cout << "3584" << endl;
				eo2.approximation = APPROXIMATION_TRY_EXACT;std::cout << "3585" << endl;
				handle_where_expression(where_struct, mstruct, eo2, vars, varms, empty_func);std::cout << "3586" << endl;
			} else {
				handle_where_expression(where_struct, mstruct, eo, vars, varms, empty_func);std::cout << "3588" << endl;
			}
		}
	}

    // not ok

	current_stage = MESSAGE_STAGE_CALCULATION;std::cout << "3595" << endl;

	mstruct.eval(eo);std::cout << "3597" << endl;

	current_stage = MESSAGE_STAGE_UNSET;std::cout << "3599" << endl;

	if(!aborted()) {
		bool b_units = mstruct.containsType(STRUCT_UNIT, true);std::cout << "3602" << endl;
		if(b_units && u) {
			current_stage = MESSAGE_STAGE_CONVERSION;std::cout << "3604" << endl;
			if(to_struct) to_struct->set(u);
			mstruct.set(convert(mstruct, u, eo, false, false));std::cout << "3606" << endl;
			if(eo.mixed_units_conversion != MIXED_UNITS_CONVERSION_NONE) mstruct.set(convertToMixedUnits(mstruct, eo));
		} else if(!str2.empty()) {
			mstruct.set(convert(mstruct, str2, eo));std::cout << "3609" << endl;
		} else if(b_units) {
			current_stage = MESSAGE_STAGE_CONVERSION;std::cout << "3611" << endl;
			switch(eo.auto_post_conversion) {
				case POST_CONVERSION_OPTIMAL: {
					mstruct.set(convertToOptimalUnit(mstruct, eo, false));std::cout << "3614" << endl;
					break;
				}
				case POST_CONVERSION_BASE: {
					mstruct.set(convertToBaseUnits(mstruct, eo));std::cout << "3618" << endl;
					break;
				}
				case POST_CONVERSION_OPTIMAL_SI: {
					mstruct.set(convertToOptimalUnit(mstruct, eo, true));std::cout << "3622" << endl;
					break;
				}
				default: {}
			}
			if(eo.mixed_units_conversion != MIXED_UNITS_CONVERSION_NONE) mstruct.set(convertToMixedUnits(mstruct, eo));
		}
	}

	cleanMessages(mstruct, n_messages + 1);std::cout << "3631" << endl;

	current_stage = MESSAGE_STAGE_UNSET;std::cout << "3633" << endl;

	for(size_t i = 0; i < vars.size(); i++) {
		mstruct.replace(vars[i], varms[i]);std::cout << "3636" << endl;
		vars[i]->destroy();std::cout << "3637" << endl;
	}

	return mstruct;

}
MathStructure Calculator::calculate(const MathStructure &mstruct_to_calculate, const EvaluationOptions &eo, string to_str) {

	remove_blank_ends(to_str);std::cout << "3645" << endl;
	MathStructure mstruct(mstruct_to_calculate);std::cout << "3646" << endl;
	current_stage = MESSAGE_STAGE_CALCULATION;std::cout << "3647" << endl;
	size_t n_messages = messages.size();std::cout << "3648" << endl;
	mstruct.eval(eo);std::cout << "3649" << endl;

	current_stage = MESSAGE_STAGE_CONVERSION;std::cout << "3651" << endl;
	if(!to_str.empty()) {
		mstruct.set(convert(mstruct, to_str, eo));std::cout << "3653" << endl;
	} else {
		switch(eo.auto_post_conversion) {
			case POST_CONVERSION_OPTIMAL: {
				mstruct.set(convertToOptimalUnit(mstruct, eo, false));std::cout << "3657" << endl;
				break;
			}
			case POST_CONVERSION_BASE: {
				mstruct.set(convertToBaseUnits(mstruct, eo));std::cout << "3661" << endl;
				break;
			}
			case POST_CONVERSION_OPTIMAL_SI: {
				mstruct.set(convertToOptimalUnit(mstruct, eo, true));std::cout << "3665" << endl;
				break;
			}
			default: {}
		}
		if(eo.mixed_units_conversion != MIXED_UNITS_CONVERSION_NONE) mstruct.set(convertToMixedUnits(mstruct, eo));
	}

	cleanMessages(mstruct, n_messages + 1);std::cout << "3673" << endl;

	current_stage = MESSAGE_STAGE_UNSET;std::cout << "3675" << endl;
	return mstruct;
}

string Calculator::print(const MathStructure &mstruct, int msecs, const PrintOptions &po) {
	startControl(msecs);std::cout << "3680" << endl;
	MathStructure mstruct2(mstruct);std::cout << "3681" << endl;
	mstruct2.format(po);
	string print_result = mstruct2.print(po);std::cout << "3683" << endl;
	stopControl();std::cout << "3684" << endl;
	return print_result;
}
string Calculator::printMathStructureTimeOut(const MathStructure &mstruct, int msecs, const PrintOptions &po) {
	return print(mstruct, msecs, po);
}

MathStructure Calculator::convertToMixedUnits(const MathStructure &mstruct, const EvaluationOptions &eo) {
	if(eo.mixed_units_conversion == MIXED_UNITS_CONVERSION_NONE) return mstruct;
	if(!mstruct.isMultiplication()) return mstruct;
	if(mstruct.size() != 2) return mstruct;
	size_t n_messages = messages.size();std::cout << "3695" << endl;
	if(mstruct[1].isUnit() && (!mstruct[1].prefix() || mstruct[1].prefix() == decimal_null_prefix) && mstruct[0].isNumber()) {
		Prefix *p = mstruct[1].prefix();std::cout << "3697" << endl;
		MathStructure mstruct_new(mstruct);std::cout << "3698" << endl;
		Unit *u = mstruct[1].unit();std::cout << "3699" << endl;
		Number nr = mstruct[0].number();std::cout << "3700" << endl;
		if(!nr.isReal()) return mstruct;
		if(nr.isOne()) return mstruct;
		if(u->subtype() == SUBTYPE_COMPOSITE_UNIT) return mstruct;
		bool negated = false;std::cout << "3704" << endl;
		if(nr.isNegative()) {
			nr.negate();std::cout << "3706" << endl;
			negated = true;std::cout << "3707" << endl;
		}
		bool accept_obsolete = (u->subtype() == SUBTYPE_ALIAS_UNIT && abs(((AliasUnit*) u)->mixWithBase()) > 1);std::cout << "3709" << endl;
		Unit *original_u = u;std::cout << "3710" << endl;
		Unit *last_nonobsolete_u = u;std::cout << "3711" << endl;
		Number last_nonobsolete_nr = nr;std::cout << "3712" << endl;
		Number nr_one(1, 1);std::cout << "3713" << endl;
		Number nr_ten(10, 1);std::cout << "3714" << endl;
		while(eo.mixed_units_conversion > MIXED_UNITS_CONVERSION_DOWNWARDS && nr.isGreaterThan(nr_one)) {
			Unit *best_u = NULL;std::cout << "3716" << endl;
			Number best_nr;std::cout << "3717" << endl;
			int best_priority = 0;std::cout << "3718" << endl;
			for(size_t i = 0; i < units.size(); i++) {
				Unit *ui = units[i];std::cout << "3720" << endl;
				if(ui->subtype() == SUBTYPE_ALIAS_UNIT && ((AliasUnit*) ui)->firstBaseUnit() == u  && ((AliasUnit*) ui)->firstBaseExponent() == 1) {
					AliasUnit *aui = (AliasUnit*) ui;std::cout << "3722" << endl;
					int priority_i = aui->mixWithBase();std::cout << "3723" << endl;
					if(((priority_i > 0 && (!best_u || priority_i <= best_priority)) || (best_priority == 0 && priority_i == 0 && ((eo.mixed_units_conversion == MIXED_UNITS_CONVERSION_FORCE_INTEGER && aui->expression().find_first_not_of(NUMBERS) == string::npos) || eo.mixed_units_conversion == MIXED_UNITS_CONVERSION_FORCE_ALL))) && (aui->mixWithBaseMinimum() <= 1 || nr.isGreaterThanOrEqualTo(aui->mixWithBaseMinimum()))) {
						MathStructure mstruct_nr(nr);std::cout << "3725" << endl;
						MathStructure m_exp(m_one);std::cout << "3726" << endl;
						aui->convertFromFirstBaseUnit(mstruct_nr, m_exp);std::cout << "3727" << endl;
						mstruct_nr.eval(eo);std::cout << "3728" << endl;
						if(mstruct_nr.isNumber() && m_exp.isOne() && mstruct_nr.number().isLessThan(nr) && mstruct_nr.number().isGreaterThanOrEqualTo(nr_one) && (!best_u || mstruct_nr.number().isLessThan(best_nr))) {
							best_u = ui;std::cout << "3730" << endl;
							best_nr = mstruct_nr.number();std::cout << "3731" << endl;
							best_priority = priority_i;std::cout << "3732" << endl;
						}
					}
				}
			}
			if(!best_u) break;
			u = best_u;std::cout << "3738" << endl;
			nr = best_nr;std::cout << "3739" << endl;
			if(accept_obsolete || best_priority <= 1) {
				last_nonobsolete_u = u;std::cout << "3741" << endl;
				last_nonobsolete_nr = nr;std::cout << "3742" << endl;
			}
		}
		u = last_nonobsolete_u;std::cout << "3745" << endl;
		nr = last_nonobsolete_nr;std::cout << "3746" << endl;
		if(u != original_u) {
			if(negated) last_nonobsolete_nr.negate();
			mstruct_new[0].set(last_nonobsolete_nr);std::cout << "3749" << endl;
			mstruct_new[1].set(u, p);std::cout << "3750" << endl;
		}
		while(u->subtype() == SUBTYPE_ALIAS_UNIT && ((AliasUnit*) u)->firstBaseUnit()->subtype() != SUBTYPE_COMPOSITE_UNIT && ((AliasUnit*) u)->firstBaseExponent() == 1 && (((AliasUnit*) u)->mixWithBase() != 0 || eo.mixed_units_conversion == MIXED_UNITS_CONVERSION_FORCE_ALL || (eo.mixed_units_conversion == MIXED_UNITS_CONVERSION_FORCE_INTEGER && ((AliasUnit*) u)->expression().find_first_not_of(NUMBERS) == string::npos)) && !nr.isInteger() && !nr.isZero()) {
			Number int_nr(nr);std::cout << "3753" << endl;
			int_nr.trunc();std::cout << "3754" << endl;
			if(eo.mixed_units_conversion == MIXED_UNITS_CONVERSION_DOWNWARDS_KEEP && int_nr.isZero()) break;
			nr -= int_nr;std::cout << "3756" << endl;
			MathStructure mstruct_nr(nr);std::cout << "3757" << endl;
			MathStructure m_exp(m_one);std::cout << "3758" << endl;
			((AliasUnit*) u)->convertToFirstBaseUnit(mstruct_nr, m_exp);std::cout << "3759" << endl;
			mstruct_nr.eval(eo);std::cout << "3760" << endl;
			while(!accept_obsolete && ((AliasUnit*) u)->firstBaseUnit()->subtype() == SUBTYPE_ALIAS_UNIT && abs(((AliasUnit*) ((AliasUnit*) u)->firstBaseUnit())->mixWithBase()) > 1) {
				u = ((AliasUnit*) u)->firstBaseUnit();std::cout << "3762" << endl;
				if(((AliasUnit*) u)->firstBaseExponent() == 1 && (((AliasUnit*) u)->mixWithBase() != 0 || eo.mixed_units_conversion == MIXED_UNITS_CONVERSION_FORCE_ALL || (eo.mixed_units_conversion == MIXED_UNITS_CONVERSION_FORCE_INTEGER && ((AliasUnit*) u)->expression().find_first_not_of(NUMBERS) == string::npos))) {
					((AliasUnit*) u)->convertToFirstBaseUnit(mstruct_nr, m_exp);std::cout << "3764" << endl;
					mstruct_nr.eval(eo);std::cout << "3765" << endl;
					if(!mstruct_nr.isNumber() || !m_exp.isOne()) break;
				} else {
					mstruct_nr.setUndefined();std::cout << "3768" << endl;
					break;
				}
			}
			if(!mstruct_nr.isNumber() || !m_exp.isOne()) break;
			if(eo.mixed_units_conversion == MIXED_UNITS_CONVERSION_FORCE_ALL && mstruct_nr.number().isLessThanOrEqualTo(nr)) break;
			u = ((AliasUnit*) u)->firstBaseUnit();std::cout << "3774" << endl;
			nr = mstruct_nr.number();std::cout << "3775" << endl;
			MathStructure mstruct_term;std::cout << "3776" << endl;
			if(negated) {
				Number pos_nr(nr);std::cout << "3778" << endl;
				pos_nr.negate();std::cout << "3779" << endl;
				mstruct_term.set(pos_nr);std::cout << "3780" << endl;
			} else {
				mstruct_term.set(nr);std::cout << "3782" << endl;
			}
			mstruct_term *= MathStructure(u, p);std::cout << "3784" << endl;
			if(int_nr.isZero()) {
				if(mstruct_new.isAddition()) mstruct_new[mstruct_new.size() - 1].set(mstruct_term);
				else mstruct_new.set(mstruct_term);std::cout << "3787" << endl;
			} else {
				if(negated) int_nr.negate();
				if(mstruct_new.isAddition()) mstruct_new[mstruct_new.size() - 1][0].set(int_nr);
				else mstruct_new[0].set(int_nr);std::cout << "3791" << endl;
				mstruct_new.add(mstruct_term, true);std::cout << "3792" << endl;
			}
		}
		cleanMessages(mstruct_new, n_messages + 1);std::cout << "3795" << endl;
		return mstruct_new;
	}
	return mstruct;
}

MathStructure Calculator::convert(double value, Unit *from_unit, Unit *to_unit, const EvaluationOptions &eo) {
	size_t n_messages = messages.size();std::cout << "3802" << endl;
	MathStructure mstruct(value);std::cout << "3803" << endl;
	mstruct *= from_unit;std::cout << "3804" << endl;
	mstruct.eval(eo);std::cout << "3805" << endl;
	if(eo.approximation == APPROXIMATION_EXACT) {
		EvaluationOptions eo2 = eo;std::cout << "3807" << endl;
		eo2.approximation = APPROXIMATION_TRY_EXACT;std::cout << "3808" << endl;
		mstruct.convert(to_unit, true, NULL, false, eo2);std::cout << "3809" << endl;
	} else {
		mstruct.convert(to_unit, true, NULL, false, eo);std::cout << "3811" << endl;
	}
	mstruct.divide(to_unit, true);std::cout << "3813" << endl;
	mstruct.eval(eo);std::cout << "3814" << endl;
	cleanMessages(mstruct, n_messages + 1);std::cout << "3815" << endl;
	return mstruct;

}
MathStructure Calculator::convert(string str, Unit *from_unit, Unit *to_unit, int msecs, const EvaluationOptions &eo) {
	return convertTimeOut(str, from_unit, to_unit, msecs, eo);
}
MathStructure Calculator::convertTimeOut(string str, Unit *from_unit, Unit *to_unit, int msecs, const EvaluationOptions &eo) {
	MathStructure mstruct;std::cout << "3823" << endl;
	parse(&mstruct, str, eo.parse_options);std::cout << "3824" << endl;
	mstruct *= from_unit;std::cout << "3825" << endl;
	b_busy = true;std::cout << "3826" << endl;
	if(!calculate_thread->running && !calculate_thread->start()) return mstruct;
	bool had_msecs = msecs > 0;std::cout << "3828" << endl;
	tmp_evaluationoptions = eo;std::cout << "3829" << endl;
	tmp_proc_command = PROC_NO_COMMAND;std::cout << "3830" << endl;
	bool b_parse = false;std::cout << "3831" << endl;
	if(!calculate_thread->write(b_parse)) {calculate_thread->cancel(); return mstruct;}
	void *x = (void*) &mstruct;std::cout << "3833" << endl;
	if(!calculate_thread->write(x)) {calculate_thread->cancel(); return mstruct;}
	while(msecs > 0 && b_busy) {
		sleep_ms(10);std::cout << "3836" << endl;
		msecs -= 10;std::cout << "3837" << endl;
	}
	if(had_msecs && b_busy) {
		abort();std::cout << "3840" << endl;
		mstruct.setAborted();std::cout << "3841" << endl;
		return mstruct;
	}
	if(eo.approximation == APPROXIMATION_EXACT) {
		EvaluationOptions eo2 = eo;std::cout << "3845" << endl;
		eo2.approximation = APPROXIMATION_TRY_EXACT;std::cout << "3846" << endl;
		mstruct.convert(to_unit, true, NULL, false, eo2);std::cout << "3847" << endl;
	} else {
		mstruct.convert(to_unit, true, NULL, false, eo);std::cout << "3849" << endl;
	}
	mstruct.divide(to_unit, true);std::cout << "3851" << endl;
	b_busy = true;std::cout << "3852" << endl;
	if(!calculate_thread->write(b_parse)) {calculate_thread->cancel(); return mstruct;}
	x = (void*) &mstruct;std::cout << "3854" << endl;
	if(!calculate_thread->write(x)) {calculate_thread->cancel(); return mstruct;}
	while(msecs > 0 && b_busy) {
		sleep_ms(10);std::cout << "3857" << endl;
		msecs -= 10;std::cout << "3858" << endl;
	}
	if(had_msecs && b_busy) {
		abort();std::cout << "3861" << endl;
		mstruct.setAborted();std::cout << "3862" << endl;
	}
	return mstruct;
}
MathStructure Calculator::convert(string str, Unit *from_unit, Unit *to_unit, const EvaluationOptions &eo) {
	size_t n_messages = messages.size();std::cout << "3867" << endl;
	MathStructure mstruct;std::cout << "3868" << endl;
	parse(&mstruct, str, eo.parse_options);std::cout << "3869" << endl;
	mstruct *= from_unit;std::cout << "3870" << endl;
	mstruct.eval(eo);std::cout << "3871" << endl;
	if(eo.approximation == APPROXIMATION_EXACT) {
		EvaluationOptions eo2 = eo;std::cout << "3873" << endl;
		eo2.approximation = APPROXIMATION_TRY_EXACT;std::cout << "3874" << endl;
		mstruct.convert(to_unit, true, NULL, false, eo2);std::cout << "3875" << endl;
	} else {
		mstruct.convert(to_unit, true, NULL, false, eo);std::cout << "3877" << endl;
	}
	mstruct.divide(to_unit, true);std::cout << "3879" << endl;
	mstruct.eval(eo);std::cout << "3880" << endl;
	cleanMessages(mstruct, n_messages + 1);std::cout << "3881" << endl;
	return mstruct;
}
MathStructure Calculator::convert(const MathStructure &mstruct, KnownVariable *to_var, const EvaluationOptions &eo) {
	if(mstruct.contains(to_var, true) > 0) return mstruct;
	size_t n_messages = messages.size();std::cout << "3886" << endl;
	if(b_var_units && !to_var->unit().empty() && to_var->isExpression()) {
		CompositeUnit cu("", "temporary_composite_convert", "", to_var->unit());
		if(cu.countUnits() > 0) {
			AliasUnit au("", "temporary_alias_convert", "", "", "", &cu, to_var->expression());
			bool unc_rel = false;std::cout << "3891" << endl;
			if(!to_var->uncertainty(&unc_rel).empty()) au.setUncertainty(to_var->uncertainty(), unc_rel);
			au.setApproximate(to_var->isApproximate());std::cout << "3893" << endl;
			au.setPrecision(to_var->precision());std::cout << "3894" << endl;
			MathStructure mstruct_new(convert(mstruct, &au, eo, false, false));std::cout << "3895" << endl;
			if(mstruct_new.contains(&au)) {
				mstruct_new.replace(&au, to_var);std::cout << "3897" << endl;
				return mstruct_new;
			}
		}
	}
	MathStructure mstruct_new(mstruct);std::cout << "3902" << endl;
	mstruct_new /= to_var->get();std::cout << "3903" << endl;
	mstruct_new.eval(eo);std::cout << "3904" << endl;
	mstruct_new *= to_var;std::cout << "3905" << endl;
	cleanMessages(mstruct, n_messages + 1);std::cout << "3906" << endl;
	return mstruct_new;
}
MathStructure Calculator::convert(const MathStructure &mstruct, Unit *to_unit, const EvaluationOptions &eo, bool always_convert, bool convert_to_mixed_units) {
	if(!mstruct.containsType(STRUCT_UNIT, true)) return mstruct;
	CompositeUnit *cu = NULL;std::cout << "3911" << endl;
	if(to_unit->subtype() == SUBTYPE_COMPOSITE_UNIT) cu = (CompositeUnit*) to_unit;
	if(cu && cu->countUnits() == 0) return mstruct;
	MathStructure mstruct_new(mstruct);std::cout << "3914" << endl;
	size_t n_messages = messages.size();std::cout << "3915" << endl;
	if(to_unit->hasNonlinearRelationTo(to_unit->baseUnit()) && to_unit->baseUnit()->subtype() == SUBTYPE_COMPOSITE_UNIT) {
		mstruct_new = convert(mstruct, to_unit->baseUnit(), eo, always_convert, convert_to_mixed_units);std::cout << "3917" << endl;
		mstruct_new.calculateDivide(((CompositeUnit*) to_unit->baseUnit())->generateMathStructure(false, eo.keep_prefixes), eo);std::cout << "3918" << endl;
		to_unit->convertFromBaseUnit(mstruct_new);std::cout << "3919" << endl;
		mstruct_new.eval(eo);std::cout << "3920" << endl;
		mstruct_new.multiply(MathStructure(to_unit, eo.keep_prefixes ? decimal_null_prefix : NULL));std::cout << "3921" << endl;
		EvaluationOptions eo2 = eo;std::cout << "3922" << endl;
		eo2.sync_units = false;std::cout << "3923" << endl;
		eo2.keep_prefixes = true;std::cout << "3924" << endl;
		mstruct_new.eval(eo2);std::cout << "3925" << endl;
		cleanMessages(mstruct, n_messages + 1);std::cout << "3926" << endl;
		return mstruct_new;
	}
	//bool b_simple = !cu && (to_unit->subtype() != SUBTYPE_ALIAS_UNIT || (((AliasUnit*) to_unit)->baseUnit()->subtype() != SUBTYPE_COMPOSITE_UNIT && ((AliasUnit*) to_unit)->baseExponent() == 1));std::cout << "3929" << endl;

	bool b_changed = false;std::cout << "3931" << endl;
	if(mstruct_new.isAddition()) {
		if(aborted()) return mstruct;
		mstruct_new.factorizeUnits();std::cout << "3934" << endl;
		if(!b_changed && !mstruct_new.equals(mstruct, true, true)) b_changed = true;
	}

	if(!mstruct_new.isPower() && !mstruct_new.isUnit() && !mstruct_new.isMultiplication()) {
		if(mstruct_new.size() > 0) {
			for(size_t i = 0; i < mstruct_new.size(); i++) {
				if(aborted()) return mstruct;
				if(!mstruct_new.isFunction() || !mstruct_new.function()->getArgumentDefinition(i + 1) || mstruct_new.function()->getArgumentDefinition(i + 1)->type() != ARGUMENT_TYPE_ANGLE) {
					mstruct_new[i] = convert(mstruct_new[i], to_unit, eo, false, convert_to_mixed_units);std::cout << "3943" << endl;
					if(!b_changed && !mstruct_new.equals(mstruct[i], true, true)) b_changed = true;
				}
			}
			if(b_changed) {
				mstruct_new.childrenUpdated();std::cout << "3948" << endl;
				EvaluationOptions eo2 = eo;std::cout << "3949" << endl;
				//eo2.calculate_functions = false;std::cout << "3950" << endl;
				eo2.sync_units = false;std::cout << "3951" << endl;
				eo2.keep_prefixes = true;std::cout << "3952" << endl;
				mstruct_new.eval(eo2);std::cout << "3953" << endl;
				cleanMessages(mstruct, n_messages + 1);std::cout << "3954" << endl;
			}
			return mstruct_new;
		}
	} else {
		EvaluationOptions eo2 = eo;std::cout << "3959" << endl;
		eo2.keep_prefixes = true;std::cout << "3960" << endl;
		bool b = false;std::cout << "3961" << endl;
		if(eo.approximation == APPROXIMATION_EXACT) eo2.approximation = APPROXIMATION_TRY_EXACT;
		if(mstruct_new.convert(to_unit, true, NULL, false, eo2, eo.keep_prefixes ? decimal_null_prefix : NULL) || always_convert) {
			b = true;std::cout << "3964" << endl;
		} else {
			CompositeUnit *cu2 = cu;std::cout << "3966" << endl;
			if(to_unit->subtype() == SUBTYPE_ALIAS_UNIT && ((AliasUnit*) to_unit)->baseUnit()->subtype() == SUBTYPE_COMPOSITE_UNIT) {
				cu2 = (CompositeUnit*) ((AliasUnit*) to_unit)->baseUnit();std::cout << "3968" << endl;
			}
			if(cu2) {
				switch(mstruct_new.type()) {
					case STRUCT_UNIT: {
						if(cu2->containsRelativeTo(mstruct_new.unit())) {
							b = true;std::cout << "3974" << endl;
						}
						break;
					}
					case STRUCT_MULTIPLICATION: {
						for(size_t i = 1; i <= mstruct_new.countChildren(); i++) {
							if(aborted()) return mstruct;
							if(mstruct_new.getChild(i)->isUnit() && cu2->containsRelativeTo(mstruct_new.getChild(i)->unit())) {
								b = true;std::cout << "3982" << endl;
							}
							if(mstruct_new.getChild(i)->isPower() && mstruct_new.getChild(i)->base()->isUnit() && cu2->containsRelativeTo(mstruct_new.getChild(i)->base()->unit())) {
								b = true;std::cout << "3985" << endl;
							}
						}
						break;
					}
					case STRUCT_POWER: {
						if(mstruct_new.base()->isUnit() && cu2->containsRelativeTo(mstruct_new.base()->unit())) {
							b = true;std::cout << "3992" << endl;
						}
						break;
					}
					default: {}
				}
			}
		}
		if(b) {
			eo2.approximation = eo.approximation;std::cout << "4001" << endl;
			eo2.sync_units = true;std::cout << "4002" << endl;
			eo2.keep_prefixes = false;std::cout << "4003" << endl;
			MathStructure mbak(mstruct_new);std::cout << "4004" << endl;
			mstruct_new.divide(MathStructure(to_unit, NULL));std::cout << "4005" << endl;
			mstruct_new.eval(eo2);std::cout << "4006" << endl;
			if(mstruct_new.containsType(STRUCT_UNIT)) {
				mbak.inverse();std::cout << "4008" << endl;
				mbak.divide(MathStructure(to_unit, NULL));std::cout << "4009" << endl;
				mbak.eval(eo2);std::cout << "4010" << endl;
				if(!mbak.containsType(STRUCT_UNIT)) mstruct_new = mbak;
			}

			if(cu) {
				MathStructure mstruct_cu(cu->generateMathStructure(false, eo.keep_prefixes));std::cout << "4015" << endl;
				Prefix *p = NULL;std::cout << "4016" << endl;
				size_t i = 1;std::cout << "4017" << endl;
				Unit *u = cu->get(i, NULL, &p);std::cout << "4018" << endl;
				while(u) {
					mstruct_new.setPrefixForUnit(u, p);std::cout << "4020" << endl;
					i++;std::cout << "4021" << endl;
					u = cu->get(i, NULL, &p);std::cout << "4022" << endl;
				}
				mstruct_new.multiply(mstruct_cu);std::cout << "4024" << endl;
			} else {
				mstruct_new.multiply(MathStructure(to_unit, eo.keep_prefixes ? decimal_null_prefix : NULL));std::cout << "4026" << endl;
			}

			eo2.sync_units = false;std::cout << "4029" << endl;
			eo2.keep_prefixes = true;std::cout << "4030" << endl;
			mstruct_new.eval(eo2);std::cout << "4031" << endl;

			cleanMessages(mstruct, n_messages + 1);std::cout << "4033" << endl;

			if(convert_to_mixed_units && eo2.mixed_units_conversion != MIXED_UNITS_CONVERSION_NONE) {
				eo2.mixed_units_conversion = MIXED_UNITS_CONVERSION_DOWNWARDS_KEEP;std::cout << "4036" << endl;
				return convertToMixedUnits(mstruct_new, eo2);
			} else {
				return mstruct_new;
			}
		}
	}

	return mstruct;

}
MathStructure Calculator::convertToBaseUnits(const MathStructure &mstruct, const EvaluationOptions &eo) {
	if(!mstruct.containsType(STRUCT_UNIT, true)) return mstruct;
	size_t n_messages = messages.size();std::cout << "4049" << endl;
	MathStructure mstruct_new(mstruct);std::cout << "4050" << endl;
	mstruct_new.convertToBaseUnits(true, NULL, true, eo);std::cout << "4051" << endl;
	if(!mstruct_new.equals(mstruct, true, true)) {
		EvaluationOptions eo2 = eo;std::cout << "4053" << endl;
		eo2.approximation = eo.approximation;std::cout << "4054" << endl;
		eo2.keep_prefixes = false;std::cout << "4055" << endl;
		eo2.isolate_x = false;std::cout << "4056" << endl;
		eo2.test_comparisons = false;std::cout << "4057" << endl;
		//eo2.calculate_functions = false;std::cout << "4058" << endl;
		mstruct_new.eval(eo2);std::cout << "4059" << endl;
		cleanMessages(mstruct, n_messages + 1);std::cout << "4060" << endl;
	}
	return mstruct_new;
}
Unit *Calculator::findMatchingUnit(const MathStructure &mstruct) {
	switch(mstruct.type()) {
		case STRUCT_POWER: {
			if(mstruct.base()->isUnit() && mstruct.exponent()->isNumber() && mstruct.exponent()->number().isInteger() && mstruct.exponent()->number() < 10 && mstruct.exponent()->number() > -10) {
				Unit *u_base = mstruct.base()->unit();std::cout << "4068" << endl;
				int exp = mstruct.exponent()->number().intValue();std::cout << "4069" << endl;
				if(u_base->subtype() == SUBTYPE_ALIAS_UNIT) {
					u_base = u_base->baseUnit();std::cout << "4071" << endl;
					exp *= ((AliasUnit*) u_base)->baseExponent();std::cout << "4072" << endl;
				}
				for(size_t i = 0; i < units.size(); i++) {
					Unit *u = units[i];std::cout << "4075" << endl;
					if(u->subtype() == SUBTYPE_ALIAS_UNIT && u->baseUnit() == u_base && ((AliasUnit*) u)->baseExponent() == exp) {
						return u;
					}
				}
				CompositeUnit *cu = new CompositeUnit("", "temporary_find_matching_unit");
				cu->add(u_base, exp);std::cout << "4081" << endl;
				Unit *u = getOptimalUnit(cu);std::cout << "4082" << endl;
				if(u != cu && !u->isRegistered()) {
					delete u;std::cout << "4084" << endl;
				} else if(u != cu) {
					MathStructure mtest(mstruct);std::cout << "4086" << endl;
					mtest.divide(u);std::cout << "4087" << endl;
					mtest.eval();std::cout << "4088" << endl;
					if(mtest.isNumber()) {
						delete cu;std::cout << "4090" << endl;
						return u;
					}
				}
				delete cu;std::cout << "4094" << endl;
			}
			return findMatchingUnit(mstruct[0]);
		}
		case STRUCT_UNIT: {
			return mstruct.unit();
		}
		case STRUCT_MULTIPLICATION: {
			if(mstruct.size() == 2 && !mstruct[0].isUnit_exp()) {
				return findMatchingUnit(mstruct[1]);
			}
			CompositeUnit *cu = new CompositeUnit("", "temporary_find_matching_unit");
			for(size_t i = 1; i <= mstruct.countChildren(); i++) {
				if(mstruct.getChild(i)->isUnit()) {
					cu->add(mstruct.getChild(i)->unit()->baseUnit());std::cout << "4108" << endl;
				} else if(mstruct.getChild(i)->isPower() && mstruct.getChild(i)->base()->isUnit() && mstruct.getChild(i)->exponent()->isNumber() && mstruct.getChild(i)->exponent()->number().isInteger()) {
					cu->add(mstruct.getChild(i)->base()->unit()->baseUnit(), mstruct.getChild(i)->exponent()->number().intValue());std::cout << "4110" << endl;
				}
			}
			if(cu->countUnits() == 1) {
				int exp = 1;std::cout << "4114" << endl;
				Unit *u_base = cu->get(1, &exp);std::cout << "4115" << endl;
				if(exp == 1) return u_base;
				for(size_t i = 0; i < units.size(); i++) {
					Unit *u = units[i];std::cout << "4118" << endl;
					if(u->subtype() == SUBTYPE_ALIAS_UNIT && u->baseUnit() == u_base && ((AliasUnit*) u)->baseExponent() == exp) {
						return u;
					}
				}
			}
			if(cu->countUnits() > 1) {
				for(size_t i = 0; i < units.size(); i++) {
					Unit *u = units[i];std::cout << "4126" << endl;
					if(u->subtype() == SUBTYPE_COMPOSITE_UNIT) {
						if(((CompositeUnit*) u)->countUnits() == cu->countUnits()) {
							bool b = true;std::cout << "4129" << endl;
							for(size_t i2 = 1; i2 <= cu->countUnits(); i2++) {
								int exp1 = 1, exp2 = 1;std::cout << "4131" << endl;
								Unit *ui1 = cu->get(i2, &exp1);std::cout << "4132" << endl;
								b = false;std::cout << "4133" << endl;
								for(size_t i3 = 1; i3 <= cu->countUnits(); i3++) {
									Unit *ui2 = ((CompositeUnit*) u)->get(i3, &exp2);std::cout << "4135" << endl;
									if(ui1 == ui2->baseUnit()) {
										b = (exp1 == exp2);std::cout << "4137" << endl;
										break;
									}
								}
								if(!b) break;
							}
							if(b) {
								delete cu;std::cout << "4144" << endl;
								return u;
							}
						}
					}
				}
			}
			Unit *u = getOptimalUnit(cu);std::cout << "4151" << endl;
			if(u != cu && !u->isRegistered()) {
				if(cu->countUnits() > 1 && u->subtype() == SUBTYPE_COMPOSITE_UNIT) {
					MathStructure m_u = ((CompositeUnit*) u)->generateMathStructure();std::cout << "4154" << endl;
					if(m_u != cu->generateMathStructure()) {
						Unit *u2 = findMatchingUnit(m_u);std::cout << "4156" << endl;
						if(u2) {
							MathStructure mtest(mstruct);std::cout << "4158" << endl;
							mtest.divide(u2);std::cout << "4159" << endl;
							mtest.eval();std::cout << "4160" << endl;
							if(mtest.isNumber()) {
								delete cu;std::cout << "4162" << endl;
								delete u;std::cout << "4163" << endl;
								return u2;
							}
						}
					}
				}
				delete u;std::cout << "4169" << endl;
			} else if(u != cu) {
				MathStructure mtest(mstruct);std::cout << "4171" << endl;
				mtest.divide(u);std::cout << "4172" << endl;
				mtest.eval();std::cout << "4173" << endl;
				if(mtest.isNumber()) {
					delete cu;std::cout << "4175" << endl;
					return u;
				}
			}
			delete cu;std::cout << "4179" << endl;
			break;
		}
		default: {
			for(size_t i = 0; i < mstruct.size(); i++) {
				if(aborted()) return NULL;
				if(!mstruct.isFunction() || !mstruct.function()->getArgumentDefinition(i + 1) || mstruct.function()->getArgumentDefinition(i + 1)->type() != ARGUMENT_TYPE_ANGLE) {
					Unit *u = findMatchingUnit(mstruct[i]);std::cout << "4186" << endl;
					if(u) return u;
				}
			}
			break;
		}
	}
	return NULL;
}
Unit *Calculator::getBestUnit(Unit *u, bool allow_only_div, bool convert_to_local_currency) {return getOptimalUnit(u, allow_only_div, convert_to_local_currency);}
Unit *Calculator::getOptimalUnit(Unit *u, bool allow_only_div, bool convert_to_local_currency) {
	switch(u->subtype()) {
		case SUBTYPE_BASE_UNIT: {
			if(convert_to_local_currency && u->isCurrency()) {
				Unit *u_local_currency = getLocalCurrency();std::cout << "4200" << endl;
				if(u_local_currency) return u_local_currency;
			}
			return u;
		}
		case SUBTYPE_ALIAS_UNIT: {
			AliasUnit *au = (AliasUnit*) u;std::cout << "4206" << endl;
			if(au->baseExponent() == 1 && au->baseUnit()->subtype() == SUBTYPE_BASE_UNIT) {
				if(au->isCurrency()) {
					if(!convert_to_local_currency) return u;
					Unit *u_local_currency = getLocalCurrency();std::cout << "4210" << endl;
					if(u_local_currency) return u_local_currency;
				}
				return (Unit*) au->baseUnit();
			} else if(au->isSIUnit() && (au->firstBaseUnit()->subtype() == SUBTYPE_COMPOSITE_UNIT || au->firstBaseExponent() != 1)) {
				return u;
			} else {
				return getOptimalUnit((Unit*) au->firstBaseUnit());
			}
		}
		case SUBTYPE_COMPOSITE_UNIT: {
			CompositeUnit *cu = (CompositeUnit*) u;std::cout << "4221" << endl;
			int exp, b_exp;std::cout << "4222" << endl;
			int points = 0;std::cout << "4223" << endl;
			bool minus = false;std::cout << "4224" << endl;
			bool has_positive = false;std::cout << "4225" << endl;
			int new_points;std::cout << "4226" << endl;
			int new_points_m;std::cout << "4227" << endl;
			int max_points = 0;std::cout << "4228" << endl;
			for(size_t i = 1; i <= cu->countUnits(); i++) {
				cu->get(i, &exp);std::cout << "4230" << endl;
				if(exp < 0) {
					max_points -= exp;std::cout << "4232" << endl;
				} else {
					max_points += exp;std::cout << "4234" << endl;
					has_positive = true;std::cout << "4235" << endl;
				}
			}
			for(size_t i = 0; i < units.size(); i++) {
				if(units[i]->subtype() == SUBTYPE_COMPOSITE_UNIT) {
					CompositeUnit *cu2 = (CompositeUnit*) units[i];std::cout << "4240" << endl;
					if(cu == cu2 && !cu2->isHidden()) {
						points = max_points - 1;std::cout << "4242" << endl;
					} else if(!cu2->isHidden() && cu2->isSIUnit() && cu2->countUnits() == cu->countUnits()) {
						bool b_match = true;std::cout << "4244" << endl;
						for(size_t i2 = 1; i2 <= cu->countUnits(); i2++) {
							int exp2;std::cout << "4246" << endl;
							if(cu->get(i2, &exp) != cu2->get(i2, &exp2) || exp != exp2) {
								b_match = false;std::cout << "4248" << endl;
								break;
							}
						}
						if(b_match) {
							points = max_points - 1;std::cout << "4253" << endl;
							break;
						}
					}
				}
			}
			Unit *best_u = NULL;std::cout << "4259" << endl;
			Unit *bu, *u2;std::cout << "4260" << endl;
			AliasUnit *au;std::cout << "4261" << endl;
			for(size_t i = 0; i < units.size(); i++) {
				u2 = units[i];std::cout << "4263" << endl;
				if(u2->subtype() == SUBTYPE_BASE_UNIT && (points == 0 || (points == 1 && minus))) {
					for(size_t i2 = 1; i2 <= cu->countUnits(); i2++) {
						if(cu->get(i2, &exp)->baseUnit() == u2 && !cu->get(i2)->hasNonlinearRelationTo(u2)) {
							points = 1;std::cout << "4267" << endl;
							best_u = u2;std::cout << "4268" << endl;
							minus = !has_positive && (exp < 0);std::cout << "4269" << endl;
							break;
						}
					}
				} else if(!u2->isSIUnit()) {
				} else if(u2->subtype() == SUBTYPE_ALIAS_UNIT) {
					au = (AliasUnit*) u2;std::cout << "4275" << endl;
					bu = (Unit*) au->baseUnit();std::cout << "4276" << endl;
					b_exp = au->baseExponent();std::cout << "4277" << endl;
					new_points = 0;std::cout << "4278" << endl;
					new_points_m = 0;std::cout << "4279" << endl;
					if((b_exp != 1 || bu->subtype() == SUBTYPE_COMPOSITE_UNIT) && !au->hasNonlinearRelationTo(bu)) {
						if(bu->subtype() == SUBTYPE_BASE_UNIT) {
							for(size_t i2 = 1; i2 <= cu->countUnits(); i2++) {
								if(cu->get(i2, &exp) == bu) {
									bool m = false;std::cout << "4284" << endl;
									if(b_exp < 0 && exp < 0) {
										b_exp = -b_exp;std::cout << "4286" << endl;
										exp = -exp;std::cout << "4287" << endl;
									} else if(b_exp < 0) {
										b_exp = -b_exp;std::cout << "4289" << endl;
										m = true;std::cout << "4290" << endl;
									} else if(exp < 0) {
										exp = -exp;std::cout << "4292" << endl;
										m = true;std::cout << "4293" << endl;
									}
									new_points = exp - b_exp;std::cout << "4295" << endl;
									if(new_points < 0) {
										new_points = -new_points;std::cout << "4297" << endl;
									}
									new_points = exp - new_points;std::cout << "4299" << endl;
									if(!allow_only_div && m && new_points >= max_points) {
										new_points = -1;std::cout << "4301" << endl;
									}
									if(new_points > points || (!m && minus && new_points == points)) {
										points = new_points;std::cout << "4304" << endl;
										minus = m;std::cout << "4305" << endl;
										best_u = au;std::cout << "4306" << endl;
									}
									break;
								}
							}
						} else if(au->firstBaseExponent() != 1 || au->firstBaseUnit()->subtype() == SUBTYPE_COMPOSITE_UNIT) {
							MathStructure cu_mstruct = ((CompositeUnit*) bu)->generateMathStructure();std::cout << "4312" << endl;
							if(b_exp != 1) {
								if(cu_mstruct.isMultiplication()) {
									for(size_t i2 = 0; i2 < cu_mstruct.size(); i2++) {
										if(cu_mstruct[i2].isPower()) cu_mstruct[i2][1].number() *= b_exp;
										else cu_mstruct[i2].raise(b_exp);std::cout << "4317" << endl;
									}
								} else if(cu_mstruct.isPower()) {
									cu_mstruct[1].number() *= b_exp;std::cout << "4320" << endl;
								} else {
									cu_mstruct.raise(b_exp);std::cout << "4322" << endl;
								}
							}
							cu_mstruct = convertToBaseUnits(cu_mstruct);std::cout << "4325" << endl;
							if(cu_mstruct.isMultiplication()) {
								for(size_t i2 = 1; i2 <= cu_mstruct.countChildren(); i2++) {
									bu = NULL;std::cout << "4328" << endl;
									if(cu_mstruct.getChild(i2)->isUnit()) {
										bu = cu_mstruct.getChild(i2)->unit();std::cout << "4330" << endl;
										b_exp = 1;std::cout << "4331" << endl;
									} else if(cu_mstruct.getChild(i2)->isPower() && cu_mstruct.getChild(i2)->base()->isUnit() && cu_mstruct.getChild(i2)->exponent()->isNumber() && cu_mstruct.getChild(i2)->exponent()->number().isInteger()) {
										bu = cu_mstruct.getChild(i2)->base()->unit();std::cout << "4333" << endl;
										b_exp = cu_mstruct.getChild(i2)->exponent()->number().intValue();std::cout << "4334" << endl;
									}
									if(bu) {
										bool b = false;std::cout << "4337" << endl;
										for(size_t i3 = 1; i3 <= cu->countUnits(); i3++) {
											if(cu->get(i3, &exp) == bu) {
												b = true;std::cout << "4340" << endl;
												bool m = false;std::cout << "4341" << endl;
												if(exp < 0 && b_exp > 0) {
													new_points -= b_exp;std::cout << "4343" << endl;
													exp = -exp;std::cout << "4344" << endl;
													m = true;std::cout << "4345" << endl;
												} else if(exp > 0 && b_exp < 0) {
													new_points += b_exp;std::cout << "4347" << endl;
													b_exp = -b_exp;std::cout << "4348" << endl;
													m = true;std::cout << "4349" << endl;
												} else {
													if(b_exp < 0) new_points_m += b_exp;
													else new_points_m -= b_exp;std::cout << "4352" << endl;
												}
												if(exp < 0) {
													exp = -exp;std::cout << "4355" << endl;
													b_exp = -b_exp;std::cout << "4356" << endl;
												}
												if(exp >= b_exp) {
													if(m) new_points_m += exp - (exp - b_exp);
													else new_points += exp - (exp - b_exp);std::cout << "4360" << endl;
												} else {
													if(m) new_points_m += exp - (b_exp - exp);
													else new_points += exp - (b_exp - exp);std::cout << "4363" << endl;
												}
												break;
											}
										}
										if(!b) {
											if(b_exp < 0) b_exp = -b_exp;
											new_points -= b_exp;std::cout << "4370" << endl;
											new_points_m -= b_exp;std::cout << "4371" << endl;
										}
									}
								}
								if(!allow_only_div && new_points_m >= max_points) {
									new_points_m = -1;std::cout << "4376" << endl;
								}
								if(new_points > points && new_points >= new_points_m) {
									minus = false;std::cout << "4379" << endl;
									points = new_points;std::cout << "4380" << endl;
									best_u = au;std::cout << "4381" << endl;
								} else if(new_points_m > points || (new_points_m == points && minus)) {
									minus = true;std::cout << "4383" << endl;
									points = new_points_m;std::cout << "4384" << endl;
									best_u = au;std::cout << "4385" << endl;
								}
							}
						}
					}
				}
				if(points >= max_points && !minus) break;
			}
			if(!best_u) return u;
			best_u = getOptimalUnit(best_u, false, convert_to_local_currency);std::cout << "4394" << endl;
			if(points > 1 && points < max_points - 1) {
				CompositeUnit *cu_new = new CompositeUnit("", "temporary_composite_convert");
				bool return_cu = minus;
				if(minus) {
					cu_new->add(best_u, -1);std::cout << "4399" << endl;
				} else {
					cu_new->add(best_u);std::cout << "4401" << endl;
				}
				MathStructure cu_mstruct = ((CompositeUnit*) u)->generateMathStructure();std::cout << "4403" << endl;
				if(minus) cu_mstruct *= best_u;
				else cu_mstruct /= best_u;std::cout << "4405" << endl;
				cu_mstruct = convertToBaseUnits(cu_mstruct);std::cout << "4406" << endl;
				CompositeUnit *cu2 = new CompositeUnit("", "temporary_composite_convert_to_optimal_unit");
				bool b = false;std::cout << "4408" << endl;
				for(size_t i = 1; i <= cu_mstruct.countChildren(); i++) {
					if(cu_mstruct.getChild(i)->isUnit()) {
						b = true;std::cout << "4411" << endl;
						cu2->add(cu_mstruct.getChild(i)->unit());std::cout << "4412" << endl;
					} else if(cu_mstruct.getChild(i)->isPower() && cu_mstruct.getChild(i)->base()->isUnit() && cu_mstruct.getChild(i)->exponent()->isNumber() && cu_mstruct.getChild(i)->exponent()->number().isInteger()) {
						if(cu_mstruct.getChild(i)->exponent()->number().isGreaterThan(10) || cu_mstruct.getChild(i)->exponent()->number().isLessThan(-10)) {
							if(aborted() || cu_mstruct.getChild(i)->exponent()->number().isGreaterThan(1000) || cu_mstruct.getChild(i)->exponent()->number().isLessThan(-1000)) {
								b = false;std::cout << "4416" << endl;
								break;
							}
						}
						b = true;std::cout << "4420" << endl;
						cu2->add(cu_mstruct.getChild(i)->base()->unit(), cu_mstruct.getChild(i)->exponent()->number().intValue());std::cout << "4421" << endl;
					}
				}
				if(b) {
					Unit *u2 = getOptimalUnit(cu2, true, convert_to_local_currency);std::cout << "4425" << endl;
					b = false;std::cout << "4426" << endl;
					if(u2->subtype() == SUBTYPE_COMPOSITE_UNIT) {
						for(size_t i3 = 1; i3 <= ((CompositeUnit*) u2)->countUnits(); i3++) {
							Unit *cu_unit = ((CompositeUnit*) u2)->get(i3, &exp);std::cout << "4429" << endl;
							for(size_t i4 = 1; i4 <= cu_new->countUnits(); i4++) {
								if(cu_new->get(i4, &b_exp) == cu_unit) {
									b = true;std::cout << "4432" << endl;
									cu_new->setExponent(i4, b_exp + exp);std::cout << "4433" << endl;
									break;
								}
							}
							if(!b) cu_new->add(cu_unit, exp);
						}
						return_cu = true;
					} else if(u2->subtype() == SUBTYPE_ALIAS_UNIT) {
						return_cu = true;
						for(size_t i3 = 1; i3 <= cu_new->countUnits(); i3++) {
							if(cu_new->get(i3, &exp) == u2) {
								b = true;std::cout << "4444" << endl;
								cu_new->setExponent(i3, exp + 1);std::cout << "4445" << endl;
								break;
							}
						}
						if(!b) cu_new->add(u2);
					}
					if(!u2->isRegistered() && u2 != cu2) delete u2;
				}
				delete cu2;std::cout << "4453" << endl;
				if(return_cu) {
					return cu_new;
				} else {
					delete cu_new;std::cout << "4457" << endl;
					return best_u;
				}
			}
			if(minus) {
				CompositeUnit *cu_new = new CompositeUnit("", "temporary_composite_convert");
				cu_new->add(best_u, -1);std::cout << "4463" << endl;
				return cu_new;
			} else {
				return best_u;
			}
		}
	}
	return u;
}
MathStructure Calculator::convertToBestUnit(const MathStructure &mstruct, const EvaluationOptions &eo, bool convert_to_si_units) {return convertToOptimalUnit(mstruct, eo, convert_to_si_units);}
MathStructure Calculator::convertToOptimalUnit(const MathStructure &mstruct, const EvaluationOptions &eo, bool convert_to_si_units) {
	EvaluationOptions eo2 = eo;std::cout << "4474" << endl;
	//eo2.calculate_functions = false;std::cout << "4475" << endl;
	eo2.sync_units = false;std::cout << "4476" << endl;
	eo2.isolate_x = false;std::cout << "4477" << endl;
	eo2.test_comparisons = false;std::cout << "4478" << endl;
	switch(mstruct.type()) {
		case STRUCT_POWER: {
			if(mstruct.base()->isUnit() && mstruct.exponent()->isNumber() && mstruct.exponent()->number().isRational() && !mstruct.exponent()->number().isZero()) {
				MathStructure mstruct_new(mstruct);std::cout << "4482" << endl;
				int old_points = 0;std::cout << "4483" << endl;
				bool overflow = false;std::cout << "4484" << endl;
				if(mstruct_new.exponent()->isInteger()) old_points = mstruct_new.exponent()->number().intValue(&overflow);
				else old_points = mstruct_new.exponent()->number().numerator().intValue(&overflow) + mstruct_new.exponent()->number().denominator().intValue() * (mstruct_new.exponent()->number().isNegative() ? -1 : 1);std::cout << "4486" << endl;
				if(overflow) return mstruct_new;
				bool old_minus = false;std::cout << "4488" << endl;
				if(old_points < 0) {
					old_points = -old_points;std::cout << "4490" << endl;
					old_minus = true;std::cout << "4491" << endl;
				}
				bool is_si_units = mstruct_new.base()->unit()->isSIUnit();std::cout << "4493" << endl;
				if(mstruct_new.base()->unit()->baseUnit()->subtype() == SUBTYPE_COMPOSITE_UNIT) {
					mstruct_new.convertToBaseUnits(true, NULL, true, eo2, true);std::cout << "4495" << endl;
					if(mstruct_new.equals(mstruct, true, true)) {
						return mstruct_new;
					} else {
						mstruct_new.eval(eo2);std::cout << "4499" << endl;
					}
					mstruct_new = convertToOptimalUnit(mstruct_new, eo, convert_to_si_units);std::cout << "4501" << endl;
					if(mstruct_new.equals(mstruct, true, true)) return mstruct_new;
				} else {
					CompositeUnit *cu = new CompositeUnit("", "temporary_composite_convert_to_optimal_unit");
					cu->add(mstruct_new.base()->unit(), mstruct_new.exponent()->number().numerator().intValue());std::cout << "4505" << endl;
					Unit *u = getOptimalUnit(cu, false, eo.local_currency_conversion);std::cout << "4506" << endl;
					if(u == cu) {
						delete cu;std::cout << "4508" << endl;
						return mstruct_new;
					}
					if(eo.approximation == APPROXIMATION_EXACT && cu->hasApproximateRelationTo(u, true)) {
						if(!u->isRegistered()) delete u;
						delete cu;std::cout << "4513" << endl;
						return mstruct_new;
					}
					delete cu;std::cout << "4516" << endl;
					mstruct_new = convert(mstruct_new, u, eo, true);std::cout << "4517" << endl;
					if(!u->isRegistered()) delete u;
				}
				int new_points = 0;std::cout << "4520" << endl;
				bool new_is_si_units = true;std::cout << "4521" << endl;
				bool new_minus = true;std::cout << "4522" << endl;
				bool is_currency = false;std::cout << "4523" << endl;
				if(mstruct_new.isMultiplication()) {
					for(size_t i = 1; i <= mstruct_new.countChildren(); i++) {
						if(mstruct_new.getChild(i)->isUnit()) {
							if(new_is_si_units && !mstruct_new.getChild(i)->unit()->isSIUnit()) new_is_si_units = false;
							is_currency = mstruct_new.getChild(i)->unit()->isCurrency();std::cout << "4528" << endl;
							new_points++;std::cout << "4529" << endl;
							new_minus = false;std::cout << "4530" << endl;
						} else if(mstruct_new.getChild(i)->isPower() && mstruct_new.getChild(i)->base()->isUnit() && mstruct_new.getChild(i)->exponent()->isNumber() && mstruct_new.getChild(i)->exponent()->number().isRational()) {
							int points = 0;std::cout << "4532" << endl;
							if(mstruct_new.getChild(i)->exponent()->isInteger()) points = mstruct_new.getChild(i)->exponent()->number().intValue();
							else points = mstruct_new.getChild(i)->exponent()->number().numerator().intValue() + mstruct_new.getChild(i)->exponent()->number().denominator().intValue() * (mstruct_new.getChild(i)->exponent()->number().isNegative() ? -1 : 1);std::cout << "4534" << endl;
							if(new_is_si_units && !mstruct_new.getChild(i)->base()->unit()->isSIUnit()) new_is_si_units = false;
							is_currency = mstruct_new.getChild(i)->base()->unit()->isCurrency();std::cout << "4536" << endl;
							if(points < 0) {
								new_points -= points;std::cout << "4538" << endl;
							} else {
								new_points += points;std::cout << "4540" << endl;
								new_minus = false;std::cout << "4541" << endl;
							}

						}
					}
				} else if(mstruct_new.isPower() && mstruct_new.base()->isUnit() && mstruct_new.exponent()->isNumber() && mstruct_new.exponent()->number().isRational()) {
					int points = 0;std::cout << "4547" << endl;
					if(mstruct_new.exponent()->isInteger()) points = mstruct_new.exponent()->number().intValue();
					else points = mstruct_new.exponent()->number().numerator().intValue() + mstruct_new.exponent()->number().denominator().intValue() * (mstruct_new.exponent()->number().isNegative() ? -1 : 1);std::cout << "4549" << endl;
					if(new_is_si_units && !mstruct_new.base()->unit()->isSIUnit()) new_is_si_units = false;
					is_currency = mstruct_new.base()->unit()->isCurrency();std::cout << "4551" << endl;
					if(points < 0) {
						new_points = -points;std::cout << "4553" << endl;
					} else {
						new_points = points;std::cout << "4555" << endl;
						new_minus = false;std::cout << "4556" << endl;
					}
				} else if(mstruct_new.isUnit()) {
					if(!mstruct_new.unit()->isSIUnit()) new_is_si_units = false;
					is_currency = mstruct_new.unit()->isCurrency();std::cout << "4560" << endl;
					new_points = 1;std::cout << "4561" << endl;
					new_minus = false;std::cout << "4562" << endl;
				}
				if(new_points == 0) return mstruct;
				if((new_points > old_points && (!convert_to_si_units || is_si_units || !new_is_si_units)) || (new_points == old_points && (new_minus || !old_minus) && (!is_currency || !eo.local_currency_conversion) && (!convert_to_si_units || !new_is_si_units))) return mstruct;
				return mstruct_new;
			}
		}
		case STRUCT_BITWISE_XOR: {}
		case STRUCT_BITWISE_OR: {}
		case STRUCT_BITWISE_AND: {}
		case STRUCT_BITWISE_NOT: {}
		case STRUCT_LOGICAL_XOR: {}
		case STRUCT_LOGICAL_OR: {}
		case STRUCT_LOGICAL_AND: {}
		case STRUCT_LOGICAL_NOT: {}
		case STRUCT_COMPARISON: {}
		case STRUCT_FUNCTION: {}
		case STRUCT_VECTOR: {}
		case STRUCT_ADDITION: {
			if(!mstruct.containsType(STRUCT_UNIT, true)) return mstruct;
			MathStructure mstruct_new(mstruct);std::cout << "4582" << endl;
			bool b = false;std::cout << "4583" << endl;
			for(size_t i = 0; i < mstruct_new.size(); i++) {
				if(aborted()) return mstruct;
				if(!mstruct_new.isFunction() || !mstruct_new.function()->getArgumentDefinition(i + 1) || mstruct_new.function()->getArgumentDefinition(i + 1)->type() != ARGUMENT_TYPE_ANGLE) {
					mstruct_new[i] = convertToOptimalUnit(mstruct_new[i], eo, convert_to_si_units);std::cout << "4587" << endl;
					if(!b && !mstruct_new[i].equals(mstruct[i], true, true)) b = true;
				}
			}
			if(b) {
				mstruct_new.childrenUpdated();std::cout << "4592" << endl;
				if(mstruct.isAddition()) mstruct_new.eval(eo2);
			}
			return mstruct_new;
		}
		case STRUCT_UNIT: {
			if((!mstruct.unit()->isCurrency() || !eo.local_currency_conversion) && (!convert_to_si_units || mstruct.unit()->isSIUnit())) return mstruct;
			Unit *u = getOptimalUnit(mstruct.unit(), false, eo.local_currency_conversion);std::cout << "4599" << endl;
			if(u != mstruct.unit()) {
				if((u->isSIUnit() || (u->isCurrency() && eo.local_currency_conversion)) && (eo.approximation != APPROXIMATION_EXACT || !mstruct.unit()->hasApproximateRelationTo(u, true))) {
					MathStructure mstruct_new = convert(mstruct, u, eo, true);std::cout << "4602" << endl;
					if(!u->isRegistered()) delete u;
					return mstruct_new;
				}
				if(!u->isRegistered()) delete u;
			}
			break;
		}
		case STRUCT_MULTIPLICATION: {
			if(!mstruct.containsType(STRUCT_UNIT, true)) return mstruct;
			int old_points = 0;std::cout << "4612" << endl;
			bool old_minus = true;std::cout << "4613" << endl;
			bool is_si_units = true;std::cout << "4614" << endl;
			bool is_currency = false;std::cout << "4615" << endl;
			bool child_updated = false;std::cout << "4616" << endl;
			MathStructure mstruct_old(mstruct);std::cout << "4617" << endl;
			for(size_t i = 1; i <= mstruct_old.countChildren(); i++) {
				if(aborted()) return mstruct_old;
				if(mstruct_old.getChild(i)->isUnit()) {
					if(is_si_units && !mstruct_old.getChild(i)->unit()->isSIUnit()) is_si_units = false;
					is_currency = mstruct_old.getChild(i)->unit()->isCurrency();std::cout << "4622" << endl;
					old_points++;std::cout << "4623" << endl;
					old_minus = false;std::cout << "4624" << endl;
				} else if(mstruct_old.getChild(i)->isPower() && mstruct_old.getChild(i)->base()->isUnit() && mstruct_old.getChild(i)->exponent()->isNumber() && mstruct_old.getChild(i)->exponent()->number().isRational()) {
					int points = 0;std::cout << "4626" << endl;
					if(mstruct_old.getChild(i)->exponent()->number().isInteger()) points = mstruct_old.getChild(i)->exponent()->number().intValue();
					else points = mstruct_old.getChild(i)->exponent()->number().numerator().intValue() + mstruct_old.getChild(i)->exponent()->number().denominator().intValue() * (mstruct_old.getChild(i)->exponent()->number().isNegative() ? -1 : 1);std::cout << "4628" << endl;;std::cout << "4628" << endl;
					if(is_si_units && !mstruct_old.getChild(i)->base()->unit()->isSIUnit()) is_si_units = false;
						is_currency = mstruct_old.getChild(i)->base()->unit()->isCurrency();std::cout << "4630" << endl;
					if(points < 0) {
						old_points -= points;std::cout << "4632" << endl;
					} else {
						old_points += points;std::cout << "4634" << endl;
						old_minus = false;std::cout << "4635" << endl;
					}
				} else if(mstruct_old.getChild(i)->size() > 0 && !aborted()) {
					mstruct_old[i - 1] = convertToOptimalUnit(mstruct_old[i - 1], eo, convert_to_si_units);std::cout << "4638" << endl;
					mstruct_old.childUpdated(i);std::cout << "4639" << endl;
					if(!mstruct_old[i - 1].equals(mstruct[i - 1], true, true)) child_updated = true;
				}
			}
			if(child_updated) mstruct_old.eval(eo2);
			if((!is_currency || !eo.local_currency_conversion) && (!convert_to_si_units || is_si_units) && old_points <= 1 && !old_minus) {
				return mstruct_old;
			}
			MathStructure mstruct_new(mstruct_old);std::cout << "4647" << endl;
			mstruct_new.convertToBaseUnits(true, NULL, true, eo2, true);std::cout << "4648" << endl;
			if(!mstruct_new.equals(mstruct, true, true)) {
				mstruct_new.eval(eo2);std::cout << "4650" << endl;
			}
			if(mstruct_new.type() != STRUCT_MULTIPLICATION) {
				if(!mstruct_new.containsInterval(true, true, false, 1, true) && !aborted()) mstruct_new = convertToOptimalUnit(mstruct_new, eo, convert_to_si_units);
			} else {
				CompositeUnit *cu = new CompositeUnit("", "temporary_composite_convert_to_optimal_unit");
				bool b = false;std::cout << "4656" << endl;
				child_updated = false;std::cout << "4657" << endl;
				for(size_t i = 1; i <= mstruct_new.countChildren(); i++) {
					if(aborted()) return mstruct_old;
					if(mstruct_new.getChild(i)->isUnit()) {
						b = true;std::cout << "4661" << endl;
						cu->add(mstruct_new.getChild(i)->unit());std::cout << "4662" << endl;
					} else if(mstruct_new.getChild(i)->isPower() && mstruct_new.getChild(i)->base()->isUnit() && mstruct_new.getChild(i)->exponent()->isNumber() && mstruct_new.getChild(i)->exponent()->number().isInteger()) {
						b = true;std::cout << "4664" << endl;
						cu->add(mstruct_new.getChild(i)->base()->unit(), mstruct_new.getChild(i)->exponent()->number().intValue());std::cout << "4665" << endl;
					} else if(mstruct_new.getChild(i)->size() > 0 && !mstruct_new.getChild(i)->containsInterval(true, true, false, 1, true) && !aborted()) {
						MathStructure m_i_old(mstruct_new[i - 1]);std::cout << "4667" << endl;
						mstruct_new[i - 1] = convertToOptimalUnit(mstruct_new[i - 1], eo, convert_to_si_units);std::cout << "4668" << endl;
						mstruct_new.childUpdated(i);std::cout << "4669" << endl;
						if(!mstruct_new[i - 1].equals(m_i_old, true, true)) child_updated = true;
					}
				}
				bool is_converted = false;std::cout << "4673" << endl;
				if(b) {
					Unit *u = getOptimalUnit(cu, false, eo.local_currency_conversion);std::cout << "4675" << endl;
					if(u != cu) {
						if(eo.approximation != APPROXIMATION_EXACT || !cu->hasApproximateRelationTo(u, true)) {
							mstruct_new = convert(mstruct_new, u, eo, true);std::cout << "4678" << endl;
							is_converted = true;std::cout << "4679" << endl;
						}
						if(!u->isRegistered()) delete u;
					}
				}
				delete cu;std::cout << "4684" << endl;
				if((!b || !is_converted) && (!convert_to_si_units || is_si_units)) {
					return mstruct_old;
				}
				if(child_updated) mstruct_new.eval(eo2);
			}
			if((eo.approximation == APPROXIMATION_EXACT && !mstruct_old.isApproximate()) && (mstruct_new.isApproximate() || (mstruct_old.containsInterval(true, true, false, 0, true) <= 0 && mstruct_new.containsInterval(true, true, false, 0, true) > 0))) return mstruct_old;
			if(mstruct_new.equals(mstruct_old, true, true)) return mstruct_old;
			int new_points = 0;std::cout << "4692" << endl;
			bool new_minus = true;std::cout << "4693" << endl;
			bool new_is_si_units = true;std::cout << "4694" << endl;
			bool new_is_currency = false;std::cout << "4695" << endl;
			if(mstruct_new.isMultiplication()) {
				for(size_t i = 1; i <= mstruct_new.countChildren(); i++) {
					if(aborted()) return mstruct_old;
					if(mstruct_new.getChild(i)->isUnit()) {
						if(new_is_si_units && !mstruct_new.getChild(i)->unit()->isSIUnit()) new_is_si_units = false;
						new_is_currency = mstruct_new.getChild(i)->unit()->isCurrency();std::cout << "4701" << endl;
						new_points++;std::cout << "4702" << endl;
						new_minus = false;std::cout << "4703" << endl;
					} else if(mstruct_new.getChild(i)->isPower() && mstruct_new.getChild(i)->base()->isUnit() && mstruct_new.getChild(i)->exponent()->isNumber() && mstruct_new.getChild(i)->exponent()->number().isRational()) {
						int points = 0;std::cout << "4705" << endl;
						if(mstruct_new.getChild(i)->exponent()->number().isInteger()) points = mstruct_new.getChild(i)->exponent()->number().intValue();
						else points = mstruct_new.getChild(i)->exponent()->number().numerator().intValue() + mstruct_new.getChild(i)->exponent()->number().denominator().intValue() * (mstruct_new.getChild(i)->exponent()->number().isNegative() ? -1 : 1);std::cout << "4707" << endl;
						if(new_is_si_units && !mstruct_new.getChild(i)->base()->unit()->isSIUnit()) new_is_si_units = false;
						new_is_currency = mstruct_new.getChild(i)->base()->unit()->isCurrency();std::cout << "4709" << endl;
						if(points < 0) {
							new_points -= points;std::cout << "4711" << endl;
						} else {
							new_points += points;std::cout << "4713" << endl;
							new_minus = false;std::cout << "4714" << endl;
						}
					}
				}
			} else if(mstruct_new.isPower() && mstruct_new.base()->isUnit() && mstruct_new.exponent()->isNumber() && mstruct_new.exponent()->number().isRational()) {
				int points = 0;std::cout << "4719" << endl;
				if(mstruct_new.exponent()->number().isInteger()) points = mstruct_new.exponent()->number().intValue();
				else points = mstruct_new.exponent()->number().numerator().intValue() + mstruct_new.exponent()->number().denominator().intValue() * (mstruct_new.exponent()->number().isNegative() ? -1 : 1);std::cout << "4721" << endl;
				if(new_is_si_units && !mstruct_new.base()->unit()->isSIUnit()) new_is_si_units = false;
				new_is_currency = mstruct_new.base()->unit()->isCurrency();std::cout << "4723" << endl;
				if(points < 0) {
					new_points = -points;std::cout << "4725" << endl;
				} else {
					new_points = points;std::cout << "4727" << endl;
					new_minus = false;std::cout << "4728" << endl;
				}
			} else if(mstruct_new.isUnit()) {
				if(!mstruct_new.unit()->isSIUnit()) new_is_si_units = false;
				new_is_currency = mstruct_new.unit()->isCurrency();std::cout << "4732" << endl;
				new_points = 1;std::cout << "4733" << endl;
				new_minus = false;std::cout << "4734" << endl;
			}
			if(new_points == 0) return mstruct_old;
			if((new_points > old_points && (!convert_to_si_units || is_si_units || !new_is_si_units)) || (new_points == old_points && (new_minus || !old_minus) && (!new_is_currency || !eo.local_currency_conversion) && (!convert_to_si_units || !new_is_si_units))) return mstruct_old;
			return mstruct_new;
		}
		default: {}
	}
	return mstruct;
}
MathStructure Calculator::convertToCompositeUnit(const MathStructure &mstruct, CompositeUnit *cu, const EvaluationOptions &eo, bool always_convert) {
	return convert(mstruct, cu, eo, always_convert);
}
MathStructure Calculator::convert(const MathStructure &mstruct_to_convert, string str2, const EvaluationOptions &eo, MathStructure *to_struct) {
	if(to_struct) to_struct->setUndefined();
	remove_blank_ends(str2);std::cout << "4749" << endl;
	if(str2.empty()) return mstruct_to_convert;
	current_stage = MESSAGE_STAGE_CONVERSION;std::cout << "4751" << endl;
	int do_prefix = 0;std::cout << "4752" << endl;
	if(str2.length() > 1 && str2[1] == '?' && (str2[0] == 'b' || str2[0] == 'a' || str2[0] == 'd')) {
		do_prefix = 2;std::cout << "4754" << endl;
	} else if(str2[0] == '?') {
		do_prefix = 1;std::cout << "4756" << endl;
	}
	EvaluationOptions eo2 = eo;std::cout << "4758" << endl;
	eo2.keep_prefixes = !do_prefix;std::cout << "4759" << endl;
	if(str2[0] == '-') eo2.mixed_units_conversion = MIXED_UNITS_CONVERSION_NONE;
	else if(str2[0] == '+') eo2.mixed_units_conversion = MIXED_UNITS_CONVERSION_FORCE_INTEGER;
	else if(eo2.mixed_units_conversion != MIXED_UNITS_CONVERSION_NONE) eo2.mixed_units_conversion = MIXED_UNITS_CONVERSION_DOWNWARDS_KEEP;
	if(do_prefix || str2[0] == '0' || str2[0] == '+' || str2[0] == '-') {
		str2 = str2.substr(do_prefix > 1 ? 2 : 1, str2.length() - (do_prefix > 1 ? 2 : 1));std::cout << "4764" << endl;
		remove_blank_ends(str2);std::cout << "4765" << endl;
		if(str2.empty()) {
			current_stage = MESSAGE_STAGE_UNSET;std::cout << "4767" << endl;
			return convertToMixedUnits(mstruct_to_convert, eo2);
		}
	}
	MathStructure mstruct;std::cout << "4771" << endl;
	bool b = false;std::cout << "4772" << endl;
	Unit *u = getUnit(str2);std::cout << "4773" << endl;
	if(!u) u = getCompositeUnit(str2);
	Variable *v = NULL;std::cout << "4775" << endl;
	if(!u) v = getVariable(str2);
	if(!u && !v) {
		for(size_t i = 0; i < signs.size(); i++) {
			if(str2 == signs[i]) {
				u = getUnit(real_signs[i]);std::cout << "4780" << endl;
				if(!u) v = getVariable(real_signs[i]);
				break;
			}
		}
	}
	if(v && !v->isKnown()) v = NULL;
	if(u) {
		if(to_struct) to_struct->set(u);
		mstruct.set(convert(mstruct_to_convert, u, eo2, false, false));std::cout << "4789" << endl;
		b = true;std::cout << "4790" << endl;
	} else if(v) {
		if(to_struct) to_struct->set(v);
		mstruct.set(convert(mstruct_to_convert, (KnownVariable*) v, eo2));std::cout << "4793" << endl;
		b = true;std::cout << "4794" << endl;
	} else {
		current_stage = MESSAGE_STAGE_CONVERSION_PARSING;std::cout << "4796" << endl;
		CompositeUnit cu("", "temporary_composite_convert", "", str2);
		if(cu.countUnits() == 2 && cu.get(1)->referenceName() == "g" && cu.get(2)->referenceName() == "m" && str2.substr(0, 2) == "kg") {
			int exp;std::cout << "4799" << endl; Prefix *p;std::cout << "4799" << endl;
			if(cu.get(1, &exp, &p) && exp == 1 && p && p->value() == 1000 && cu.get(2, &exp, &p) && exp == -2) {
				Unit *u = getUnit("pond");
				if(u) {
					MathStructure mtest(convertToBaseUnits(mstruct_to_convert, eo));std::cout << "4803" << endl;
					mtest.sort();std::cout << "4804" << endl;
					if(mtest.isMultiplication() && mtest.size() >= 3 && mtest[mtest.size() - 3].isUnit() && mtest[mtest.size() - 3].unit()->referenceName() == "g" && mtest[mtest.size() - 2].isPower() && mtest[mtest.size() - 2][1].isMinusOne() && mtest[mtest.size() - 2][0].isUnit() && mtest[mtest.size() - 2][0].unit()->referenceName() == "m" && mtest[mtest.size() - 1].isPower() && mtest[mtest.size() - 1][1] == Number(-2, 1) && mtest[mtest.size() - 1][0].isUnit() && mtest[mtest.size() - 1][0].unit()->referenceName() == "s") {
						str2.replace(1, 2, "pond");
						cu.setBaseExpression(str2);std::cout << "4807" << endl;
					}
				}
			}
		}
		current_stage = MESSAGE_STAGE_CONVERSION;std::cout << "4812" << endl;
		if(to_struct) to_struct->set(cu.generateMathStructure());
		if(cu.countUnits() > 0) {
			mstruct.set(convert(mstruct_to_convert, &cu, eo2, false, false));std::cout << "4815" << endl;
			b = true;std::cout << "4816" << endl;
		}
	}
	if(!b) return mstruct_to_convert;
	if(!v && eo2.mixed_units_conversion != MIXED_UNITS_CONVERSION_NONE) mstruct.set(convertToMixedUnits(mstruct, eo2));
	current_stage = MESSAGE_STAGE_UNSET;std::cout << "4821" << endl;
	return mstruct;
}
Unit* Calculator::addUnit(Unit *u, bool force, bool check_names) {
	if(check_names) {
		for(size_t i = 1; i <= u->countNames(); i++) {
			u->setName(getName(u->getName(i).name, u, force), i);
		}
	}
	if(!u->isLocal() && units.size() > 0 && units[units.size() - 1]->isLocal()) {
		units.insert(units.begin(), u);std::cout << "4831" << endl;
	} else {
		units.push_back(u);std::cout << "4833" << endl;
	}
	unitNameChanged(u, true);std::cout << "4835" << endl;
	for(vector<Unit*>::iterator it = deleted_units.begin(); it != deleted_units.end(); ++it) {
		if(*it == u) {
			deleted_units.erase(it);std::cout << "4838" << endl;
			break;
		}
	}
	u->setRegistered(true);std::cout << "4842" << endl;
	u->setChanged(false);std::cout << "4843" << endl;
	return u;
}
void Calculator::delPrefixUFV(Prefix *object) {
	int i = 0;std::cout << "4847" << endl;
	for(vector<void*>::iterator it = ufvl.begin(); ; ++it) {
		del_ufvl:
		if(it == ufvl.end()) {
			break;
		}
		if(*it == object) {
			it = ufvl.erase(it);std::cout << "4854" << endl;
			ufvl_t.erase(ufvl_t.begin() + i);std::cout << "4855" << endl;
			ufvl_i.erase(ufvl_i.begin() + i);std::cout << "4856" << endl;
			if(it == ufvl.end()) break;
			goto del_ufvl;std::cout << "4858" << endl;
		}
		i++;std::cout << "4860" << endl;
	}
	for(size_t i2 = 0; i2 < UFV_LENGTHS; i2++) {
		i = 0;std::cout << "4863" << endl;
		for(vector<void*>::iterator it = ufv[0][i2].begin(); ; ++it) {
			del_ufv:
			if(it == ufv[0][i2].end()) {
				break;
			}
			if(*it == object) {
				it = ufv[0][i2].erase(it);std::cout << "4870" << endl;
				ufv_i[0][i2].erase(ufv_i[0][i2].begin() + i);std::cout << "4871" << endl;
				if(it == ufv[0][i2].end()) break;
				goto del_ufv;std::cout << "4873" << endl;
			}
			i++;std::cout << "4875" << endl;
		}
	}
}
void Calculator::delUFV(ExpressionItem *object) {
	int i = 0;std::cout << "4880" << endl;
	for(vector<void*>::iterator it = ufvl.begin(); ; ++it) {
		del_ufvl:
		if(it == ufvl.end()) {
			break;
		}
		if(*it == object) {
			it = ufvl.erase(it);std::cout << "4887" << endl;
			ufvl_t.erase(ufvl_t.begin() + i);std::cout << "4888" << endl;
			ufvl_i.erase(ufvl_i.begin() + i);std::cout << "4889" << endl;
			if(it == ufvl.end()) break;
			goto del_ufvl;std::cout << "4891" << endl;
		}
		i++;std::cout << "4893" << endl;
	}
	int i3 = 0;std::cout << "4895" << endl;
	switch(object->type()) {
		case TYPE_FUNCTION: {i3 = 1; break;}
		case TYPE_UNIT: {i3 = 2; break;}
		case TYPE_VARIABLE: {i3 = 3; break;}
	}
	for(size_t i2 = 0; i2 < UFV_LENGTHS; i2++) {
		i = 0;std::cout << "4902" << endl;
		for(vector<void*>::iterator it = ufv[i3][i2].begin(); ; ++it) {
			del_ufv:
			if(it == ufv[i3][i2].end()) {
				break;
			}
			if(*it == object) {
				it = ufv[i3][i2].erase(it);std::cout << "4909" << endl;
				ufv_i[i3][i2].erase(ufv_i[i3][i2].begin() + i);std::cout << "4910" << endl;
				if(it == ufv[i3][i2].end()) break;
				goto del_ufv;std::cout << "4912" << endl;
			}
			i++;std::cout << "4914" << endl;
		}
	}
}
Unit* Calculator::getUnit(string name_) {
	if(name_.empty()) return NULL;
	for(size_t i = 0; i < units.size(); i++) {
		if(units[i]->subtype() != SUBTYPE_COMPOSITE_UNIT && (units[i]->hasName(name_))) {
			return units[i];
		}
	}
	return NULL;
}
Unit* Calculator::getActiveUnit(string name_) {
	if(name_.empty()) return NULL;
	for(size_t i = 0; i < units.size(); i++) {
		if(units[i]->isActive() && units[i]->subtype() != SUBTYPE_COMPOSITE_UNIT && units[i]->hasName(name_)) {
			return units[i];
		}
	}
	return NULL;
}
Unit* Calculator::getLocalCurrency() {
	if(priv->local_currency) return priv->local_currency;
	struct lconv *lc = localeconv();std::cout << "4938" << endl;
	if(lc) {
		string local_currency = lc->int_curr_symbol;std::cout << "4940" << endl;
		remove_blank_ends(local_currency);std::cout << "4941" << endl;
		if(!local_currency.empty()) {
			if(local_currency.length() > 3) local_currency = local_currency.substr(0, 3);
			return getActiveUnit(local_currency);
		}
	}
	return NULL;
}
void Calculator::setLocalCurrency(Unit *u) {
	priv->local_currency = u;std::cout << "4950" << endl;
}
Unit* Calculator::getCompositeUnit(string internal_name_) {
	if(internal_name_.empty()) return NULL;
	for(size_t i = 0; i < units.size(); i++) {
		if(units[i]->subtype() == SUBTYPE_COMPOSITE_UNIT && units[i]->hasName(internal_name_)) {
			return units[i];
		}
	}
	return NULL;
}

Variable* Calculator::addVariable(Variable *v, bool force, bool check_names) {
	if(check_names) {
		for(size_t i = 1; i <= v->countNames(); i++) {
			v->setName(getName(v->getName(i).name, v, force), i);
		}
	}
	if(!v->isLocal() && variables.size() > 0 && variables[variables.size() - 1]->isLocal()) {
		variables.insert(variables.begin(), v);std::cout << "4969" << endl;
	} else {
		variables.push_back(v);std::cout << "4971" << endl;
	}
	variableNameChanged(v, true);std::cout << "4973" << endl;
	for(vector<Variable*>::iterator it = deleted_variables.begin(); it != deleted_variables.end(); ++it) {
		if(*it == v) {
			deleted_variables.erase(it);std::cout << "4976" << endl;
			break;
		}
	}
	v->setRegistered(true);std::cout << "4980" << endl;
	v->setChanged(false);std::cout << "4981" << endl;
	return v;
}
void Calculator::expressionItemDeactivated(ExpressionItem *item) {
	delUFV(item);std::cout << "4985" << endl;
}
void Calculator::expressionItemActivated(ExpressionItem *item) {
	ExpressionItem *item2 = getActiveExpressionItem(item);std::cout << "4988" << endl;
	if(item2) {
		item2->setActive(false);std::cout << "4990" << endl;
	}
	nameChanged(item);std::cout << "4992" << endl;
}
void Calculator::expressionItemDeleted(ExpressionItem *item) {
	switch(item->type()) {
		case TYPE_VARIABLE: {
			for(vector<Variable*>::iterator it = variables.begin(); it != variables.end(); ++it) {
				if(*it == item) {
					variables.erase(it);std::cout << "4999" << endl;
					deleted_variables.push_back((Variable*) item);std::cout << "5000" << endl;
					break;
				}
			}
			break;
		}
		case TYPE_FUNCTION: {
			for(vector<MathFunction*>::iterator it = functions.begin(); it != functions.end(); ++it) {
				if(*it == item) {
					functions.erase(it);std::cout << "5009" << endl;
					deleted_functions.push_back((MathFunction*) item);std::cout << "5010" << endl;
					break;
				}
			}
			if(item->subtype() == SUBTYPE_DATA_SET) {
				for(vector<DataSet*>::iterator it = data_sets.begin(); it != data_sets.end(); ++it) {
					if(*it == item) {
						data_sets.erase(it);std::cout << "5017" << endl;
						break;
					}
				}
			}
			break;
		}
		case TYPE_UNIT: {
			for(vector<Unit*>::iterator it = units.begin(); it != units.end(); ++it) {
				if(*it == item) {
					units.erase(it);std::cout << "5027" << endl;
					deleted_units.push_back((Unit*) item);std::cout << "5028" << endl;
					break;
				}
			}
			break;
		}
	}
	for(size_t i2 = 1; i2 <= item->countNames(); i2++) {
		if(item->type() == TYPE_VARIABLE || item->type() == TYPE_UNIT) {
			for(size_t i = 0; i < variables.size(); i++) {
				if(!variables[i]->isLocal() && !variables[i]->isActive() && variables[i]->hasName(item->getName(i2).name, item->getName(i2).case_sensitive) && !getActiveExpressionItem(variables[i])) {variables[i]->setActive(true);}
			}
			for(size_t i = 0; i < units.size(); i++) {
				if(!units[i]->isLocal() && !units[i]->isActive() && units[i]->hasName(item->getName(i2).name, item->getName(i2).case_sensitive) && !getActiveExpressionItem(units[i])) units[i]->setActive(true);
			}
		} else {
			for(size_t i = 0; i < functions.size(); i++) {
				if(!functions[i]->isLocal() && !functions[i]->isActive() && functions[i]->hasName(item->getName(i2).name, item->getName(i2).case_sensitive) && !getActiveExpressionItem(functions[i])) functions[i]->setActive(true);
			}
		}
	}
	delUFV(item);std::cout << "5049" << endl;
}
void Calculator::nameChanged(ExpressionItem *item, bool new_item) {
	if(!item->isActive() || item->countNames() == 0) return;
	if(item->type() == TYPE_UNIT && ((Unit*) item)->subtype() == SUBTYPE_COMPOSITE_UNIT) {
		return;
	}
	size_t l2;std::cout << "5056" << endl;
	if(!new_item) delUFV(item);
	for(size_t i2 = 1; i2 <= item->countNames(); i2++) {
		l2 = item->getName(i2).name.length();std::cout << "5059" << endl;
		if(l2 > UFV_LENGTHS) {
			size_t i = 0, l = 0;std::cout << "5061" << endl;
			for(vector<void*>::iterator it = ufvl.begin(); ; ++it) {
				if(it != ufvl.end()) {
					if(ufvl_t[i] == 'v')						l = ((Variable*) (*it))->getName(ufvl_i[i]).name.length();
					else if(ufvl_t[i] == 'f')						l = ((MathFunction*) (*it))->getName(ufvl_i[i]).name.length();
					else if(ufvl_t[i] == 'u')						l = ((Unit*) (*it))->getName(ufvl_i[i]).name.length();
					else if(ufvl_t[i] == 'p')						l = ((Prefix*) (*it))->shortName(false).length();
					else if(ufvl_t[i] == 'P')						l = ((Prefix*) (*it))->longName(false).length();
					else if(ufvl_t[i] == 'q')						l = ((Prefix*) (*it))->unicodeName(false).length();
				}
				if(it == ufvl.end()) {
					ufvl.push_back((void*) item);std::cout << "5072" << endl;
					switch(item->type()) {
						case TYPE_VARIABLE: {ufvl_t.push_back('v'); break;}
						case TYPE_FUNCTION: {ufvl_t.push_back('f'); break;}
						case TYPE_UNIT: {ufvl_t.push_back('u'); break;}
					}
					ufvl_i.push_back(i2);std::cout << "5078" << endl;
					break;
				} else {
					if(l < l2
					|| (item->type() == TYPE_VARIABLE && l == l2 && ufvl_t[i] == 'v')					|| (item->type() == TYPE_FUNCTION && l == l2 && (ufvl_t[i] != 'p' && ufvl_t[i] != 'P' && ufvl_t[i] != 'q'))					|| (item->type() == TYPE_UNIT && l == l2 && (ufvl_t[i] != 'p' && ufvl_t[i] != 'P' && ufvl_t[i] != 'q' && ufvl_t[i] != 'f'))					) {
						ufvl.insert(it, (void*) item);std::cout << "5083" << endl;
						switch(item->type()) {
							case TYPE_VARIABLE: {ufvl_t.insert(ufvl_t.begin() + i, 'v'); break;}
							case TYPE_FUNCTION: {ufvl_t.insert(ufvl_t.begin() + i, 'f'); break;}
							case TYPE_UNIT: {ufvl_t.insert(ufvl_t.begin() + i, 'u'); break;}
						}
						ufvl_i.insert(ufvl_i.begin() + i, i2);std::cout << "5089" << endl;
						break;
					}
				}
				i++;std::cout << "5093" << endl;
			}
		} else if(l2 > 0) {
			l2--;std::cout << "5096" << endl;
			switch(item->type()) {
				case TYPE_VARIABLE: {
					ufv[3][l2].push_back((void*) item);std::cout << "5099" << endl;
					ufv_i[3][l2].push_back(i2);std::cout << "5100" << endl;
					break;
				}
				case TYPE_FUNCTION:  {
					ufv[1][l2].push_back((void*) item);std::cout << "5104" << endl;
					ufv_i[1][l2].push_back(i2);std::cout << "5105" << endl;
					break;
				}
				case TYPE_UNIT:  {
					ufv[2][l2].push_back((void*) item);std::cout << "5109" << endl;
					ufv_i[2][l2].push_back(i2);std::cout << "5110" << endl;
					break;
				}
			}
		}
	}
}
void Calculator::variableNameChanged(Variable *v, bool new_item) {
	nameChanged(v, new_item);std::cout << "5118" << endl;
}
void Calculator::functionNameChanged(MathFunction *f, bool new_item) {
	nameChanged(f, new_item);std::cout << "5121" << endl;
}
void Calculator::unitNameChanged(Unit *u, bool new_item) {
	nameChanged(u, new_item);std::cout << "5124" << endl;
}

Variable* Calculator::getVariable(string name_) {
	if(name_.empty()) return NULL;
	for(size_t i = 0; i < variables.size(); i++) {
		if(variables[i]->hasName(name_)) {
			return variables[i];
		}
	}
	return NULL;
}
Variable* Calculator::getActiveVariable(string name_) {
	if(name_.empty()) return NULL;
	for(size_t i = 0; i < variables.size(); i++) {
		if(variables[i]->isActive() && variables[i]->hasName(name_)) {
			return variables[i];
		}
	}
	return NULL;
}
ExpressionItem* Calculator::addExpressionItem(ExpressionItem *item, bool force) {
	switch(item->type()) {
		case TYPE_VARIABLE: {
			return addVariable((Variable*) item, force);
		}
		case TYPE_FUNCTION: {
			if(item->subtype() == SUBTYPE_DATA_SET) return addDataSet((DataSet*) item, force);
			else return addFunction((MathFunction*) item, force);
		}
		case TYPE_UNIT: {
			return addUnit((Unit*) item, force);
		}
	}
	return NULL;
}
MathFunction* Calculator::addFunction(MathFunction *f, bool force, bool check_names) {
	if(check_names) {
		for(size_t i = 1; i <= f->countNames(); i++) {
			f->setName(getName(f->getName(i).name, f, force), i);
		}
	}
	if(!f->isLocal() && functions.size() > 0 && functions[functions.size() - 1]->isLocal()) {
		functions.insert(functions.begin(), f);std::cout << "5167" << endl;
	} else {
		functions.push_back(f);std::cout << "5169" << endl;
	}
	functionNameChanged(f, true);std::cout << "5171" << endl;
	for(vector<MathFunction*>::iterator it = deleted_functions.begin(); it != deleted_functions.end(); ++it) {
		if(*it == f) {
			deleted_functions.erase(it);std::cout << "5174" << endl;
			break;
		}
	}
	f->setRegistered(true);std::cout << "5178" << endl;
	f->setChanged(false);std::cout << "5179" << endl;
	return f;
}
DataSet* Calculator::addDataSet(DataSet *dc, bool force, bool check_names) {
	addFunction(dc, force, check_names);
	data_sets.push_back(dc);std::cout << "5184" << endl;
	return dc;
}
DataSet* Calculator::getDataSet(size_t index) {
	if(index > 0 && index <= data_sets.size()) {
		return data_sets[index - 1];
	}
	return 0;
}
DataSet* Calculator::getDataSet(string name) {
	if(name.empty()) return NULL;
	for(size_t i = 0; i < data_sets.size(); i++) {
		if(data_sets[i]->hasName(name)) {
			return data_sets[i];
		}
	}
	return NULL;
}
MathFunction* Calculator::getFunction(string name_) {
	if(name_.empty()) return NULL;
	for(size_t i = 0; i < functions.size(); i++) {
		if(functions[i]->hasName(name_)) {
			return functions[i];
		}
	}
	return NULL;
}
MathFunction* Calculator::getActiveFunction(string name_) {
	if(name_.empty()) return NULL;
	for(size_t i = 0; i < functions.size(); i++) {
		if(functions[i]->isActive() && functions[i]->hasName(name_)) {
			return functions[i];
		}
	}
	return NULL;
}
bool Calculator::variableNameIsValid(const string &name_) {
	return !name_.empty() && name_.find_first_of(ILLEGAL_IN_NAMES) == string::npos && is_not_in(NUMBERS, name_[0]);
}
bool Calculator::functionNameIsValid(const string &name_) {
	return !name_.empty() && name_.find_first_of(ILLEGAL_IN_NAMES) == string::npos && is_not_in(NUMBERS, name_[0]);
}
bool Calculator::unitNameIsValid(const string &name_) {
	return !name_.empty() && name_.find_first_of(ILLEGAL_IN_UNITNAMES) == string::npos;
}
bool Calculator::variableNameIsValid(const char *name_) {
	if(strlen(name_) == 0) return false;
	if(is_in(NUMBERS, name_[0])) return false;
	for(size_t i = 0; name_[i] != '\0'; i++) {
		if(is_in(ILLEGAL_IN_NAMES, name_[i])) return false;
	}
	return true;
}
bool Calculator::functionNameIsValid(const char *name_) {
	if(strlen(name_) == 0) return false;
	if(is_in(NUMBERS, name_[0])) return false;
	for(size_t i = 0; name_[i] != '\0'; i++) {
		if(is_in(ILLEGAL_IN_NAMES, name_[i])) return false;
	}
	return true;
}
bool Calculator::unitNameIsValid(const char *name_) {
	if(strlen(name_) == 0) return false;
	for(size_t i = 0; name_[i] != '\0'; i++) {
		if(is_in(ILLEGAL_IN_UNITNAMES, name_[i])) return false;
	}
	return true;
}
#define VERSION_BEFORE(i1, i2, i3) (version_numbers[0] < i1 || (version_numbers[0] == i1 && (version_numbers[1] < i2 || (version_numbers[1] == i2 && version_numbers[2] < i3))))
bool Calculator::variableNameIsValid(const string &name_, int version_numbers[3], bool is_user_defs) {
	return variableNameIsValid(name_.c_str(), version_numbers, is_user_defs);
}
bool Calculator::functionNameIsValid(const string &name_, int version_numbers[3], bool is_user_defs) {
	return functionNameIsValid(name_.c_str(), version_numbers, is_user_defs);
}
bool Calculator::unitNameIsValid(const string &name_, int version_numbers[3], bool is_user_defs) {
	return unitNameIsValid(name_.c_str(), version_numbers, is_user_defs);
}
bool Calculator::variableNameIsValid(const char *name_, int version_numbers[3], bool is_user_defs) {
	if(strlen(name_) == 0) return false;
	if(is_in(NUMBERS, name_[0])) return false;
	bool b = false;std::cout << "5264" << endl;
	for(size_t i = 0; name_[i] != '\0'; i++) {
		if(is_in(ILLEGAL_IN_NAMES, name_[i])) {
			if(is_user_defs && VERSION_BEFORE(0, 8, 1) && name_[i] == BITWISE_NOT_CH) {
				b = true;std::cout << "5268" << endl;
			} else {
				return false;
			}
		}
	}
	if(b) {
		error(true, _("\"%s\" is not allowed in names anymore. Please change the name of \"%s\", or the variable will be lost."), BITWISE_NOT, name_, NULL);
	}
	return true;
}
bool Calculator::functionNameIsValid(const char *name_, int version_numbers[3], bool is_user_defs) {
	if(strlen(name_) == 0) return false;
	if(is_in(NUMBERS, name_[0])) return false;
	bool b = false;std::cout << "5282" << endl;
	for(size_t i = 0; name_[i] != '\0'; i++) {
		if(is_in(ILLEGAL_IN_NAMES, name_[i])) {
			if(is_user_defs && VERSION_BEFORE(0, 8, 1) && name_[i] == BITWISE_NOT_CH) {
				b = true;std::cout << "5286" << endl;
			} else {
				return false;
			}
		}
	}
	if(b) {
		error(true, _("\"%s\" is not allowed in names anymore. Please change the name \"%s\", or the function will be lost."), BITWISE_NOT, name_, NULL);
	}
	return true;
}
bool Calculator::unitNameIsValid(const char *name_, int version_numbers[3], bool is_user_defs) {
	if(strlen(name_) == 0) return false;
	bool b = false;std::cout << "5299" << endl;
	for(size_t i = 0; name_[i] != '\0'; i++) {
		if(is_in(ILLEGAL_IN_UNITNAMES, name_[i])) {
			if(is_user_defs && VERSION_BEFORE(0, 8, 1) && name_[i] == BITWISE_NOT_CH) {
				b = true;std::cout << "5303" << endl;
			} else {
				return false;
			}
		}
	}
	if(b) {
		error(true, _("\"%s\" is not allowed in names anymore. Please change the name \"%s\", or the unit will be lost."), BITWISE_NOT, name_, NULL);
	}
	return true;
}
string Calculator::convertToValidVariableName(string name_) {
	if(name_.empty()) return "var_1";
	size_t i = 0;std::cout << "5316" << endl;
	while(true) {
		i = name_.find_first_of(ILLEGAL_IN_NAMES_MINUS_SPACE_STR, i);std::cout << "5318" << endl;
		if(i == string::npos)			break;
		name_.erase(name_.begin() + i);std::cout << "5320" << endl;
	}
	gsub(SPACE, UNDERSCORE, name_);std::cout << "5322" << endl;
	while(is_in(NUMBERS, name_[0])) {
		name_.erase(name_.begin());std::cout << "5324" << endl;
	}
	return name_;
}
string Calculator::convertToValidFunctionName(string name_) {
	if(name_.empty()) return "func_1";
	return convertToValidVariableName(name_);
}
string Calculator::convertToValidUnitName(string name_) {
	if(name_.empty()) return "new_unit";
	size_t i = 0;std::cout << "5334" << endl;
	string stmp = ILLEGAL_IN_NAMES_MINUS_SPACE_STR + NUMBERS;std::cout << "5335" << endl;
	while(true) {
		i = name_.find_first_of(stmp, i);std::cout << "5337" << endl;
		if(i == string::npos)			break;
		name_.erase(name_.begin() + i);std::cout << "5339" << endl;
	}
	gsub(SPACE, UNDERSCORE, name_);std::cout << "5341" << endl;
	return name_;
}
bool Calculator::nameTaken(string name, ExpressionItem *object) {
	if(name.empty()) return false;
	if(object) {
		switch(object->type()) {
			case TYPE_VARIABLE: {}
			case TYPE_UNIT: {
				for(size_t index = 0; index < variables.size(); index++) {
					if(variables[index]->isActive() && variables[index]->hasName(name)) {
						return variables[index] != object;
					}
				}
				for(size_t i = 0; i < units.size(); i++) {
					if(units[i]->isActive() && units[i]->hasName(name)) {
						return units[i] != object;
					}
				}
				break;
			}
			case TYPE_FUNCTION: {
				for(size_t index = 0; index < functions.size(); index++) {
					if(functions[index]->isActive() && functions[index]->hasName(name)) {
						return functions[index] != object;
					}
				}
				break;
			}
		}
	} else {
		return getActiveExpressionItem(name) != NULL;
	}
	return false;
}
bool Calculator::variableNameTaken(string name, Variable *object) {
	if(name.empty()) return false;
	for(size_t index = 0; index < variables.size(); index++) {
		if(variables[index]->isActive() && variables[index]->hasName(name)) {
			return variables[index] != object;
		}
	}

	for(size_t i = 0; i < units.size(); i++) {
		if(units[i]->isActive() && units[i]->hasName(name)) {
			return true;
		}
	}
	return false;
}
bool Calculator::unitNameTaken(string name, Unit *object) {
	if(name.empty()) return false;
	for(size_t index = 0; index < variables.size(); index++) {
		if(variables[index]->isActive() && variables[index]->hasName(name)) {
			return true;
		}
	}

	for(size_t i = 0; i < units.size(); i++) {
		if(units[i]->isActive() && units[i]->hasName(name)) {
			return units[i] == object;
		}
	}
	return false;
}
bool Calculator::functionNameTaken(string name, MathFunction *object) {
	if(name.empty()) return false;
	for(size_t index = 0; index < functions.size(); index++) {
		if(functions[index]->isActive() && functions[index]->hasName(name)) {
			return functions[index] != object;
		}
	}
	return false;
}
bool Calculator::unitIsUsedByOtherUnits(const Unit *u) const {
	const Unit *u2;std::cout << "5416" << endl;
	for(size_t i = 0; i < units.size(); i++) {
		if(units[i] != u) {
			u2 = units[i];std::cout << "5419" << endl;
			while(u2->subtype() == SUBTYPE_ALIAS_UNIT) {
				u2 = ((AliasUnit*) u2)->firstBaseUnit();std::cout << "5421" << endl;
				if(u2 == u) {
					return true;
				}
			}
		}
	}
	return false;
}

bool compare_name(const string &name, const string &str, const size_t &name_length, const size_t &str_index, int base) {
	if(name_length == 0) return false;
	if(name[0] != str[str_index]) return false;
	if(name_length == 1) {
		if(base < 2 || base > 10) return is_not_number(str[str_index], base);
		return true;
	}
	for(size_t i = 1; i < name_length; i++) {
		if(name[i] != str[str_index + i]) return false;
	}
	if(base < 2 || base > 10) {
		for(size_t i = 0; i < name_length; i++) {
			if(is_not_number(str[str_index + i], base)) return true;
		}
		return false;
	}
	return true;
}
size_t compare_name_no_case(const string &name, const string &str, const size_t &name_length, const size_t &str_index, int base) {
	if(name_length == 0) return 0;
	size_t is = str_index;std::cout << "5451" << endl;
	for(size_t i = 0; i < name_length; i++, is++) {
		if(is >= str.length()) return 0;
		if((name[i] < 0 && i + 1 < name_length) || (str[is] < 0 && is + 1 < str.length())) {
			size_t i2 = 1, is2 = 1;std::cout << "5455" << endl;
			if(name[i] < 0) {
				while(i2 + i < name_length && name[i2 + i] < 0) {
					i2++;std::cout << "5458" << endl;
				}
			}
			if(str[is] < 0) {
				while(is2 + is < str.length() && str[is2 + is] < 0) {
					is2++;std::cout << "5463" << endl;
				}
			}
			bool isequal = (i2 == is2);std::cout << "5466" << endl;
			if(isequal) {
				for(size_t i3 = 0; i3 < i2; i3++) {
					if(str[is + i3] != name[i + i3]) {
						isequal = false;std::cout << "5470" << endl;
						break;
					}
				}
			}
			if(!isequal) {
				char *gstr1 = utf8_strdown(name.c_str() + (sizeof(char) * i), i2);std::cout << "5476" << endl;
				char *gstr2 = utf8_strdown(str.c_str() + (sizeof(char) * (is)), is2);std::cout << "5477" << endl;
				if(!gstr1 || !gstr2) return 0;
				if(strcmp(gstr1, gstr2) != 0) {free(gstr1); free(gstr2); return 0;}
				free(gstr1);std::cout << "5480" << endl; free(gstr2);std::cout << "5480" << endl;
			}
			i += i2 - 1;std::cout << "5482" << endl;
			is += is2 - 1;std::cout << "5483" << endl;
		} else if(name[i] != str[is] && !((name[i] >= 'a' && name[i] <= 'z') && name[i] - 32 == str[is]) && !((name[i] <= 'Z' && name[i] >= 'A') && name[i] + 32 == str[is])) {
			return 0;
		}
	}
	if(base < 2 || base > 10) {
		for(size_t i = str_index; i < is; i++) {
			if(is_not_number(str[i], base)) return is - str_index;
		}
		return 0;
	}
	return is - str_index;
}

const char *internal_signs[] = {SIGN_PLUSMINUS, "\b", "+/-", "\b", "⊻", "\a", "∠", "\x1c"};
#define INTERNAL_SIGNS_COUNT 8
#define INTERNAL_NUMBER_CHARS "\b"
#define INTERNAL_OPERATORS "\a\b%\x1c"
#define DUODECIMAL_CHARS "EX"

void Calculator::parseSigns(string &str, bool convert_to_internal_representation) const {
	vector<size_t> q_begin;std::cout << "5504" << endl;
	vector<size_t> q_end;std::cout << "5505" << endl;
	size_t quote_index = 0;std::cout << "5506" << endl;
	while(true) {
		quote_index = str.find_first_of("\"\'", quote_index);
		if(quote_index == string::npos) {
			break;
		}
		q_begin.push_back(quote_index);std::cout << "5512" << endl;
		quote_index = str.find(str[quote_index], quote_index + 1);std::cout << "5513" << endl;
		if(quote_index == string::npos) {
			q_end.push_back(str.length() - 1);std::cout << "5515" << endl;
			break;
		}
		q_end.push_back(quote_index);std::cout << "5518" << endl;
		quote_index++;std::cout << "5519" << endl;
	}
	for(size_t i = 0; i < signs.size(); i++) {
		size_t ui = str.find(signs[i]);std::cout << "5522" << endl;
		size_t ui2 = 0;std::cout << "5523" << endl;
		while(ui != string::npos) {
			for(size_t ui2 = 0; ui2 < q_end.size(); ui2++) {
				if(ui >= q_begin[ui2]) {
					if(ui <= q_end[ui2]) {
						ui = str.find(signs[i], q_end[ui2] + 1);std::cout << "5528" << endl;
						if(ui == string::npos) break;
					}
				} else {
					break;
				}
			}
			if(ui == string::npos) break;
			int index_shift = real_signs[i].length() - signs[i].length();
			for(size_t ui3 = ui2; ui3 < q_begin.size(); ui3++) {
				q_begin[ui3] += index_shift;
				q_end[ui3] += index_shift;
			}
			str.replace(ui, signs[i].length(), real_signs[i]);std::cout << "5541" << endl;
			ui = str.find(signs[i], ui + real_signs[i].length());std::cout << "5542" << endl;
		}
	}

	size_t prev_ui = string::npos, space_n = 0;std::cout << "5546" << endl;
	while(true) {
		size_t ui = str.find("\xe2\x81", prev_ui == string::npos ? 0 : prev_ui);
		if(ui != string::npos && (ui == str.length() - 2 || (str[ui + 2] != -80 && (str[ui + 2] < -76 || str[ui + 2] > -71)))) ui = string::npos;
		size_t ui2 = str.find('\xc2', prev_ui == string::npos ? 0 : prev_ui);std::cout << "5550" << endl;
		if(ui2 != string::npos && (ui2 == str.length() - 1 || (str[ui2 + 1] != -71 && str[ui2 + 1] != -77 && str[ui2 + 1] != -78))) ui2 = string::npos;
		if(ui2 != string::npos && (ui == string::npos || ui2 < ui)) ui = ui2;
		if(ui != string::npos) {
			for(size_t ui3 = 0; ui3 < q_end.size(); ui3++) {
				if(ui <= q_end[ui3] && ui >= q_begin[ui3]) {
					ui = str.find("\xe2\x81", q_end[ui3] + 1);
					if(ui != string::npos && (ui == str.length() - 2 || (str[ui + 2] != -80 && (str[ui + 2] < -76 || str[ui + 2] > -71)))) ui = string::npos;
					ui2 = str.find('\xc2', q_end[ui3] + 1);std::cout << "5558" << endl;
					if(ui2 != string::npos && (ui2 == str.length() - 1 || (str[ui2 + 1] != -71 && str[ui2 + 1] != -77 && str[ui2 + 1] != -78))) ui2 = string::npos;
					if(ui2 != string::npos && (ui == string::npos || ui2 < ui)) ui = ui2;
					if(ui == string::npos) break;
				}
			}
		}
		if(ui == string::npos) break;
		int index_shift = (str[ui] == '\xc2' ? -2 : -3);
		if(ui == prev_ui) index_shift += 1;
		else index_shift += 4;
		for(size_t ui3 = 0; ui3 < q_begin.size(); ui3++) {
			if(q_begin[ui3] >= ui) {
				q_begin[ui3] += index_shift;
				q_end[ui3] += index_shift;
			}
		}
		if(str[ui] == '\xc2') {
			if(str[ui + 1] == -71) str.replace(ui, 2, ui == prev_ui ? "1)" : "^(1)");
			else if(str[ui + 1] == -78) str.replace(ui, 2, ui == prev_ui ? "2)" : "^(2)");
			else if(str[ui + 1] == -77) str.replace(ui, 2, ui == prev_ui ? "3)" : "^(3)");
		} else {
			if(str[ui + 2] == -80) str.replace(ui, 3, ui == prev_ui ? "0)" : "^(0)");
			else if(str[ui + 2] == -76) str.replace(ui, 3, ui == prev_ui ? "4)" : "^(4)");
			else if(str[ui + 2] == -75) str.replace(ui, 3, ui == prev_ui ? "5)" : "^(5)");
			else if(str[ui + 2] == -74) str.replace(ui, 3, ui == prev_ui ? "6)" : "^(6)");
			else if(str[ui + 2] == -73) str.replace(ui, 3, ui == prev_ui ? "7)" : "^(7)");
			else if(str[ui + 2] == -72) str.replace(ui, 3, ui == prev_ui ? "8)" : "^(8)");
			else if(str[ui + 2] == -71) str.replace(ui, 3, ui == prev_ui ? "9)" : "^(9)");
		}
		if(ui == prev_ui) {
			str.erase(prev_ui - space_n - 1, 1);std::cout << "5589" << endl;
			prev_ui = ui + 1;std::cout << "5590" << endl;
		} else {
			prev_ui = ui + 4;std::cout << "5592" << endl;
		}
		space_n = 0;std::cout << "5594" << endl;
		while(prev_ui + 1 < str.length() && str[prev_ui] == SPACE_CH) {
			space_n++;std::cout << "5596" << endl;
			prev_ui++;std::cout << "5597" << endl;
		}
	}
	prev_ui = string::npos;std::cout << "5600" << endl;
	while(true) {
		size_t ui = str.find("\xe2\x85", prev_ui == string::npos ? 0 : prev_ui);
		if(ui != string::npos && (ui == str.length() - 2 || str[ui + 2] < -112 || str[ui + 2] > -98)) ui = string::npos;
		if(ui != string::npos) {
			for(size_t ui3 = 0; ui3 < q_end.size(); ui3++) {
				if(ui <= q_end[ui3] && ui >= q_begin[ui3]) {
					ui = str.find("\xe2\x85", q_end[ui3] + 1);
					if(ui != string::npos && (ui == str.length() - 2 || str[ui + 2] < -112 || str[ui + 2] > -98)) ui = string::npos;
					if(ui == string::npos) break;
				}
			}
		}
		if(ui == string::npos) break;
		space_n = 0;std::cout << "5614" << endl;
		while(ui > 0 && ui - 1 - space_n != 0 && str[ui - 1 - space_n] == SPACE_CH) space_n++;
		bool b_add = (ui > 0 && is_in(NUMBER_ELEMENTS, str[ui - 1 - space_n]));std::cout << "5616" << endl;
		int index_shift = (b_add ? 6 : 5) - 3;
		if(str[ui + 2] == -110) index_shift++;
		for(size_t ui2 = 0; ui2 < q_begin.size(); ui2++) {
			if(q_begin[ui2] >= ui) {
				q_begin[ui2] += index_shift;
				q_end[ui2] += index_shift;
			}
		}
		if(str[ui + 2] == -98) str.replace(ui, 3, b_add ? "+(7/8)" : "(7/8)");
		else if(str[ui + 2] == -99) str.replace(ui, 3, b_add ? "+(5/8)" : "(5/8)");
		else if(str[ui + 2] == -100) str.replace(ui, 3, b_add ? "+(3/8)" : "(3/8)");
		else if(str[ui + 2] == -101) str.replace(ui, 3, b_add ? "+(1/8)" : "(1/8)");
		else if(str[ui + 2] == -102) str.replace(ui, 3, b_add ? "+(5/6)" : "(5/6)");
		else if(str[ui + 2] == -103) str.replace(ui, 3, b_add ? "+(1/6)" : "(1/6)");
		else if(str[ui + 2] == -104) str.replace(ui, 3, b_add ? "+(4/5)" : "(4/5)");
		else if(str[ui + 2] == -105) str.replace(ui, 3, b_add ? "+(3/5)" : "(3/5)");
		else if(str[ui + 2] == -106) str.replace(ui, 3, b_add ? "+(2/5)" : "(2/5)");
		else if(str[ui + 2] == -107) str.replace(ui, 3, b_add ? "+(1/5)" : "(1/5)");
		else if(str[ui + 2] == -108) str.replace(ui, 3, b_add ? "+(2/3)" : "(2/3)");
		else if(str[ui + 2] == -109) str.replace(ui, 3, b_add ? "+(1/3)" : "(1/3)");
		else if(str[ui + 2] == -110) {str.replace(ui, 3, b_add ? "+(1/10)" : "(1/10)"); ui++;}
		else if(str[ui + 2] == -111) str.replace(ui, 3, b_add ? "+(1/9)" : "(1/9)");
		else if(str[ui + 2] == -112) str.replace(ui, 3, b_add ? "+(1/7)" : "(1/7)");
		if(b_add) prev_ui = ui + 6;
		else prev_ui = ui + 5;std::cout << "5641" << endl;
	}
	prev_ui = string::npos;std::cout << "5643" << endl;
	while(true) {
		size_t ui = str.find('\xc2', prev_ui == string::npos ? 0 : prev_ui);std::cout << "5645" << endl;
		if(ui != string::npos && (ui == str.length() - 1 || (str[ui + 1] != -66 && str[ui + 1] != -67 && str[ui + 1] != -68))) ui = string::npos;
		if(ui != string::npos) {
			for(size_t ui3 = 0; ui3 < q_end.size(); ui3++) {
				if(ui <= q_end[ui3] && ui >= q_begin[ui3]) {
					ui = str.find('\xc2', q_end[ui3] + 1);std::cout << "5650" << endl;
					if(ui != string::npos && (ui == str.length() - 1 || (str[ui + 1] != -66 && str[ui + 1] != -67 && str[ui + 1] != -68))) ui = string::npos;
					if(ui == string::npos) break;
				}
			}
		}
		if(ui == string::npos) break;
		space_n = 0;std::cout << "5657" << endl;
		while(ui > 0 && ui - 1 - space_n != 0 && str[ui - 1 - space_n] == SPACE_CH) space_n++;
		bool b_add = (ui > 0 && is_in(NUMBER_ELEMENTS, str[ui - 1 - space_n]));std::cout << "5659" << endl;
		int index_shift = (b_add ? 6 : 5) - 2;
		for(size_t ui2 = 0; ui2 < q_begin.size(); ui2++) {
			if(q_begin[ui2] >= ui) {
				q_begin[ui2] += index_shift;
				q_end[ui2] += index_shift;
			}
		}
		if(str[ui + 1] == -66) str.replace(ui, 2, b_add ? "+(3/4)" : "(3/4)");
		else if(str[ui + 1] == -67) str.replace(ui, 2, b_add ? "+(1/2)" : "(1/2)");
		else if(str[ui + 1] == -68) str.replace(ui, 2, b_add ? "+(1/4)" : "(1/4)");
		if(b_add) prev_ui = ui + 6;
		else prev_ui = ui + 5;std::cout << "5671" << endl;
	}
	if(convert_to_internal_representation) {
		remove_blank_ends(str);std::cout << "5674" << endl;
		remove_duplicate_blanks(str);std::cout << "5675" << endl;
		for(size_t i = 0; i < INTERNAL_SIGNS_COUNT; i += 2) {
			size_t ui = str.find(internal_signs[i]);std::cout << "5677" << endl;
			size_t ui2 = 0;std::cout << "5678" << endl;
			while(ui != string::npos) {
				for(; ui2 < q_end.size(); ui2++) {
					if(ui >= q_begin[ui2]) {
						if(ui <= q_end[ui2]) {
							ui = str.find(internal_signs[i], q_end[ui2] + 1);std::cout << "5683" << endl;
							if(ui == string::npos) break;
						}
					} else {
						break;
					}
				}
				if(ui == string::npos) break;
				int index_shift = strlen(internal_signs[i + 1]) - strlen(internal_signs[i]);
				for(size_t ui3 = ui2; ui3 < q_begin.size(); ui3++) {
					q_begin[ui3] += index_shift;
					q_end[ui3] += index_shift;
				}
				str.replace(ui, strlen(internal_signs[i]), internal_signs[i + 1]);std::cout << "5696" << endl;
				ui = str.find(internal_signs[i], ui + strlen(internal_signs[i + 1]));std::cout << "5697" << endl;
			}
		}
	}
}


MathStructure Calculator::parse(string str, const ParseOptions &po) {

	MathStructure mstruct;std::cout << "5706" << endl;
	parse(&mstruct, str, po);std::cout << "5707" << endl;
	return mstruct;

}

void Calculator::parse(MathStructure *mstruct, string str, const ParseOptions &parseoptions) {

	ParseOptions po = parseoptions;std::cout << "5714" << endl;
	MathStructure *unended_function = po.unended_function;std::cout << "5715" << endl;
	po.unended_function = NULL;std::cout << "5716" << endl;

	if(po.base == BASE_UNICODE || (po.base == BASE_CUSTOM && priv->custom_input_base_i > 62)) {
		mstruct->set(Number(str, po));std::cout << "5719" << endl;
		return;
	}
	int base = po.base;std::cout << "5722" << endl;
	if(base == BASE_CUSTOM) {
		base = (int) priv->custom_input_base_i;std::cout << "5724" << endl;
	} else if(base == BASE_GOLDEN_RATIO || base == BASE_SUPER_GOLDEN_RATIO || base == BASE_SQRT2) {
		base = 2;std::cout << "5726" << endl;
	} else if(base == BASE_PI) {
		base = 4;std::cout << "5728" << endl;
	} else if(base == BASE_E) {
		base = 3;std::cout << "5730" << endl;
	} else if(base == BASE_DUODECIMAL) {
		base = -12;std::cout << "5732" << endl;
	} else if(base < 2 || base > 36) {
		base = -1;std::cout << "5734" << endl;
	}



	mstruct->clear();std::cout << "5739" << endl;

	const string *name = NULL;std::cout << "5741" << endl;
	string stmp, stmp2;std::cout << "5742" << endl;

	bool b_prime_quote = true;std::cout << "5744" << endl;

	size_t i_degree = str.find(SIGN_DEGREE);std::cout << "5746" << endl;
	if(i_degree != string::npos && i_degree < str.length() - strlen(SIGN_DEGREE) && is_not_in(NOT_IN_NAMES INTERNAL_OPERATORS NUMBER_ELEMENTS, str[i_degree + strlen(SIGN_DEGREE)])) i_degree = string::npos;

	if(base != -1 && base <= BASE_HEXADECIMAL) {
		if(i_degree == string::npos) {
			size_t i_quote = str.find('\'', 0);std::cout << "5751" << endl;
			size_t i_dquote = str.find('\"', 0);
			if(i_quote == 0 || i_dquote == 0) {
				b_prime_quote = false;std::cout << "5754" << endl;
			} else if((i_quote != string::npos && i_quote < str.length() - 1 && str.find('\'', i_quote + 1) != string::npos) || (i_quote != string::npos && i_dquote == i_quote + 1) || (i_dquote != string::npos && i_dquote < str.length() - 1 && str.find('\"', i_dquote + 1) != string::npos)) {
				b_prime_quote = false;std::cout << "5756" << endl;
				while(i_dquote != string::npos) {
					i_quote = str.rfind('\'', i_dquote - 1);std::cout << "5758" << endl;
					if(i_quote != string::npos) {
						size_t i_prev = str.find_last_not_of(SPACES, i_quote - 1);std::cout << "5760" << endl;
						if(i_prev != string::npos && is_in(NUMBER_ELEMENTS, str[i_prev])) {
							if(is_in(NUMBER_ELEMENTS, str[str.find_first_not_of(SPACES, i_quote + 1)]) && str.find_first_not_of(SPACES NUMBER_ELEMENTS, i_quote + 1) == i_dquote) {
								if(i_prev == 0) {
									b_prime_quote = true;std::cout << "5764" << endl;
									break;
								} else {
									i_prev = str.find_last_not_of(NUMBER_ELEMENTS, i_prev - 1);std::cout << "5767" << endl;
									if(i_prev == string::npos || (str[i_prev] != '\"' && str[i_prev] != '\'')) {
										b_prime_quote = true;std::cout << "5769" << endl;
										break;
									}
								}
							}
						}
					}
					i_dquote = str.find('\"', i_dquote + 2);
				}
			}
		}
		if(b_prime_quote) {
			gsub("\'", "′", str);
			gsub("\"", "″", str);
		}
	}

	parseSigns(str, true);std::cout << "5786" << endl;

	for(size_t str_index = 0; str_index < str.length(); str_index++) {
		if(str[str_index] == '\"' || str[str_index] == '\'') {
			if(str_index == str.length() - 1) {
				str.erase(str_index, 1);std::cout << "5791" << endl;
			} else {
				size_t i = str.find(str[str_index], str_index + 1);std::cout << "5793" << endl;
				size_t name_length;std::cout << "5794" << endl;
				if(i == string::npos) {
					i = str.length();std::cout << "5796" << endl;
					name_length = i - str_index;std::cout << "5797" << endl;
				} else {
					name_length = i - str_index + 1;std::cout << "5799" << endl;
				}
				stmp = LEFT_PARENTHESIS ID_WRAP_LEFT;std::cout << "5801" << endl;
				MathStructure *mstruct = new MathStructure(str.substr(str_index + 1, i - str_index - 1));std::cout << "5802" << endl;
				stmp += i2s(addId(mstruct));std::cout << "5803" << endl;
				stmp += ID_WRAP_RIGHT RIGHT_PARENTHESIS;std::cout << "5804" << endl;
				str.replace(str_index, name_length, stmp);std::cout << "5805" << endl;
				str_index += stmp.length() - 1;std::cout << "5806" << endl;
			}
		}
	}


	if(po.brackets_as_parentheses) {
		gsub(LEFT_VECTOR_WRAP, LEFT_PARENTHESIS, str);std::cout << "5813" << endl;
		gsub(RIGHT_VECTOR_WRAP, RIGHT_PARENTHESIS, str);std::cout << "5814" << endl;
	}


	size_t isave = 0;std::cout << "5818" << endl;
	if((isave = str.find(":=", 1)) != string::npos) {
		string name = str.substr(0, isave);std::cout << "5820" << endl;
		string value = str.substr(isave + 2, str.length() - (isave + 2));std::cout << "5821" << endl;
		str = value;std::cout << "5822" << endl;
		str += COMMA;std::cout << "5823" << endl;
		str += name;std::cout << "5824" << endl;
		f_save->parse(*mstruct, str, po);std::cout << "5825" << endl;
		return;
	}

	if(po.default_dataset != NULL && str.length() > 1) {
		size_t str_index = str.find(DOT_CH, 1);std::cout << "5830" << endl;
		while(str_index != string::npos) {
			if(str_index + 1 < str.length() && ((is_not_number(str[str_index + 1], base) && is_not_in(INTERNAL_OPERATORS NOT_IN_NAMES, str[str_index + 1]) && is_not_in(INTERNAL_OPERATORS NOT_IN_NAMES, str[str_index - 1])) || (is_not_in(INTERNAL_OPERATORS NOT_IN_NAMES, str[str_index + 1]) && is_not_number(str[str_index - 1], base) && is_not_in(INTERNAL_OPERATORS NOT_IN_NAMES, str[str_index - 1])))) {
				size_t dot_index = str.find_first_of(NOT_IN_NAMES INTERNAL_OPERATORS DOT, str_index + 1);std::cout << "5833" << endl;
				if(dot_index != string::npos && str[dot_index] == DOT_CH) {
					str_index = dot_index;std::cout << "5835" << endl;
				} else {
					size_t property_index = str.find_last_of(NOT_IN_NAMES INTERNAL_OPERATORS, str_index - 1);std::cout << "5837" << endl;
					if(property_index == string::npos) {
						str.insert(0, 1, '.');std::cout << "5839" << endl;
						str.insert(0, po.default_dataset->referenceName());std::cout << "5840" << endl;
						str_index += po.default_dataset->referenceName().length() + 1;std::cout << "5841" << endl;
					} else {
						str.insert(property_index + 1, 1, '.');std::cout << "5843" << endl;
						str.insert(property_index + 1, po.default_dataset->referenceName());std::cout << "5844" << endl;
						str_index += po.default_dataset->referenceName().length() + 1;std::cout << "5845" << endl;
					}
				}
			}
			str_index = str.find(DOT_CH, str_index + 1);std::cout << "5849" << endl;
		}
	}

	//remove spaces in numbers
	size_t space_i = 0;std::cout << "5854" << endl;
	if(!po.rpn) {
		space_i = str.find(SPACE_CH, 0);std::cout << "5856" << endl;
		while(space_i != string::npos) {
			if(is_in(NUMBERS INTERNAL_NUMBER_CHARS DOT, str[space_i + 1]) && is_in(NUMBERS INTERNAL_NUMBER_CHARS DOT, str[space_i - 1])) {
				str.erase(space_i, 1);std::cout << "5859" << endl;
				space_i--;std::cout << "5860" << endl;
			}
			space_i = str.find(SPACE_CH, space_i + 1);std::cout << "5862" << endl;
		}
	}

	if(base != -1 && base <= BASE_HEXADECIMAL) {
		bool b_degree = (i_degree != string::npos);std::cout << "5867" << endl;
		size_t i_quote = str.find("′");
		size_t i_dquote = str.find("″");
		while(i_quote != string::npos || i_dquote != string::npos) {
			size_t i_op = 0;std::cout << "5871" << endl;
			if(i_quote == string::npos || i_dquote < i_quote) {
				bool b = false;std::cout << "5873" << endl;
				if(b_degree) {
					i_degree = str.rfind(SIGN_DEGREE, i_dquote - 1);std::cout << "5875" << endl;
					if(i_degree != string::npos && i_degree > 0 && i_degree < i_dquote) {
						size_t i_op = str.find_first_not_of(SPACE, i_degree + strlen(SIGN_DEGREE));std::cout << "5877" << endl;
						if(i_op != string::npos) {
							i_op = str.find_first_not_of(SPACE, i_degree + strlen(SIGN_DEGREE));std::cout << "5879" << endl;
							if(is_in(NUMBER_ELEMENTS, str[i_op])) i_op = str.find_first_not_of(NUMBER_ELEMENTS SPACE, i_op);
							else i_op = 0;std::cout << "5881" << endl;
						}
						size_t i_prev = string::npos;std::cout << "5883" << endl;
						if(i_op == i_dquote) {
							i_prev = str.find_last_not_of(SPACE, i_degree - 1);std::cout << "5885" << endl;
							if(i_prev != string::npos) {
								if(is_in(NUMBER_ELEMENTS, str[i_prev])) {
									i_prev = str.find_last_not_of(NUMBER_ELEMENTS SPACE, i_prev);std::cout << "5888" << endl;
									if(i_prev == string::npos) i_prev = 0;
									else i_prev++;std::cout << "5890" << endl;
								} else {
									i_prev = string::npos;std::cout << "5892" << endl;
								}
							}
						}
						if(i_prev != string::npos) {
							str.insert(i_prev, LEFT_PARENTHESIS);std::cout << "5897" << endl;
							i_degree++;std::cout << "5898" << endl;
							i_op++;std::cout << "5899" << endl;
							str.replace(i_op, strlen("″"), "arcsec" RIGHT_PARENTHESIS);
							str.replace(i_degree, strlen(SIGN_DEGREE), "deg" PLUS);
							b = true;std::cout << "5902" << endl;
						}
					}
				}
				if(!b) {
					if(str.length() >= i_dquote + strlen("″") && is_in(NUMBERS, str[i_dquote + strlen("″")])) str.insert(i_dquote + strlen("″"), " ");
					str.replace(i_dquote, strlen("″"), b_degree ? "arcsec" : "in");
					i_op = i_dquote;std::cout << "5909" << endl;
				}
			} else {
				bool b = false;std::cout << "5912" << endl;
				if(b_degree) {
					i_degree = str.rfind(SIGN_DEGREE, i_quote - 1);std::cout << "5914" << endl;
					if(i_degree != string::npos && i_degree > 0 && i_degree < i_quote) {
						size_t i_op = str.find_first_not_of(SPACE, i_degree + strlen(SIGN_DEGREE));std::cout << "5916" << endl;
						if(i_op != string::npos) {
							i_op = str.find_first_not_of(SPACE, i_degree + strlen(SIGN_DEGREE));std::cout << "5918" << endl;
							if(is_in(NUMBER_ELEMENTS, str[i_op])) i_op = str.find_first_not_of(NUMBER_ELEMENTS SPACE, i_op);
							else i_op = 0;std::cout << "5920" << endl;
						}
						size_t i_prev = string::npos;std::cout << "5922" << endl;
						if(i_op == i_quote) {
							i_prev = str.find_last_not_of(SPACE, i_degree - 1);std::cout << "5924" << endl;
							if(i_prev != string::npos) {
								if(is_in(NUMBER_ELEMENTS, str[i_prev])) {
									i_prev = str.find_last_not_of(NUMBER_ELEMENTS SPACE, i_prev);std::cout << "5927" << endl;
									if(i_prev == string::npos) i_prev = 0;
									else i_prev++;std::cout << "5929" << endl;
								} else {
									i_prev = string::npos;std::cout << "5931" << endl;
								}
							}
						}
						if(i_prev != string::npos) {
							str.insert(i_prev, LEFT_PARENTHESIS);std::cout << "5936" << endl;
							i_degree++;std::cout << "5937" << endl;
							i_quote++;std::cout << "5938" << endl;
							i_op++;std::cout << "5939" << endl;
							if(i_dquote != string::npos) {
								i_dquote++;std::cout << "5941" << endl;
								size_t i_op2 = str.find_first_not_of(SPACE, i_quote + strlen("′"));
								if(i_op2 != string::npos && is_in(NUMBER_ELEMENTS, str[i_op2])) i_op2 = str.find_first_not_of(NUMBER_ELEMENTS SPACE, i_op2);
								else i_op2 = 0;std::cout << "5944" << endl;
								if(i_op2 == i_dquote) {
									str.replace(i_dquote, strlen("″"), "arcsec" RIGHT_PARENTHESIS);
									i_op = i_op2;std::cout << "5947" << endl;
								}
							}
							str.replace(i_quote, strlen("′"), i_op == i_quote ? "arcmin" RIGHT_PARENTHESIS : "arcmin" PLUS);
							str.replace(i_degree, strlen(SIGN_DEGREE), "deg" PLUS);
							b = true;std::cout << "5952" << endl;
						}
					}
				}
				if(!b) {
					i_op = str.find_first_not_of(SPACE, i_quote + strlen("′"));
					if(i_op != string::npos && is_in(NUMBER_ELEMENTS, str[i_op])) i_op = str.find_first_not_of(NUMBER_ELEMENTS SPACE, i_op);
					else i_op = 0;std::cout << "5959" << endl;
					size_t i_prev = string::npos;std::cout << "5960" << endl;
					if(((!b_degree && i_op == string::npos) || i_op == i_dquote) && i_quote != 0) {
						i_prev = str.find_last_not_of(SPACE, i_quote - 1);std::cout << "5962" << endl;
						if(i_prev != string::npos) {
							if(is_in(NUMBER_ELEMENTS, str[i_prev])) {
								i_prev = str.find_last_not_of(NUMBER_ELEMENTS SPACE, i_prev);std::cout << "5965" << endl;
								if(i_prev == string::npos) i_prev = 0;
								else i_prev++;std::cout << "5967" << endl;
							} else {
								i_prev = string::npos;std::cout << "5969" << endl;
							}
						}
					}
					if(i_prev != string::npos) {
						str.insert(i_prev, LEFT_PARENTHESIS);std::cout << "5974" << endl;
						i_quote++;std::cout << "5975" << endl;
						if(i_op == string::npos) str += b_degree ? "arcsec" RIGHT_PARENTHESIS : "in" RIGHT_PARENTHESIS;
						else str.replace(i_op + 1, strlen("″"), b_degree ? "arcsec" RIGHT_PARENTHESIS : "in" RIGHT_PARENTHESIS);
						str.replace(i_quote, strlen("′"), b_degree ? "arcmin" PLUS : "ft" PLUS);
						if(i_op == string::npos) break;
						i_op++;std::cout << "5980" << endl;
					} else {
						if(str.length() >= i_quote + strlen("′") && is_in(NUMBERS, str[i_quote + strlen("′")])) str.insert(i_quote + strlen("′"), " ");
						str.replace(i_quote, strlen("′"), b_degree ? "arcmin" : "ft");
						i_op = i_quote;std::cout << "5984" << endl;
					}
				}
			}
			if(i_dquote != string::npos) i_dquote = str.find("″", i_op);
			if(i_quote != string::npos) i_quote = str.find("′", i_op);
		}
	}

	size_t i_mod = str.find("%");
	if(i_mod != string::npos && !v_percent->hasName("%")) i_mod = string::npos;
	while(i_mod != string::npos) {
		if(po.rpn) {
			if(i_mod == 0 || is_not_in(OPERATORS "\\" INTERNAL_OPERATORS SPACE, str[i_mod - 1])) {
				str.replace(i_mod, 1, v_percent->referenceName());std::cout << "5998" << endl;
				i_mod += v_percent->referenceName().length() - 1;std::cout << "5999" << endl;
			}
		} else if(i_mod == 0 || i_mod == str.length() - 1 || (is_in(RIGHT_PARENTHESIS RIGHT_VECTOR_WRAP COMMA OPERATORS "%\a\b", str[i_mod + 1]) && str[i_mod + 1] != BITWISE_NOT_CH && str[i_mod + 1] != NOT_CH) || is_in(LEFT_PARENTHESIS LEFT_VECTOR_WRAP COMMA OPERATORS "\a\b", str[i_mod - 1])) {
			str.replace(i_mod, 1, v_percent->referenceName());std::cout << "6002" << endl;
			i_mod += v_percent->referenceName().length() - 1;std::cout << "6003" << endl;
		}
		i_mod = str.find("%", i_mod + 1);
	}

	if(po.rpn) {
		gsub("&&", "& &", str);
		gsub("||", "| |", str);
		gsub("\\%\\%", "\\% \\%", str);
	}

	for(size_t str_index = 0; str_index < str.length(); str_index++) {
		if(str[str_index] == LEFT_VECTOR_WRAP_CH) {
			int i4 = 1;std::cout << "6016" << endl;
			size_t i3 = str_index;std::cout << "6017" << endl;
			while(true) {
				i3 = str.find_first_of(LEFT_VECTOR_WRAP RIGHT_VECTOR_WRAP, i3 + 1);std::cout << "6019" << endl;
				if(i3 == string::npos) {
					for(; i4 > 0; i4--) {
						str += RIGHT_VECTOR_WRAP;std::cout << "6022" << endl;
					}
					i3 = str.length() - 1;std::cout << "6024" << endl;
				} else if(str[i3] == LEFT_VECTOR_WRAP_CH) {
					i4++;std::cout << "6026" << endl;
				} else if(str[i3] == RIGHT_VECTOR_WRAP_CH) {
					i4--;std::cout << "6028" << endl;
					if(i4 > 0) {
						size_t i5 = str.find_first_not_of(SPACE, i3 + 1);std::cout << "6030" << endl;
						if(i5 != string::npos && str[i5] == LEFT_VECTOR_WRAP_CH) {
							str.insert(i5, COMMA);std::cout << "6032" << endl;
						}
					}
				}
				if(i4 == 0) {
					stmp2 = str.substr(str_index + 1, i3 - str_index - 1);std::cout << "6037" << endl;
					stmp = LEFT_PARENTHESIS ID_WRAP_LEFT;std::cout << "6038" << endl;
					stmp += i2s(parseAddVectorId(stmp2, po));std::cout << "6039" << endl;
					stmp += ID_WRAP_RIGHT RIGHT_PARENTHESIS;std::cout << "6040" << endl;
					str.replace(str_index, i3 + 1 - str_index, stmp);std::cout << "6041" << endl;
					str_index += stmp.length() - 1;std::cout << "6042" << endl;
					break;
				}
			}
		} else if(str[str_index] == '\\' && str_index + 1 < str.length() && (is_not_in(NOT_IN_NAMES INTERNAL_OPERATORS NUMBERS, str[str_index + 1]) || (!po.rpn && str_index > 0 && is_in(NUMBERS SPACE PLUS MINUS BITWISE_NOT NOT LEFT_PARENTHESIS, str[str_index + 1])))) {
			if(is_in(NUMBERS SPACE PLUS MINUS BITWISE_NOT NOT LEFT_PARENTHESIS, str[str_index + 1])) {
				str.replace(str_index, 1, "//");
				str_index++;std::cout << "6049" << endl;
			} else {
				stmp = LEFT_PARENTHESIS ID_WRAP_LEFT;std::cout << "6051" << endl;
				size_t l = 1;std::cout << "6052" << endl;
				if(str[str_index + l] < 0) {
					do {
						l++;std::cout << "6055" << endl;
					} while(str_index + l < str.length() && str[str_index + l] < 0 && (unsigned char) str[str_index + l] < 0xC0);
					l--;std::cout << "6057" << endl;
				}
				MathStructure *mstruct = new MathStructure(str.substr(str_index + 1, l));std::cout << "6059" << endl;
				stmp += i2s(addId(mstruct));std::cout << "6060" << endl;
				stmp += ID_WRAP_RIGHT RIGHT_PARENTHESIS;std::cout << "6061" << endl;
				str.replace(str_index, l + 1, stmp);std::cout << "6062" << endl;
				str_index += stmp.length() - l;std::cout << "6063" << endl;
			}
		} else if(str[str_index] == '!' && po.functions_enabled) {
			if(str_index > 0 && (str.length() - str_index == 1 || str[str_index + 1] != EQUALS_CH)) {
				stmp2 = "";
				size_t i5 = str.find_last_not_of(SPACE, str_index - 1);std::cout << "6068" << endl;
				size_t i3;std::cout << "6069" << endl;
				if(i5 == string::npos) {
				} else if(str[i5] == RIGHT_PARENTHESIS_CH) {
					if(i5 == 0) {
						stmp2 = str.substr(0, i5 + 1);std::cout << "6073" << endl;
					} else {
						i3 = i5 - 1;std::cout << "6075" << endl;
						size_t i4 = 1;std::cout << "6076" << endl;
						while(true) {
							i3 = str.find_last_of(LEFT_PARENTHESIS RIGHT_PARENTHESIS, i3);std::cout << "6078" << endl;
							if(i3 == string::npos) {
								stmp2 = str.substr(0, i5 + 1);std::cout << "6080" << endl;
								break;
							}
							if(str[i3] == RIGHT_PARENTHESIS_CH) {
								i4++;std::cout << "6084" << endl;
							} else {
								i4--;std::cout << "6086" << endl;
								if(i4 == 0) {
									stmp2 = str.substr(i3, i5 + 1 - i3);std::cout << "6088" << endl;
									break;
								}
							}
							if(i3 == 0) {
								stmp2 = str.substr(0, i5 + 1);std::cout << "6093" << endl;
								break;
							}
							i3--;std::cout << "6096" << endl;
						}
					}
				} else if(str[i5] == ID_WRAP_RIGHT_CH && (i3 = str.find_last_of(ID_WRAP_LEFT, i5 - 1)) != string::npos) {
					stmp2 = str.substr(i3, i5 + 1 - i3);std::cout << "6100" << endl;
				} else if(is_not_in(RESERVED OPERATORS INTERNAL_OPERATORS SPACES VECTOR_WRAPS PARENTHESISS COMMAS, str[i5])) {
					i3 = str.find_last_of(RESERVED OPERATORS INTERNAL_OPERATORS SPACES VECTOR_WRAPS PARENTHESISS COMMAS, i5);std::cout << "6102" << endl;
					if(i3 == string::npos) {
						stmp2 = str.substr(0, i5 + 1);std::cout << "6104" << endl;
					} else {
						stmp2 = str.substr(i3 + 1, i5 - i3);std::cout << "6106" << endl;
					}
				}
				if(!stmp2.empty()) {
					stmp = LEFT_PARENTHESIS ID_WRAP_LEFT;std::cout << "6110" << endl;
					int ifac = 1;
					i3 = str_index + 1;std::cout << "6112" << endl;
					size_t i4 = i3;std::cout << "6113" << endl;
					while((i3 = str.find_first_not_of(SPACE, i3)) != string::npos && str[i3] == '!') {
						ifac++;
						i3++;std::cout << "6116" << endl;
						i4 = i3;std::cout << "6117" << endl;
					}
					if(ifac == 2) stmp += i2s(parseAddId(f_factorial2, stmp2, po));
					else if(ifac == 1) stmp += i2s(parseAddId(f_factorial, stmp2, po));
					else stmp += i2s(parseAddIdAppend(f_multifactorial, MathStructure(ifac, 1, 0), stmp2, po));
					stmp += ID_WRAP_RIGHT RIGHT_PARENTHESIS;std::cout << "6122" << endl;
					str.replace(i5 - stmp2.length() + 1, stmp2.length() + i4 - i5 - 1, stmp);std::cout << "6123" << endl;
					str_index = stmp.length() + i5 - stmp2.length();std::cout << "6124" << endl;
				}
			}
		} else if(!po.rpn && (str[str_index] == 'c' || str[str_index] == 'C') && str.length() > str_index + 6 && str[str_index + 5] == SPACE_CH && (str_index == 0 || is_in(OPERATORS INTERNAL_OPERATORS PARENTHESISS, str[str_index - 1])) && compare_name_no_case("compl", str, 5, str_index, base)) {
			str.replace(str_index, 6, BITWISE_NOT);std::cout << "6128" << endl;
		} else if(str[str_index] == SPACE_CH) {
			size_t i = str.find(SPACE, str_index + 1);std::cout << "6130" << endl;
			if(po.rpn && i == string::npos) i = str.length();
			if(i != string::npos) {
				i -= str_index + 1;std::cout << "6133" << endl;
				size_t il = 0;std::cout << "6134" << endl;
				if(i == per_str_len && (il = compare_name_no_case(per_str, str, per_str_len, str_index + 1, base))) {
					str.replace(str_index + 1, il, DIVISION);std::cout << "6136" << endl;
					str_index++;std::cout << "6137" << endl;
				} else if(i == times_str_len && (il = compare_name_no_case(times_str, str, times_str_len, str_index + 1, base))) {
					str.replace(str_index + 1, il, MULTIPLICATION);std::cout << "6139" << endl;
					str_index++;std::cout << "6140" << endl;
				} else if(i == plus_str_len && (il = compare_name_no_case(plus_str, str, plus_str_len, str_index + 1, base))) {
					str.replace(str_index + 1, il, PLUS);std::cout << "6142" << endl;
					str_index++;std::cout << "6143" << endl;
				} else if(i == minus_str_len && (il = compare_name_no_case(minus_str, str, minus_str_len, str_index + 1, base))) {
					str.replace(str_index + 1, il, MINUS);std::cout << "6145" << endl;
					str_index++;std::cout << "6146" << endl;
				} else if(and_str_len > 0 && i == and_str_len && (il = compare_name_no_case(and_str, str, and_str_len, str_index + 1, base))) {
					str.replace(str_index + 1, il, LOGICAL_AND);std::cout << "6148" << endl;
					str_index += 2;std::cout << "6149" << endl;
				} else if(i == AND_str_len && (il = compare_name_no_case(AND_str, str, AND_str_len, str_index + 1, base))) {
					str.replace(str_index + 1, il, LOGICAL_AND);std::cout << "6151" << endl;
					str_index += 2;std::cout << "6152" << endl;
				} else if(or_str_len > 0 && i == or_str_len && (il = compare_name_no_case(or_str, str, or_str_len, str_index + 1, base))) {
					str.replace(str_index + 1, il, LOGICAL_OR);std::cout << "6154" << endl;
					str_index += 2;std::cout << "6155" << endl;
				} else if(i == OR_str_len && (il = compare_name_no_case(OR_str, str, OR_str_len, str_index + 1, base))) {
					str.replace(str_index + 1, il, LOGICAL_OR);std::cout << "6157" << endl;
					str_index += 2;std::cout << "6158" << endl;
				} else if(i == XOR_str_len && (il = compare_name_no_case(XOR_str, str, XOR_str_len, str_index + 1, base))) {
					str.replace(str_index + 1, il, "\a");
					str_index++;std::cout << "6161" << endl;
				} else if(i == 5 && (il = compare_name_no_case("bitor", str, 5, str_index + 1, base))) {
					str.replace(str_index + 1, il, BITWISE_OR);std::cout << "6163" << endl;
					str_index++;std::cout << "6164" << endl;
				} else if(i == 6 && (il = compare_name_no_case("bitand", str, 6, str_index + 1, base))) {
					str.replace(str_index + 1, il, BITWISE_AND);std::cout << "6166" << endl;
					str_index++;std::cout << "6167" << endl;
				} else if(i == 3 && (il = compare_name_no_case("mod", str, 3, str_index + 1, base))) {
					str.replace(str_index + 1, il, "\%\%");
					str_index += 2;std::cout << "6170" << endl;
				} else if(i == 3 && (il = compare_name_no_case("rem", str, 3, str_index + 1, base))) {
					str.replace(str_index + 1, il, "%");
					str_index++;std::cout << "6173" << endl;
				} else if(i == 3 && (il = compare_name_no_case("div", str, 3, str_index + 1, base))) {
					if(po.rpn) {
						str.replace(str_index + 1, il, "\\");
						str_index++;std::cout << "6177" << endl;
					} else {
						str.replace(str_index + 1, il, "//");
						str_index += 2;std::cout << "6180" << endl;
					}
				}
			}
		} else if(str_index > 0 && base >= 2 && base <= 10 && is_in(EXPS, str[str_index]) && str_index + 1 < str.length() && (is_in(NUMBER_ELEMENTS, str[str_index + 1]) || (is_in(PLUS MINUS, str[str_index + 1]) && str_index + 2 < str.length() && is_in(NUMBER_ELEMENTS, str[str_index + 2]))) && is_in(NUMBER_ELEMENTS, str[str_index - 1])) {
			//don't do anything when e is used instead of E for EXP
		} else if(base <= 33 && str[str_index] == '0' && (str_index == 0 || is_in(NOT_IN_NAMES INTERNAL_OPERATORS, str[str_index - 1]))) {
			if(str_index + 2 < str.length() && (str[str_index + 1] == 'x' || str[str_index + 1] == 'X') && is_in(NUMBER_ELEMENTS "abcdefABCDEF", str[str_index + 2])) {
				//hexadecimal number 0x...
				if(po.base == BASE_HEXADECIMAL) {
					str.erase(str_index, 2);std::cout << "6190" << endl;
				} else {
					size_t i;std::cout << "6192" << endl;
					if(po.rpn) i = str.find_first_not_of(NUMBER_ELEMENTS "abcdefABCDEF", str_index + 2);
					else i = str.find_first_not_of(SPACE NUMBER_ELEMENTS "abcdefABCDEF", str_index + 2);
					size_t name_length;std::cout << "6195" << endl;
					if(i == string::npos) i = str.length();
					while(str[i - 1] == SPACE_CH) i--;
					name_length = i - str_index;std::cout << "6198" << endl;
					ParseOptions po_hex = po;std::cout << "6199" << endl;
					po_hex.base = BASE_HEXADECIMAL;std::cout << "6200" << endl;
					stmp = LEFT_PARENTHESIS ID_WRAP_LEFT;std::cout << "6201" << endl;
					MathStructure *mstruct = new MathStructure(Number(str.substr(str_index, i - str_index), po_hex));std::cout << "6202" << endl;
					stmp += i2s(addId(mstruct));std::cout << "6203" << endl;
					stmp += ID_WRAP_RIGHT RIGHT_PARENTHESIS;std::cout << "6204" << endl;
					str.replace(str_index, name_length, stmp);std::cout << "6205" << endl;
					str_index += stmp.length() - 1;std::cout << "6206" << endl;
				}

			} else if(base <= 12 && str_index + 2 < str.length() && (str[str_index + 1] == 'b' || str[str_index + 1] == 'B') && is_in("01", str[str_index + 2])) {
				//binary number 0b...
				if(po.base == BASE_BINARY) {
					str.erase(str_index, 2);std::cout << "6212" << endl;
				} else {
					size_t i;std::cout << "6214" << endl;
					if(po.rpn) i = str.find_first_not_of(NUMBER_ELEMENTS, str_index + 2);
					else i = str.find_first_not_of(SPACE NUMBER_ELEMENTS, str_index + 2);std::cout << "6216" << endl;
					size_t name_length;std::cout << "6217" << endl;
					if(i == string::npos) i = str.length();
					while(str[i - 1] == SPACE_CH) i--;
					name_length = i - str_index;std::cout << "6220" << endl;
					ParseOptions po_bin = po;std::cout << "6221" << endl;
					po_bin.base = BASE_BINARY;std::cout << "6222" << endl;
					stmp = LEFT_PARENTHESIS ID_WRAP_LEFT;std::cout << "6223" << endl;
					MathStructure *mstruct = new MathStructure(Number(str.substr(str_index, i - str_index), po_bin));std::cout << "6224" << endl;
					stmp += i2s(addId(mstruct));std::cout << "6225" << endl;
					stmp += ID_WRAP_RIGHT RIGHT_PARENTHESIS;std::cout << "6226" << endl;
					str.replace(str_index, name_length, stmp);std::cout << "6227" << endl;
					str_index += stmp.length() - 1;std::cout << "6228" << endl;
				}
			} else if(base <= 24 && str_index + 2 < str.length() && (str[str_index + 1] == 'o' || str[str_index + 1] == 'O') && is_in(NUMBERS, str[str_index + 2])) {
				//octal number 0o...
				if(po.base == BASE_OCTAL) {
					str.erase(str_index, 2);std::cout << "6233" << endl;
				} else {
					size_t i;std::cout << "6235" << endl;
					if(po.rpn) i = str.find_first_not_of(NUMBER_ELEMENTS, str_index + 2);
					else i = str.find_first_not_of(SPACE NUMBER_ELEMENTS, str_index + 2);std::cout << "6237" << endl;
					size_t name_length;std::cout << "6238" << endl;
					if(i == string::npos) i = str.length();
					while(str[i - 1] == SPACE_CH) i--;
					name_length = i - str_index;std::cout << "6241" << endl;
					ParseOptions po_oct = po;std::cout << "6242" << endl;
					po_oct.base = BASE_OCTAL;std::cout << "6243" << endl;
					stmp = LEFT_PARENTHESIS ID_WRAP_LEFT;std::cout << "6244" << endl;
					MathStructure *mstruct = new MathStructure(Number(str.substr(str_index, i - str_index), po_oct));std::cout << "6245" << endl;
					stmp += i2s(addId(mstruct));std::cout << "6246" << endl;
					stmp += ID_WRAP_RIGHT RIGHT_PARENTHESIS;std::cout << "6247" << endl;
					str.replace(str_index, name_length, stmp);std::cout << "6248" << endl;
					str_index += stmp.length() - 1;std::cout << "6249" << endl;
				}
			}
		} else if(is_not_in(NUMBERS INTERNAL_OPERATORS NOT_IN_NAMES, str[str_index])) {
			bool p_mode = false;std::cout << "6253" << endl;
			void *best_p_object = NULL;std::cout << "6254" << endl;
			Prefix *best_p = NULL;std::cout << "6255" << endl;
			size_t best_pl = 0;std::cout << "6256" << endl;
			size_t best_pnl = 0;std::cout << "6257" << endl;
			bool moved_forward = false;
			const string *found_function_name = NULL;std::cout << "6259" << endl;
			bool case_sensitive = false;std::cout << "6260" << endl;
			size_t found_function_name_length = 0;std::cout << "6261" << endl;
			void *found_function = NULL, *object = NULL;std::cout << "6262" << endl;
			int vt2 = -1;std::cout << "6263" << endl;
			size_t ufv_index;std::cout << "6264" << endl;
			size_t name_length;std::cout << "6265" << endl;
			size_t vt3 = 0;std::cout << "6266" << endl;
			char ufvt = 0;std::cout << "6267" << endl;
			size_t last_name_char = str.find_first_of(NOT_IN_NAMES INTERNAL_OPERATORS, str_index + 1);std::cout << "6268" << endl;
			if(last_name_char == string::npos) {
				last_name_char = str.length() - 1;std::cout << "6270" << endl;
			} else {
				last_name_char--;std::cout << "6272" << endl;
			}
			size_t last_unit_char = str.find_last_not_of(NUMBERS, last_name_char);std::cout << "6274" << endl;
			size_t name_chars_left = last_name_char - str_index + 1;std::cout << "6275" << endl;
			size_t unit_chars_left = last_unit_char - str_index + 1;std::cout << "6276" << endl;
			if(name_chars_left <= UFV_LENGTHS) {
				ufv_index = name_chars_left - 1;std::cout << "6278" << endl;
				vt2 = 0;std::cout << "6279" << endl;
			} else {
				ufv_index = 0;std::cout << "6281" << endl;
			}
			Prefix *p = NULL;std::cout << "6283" << endl;
			while(vt2 < 4) {
				name = NULL;std::cout << "6285" << endl;
				p = NULL;std::cout << "6286" << endl;
				switch(vt2) {
					case -1: {
						if(ufv_index < ufvl.size()) {
							switch(ufvl_t[ufv_index]) {
								case 'v': {
									if(po.variables_enabled && !p_mode) {
										name = &((ExpressionItem*) ufvl[ufv_index])->getName(ufvl_i[ufv_index]).name;std::cout << "6293" << endl;
										case_sensitive = ((ExpressionItem*) ufvl[ufv_index])->getName(ufvl_i[ufv_index]).case_sensitive;std::cout << "6294" << endl;
										name_length = name->length();std::cout << "6295" << endl;
										if(name_length < found_function_name_length) {
											name = NULL;std::cout << "6297" << endl;
										} else if(po.limit_implicit_multiplication) {
											if(name_length != name_chars_left && name_length != unit_chars_left) name = NULL;
										} else if(name_length > name_chars_left) {
											name = NULL;std::cout << "6301" << endl;
										}
									}
									break;
								}
								case 'f': {
									if(po.functions_enabled && !found_function_name && !p_mode) {
										name = &((ExpressionItem*) ufvl[ufv_index])->getName(ufvl_i[ufv_index]).name;std::cout << "6308" << endl;
										case_sensitive = ((ExpressionItem*) ufvl[ufv_index])->getName(ufvl_i[ufv_index]).case_sensitive;std::cout << "6309" << endl;
										name_length = name->length();std::cout << "6310" << endl;
										if(po.limit_implicit_multiplication) {
											if(name_length != name_chars_left && name_length != unit_chars_left) name = NULL;
										} else if(name_length > name_chars_left || name_length < found_function_name_length) {
											name = NULL;std::cout << "6314" << endl;
										}
									}
									break;
								}
								case 'u': {
									if(po.units_enabled && !p_mode) {
										name = &((ExpressionItem*) ufvl[ufv_index])->getName(ufvl_i[ufv_index]).name;std::cout << "6321" << endl;
										case_sensitive = ((ExpressionItem*) ufvl[ufv_index])->getName(ufvl_i[ufv_index]).case_sensitive;std::cout << "6322" << endl;
										name_length = name->length();std::cout << "6323" << endl;
										if(name_length < found_function_name_length) {
											name = NULL;std::cout << "6325" << endl;
										} else if(po.limit_implicit_multiplication || ((ExpressionItem*) ufvl[ufv_index])->getName(ufvl_i[ufv_index]).plural) {
											if(name_length != unit_chars_left) name = NULL;
										} else if(name_length > unit_chars_left) {
											name = NULL;std::cout << "6329" << endl;
										}
									}
									break;
								}
								case 'p': {
									if(!p && po.units_enabled) {
										name = &((Prefix*) ufvl[ufv_index])->shortName();std::cout << "6336" << endl;
										name_length = name->length();std::cout << "6337" << endl;
										if(name_length >= unit_chars_left || name_length < found_function_name_length) {
											name = NULL;std::cout << "6339" << endl;
										}
									}
									case_sensitive = true;std::cout << "6342" << endl;
									break;
								}
								case 'P': {
									if(!p && po.units_enabled) {
										name = &((Prefix*) ufvl[ufv_index])->longName();std::cout << "6347" << endl;
										name_length = name->length();std::cout << "6348" << endl;
										if(name_length >= unit_chars_left || name_length < found_function_name_length) {
											name = NULL;std::cout << "6350" << endl;
										}
									}
									case_sensitive = false;std::cout << "6353" << endl;
									break;
								}
								case 'q': {
									if(!p && po.units_enabled) {
										name = &((Prefix*) ufvl[ufv_index])->unicodeName();std::cout << "6358" << endl;
										name_length = name->length();std::cout << "6359" << endl;
										if(name_length >= unit_chars_left || name_length < found_function_name_length) {
											name = NULL;std::cout << "6361" << endl;
										}
									}
									case_sensitive = true;std::cout << "6364" << endl;
									break;
								}
							}
							ufvt = ufvl_t[ufv_index];std::cout << "6368" << endl;
							object = ufvl[ufv_index];std::cout << "6369" << endl;
							ufv_index++;std::cout << "6370" << endl;
							break;
						} else {
							if(found_function_name) {
								vt2 = 4;std::cout << "6374" << endl;
								break;
							}
							vt2 = 0;std::cout << "6377" << endl;
							vt3 = 0;std::cout << "6378" << endl;
							if(po.limit_implicit_multiplication && unit_chars_left <= UFV_LENGTHS) {
								ufv_index = unit_chars_left - 1;std::cout << "6380" << endl;
							} else {
								ufv_index = UFV_LENGTHS - 1;std::cout << "6382" << endl;
							}
						}
					}
					case 0: {
						if(po.units_enabled && ufv_index < unit_chars_left - 1 && vt3 < ufv[vt2][ufv_index].size()) {
							object = ufv[vt2][ufv_index][vt3];std::cout << "6388" << endl;
							switch(ufv_i[vt2][ufv_index][vt3]) {
								case 1: {
									ufvt = 'P';std::cout << "6391" << endl;
									name = &((Prefix*) object)->longName();std::cout << "6392" << endl;
									name_length = name->length();std::cout << "6393" << endl;
									case_sensitive = false;std::cout << "6394" << endl;
									break;
								}
								case 2: {
									ufvt = 'p';std::cout << "6398" << endl;
									name = &((Prefix*) object)->shortName();std::cout << "6399" << endl;
									name_length = name->length();std::cout << "6400" << endl;
									case_sensitive = true;std::cout << "6401" << endl;
									break;
								}
								case 3: {
									ufvt = 'q';std::cout << "6405" << endl;
									name = &((Prefix*) object)->unicodeName();std::cout << "6406" << endl;
									name_length = name->length();std::cout << "6407" << endl;
									case_sensitive = true;std::cout << "6408" << endl;
									break;
								}
							}
							vt3++;std::cout << "6412" << endl;
							break;
						}
						vt2 = 1;std::cout << "6415" << endl;
						vt3 = 0;std::cout << "6416" << endl;
					}
					case 1: {
						if(!found_function_name && po.functions_enabled && !p_mode && (!po.limit_implicit_multiplication || ufv_index + 1 == unit_chars_left || ufv_index + 1 == name_chars_left) && vt3 < ufv[vt2][ufv_index].size()) {
							object = ufv[vt2][ufv_index][vt3];std::cout << "6420" << endl;
							ufvt = 'f';std::cout << "6421" << endl;
							name = &((MathFunction*) object)->getName(ufv_i[vt2][ufv_index][vt3]).name;std::cout << "6422" << endl;
							name_length = name->length();std::cout << "6423" << endl;
							case_sensitive = ((MathFunction*) object)->getName(ufv_i[vt2][ufv_index][vt3]).case_sensitive;std::cout << "6424" << endl;
							vt3++;std::cout << "6425" << endl;
							break;
						}
						vt2 = 2;std::cout << "6428" << endl;
						vt3 = 0;std::cout << "6429" << endl;
					}
					case 2: {
						if(po.units_enabled && !p_mode && (!po.limit_implicit_multiplication || ufv_index + 1 == unit_chars_left) && ufv_index < unit_chars_left && vt3 < ufv[vt2][ufv_index].size()) {
							object = ufv[vt2][ufv_index][vt3];std::cout << "6433" << endl;
							if(ufv_index + 1 == unit_chars_left || !((Unit*) object)->getName(ufv_i[vt2][ufv_index][vt3]).plural) {
								ufvt = 'u';std::cout << "6435" << endl;
								name = &((Unit*) object)->getName(ufv_i[vt2][ufv_index][vt3]).name;std::cout << "6436" << endl;
								name_length = name->length();std::cout << "6437" << endl;
								case_sensitive = ((Unit*) object)->getName(ufv_i[vt2][ufv_index][vt3]).case_sensitive;std::cout << "6438" << endl;
							}
							vt3++;std::cout << "6440" << endl;
							break;
						}
						vt2 = 3;std::cout << "6443" << endl;
						vt3 = 0;std::cout << "6444" << endl;
					}
					case 3: {
						if(po.variables_enabled && !p_mode && (!po.limit_implicit_multiplication || ufv_index + 1 == unit_chars_left || ufv_index + 1 == name_chars_left) && vt3 < ufv[vt2][ufv_index].size()) {
							object = ufv[vt2][ufv_index][vt3];std::cout << "6448" << endl;
							ufvt = 'v';std::cout << "6449" << endl;
							name = &((Variable*) object)->getName(ufv_i[vt2][ufv_index][vt3]).name;std::cout << "6450" << endl;
							name_length = name->length();std::cout << "6451" << endl;
							case_sensitive = ((Variable*) object)->getName(ufv_i[vt2][ufv_index][vt3]).case_sensitive;std::cout << "6452" << endl;
							vt3++;std::cout << "6453" << endl;
							break;
						}
						if(ufv_index == 0 || found_function_name) {
							vt2 = 4;std::cout << "6457" << endl;
						} else {
							ufv_index--;std::cout << "6459" << endl;
							vt3 = 0;std::cout << "6460" << endl;
							vt2 = 0;std::cout << "6461" << endl;
						}
					}
				}
				if(name && name_length >= found_function_name_length && ((case_sensitive && compare_name(*name, str, name_length, str_index, base)) || (!case_sensitive && (name_length = compare_name_no_case(*name, str, name_length, str_index, base))))) {
					moved_forward = false;
					switch(ufvt) {
						case 'v': {
							stmp = LEFT_PARENTHESIS ID_WRAP_LEFT;std::cout << "6469" << endl;
							stmp += i2s(addId(new MathStructure((Variable*) object)));std::cout << "6470" << endl;
							stmp += ID_WRAP_RIGHT RIGHT_PARENTHESIS;std::cout << "6471" << endl;
							str.replace(str_index, name_length, stmp);std::cout << "6472" << endl;
							str_index += stmp.length();std::cout << "6473" << endl;
							moved_forward = true;
							break;
						}
						case 'f': {
							if(((ExpressionItem*) object)->subtype() == SUBTYPE_DATA_SET && str[str_index + name_length] == DOT_CH) {
								str[str_index + name_length] = LEFT_PARENTHESIS_CH;std::cout << "6479" << endl;
								size_t dot2_index = str.find(DOT_CH, str_index + name_length + 1);std::cout << "6480" << endl;
								str[dot2_index] = COMMA_CH;std::cout << "6481" << endl;
								size_t end_index = str.find_first_of(NOT_IN_NAMES INTERNAL_OPERATORS, dot2_index + 1);std::cout << "6482" << endl;
								if(end_index == string::npos) str += RIGHT_PARENTHESIS_CH;
								else str.insert(end_index, 1, RIGHT_PARENTHESIS_CH);std::cout << "6484" << endl;
							}
							size_t not_space_index;std::cout << "6486" << endl;
							if((not_space_index = str.find_first_not_of(SPACES, str_index + name_length)) == string::npos || str[not_space_index] != LEFT_PARENTHESIS_CH) {
								found_function = object;std::cout << "6488" << endl;
								found_function_name = name;std::cout << "6489" << endl;
								found_function_name_length = name_length;std::cout << "6490" << endl;
								break;
							}
							set_function:
							MathFunction *f = (MathFunction*) object;std::cout << "6494" << endl;
							int i4 = -1;std::cout << "6495" << endl;
							size_t i6;std::cout << "6496" << endl;
							if(f->args() == 0) {
								size_t i7 = str.find_first_not_of(SPACES, str_index + name_length);std::cout << "6498" << endl;
								if(i7 != string::npos && str[i7] == LEFT_PARENTHESIS_CH) {
									i7 = str.find_first_not_of(SPACES, i7 + 1);std::cout << "6500" << endl;
									if(i7 != string::npos && str[i7] == RIGHT_PARENTHESIS_CH) {
										i4 = i7 - str_index + 1;std::cout << "6502" << endl;
									}
								}
								stmp = LEFT_PARENTHESIS ID_WRAP_LEFT;std::cout << "6505" << endl;
								stmp += i2s(parseAddId(f, empty_string, po));std::cout << "6506" << endl;
								stmp += ID_WRAP_RIGHT RIGHT_PARENTHESIS;std::cout << "6507" << endl;
								if(i4 < 0) i4 = name_length;
							} else if(po.rpn && f->args() == 1 && str_index > 0 && str[str_index - 1] != LEFT_PARENTHESIS_CH && (str_index + name_length >= str.length() || str[str_index + name_length] != LEFT_PARENTHESIS_CH) && (i6 = str.find_last_not_of(SPACE, str_index - 1)) != string::npos) {
								size_t i7 = i6;std::cout << "6510" << endl;
								int nr_of_p = 0, nr_of_op = 0;std::cout << "6511" << endl;
								bool b_started = false;std::cout << "6512" << endl;
								while(i7 != 0) {
									if(nr_of_p > 0) {
										if(str[i7] == LEFT_PARENTHESIS_CH) {
											nr_of_p--;std::cout << "6516" << endl;
											if(nr_of_p == 0 && nr_of_op == 0) break;
										} else if(str[i7] == RIGHT_PARENTHESIS_CH) {
											nr_of_p++;std::cout << "6519" << endl;
										}
									} else if(nr_of_p == 0 && is_in(OPERATORS INTERNAL_OPERATORS SPACE RIGHT_PARENTHESIS, str[i7])) {
										if(nr_of_op == 0 && b_started) {
											i7++;std::cout << "6523" << endl;
											break;
										} else {
											if(is_in(OPERATORS INTERNAL_OPERATORS, str[i7])) {
												nr_of_op++;std::cout << "6527" << endl;
												b_started = false;std::cout << "6528" << endl;
											} else if(str[i7] == RIGHT_PARENTHESIS_CH) {
												nr_of_p++;std::cout << "6530" << endl;
												b_started = true;std::cout << "6531" << endl;
											} else if(b_started) {
												nr_of_op--;std::cout << "6533" << endl;
												b_started = false;std::cout << "6534" << endl;
											}
										}
									} else {
										b_started = true;std::cout << "6538" << endl;
									}
									i7--;std::cout << "6540" << endl;
								}
								stmp2 = str.substr(i7, i6 - i7 + 1);std::cout << "6542" << endl;
								stmp = LEFT_PARENTHESIS ID_WRAP_LEFT;std::cout << "6543" << endl;
								if(f == f_vector) stmp += i2s(parseAddVectorId(stmp2, po));
								else stmp += i2s(parseAddId(f, stmp2, po));std::cout << "6545" << endl;
								stmp += ID_WRAP_RIGHT RIGHT_PARENTHESIS;std::cout << "6546" << endl;
								str.replace(i7, str_index + name_length - i7, stmp);std::cout << "6547" << endl;
								str_index += name_length;std::cout << "6548" << endl;
								moved_forward = true;
							} else {
								bool b = false, b_unended_function = false, b_comma_before = false, b_power_before = false;
								//bool b_space_first = false;std::cout << "6552" << endl;
								size_t i5 = 1;std::cout << "6553" << endl;
								int arg_i = f->args();std::cout << "6554" << endl;
								i6 = 0;std::cout << "6555" << endl;
								while(!b) {
									if(i6 + str_index + name_length >= str.length()) {
										b = true;std::cout << "6558" << endl;
										i5 = 2;std::cout << "6559" << endl;
										i6++;std::cout << "6560" << endl;
										b_unended_function = true;std::cout << "6561" << endl;
										break;
									} else {
										char c = str[str_index + name_length + i6];std::cout << "6564" << endl;
										if(c == LEFT_PARENTHESIS_CH) {
											if(i5 < 2) b = true;
											else if(i5 == 2 && po.parsing_mode == PARSING_MODE_CONVENTIONAL && !b_power_before) b = true;
											else i5++;std::cout << "6568" << endl;
										} else if(c == RIGHT_PARENTHESIS_CH) {
											if(i5 <= 2) b = true;
											else i5--;std::cout << "6571" << endl;
										} else if(c == POWER_CH) {
											if(i5 < 2) i5 = 2;
											b_power_before = true;
										} else if(!b_comma_before && !b_power_before && c == ' ' && arg_i <= 1) {
											//if(i5 < 2) b_space_first = true;
											if(i5 == 2) b = true;
										} else if(!b_comma_before && i5 == 2 && arg_i <= 1 && is_in(OPERATORS INTERNAL_OPERATORS, c) && c != POWER_CH) {
											b = true;std::cout << "6579" << endl;
										} else if(c == COMMA_CH) {
											if(i5 == 2) arg_i--;
											b_comma_before = true;
											if(i5 < 2) i5 = 2;
										} else if(i5 < 2) {
											i5 = 2;std::cout << "6585" << endl;
										}
										if(c != COMMA_CH && c != ' ') b_comma_before = false;
										if(c != POWER_CH && c != ' ') b_power_before = false;
									}
									i6++;std::cout << "6590" << endl;
								}
								if(b && i5 >= 2) {
									stmp2 = str.substr(str_index + name_length, i6 - 1);std::cout << "6593" << endl;
									stmp = LEFT_PARENTHESIS ID_WRAP_LEFT;std::cout << "6594" << endl;
									if(b_unended_function && unended_function) {
										po.unended_function = unended_function;std::cout << "6596" << endl;
									}
									if(f == f_vector) {
										stmp += i2s(parseAddVectorId(stmp2, po));std::cout << "6599" << endl;
									} else if((f == f_interval || f == f_uncertainty) && po.read_precision != DONT_READ_PRECISION) {
										ParseOptions po2 = po;std::cout << "6601" << endl;
										po2.read_precision = DONT_READ_PRECISION;std::cout << "6602" << endl;
										stmp += i2s(parseAddId(f, stmp2, po2));std::cout << "6603" << endl;
									} else {
										stmp += i2s(parseAddId(f, stmp2, po));std::cout << "6605" << endl;
									}
									po.unended_function = NULL;std::cout << "6607" << endl;
									stmp += ID_WRAP_RIGHT RIGHT_PARENTHESIS;std::cout << "6608" << endl;
									i4 = i6 + 1 + name_length - 2;std::cout << "6609" << endl;
									b = false;std::cout << "6610" << endl;
								}
								size_t i9 = i6;std::cout << "6612" << endl;
								if(b) {
									b = false;std::cout << "6614" << endl;
									i6 = i6 + 1 + str_index + name_length;std::cout << "6615" << endl;
									size_t i7 = i6 - 1;std::cout << "6616" << endl;
									size_t i8 = i7;std::cout << "6617" << endl;
									while(true) {
										i5 = str.find(RIGHT_PARENTHESIS_CH, i7);std::cout << "6619" << endl;
										if(i5 == string::npos) {
											b_unended_function = true;std::cout << "6621" << endl;
											//str.append(1, RIGHT_PARENTHESIS_CH);std::cout << "6622" << endl;
											//i5 = str.length() - 1;std::cout << "6623" << endl;
											i5 = str.length();std::cout << "6624" << endl;
										}
										if(i5 < (i6 = str.find(LEFT_PARENTHESIS_CH, i8)) || i6 == string::npos) {
											i6 = i5;std::cout << "6627" << endl;
											b = true;std::cout << "6628" << endl;
											break;
										}
										i7 = i5 + 1;std::cout << "6631" << endl;
										i8 = i6 + 1;std::cout << "6632" << endl;
									}
									if(!b) {
										b_unended_function = false;std::cout << "6635" << endl;
									}
								}
								if(b) {
									stmp2 = str.substr(str_index + name_length + i9, i6 - (str_index + name_length + i9));std::cout << "6639" << endl;
									stmp = LEFT_PARENTHESIS ID_WRAP_LEFT;std::cout << "6640" << endl;
									if(b_unended_function && unended_function) {
										po.unended_function = unended_function;std::cout << "6642" << endl;
									}
									if(f == f_vector) {
										stmp += i2s(parseAddVectorId(stmp2, po));std::cout << "6645" << endl;
									} else if((f == f_interval || f == f_uncertainty) && po.read_precision != DONT_READ_PRECISION) {
										ParseOptions po2 = po;std::cout << "6647" << endl;
										po2.read_precision = DONT_READ_PRECISION;std::cout << "6648" << endl;
										stmp += i2s(parseAddId(f, stmp2, po2));std::cout << "6649" << endl;
									} else {
										stmp += i2s(parseAddId(f, stmp2, po));std::cout << "6651" << endl;
									}
									po.unended_function = NULL;std::cout << "6653" << endl;
									stmp += ID_WRAP_RIGHT RIGHT_PARENTHESIS;std::cout << "6654" << endl;
									i4 = i6 + 1 - str_index;std::cout << "6655" << endl;
								}
							}
							if(i4 > 0) {
								str.replace(str_index, i4, stmp);std::cout << "6659" << endl;
								str_index += stmp.length();std::cout << "6660" << endl;
								moved_forward = true;
							}
							break;
						}
						case 'u': {
							replace_text_by_unit_place:
							if(str.length() > str_index + name_length && is_in("23", str[str_index + name_length]) && (str.length() == str_index + name_length + 1 || is_not_in(NUMBER_ELEMENTS, str[str_index + name_length + 1])) && *name != SIGN_DEGREE && !((Unit*) object)->isCurrency()) {
								str.insert(str_index + name_length, 1, POWER_CH);std::cout << "6668" << endl;
							}
							stmp = LEFT_PARENTHESIS ID_WRAP_LEFT;std::cout << "6670" << endl;
							stmp += i2s(addId(new MathStructure((Unit*) object, p)));std::cout << "6671" << endl;
							stmp += ID_WRAP_RIGHT RIGHT_PARENTHESIS;std::cout << "6672" << endl;
							str.replace(str_index, name_length, stmp);std::cout << "6673" << endl;
							str_index += stmp.length();std::cout << "6674" << endl;
							moved_forward = true;
							p = NULL;std::cout << "6676" << endl;
							break;
						}
						case 'p': {}
						case 'q': {}
						case 'P': {
							if(str_index + name_length == str.length() || is_in(NOT_IN_NAMES INTERNAL_OPERATORS, str[str_index + name_length])) {
								break;
							}
							p = (Prefix*) object;std::cout << "6685" << endl;
							str_index += name_length;std::cout << "6686" << endl;
							unit_chars_left = last_unit_char - str_index + 1;std::cout << "6687" << endl;
							size_t name_length_old = name_length;std::cout << "6688" << endl;
							int index = 0;std::cout << "6689" << endl;
							if(unit_chars_left > UFV_LENGTHS) {
								for(size_t ufv_index2 = 0; ufv_index2 < ufvl.size(); ufv_index2++) {
									name = NULL;std::cout << "6692" << endl;
									switch(ufvl_t[ufv_index2]) {
										case 'u': {
											name = &((Unit*) ufvl[ufv_index2])->getName(ufvl_i[ufv_index2]).name;std::cout << "6695" << endl;
											case_sensitive = ((Unit*) ufvl[ufv_index2])->getName(ufvl_i[ufv_index2]).case_sensitive;std::cout << "6696" << endl;
											name_length = name->length();std::cout << "6697" << endl;
											if(po.limit_implicit_multiplication || ((Unit*) ufvl[ufv_index2])->getName(ufvl_i[ufv_index2]).plural) {
												if(name_length != unit_chars_left) name = NULL;
											} else if(name_length > unit_chars_left) {
												name = NULL;std::cout << "6701" << endl;
											}
											break;
										}
									}
									if(name && ((case_sensitive && compare_name(*name, str, name_length, str_index, base)) || (!case_sensitive && (name_length = compare_name_no_case(*name, str, name_length, str_index, base))))) {
										if((!p_mode && name_length_old > 1) || (p_mode && (name_length + name_length_old > best_pl || ((ufvt != 'P' || !((Unit*) ufvl[ufv_index2])->getName(ufvl_i[ufv_index2]).abbreviation) && name_length + name_length_old == best_pl)))) {
											p_mode = true;std::cout << "6708" << endl;
											best_p = p;std::cout << "6709" << endl;
											best_p_object = ufvl[ufv_index2];std::cout << "6710" << endl;
											best_pl = name_length + name_length_old;std::cout << "6711" << endl;
											best_pnl = name_length_old;std::cout << "6712" << endl;
											index = -1;std::cout << "6713" << endl;
											break;
										}
										if(!p_mode) {
											str.erase(str_index - name_length_old, name_length_old);std::cout << "6717" << endl;
											str_index -= name_length_old;std::cout << "6718" << endl;
											object = ufvl[ufv_index2];std::cout << "6719" << endl;
											goto replace_text_by_unit_place;std::cout << "6720" << endl;
										}
									}
								}
							}
							if(index < 0) {
							} else if(UFV_LENGTHS >= unit_chars_left) {
								index = unit_chars_left - 1;std::cout << "6727" << endl;
							} else if(po.limit_implicit_multiplication) {
								index = -1;std::cout << "6729" << endl;
							} else {
								index = UFV_LENGTHS - 1;std::cout << "6731" << endl;
							}
							for(; index >= 0; index--) {
								for(size_t ufv_index2 = 0; ufv_index2 < ufv[2][index].size(); ufv_index2++) {
									name = &((Unit*) ufv[2][index][ufv_index2])->getName(ufv_i[2][index][ufv_index2]).name;std::cout << "6735" << endl;
									case_sensitive = ((Unit*) ufv[2][index][ufv_index2])->getName(ufv_i[2][index][ufv_index2]).case_sensitive;std::cout << "6736" << endl;
									name_length = name->length();std::cout << "6737" << endl;
									if(index + 1 == (int) unit_chars_left || !((Unit*) ufv[2][index][ufv_index2])->getName(ufv_i[2][index][ufv_index2]).plural) {
										if(name_length <= unit_chars_left && ((case_sensitive && compare_name(*name, str, name_length, str_index, base)) || (!case_sensitive && (name_length = compare_name_no_case(*name, str, name_length, str_index, base))))) {
											if((!p_mode && name_length_old > 1) || (p_mode && (name_length + name_length_old > best_pl || ((ufvt != 'P' || !((Unit*) ufv[2][index][ufv_index2])->getName(ufv_i[2][index][ufv_index2]).abbreviation) && name_length + name_length_old == best_pl)))) {
												p_mode = true;std::cout << "6741" << endl;
												best_p = p;std::cout << "6742" << endl;
												best_p_object = ufv[2][index][ufv_index2];std::cout << "6743" << endl;
												best_pl = name_length + name_length_old;std::cout << "6744" << endl;
												best_pnl = name_length_old;std::cout << "6745" << endl;
												index = -1;std::cout << "6746" << endl;
											}
											if(!p_mode) {
												str.erase(str_index - name_length_old, name_length_old);std::cout << "6749" << endl;
												str_index -= name_length_old;std::cout << "6750" << endl;
												object = ufv[2][index][ufv_index2];std::cout << "6751" << endl;
												goto replace_text_by_unit_place;std::cout << "6752" << endl;
											}
										}
									}
								}
								if(po.limit_implicit_multiplication || (p_mode && index + 1 + name_length_old < best_pl)) {
									break;
								}
							}
							str_index -= name_length_old;std::cout << "6761" << endl;
							unit_chars_left = last_unit_char - str_index + 1;std::cout << "6762" << endl;
							break;
						}
					}
					if(moved_forward) {
						str_index--;std::cout << "6767" << endl;
						break;
					}
				}
			}
			if(!moved_forward && p_mode) {
				object = best_p_object;std::cout << "6773" << endl;
				p = best_p;std::cout << "6774" << endl;
				str.erase(str_index, best_pnl);std::cout << "6775" << endl;
				name_length = best_pl - best_pnl;std::cout << "6776" << endl;
				goto replace_text_by_unit_place;std::cout << "6777" << endl;
			} else if(!moved_forward && found_function) {
				object = found_function;std::cout << "6779" << endl;
				name = found_function_name;std::cout << "6780" << endl;
				name_length = found_function_name_length;std::cout << "6781" << endl;
				goto set_function;std::cout << "6782" << endl;
			}
			if(!moved_forward) {
				bool b = po.unknowns_enabled && is_not_number(str[str_index], base) && !(str_index > 0 && is_in(EXPS, str[str_index]) && str_index + 1 < str.length() && (is_in(NUMBER_ELEMENTS, str[str_index + 1]) || (is_in(PLUS MINUS, str[str_index + 1]) && str_index + 2 < str.length() && is_in(NUMBER_ELEMENTS, str[str_index + 2]))) && is_in(NUMBER_ELEMENTS, str[str_index - 1]));std::cout << "6785" << endl;
				if(po.limit_implicit_multiplication) {
					if(b) {
						stmp = LEFT_PARENTHESIS ID_WRAP_LEFT;std::cout << "6788" << endl;
						stmp += i2s(addId(new MathStructure(str.substr(str_index, unit_chars_left))));std::cout << "6789" << endl;
						stmp += ID_WRAP_RIGHT RIGHT_PARENTHESIS;std::cout << "6790" << endl;
						str.replace(str_index, unit_chars_left, stmp);std::cout << "6791" << endl;
						str_index += stmp.length() - 1;std::cout << "6792" << endl;
					} else {
						str_index += unit_chars_left - 1;std::cout << "6794" << endl;
					}
				} else if(b) {
					size_t i = 1;std::cout << "6797" << endl;
					if(str[str_index + 1] < 0) {
						i++;std::cout << "6799" << endl;
						while(i <= unit_chars_left && (unsigned char) str[str_index + i] >= 0x80 && (unsigned char) str[str_index + i] <= 0xBF) {
							i++;std::cout << "6801" << endl;
						}
					}
					stmp = LEFT_PARENTHESIS ID_WRAP_LEFT;std::cout << "6804" << endl;
					stmp += i2s(addId(new MathStructure(str.substr(str_index, i))));std::cout << "6805" << endl;
					stmp += ID_WRAP_RIGHT RIGHT_PARENTHESIS;std::cout << "6806" << endl;
					str.replace(str_index, i, stmp);std::cout << "6807" << endl;
					str_index += stmp.length() - 1;std::cout << "6808" << endl;
				}
			}
		}
	}

	size_t comma_i = str.find(COMMA, 0);std::cout << "6814" << endl;
	while(comma_i != string::npos) {
		int i3 = 1;std::cout << "6816" << endl;
		size_t left_par_i = comma_i;std::cout << "6817" << endl;
		while(left_par_i > 0) {
			left_par_i = str.find_last_of(LEFT_PARENTHESIS RIGHT_PARENTHESIS, left_par_i - 1);std::cout << "6819" << endl;
			if(left_par_i == string::npos) break;
			if(str[left_par_i] == LEFT_PARENTHESIS_CH) {
				i3--;std::cout << "6822" << endl;
				if(i3 == 0) break;
			} else if(str[left_par_i] == RIGHT_PARENTHESIS_CH) {
				i3++;std::cout << "6825" << endl;
			}
		}
		if(i3 > 0) {
			str.insert(0, i3, LEFT_PARENTHESIS_CH);std::cout << "6829" << endl;
			comma_i += i3;std::cout << "6830" << endl;
			i3 = 0;std::cout << "6831" << endl;
			left_par_i = 0;std::cout << "6832" << endl;
		}
		if(i3 == 0) {
			i3 = 1;std::cout << "6835" << endl;
			size_t right_par_i = comma_i;std::cout << "6836" << endl;
			while(true) {
				right_par_i = str.find_first_of(LEFT_PARENTHESIS RIGHT_PARENTHESIS, right_par_i + 1);std::cout << "6838" << endl;
				if(right_par_i == string::npos) {
					for(; i3 > 0; i3--) {
						str += RIGHT_PARENTHESIS;std::cout << "6841" << endl;
					}
					right_par_i = str.length() - 1;std::cout << "6843" << endl;
				} else if(str[right_par_i] == LEFT_PARENTHESIS_CH) {
					i3++;std::cout << "6845" << endl;
				} else if(str[right_par_i] == RIGHT_PARENTHESIS_CH) {
					i3--;std::cout << "6847" << endl;
				}
				if(i3 == 0) {
					stmp2 = str.substr(left_par_i + 1, right_par_i - left_par_i - 1);std::cout << "6850" << endl;
					stmp = LEFT_PARENTHESIS ID_WRAP_LEFT;std::cout << "6851" << endl;
					stmp += i2s(parseAddVectorId(stmp2, po));std::cout << "6852" << endl;
					stmp += ID_WRAP_RIGHT RIGHT_PARENTHESIS;std::cout << "6853" << endl;
					str.replace(left_par_i, right_par_i + 1 - left_par_i, stmp);std::cout << "6854" << endl;
					comma_i = left_par_i + stmp.length() - 1;std::cout << "6855" << endl;
					break;
				}
			}
		}
		comma_i = str.find(COMMA, comma_i + 1);std::cout << "6860" << endl;
	}

	if(po.rpn) {
		size_t rpn_i = str.find(SPACE, 0);std::cout << "6864" << endl;
		while(rpn_i != string::npos) {
			if(rpn_i == 0 || rpn_i + 1 == str.length() || is_in("~+-*/^\a\b\\\x1c", str[rpn_i - 1]) || (is_in("%&|", str[rpn_i - 1]) && str[rpn_i + 1] != str[rpn_i - 1]) || (is_in("!><=", str[rpn_i - 1]) && is_not_in("=<>", str[rpn_i + 1])) || (is_in(SPACE OPERATORS INTERNAL_OPERATORS, str[rpn_i + 1]) && (str[rpn_i - 1] == SPACE_CH || (str[rpn_i - 1] != str[rpn_i + 1] && is_not_in("!><=", str[rpn_i - 1]))))) {
				str.erase(rpn_i, 1);std::cout << "6867" << endl;
			} else {
				rpn_i++;std::cout << "6869" << endl;
			}
			rpn_i = str.find(SPACE, rpn_i);std::cout << "6871" << endl;
		}
	} else if(po.parsing_mode != PARSING_MODE_ADAPTIVE) {
		remove_blanks(str);std::cout << "6874" << endl;
	} else {
		//remove spaces between next to operators (except '/') and before/after parentheses
		space_i = str.find(SPACE_CH, 0);std::cout << "6877" << endl;
		while(space_i != string::npos) {
			if((str[space_i + 1] != DIVISION_CH && is_in(OPERATORS INTERNAL_OPERATORS RIGHT_PARENTHESIS, str[space_i + 1])) || (str[space_i - 1] != DIVISION_CH && is_in(OPERATORS INTERNAL_OPERATORS LEFT_PARENTHESIS, str[space_i - 1]))) {
				str.erase(space_i, 1);std::cout << "6880" << endl;
				space_i--;std::cout << "6881" << endl;
			}
			space_i = str.find(SPACE_CH, space_i + 1);std::cout << "6883" << endl;
		}
	}

	parseOperators(mstruct, str, po);std::cout << "6887" << endl;

}

#define BASE_2_10 ((po.base >= 2 && po.base <= 10) || (po.base < BASE_CUSTOM && po.base != BASE_UNICODE && po.base != BASE_BIJECTIVE_26) || (po.base == BASE_CUSTOM && priv->custom_input_base_i <= 10))
bool Calculator::parseNumber(MathStructure *mstruct, string str, const ParseOptions &po) {
	mstruct->clear();std::cout << "6893" << endl;
	if(str.empty()) return false;
	if(str.find_first_not_of(OPERATORS "\a%\x1c" SPACE) == string::npos && (po.base != BASE_ROMAN_NUMERALS || str.find("|") == string::npos)) {
		gsub("\a", str.find_first_of("%" OPERATORS) != string::npos ? " xor " : "xor", str);
		gsub("\x1c", "∠", str);
		error(false, _("Misplaced operator(s) \"%s\" ignored"), str.c_str(), NULL);
		return false;
	}
	int minus_count = 0;std::cout << "6901" << endl;
	bool has_sign = false, had_non_sign = false, b_dot = false, b_exp = false, after_sign_e = false;std::cout << "6902" << endl;
	int i_colon = 0;std::cout << "6903" << endl;
	size_t i = 0;std::cout << "6904" << endl;

	while(i < str.length()) {
		if(!had_non_sign && str[i] == MINUS_CH) {
			has_sign = true;std::cout << "6908" << endl;
			minus_count++;std::cout << "6909" << endl;
			str.erase(i, 1);std::cout << "6910" << endl;
		} else if(!had_non_sign && str[i] == PLUS_CH) {
			has_sign = true;std::cout << "6912" << endl;
			str.erase(i, 1);std::cout << "6913" << endl;
		} else if(str[i] == SPACE_CH) {
			str.erase(i, 1);std::cout << "6915" << endl;
		} else if(!b_exp && BASE_2_10 && (str[i] == EXP_CH || str[i] == EXP2_CH)) {
			b_exp = true;std::cout << "6917" << endl;
			had_non_sign = true;std::cout << "6918" << endl;
			after_sign_e = true;std::cout << "6919" << endl;
			i++;std::cout << "6920" << endl;
		} else if(after_sign_e && (str[i] == MINUS_CH || str[i] == PLUS_CH)) {
			after_sign_e = false;std::cout << "6922" << endl;
			i++;std::cout << "6923" << endl;
		} else if(po.preserve_format && str[i] == DOT_CH) {
			b_dot = true;std::cout << "6925" << endl;
			had_non_sign = true;std::cout << "6926" << endl;
			after_sign_e = false;std::cout << "6927" << endl;
			i++;std::cout << "6928" << endl;
		} else if(po.preserve_format && (!b_dot || i_colon > 0) && str[i] == ':') {
			i_colon++;std::cout << "6930" << endl;
			had_non_sign = true;std::cout << "6931" << endl;
			after_sign_e = false;std::cout << "6932" << endl;
			i++;std::cout << "6933" << endl;
		} else if(str[i] == COMMA_CH && DOT_S == ".") {
			str.erase(i, 1);std::cout << "6935" << endl;
			after_sign_e = false;std::cout << "6936" << endl;
			had_non_sign = true;std::cout << "6937" << endl;
		} else if(is_in(OPERATORS, str[i]) && (po.base != BASE_ROMAN_NUMERALS || (str[i] != '(' && str[i] != ')' && str[i] != '|'))) {
			error(false, _("Misplaced '%c' ignored"), str[i], NULL);
			str.erase(i, 1);std::cout << "6940" << endl;
		} else if(str[i] == '\a') {
			error(false, _("Misplaced operator(s) \"%s\" ignored"), "xor", NULL);
			str.erase(i, 1);std::cout << "6943" << endl;
		} else if(str[i] == '\x1c') {
			error(false, _("Misplaced operator(s) \"%s\" ignored"), "∠", NULL);
			str.erase(i, 1);std::cout << "6946" << endl;
		} else if(str[i] == '\b') {
			b_exp = false;std::cout << "6948" << endl;
			had_non_sign = false;std::cout << "6949" << endl;
			after_sign_e = false;std::cout << "6950" << endl;
			i++;std::cout << "6951" << endl;
		} else {
			had_non_sign = true;std::cout << "6953" << endl;
			after_sign_e = false;std::cout << "6954" << endl;
			i++;std::cout << "6955" << endl;
		}
	}
	if(str.empty()) {
		if(minus_count % 2 == 1 && !po.preserve_format) {
			mstruct->set(-1, 1, 0);std::cout << "6960" << endl;
		} else if(has_sign) {
			mstruct->set(1, 1, 0);std::cout << "6962" << endl;
			if(po.preserve_format) {
				while(minus_count > 0) {
					mstruct->transform(STRUCT_NEGATE);
					minus_count--;std::cout << "6966" << endl;
				}
			}
		}
		return false;
	}
	if(str[0] == ID_WRAP_LEFT_CH && str.length() > 2 && str[str.length() - 1] == ID_WRAP_RIGHT_CH) {
		int id = s2i(str.substr(1, str.length() - 2));std::cout << "6973" << endl;
		MathStructure *m_temp = getId((size_t) id);std::cout << "6974" << endl;
		if(!m_temp) {
			mstruct->setUndefined();std::cout << "6976" << endl;
			error(true, _("Internal id %s does not exist."), i2s(id).c_str(), NULL);
			return true;
		}
		mstruct->set_nocopy(*m_temp);std::cout << "6980" << endl;
		m_temp->unref();std::cout << "6981" << endl;
		if(po.preserve_format) {
			while(minus_count > 0) {
				mstruct->transform(STRUCT_NEGATE);
				minus_count--;std::cout << "6985" << endl;
			}
		} else if(minus_count % 2 == 1) {
			mstruct->negate();std::cout << "6988" << endl;
		}
		return true;
	}
	size_t itmp;std::cout << "6992" << endl;
	if((BASE_2_10 || po.base == BASE_DUODECIMAL) &&
	    (itmp = str.find_first_not_of(po.base == BASE_DUODECIMAL ?
	     NUMBER_ELEMENTS INTERNAL_NUMBER_CHARS MINUS DUODECIMAL_CHARS :
	     NUMBER_ELEMENTS INTERNAL_NUMBER_CHARS EXPS MINUS, 0)) != string::npos) {
		if(itmp == 0) {
		    std::cout << "6997-x" << endl;
			error(true, _("\"%s\" is not a valid variable/function/unit."), str.c_str(), NULL);
			if(minus_count % 2 == 1 && !po.preserve_format) {
				mstruct->set(-1, 1, 0);std::cout << "6997" << endl;
			} else if(has_sign) {
				mstruct->set(1, 1, 0);std::cout << "6999" << endl;
				if(po.preserve_format) {
					while(minus_count > 0) {
						mstruct->transform(STRUCT_NEGATE);
						minus_count--;std::cout << "7003" << endl;
					}
				}
			}
			return false;
		} else {
		    std::cout << "6997-y" << endl;
			string stmp = str.substr(itmp, str.length() - itmp);std::cout << "7009" << endl;
			error(true, _("Trailing characters \"%s\" (not a valid variable/function/unit) in number \"%s\" was ignored."), stmp.c_str(), str.c_str(), NULL);
			str.erase(itmp, str.length() - itmp);std::cout << "7011" << endl;
		}
	}
	std::cout << "6992-z" << endl;
	gsub("\b", "±", str);
	std::cout << "6992-w" << endl;
	Number nr(str, po);std::cout << "7015" << endl;
	if(!po.preserve_format && minus_count % 2 == 1) {
		nr.negate();std::cout << "7017" << endl;
	}
	if(i_colon && nr.isRational() && !nr.isInteger()) {
		Number nr_num(nr.numerator()), nr_den(1, 1, 0);std::cout << "7020" << endl;
		while(i_colon) {
			nr_den *= 60;std::cout << "7022" << endl;
			i_colon--;std::cout << "7023" << endl;
		}
		nr_num *= nr_den;std::cout << "7025" << endl;
		nr_num /= nr.denominator();std::cout << "7026" << endl;
		mstruct->set(nr_num);std::cout << "7027" << endl;
		mstruct->transform(STRUCT_DIVISION, nr_den);
	} else {
		mstruct->set(nr);std::cout << "7030" << endl;
	}
	if(po.preserve_format) {
		while(minus_count > 0) {
			mstruct->transform(STRUCT_NEGATE);
			minus_count--;std::cout << "7035" << endl;
		}
	}
	return true;

}

bool Calculator::parseAdd(string &str, MathStructure *mstruct, const ParseOptions &po) {
	if(str.length() > 0) {
		size_t i;std::cout << "7044" << endl;
		if(BASE_2_10) {
			i = str.find_first_of(SPACE MULTIPLICATION_2 OPERATORS INTERNAL_OPERATORS PARENTHESISS EXPS ID_WRAP_LEFT, 1);std::cout << "7046" << endl;
		} else {
			i = str.find_first_of(SPACE MULTIPLICATION_2 OPERATORS INTERNAL_OPERATORS PARENTHESISS ID_WRAP_LEFT, 1);std::cout << "7048" << endl;
		}
		if(i == string::npos && str[0] != LOGICAL_NOT_CH && str[0] != BITWISE_NOT_CH && !(str[0] == ID_WRAP_LEFT_CH && str.find(ID_WRAP_RIGHT) < str.length() - 1)) {
			return parseNumber(mstruct, str, po);
		} else {
			return parseOperators(mstruct, str, po);
		}
	}
	return false;
}
bool Calculator::parseAdd(string &str, MathStructure *mstruct, const ParseOptions &po, MathOperation s, bool append) {
	if(str.length() > 0) {
		size_t i;std::cout << "7060" << endl;
		if(BASE_2_10) {
			i = str.find_first_of(SPACE MULTIPLICATION_2 OPERATORS INTERNAL_OPERATORS PARENTHESISS EXPS ID_WRAP_LEFT, 1);std::cout << "7062" << endl;
		} else {
			i = str.find_first_of(SPACE MULTIPLICATION_2 OPERATORS INTERNAL_OPERATORS PARENTHESISS ID_WRAP_LEFT, 1);std::cout << "7064" << endl;
		}
		if(i == string::npos && str[0] != LOGICAL_NOT_CH && str[0] != BITWISE_NOT_CH && !(str[0] == ID_WRAP_LEFT_CH && str.find(ID_WRAP_RIGHT) < str.length() - 1)) {
			if(s == OPERATION_EXP10 && po.read_precision == ALWAYS_READ_PRECISION) {
				ParseOptions po2 = po;std::cout << "7068" << endl;
				po2.read_precision = READ_PRECISION_WHEN_DECIMALS;std::cout << "7069" << endl;
				MathStructure *mstruct2 = new MathStructure();std::cout << "7070" << endl;
				if(!parseNumber(mstruct2, str, po2)) {
					mstruct2->unref();std::cout << "7072" << endl;
					return false;
				}
				mstruct->add_nocopy(mstruct2, s, append);std::cout << "7075" << endl;
			} else {
				MathStructure *mstruct2 = new MathStructure();std::cout << "7077" << endl;
				if(!parseNumber(mstruct2, str, po)) {
					mstruct2->unref();std::cout << "7079" << endl;
					return false;
				}
				if(s == OPERATION_EXP10 && !po.preserve_format && mstruct->isNumber() && mstruct2->isNumber()) {
					mstruct->number().exp10(mstruct2->number());std::cout << "7083" << endl;
					mstruct->numberUpdated();std::cout << "7084" << endl;
					mstruct->mergePrecision(*mstruct2);std::cout << "7085" << endl;
				} else if(s == OPERATION_DIVIDE && po.preserve_format) {
					mstruct->transform_nocopy(STRUCT_DIVISION, mstruct2);
				} else if(s == OPERATION_SUBTRACT && po.preserve_format) {
					mstruct2->transform(STRUCT_NEGATE);
					mstruct->add_nocopy(mstruct2, OPERATION_ADD, append);std::cout << "7090" << endl;
				} else {
					mstruct->add_nocopy(mstruct2, s, append);std::cout << "7092" << endl;
				}
			}
		} else {
			MathStructure *mstruct2 = new MathStructure();std::cout << "7096" << endl;
			if(!parseOperators(mstruct2, str, po)) {
				mstruct2->unref();std::cout << "7098" << endl;
				return false;
			}
			if(s == OPERATION_DIVIDE && po.preserve_format) {
				mstruct->transform_nocopy(STRUCT_DIVISION, mstruct2);
			} else if(s == OPERATION_SUBTRACT && po.preserve_format) {
				mstruct2->transform(STRUCT_NEGATE);
				mstruct->add_nocopy(mstruct2, OPERATION_ADD, append);std::cout << "7105" << endl;
			} else {
				mstruct->add_nocopy(mstruct2, s, append);std::cout << "7107" << endl;
			}
		}
	}
	return true;
}

MathStructure *get_out_of_negate(MathStructure &mstruct, int *i_neg) {
	if(mstruct.isNegate() || (mstruct.isMultiplication() && mstruct.size() == 2 && mstruct[0].isMinusOne())) {
		(*i_neg)++;std::cout << "7116" << endl;
		return get_out_of_negate(mstruct.last(), i_neg);
	}
	return &mstruct;
}

bool Calculator::parseOperators(MathStructure *mstruct, string str, const ParseOptions &po) {
	string save_str = str;std::cout << "7123" << endl;
	mstruct->clear();std::cout << "7124" << endl;
	size_t i = 0, i2 = 0, i3 = 0;std::cout << "7125" << endl;
	string str2, str3;std::cout << "7126" << endl;
	bool extended_roman = (po.base == BASE_ROMAN_NUMERALS && (i = str.find("|")) != string::npos && i + 1 < str.length() && str[i + 1] == RIGHT_PARENTHESIS_CH);
	while(!extended_roman) {
		//find first right parenthesis and then the last left parenthesis before
		i2 = str.find(RIGHT_PARENTHESIS_CH);std::cout << "7130" << endl;
		if(i2 == string::npos) {
			i = str.rfind(LEFT_PARENTHESIS_CH);std::cout << "7132" << endl;
			if(i == string::npos) {
				//if no parenthesis break
				break;
			} else {
				//right parenthesis missing -- append
				str += RIGHT_PARENTHESIS_CH;std::cout << "7138" << endl;
				i2 = str.length() - 1;std::cout << "7139" << endl;
			}
		} else {
			if(i2 > 0) {
				i = str.rfind(LEFT_PARENTHESIS_CH, i2 - 1);std::cout << "7143" << endl;
			} else {
				i = string::npos;std::cout << "7145" << endl;
			}
			if(i == string::npos) {
				//left parenthesis missing -- prepend
				str.insert(str.begin(), 1, LEFT_PARENTHESIS_CH);std::cout << "7149" << endl;
				i = 0;std::cout << "7150" << endl;
				i2++;std::cout << "7151" << endl;
			}
		}
		while(true) {
			//remove unnecessary double parenthesis and the found parenthesis
			if(i > 0 && i2 + 1 < str.length() && str[i - 1] == LEFT_PARENTHESIS_CH && str[i2 + 1] == RIGHT_PARENTHESIS_CH) {
				str.erase(str.begin() + (i - 1));std::cout << "7157" << endl;
				i--;std::cout << "7158" << endl; i2--;std::cout << "7158" << endl;
				str.erase(str.begin() + (i2 + 1));std::cout << "7159" << endl;
			} else {
				break;
			}
		}
		if(i > 0 && is_not_in(MULTIPLICATION_2 OPERATORS INTERNAL_OPERATORS PARENTHESISS SPACE, str[i - 1]) && (!BASE_2_10 || (str[i - 1] != EXP_CH && str[i - 1] != EXP2_CH))) {
			if(po.rpn) {
				str.insert(i2 + 1, MULTIPLICATION);std::cout << "7166" << endl;
				str.insert(i, SPACE);std::cout << "7167" << endl;
				i++;std::cout << "7168" << endl;
				i2++;std::cout << "7169" << endl;
			}
		}
		if(i2 + 1 < str.length() && is_not_in(MULTIPLICATION_2 OPERATORS INTERNAL_OPERATORS PARENTHESISS SPACE, str[i2 + 1]) && (!BASE_2_10 || (str[i2 + 1] != EXP_CH && str[i2 + 1] != EXP2_CH))) {
			if(po.rpn) {
				i3 = str.find(SPACE, i2 + 1);std::cout << "7174" << endl;
				if(i3 == string::npos) {
					str += MULTIPLICATION;std::cout << "7176" << endl;
				} else {
					str.replace(i3, 1, MULTIPLICATION);std::cout << "7178" << endl;
				}
				str.insert(i2 + 1, SPACE);std::cout << "7180" << endl;
			}
		}
		if(po.rpn && i > 0 && i2 + 1 == str.length() && is_not_in(PARENTHESISS SPACE, str[i - 1])) {
			str += MULTIPLICATION_CH;std::cout << "7184" << endl;
		}
		str2 = str.substr(i + 1, i2 - (i + 1));std::cout << "7186" << endl;
		MathStructure *mstruct2 = new MathStructure();std::cout << "7187" << endl;
		if(str2.empty()) {
			error(false, "Empty expression in parentheses interpreted as zero.", NULL);
		} else {
			parseOperators(mstruct2, str2, po);std::cout << "7191" << endl;
		}
		mstruct2->setInParentheses(true);std::cout << "7193" << endl;
		str2 = ID_WRAP_LEFT;std::cout << "7194" << endl;
		str2 += i2s(addId(mstruct2));std::cout << "7195" << endl;
		str2 += ID_WRAP_RIGHT;std::cout << "7196" << endl;
		str.replace(i, i2 - i + 1, str2);std::cout << "7197" << endl;
		mstruct->clear();std::cout << "7198" << endl;
	}
	bool b_abs_or = false, b_bit_or = false;std::cout << "7200" << endl;
	i = 0;std::cout << "7201" << endl;
	if(!po.rpn) {
		while(po.base != BASE_ROMAN_NUMERALS && (i = str.find('|', i)) != string::npos) {
			if(i == 0 || i == str.length() - 1 || is_in(OPERATORS INTERNAL_OPERATORS SPACE, str[i - 1])) {b_abs_or = true; break;}
			if(str[i + 1] == '|') {
				if(i == str.length() - 2) {b_abs_or = true; break;}
				if(b_bit_or) {
					b_abs_or = true;std::cout << "7208" << endl;
					break;
				}
				i += 2;std::cout << "7211" << endl;
			} else {
				b_bit_or = true;std::cout << "7213" << endl;
				i++;std::cout << "7214" << endl;
			}
		}
	}
	if(b_abs_or) {
		while((i = str.find('|', 0)) != string::npos && i + 1 != str.length()) {
			if(str[i + 1] == '|') {
				size_t depth = 1;std::cout << "7221" << endl;
				i2 = i;std::cout << "7222" << endl;
				while((i2 = str.find("||", i2 + 2)) != string::npos) {
					if(is_in(OPERATORS INTERNAL_OPERATORS, str[i2 - 1])) depth++;
					else depth--;std::cout << "7225" << endl;
					if(depth == 0) break;
				}
				if(i2 == string::npos) str2 = str.substr(i + 2);
				else str2 = str.substr(i + 2, i2 - (i + 2));std::cout << "7229" << endl;
				str3 = ID_WRAP_LEFT;std::cout << "7230" << endl;
				str3 += i2s(parseAddId(f_magnitude, str2, po));std::cout << "7231" << endl;
				str3 += ID_WRAP_RIGHT;std::cout << "7232" << endl;
				if(i2 == string::npos) str.replace(i, str.length() - i, str3);
				else str.replace(i, i2 - i + 2, str3);std::cout << "7234" << endl;
			} else {
				size_t depth = 1;std::cout << "7236" << endl;
				i2 = i;std::cout << "7237" << endl;
				while((i2 = str.find('|', i2 + 1)) != string::npos) {
					if(is_in(OPERATORS INTERNAL_OPERATORS, str[i2 - 1])) depth++;
					else depth--;std::cout << "7240" << endl;
					if(depth == 0) break;
				}
				if(i2 == string::npos) str2 = str.substr(i + 1);
				else str2 = str.substr(i + 1, i2 - (i + 1));std::cout << "7244" << endl;
				str3 = ID_WRAP_LEFT;std::cout << "7245" << endl;
				str3 += i2s(parseAddId(f_abs, str2, po));std::cout << "7246" << endl;
				str3 += ID_WRAP_RIGHT;std::cout << "7247" << endl;
				if(i2 == string::npos) str.replace(i, str.length() - i, str3);
				else str.replace(i, i2 - i + 1, str3);std::cout << "7249" << endl;
			}
		}
	}
	if(po.rpn) {
		i = 0;std::cout << "7254" << endl;
		i3 = 0;std::cout << "7255" << endl;
		ParseOptions po2 = po;std::cout << "7256" << endl;
		po2.rpn = false;std::cout << "7257" << endl;
		vector<MathStructure*> mstack;std::cout << "7258" << endl;
		bool b = false;std::cout << "7259" << endl;
		char last_operator = 0;std::cout << "7260" << endl;
		char last_operator2 = 0;std::cout << "7261" << endl;
		while(true) {
			i = str.find_first_of(OPERATORS "\a%\x1c" SPACE "\\", i3 + 1);
			if(i == string::npos) {
				if(!b) {
					parseAdd(str, mstruct, po2);std::cout << "7266" << endl;
					return true;
				}
				if(i3 != 0) {
					str2 = str.substr(i3 + 1, str.length() - i3 - 1);std::cout << "7270" << endl;
				} else {
					str2 = str.substr(i3, str.length() - i3);std::cout << "7272" << endl;
				}
				remove_blank_ends(str2);std::cout << "7274" << endl;
				if(!str2.empty()) {
					error(false, _("RPN syntax error. Values left at the end of the RPN expression."), NULL);
				} else if(mstack.size() > 1) {
					if(last_operator == 0 && mstack.size() > 1) {
						error(false, _("Unused stack values."), NULL);
					} else {
						while(mstack.size() > 1) {
							switch(last_operator) {
								case PLUS_CH: {
									mstack[mstack.size() - 2]->add_nocopy(mstack.back());std::cout << "7284" << endl;
									mstack.pop_back();std::cout << "7285" << endl;
									break;
								}
								case MINUS_CH: {
									if(po.preserve_format) {
										mstack.back()->transform(STRUCT_NEGATE);
										mstack[mstack.size() - 2]->add_nocopy(mstack.back());std::cout << "7291" << endl;
									} else {
										mstack[mstack.size() - 2]->subtract_nocopy(mstack.back());std::cout << "7293" << endl;
									}
									mstack.pop_back();std::cout << "7295" << endl;
									break;
								}
								case MULTIPLICATION_CH: {
									mstack[mstack.size() - 2]->multiply_nocopy(mstack.back());std::cout << "7299" << endl;
									mstack.pop_back();std::cout << "7300" << endl;
									break;
								}
								case DIVISION_CH: {
									if(po.preserve_format) {
										mstack[mstack.size() - 2]->transform_nocopy(STRUCT_DIVISION, mstack.back());
									} else {
										mstack[mstack.size() - 2]->divide_nocopy(mstack.back());std::cout << "7307" << endl;
									}
									mstack.pop_back();std::cout << "7309" << endl;
									break;
								}
								case POWER_CH: {
									mstack[mstack.size() - 2]->raise_nocopy(mstack.back());std::cout << "7313" << endl;
									mstack.pop_back();std::cout << "7314" << endl;
									break;
								}
								case AND_CH: {
									mstack[mstack.size() - 2]->transform_nocopy(STRUCT_BITWISE_AND, mstack.back());
									mstack.pop_back();std::cout << "7319" << endl;
									break;
								}
								case OR_CH: {
									mstack[mstack.size() - 2]->transform_nocopy(STRUCT_BITWISE_OR, mstack.back());
									mstack.pop_back();std::cout << "7324" << endl;
									break;
								}
								case GREATER_CH: {
									if(last_operator2 == GREATER_CH) {
										if(po.preserve_format) mstack.back()->transform(STRUCT_NEGATE);
										else mstack.back()->negate();std::cout << "7330" << endl;
										mstack[mstack.size() - 2]->transform(CALCULATOR->f_shift);
										mstack[mstack.size() - 2]->addChild_nocopy(mstack.back());std::cout << "7332" << endl;
										mstack[mstack.size() - 2]->addChild(m_one);std::cout << "7333" << endl;
									} else if(last_operator2 == EQUALS_CH) {
										mstack[mstack.size() - 2]->add_nocopy(mstack.back(), OPERATION_EQUALS_GREATER);std::cout << "7335" << endl;
									} else {
										mstack[mstack.size() - 2]->add_nocopy(mstack.back(), OPERATION_GREATER);std::cout << "7337" << endl;
									}
									mstack.pop_back();std::cout << "7339" << endl;
									break;
								}
								case LESS_CH: {
									if(last_operator2 == LESS_CH) {
										mstack[mstack.size() - 2]->transform(CALCULATOR->f_shift);
										mstack[mstack.size() - 2]->addChild_nocopy(mstack.back());std::cout << "7345" << endl;
										mstack[mstack.size() - 2]->addChild(m_one);std::cout << "7346" << endl;
									} else if(last_operator2 == EQUALS_CH) {
										mstack[mstack.size() - 2]->add_nocopy(mstack.back(), OPERATION_EQUALS_LESS);std::cout << "7348" << endl;
									} else {
										mstack[mstack.size() - 2]->add_nocopy(mstack.back(), OPERATION_LESS);std::cout << "7350" << endl;
									}
									mstack.pop_back();std::cout << "7352" << endl;
									break;
								}
								case NOT_CH: {
									mstack.back()->transform(STRUCT_LOGICAL_NOT);
									break;
								}
								case EQUALS_CH: {
									if(last_operator2 == NOT_CH) {
										mstack[mstack.size() - 2]->add_nocopy(mstack.back(), OPERATION_NOT_EQUALS);std::cout << "7361" << endl;
									} else {
										mstack[mstack.size() - 2]->add_nocopy(mstack.back(), OPERATION_EQUALS);std::cout << "7363" << endl;
									}
									mstack.pop_back();std::cout << "7365" << endl;
									break;
								}
								case BITWISE_NOT_CH: {
									mstack.back()->transform(STRUCT_BITWISE_NOT);
									error(false, _("Unused stack values."), NULL);
									break;
								}
								case '\x1c': {
									if(po.angle_unit != ANGLE_UNIT_NONE && po.angle_unit != ANGLE_UNIT_RADIANS && mstack.back()->contains(getRadUnit(), false, true, true) <= 0 && mstack.back()->contains(getGraUnit(), false, true, true) <= 0 && mstack.back()->contains(getDegUnit(), false, true, true) <= 0) {
										switch(po.angle_unit) {
											case ANGLE_UNIT_DEGREES: {mstack.back()->multiply(getDegUnit()); break;}
											case ANGLE_UNIT_GRADIANS: {mstack.back()->multiply(getGraUnit()); break;}
											default: {}
										}
									}
									mstack.back()->transform(priv->f_cis);
									mstack[mstack.size() - 2]->transform_nocopy(STRUCT_MULTIPLICATION, mstack.back());
									mstack.pop_back();std::cout << "7383" << endl;
									break;
								}
								case '\a': {
									mstack[mstack.size() - 2]->transform_nocopy(STRUCT_BITWISE_XOR, mstack.back());
									mstack.pop_back();std::cout << "7388" << endl;
									break;
								}
								case '%': {
									if(last_operator2 == '%') {
										mstack[mstack.size() - 2]->transform(f_mod);
									} else {
										mstack[mstack.size() - 2]->transform(f_rem);
									}
									mstack[mstack.size() - 2]->addChild_nocopy(mstack.back());std::cout << "7397" << endl;
									mstack.pop_back();std::cout << "7398" << endl;
									break;
								}
								case '\\': {
									if(po.preserve_format) {
										mstack[mstack.size() - 2]->transform_nocopy(STRUCT_DIVISION, mstack.back());
									} else {
										mstack[mstack.size() - 2]->divide_nocopy(mstack.back());std::cout << "7405" << endl;
									}
									mstack[mstack.size() - 2]->transform(f_trunc);
									mstack.pop_back();std::cout << "7408" << endl;
									break;
								}
								default: {
									error(true, _("RPN syntax error. Operator '%c' not supported."), last_operator, NULL);
									mstack.pop_back();std::cout << "7413" << endl;
									break;
								}
							}
							if(last_operator == NOT_CH || last_operator == BITWISE_NOT_CH)  break;
						}
					}
				} else if(mstack.size() == 1) {
					if(last_operator == NOT_CH) {
						mstack.back()->transform(STRUCT_LOGICAL_NOT);
					} else if(last_operator == BITWISE_NOT_CH) {
						mstack.back()->transform(STRUCT_BITWISE_NOT);
					}
				}
				mstruct->set_nocopy(*mstack.back());std::cout << "7427" << endl;
				while(!mstack.empty()) {
					mstack.back()->unref();std::cout << "7429" << endl;
					mstack.pop_back();std::cout << "7430" << endl;
				}
				return true;
			}
			b = true;std::cout << "7434" << endl;
			if(i3 != 0) {
				str2 = str.substr(i3 + 1, i - i3 - 1);std::cout << "7436" << endl;
			} else {
				str2 = str.substr(i3, i - i3);std::cout << "7438" << endl;
			}
			remove_blank_ends(str2);std::cout << "7440" << endl;
			if(!str2.empty()) {
				mstack.push_back(new MathStructure());std::cout << "7442" << endl;
				if((str[i] == GREATER_CH || str[i] == LESS_CH) && po2.base < 10 && po2.base >= 2 && i + 1 < str.length() && str[i + 1] == str[i] && str2.find_first_not_of(NUMBERS SPACE PLUS MINUS) == string::npos) {
					for(i = 0; i < str2.size(); i++) {
						if(str2[i] >= '0' && str2[i] <= '9' && po.base <= str2[i] - '0') {
							po2.base = BASE_DECIMAL;std::cout << "7446" << endl;
							break;
						}
					}
					parseAdd(str2, mstack.back(), po2);std::cout << "7450" << endl;
					po2.base = po.base;std::cout << "7451" << endl;
				} else {
					parseAdd(str2, mstack.back(), po2);std::cout << "7453" << endl;
				}
			}
			if(str[i] != SPACE_CH) {
				if(mstack.size() < 1) {
					error(true, _("RPN syntax error. Stack is empty."), NULL);
				} else if(mstack.size() < 2) {
					if(str[i] == NOT_CH) {
						mstack.back()->transform(STRUCT_LOGICAL_NOT);
					} else if(str[i] == MINUS_CH) {
						if(po.preserve_format) mstack.back()->transform(STRUCT_NEGATE);
						else mstack.back()->negate();std::cout << "7464" << endl;
					} else if(str[i] == BITWISE_NOT_CH) {
						mstack.back()->transform(STRUCT_BITWISE_NOT);
					} else if(str[i] == '\x1c') {
						if(po.angle_unit != ANGLE_UNIT_NONE && po.angle_unit != ANGLE_UNIT_RADIANS && mstack.back()->contains(getRadUnit(), false, true, true) <= 0 && mstack.back()->contains(getGraUnit(), false, true, true) <= 0 && mstack.back()->contains(getDegUnit(), false, true, true) <= 0) {
							switch(po.angle_unit) {
								case ANGLE_UNIT_DEGREES: {mstack.back()->multiply(getDegUnit()); break;}
								case ANGLE_UNIT_GRADIANS: {mstack.back()->multiply(getGraUnit()); break;}
								default: {}
							}
						}
						mstack.back()->transform(priv->f_cis);
						mstack.back()->multiply(m_one);std::cout << "7476" << endl;
						if(po.preserve_format) mstack.back()->swapChildren(1, 2);
					} else {
						error(false, _("RPN syntax error. Operator ignored as there where only one stack value."), NULL);
					}
				} else {
					switch(str[i]) {
						case PLUS_CH: {
							mstack[mstack.size() - 2]->add_nocopy(mstack.back());std::cout << "7484" << endl;
							mstack.pop_back();std::cout << "7485" << endl;
							break;
						}
						case MINUS_CH: {
							if(po.preserve_format) {
								mstack.back()->transform(STRUCT_NEGATE);
								mstack[mstack.size() - 2]->add_nocopy(mstack.back());std::cout << "7491" << endl;
							} else {
								mstack[mstack.size() - 2]->subtract_nocopy(mstack.back());std::cout << "7493" << endl;
							}
							mstack.pop_back();std::cout << "7495" << endl;
							break;
						}
						case MULTIPLICATION_CH: {
							mstack[mstack.size() - 2]->multiply_nocopy(mstack.back());std::cout << "7499" << endl;
							mstack.pop_back();std::cout << "7500" << endl;
							break;
						}
						case DIVISION_CH: {
							if(po.preserve_format) {
								mstack[mstack.size() - 2]->transform_nocopy(STRUCT_DIVISION, mstack.back());
							} else {
								mstack[mstack.size() - 2]->divide_nocopy(mstack.back());std::cout << "7507" << endl;
							}
							mstack.pop_back();std::cout << "7509" << endl;
							break;
						}
						case POWER_CH: {
							mstack[mstack.size() - 2]->raise_nocopy(mstack.back());std::cout << "7513" << endl;
							mstack.pop_back();std::cout << "7514" << endl;
							break;
						}
						case AND_CH: {
							if(i + 1 < str.length() && str[i + 1] == AND_CH) {
								mstack[mstack.size() - 2]->transform_nocopy(STRUCT_LOGICAL_AND, mstack.back());
							} else {
								mstack[mstack.size() - 2]->transform_nocopy(STRUCT_BITWISE_AND, mstack.back());
							}
							mstack.pop_back();std::cout << "7523" << endl;
							break;
						}
						case OR_CH: {
							if(i + 1 < str.length() && str[i + 1] == OR_CH) {
								mstack[mstack.size() - 2]->transform_nocopy(STRUCT_LOGICAL_OR, mstack.back());
							} else {
								mstack[mstack.size() - 2]->transform_nocopy(STRUCT_BITWISE_OR, mstack.back());
							}
							mstack.pop_back();std::cout << "7532" << endl;
							break;
						}
						case GREATER_CH: {
							if(i + 1 < str.length() && str[i + 1] == GREATER_CH) {
								if(po.preserve_format) mstack.back()->transform(STRUCT_NEGATE);
								else mstack.back()->negate();std::cout << "7538" << endl;
								mstack[mstack.size() - 2]->transform(CALCULATOR->f_shift);
								mstack[mstack.size() - 2]->addChild_nocopy(mstack.back());std::cout << "7540" << endl;
								mstack[mstack.size() - 2]->addChild(m_one);std::cout << "7541" << endl;
							} else if(i + 1 < str.length() && str[i + 1] == EQUALS_CH) {
								mstack[mstack.size() - 2]->add_nocopy(mstack.back(), OPERATION_EQUALS_GREATER);std::cout << "7543" << endl;
							} else {
								mstack[mstack.size() - 2]->add_nocopy(mstack.back(), OPERATION_GREATER);std::cout << "7545" << endl;
							}
							mstack.pop_back();std::cout << "7547" << endl;
							break;
						}
						case LESS_CH: {
							if(i + 1 < str.length() && str[i + 1] == LESS_CH) {
								mstack[mstack.size() - 2]->transform(CALCULATOR->f_shift);
								mstack[mstack.size() - 2]->addChild_nocopy(mstack.back());std::cout << "7553" << endl;
								mstack[mstack.size() - 2]->addChild(m_one);std::cout << "7554" << endl;
							} else if(i + 1 < str.length() && str[i + 1] == EQUALS_CH) {
								mstack[mstack.size() - 2]->add_nocopy(mstack.back(), OPERATION_EQUALS_LESS);std::cout << "7556" << endl;
							} else {
								mstack[mstack.size() - 2]->add_nocopy(mstack.back(), OPERATION_LESS);std::cout << "7558" << endl;
							}
							mstack.pop_back();std::cout << "7560" << endl;
							break;
						}
						case NOT_CH: {
							mstack.back()->transform(STRUCT_LOGICAL_NOT);
							break;
						}
						case EQUALS_CH: {
							if(i + 1 < str.length() && str[i + 1] == NOT_CH) {
								mstack[mstack.size() - 2]->add_nocopy(mstack.back(), OPERATION_NOT_EQUALS);std::cout << "7569" << endl;
								mstack.pop_back();std::cout << "7570" << endl;
							} else {
								mstack[mstack.size() - 2]->add_nocopy(mstack.back(), OPERATION_EQUALS);std::cout << "7572" << endl;
							}
							mstack.pop_back();std::cout << "7574" << endl;
							break;
						}
						case BITWISE_NOT_CH: {
							mstack.back()->transform(STRUCT_BITWISE_NOT);
							break;
						}
						case '\x1c': {
							if(po.angle_unit != ANGLE_UNIT_NONE && po.angle_unit != ANGLE_UNIT_RADIANS && mstack.back()->contains(getRadUnit(), false, true, true) <= 0 && mstack.back()->contains(getGraUnit(), false, true, true) <= 0 && mstack.back()->contains(getDegUnit(), false, true, true) <= 0) {
								switch(po.angle_unit) {
									case ANGLE_UNIT_DEGREES: {mstack.back()->multiply(getDegUnit()); break;}
									case ANGLE_UNIT_GRADIANS: {mstack.back()->multiply(getGraUnit()); break;}
									default: {}
								}
							}
							mstack.back()->transform(priv->f_cis);
							mstack[mstack.size() - 2]->transform_nocopy(STRUCT_MULTIPLICATION, mstack.back());
							mstack.pop_back();std::cout << "7591" << endl;
							break;
						}
						case '\a': {
							mstack[mstack.size() - 2]->transform_nocopy(STRUCT_BITWISE_XOR, mstack.back());
							mstack.pop_back();std::cout << "7596" << endl;
							break;
						}
						case '%': {
							if(i + 1 < str.length() && str[i + 1] == '%') {
								mstack[mstack.size() - 2]->transform(f_mod);
							} else {
								mstack[mstack.size() - 2]->transform(f_rem);
							}
							mstack[mstack.size() - 2]->addChild_nocopy(mstack.back());std::cout << "7605" << endl;
							mstack.pop_back();std::cout << "7606" << endl;
							break;
						}
						case '\\': {
							if(po.preserve_format) {
								mstack[mstack.size() - 2]->transform_nocopy(STRUCT_DIVISION, mstack.back());
							} else {
								mstack[mstack.size() - 2]->divide_nocopy(mstack.back());std::cout << "7613" << endl;
							}
							mstack[mstack.size() - 2]->transform(f_trunc);
							mstack.pop_back();std::cout << "7616" << endl;
							break;
						}
						default: {
							error(true, _("RPN syntax error. Operator '%c' not supported."), str[i], NULL);
							mstack.pop_back();std::cout << "7621" << endl;
							break;
						}
					}
					last_operator = str[i];std::cout << "7625" << endl;
					if(i + 1 < str.length()) last_operator2 = str[i + 1];
					else last_operator2 = 0;std::cout << "7627" << endl;
					if((last_operator2 == EQUALS_CH && (last_operator == GREATER_CH || last_operator == LESS_CH || last_operator == EQUALS_CH)) || (last_operator2 == NOT_CH && last_operator == EQUALS_CH) || (last_operator == last_operator2 && (last_operator == GREATER_CH || last_operator == LESS_CH || last_operator == '%' || last_operator == AND_CH || last_operator == OR_CH))) {
						i++;std::cout << "7629" << endl;
					}
				}
			}
			i3 = i;std::cout << "7633" << endl;
		}
	}
	if(po.rpn) remove_blanks(str);
	i = 0;std::cout << "7637" << endl;
	i3 = 0;std::cout << "7638" << endl;
	if((i = str.find(LOGICAL_AND, 1)) != string::npos && i + 2 != str.length()) {
		bool b = false, append = false;std::cout << "7640" << endl;
		while(i != string::npos && i + 2 != str.length()) {
			str2 = str.substr(0, i);std::cout << "7642" << endl;
			str = str.substr(i + 2, str.length() - (i + 2));std::cout << "7643" << endl;
			if(b) {
				parseAdd(str2, mstruct, po, OPERATION_LOGICAL_AND, append);std::cout << "7645" << endl;
				append = true;std::cout << "7646" << endl;
			} else {
				parseAdd(str2, mstruct, po);std::cout << "7648" << endl;
				b = true;std::cout << "7649" << endl;
			}
			i = str.find(LOGICAL_AND, 1);std::cout << "7651" << endl;
		}
		if(b) {
			parseAdd(str, mstruct, po, OPERATION_LOGICAL_AND, append);std::cout << "7654" << endl;
		} else {
			parseAdd(str, mstruct, po);std::cout << "7656" << endl;
		}
		return true;
	}
	if(po.base != BASE_ROMAN_NUMERALS && (i = str.find(LOGICAL_OR, 1)) != string::npos && i + 2 != str.length()) {
		bool b = false, append = false;std::cout << "7661" << endl;
		while(i != string::npos && i + 2 != str.length()) {
			str2 = str.substr(0, i);std::cout << "7663" << endl;
			str = str.substr(i + 2, str.length() - (i + 2));std::cout << "7664" << endl;
			if(b) {
				parseAdd(str2, mstruct, po, OPERATION_LOGICAL_OR, append);std::cout << "7666" << endl;
				append = true;std::cout << "7667" << endl;
			} else {
				parseAdd(str2, mstruct, po);std::cout << "7669" << endl;
				b = true;std::cout << "7670" << endl;
			}
			i = str.find(LOGICAL_OR, 1);std::cout << "7672" << endl;
		}
		if(b) {
			parseAdd(str, mstruct, po, OPERATION_LOGICAL_OR, append);std::cout << "7675" << endl;
		} else {
			parseAdd(str, mstruct, po);std::cout << "7677" << endl;
		}
		return true;
	}
	/*if((i = str.find(LOGICAL_XOR, 1)) != string::npos && i + strlen(LOGICAL_XOR) != str.length()) {
		str2 = str.substr(0, i);std::cout << "7682" << endl;
		str = str.substr(i + strlen(LOGICAL_XOR), str.length() - (i + strlen(LOGICAL_XOR)));std::cout << "7683" << endl;
		parseAdd(str2, mstruct, po);std::cout << "7684" << endl;
		parseAdd(str, mstruct, po, OPERATION_LOGICAL_XOR);std::cout << "7685" << endl;
		return true;
	}*/
	if(po.base != BASE_ROMAN_NUMERALS && (i = str.find(BITWISE_OR, 1)) != string::npos && i + 1 != str.length()) {
		bool b = false, append = false;std::cout << "7689" << endl;
		while(i != string::npos && i + 1 != str.length()) {
			str2 = str.substr(0, i);std::cout << "7691" << endl;
			str = str.substr(i + 1, str.length() - (i + 1));std::cout << "7692" << endl;
			if(b) {
				parseAdd(str2, mstruct, po, OPERATION_BITWISE_OR, append);std::cout << "7694" << endl;
				append = true;std::cout << "7695" << endl;
			} else {
				parseAdd(str2, mstruct, po);std::cout << "7697" << endl;
				b = true;std::cout << "7698" << endl;
			}
			i = str.find(BITWISE_OR, 1);std::cout << "7700" << endl;
		}
		if(b) {
			parseAdd(str, mstruct, po, OPERATION_BITWISE_OR, append);std::cout << "7703" << endl;
		} else {
			parseAdd(str, mstruct, po);std::cout << "7705" << endl;
		}
		return true;
	}
	if((i = str.find('\a', 1)) != string::npos && i + 1 != str.length()) {
		str2 = str.substr(0, i);std::cout << "7710" << endl;
		str = str.substr(i + 1, str.length() - (i + 1));std::cout << "7711" << endl;
		parseAdd(str2, mstruct, po);std::cout << "7712" << endl;
		parseAdd(str, mstruct, po, OPERATION_BITWISE_XOR);std::cout << "7713" << endl;
		return true;
	}
	if((i = str.find(BITWISE_AND, 1)) != string::npos && i + 1 != str.length()) {
		bool b = false, append = false;std::cout << "7717" << endl;
		while(i != string::npos && i + 1 != str.length()) {
			str2 = str.substr(0, i);std::cout << "7719" << endl;
			str = str.substr(i + 1, str.length() - (i + 1));std::cout << "7720" << endl;
			if(b) {
				parseAdd(str2, mstruct, po, OPERATION_BITWISE_AND, append);std::cout << "7722" << endl;
				append = true;std::cout << "7723" << endl;
			} else {
				parseAdd(str2, mstruct, po);std::cout << "7725" << endl;
				b = true;std::cout << "7726" << endl;
			}
			i = str.find(BITWISE_AND, 1);std::cout << "7728" << endl;
		}
		if(b) {
			parseAdd(str, mstruct, po, OPERATION_BITWISE_AND, append);std::cout << "7731" << endl;
		} else {
			parseAdd(str, mstruct, po);std::cout << "7733" << endl;
		}
		return true;
	}
	if((i = str.find_first_of(LESS GREATER EQUALS NOT, 0)) != string::npos) {
		while(i != string::npos && ((str[i] == LOGICAL_NOT_CH && (i + 1 >= str.length() || str[i + 1] != EQUALS_CH)) || (str[i] == LESS_CH && i + 1 < str.length() && str[i + 1] == LESS_CH) || (str[i] == GREATER_CH && i + 1 < str.length() && str[i + 1] == GREATER_CH))) {
			i = str.find_first_of(LESS GREATER NOT EQUALS, i + 2);std::cout << "7739" << endl;
		}
	}
	if(i != string::npos) {
		bool b = false;std::cout << "7743" << endl;
		bool c = false;std::cout << "7744" << endl;
		while(i != string::npos && str[i] == NOT_CH && str.length() > i + 1 && str[i + 1] == NOT_CH) {
			i++;std::cout << "7746" << endl;
			if(i + 1 == str.length()) {
				c = true;std::cout << "7748" << endl;
			}
		}
		MathOperation s = OPERATION_ADD;std::cout << "7751" << endl;
		while(!c) {
			while(i != string::npos && ((str[i] == LOGICAL_NOT_CH && (i + 1 >= str.length() || str[i + 1] != EQUALS_CH)) || (str[i] == LESS_CH && i + 1 < str.length() && str[i + 1] == LESS_CH) || (str[i] == GREATER_CH && i + 1 < str.length() && str[i + 1] == GREATER_CH))) {
				i = str.find_first_of(LESS GREATER NOT EQUALS, i + 2);std::cout << "7754" << endl;
				while(i != string::npos && str[i] == NOT_CH && str.length() > i + 1 && str[i + 1] == NOT_CH) {
					i++;std::cout << "7756" << endl;
					if(i + 1 == str.length()) {
						i = string::npos;std::cout << "7758" << endl;
					}
				}
			}
			if(i == string::npos) {
				str2 = str.substr(0, str.length());std::cout << "7763" << endl;
			} else {
				str2 = str.substr(0, i);std::cout << "7765" << endl;
			}
			if(b) {
				switch(i3) {
					case EQUALS_CH: {s = OPERATION_EQUALS; break;}
					case GREATER_CH: {s = OPERATION_GREATER; break;}
					case LESS_CH: {s = OPERATION_LESS; break;}
					case GREATER_CH * EQUALS_CH: {s = OPERATION_EQUALS_GREATER; break;}
					case LESS_CH * EQUALS_CH: {s = OPERATION_EQUALS_LESS; break;}
					case GREATER_CH * LESS_CH: {s = OPERATION_NOT_EQUALS; break;}
				}
				parseAdd(str2, mstruct, po, s);std::cout << "7776" << endl;
			}
			if(i == string::npos) {
				return true;
			}
			if(!b) {
				parseAdd(str2, mstruct, po);std::cout << "7782" << endl;
				b = true;std::cout << "7783" << endl;
			}
			if(str.length() > i + 1 && is_in(LESS GREATER NOT EQUALS, str[i + 1])) {
				if(str[i] == str[i + 1]) {
					i3 = str[i];std::cout << "7787" << endl;
				} else {
					i3 = str[i] * str[i + 1];std::cout << "7789" << endl;
					if(i3 == NOT_CH * EQUALS_CH) {
						i3 = GREATER_CH * LESS_CH;std::cout << "7791" << endl;
					} else if(i3 == NOT_CH * LESS_CH) {
						i3 = GREATER_CH;std::cout << "7793" << endl;
					} else if(i3 == NOT_CH * GREATER_CH) {
						i3 = LESS_CH;std::cout << "7795" << endl;
					}
				}
				i++;std::cout << "7798" << endl;
			} else {
				i3 = str[i];std::cout << "7800" << endl;
			}
			str = str.substr(i + 1, str.length() - (i + 1));std::cout << "7802" << endl;
			i = str.find_first_of(LESS GREATER NOT EQUALS, 0);std::cout << "7803" << endl;
			while(i != string::npos && str[i] == NOT_CH && str.length() > i + 1 && str[i + 1] == NOT_CH) {
				i++;std::cout << "7805" << endl;
				if(i + 1 == str.length()) {
					i = string::npos;std::cout << "7807" << endl;
				}
			}
		}
	}
	i = str.find(SHIFT_LEFT, 1);std::cout << "7812" << endl;
	i2 = str.find(SHIFT_RIGHT, 1);std::cout << "7813" << endl;
	if(i2 != string::npos && (i == string::npos || i2 < i)) i = i2;
	if(i != string::npos && i + 2 != str.length()) {
		MathStructure mstruct1, mstruct2;std::cout << "7816" << endl;
		bool b_neg = (str[i] == '>');std::cout << "7817" << endl;
		str2 = str.substr(0, i);std::cout << "7818" << endl;
		str = str.substr(i + 2, str.length() - (i + 2));std::cout << "7819" << endl;
		parseAdd(str2, &mstruct1, po);std::cout << "7820" << endl;
		if(po.base < 10 && po.base >= 2 && str.find_first_not_of(NUMBERS SPACE PLUS MINUS) == string::npos) {
			for(i = 0; i < str.size(); i++) {
				if(str[i] >= '0' && str[i] <= '9' && po.base <= str[i] - '0') {
					ParseOptions po2 = po;std::cout << "7824" << endl;
					po2.base = BASE_DECIMAL;std::cout << "7825" << endl;
					parseAdd(str, &mstruct2, po2);std::cout << "7826" << endl;
					if(b_neg) {
						if(po.preserve_format) mstruct2.transform(STRUCT_NEGATE);
						else mstruct2.negate();std::cout << "7829" << endl;
					}
					mstruct->set(f_shift, &mstruct1, &mstruct2, &m_one, NULL);
					return true;
				}
			}
		}
		parseAdd(str, &mstruct2, po);std::cout << "7836" << endl;
		if(b_neg) {
			if(po.preserve_format) mstruct2.transform(STRUCT_NEGATE);
			else mstruct2.negate();std::cout << "7839" << endl;
		}
		mstruct->set(f_shift, &mstruct1, &mstruct2, &m_one, NULL);
		return true;
	}

	if((i = str.find_first_of(PLUS MINUS, 1)) != string::npos && i + 1 != str.length()) {
		bool b = false, c = false, append = false;std::cout << "7846" << endl;
		bool min = false;std::cout << "7847" << endl;
		while(i != string::npos && i + 1 != str.length()) {
			if(is_not_in(MULTIPLICATION_2 OPERATORS INTERNAL_OPERATORS EXPS, str[i - 1])) {
				str2 = str.substr(0, i);std::cout << "7850" << endl;
				if(!c && b) {
					bool b_add;std::cout << "7852" << endl;
					if(min) {
						b_add = parseAdd(str2, mstruct, po, OPERATION_SUBTRACT, append) && mstruct->isAddition();std::cout << "7854" << endl;
					} else {
						b_add = parseAdd(str2, mstruct, po, OPERATION_ADD, append) && mstruct->isAddition();std::cout << "7856" << endl;
					}
					append = true;std::cout << "7858" << endl;
					if(b_add) {
						int i_neg = 0;std::cout << "7860" << endl;
						MathStructure *mstruct_a = get_out_of_negate(mstruct->last(), &i_neg);std::cout << "7861" << endl;
						MathStructure *mstruct_b = mstruct_a;std::cout << "7862" << endl;
						if(mstruct_a->isMultiplication() && mstruct_a->size() >= 2) mstruct_b = &mstruct_a->last();
						if(mstruct_b->isVariable() && (mstruct_b->variable() == v_percent || mstruct_b->variable() == v_permille || mstruct_b->variable() == v_permyriad)) {
							Variable *v = mstruct_b->variable();std::cout << "7865" << endl;
							bool b_neg = (i_neg % 2 == 1);std::cout << "7866" << endl;
							while(i_neg > 0) {
								mstruct->last().setToChild(mstruct->last().size());std::cout << "7868" << endl;
								i_neg--;std::cout << "7869" << endl;
							}
							if(mstruct->last().isVariable()) {
								mstruct->last().multiply(m_one);std::cout << "7872" << endl;
								mstruct->last().swapChildren(1, 2);std::cout << "7873" << endl;
							}
							if(mstruct->last().size() > 2) {
								mstruct->last().delChild(mstruct->last().size());std::cout << "7876" << endl;
								mstruct->last().multiply(v);std::cout << "7877" << endl;
							}
							if(mstruct->last()[0].isNumber()) {
								if(b_neg) mstruct->last()[0].number().negate();
								if(v == v_percent) mstruct->last()[0].number().add(100);
								else if(v == v_permille) mstruct->last()[0].number().add(1000);
								else mstruct->last()[0].number().add(10000);std::cout << "7883" << endl;
							} else {
								if(b_neg && po.preserve_format) mstruct->last()[0].transform(STRUCT_NEGATE);
								else if(b_neg) mstruct->last()[0].negate();
								if(v == v_percent) mstruct->last()[0] += Number(100, 1);
								else if(v == v_permille) mstruct->last()[0] += Number(1000, 1);
								else mstruct->last()[0] += Number(10000, 1);std::cout << "7889" << endl;
								mstruct->last()[0].swapChildren(1, 2);std::cout << "7890" << endl;
							}
							if(mstruct->size() == 2) {
								mstruct->setType(STRUCT_MULTIPLICATION);std::cout << "7893" << endl;
							} else {
								MathStructure *mpercent = &mstruct->last();std::cout << "7895" << endl;
								mpercent->ref();std::cout << "7896" << endl;
								mstruct->delChild(mstruct->size());std::cout << "7897" << endl;
								mstruct->multiply_nocopy(mpercent);std::cout << "7898" << endl;
							}
						}
					}
				} else {
					if(!b && str2.empty()) {
						c = true;std::cout << "7904" << endl;
					} else {
						parseAdd(str2, mstruct, po);std::cout << "7906" << endl;
						if(c && min) {
							if(po.preserve_format) mstruct->transform(STRUCT_NEGATE);
							else mstruct->negate();std::cout << "7909" << endl;
						}
						c = false;std::cout << "7911" << endl;
					}
					b = true;std::cout << "7913" << endl;
				}
				min = str[i] == MINUS_CH;std::cout << "7915" << endl;
				str = str.substr(i + 1, str.length() - (i + 1));std::cout << "7916" << endl;
				i = str.find_first_of(PLUS MINUS, 1);std::cout << "7917" << endl;
			} else {
				i = str.find_first_of(PLUS MINUS, i + 1);std::cout << "7919" << endl;
			}
		}
		if(b) {
			if(c) {
				b = parseAdd(str, mstruct, po);std::cout << "7924" << endl;
				if(min) {
					if(po.preserve_format) mstruct->transform(STRUCT_NEGATE);
					else mstruct->negate();std::cout << "7927" << endl;
				}
				return b;
			} else {
				bool b_add;std::cout << "7931" << endl;
				if(min) {
					b_add = parseAdd(str, mstruct, po, OPERATION_SUBTRACT, append) && mstruct->isAddition();std::cout << "7933" << endl;
				} else {
					b_add = parseAdd(str, mstruct, po, OPERATION_ADD, append) && mstruct->isAddition();std::cout << "7935" << endl;
				}
				if(b_add) {
					int i_neg = 0;std::cout << "7938" << endl;
					MathStructure *mstruct_a = get_out_of_negate(mstruct->last(), &i_neg);std::cout << "7939" << endl;
					MathStructure *mstruct_b = mstruct_a;std::cout << "7940" << endl;
					if(mstruct_a->isMultiplication() && mstruct_a->size() >= 2) mstruct_b = &mstruct_a->last();
					if(mstruct_b->isVariable() && (mstruct_b->variable() == v_percent || mstruct_b->variable() == v_permille || mstruct_b->variable() == v_permyriad)) {
						Variable *v = mstruct_b->variable();std::cout << "7943" << endl;
						bool b_neg = (i_neg % 2 == 1);std::cout << "7944" << endl;
						while(i_neg > 0) {
							mstruct->last().setToChild(mstruct->last().size());std::cout << "7946" << endl;
							i_neg--;std::cout << "7947" << endl;
						}
						if(mstruct->last().isVariable()) {
							mstruct->last().multiply(m_one);std::cout << "7950" << endl;
							mstruct->last().swapChildren(1, 2);std::cout << "7951" << endl;
						}
						if(mstruct->last().size() > 2) {
							mstruct->last().delChild(mstruct->last().size());std::cout << "7954" << endl;
							mstruct->last().multiply(v);std::cout << "7955" << endl;
						}
						if(mstruct->last()[0].isNumber()) {
							if(b_neg) mstruct->last()[0].number().negate();
							if(v == v_percent) mstruct->last()[0].number().add(100);
							else if(v == v_permille) mstruct->last()[0].number().add(1000);
							else mstruct->last()[0].number().add(10000);std::cout << "7961" << endl;
						} else {
							if(b_neg && po.preserve_format) mstruct->last()[0].transform(STRUCT_NEGATE);
							else if(b_neg) mstruct->last()[0].negate();
							if(v == v_percent) mstruct->last()[0] += Number(100, 1);
							else if(v == v_permille) mstruct->last()[0] += Number(1000, 1);
							else mstruct->last()[0] += Number(10000, 1);std::cout << "7967" << endl;
							mstruct->last()[0].swapChildren(1, 2);std::cout << "7968" << endl;
						}
						if(mstruct->size() == 2) {
							mstruct->setType(STRUCT_MULTIPLICATION);std::cout << "7971" << endl;
						} else {
							MathStructure *mpercent = &mstruct->last();std::cout << "7973" << endl;
							mpercent->ref();std::cout << "7974" << endl;
							mstruct->delChild(mstruct->size());std::cout << "7975" << endl;
							mstruct->multiply_nocopy(mpercent);std::cout << "7976" << endl;
						}
					}
				}
			}
			return true;
		}
	}
	if(!po.rpn && po.parsing_mode == PARSING_MODE_ADAPTIVE && (i = str.find(DIVISION_CH, 1)) != string::npos && i + 1 != str.length()) {
		while(i != string::npos && i + 1 != str.length()) {
			bool b = false;std::cout << "7986" << endl;
			if(i > 2 && i < str.length() - 3 && str[i + 1] == ID_WRAP_LEFT_CH) {
				i2 = i;std::cout << "7988" << endl;
				b = true;std::cout << "7989" << endl;
				bool had_unit = false, had_nonunit = false;std::cout << "7990" << endl;
				MathStructure *m_temp = NULL, *m_temp2 = NULL;std::cout << "7991" << endl;
				while(b) {
					b = false;std::cout << "7993" << endl;
					size_t i4 = i2;std::cout << "7994" << endl;
					if(i2 > 2 && str[i2 - 1] == ID_WRAP_RIGHT_CH) {
						b = true;std::cout << "7996" << endl;
					} else if(i2 > 4 && str[i2 - 3] == ID_WRAP_RIGHT_CH && str[i2 - 2] == POWER_CH && is_in(NUMBERS, str[i2 - 1])) {
						b = true;std::cout << "7998" << endl;
						i4 -= 2;std::cout << "7999" << endl;
					}
					if(!b) {
						if((i2 > 1 && is_not_in(OPERATORS INTERNAL_OPERATORS MULTIPLICATION_2, str[i2 - 1])) || (i2 > 2 && str[i2 - 1] == MULTIPLICATION_2_CH && is_not_in(OPERATORS INTERNAL_OPERATORS, str[i2 - 2]))) had_nonunit = true;
						break;
					}
					i2 = str.rfind(ID_WRAP_LEFT_CH, i4 - 2);std::cout << "8005" << endl;
					m_temp = NULL;std::cout << "8006" << endl;
					if(i2 != string::npos) {
						int id = s2i(str.substr(i2 + 1, (i4 - 1) - (i2 + 1)));std::cout << "8008" << endl;
						if(priv->id_structs.find(id) != priv->id_structs.end()) m_temp = priv->id_structs[id];
					}
					if(!m_temp || !m_temp->isUnit()) {
						had_nonunit = true;std::cout << "8012" << endl;
						break;
					}
					had_unit = true;std::cout << "8015" << endl;
				}
				i3 = i;std::cout << "8017" << endl;
				b = had_unit && had_nonunit;std::cout << "8018" << endl;
				had_unit = false;std::cout << "8019" << endl;
				while(b) {
					size_t i4 = i3;std::cout << "8021" << endl;
					i3 = str.find(ID_WRAP_RIGHT_CH, i4 + 2);std::cout << "8022" << endl;
					m_temp2 = NULL;std::cout << "8023" << endl;
					if(i3 != string::npos) {
						int id = s2i(str.substr(i4 + 2, (i3 - 1) - (i4 + 1)));std::cout << "8025" << endl;
						if(priv->id_structs.find(id) != priv->id_structs.end()) m_temp2 = priv->id_structs[id];
					}
					if(!m_temp2 || !m_temp2->isUnit()) {
						b = false;std::cout << "8029" << endl;
						break;
					}
					had_unit = true;std::cout << "8032" << endl;
					b = false;std::cout << "8033" << endl;
					if(i3 < str.length() - 3 && str[i3 + 1] == ID_WRAP_LEFT_CH) {
						b = true;std::cout << "8035" << endl;
					} else if(i3 < str.length() - 5 && str[i3 + 3] == ID_WRAP_LEFT_CH && str[i3 + 1] == POWER_CH && is_in(NUMBERS, str[i3 + 2])) {
						b = true;std::cout << "8037" << endl;
						i3 += 2;std::cout << "8038" << endl;
					}
				}
				b = had_unit;std::cout << "8041" << endl;
				if(b) {
					if(i3 < str.length() - 2 && str[i3 + 1] == POWER_CH && is_in(NUMBERS, str[i3 + 2])) {
						i3 += 2;std::cout << "8044" << endl;
						while(i3 < str.length() - 1 && is_in(NUMBERS, str[i3 + 1])) i3++;
					}
					if(i3 == str.length() - 1 || (str[i3 + 1] != POWER_CH && str[i3 + 1] != DIVISION_CH)) {
						MathStructure *mstruct2 = new MathStructure();std::cout << "8048" << endl;
						str2 = str.substr(i2, i - i2);std::cout << "8049" << endl;
						parseAdd(str2, mstruct2, po);std::cout << "8050" << endl;
						str2 = str.substr(i + 1, i3 - i);std::cout << "8051" << endl;
						parseAdd(str2, mstruct2, po, OPERATION_DIVIDE);std::cout << "8052" << endl;
						str2 = ID_WRAP_LEFT;std::cout << "8053" << endl;
						str2 += i2s(addId(mstruct2));std::cout << "8054" << endl;
						str2 += ID_WRAP_RIGHT;std::cout << "8055" << endl;
						str.replace(i2, i3 - i2 + 1, str2);std::cout << "8056" << endl;
					} else {
						b = false;std::cout << "8058" << endl;
					}
				}
			}
			if(!b) {
				i2 = str.find_last_not_of(NUMBERS INTERNAL_NUMBER_CHARS PLUS MINUS EXPS, i - 1);std::cout << "8063" << endl;
				if(i2 == string::npos || (i2 != i - 1 && str[i2] == MULTIPLICATION_2_CH)) b = true;
				i2 = str.rfind(MULTIPLICATION_2_CH, i - 1);std::cout << "8065" << endl;
				if(i2 == string::npos) b = true;
				if(b) {
					i3 = str.find_first_of(MULTIPLICATION_2 "%" MULTIPLICATION DIVISION, i + 1);
					if(i3 == string::npos || i3 == i + 1 || str[i3] != MULTIPLICATION_2_CH) b = false;
					if(i3 < str.length() + 1 && (str[i3 + 1] == '%' || str[i3 + 1] == DIVISION_CH || str[i3 + 1] == MULTIPLICATION_CH || str[i3 + 1] == POWER_CH)) b = false;
				}
				if(b) {
					if(i3 != string::npos) str[i3] = MULTIPLICATION_CH;
					if(i2 != string::npos) str[i2] = MULTIPLICATION_CH;
				} else {
					if(str[i + 1] == MULTIPLICATION_2_CH) {
						str.erase(i + 1, 1);std::cout << "8077" << endl;
					}
					if(str[i - 1] == MULTIPLICATION_2_CH) {
						str.erase(i - 1, 1);std::cout << "8080" << endl;
						i--;std::cout << "8081" << endl;
					}
				}
			}
			i = str.find(DIVISION_CH, i + 1);std::cout << "8085" << endl;
		}
	}
	if(po.parsing_mode == PARSING_MODE_ADAPTIVE && !po.rpn) remove_blanks(str);
	if(po.parsing_mode == PARSING_MODE_CONVENTIONAL) {
		if((i = str.find(ID_WRAP_RIGHT_CH, 1)) != string::npos && i + 1 != str.length()) {
			while(i != string::npos && i + 1 != str.length()) {
				if(is_in(NUMBERS ID_WRAP_LEFT, str[i + 1])) {
					str.insert(i + 1, 1, MULTIPLICATION_CH);std::cout << "8093" << endl;
					i++;std::cout << "8094" << endl;
				}
				i = str.find(ID_WRAP_RIGHT_CH, i + 1);std::cout << "8096" << endl;
			}
		}
		if((i = str.find(ID_WRAP_LEFT_CH, 1)) != string::npos) {
			while(i != string::npos) {
				if(is_in(NUMBERS, str[i - 1])) {
					str.insert(i, 1, MULTIPLICATION_CH);std::cout << "8102" << endl;
					i++;std::cout << "8103" << endl;
				}
				i = str.find(ID_WRAP_LEFT_CH, i + 1);std::cout << "8105" << endl;
			}
		}
	}
	if((i = str.find_first_of(MULTIPLICATION DIVISION "%", 0)) != string::npos && i + 1 != str.length()) {
		bool b = false, append = false;std::cout << "8110" << endl;
		int type = 0;std::cout << "8111" << endl;
		while(i != string::npos && i + 1 != str.length()) {
			if(i < 1) {
				if(i < 1 && str.find_first_not_of(MULTIPLICATION_2 OPERATORS INTERNAL_OPERATORS EXPS) == string::npos) {
					gsub("\a", str.find_first_of(OPERATORS "%") != string::npos ? " xor " : "xor", str);
					error(false, _("Misplaced operator(s) \"%s\" ignored"), str.c_str(), NULL);
					return b;
				}
				i = 1;std::cout << "8119" << endl;
				while(i < str.length() && is_in(MULTIPLICATION DIVISION "%", str[i])) {
					i++;std::cout << "8121" << endl;
				}
				string errstr = str.substr(0, i);std::cout << "8123" << endl;
				gsub("\a", str.find_first_of(OPERATORS "%") != string::npos ? " xor " : "xor", errstr);
				error(false, _("Misplaced operator(s) \"%s\" ignored"), errstr.c_str(), NULL);
				str = str.substr(i, str.length() - i);std::cout << "8126" << endl;
				i = str.find_first_of(MULTIPLICATION DIVISION "%", 0);
			} else {
				str2 = str.substr(0, i);std::cout << "8129" << endl;
				if(b) {
					switch(type) {
						case 1: {
							parseAdd(str2, mstruct, po, OPERATION_DIVIDE, append);std::cout << "8133" << endl;
							break;
						}
						case 2: {
							MathStructure *mstruct2 = new MathStructure();std::cout << "8137" << endl;
							parseAdd(str2, mstruct2, po);std::cout << "8138" << endl;
							mstruct->transform(f_rem);
							mstruct->addChild_nocopy(mstruct2);std::cout << "8140" << endl;
							break;
						}
						case 3: {
							parseAdd(str2, mstruct, po, OPERATION_DIVIDE, append);std::cout << "8144" << endl;
							mstruct->transform(f_trunc);
							break;
						}
						case 4: {
							MathStructure *mstruct2 = new MathStructure();std::cout << "8149" << endl;
							parseAdd(str2, mstruct2, po);std::cout << "8150" << endl;
							mstruct->transform(f_mod);
							mstruct->addChild_nocopy(mstruct2);std::cout << "8152" << endl;
							break;
						}
						default: {
							parseAdd(str2, mstruct, po, OPERATION_MULTIPLY, append);std::cout << "8156" << endl;
						}
					}
					append = true;std::cout << "8159" << endl;
				} else {
					parseAdd(str2, mstruct, po);std::cout << "8161" << endl;
					b = true;std::cout << "8162" << endl;
				}
				if(str[i] == DIVISION_CH) {
					if(str[i + 1] == DIVISION_CH) {type = 3; i++;}
					else type = 1;std::cout << "8166" << endl;
				} else if(str[i] == '%') {
					if(str[i + 1] == '%') {type = 4; i++;}
					else type = 2;std::cout << "8169" << endl;
				} else {
					type = 0;std::cout << "8171" << endl;
				}
				if(is_in(MULTIPLICATION DIVISION "%", str[i + 1])) {
					i2 = 1;std::cout << "8174" << endl;
					while(i2 + i + 1 != str.length() && is_in(MULTIPLICATION DIVISION "%", str[i2 + i + 1])) {
						i2++;std::cout << "8176" << endl;
					}
					string errstr = str.substr(i, i2);std::cout << "8178" << endl;
					gsub("\a", str.find_first_of(OPERATORS "%") != string::npos ? " xor " : "xor", errstr);
					error(false, _("Misplaced operator(s) \"%s\" ignored"), errstr.c_str(), NULL);
					i += i2;std::cout << "8181" << endl;
				}
				str = str.substr(i + 1, str.length() - (i + 1));std::cout << "8183" << endl;
				i = str.find_first_of(MULTIPLICATION DIVISION "%", 0);
			}
		}
		if(b) {
			switch(type) {
				case 1: {
					parseAdd(str, mstruct, po, OPERATION_DIVIDE, append);std::cout << "8190" << endl;
					break;
				}
				case 2: {
					MathStructure *mstruct2 = new MathStructure();std::cout << "8194" << endl;
					parseAdd(str, mstruct2, po);std::cout << "8195" << endl;
					mstruct->transform(f_rem);
					mstruct->addChild_nocopy(mstruct2);std::cout << "8197" << endl;
					break;
				}
				case 3: {
					parseAdd(str, mstruct, po, OPERATION_DIVIDE, append);std::cout << "8201" << endl;
					mstruct->transform(f_trunc);
					break;
				}
				case 4: {
					MathStructure *mstruct2 = new MathStructure();std::cout << "8206" << endl;
					parseAdd(str, mstruct2, po);std::cout << "8207" << endl;
					mstruct->transform(f_mod);
					mstruct->addChild_nocopy(mstruct2);std::cout << "8209" << endl;
					break;
				}
				default: {
					parseAdd(str, mstruct, po, OPERATION_MULTIPLY, append);std::cout << "8213" << endl;
				}
			}
			return true;
		}
	}

	if((i = str.find('\x1c', 0)) != string::npos && i + 1 != str.length()) {
		if(i != 0) str2 = str.substr(0, i);
		str = str.substr(i + 1, str.length() - (i + 1));std::cout << "8222" << endl;
		if(i != 0) parseAdd(str2, mstruct, po);
		else mstruct->set(1, 1, 0);std::cout << "8224" << endl;
		if(parseAdd(str, mstruct, po, OPERATION_MULTIPLY)) {
			if(po.angle_unit != ANGLE_UNIT_NONE && po.angle_unit != ANGLE_UNIT_RADIANS && mstruct->last().contains(getRadUnit(), false, true, true) <= 0 && mstruct->last().contains(getGraUnit(), false, true, true) <= 0 && mstruct->last().contains(getDegUnit(), false, true, true) <= 0) {
				switch(po.angle_unit) {
					case ANGLE_UNIT_DEGREES: {mstruct->last().multiply(getDegUnit()); break;}
					case ANGLE_UNIT_GRADIANS: {mstruct->last().multiply(getGraUnit()); break;}
					default: {}
				}
			}
			mstruct->last().transform(priv->f_cis);
		}
		return true;
	}

	if(str.empty()) return false;
	if(str.find_first_not_of(OPERATORS INTERNAL_OPERATORS SPACE) == string::npos && (po.base != BASE_ROMAN_NUMERALS || str.find_first_of("(|)") == string::npos)) {
		gsub("\a", str.find_first_of(OPERATORS "%") != string::npos ? " xor " : "xor", str);
		gsub("\x1c", "∠", str);
		error(false, _("Misplaced operator(s) \"%s\" ignored"), str.c_str(), NULL);
		return false;
	}

	i = 0;std::cout << "8246" << endl;
	bool ret = true;std::cout << "8247" << endl;
	bool has_sign = false;std::cout << "8248" << endl;
	int minus_count = 0;std::cout << "8249" << endl;
	while(i < str.length()) {
		if(str[i] == MINUS_CH) {
			has_sign = true;std::cout << "8252" << endl;
			minus_count++;std::cout << "8253" << endl;
			str.erase(i, 1);std::cout << "8254" << endl;
		} else if(str[i] == PLUS_CH) {
			has_sign = true;std::cout << "8256" << endl;
			str.erase(i, 1);std::cout << "8257" << endl;
		} else if(str[i] == SPACE_CH) {
			str.erase(i, 1);std::cout << "8259" << endl;
		} else if(str[i] == BITWISE_NOT_CH || str[i] == LOGICAL_NOT_CH) {
			break;
		} else if(is_in(OPERATORS INTERNAL_OPERATORS, str[i]) && (po.base != BASE_ROMAN_NUMERALS || (str[i] != '(' && str[i] != ')' && str[i] != '|'))) {
			if(str[i] == '\a') error(false, _("Misplaced operator(s) \"%s\" ignored"), "xor", NULL);
			else if(str[i] == '\x1c') error(false, _("Misplaced operator(s) \"%s\" ignored"), "∠", NULL);
			else error(false, _("Misplaced '%c' ignored"), str[i], NULL);
			str.erase(i, 1);std::cout << "8266" << endl;
		} else {
			break;
		}
	}

	if(!str.empty() && (str[0] == BITWISE_NOT_CH || str[0] == LOGICAL_NOT_CH)) {
		bool bit = (str[0] == BITWISE_NOT_CH);std::cout << "8273" << endl;
		str.erase(0, 1);std::cout << "8274" << endl;
		parseAdd(str, mstruct, po);std::cout << "8275" << endl;
		if(bit) mstruct->setBitwiseNot();
		else mstruct->setLogicalNot();std::cout << "8277" << endl;
		if(po.preserve_format) {
			while(minus_count > 0) {
				mstruct->transform(STRUCT_NEGATE);
				minus_count--;std::cout << "8281" << endl;
			}
		} else if(minus_count % 2 == 1) {
			mstruct->negate();std::cout << "8284" << endl;
		}
		return true;
	}

	if(str.empty()) {
		if(minus_count % 2 == 1 && !po.preserve_format) {
			mstruct->set(-1, 1, 0);std::cout << "8291" << endl;
		} else if(has_sign) {
			mstruct->set(1, 1, 0);std::cout << "8293" << endl;
			if(po.preserve_format) {
				while(minus_count > 0) {
					mstruct->transform(STRUCT_NEGATE);
					minus_count--;std::cout << "8297" << endl;
				}
			}
		}
		return false;
	}
	if((i = str.find(ID_WRAP_RIGHT_CH, 1)) != string::npos && i + 1 != str.length()) {
		bool b = false, append = false;std::cout << "8304" << endl;
		while(i != string::npos && i + 1 != str.length()) {
			if(str[i + 1] != POWER_CH && str[i + 1] != '\b') {
				str2 = str.substr(0, i + 1);std::cout << "8307" << endl;
				str = str.substr(i + 1, str.length() - (i + 1));std::cout << "8308" << endl;
				if(b) {
					parseAdd(str2, mstruct, po, OPERATION_MULTIPLY, append);std::cout << "8310" << endl;
					append = true;std::cout << "8311" << endl;
				} else {
					parseAdd(str2, mstruct, po);std::cout << "8313" << endl;
					b = true;std::cout << "8314" << endl;
				}
				i = str.find(ID_WRAP_RIGHT_CH, 1);std::cout << "8316" << endl;
			} else {
				i = str.find(ID_WRAP_RIGHT_CH, i + 1);std::cout << "8318" << endl;
			}
		}
		if(b) {
			parseAdd(str, mstruct, po, OPERATION_MULTIPLY, append);std::cout << "8322" << endl;
			if(po.parsing_mode == PARSING_MODE_ADAPTIVE && mstruct->isMultiplication() && mstruct->size() >= 2 && !(*mstruct)[0].inParentheses()) {
				Unit *u1 = NULL;std::cout << "8324" << endl; Prefix *p1 = NULL;std::cout << "8324" << endl;
				bool b_plus = false;std::cout << "8325" << endl;
				if((*mstruct)[0].isMultiplication() && (*mstruct)[0].size() == 2 && (*mstruct)[0][0].isNumber() && (*mstruct)[0][1].isUnit()) {u1 = (*mstruct)[0][1].unit(); p1 = (*mstruct)[0][1].prefix();}
				if(u1 && u1->subtype() == SUBTYPE_BASE_UNIT && (u1->referenceName() == "m" || (!p1 && u1->referenceName() == "L")) && (!p1 || (p1->type() == PREFIX_DECIMAL && ((DecimalPrefix*) p1)->exponent() <= 3 && ((DecimalPrefix*) p1)->exponent() > -3))) {
					b_plus = true;std::cout << "8328" << endl;
					for(size_t i2 = 1; i2 < mstruct->size(); i2++) {
						if(!(*mstruct)[i2].inParentheses() && (*mstruct)[i2].isMultiplication() && (*mstruct)[i2].size() == 2 && (*mstruct)[i2][0].isNumber() && (*mstruct)[i2][1].isUnit() && (*mstruct)[i2][1].unit() == u1) {
							Prefix *p2 = (*mstruct)[i2][1].prefix();std::cout << "8331" << endl;
							if(p1 && p2) b_plus = p1->type() == PREFIX_DECIMAL && p2->type() == PREFIX_DECIMAL && ((DecimalPrefix*) p1)->exponent() > ((DecimalPrefix*) p2)->exponent() && ((DecimalPrefix*) p2)->exponent() >= -3;
							else if(p2) b_plus = p2->type() == PREFIX_DECIMAL && ((DecimalPrefix*) p2)->exponent() < 0 && ((DecimalPrefix*) p2)->exponent() >= -3;
							else if(p1) b_plus = p1->type() == PREFIX_DECIMAL && ((DecimalPrefix*) p1)->exponent() > 1;
							else b_plus = false;std::cout << "8335" << endl;
							if(!b_plus) break;
							p1 = p2;std::cout << "8337" << endl;
						} else {
							b_plus = false;std::cout << "8339" << endl;
							break;
						}
					}
				} else if(u1 && !p1 && u1->subtype() == SUBTYPE_ALIAS_UNIT && ((AliasUnit*) u1)->mixWithBase()) {
					b_plus = true;std::cout << "8344" << endl;
					for(size_t i2 = 1; i2 < mstruct->size(); i2++) {
						if(!(*mstruct)[i2].inParentheses() && (*mstruct)[i2].isMultiplication() && (*mstruct)[i2].size() == 2 && (*mstruct)[i2][0].isNumber() && (*mstruct)[i2][1].isUnit() && u1->isChildOf((*mstruct)[i2][1].unit()) && !(*mstruct)[i2][1].prefix() && (i2 == mstruct->size() - 1 || ((*mstruct)[i2][1].unit()->subtype() == SUBTYPE_ALIAS_UNIT && ((AliasUnit*) (*mstruct)[i2][1].unit())->mixWithBase()))) {
							while(((AliasUnit*) u1)->firstBaseUnit() != (*mstruct)[i2][1].unit()) {
								u1 = ((AliasUnit*) u1)->firstBaseUnit();std::cout << "8348" << endl;
								if(u1->subtype() != SUBTYPE_ALIAS_UNIT || !((AliasUnit*) u1)->mixWithBase()) {
									b_plus = false;std::cout << "8350" << endl;
									break;
								}
							}
							if(!b_plus) break;
							u1 = (*mstruct)[i2][1].unit();std::cout << "8355" << endl;
						} else {
							b_plus = false;std::cout << "8357" << endl;
							break;
						}
					}
				}
				if(b_plus) mstruct->setType(STRUCT_ADDITION);
			}
			if(po.preserve_format) {
				while(minus_count > 0) {
					mstruct->transform(STRUCT_NEGATE);
					minus_count--;std::cout << "8367" << endl;
				}
			} else if(minus_count % 2 == 1) {
				mstruct->negate();std::cout << "8370" << endl;
			}
			return true;
		}
	}
	if((i = str.find(ID_WRAP_LEFT_CH, 1)) != string::npos) {
		bool b = false, append = false;std::cout << "8376" << endl;
		while(i != string::npos) {
			if(str[i - 1] != POWER_CH && (i < 2 || str[i - 1] != MINUS_CH || str[i - 2] != POWER_CH) && str[i - 1] != '\b') {
				str2 = str.substr(0, i);std::cout << "8379" << endl;
				str = str.substr(i, str.length() - i);std::cout << "8380" << endl;
				if(b) {
					parseAdd(str2, mstruct, po, OPERATION_MULTIPLY, append);std::cout << "8382" << endl;
					append = true;std::cout << "8383" << endl;
				} else {
					parseAdd(str2, mstruct, po);std::cout << "8385" << endl;
					b = true;std::cout << "8386" << endl;
				}
				i = str.find(ID_WRAP_LEFT_CH, 1);std::cout << "8388" << endl;
			} else {
				i = str.find(ID_WRAP_LEFT_CH, i + 1);std::cout << "8390" << endl;
			}
		}
		if(b) {
			parseAdd(str, mstruct, po, OPERATION_MULTIPLY, append);std::cout << "8394" << endl;
			if(po.preserve_format) {
				while(minus_count > 0) {
					mstruct->transform(STRUCT_NEGATE);
					minus_count--;std::cout << "8398" << endl;
				}
			} else if(minus_count % 2 == 1) {
				mstruct->negate();std::cout << "8401" << endl;
			}
			return true;
		}
	}
	if((i = str.find(POWER_CH, 1)) != string::npos && i + 1 != str.length()) {
		str2 = str.substr(0, i);std::cout << "8407" << endl;
		str = str.substr(i + 1, str.length() - (i + 1));std::cout << "8408" << endl;
		parseAdd(str2, mstruct, po);std::cout << "8409" << endl;
		parseAdd(str, mstruct, po, OPERATION_RAISE);std::cout << "8410" << endl;
	} else if((i = str.find("\b", 1)) != string::npos && i + 1 != str.length()) {
		str2 = str.substr(0, i);std::cout << "8412" << endl;
		str = str.substr(i + 1, str.length() - (i + 1));std::cout << "8413" << endl;
		MathStructure *mstruct2 = new MathStructure;std::cout << "8414" << endl;
		if(po.read_precision != DONT_READ_PRECISION) {
			ParseOptions po2 = po;std::cout << "8416" << endl;
			po2.read_precision = DONT_READ_PRECISION;std::cout << "8417" << endl;
			parseAdd(str2, mstruct, po2);std::cout << "8418" << endl;
			parseAdd(str, mstruct2, po2);std::cout << "8419" << endl;
		} else {
			parseAdd(str2, mstruct, po);std::cout << "8421" << endl;
			parseAdd(str, mstruct2, po);std::cout << "8422" << endl;
		}
		mstruct->transform(f_uncertainty);
		mstruct->addChild_nocopy(mstruct2);std::cout << "8425" << endl;
		mstruct->addChild(m_zero);std::cout << "8426" << endl;
	} else if(BASE_2_10 && (i = str.find_first_of(EXPS, 1)) != string::npos && i + 1 != str.length() && str.find("\b") == string::npos) {
		str2 = str.substr(0, i);std::cout << "8428" << endl;
		str = str.substr(i + 1, str.length() - (i + 1));std::cout << "8429" << endl;
		parseAdd(str2, mstruct, po);std::cout << "8430" << endl;
		parseAdd(str, mstruct, po, OPERATION_EXP10);std::cout << "8431" << endl;
	} else if((i = str.find(ID_WRAP_LEFT_CH, 1)) != string::npos && i + 1 != str.length() && str.find(ID_WRAP_RIGHT_CH, i + 1) && str.find_first_not_of(PLUS MINUS, 0) != i) {
		str2 = str.substr(0, i);std::cout << "8433" << endl;
		str = str.substr(i, str.length() - i);std::cout << "8434" << endl;
		parseAdd(str2, mstruct, po);std::cout << "8435" << endl;
		parseAdd(str, mstruct, po, OPERATION_MULTIPLY);std::cout << "8436" << endl;
	} else if(str.length() > 0 && str[0] == ID_WRAP_LEFT_CH && (i = str.find(ID_WRAP_RIGHT_CH, 1)) != string::npos && i + 1 != str.length()) {
		str2 = str.substr(0, i + 1);std::cout << "8438" << endl;
		str = str.substr(i + 1, str.length() - (i + 1));std::cout << "8439" << endl;
		parseAdd(str2, mstruct, po);std::cout << "8440" << endl;
		parseAdd(str, mstruct, po, OPERATION_MULTIPLY);std::cout << "8441" << endl;
	} else {
		ret = parseNumber(mstruct, str, po);std::cout << "8443" << endl;
	}
	if(po.preserve_format) {
		while(minus_count > 0) {
			mstruct->transform(STRUCT_NEGATE);
			minus_count--;std::cout << "8448" << endl;
		}
	} else if(minus_count % 2 == 1) {
		mstruct->negate();std::cout << "8451" << endl;
	}
	return ret;
}

string Calculator::getName(string name, ExpressionItem *object, bool force, bool always_append) {
	ExpressionItem *item = NULL;std::cout << "8457" << endl;
	if(!object) {
	} else if(object->type() == TYPE_FUNCTION) {
		item = getActiveFunction(name);std::cout << "8460" << endl;
	} else {
		item = getActiveVariable(name);std::cout << "8462" << endl;
		if(!item) {
			item = getActiveUnit(name);std::cout << "8464" << endl;
		}
		if(!item) {
			item = getCompositeUnit(name);std::cout << "8467" << endl;
		}
	}
	if(item && force && !name.empty() && item != object && object) {
		if(!item->isLocal()) {
			bool b = item->hasChanged();std::cout << "8472" << endl;
			if(object->isActive()) {
				item->setActive(false);std::cout << "8474" << endl;
			}
			if(!object->isLocal()) {
				item->setChanged(b);std::cout << "8477" << endl;
			}
		} else {
			if(object->isActive()) {
				item->destroy();std::cout << "8481" << endl;
			}
		}
		return name;
	}
	int i2 = 1;std::cout << "8486" << endl;
	bool changed = false;std::cout << "8487" << endl;
	if(name.empty()) {
		name = "var";
		always_append = true;std::cout << "8490" << endl;
		item = NULL;std::cout << "8491" << endl;
		changed = true;std::cout << "8492" << endl;
	}
	string stmp = name;std::cout << "8494" << endl;
	if(always_append) {
		stmp += NAME_NUMBER_PRE_STR;std::cout << "8496" << endl;
		stmp += "1";
	}
	if(changed || (item && item != object)) {
		if(item) {
			i2++;std::cout << "8501" << endl;
			stmp = name;std::cout << "8502" << endl;
			stmp += NAME_NUMBER_PRE_STR;std::cout << "8503" << endl;
			stmp += i2s(i2);std::cout << "8504" << endl;
		}
		while(true) {
			if(!object) {
				item = getActiveFunction(stmp);std::cout << "8508" << endl;
				if(!item) {
					item = getActiveVariable(stmp);std::cout << "8510" << endl;
				}
				if(!item) {
					item = getActiveUnit(stmp);std::cout << "8513" << endl;
				}
				if(!item) {
					item = getCompositeUnit(stmp);std::cout << "8516" << endl;
				}
			} else if(object->type() == TYPE_FUNCTION) {
				item = getActiveFunction(stmp);std::cout << "8519" << endl;
			} else {
				item = getActiveVariable(stmp);std::cout << "8521" << endl;
				if(!item) {
					item = getActiveUnit(stmp);std::cout << "8523" << endl;
				}
				if(!item) {
					item = getCompositeUnit(stmp);std::cout << "8526" << endl;
				}
			}
			if(item && item != object) {
				i2++;std::cout << "8530" << endl;
				stmp = name;std::cout << "8531" << endl;
				stmp += NAME_NUMBER_PRE_STR;std::cout << "8532" << endl;
				stmp += i2s(i2);std::cout << "8533" << endl;
			} else {
				break;
			}
		}
	}
	if(i2 > 1 && !always_append) {
		error(false, _("Name \"%s\" is in use. Replacing with \"%s\"."), name.c_str(), stmp.c_str(), NULL);
	}
	return stmp;
}

bool Calculator::loadGlobalDefinitions() {
	bool b = true;std::cout << "8546" << endl;
	if(!loadDefinitions(buildPath(getGlobalDefinitionsDir(), "prefixes.xml").c_str(), false)) b = false;
	if(!loadDefinitions(buildPath(getGlobalDefinitionsDir(), "currencies.xml").c_str(), false)) b = false;
	if(!loadDefinitions(buildPath(getGlobalDefinitionsDir(), "units.xml").c_str(), false)) b = false;
	if(!loadDefinitions(buildPath(getGlobalDefinitionsDir(), "functions.xml").c_str(), false)) b = false;
	if(!loadDefinitions(buildPath(getGlobalDefinitionsDir(), "datasets.xml").c_str(), false)) b = false;
	if(!loadDefinitions(buildPath(getGlobalDefinitionsDir(), "variables.xml").c_str(), false)) b = false;
	return b;
}
bool Calculator::loadGlobalDefinitions(string filename) {
	return loadDefinitions(buildPath(getGlobalDefinitionsDir(), filename).c_str(), false);
}
bool Calculator::loadGlobalPrefixes() {
	return loadGlobalDefinitions("prefixes.xml");
}
bool Calculator::loadGlobalCurrencies() {
	return loadGlobalDefinitions("currencies.xml");
}
bool Calculator::loadGlobalUnits() {
	bool b = loadGlobalDefinitions("currencies.xml");
	return loadGlobalDefinitions("units.xml") && b;
}
bool Calculator::loadGlobalVariables() {
	return loadGlobalDefinitions("variables.xml");
}
bool Calculator::loadGlobalFunctions() {
	return loadGlobalDefinitions("functions.xml");
}
bool Calculator::loadGlobalDataSets() {
	return loadGlobalDefinitions("datasets.xml");
}
bool Calculator::loadLocalDefinitions() {
	string homedir = buildPath(getLocalDataDir(), "definitions");
	if(!dirExists(homedir)) {
		string homedir_old = buildPath(getOldLocalDir(), "definitions");
		if(dirExists(homedir)) {
			if(!dirExists(getLocalDataDir())) {
				recursiveMakeDir(getLocalDataDir());std::cout << "8583" << endl;
			}
			if(makeDir(homedir)) {
				list<string> eps_old;std::cout << "8586" << endl;
				struct dirent *ep_old;std::cout << "8587" << endl;
				DIR *dp_old = opendir(homedir_old.c_str());std::cout << "8588" << endl;
				if(dp_old) {
					while((ep_old = readdir(dp_old))) {
#ifdef _DIRENT_HAVE_D_TYPE
						if(ep_old->d_type != DT_DIR) {
#endif
							if(strcmp(ep_old->d_name, "..") != 0 && strcmp(ep_old->d_name, ".") != 0 && strcmp(ep_old->d_name, "datasets") != 0) {
								eps_old.push_back(ep_old->d_name);std::cout << "8595" << endl;
							}
#ifdef _DIRENT_HAVE_D_TYPE
						}
#endif
					}
					closedir(dp_old);std::cout << "8601" << endl;
				}
				for(list<string>::iterator it = eps_old.begin(); it != eps_old.end(); ++it) {
					move_file(buildPath(homedir_old, *it).c_str(), buildPath(homedir, *it).c_str());std::cout << "8604" << endl;
				}
				if(removeDir(homedir_old)) {
					removeDir(getOldLocalDir());std::cout << "8607" << endl;
				}
			}
		}
	}
	list<string> eps;std::cout << "8612" << endl;
	struct dirent *ep;std::cout << "8613" << endl;
	DIR *dp = opendir(homedir.c_str());std::cout << "8614" << endl;
	if(dp) {
		while((ep = readdir(dp))) {
#ifdef _DIRENT_HAVE_D_TYPE
			if(ep->d_type != DT_DIR) {
#endif
				if(strcmp(ep->d_name, "..") != 0 && strcmp(ep->d_name, ".") != 0 && strcmp(ep->d_name, "datasets") != 0) {
					eps.push_back(ep->d_name);std::cout << "8621" << endl;
				}
#ifdef _DIRENT_HAVE_D_TYPE
			}
#endif
		}
		closedir(dp);std::cout << "8627" << endl;
	}
	eps.sort();std::cout << "8629" << endl;
	for(list<string>::iterator it = eps.begin(); it != eps.end(); ++it) {
		loadDefinitions(buildPath(homedir, *it).c_str(), (*it) == "functions.xml" || (*it) == "variables.xml" || (*it) == "units.xml" || (*it) == "datasets.xml", true);
	}
	for(size_t i = 0; i < variables.size(); i++) {
		if(!variables[i]->isLocal() && !variables[i]->isActive() && !getActiveExpressionItem(variables[i])) variables[i]->setActive(true);
	}
	for(size_t i = 0; i < units.size(); i++) {
		if(!units[i]->isLocal() && !units[i]->isActive() && !getActiveExpressionItem(units[i])) units[i]->setActive(true);
	}
	for(size_t i = 0; i < functions.size(); i++) {
		if(!functions[i]->isLocal() && !functions[i]->isActive() && !getActiveExpressionItem(functions[i])) functions[i]->setActive(true);
	}
	return true;
}

#define ITEM_SAVE_BUILTIN_NAMES\
	if(!is_user_defs) {item->setRegistered(false);} \
	for(size_t i = 1; i <= item->countNames(); i++) { \
		if(item->getName(i).reference) { \
			for(size_t i2 = 0; i2 < 10; i2++) { \
				if(ref_names[i2].name.empty()) { \
					ref_names[i2] = item->getName(i);std::cout << "8651" << endl; \
					break; \
				} \
			} \
		} \
	} \
	item->clearNames();

#define ITEM_SET_BEST_NAMES(validation) \
	size_t names_i = 0, i2 = 0; \
	string *str_names; \
	if(best_names == "-") {best_names = ""; nextbest_names = "";} \
	if(!best_names.empty()) {str_names = &best_names;} \
	else if(!nextbest_names.empty()) {str_names = &nextbest_names;} \
	else {str_names = &default_names;} \
	if(!str_names->empty() && (*str_names)[0] == '!') { \
		names_i = str_names->find('!', 1) + 1;std::cout << "8667" << endl; \
	} \
	while(true) { \
		size_t i3 = names_i;std::cout << "8670" << endl; \
		names_i = str_names->find(",", i3); \
		if(i2 == 0) { \
			i2 = str_names->find(":", i3); \
		} \
		bool case_set = false;std::cout << "8675" << endl; \
		ename.unicode = false;std::cout << "8676" << endl; \
		ename.abbreviation = false;std::cout << "8677" << endl; \
		ename.case_sensitive = false;std::cout << "8678" << endl; \
		ename.suffix = false;std::cout << "8679" << endl; \
		ename.avoid_input = false;std::cout << "8680" << endl; \
		ename.completion_only = false;std::cout << "8681" << endl; \
		ename.reference = false;std::cout << "8682" << endl; \
		ename.plural = false;std::cout << "8683" << endl; \
		if(i2 < names_i) { \
			bool b = true;std::cout << "8685" << endl; \
			for(; i3 < i2; i3++) { \
				switch((*str_names)[i3]) { \
					case '-': {b = false; break;} \
					case 'a': {ename.abbreviation = b; b = true; break;} \
					case 'c': {ename.case_sensitive = b; b = true; case_set = true; break;} \
					case 'i': {ename.avoid_input = b; b = true; break;} \
					case 'p': {ename.plural = b; b = true; break;} \
					case 'r': {ename.reference = b; b = true; break;} \
					case 's': {ename.suffix = b; b = true; break;} \
					case 'u': {ename.unicode = b; b = true; break;} \
					case 'o': {ename.completion_only = b; b = true; break;} \
				} \
			} \
			i3++;std::cout << "8699" << endl; \
			i2 = 0;std::cout << "8700" << endl; \
		} \
		if(names_i == string::npos) {ename.name = str_names->substr(i3, str_names->length() - i3);} \
		else {ename.name = str_names->substr(i3, names_i - i3);std::cout << "8703" << endl;} \
		remove_blank_ends(ename.name);std::cout << "8704" << endl; \
		if(!ename.name.empty() && validation(ename.name, version_numbers, is_user_defs)) { \
			if(!case_set) { \
				ename.case_sensitive = ename.abbreviation || text_length_is_one(ename.name);std::cout << "8707" << endl; \
			} \
			item->addName(ename);std::cout << "8709" << endl; \
		} \
		if(names_i == string::npos) {break;} \
		names_i++;std::cout << "8712" << endl; \
	}

#define ITEM_SET_BUILTIN_NAMES \
	for(size_t i = 0; i < 10; i++) { \
		if(ref_names[i].name.empty()) { \
			break; \
		} else { \
			size_t i4 = item->hasName(ref_names[i].name, ref_names[i].case_sensitive);std::cout << "8720" << endl; \
			if(i4 > 0) { \
				const ExpressionName *enameptr = &item->getName(i4);std::cout << "8722" << endl; \
				ref_names[i].case_sensitive = enameptr->case_sensitive;std::cout << "8723" << endl; \
				ref_names[i].abbreviation = enameptr->abbreviation;std::cout << "8724" << endl; \
				ref_names[i].avoid_input = enameptr->avoid_input;std::cout << "8725" << endl; \
				ref_names[i].completion_only = enameptr->completion_only;std::cout << "8726" << endl; \
				ref_names[i].plural = enameptr->plural;std::cout << "8727" << endl; \
				ref_names[i].suffix = enameptr->suffix;std::cout << "8728" << endl; \
				item->setName(ref_names[i], i4);std::cout << "8729" << endl; \
			} else { \
				item->addName(ref_names[i]);std::cout << "8731" << endl; \
			} \
			ref_names[i].name = ""; \
		} \
	} \
	if(!is_user_defs) { \
		item->setRegistered(true);std::cout << "8737" << endl; \
		nameChanged(item);std::cout << "8738" << endl; \
	}

#define ITEM_SET_REFERENCE_NAMES(validation) \
	if(str_names != &default_names && !default_names.empty()) { \
		if(default_names[0] == '!') { \
			names_i = default_names.find('!', 1) + 1;std::cout << "8744" << endl; \
		} else { \
			names_i = 0;std::cout << "8746" << endl; \
		} \
		i2 = 0;std::cout << "8748" << endl; \
		while(true) { \
			size_t i3 = names_i;std::cout << "8750" << endl; \
			names_i = default_names.find(",", i3); \
			if(i2 == 0) { \
				i2 = default_names.find(":", i3); \
			} \
			bool case_set = false;std::cout << "8755" << endl; \
			ename.unicode = false;std::cout << "8756" << endl; \
			ename.abbreviation = false;std::cout << "8757" << endl; \
			ename.case_sensitive = false;std::cout << "8758" << endl; \
			ename.suffix = false;std::cout << "8759" << endl; \
			ename.avoid_input = false;std::cout << "8760" << endl; \
			ename.completion_only = false;std::cout << "8761" << endl; \
			ename.reference = false;std::cout << "8762" << endl; \
			ename.plural = false;std::cout << "8763" << endl; \
			if(i2 < names_i) { \
				bool b = true;std::cout << "8765" << endl; \
				for(; i3 < i2; i3++) { \
					switch(default_names[i3]) { \
						case '-': {b = false; break;} \
						case 'a': {ename.abbreviation = b; b = true; break;} \
						case 'c': {ename.case_sensitive = b; b = true; case_set = true; break;} \
						case 'i': {ename.avoid_input = b; b = true; break;} \
						case 'p': {ename.plural = b; b = true; break;} \
						case 'r': {ename.reference = b; b = true; break;} \
						case 's': {ename.suffix = b; b = true; break;} \
						case 'u': {ename.unicode = b; b = true; break;} \
						case 'o': {ename.completion_only = b; b = true; break;} \
					} \
				} \
				i3++;std::cout << "8779" << endl; \
				i2 = 0;std::cout << "8780" << endl; \
			} \
			if(ename.reference) { \
				if(names_i == string::npos) {ename.name = default_names.substr(i3, default_names.length() - i3);} \
				else {ename.name = default_names.substr(i3, names_i - i3);std::cout << "8784" << endl;} \
				remove_blank_ends(ename.name);std::cout << "8785" << endl; \
				size_t i4 = item->hasName(ename.name, ename.case_sensitive);std::cout << "8786" << endl; \
				if(i4 > 0) { \
					const ExpressionName *enameptr = &item->getName(i4);std::cout << "8788" << endl; \
					ename.suffix = enameptr->suffix;std::cout << "8789" << endl; \
					ename.abbreviation = enameptr->abbreviation;std::cout << "8790" << endl; \
					ename.avoid_input = enameptr->avoid_input;std::cout << "8791" << endl; \
					ename.completion_only = enameptr->completion_only;std::cout << "8792" << endl; \
					ename.plural = enameptr->plural;std::cout << "8793" << endl; \
					ename.case_sensitive = enameptr->case_sensitive;std::cout << "8794" << endl; \
					item->setName(ename, i4);std::cout << "8795" << endl; \
				} else if(!ename.name.empty() && validation(ename.name, version_numbers, is_user_defs)) { \
					if(!case_set) { \
						ename.case_sensitive = ename.abbreviation || text_length_is_one(ename.name);std::cout << "8798" << endl; \
					} \
					item->addName(ename);std::cout << "8800" << endl; \
				} \
			} \
			if(names_i == string::npos) {break;} \
			names_i++;std::cout << "8804" << endl; \
		} \
	}


#define ITEM_READ_NAME(validation)\
					if(!new_names && (!xmlStrcmp(child->name, (const xmlChar*) "name") || !xmlStrcmp(child->name, (const xmlChar*) "abbreviation") || !xmlStrcmp(child->name, (const xmlChar*) "plural"))) {\
						name_index = 1;std::cout << "8811" << endl;\
						XML_GET_INT_FROM_PROP(child, "index", name_index)\
						if(name_index > 0 && name_index <= 10) {\
							name_index--;std::cout << "8814" << endl;\
							names[name_index] = empty_expression_name;std::cout << "8815" << endl;\
							ref_names[name_index] = empty_expression_name;std::cout << "8816" << endl;\
							value2 = NULL;std::cout << "8817" << endl;\
							bool case_set = false;std::cout << "8818" << endl;\
							if(child->name[0] == 'a') {\
								names[name_index].abbreviation = true;std::cout << "8820" << endl;\
								ref_names[name_index].abbreviation = true;std::cout << "8821" << endl;\
							} else if(child->name[0] == 'p') {\
								names[name_index].plural = true;std::cout << "8823" << endl;\
								ref_names[name_index].plural = true;std::cout << "8824" << endl;\
							}\
							child2 = child->xmlChildrenNode;std::cout << "8826" << endl;\
							while(child2 != NULL) {\
								if((!best_name[name_index] || (ref_names[name_index].name.empty() && !locale.empty())) && !xmlStrcmp(child2->name, (const xmlChar*) "name")) {\
									lang = xmlNodeGetLang(child2);std::cout << "8829" << endl;\
									if(!lang) {\
										value2 = xmlNodeListGetString(doc, child2->xmlChildrenNode, 1);std::cout << "8831" << endl;\
										if(!value2 || validation((char*) value2, version_numbers, is_user_defs)) {\
											if(locale.empty()) {\
												best_name[name_index] = true;std::cout << "8834" << endl;\
												if(value2) names[name_index].name = (char*) value2;\
												else names[name_index].name = "";\
											} else if(!require_translation) {\
												if(!best_name[name_index] && !nextbest_name[name_index]) {\
													if(value2) names[name_index].name = (char*) value2;\
													else names[name_index].name = "";\
												}\
												if(value2) ref_names[name_index].name = (char*) value2;\
												else ref_names[name_index].name = "";\
											}\
										}\
									} else if(!best_name[name_index] && !locale.empty()) {\
										if(locale == (char*) lang) {\
											value2 = xmlNodeListGetString(doc, child2->xmlChildrenNode, 1);std::cout << "8848" << endl;\
											if(!value2 || validation((char*) value2, version_numbers, is_user_defs)) {\
												best_name[name_index] = true;std::cout << "8850" << endl;\
												if(value2) names[name_index].name = (char*) value2;\
												else names[name_index].name = "";\
											}\
										} else if(!nextbest_name[name_index] && strlen((char*) lang) >= 2 && fulfilled_translation == 0 && lang[0] == localebase[0] && lang[1] == localebase[1]) {\
											value2 = xmlNodeListGetString(doc, child2->xmlChildrenNode, 1);std::cout << "8855" << endl;\
											if(!value2 || validation((char*) value2, version_numbers, is_user_defs)) {\
												nextbest_name[name_index] = true;std::cout << "8857" << endl; \
												if(value2) names[name_index].name = (char*) value2;\
												else names[name_index].name = "";\
											}\
										}\
									}\
									if(value2) xmlFree(value2);\
									if(lang) xmlFree(lang);\
									value2 = NULL;std::cout << "8865" << endl; lang = NULL;std::cout << "8865" << endl;\
								} else if(!xmlStrcmp(child2->name, (const xmlChar*) "unicode")) {\
									XML_GET_BOOL_FROM_TEXT(child2, names[name_index].unicode)\
									ref_names[name_index].unicode = names[name_index].unicode;std::cout << "8868" << endl;\
								} else if(!xmlStrcmp(child2->name, (const xmlChar*) "reference")) {\
									XML_GET_BOOL_FROM_TEXT(child2, names[name_index].reference)\
									ref_names[name_index].reference = names[name_index].reference;std::cout << "8871" << endl;\
								} else if(!xmlStrcmp(child2->name, (const xmlChar*) "suffix")) {\
									XML_GET_BOOL_FROM_TEXT(child2, names[name_index].suffix)\
									ref_names[name_index].suffix = names[name_index].suffix;std::cout << "8874" << endl;\
								} else if(!xmlStrcmp(child2->name, (const xmlChar*) "avoid_input")) {\
									XML_GET_BOOL_FROM_TEXT(child2, names[name_index].avoid_input)\
									ref_names[name_index].avoid_input = names[name_index].avoid_input;std::cout << "8877" << endl;\
								} else if(!xmlStrcmp(child2->name, (const xmlChar*) "completion_only")) {\
									XML_GET_BOOL_FROM_TEXT(child2, names[name_index].completion_only)\
									ref_names[name_index].completion_only = names[name_index].completion_only;std::cout << "8880" << endl;\
								} else if(!xmlStrcmp(child2->name, (const xmlChar*) "plural")) {\
									XML_GET_BOOL_FROM_TEXT(child2, names[name_index].plural)\
									ref_names[name_index].plural = names[name_index].plural;std::cout << "8883" << endl;\
								} else if(!xmlStrcmp(child2->name, (const xmlChar*) "abbreviation")) {\
									XML_GET_BOOL_FROM_TEXT(child2, names[name_index].abbreviation)\
									ref_names[name_index].abbreviation = names[name_index].abbreviation;std::cout << "8886" << endl;\
								} else if(!xmlStrcmp(child2->name, (const xmlChar*) "case_sensitive")) {\
									XML_GET_BOOL_FROM_TEXT(child2, names[name_index].case_sensitive)\
									ref_names[name_index].case_sensitive = names[name_index].case_sensitive;std::cout << "8889" << endl;\
									case_set = true;std::cout << "8890" << endl;\
								}\
								child2 = child2->next;std::cout << "8892" << endl;\
							}\
							if(!case_set) {\
								ref_names[name_index].case_sensitive = ref_names[name_index].abbreviation || text_length_is_one(ref_names[name_index].name);std::cout << "8895" << endl;\
								names[name_index].case_sensitive = names[name_index].abbreviation || text_length_is_one(names[name_index].name);std::cout << "8896" << endl;\
							}\
							if(names[name_index].reference) {\
								if(!ref_names[name_index].name.empty()) {\
									if(ref_names[name_index].name == names[name_index].name) {\
										ref_names[name_index].name = "";\
									} else {\
										names[name_index].reference = false;std::cout << "8903" << endl;\
									}\
								}\
							} else if(!ref_names[name_index].name.empty()) {\
								ref_names[name_index].name = "";\
							}\
						}\
					}

#define ITEM_READ_DTH \
					if(!xmlStrcmp(child->name, (const xmlChar*) "description")) {\
						XML_GET_LOCALE_STRING_FROM_TEXT(child, description, best_description, next_best_description)\
					} else if(!xmlStrcmp(child->name, (const xmlChar*) "title")) {\
						XML_GET_LOCALE_STRING_FROM_TEXT_REQ(child, title, best_title, next_best_title)\
					} else if(!xmlStrcmp(child->name, (const xmlChar*) "hidden")) {\
						XML_GET_TRUE_FROM_TEXT(child, hidden);std::cout << "8918" << endl;\
					}

#define ITEM_READ_NAMES \
					if(new_names && ((best_names.empty() && fulfilled_translation != 2) || default_names.empty()) && !xmlStrcmp(child->name, (const xmlChar*) "names")) {\
						value = xmlNodeListGetString(doc, child->xmlChildrenNode, 1);std::cout << "8923" << endl;\
 						lang = xmlNodeGetLang(child);std::cout << "8924" << endl;\
						if(!lang) {\
							if(default_names.empty()) {\
								if(value) {\
									default_names = (char*) value;std::cout << "8928" << endl;\
									remove_blank_ends(default_names);std::cout << "8929" << endl;\
								} else {\
									default_names = "";\
								}\
							}\
						} else if(best_names.empty()) {\
							if(locale == (char*) lang) {\
								if(value) {\
									best_names = (char*) value;std::cout << "8937" << endl;\
									remove_blank_ends(best_names);std::cout << "8938" << endl;\
								} else {\
									best_names = " ";\
								}\
							} else if(nextbest_names.empty() && strlen((char*) lang) >= 2 && fulfilled_translation == 0 && lang[0] == localebase[0] && lang[1] == localebase[1]) {\
								if(value) {\
									nextbest_names = (char*) value;std::cout << "8944" << endl;\
									remove_blank_ends(nextbest_names);std::cout << "8945" << endl;\
								} else {\
									nextbest_names = " ";\
								}\
							} else if(nextbest_names.empty() && default_names.empty() && value && !require_translation) {\
								nextbest_names = (char*) value;std::cout << "8950" << endl;\
								remove_blank_ends(nextbest_names);std::cout << "8951" << endl;\
							}\
						}\
						if(value) xmlFree(value);\
						if(lang) xmlFree(lang);\
					}

#define ITEM_INIT_DTH \
					hidden = false;\
					title = ""; best_title = false; next_best_title = false;\
					description = ""; best_description = false; next_best_description = false;\
					if(fulfilled_translation > 0) require_translation = false; \
					else {XML_GET_TRUE_FROM_PROP(cur, "require_translation", require_translation)}

#define ITEM_INIT_NAME \
					if(new_names) {\
						best_names = "";\
						nextbest_names = "";\
						default_names = "";\
					} else {\
						for(size_t i = 0; i < 10; i++) {\
							best_name[i] = false;std::cout << "8972" << endl;\
							nextbest_name[i] = false;std::cout << "8973" << endl;\
						}\
					}


#define ITEM_SET_NAME_1(validation)\
					if(!name.empty() && validation(name, version_numbers, is_user_defs)) {\
						ename.name = name;std::cout << "8980" << endl;\
						ename.unicode = false;std::cout << "8981" << endl;\
						ename.abbreviation = false;std::cout << "8982" << endl;\
						ename.case_sensitive = text_length_is_one(ename.name);std::cout << "8983" << endl;\
						ename.suffix = false;std::cout << "8984" << endl;\
						ename.avoid_input = false;std::cout << "8985" << endl;\
						ename.completion_only = false;std::cout << "8986" << endl;\
						ename.reference = true;std::cout << "8987" << endl;\
						ename.plural = false;std::cout << "8988" << endl;\
						item->addName(ename);std::cout << "8989" << endl;\
					}

#define ITEM_SET_NAME_2\
					for(size_t i = 0; i < 10; i++) {\
						if(!names[i].name.empty()) {\
							item->addName(names[i], i + 1);std::cout << "8995" << endl;\
							names[i].name = "";\
						} else if(!ref_names[i].name.empty()) {\
							item->addName(ref_names[i], i + 1);std::cout << "8998" << endl;\
							ref_names[i].name = "";\
						}\
					}

#define ITEM_SET_NAME_3\
					for(size_t i = 0; i < 10; i++) {\
						if(!ref_names[i].name.empty()) {\
							item->addName(ref_names[i]);std::cout << "9006" << endl;\
							ref_names[i].name = "";\
						}\
					}

#define ITEM_SET_DTH\
					item->setDescription(description);\
					if(!title.empty() && title[0] == '!') {\
						size_t i = title.find('!', 1);std::cout << "9014" << endl;\
						if(i == string::npos) {\
							item->setTitle(title);std::cout << "9016" << endl;\
						} else if(i + 1 == title.length()) {\
							item->setTitle("");\
						} else {\
							item->setTitle(title.substr(i + 1, title.length() - (i + 1)));std::cout << "9020" << endl;\
						}\
					} else {\
						item->setTitle(title);std::cout << "9023" << endl;\
					}\
					item->setHidden(hidden);

#define ITEM_SET_SHORT_NAME\
					if(!name.empty() && unitNameIsValid(name, version_numbers, is_user_defs)) {\
						ename.name = name;std::cout << "9029" << endl;\
						ename.unicode = false;std::cout << "9030" << endl;\
						ename.abbreviation = true;std::cout << "9031" << endl;\
						ename.case_sensitive = true;std::cout << "9032" << endl;\
						ename.suffix = false;std::cout << "9033" << endl;\
						ename.avoid_input = false;std::cout << "9034" << endl;\
						ename.completion_only = false;std::cout << "9035" << endl;\
						ename.reference = true;std::cout << "9036" << endl;\
						ename.plural = false;std::cout << "9037" << endl;\
						item->addName(ename);std::cout << "9038" << endl;\
					}

#define ITEM_SET_SINGULAR\
					if(!singular.empty()) {\
						ename.name = singular;std::cout << "9043" << endl;\
						ename.unicode = false;std::cout << "9044" << endl;\
						ename.abbreviation = false;std::cout << "9045" << endl;\
						ename.case_sensitive = text_length_is_one(ename.name);std::cout << "9046" << endl;\
						ename.suffix = false;std::cout << "9047" << endl;\
						ename.avoid_input = false;std::cout << "9048" << endl;\
						ename.completion_only = false;std::cout << "9049" << endl;\
						ename.reference = false;std::cout << "9050" << endl;\
						ename.plural = false;std::cout << "9051" << endl;\
						item->addName(ename);std::cout << "9052" << endl;\
					}

#define ITEM_SET_PLURAL\
					if(!plural.empty()) {\
						ename.name = plural;std::cout << "9057" << endl;\
						ename.unicode = false;std::cout << "9058" << endl;\
						ename.abbreviation = false;std::cout << "9059" << endl;\
						ename.case_sensitive = text_length_is_one(ename.name);std::cout << "9060" << endl;\
						ename.suffix = false;std::cout << "9061" << endl;\
						ename.avoid_input = false;std::cout << "9062" << endl;\
						ename.completion_only = false;std::cout << "9063" << endl;\
						ename.reference = false;std::cout << "9064" << endl;\
						ename.plural = true;std::cout << "9065" << endl;\
						item->addName(ename);std::cout << "9066" << endl;\
					}

#define BUILTIN_NAMES_1\
				if(!is_user_defs) item->setRegistered(false);\
					bool has_ref_name;\
					for(size_t i = 1; i <= item->countNames(); i++) {\
						if(item->getName(i).reference) {\
							has_ref_name = false;std::cout << "9074" << endl;\
							for(size_t i2 = 0; i2 < 10; i2++) {\
								if(names[i2].name == item->getName(i).name || ref_names[i2].name == item->getName(i).name) {\
									has_ref_name = true;std::cout << "9077" << endl;\
									break;\
								}\
							}\
							if(!has_ref_name) {\
								for(int i2 = 9; i2 >= 0; i2--) {\
									if(ref_names[i2].name.empty()) {\
										ref_names[i2] = item->getName(i);std::cout << "9084" << endl;\
										break;\
									}\
								}\
							}\
						}\
					}\
					item->clearNames();

#define BUILTIN_UNIT_NAMES_1\
				if(!is_user_defs) item->setRegistered(false);\
					bool has_ref_name;\
					for(size_t i = 1; i <= item->countNames(); i++) {\
						if(item->getName(i).reference) {\
							has_ref_name = item->getName(i).name == singular || item->getName(i).name == plural;std::cout << "9098" << endl;\
							for(size_t i2 = 0; !has_ref_name && i2 < 10; i2++) {\
								if(names[i2].name == item->getName(i).name || ref_names[i2].name == item->getName(i).name) {\
									has_ref_name = true;std::cout << "9101" << endl;\
									break;\
								}\
							}\
							if(!has_ref_name) {\
								for(int i2 = 9; i2 >= 0; i2--) {\
									if(ref_names[i2].name.empty()) {\
										ref_names[i2] = item->getName(i);std::cout << "9108" << endl;\
										break;\
									}\
								}\
							}\
						}\
					}\
					item->clearNames();

#define BUILTIN_NAMES_2\
				if(!is_user_defs) {\
					item->setRegistered(true);std::cout << "9119" << endl;\
					nameChanged(item);std::cout << "9120" << endl;\
				}

#define ITEM_CLEAR_NAMES\
					for(size_t i = 0; i < 10; i++) {\
						if(!names[i].name.empty()) {\
							names[i].name = "";\
						}\
						if(!ref_names[i].name.empty()) {\
							ref_names[i].name = "";\
						}\
					}

int Calculator::loadDefinitions(const char* file_name, bool is_user_defs, bool check_duplicates) {

	xmlDocPtr doc;std::cout << "9135" << endl;
	xmlNodePtr cur, child, child2, child3;std::cout << "9136" << endl;
	string version, stmp, name, uname, type, svalue, sexp, plural, countries, singular, category_title, category, description, title, inverse, suncertainty, base, argname, usystem;std::cout << "9137" << endl;
	bool unc_rel;std::cout << "9138" << endl;
	bool best_title, next_best_title, best_category_title, next_best_category_title, best_description, next_best_description;std::cout << "9139" << endl;
	bool best_plural, next_best_plural, best_singular, next_best_singular, best_argname, next_best_argname, best_countries, next_best_countries;std::cout << "9140" << endl;
	bool best_proptitle, next_best_proptitle, best_propdescr, next_best_propdescr;std::cout << "9141" << endl;
	string proptitle, propdescr;std::cout << "9142" << endl;
	ExpressionName names[10];std::cout << "9143" << endl;
	ExpressionName ref_names[10];std::cout << "9144" << endl;
	string prop_names[10];std::cout << "9145" << endl;
	string ref_prop_names[10];std::cout << "9146" << endl;
	bool best_name[10];std::cout << "9147" << endl;
	bool nextbest_name[10];std::cout << "9148" << endl;
	string best_names, nextbest_names, default_names;std::cout << "9149" << endl;
	string best_prop_names, nextbest_prop_names, default_prop_names;std::cout << "9150" << endl;
	int name_index, prec;std::cout << "9151" << endl;
	ExpressionName ename;std::cout << "9152" << endl;

	string locale;std::cout << "9154" << endl;
#ifdef _WIN32
	WCHAR wlocale[LOCALE_NAME_MAX_LENGTH];std::cout << "9156" << endl;
	if(LCIDToLocaleName(LOCALE_USER_DEFAULT, wlocale, LOCALE_NAME_MAX_LENGTH, 0) != 0) locale = utf8_encode(wlocale);
	gsub("-", "_", locale);
#else
	char *clocale = setlocale(LC_MESSAGES, NULL);std::cout << "9160" << endl;
	if(clocale) locale = clocale;
#endif

	if(b_ignore_locale || locale == "POSIX" || locale == "C") {
		locale = "";
	} else {
		size_t i = locale.find('.');std::cout << "9167" << endl;
		if(i != string::npos) locale = locale.substr(0, i);
	}

	int fulfilled_translation = 0;std::cout << "9171" << endl;
	string localebase;std::cout << "9172" << endl;
	if(locale.length() > 2) {
		localebase = locale.substr(0, 2);std::cout << "9174" << endl;
		if(locale == "en_US") {
			fulfilled_translation = 2;std::cout << "9176" << endl;
		} else if(localebase == "en") {
			fulfilled_translation = 1;std::cout << "9178" << endl;
		}
	} else {
		localebase = locale;std::cout << "9181" << endl;
		if(locale == "en") {
			fulfilled_translation = 2;std::cout << "9183" << endl;
		}
	}
	while(localebase.length() < 2) {
		localebase += " ";
		fulfilled_translation = 2;std::cout << "9188" << endl;
	}

	int exponent = 1, litmp = 0, mix_priority = 0, mix_min = 0;std::cout << "9191" << endl;
	bool active = false, hidden = false, b = false, require_translation = false, use_with_prefixes = false, use_with_prefixes_set = false;std::cout << "9192" << endl;
	Number nr;std::cout << "9193" << endl;
	ExpressionItem *item;std::cout << "9194" << endl;
	MathFunction *f;std::cout << "9195" << endl;
	Variable *v;std::cout << "9196" << endl;
	Unit *u;std::cout << "9197" << endl;
	AliasUnit *au;std::cout << "9198" << endl;
	CompositeUnit *cu;std::cout << "9199" << endl;
	Prefix *p;std::cout << "9200" << endl;
	Argument *arg;std::cout << "9201" << endl;
	DataSet *dc;std::cout << "9202" << endl;
	DataProperty *dp;std::cout << "9203" << endl;
	int itmp;std::cout << "9204" << endl;
	IntegerArgument *iarg;std::cout << "9205" << endl;
	NumberArgument *farg;std::cout << "9206" << endl;
	xmlChar *value, *lang, *value2;std::cout << "9207" << endl;
	int in_unfinished = 0;std::cout << "9208" << endl;
	bool done_something = false;std::cout << "9209" << endl;
	doc = xmlParseFile(file_name);std::cout << "9210" << endl;
	if(doc == NULL) {
		return false;
	}
	cur = xmlDocGetRootElement(doc);std::cout << "9214" << endl;
	if(cur == NULL) {
		xmlFreeDoc(doc);std::cout << "9216" << endl;
		return false;
	}
	while(cur != NULL) {
		if(!xmlStrcmp(cur->name, (const xmlChar*) "QALCULATE")) {
			XML_GET_STRING_FROM_PROP(cur, "version", version)			break;
		}
		cur = cur->next;std::cout << "9223" << endl;
	}
	if(cur == NULL) {
		error(true, _("File not identified as Qalculate! definitions file: %s."), file_name, NULL);
		xmlFreeDoc(doc);std::cout << "9227" << endl;
		return false;
	}
	int version_numbers[] = {3, 6, 0};std::cout << "9230" << endl;
	parse_qalculate_version(version, version_numbers);std::cout << "9231" << endl;

	bool new_names = version_numbers[0] > 0 || version_numbers[1] > 9 || (version_numbers[1] == 9 && version_numbers[2] >= 4);std::cout << "9233" << endl;

	ParseOptions po;std::cout << "9235" << endl;

	vector<xmlNodePtr> unfinished_nodes;std::cout << "9237" << endl;
	vector<string> unfinished_cats;std::cout << "9238" << endl;
	queue<xmlNodePtr> sub_items;std::cout << "9239" << endl;
	vector<queue<xmlNodePtr> > nodes;std::cout << "9240" << endl;

	category = "";
	nodes.resize(1);std::cout << "9243" << endl;

	Unit *u_usd = getUnit("USD");

	while(true) {
		if(!in_unfinished) {
			category_title = ""; best_category_title = false; next_best_category_title = false;
			child = cur->xmlChildrenNode;std::cout << "9250" << endl;
			while(child != NULL) {
				if(!xmlStrcmp(child->name, (const xmlChar*) "title")) {
					XML_GET_LOCALE_STRING_FROM_TEXT(child, category_title, best_category_title, next_best_category_title)				} else if(!xmlStrcmp(child->name, (const xmlChar*) "category")) {
					nodes.back().push(child);std::cout << "9254" << endl;
				} else {
					sub_items.push(child);std::cout << "9256" << endl;
				}
				child = child->next;std::cout << "9258" << endl;
			}
			if(!category.empty()) {
				category += "/";
			}
			if(!category_title.empty() && category_title[0] == '!') {\
				size_t i = category_title.find('!', 1);std::cout << "9264" << endl;
				if(i == string::npos) {
					category += category_title;std::cout << "9266" << endl;
				} else if(i + 1 < category_title.length()) {
					category += category_title.substr(i + 1, category_title.length() - (i + 1));std::cout << "9268" << endl;
				}
			} else {
				category += category_title;std::cout << "9271" << endl;
			}
		}
		while(!sub_items.empty() || (in_unfinished && cur)) {
			if(!in_unfinished) {
				cur = sub_items.front();std::cout << "9276" << endl;
				sub_items.pop();std::cout << "9277" << endl;
			}
			if(!xmlStrcmp(cur->name, (const xmlChar*) "activate")) {
				XML_GET_STRING_FROM_TEXT(cur, name)				ExpressionItem *item = getInactiveExpressionItem(name);std::cout << "9280" << endl;
				if(item && !item->isLocal()) {
					item->setActive(true);std::cout << "9282" << endl;
					done_something = true;std::cout << "9283" << endl;
				}
			} else if(!xmlStrcmp(cur->name, (const xmlChar*) "deactivate")) {
				XML_GET_STRING_FROM_TEXT(cur, name)				ExpressionItem *item = getActiveExpressionItem(name);std::cout << "9286" << endl;
				if(item && !item->isLocal()) {
					item->setActive(false);std::cout << "9288" << endl;
					done_something = true;std::cout << "9289" << endl;
				}
			} else if(!xmlStrcmp(cur->name, (const xmlChar*) "function")) {
				if(VERSION_BEFORE(0, 6, 3)) {
					XML_GET_STRING_FROM_PROP(cur, "name", name)				} else {
					name = "";
				}
				XML_GET_FALSE_FROM_PROP(cur, "active", active)				f = new UserFunction(category, "", "", is_user_defs, 0, "", "", 0, active);
				item = f;std::cout << "9297" << endl;
				done_something = true;std::cout << "9298" << endl;
				child = cur->xmlChildrenNode;std::cout << "9299" << endl;
				ITEM_INIT_DTH
				ITEM_INIT_NAME
				while(child != NULL) {
					if(!xmlStrcmp(child->name, (const xmlChar*) "expression")) {
						XML_DO_FROM_TEXT(child, ((UserFunction*) f)->setFormula);std::cout << "9304" << endl;
						XML_GET_PREC_FROM_PROP(child, prec)						f->setPrecision(prec);std::cout << "9305" << endl;
						XML_GET_APPROX_FROM_PROP(child, b)						f->setApproximate(b);std::cout << "9306" << endl;
					} else if(!xmlStrcmp(child->name, (const xmlChar*) "condition")) {
						XML_DO_FROM_TEXT(child, f->setCondition);std::cout << "9308" << endl;
					} else if(!xmlStrcmp(child->name, (const xmlChar*) "subfunction")) {
						XML_GET_FALSE_FROM_PROP(child, "precalculate", b);
						value = xmlNodeListGetString(doc, child->xmlChildrenNode, 1);std::cout << "9311" << endl;
						if(value) ((UserFunction*) f)->addSubfunction((char*) value, b);
						else ((UserFunction*) f)->addSubfunction("", true);
						if(value) xmlFree(value);
					} else if(!xmlStrcmp(child->name, (const xmlChar*) "argument")) {
						farg = NULL;std::cout << "9316" << endl; iarg = NULL;std::cout << "9316" << endl;
						XML_GET_STRING_FROM_PROP(child, "type", type);
						if(type == "text") {
							arg = new TextArgument();std::cout << "9319" << endl;
						} else if(type == "symbol") {
							arg = new SymbolicArgument();std::cout << "9321" << endl;
						} else if(type == "date") {
							arg = new DateArgument();std::cout << "9323" << endl;
						} else if(type == "integer") {
							iarg = new IntegerArgument();std::cout << "9325" << endl;
							arg = iarg;std::cout << "9326" << endl;
						} else if(type == "number") {
							farg = new NumberArgument();std::cout << "9328" << endl;
							arg = farg;std::cout << "9329" << endl;
						} else if(type == "vector") {
							arg = new VectorArgument();std::cout << "9331" << endl;
						} else if(type == "matrix") {
							arg = new MatrixArgument();std::cout << "9333" << endl;
						} else if(type == "boolean") {
							arg = new BooleanArgument();std::cout << "9335" << endl;
						} else if(type == "function") {
							arg = new FunctionArgument();std::cout << "9337" << endl;
						} else if(type == "unit") {
							arg = new UnitArgument();std::cout << "9339" << endl;
						} else if(type == "variable") {
							arg = new VariableArgument();std::cout << "9341" << endl;
						} else if(type == "object") {
							arg = new ExpressionItemArgument();std::cout << "9343" << endl;
						} else if(type == "angle") {
							arg = new AngleArgument();std::cout << "9345" << endl;
						} else if(type == "data-object") {
							arg = new DataObjectArgument(NULL, "");
						} else if(type == "data-property") {
							arg = new DataPropertyArgument(NULL, "");
						} else {
							arg = new Argument();std::cout << "9351" << endl;
						}
						child2 = child->xmlChildrenNode;std::cout << "9353" << endl;
						argname = ""; best_argname = false; next_best_argname = false;
						while(child2 != NULL) {
							if(!xmlStrcmp(child2->name, (const xmlChar*) "title")) {
								XML_GET_LOCALE_STRING_FROM_TEXT(child2, argname, best_argname, next_best_argname)							} else if(!xmlStrcmp(child2->name, (const xmlChar*) "min")) {
								if(farg) {
									XML_DO_FROM_TEXT(child2, nr.set);std::cout << "9359" << endl;
									farg->setMin(&nr);std::cout << "9360" << endl;
									XML_GET_FALSE_FROM_PROP(child, "include_equals", b)									farg->setIncludeEqualsMin(b);
								} else if(iarg) {
									XML_GET_STRING_FROM_TEXT(child2, stmp);std::cout << "9363" << endl;
									Number integ(stmp);std::cout << "9364" << endl;
									iarg->setMin(&integ);std::cout << "9365" << endl;
								}
							} else if(!xmlStrcmp(child2->name, (const xmlChar*) "max")) {
								if(farg) {
									XML_DO_FROM_TEXT(child2, nr.set);std::cout << "9369" << endl;
									farg->setMax(&nr);std::cout << "9370" << endl;
									XML_GET_FALSE_FROM_PROP(child, "include_equals", b)									farg->setIncludeEqualsMax(b);
								} else if(iarg) {
									XML_GET_STRING_FROM_TEXT(child2, stmp);std::cout << "9373" << endl;
									Number integ(stmp);std::cout << "9374" << endl;
									iarg->setMax(&integ);std::cout << "9375" << endl;
								}
							} else if(farg && !xmlStrcmp(child2->name, (const xmlChar*) "complex_allowed")) {
								XML_GET_FALSE_FROM_TEXT(child2, b);std::cout << "9378" << endl;
								farg->setComplexAllowed(b);std::cout << "9379" << endl;
							} else if(!xmlStrcmp(child2->name, (const xmlChar*) "condition")) {
								XML_DO_FROM_TEXT(child2, arg->setCustomCondition);std::cout << "9381" << endl;
							} else if(!xmlStrcmp(child2->name, (const xmlChar*) "matrix_allowed")) {
								XML_GET_TRUE_FROM_TEXT(child2, b);std::cout << "9383" << endl;
								arg->setMatrixAllowed(b);std::cout << "9384" << endl;
							} else if(!xmlStrcmp(child2->name, (const xmlChar*) "zero_forbidden")) {
								XML_GET_TRUE_FROM_TEXT(child2, b);std::cout << "9386" << endl;
								arg->setZeroForbidden(b);std::cout << "9387" << endl;
							} else if(!xmlStrcmp(child2->name, (const xmlChar*) "test")) {
								XML_GET_FALSE_FROM_TEXT(child2, b);std::cout << "9389" << endl;
								arg->setTests(b);std::cout << "9390" << endl;
							} else if(!xmlStrcmp(child2->name, (const xmlChar*) "handle_vector")) {
								XML_GET_FALSE_FROM_TEXT(child2, b);std::cout << "9392" << endl;
								arg->setHandleVector(b);std::cout << "9393" << endl;
							} else if(!xmlStrcmp(child2->name, (const xmlChar*) "alert")) {
								XML_GET_FALSE_FROM_TEXT(child2, b);std::cout << "9395" << endl;
								arg->setAlerts(b);std::cout << "9396" << endl;
							}
							child2 = child2->next;std::cout << "9398" << endl;
						}
						if(!argname.empty() && argname[0] == '!') {
							size_t i = argname.find('!', 1);std::cout << "9401" << endl;
							if(i == string::npos) {
								arg->setName(argname);std::cout << "9403" << endl;
							} else if(i + 1 < argname.length()) {
								arg->setName(argname.substr(i + 1, argname.length() - (i + 1)));std::cout << "9405" << endl;
							}
						} else {
							arg->setName(argname);std::cout << "9408" << endl;
						}
						itmp = 1;std::cout << "9410" << endl;
						XML_GET_INT_FROM_PROP(child, "index", itmp);
						f->setArgumentDefinition(itmp, arg);std::cout << "9412" << endl;
					} else if(!xmlStrcmp(child->name, (const xmlChar*) "example")) {
						XML_DO_FROM_TEXT(child, f->setExample);std::cout << "9414" << endl;
					} else ITEM_READ_NAME(functionNameIsValid)					 else ITEM_READ_DTH
					 else {
						ITEM_READ_NAMES
					}
					child = child->next;std::cout << "9419" << endl;
				}
				if(new_names) {
					ITEM_SET_BEST_NAMES(functionNameIsValid)					ITEM_SET_REFERENCE_NAMES(functionNameIsValid)				} else {
					ITEM_SET_NAME_1(functionNameIsValid)					ITEM_SET_NAME_2
					ITEM_SET_NAME_3
				}
				ITEM_SET_DTH
				if(check_duplicates && !is_user_defs) {
					for(size_t i = 1; i <= f->countNames();) {
						if(getActiveFunction(f->getName(i).name)) f->removeName(i);
						else i++;std::cout << "9430" << endl;
					}
				}
				if(f->countNames() == 0) {
					f->destroy();std::cout << "9434" << endl;
					f = NULL;std::cout << "9435" << endl;
				} else {
					f->setChanged(false);std::cout << "9437" << endl;
					addFunction(f, true, is_user_defs);std::cout << "9438" << endl;
				}
			} else if(!xmlStrcmp(cur->name, (const xmlChar*) "dataset") || !xmlStrcmp(cur->name, (const xmlChar*) "builtin_dataset")) {
				bool builtin = !xmlStrcmp(cur->name, (const xmlChar*) "builtin_dataset");
				XML_GET_FALSE_FROM_PROP(cur, "active", active)				if(builtin) {
					XML_GET_STRING_FROM_PROP(cur, "name", name)					dc = getDataSet(name);
					if(!dc) {
						goto after_load_object;std::cout << "9445" << endl;
					}
					dc->setCategory(category);std::cout << "9447" << endl;
				} else {
					dc = new DataSet(category, "", "", "", "", is_user_defs);
				}
				item = dc;std::cout << "9451" << endl;
				done_something = true;std::cout << "9452" << endl;
				child = cur->xmlChildrenNode;std::cout << "9453" << endl;
				ITEM_INIT_DTH
				ITEM_INIT_NAME
				while(child != NULL) {
					if(!xmlStrcmp(child->name, (const xmlChar*) "property")) {
						dp = new DataProperty(dc);std::cout << "9458" << endl;
						child2 = child->xmlChildrenNode;std::cout << "9459" << endl;
						if(new_names) {
							default_prop_names = ""; best_prop_names = ""; nextbest_prop_names = "";
						} else {
							for(size_t i = 0; i < 10; i++) {
								best_name[i] = false;std::cout << "9464" << endl;
								nextbest_name[i] = false;std::cout << "9465" << endl;
							}
						}
						proptitle = ""; best_proptitle = false; next_best_proptitle = false;
						propdescr = ""; best_propdescr = false; next_best_propdescr = false;
						while(child2 != NULL) {
							if(!xmlStrcmp(child2->name, (const xmlChar*) "title")) {
								XML_GET_LOCALE_STRING_FROM_TEXT(child2, proptitle, best_proptitle, next_best_proptitle)							} else if(!new_names && !xmlStrcmp(child2->name, (const xmlChar*) "name")) {
								name_index = 1;std::cout << "9473" << endl;
								XML_GET_INT_FROM_PROP(child2, "index", name_index)								if(name_index > 0 && name_index <= 10) {
									name_index--;std::cout << "9475" << endl;
									prop_names[name_index] = "";
									ref_prop_names[name_index] = "";
									value2 = NULL;std::cout << "9478" << endl;
									child3 = child2->xmlChildrenNode;std::cout << "9479" << endl;
									while(child3 != NULL) {
										if((!best_name[name_index] || (ref_prop_names[name_index].empty() && !locale.empty())) && !xmlStrcmp(child3->name, (const xmlChar*) "name")) {
											lang = xmlNodeGetLang(child3);std::cout << "9482" << endl;
											if(!lang) {
												value2 = xmlNodeListGetString(doc, child3->xmlChildrenNode, 1);std::cout << "9484" << endl;
												if(locale.empty()) {
													best_name[name_index] = true;std::cout << "9486" << endl;
													if(value2) prop_names[name_index] = (char*) value2;
													else prop_names[name_index] = "";
												} else {
													if(!best_name[name_index] && !nextbest_name[name_index]) {
														if(value2) prop_names[name_index] = (char*) value2;
														else prop_names[name_index] = "";
													}
													if(value2) ref_prop_names[name_index] = (char*) value2;
													else ref_prop_names[name_index] = "";
												}
											} else if(!best_name[name_index] && !locale.empty()) {
												if(locale == (char*) lang) {
													value2 = xmlNodeListGetString(doc, child3->xmlChildrenNode, 1);std::cout << "9499" << endl;
													best_name[name_index] = true;std::cout << "9500" << endl;
													if(value2) prop_names[name_index] = (char*) value2;
													else prop_names[name_index] = "";
												} else if(!nextbest_name[name_index] && strlen((char*) lang) >= 2 && fulfilled_translation == 0 && lang[0] == localebase[0] && lang[1] == localebase[1]) {
													value2 = xmlNodeListGetString(doc, child3->xmlChildrenNode, 1);std::cout << "9504" << endl;
													nextbest_name[name_index] = true;std::cout << "9505" << endl;
													if(value2) prop_names[name_index] = (char*) value2;
													else prop_names[name_index] = "";
												}
											}
											if(value2) xmlFree(value2);
											if(lang) xmlFree(lang);
											value2 = NULL;std::cout << "9512" << endl; lang = NULL;std::cout << "9512" << endl;
										}
										child3 = child3->next;std::cout << "9514" << endl;
									}
									if(!ref_prop_names[name_index].empty() && ref_prop_names[name_index] == prop_names[name_index]) {
										ref_prop_names[name_index] = "";
									}
								}
							} else if(new_names && !xmlStrcmp(child2->name, (const xmlChar*) "names") && ((best_prop_names.empty() && fulfilled_translation != 2) || default_prop_names.empty())) {
									value2 = xmlNodeListGetString(doc, child2->xmlChildrenNode, 1);std::cout << "9521" << endl;
 									lang = xmlNodeGetLang(child2);std::cout << "9522" << endl;
									if(!lang) {
										if(default_prop_names.empty()) {
											if(value2) {
												default_prop_names = (char*) value2;std::cout << "9526" << endl;
												remove_blank_ends(default_prop_names);std::cout << "9527" << endl;
											} else {
												default_prop_names = "";
											}
										}
									} else {
										if(locale == (char*) lang) {
											if(value2) {
												best_prop_names = (char*) value2;std::cout << "9535" << endl;
												remove_blank_ends(best_prop_names);std::cout << "9536" << endl;
											} else {
												best_prop_names = " ";
											}
									} else if(nextbest_prop_names.empty() && strlen((char*) lang) >= 2 && fulfilled_translation == 0 && lang[0] == localebase[0] && lang[1] == localebase[1]) {
										if(value2) {
											nextbest_prop_names = (char*) value2;std::cout << "9542" << endl;
											remove_blank_ends(nextbest_prop_names);std::cout << "9543" << endl;
										} else {
											nextbest_prop_names = " ";
										}
									} else if(nextbest_prop_names.empty() && default_prop_names.empty() && value2 && !require_translation) {
										nextbest_prop_names = (char*) value2;std::cout << "9548" << endl;
										remove_blank_ends(nextbest_prop_names);std::cout << "9549" << endl;
									}
								}
								if(value2) xmlFree(value2);
								if(lang) xmlFree(lang);
							} else if(!xmlStrcmp(child2->name, (const xmlChar*) "description")) {
								XML_GET_LOCALE_STRING_FROM_TEXT(child2, propdescr, best_propdescr, next_best_propdescr)							} else if(!xmlStrcmp(child2->name, (const xmlChar*) "unit")) {
								XML_DO_FROM_TEXT(child2, dp->setUnit)							} else if(!xmlStrcmp(child2->name, (const xmlChar*) "key")) {
								XML_GET_TRUE_FROM_TEXT(child2, b)								dp->setKey(b);std::cout << "9557" << endl;
							} else if(!xmlStrcmp(child2->name, (const xmlChar*) "hidden")) {
								XML_GET_TRUE_FROM_TEXT(child2, b)								dp->setHidden(b);std::cout << "9559" << endl;
							} else if(!xmlStrcmp(child2->name, (const xmlChar*) "brackets")) {
								XML_GET_TRUE_FROM_TEXT(child2, b)								dp->setUsesBrackets(b);std::cout << "9561" << endl;
							} else if(!xmlStrcmp(child2->name, (const xmlChar*) "approximate")) {
								XML_GET_TRUE_FROM_TEXT(child2, b)								dp->setApproximate(b);std::cout << "9563" << endl;
							} else if(!xmlStrcmp(child2->name, (const xmlChar*) "case_sensitive")) {
								XML_GET_TRUE_FROM_TEXT(child2, b)								dp->setCaseSensitive(b);std::cout << "9565" << endl;
							} else if(!xmlStrcmp(child2->name, (const xmlChar*) "type")) {
								XML_GET_STRING_FROM_TEXT(child2, stmp)								if(stmp == "text") {
									dp->setPropertyType(PROPERTY_STRING);std::cout << "9568" << endl;
								} else if(stmp == "number") {
									dp->setPropertyType(PROPERTY_NUMBER);std::cout << "9570" << endl;
								} else if(stmp == "expression") {
									dp->setPropertyType(PROPERTY_EXPRESSION);std::cout << "9572" << endl;
								}
							}
							child2 = child2->next;std::cout << "9575" << endl;
						}
						if(!proptitle.empty() && proptitle[0] == '!') {\
							size_t i = proptitle.find('!', 1);std::cout << "9578" << endl;
							if(i == string::npos) {
								dp->setTitle(proptitle);std::cout << "9580" << endl;
							} else if(i + 1 < proptitle.length()) {
								dp->setTitle(proptitle.substr(i + 1, proptitle.length() - (i + 1)));std::cout << "9582" << endl;
							}
						} else {
							dp->setTitle(proptitle);std::cout << "9585" << endl;
						}
						dp->setDescription(propdescr);std::cout << "9587" << endl;
						if(new_names) {
							size_t names_i = 0, i2 = 0;std::cout << "9589" << endl;
							string *str_names;std::cout << "9590" << endl;
							bool had_ref = false;std::cout << "9591" << endl;
							if(best_prop_names == "-") {best_prop_names = ""; nextbest_prop_names = "";}
							if(!best_prop_names.empty()) {str_names = &best_prop_names;}
							else if(!nextbest_prop_names.empty()) {str_names = &nextbest_prop_names;}
							else {str_names = &default_prop_names;std::cout << "9595" << endl;}
							if(!str_names->empty() && (*str_names)[0] == '!') {
								names_i = str_names->find('!', 1) + 1;std::cout << "9597" << endl;
							}
							while(true) {
								size_t i3 = names_i;std::cout << "9600" << endl;
								names_i = str_names->find(",", i3);
								if(i2 == 0) {
									i2 = str_names->find(":", i3);
								}
								bool b_prop_ref = false;std::cout << "9605" << endl;
								if(i2 < names_i) {
									bool b = true;std::cout << "9607" << endl;
									for(; i3 < i2; i3++) {
										switch((*str_names)[i3]) {
											case '-': {b = false; break;}
											case 'r': {b_prop_ref = b; b = true; break;}
										}
									}
									i3++;std::cout << "9614" << endl;
									i2 = 0;std::cout << "9615" << endl;
								}
								if(names_i == string::npos) {stmp = str_names->substr(i3, str_names->length() - i3);}
								else {stmp = str_names->substr(i3, names_i - i3);std::cout << "9618" << endl;}
								remove_blank_ends(stmp);std::cout << "9619" << endl;
								if(!stmp.empty()) {
									if(b_prop_ref) had_ref = true;
									dp->addName(stmp, b_prop_ref);std::cout << "9622" << endl;
								}
								if(names_i == string::npos) {break;}
								names_i++;std::cout << "9625" << endl;
							}
							if(str_names != &default_prop_names && !default_prop_names.empty()) {
								if(default_prop_names[0] == '!') {
									names_i = default_prop_names.find('!', 1) + 1;std::cout << "9629" << endl;
								} else {
									names_i = 0;std::cout << "9631" << endl;
								}
								i2 = 0;std::cout << "9633" << endl;
								while(true) {
									size_t i3 = names_i;std::cout << "9635" << endl;
									names_i = default_prop_names.find(",", i3);
									if(i2 == 0) {
										i2 = default_prop_names.find(":", i3);
									}
									bool b_prop_ref = false;std::cout << "9640" << endl;
									if(i2 < names_i) {
										bool b = true;std::cout << "9642" << endl;
										for(; i3 < i2; i3++) {
											switch(default_prop_names[i3]) {
												case '-': {b = false; break;}
												case 'r': {b_prop_ref = b; b = true; break;}
											}
										}
										i3++;std::cout << "9649" << endl;
										i2 = 0;std::cout << "9650" << endl;
									}
									if(b_prop_ref || (!had_ref && names_i == string::npos)) {
										had_ref = true;std::cout << "9653" << endl;
										if(names_i == string::npos) {stmp = default_prop_names.substr(i3, default_prop_names.length() - i3);}
										else {stmp = default_prop_names.substr(i3, names_i - i3);std::cout << "9655" << endl;}
										remove_blank_ends(stmp);std::cout << "9656" << endl;
										size_t i4 = dp->hasName(stmp);std::cout << "9657" << endl;
										if(i4 > 0) {
											dp->setNameIsReference(i4, true);std::cout << "9659" << endl;
										} else if(!stmp.empty()) {
											dp->addName(stmp, true);std::cout << "9661" << endl;
										}
									}
									if(names_i == string::npos) {break;}
									names_i++;std::cout << "9665" << endl;
								}
							}
							if(!had_ref && dp->countNames() > 0) dp->setNameIsReference(1, true);
						} else {
							bool b = false;std::cout << "9670" << endl;
							for(size_t i = 0; i < 10; i++) {
								if(!prop_names[i].empty()) {
									if(!b && ref_prop_names[i].empty()) {
										dp->addName(prop_names[i], true, i + 1);std::cout << "9674" << endl;
										b = true;std::cout << "9675" << endl;
									} else {
										dp->addName(prop_names[i], false, i + 1);std::cout << "9677" << endl;
									}
									prop_names[i] = "";
								}
							}
							for(size_t i = 0; i < 10; i++) {
								if(!ref_prop_names[i].empty()) {
									if(!b) {
										dp->addName(ref_prop_names[i], true);std::cout << "9685" << endl;
										b = true;std::cout << "9686" << endl;
									} else {
										dp->addName(ref_prop_names[i], false);std::cout << "9688" << endl;
									}
									ref_prop_names[i] = "";
								}
							}
						}
						dp->setUserModified(is_user_defs);
						dc->addProperty(dp);std::cout << "9695" << endl;
					} else if(!xmlStrcmp(child->name, (const xmlChar*) "argument")) {
						child2 = child->xmlChildrenNode;std::cout << "9697" << endl;
						argname = ""; best_argname = false; next_best_argname = false;
						while(child2 != NULL) {
							if(!xmlStrcmp(child2->name, (const xmlChar*) "title")) {
								XML_GET_LOCALE_STRING_FROM_TEXT(child2, argname, best_argname, next_best_argname)							}
							child2 = child2->next;std::cout << "9702" << endl;
						}
						itmp = 1;std::cout << "9704" << endl;
						XML_GET_INT_FROM_PROP(child, "index", itmp);
						if(dc->getArgumentDefinition(itmp)) {
							dc->getArgumentDefinition(itmp)->setName(argname);std::cout << "9707" << endl;
						}
					} else if(!xmlStrcmp(child->name, (const xmlChar*) "object_argument")) {
						child2 = child->xmlChildrenNode;std::cout << "9710" << endl;
						argname = ""; best_argname = false; next_best_argname = false;
						while(child2 != NULL) {
							if(!xmlStrcmp(child2->name, (const xmlChar*) "title")) {
								XML_GET_LOCALE_STRING_FROM_TEXT(child2, argname, best_argname, next_best_argname)							}
							child2 = child2->next;std::cout << "9715" << endl;
						}
						itmp = 1;std::cout << "9717" << endl;
						if(dc->getArgumentDefinition(itmp)) {
							if(!argname.empty() && argname[0] == '!') {
								size_t i = argname.find('!', 1);std::cout << "9720" << endl;
								if(i == string::npos) {
									dc->getArgumentDefinition(itmp)->setName(argname);std::cout << "9722" << endl;
								} else if(i + 1 < argname.length()) {
									dc->getArgumentDefinition(itmp)->setName(argname.substr(i + 1, argname.length() - (i + 1)));std::cout << "9724" << endl;
								}
							} else {
								dc->getArgumentDefinition(itmp)->setName(argname);std::cout << "9727" << endl;
							}
						}
					} else if(!xmlStrcmp(child->name, (const xmlChar*) "property_argument")) {
						child2 = child->xmlChildrenNode;std::cout << "9731" << endl;
						argname = ""; best_argname = false; next_best_argname = false;
						while(child2 != NULL) {
							if(!xmlStrcmp(child2->name, (const xmlChar*) "title")) {
								XML_GET_LOCALE_STRING_FROM_TEXT(child2, argname, best_argname, next_best_argname)							}
							child2 = child2->next;std::cout << "9736" << endl;
						}
						itmp = 2;std::cout << "9738" << endl;
						if(dc->getArgumentDefinition(itmp)) {
							if(!argname.empty() && argname[0] == '!') {
								size_t i = argname.find('!', 1);std::cout << "9741" << endl;
								if(i == string::npos) {
									dc->getArgumentDefinition(itmp)->setName(argname);std::cout << "9743" << endl;
								} else if(i + 1 < argname.length()) {
									dc->getArgumentDefinition(itmp)->setName(argname.substr(i + 1, argname.length() - (i + 1)));std::cout << "9745" << endl;
								}
							} else {
								dc->getArgumentDefinition(itmp)->setName(argname);std::cout << "9748" << endl;
							}
						}
					} else if(!xmlStrcmp(child->name, (const xmlChar*) "default_property")) {
						XML_DO_FROM_TEXT(child, dc->setDefaultProperty)					} else if(!builtin && !xmlStrcmp(child->name, (const xmlChar*) "copyright")) {
						XML_DO_FROM_TEXT(child, dc->setCopyright)					} else if(!builtin && !xmlStrcmp(child->name, (const xmlChar*) "datafile")) {
						XML_DO_FROM_TEXT(child, dc->setDefaultDataFile)					} else if(!xmlStrcmp(child->name, (const xmlChar*) "example")) {
						XML_DO_FROM_TEXT(child, dc->setExample);std::cout << "9755" << endl;
					} else ITEM_READ_NAME(functionNameIsValid)					 else ITEM_READ_DTH
					 else {
						ITEM_READ_NAMES
					}
					child = child->next;std::cout << "9760" << endl;
				}
				if(new_names) {
					if(builtin) {
						ITEM_SAVE_BUILTIN_NAMES
					}
					ITEM_SET_BEST_NAMES(functionNameIsValid)					ITEM_SET_REFERENCE_NAMES(functionNameIsValid)					if(builtin) {
						ITEM_SET_BUILTIN_NAMES
					}
				} else {
					if(builtin) {
						BUILTIN_NAMES_1
					}
					ITEM_SET_NAME_2
					ITEM_SET_NAME_3
					if(builtin) {
						BUILTIN_NAMES_2
					}
				}
				ITEM_SET_DTH
				if(check_duplicates && !is_user_defs) {
					for(size_t i = 1; i <= dc->countNames();) {
						if(getActiveFunction(dc->getName(i).name)) dc->removeName(i);
						else i++;std::cout << "9783" << endl;
					}
				}
				if(!builtin && dc->countNames() == 0) {
					dc->destroy();std::cout << "9787" << endl;
					dc = NULL;std::cout << "9788" << endl;
				} else {
					dc->setChanged(builtin && is_user_defs);std::cout << "9790" << endl;
					if(!builtin) addDataSet(dc, true, is_user_defs);
				}
				done_something = true;std::cout << "9793" << endl;
			} else if(!xmlStrcmp(cur->name, (const xmlChar*) "builtin_function")) {
				XML_GET_STRING_FROM_PROP(cur, "name", name)				f = getFunction(name);
				if(f) {
					XML_GET_FALSE_FROM_PROP(cur, "active", active)					f->setLocal(is_user_defs, active);
					f->setCategory(category);std::cout << "9798" << endl;
					item = f;std::cout << "9799" << endl;
					child = cur->xmlChildrenNode;std::cout << "9800" << endl;
					ITEM_INIT_DTH
					ITEM_INIT_NAME
					while(child != NULL) {
						if(!xmlStrcmp(child->name, (const xmlChar*) "argument")) {
							child2 = child->xmlChildrenNode;std::cout << "9805" << endl;
							argname = ""; best_argname = false; next_best_argname = false;
							while(child2 != NULL) {
								if(!xmlStrcmp(child2->name, (const xmlChar*) "title")) {
									XML_GET_LOCALE_STRING_FROM_TEXT(child2, argname, best_argname, next_best_argname)								}
								child2 = child2->next;std::cout << "9810" << endl;
							}
							itmp = 1;std::cout << "9812" << endl;
							XML_GET_INT_FROM_PROP(child, "index", itmp);
							if(f->getArgumentDefinition(itmp)) {
								if(!argname.empty() && argname[0] == '!') {
									size_t i = argname.find('!', 1);std::cout << "9816" << endl;
									if(i == string::npos) {
										f->getArgumentDefinition(itmp)->setName(argname);std::cout << "9818" << endl;
									} else if(i + 1 < argname.length()) {
										f->getArgumentDefinition(itmp)->setName(argname.substr(i + 1, argname.length() - (i + 1)));std::cout << "9820" << endl;
									}
								} else {
									f->getArgumentDefinition(itmp)->setName(argname);std::cout << "9823" << endl;
								}
							} else if(itmp <= f->maxargs() || itmp <= f->minargs()) {
								if(!argname.empty() && argname[0] == '!') {
									size_t i = argname.find('!', 1);std::cout << "9827" << endl;
									if(i == string::npos) {
										f->setArgumentDefinition(itmp, new Argument(argname, false));std::cout << "9829" << endl;
									} else if(i + 1 < argname.length()) {
										f->setArgumentDefinition(itmp, new Argument(argname.substr(i + 1, argname.length() - (i + 1)), false));std::cout << "9831" << endl;
									}
								} else {
									f->setArgumentDefinition(itmp, new Argument(argname, false));std::cout << "9834" << endl;
								}
							}
						} else if(!xmlStrcmp(child->name, (const xmlChar*) "example")) {
							XML_DO_FROM_TEXT(child, f->setExample);std::cout << "9838" << endl;
						} else ITEM_READ_NAME(functionNameIsValid)						 else ITEM_READ_DTH
						 else {
							ITEM_READ_NAMES
						}
						child = child->next;std::cout << "9843" << endl;
					}
					if(new_names) {
						ITEM_SAVE_BUILTIN_NAMES
						ITEM_SET_BEST_NAMES(functionNameIsValid)						ITEM_SET_REFERENCE_NAMES(functionNameIsValid)						ITEM_SET_BUILTIN_NAMES
					} else {
						BUILTIN_NAMES_1
						ITEM_SET_NAME_2
						ITEM_SET_NAME_3
						BUILTIN_NAMES_2
					}
					ITEM_SET_DTH
					f->setChanged(false);std::cout << "9855" << endl;
					done_something = true;std::cout << "9856" << endl;
				}
			} else if(!xmlStrcmp(cur->name, (const xmlChar*) "unknown")) {
				if(VERSION_BEFORE(0, 6, 3)) {
					XML_GET_STRING_FROM_PROP(cur, "name", name)				} else {
					name = "";
				}
				XML_GET_FALSE_FROM_PROP(cur, "active", active)				svalue = "";
				v = new UnknownVariable(category, "", "", is_user_defs, false, active);
				item = v;std::cout << "9865" << endl;
				done_something = true;std::cout << "9866" << endl;
				child = cur->xmlChildrenNode;std::cout << "9867" << endl;
				b = true;std::cout << "9868" << endl;
				ITEM_INIT_DTH
				ITEM_INIT_NAME
				while(child != NULL) {
					if(!xmlStrcmp(child->name, (const xmlChar*) "type")) {
						XML_GET_STRING_FROM_TEXT(child, stmp);std::cout << "9873" << endl;
						if(!((UnknownVariable*) v)->assumptions()) ((UnknownVariable*) v)->setAssumptions(new Assumptions());
						if(stmp == "integer") ((UnknownVariable*) v)->assumptions()->setType(ASSUMPTION_TYPE_INTEGER);
						else if(stmp == "rational") ((UnknownVariable*) v)->assumptions()->setType(ASSUMPTION_TYPE_RATIONAL);
						else if(stmp == "real") ((UnknownVariable*) v)->assumptions()->setType(ASSUMPTION_TYPE_REAL);
						else if(stmp == "complex") ((UnknownVariable*) v)->assumptions()->setType(ASSUMPTION_TYPE_COMPLEX);
						else if(stmp == "number") ((UnknownVariable*) v)->assumptions()->setType(ASSUMPTION_TYPE_NUMBER);
						else if(stmp == "non-matrix") {
							if(VERSION_BEFORE(0, 9, 13)) {
								((UnknownVariable*) v)->assumptions()->setType(ASSUMPTION_TYPE_NUMBER);std::cout << "9882" << endl;
							} else {
								((UnknownVariable*) v)->assumptions()->setType(ASSUMPTION_TYPE_NONMATRIX);std::cout << "9884" << endl;
							}
						} else if(stmp == "none") {
							if(VERSION_BEFORE(0, 9, 13)) {
								((UnknownVariable*) v)->assumptions()->setType(ASSUMPTION_TYPE_NUMBER);std::cout << "9888" << endl;
							} else {
								((UnknownVariable*) v)->assumptions()->setType(ASSUMPTION_TYPE_NONE);std::cout << "9890" << endl;
							}
						}
					} else if(!xmlStrcmp(child->name, (const xmlChar*) "sign")) {
						XML_GET_STRING_FROM_TEXT(child, stmp);std::cout << "9894" << endl;
						if(!((UnknownVariable*) v)->assumptions()) ((UnknownVariable*) v)->setAssumptions(new Assumptions());
						if(stmp == "non-zero") ((UnknownVariable*) v)->assumptions()->setSign(ASSUMPTION_SIGN_NONZERO);
						else if(stmp == "non-positive") ((UnknownVariable*) v)->assumptions()->setSign(ASSUMPTION_SIGN_NONPOSITIVE);
						else if(stmp == "negative") ((UnknownVariable*) v)->assumptions()->setSign(ASSUMPTION_SIGN_NEGATIVE);
						else if(stmp == "non-negative") ((UnknownVariable*) v)->assumptions()->setSign(ASSUMPTION_SIGN_NONNEGATIVE);
						else if(stmp == "positive") ((UnknownVariable*) v)->assumptions()->setSign(ASSUMPTION_SIGN_POSITIVE);
						else if(stmp == "unknown") ((UnknownVariable*) v)->assumptions()->setSign(ASSUMPTION_SIGN_UNKNOWN);
					} else ITEM_READ_NAME(variableNameIsValid)					 else ITEM_READ_DTH
					 else {
						ITEM_READ_NAMES
					}
					child = child->next;std::cout << "9906" << endl;
				}
				if(new_names) {
					ITEM_SET_BEST_NAMES(variableNameIsValid)					ITEM_SET_REFERENCE_NAMES(variableNameIsValid)				} else {
					ITEM_SET_NAME_1(variableNameIsValid)					ITEM_SET_NAME_2
					ITEM_SET_NAME_3
				}
				ITEM_SET_DTH
				if(check_duplicates && !is_user_defs) {
					for(size_t i = 1; i <= v->countNames();) {
						if(getActiveVariable(v->getName(i).name) || getActiveUnit(v->getName(i).name) || getCompositeUnit(v->getName(i).name)) v->removeName(i);
						else i++;std::cout << "9917" << endl;
					}
				}
				for(size_t i = 1; i <= v->countNames(); i++) {
					if(v->getName(i).name == "x") {v_x->destroy(); v_x = (UnknownVariable*) v; break;}
					if(v->getName(i).name == "y") {v_y->destroy(); v_y = (UnknownVariable*) v; break;}
					if(v->getName(i).name == "z") {v_z->destroy(); v_z = (UnknownVariable*) v; break;}
				}
				if(v->countNames() == 0) {
					v->destroy();std::cout << "9926" << endl;
					v = NULL;std::cout << "9927" << endl;
				} else {
					addVariable(v, true, is_user_defs);std::cout << "9929" << endl;
					v->setChanged(false);std::cout << "9930" << endl;
				}
			} else if(!xmlStrcmp(cur->name, (const xmlChar*) "variable")) {
				if(VERSION_BEFORE(0, 6, 3)) {
					XML_GET_STRING_FROM_PROP(cur, "name", name)				} else {
					name = "";
				}
				XML_GET_FALSE_FROM_PROP(cur, "active", active)				svalue = "";
				v = new KnownVariable(category, "", "", "", is_user_defs, false, active);
				item = v;std::cout << "9939" << endl;
				done_something = true;std::cout << "9940" << endl;
				child = cur->xmlChildrenNode;std::cout << "9941" << endl;
				b = true;std::cout << "9942" << endl;
				ITEM_INIT_DTH
				ITEM_INIT_NAME
				while(child != NULL) {
					if(!xmlStrcmp(child->name, (const xmlChar*) "value")) {
						XML_DO_FROM_TEXT(child, ((KnownVariable*) v)->set);std::cout << "9947" << endl;
						XML_GET_STRING_FROM_PROP(child, "relative_uncertainty", suncertainty)						unc_rel = false;
						if(suncertainty.empty()) {XML_GET_STRING_FROM_PROP(child, "uncertainty", suncertainty)}
						else unc_rel = true;std::cout << "9950" << endl;
						((KnownVariable*) v)->setUncertainty(suncertainty, unc_rel);std::cout << "9951" << endl;
						XML_DO_FROM_PROP(child, "unit", ((KnownVariable*) v)->setUnit)						XML_GET_PREC_FROM_PROP(child, prec)						v->setPrecision(prec);
						XML_GET_APPROX_FROM_PROP(child, b);std::cout << "9953" << endl;
						if(b) v->setApproximate(true);
					} else ITEM_READ_NAME(variableNameIsValid)					 else ITEM_READ_DTH
					 else {
						ITEM_READ_NAMES
					}
					child = child->next;std::cout << "9959" << endl;
				}
				if(new_names) {
					ITEM_SET_BEST_NAMES(variableNameIsValid)					ITEM_SET_REFERENCE_NAMES(variableNameIsValid)				} else {
					ITEM_SET_NAME_1(variableNameIsValid)					ITEM_SET_NAME_2
					ITEM_SET_NAME_3
				}
				ITEM_SET_DTH
				if(check_duplicates && !is_user_defs) {
					for(size_t i = 1; i <= v->countNames();) {
						if(getActiveVariable(v->getName(i).name) || getActiveUnit(v->getName(i).name) || getCompositeUnit(v->getName(i).name)) v->removeName(i);
						else i++;std::cout << "9970" << endl;
					}
				}
				if(v->countNames() == 0) {
					v->destroy();std::cout << "9974" << endl;
					v = NULL;std::cout << "9975" << endl;
				} else {
					addVariable(v, true, is_user_defs);std::cout << "9977" << endl;
					item->setChanged(false);std::cout << "9978" << endl;
				}
			} else if(!xmlStrcmp(cur->name, (const xmlChar*) "builtin_variable")) {
				XML_GET_STRING_FROM_PROP(cur, "name", name)				v = getVariable(name);
				if(v) {
					XML_GET_FALSE_FROM_PROP(cur, "active", active)					v->setLocal(is_user_defs, active);
					v->setCategory(category);std::cout << "9984" << endl;
					item = v;std::cout << "9985" << endl;
					child = cur->xmlChildrenNode;std::cout << "9986" << endl;
					ITEM_INIT_DTH
					ITEM_INIT_NAME
					while(child != NULL) {
						ITEM_READ_NAME(variableNameIsValid)						 else ITEM_READ_DTH
						 else {
							ITEM_READ_NAMES
						}
						child = child->next;std::cout << "9994" << endl;
					}
					if(new_names) {
						ITEM_SAVE_BUILTIN_NAMES
						ITEM_SET_BEST_NAMES(variableNameIsValid)						ITEM_SET_REFERENCE_NAMES(variableNameIsValid)						ITEM_SET_BUILTIN_NAMES
					} else {
						BUILTIN_NAMES_1
						ITEM_SET_NAME_2
						ITEM_SET_NAME_3
						BUILTIN_NAMES_2
					}
					ITEM_SET_DTH
					v->setChanged(false);std::cout << "10006" << endl;
					done_something = true;std::cout << "10007" << endl;
				}
			} else if(!xmlStrcmp(cur->name, (const xmlChar*) "unit")) {
				XML_GET_STRING_FROM_PROP(cur, "type", type)				if(type == "base") {
					if(VERSION_BEFORE(0, 6, 3)) {
						XML_GET_STRING_FROM_PROP(cur, "name", name)					} else {
						name = "";
					}
					XML_GET_FALSE_FROM_PROP(cur, "active", active)					u = new Unit(category, "", "", "", "", is_user_defs, false, active);
					item = u;std::cout << "10016" << endl;
					child = cur->xmlChildrenNode;std::cout << "10017" << endl;
					singular = ""; best_singular = false; next_best_singular = false;
					plural = ""; best_plural = false; next_best_plural = false;
					countries = "", best_countries = false, next_best_countries = false;
					use_with_prefixes_set = false;std::cout << "10021" << endl;
					ITEM_INIT_DTH
					ITEM_INIT_NAME
					while(child != NULL) {
						if(!xmlStrcmp(child->name, (const xmlChar*) "system")) {
							XML_DO_FROM_TEXT(child, u->setSystem)						} else if(!xmlStrcmp(child->name, (const xmlChar*) "use_with_prefixes")) {
							XML_GET_TRUE_FROM_TEXT(child, use_with_prefixes)							use_with_prefixes_set = true;std::cout << "10027" << endl;
						} else if((VERSION_BEFORE(0, 6, 3)) && !xmlStrcmp(child->name, (const xmlChar*) "singular")) {
							XML_GET_LOCALE_STRING_FROM_TEXT(child, singular, best_singular, next_best_singular)							if(!unitNameIsValid(singular, version_numbers, is_user_defs)) {
								singular = "";
							}
						} else if((VERSION_BEFORE(0, 6, 3)) && !xmlStrcmp(child->name, (const xmlChar*) "plural") && !xmlGetProp(child, (xmlChar*) "index")) {
							XML_GET_LOCALE_STRING_FROM_TEXT(child, plural, best_plural, next_best_plural)							if(!unitNameIsValid(plural, version_numbers, is_user_defs)) {
								plural = "";
							}
						} else if(!xmlStrcmp(child->name, (const xmlChar*) "countries")) {
							XML_GET_LOCALE_STRING_FROM_TEXT(child, countries, best_countries, next_best_countries)						} else ITEM_READ_NAME(unitNameIsValid)						 else ITEM_READ_DTH
						 else {
							ITEM_READ_NAMES
						}
						child = child->next;std::cout << "10041" << endl;
					}
					u->setCountries(countries);std::cout << "10043" << endl;
					if(new_names) {
						ITEM_SET_BEST_NAMES(unitNameIsValid)						ITEM_SET_REFERENCE_NAMES(unitNameIsValid)					} else {
						ITEM_SET_SHORT_NAME
						ITEM_SET_SINGULAR
						ITEM_SET_PLURAL
						ITEM_SET_NAME_2
						ITEM_SET_NAME_3
					}
					ITEM_SET_DTH
					if(use_with_prefixes_set) {
						u->setUseWithPrefixesByDefault(use_with_prefixes);std::cout << "10054" << endl;
					}
					if(check_duplicates && !is_user_defs) {
						for(size_t i = 1; i <= u->countNames();) {
							if(getActiveVariable(u->getName(i).name) || getActiveUnit(u->getName(i).name) || getCompositeUnit(u->getName(i).name)) u->removeName(i);
							else i++;std::cout << "10059" << endl;
						}
					}
					if(u->countNames() == 0) {
						u->destroy();std::cout << "10063" << endl;
						u = NULL;std::cout << "10064" << endl;
					} else {
						if(!is_user_defs && u->referenceName() == "s") u_second = u;
						addUnit(u, true, is_user_defs);std::cout << "10067" << endl;
						u->setChanged(false);std::cout << "10068" << endl;
					}
					done_something = true;std::cout << "10070" << endl;
				} else if(type == "alias") {
					if(VERSION_BEFORE(0, 6, 3)) {
						XML_GET_STRING_FROM_PROP(cur, "name", name)					} else {
						name = "";
					}
					XML_GET_FALSE_FROM_PROP(cur, "active", active)					u = NULL;
					child = cur->xmlChildrenNode;std::cout << "10077" << endl;
					singular = ""; best_singular = false; next_best_singular = false;
					plural = ""; best_plural = false; next_best_plural = false;
					countries = "", best_countries = false, next_best_countries = false;
					bool b_currency = false;std::cout << "10081" << endl;
					use_with_prefixes_set = false;std::cout << "10082" << endl;
					usystem = "";
					prec = -1;std::cout << "10084" << endl;
					ITEM_INIT_DTH
					ITEM_INIT_NAME
					unc_rel = false;std::cout << "10087" << endl;
					while(child != NULL) {
						if(!xmlStrcmp(child->name, (const xmlChar*) "base")) {
							child2 = child->xmlChildrenNode;std::cout << "10090" << endl;
							exponent = 1;std::cout << "10091" << endl;
							mix_priority = 0;std::cout << "10092" << endl;
							mix_min = 0;std::cout << "10093" << endl;
							svalue = "";
							inverse = "";
							suncertainty = "";
							b = true;std::cout << "10097" << endl;
							while(child2 != NULL) {
								if(!xmlStrcmp(child2->name, (const xmlChar*) "unit")) {
									XML_GET_STRING_FROM_TEXT(child2, base);std::cout << "10100" << endl;
									u = getUnit(base);std::cout << "10101" << endl;
									b_currency = (!is_user_defs && u && u == u_euro);std::cout << "10102" << endl;
									if(!u) {
										u = getCompositeUnit(base);std::cout << "10104" << endl;
									}
								} else if(!xmlStrcmp(child2->name, (const xmlChar*) "relation")) {
									XML_GET_STRING_FROM_TEXT(child2, svalue);std::cout << "10107" << endl;
									XML_GET_APPROX_FROM_PROP(child2, b)									XML_GET_PREC_FROM_PROP(child2, prec)									XML_GET_STRING_FROM_PROP(child2, "relative_uncertainty", suncertainty)									if(suncertainty.empty()) {XML_GET_STRING_FROM_PROP(child2, "uncertainty", suncertainty)}
									else unc_rel = true;std::cout << "10109" << endl;
								} else if(!xmlStrcmp(child2->name, (const xmlChar*) "reverse_relation")) {
									XML_GET_STRING_FROM_TEXT(child2, inverse);std::cout << "10111" << endl;
								} else if(!xmlStrcmp(child2->name, (const xmlChar*) "inverse_relation")) {
									XML_GET_STRING_FROM_TEXT(child2, inverse);std::cout << "10113" << endl;
								} else if(!xmlStrcmp(child2->name, (const xmlChar*) "exponent")) {
									XML_GET_STRING_FROM_TEXT(child2, stmp);std::cout << "10115" << endl;
									if(stmp.empty()) {
										exponent = 1;std::cout << "10117" << endl;
									} else {
										exponent = s2i(stmp);std::cout << "10119" << endl;
									}
								} else if(!xmlStrcmp(child2->name, (const xmlChar*) "mix")) {
									XML_GET_INT_FROM_PROP(child2, "min", mix_min);
									XML_GET_STRING_FROM_TEXT(child2, stmp);std::cout << "10123" << endl;
									if(stmp.empty()) {
										mix_priority = 0;std::cout << "10125" << endl;
									} else {
										mix_priority = s2i(stmp);std::cout << "10127" << endl;
									}
								}
								child2 = child2->next;std::cout << "10130" << endl;
							}
						} else if(!xmlStrcmp(child->name, (const xmlChar*) "system")) {
							XML_GET_STRING_FROM_TEXT(child, usystem);std::cout << "10133" << endl;
						} else if(!xmlStrcmp(child->name, (const xmlChar*) "use_with_prefixes")) {
							XML_GET_TRUE_FROM_TEXT(child, use_with_prefixes)							use_with_prefixes_set = true;std::cout << "10135" << endl;
						} else if((VERSION_BEFORE(0, 6, 3)) && !xmlStrcmp(child->name, (const xmlChar*) "singular")) {
							XML_GET_LOCALE_STRING_FROM_TEXT(child, singular, best_singular, next_best_singular)							if(!unitNameIsValid(singular, version_numbers, is_user_defs)) {
								singular = "";
							}
						} else if((VERSION_BEFORE(0, 6, 3)) && !xmlStrcmp(child->name, (const xmlChar*) "plural") && !xmlGetProp(child, (xmlChar*) "index")) {
							XML_GET_LOCALE_STRING_FROM_TEXT(child, plural, best_plural, next_best_plural)							if(!unitNameIsValid(plural, version_numbers, is_user_defs)) {
								plural = "";
							}
						} else if(!xmlStrcmp(child->name, (const xmlChar*) "countries")) {
							XML_GET_LOCALE_STRING_FROM_TEXT(child, countries, best_countries, next_best_countries)						} else ITEM_READ_NAME(unitNameIsValid)						 else ITEM_READ_DTH
						 else {
							ITEM_READ_NAMES
						}
						child = child->next;std::cout << "10149" << endl;
					}
					if(!u) {
						ITEM_CLEAR_NAMES
						if(!in_unfinished) {
							unfinished_nodes.push_back(cur);std::cout << "10154" << endl;
							unfinished_cats.push_back(category);std::cout << "10155" << endl;
						}
					} else {
						au = new AliasUnit(category, name, plural, singular, title, u, svalue, exponent, inverse, is_user_defs, false, active);std::cout << "10158" << endl;
						au->setCountries(countries);std::cout << "10159" << endl;
						if(mix_priority > 0) {
							au->setMixWithBase(mix_priority);std::cout << "10161" << endl;
							au->setMixWithBaseMinimum(mix_min);std::cout << "10162" << endl;
						}
						au->setDescription(description);std::cout << "10164" << endl;
						au->setPrecision(prec);std::cout << "10165" << endl;
						if(b) au->setApproximate(true);
						au->setUncertainty(suncertainty, unc_rel);std::cout << "10167" << endl;
						au->setHidden(hidden);std::cout << "10168" << endl;
						au->setSystem(usystem);std::cout << "10169" << endl;
						if(use_with_prefixes_set) {
							au->setUseWithPrefixesByDefault(use_with_prefixes);std::cout << "10171" << endl;
						}
						item = au;std::cout << "10173" << endl;
						if(new_names) {
							ITEM_SET_BEST_NAMES(unitNameIsValid)							ITEM_SET_REFERENCE_NAMES(unitNameIsValid)						} else {
							ITEM_SET_NAME_2
							ITEM_SET_NAME_3
						}
						if(b_currency && !au->referenceName().empty()) {
							u = getUnit(au->referenceName());std::cout << "10180" << endl;
							if(u && u->subtype() == SUBTYPE_ALIAS_UNIT && ((AliasUnit*) u)->baseUnit() == u_euro) u->destroy();
						}
						if(check_duplicates && !is_user_defs) {
							for(size_t i = 1; i <= au->countNames();) {
								if(getActiveVariable(au->getName(i).name) || getActiveUnit(au->getName(i).name) || getCompositeUnit(au->getName(i).name)) au->removeName(i);
								else i++;std::cout << "10186" << endl;
							}
						}
						if(au->countNames() == 0) {
							au->destroy();std::cout << "10190" << endl;
							au = NULL;std::cout << "10191" << endl;
						} else {
							if(!is_user_defs && au->baseUnit() == u_second) {
								if(au->referenceName() == "d" || au->referenceName() == "day") u_day = au;
								else if(au->referenceName() == "year") u_year = au;
								else if(au->referenceName() == "month") u_month = au;
								else if(au->referenceName() == "min") u_minute = au;
								else if(au->referenceName() == "h") u_hour = au;
							}
							addUnit(au, true, is_user_defs);std::cout << "10200" << endl;
							au->setChanged(false);std::cout << "10201" << endl;
						}
						done_something = true;std::cout << "10203" << endl;
					}
				} else if(type == "composite") {
					if(VERSION_BEFORE(0, 6, 3)) {
						XML_GET_STRING_FROM_PROP(cur, "name", name)					} else {
						name = "";
					}
					XML_GET_FALSE_FROM_PROP(cur, "active", active)					child = cur->xmlChildrenNode;
					usystem = "";
					cu = NULL;std::cout << "10212" << endl;
					ITEM_INIT_DTH
					ITEM_INIT_NAME
					b = true;std::cout << "10215" << endl;
					while(child != NULL) {
						u = NULL;std::cout << "10217" << endl;
						if(!xmlStrcmp(child->name, (const xmlChar*) "part")) {
							child2 = child->xmlChildrenNode;std::cout << "10219" << endl;
							p = NULL;std::cout << "10220" << endl;
							exponent = 1;std::cout << "10221" << endl;
							while(child2 != NULL) {
								if(!xmlStrcmp(child2->name, (const xmlChar*) "unit")) {
									XML_GET_STRING_FROM_TEXT(child2, base);std::cout << "10224" << endl;
									u = getUnit(base);std::cout << "10225" << endl;
									if(!u) {
										u = getCompositeUnit(base);std::cout << "10227" << endl;
									}
								} else if(!xmlStrcmp(child2->name, (const xmlChar*) "prefix")) {
									XML_GET_STRING_FROM_PROP(child2, "type", stmp)									XML_GET_STRING_FROM_TEXT(child2, svalue);
									p = NULL;std::cout << "10231" << endl;
									if(stmp == "binary") {
										litmp = s2i(svalue);std::cout << "10233" << endl;
										if(litmp != 0) {
											p = getExactBinaryPrefix(litmp);std::cout << "10235" << endl;
											if(!p) b = false;
										}
									} else if(stmp == "number") {
										nr.set(stmp);std::cout << "10239" << endl;
										if(!nr.isZero()) {
											p = getExactPrefix(stmp);std::cout << "10241" << endl;
											if(!p) b = false;
										}
									} else {
										litmp = s2i(svalue);std::cout << "10245" << endl;
										if(litmp != 0) {
											p = getExactDecimalPrefix(litmp);std::cout << "10247" << endl;
											if(!p) b = false;
										}
									}
									if(!b) {
										if(cu) {
											delete cu;std::cout << "10253" << endl;
										}
										cu = NULL;std::cout << "10255" << endl;
										break;
									}
								} else if(!xmlStrcmp(child2->name, (const xmlChar*) "exponent")) {
									XML_GET_STRING_FROM_TEXT(child2, stmp);std::cout << "10259" << endl;
									if(stmp.empty()) {
										exponent = 1;std::cout << "10261" << endl;
									} else {
										exponent = s2i(stmp);std::cout << "10263" << endl;
									}
								}
								child2 = child2->next;std::cout << "10266" << endl;
							}
							if(!b) break;
							if(u) {
								if(!cu) {
									cu = new CompositeUnit("", "", "", "", is_user_defs, false, active);
								}
								cu->add(u, exponent, p);std::cout << "10273" << endl;
							} else {
								if(cu) delete cu;
								cu = NULL;std::cout << "10276" << endl;
								if(!in_unfinished) {
									unfinished_nodes.push_back(cur);std::cout << "10278" << endl;
									unfinished_cats.push_back(category);std::cout << "10279" << endl;
								}
								break;
							}
						} else if(!xmlStrcmp(child->name, (const xmlChar*) "system")) {
							XML_GET_STRING_FROM_TEXT(child, usystem);std::cout << "10284" << endl;
						} else if(!xmlStrcmp(child->name, (const xmlChar*) "use_with_prefixes")) {
							XML_GET_TRUE_FROM_TEXT(child, use_with_prefixes)							use_with_prefixes_set = true;std::cout << "10286" << endl;
						} else ITEM_READ_NAME(unitNameIsValid)						 else ITEM_READ_DTH
						 else {
							ITEM_READ_NAMES
						}
						child = child->next;std::cout << "10291" << endl;
					}
					if(cu) {
						item = cu;std::cout << "10294" << endl;
						cu->setCategory(category);std::cout << "10295" << endl;
						cu->setSystem(usystem);std::cout << "10296" << endl;
						/*if(use_with_prefixes_set) {
							cu->setUseWithPrefixesByDefault(use_with_prefixes);std::cout << "10298" << endl;
						}*/
						if(new_names) {
							ITEM_SET_BEST_NAMES(unitNameIsValid)							ITEM_SET_REFERENCE_NAMES(unitNameIsValid)						} else {
							ITEM_SET_NAME_1(unitNameIsValid)							ITEM_SET_NAME_2
							ITEM_SET_NAME_3
						}
						ITEM_SET_DTH
						if(check_duplicates && !is_user_defs) {
							for(size_t i = 1; i <= cu->countNames();) {
								if(getActiveVariable(cu->getName(i).name) || getActiveUnit(cu->getName(i).name) || getCompositeUnit(cu->getName(i).name)) cu->removeName(i);
								else i++;std::cout << "10309" << endl;
							}
						}
						if(cu->countNames() == 0) {
							cu->destroy();std::cout << "10313" << endl;
							cu = NULL;std::cout << "10314" << endl;
						} else {
							addUnit(cu, true, is_user_defs);std::cout << "10316" << endl;
							cu->setChanged(false);std::cout << "10317" << endl;
						}
						done_something = true;std::cout << "10319" << endl;
					} else {
						ITEM_CLEAR_NAMES
					}
				}
			} else if(!xmlStrcmp(cur->name, (const xmlChar*) "builtin_unit")) {
				XML_GET_STRING_FROM_PROP(cur, "name", name)				u = getUnit(name);
				if(!u) {
					u = getCompositeUnit(name);std::cout << "10327" << endl;
				}
				if(u) {
					XML_GET_FALSE_FROM_PROP(cur, "active", active)					u->setLocal(is_user_defs, active);
					u->setCategory(category);std::cout << "10331" << endl;
					item = u;std::cout << "10332" << endl;
					child = cur->xmlChildrenNode;std::cout << "10333" << endl;
					singular = ""; best_singular = false; next_best_singular = false;
					plural = ""; best_plural = false; next_best_plural = false;
					countries = "", best_countries = false, next_best_countries = false;
					use_with_prefixes_set = false;std::cout << "10337" << endl;
					ITEM_INIT_DTH
					ITEM_INIT_NAME
					while(child != NULL) {
						if(!xmlStrcmp(child->name, (const xmlChar*) "singular")) {
							XML_GET_LOCALE_STRING_FROM_TEXT(child, singular, best_singular, next_best_singular)							if(!unitNameIsValid(singular, version_numbers, is_user_defs)) {
								singular = "";
							}
						} else if(!xmlStrcmp(child->name, (const xmlChar*) "plural") && !xmlGetProp(child, (xmlChar*) "index")) {
							XML_GET_LOCALE_STRING_FROM_TEXT(child, plural, best_plural, next_best_plural)							if(!unitNameIsValid(plural, version_numbers, is_user_defs)) {
								plural = "";
							}
						} else if(!xmlStrcmp(child->name, (const xmlChar*) "use_with_prefixes")) {
							XML_GET_TRUE_FROM_TEXT(child, use_with_prefixes)							use_with_prefixes_set = true;std::cout << "10350" << endl;
						} else if(!xmlStrcmp(child->name, (const xmlChar*) "countries")) {
							XML_GET_LOCALE_STRING_FROM_TEXT(child, countries, best_countries, next_best_countries)						} else ITEM_READ_NAME(unitNameIsValid)						 else ITEM_READ_DTH
						 else {
							ITEM_READ_NAMES
						}
						child = child->next;std::cout << "10356" << endl;
					}
					if(use_with_prefixes_set) {
						u->setUseWithPrefixesByDefault(use_with_prefixes);std::cout << "10359" << endl;
					}
					u->setCountries(countries);std::cout << "10361" << endl;
					if(new_names) {
						ITEM_SAVE_BUILTIN_NAMES
						ITEM_SET_BEST_NAMES(unitNameIsValid)						ITEM_SET_REFERENCE_NAMES(unitNameIsValid)						ITEM_SET_BUILTIN_NAMES
					} else {
						BUILTIN_UNIT_NAMES_1
						ITEM_SET_SINGULAR
						ITEM_SET_PLURAL
						ITEM_SET_NAME_2
						ITEM_SET_NAME_3
						BUILTIN_NAMES_2
					}
					ITEM_SET_DTH
					if(u_usd && u->subtype() == SUBTYPE_ALIAS_UNIT && ((AliasUnit*) u)->firstBaseUnit() == u_usd) u->setHidden(true);
					u->setChanged(false);std::cout << "10375" << endl;
					done_something = true;std::cout << "10376" << endl;
				}
			} else if(!xmlStrcmp(cur->name, (const xmlChar*) "prefix")) {
				child = cur->xmlChildrenNode;std::cout << "10379" << endl;
				XML_GET_STRING_FROM_PROP(cur, "type", type)				uname = ""; sexp = ""; svalue = ""; name = "";
				bool b_best = false;std::cout << "10381" << endl;
				while(child != NULL) {
					if(!xmlStrcmp(child->name, (const xmlChar*) "name")) {
						lang = xmlNodeGetLang(child);std::cout << "10384" << endl;
						if(!lang) {
							if(name.empty()) {
								XML_GET_STRING_FROM_TEXT(child, name);std::cout << "10387" << endl;
							}
						} else {
							if(!b_best && !locale.empty()) {
								if(locale == (char*) lang) {
									XML_GET_STRING_FROM_TEXT(child, name);std::cout << "10392" << endl;
									b_best = true;std::cout << "10393" << endl;
								} else if(strlen((char*) lang) >= 2 && lang[0] == localebase[0] && lang[1] == localebase[1]) {
									XML_GET_STRING_FROM_TEXT(child, name);std::cout << "10395" << endl;
								}
							}
							xmlFree(lang);std::cout << "10398" << endl;
						}
					} else if(!xmlStrcmp(child->name, (const xmlChar*) "abbreviation")) {
						XML_GET_STRING_FROM_TEXT(child, stmp);std::cout << "10401" << endl;
					} else if(!xmlStrcmp(child->name, (const xmlChar*) "unicode")) {
						XML_GET_STRING_FROM_TEXT(child, uname);std::cout << "10403" << endl;
					} else if(!xmlStrcmp(child->name, (const xmlChar*) "exponent")) {
						XML_GET_STRING_FROM_TEXT(child, sexp);std::cout << "10405" << endl;
					} else if(!xmlStrcmp(child->name, (const xmlChar*) "value")) {
						XML_GET_STRING_FROM_TEXT(child, svalue);std::cout << "10407" << endl;
					}
					child = child->next;std::cout << "10409" << endl;
				}
				if(type == "decimal") {
					addPrefix(new DecimalPrefix(s2i(sexp), name, stmp, uname));std::cout << "10412" << endl;
				} else if(type == "number") {
					addPrefix(new NumberPrefix(svalue, name, stmp, uname));std::cout << "10414" << endl;
				} else if(type == "binary") {
					addPrefix(new BinaryPrefix(s2i(sexp), name, stmp, uname));std::cout << "10416" << endl;
				} else {
					if(svalue.empty()) {
						addPrefix(new DecimalPrefix(s2i(sexp), name, stmp, uname));std::cout << "10419" << endl;
					} else {
						addPrefix(new NumberPrefix(svalue, name, stmp, uname));std::cout << "10421" << endl;
					}
				}
				done_something = true;std::cout << "10424" << endl;
			}
			after_load_object:
			cur = NULL;std::cout << "10427" << endl;
			if(in_unfinished) {
				if(done_something) {
					in_unfinished--;std::cout << "10430" << endl;
					unfinished_nodes.erase(unfinished_nodes.begin() + in_unfinished);std::cout << "10431" << endl;
					unfinished_cats.erase(unfinished_cats.begin() + in_unfinished);std::cout << "10432" << endl;
				}
				if((int) unfinished_nodes.size() > in_unfinished) {
					cur = unfinished_nodes[in_unfinished];std::cout << "10435" << endl;
					category = unfinished_cats[in_unfinished];std::cout << "10436" << endl;
				} else if(done_something && unfinished_nodes.size() > 0) {
					cur = unfinished_nodes[0];std::cout << "10438" << endl;
					category = unfinished_cats[0];std::cout << "10439" << endl;
					in_unfinished = 0;std::cout << "10440" << endl;
					done_something = false;std::cout << "10441" << endl;
				}
				in_unfinished++;std::cout << "10443" << endl;
				done_something = false;std::cout << "10444" << endl;
			}
		}
		if(in_unfinished) {
			break;
		}
		while(!nodes.empty() && nodes.back().empty()) {
			size_t cat_i = category.rfind("/");
			if(cat_i == string::npos) {
				category = "";
			} else {
				category = category.substr(0, cat_i);std::cout << "10455" << endl;
			}
			nodes.pop_back();std::cout << "10457" << endl;
		}
		if(!nodes.empty()) {
			cur = nodes.back().front();std::cout << "10460" << endl;
			nodes.back().pop();std::cout << "10461" << endl;
			nodes.resize(nodes.size() + 1);std::cout << "10462" << endl;
		} else {
			if(unfinished_nodes.size() > 0) {
				cur = unfinished_nodes[0];std::cout << "10465" << endl;
				category = unfinished_cats[0];std::cout << "10466" << endl;
				in_unfinished = 1;std::cout << "10467" << endl;
				done_something = false;std::cout << "10468" << endl;
			} else {
				cur = NULL;std::cout << "10470" << endl;
			}
		}
		if(cur == NULL) {
			break;
		}
	}
	xmlFreeDoc(doc);std::cout << "10477" << endl;
	return true;
}
bool Calculator::saveDefinitions() {

	recursiveMakeDir(getLocalDataDir());std::cout << "10482" << endl;
	string homedir = buildPath(getLocalDataDir(), "definitions");
	makeDir(homedir);std::cout << "10484" << endl;
	bool b = true;std::cout << "10485" << endl;
	if(!saveFunctions(buildPath(homedir, "functions.xml").c_str())) b = false;
	if(!saveUnits(buildPath(homedir, "units.xml").c_str())) b = false;
	if(!saveVariables(buildPath(homedir, "variables.xml").c_str())) b = false;
	if(!saveDataSets(buildPath(homedir, "datasets.xml").c_str())) b = false;
	if(!saveDataObjects()) b = false;
	return b;
}

struct node_tree_item {
	xmlNodePtr node;
	string category;
	vector<node_tree_item> items;
};

int Calculator::saveDataObjects() {
	int returnvalue = 1;
	for(size_t i = 0; i < data_sets.size(); i++) {
		int rv = data_sets[i]->saveObjects(NULL, false);std::cout << "10503" << endl;
		if(rv <= 0) returnvalue = rv;
	}
	return returnvalue;
}

int Calculator::savePrefixes(const char* file_name, bool save_global) {
	if(!save_global) {
		return true;
	}
	xmlDocPtr doc = xmlNewDoc((xmlChar*) "1.0");
	xmlNodePtr cur, newnode;std::cout << "10514" << endl;
	doc->children = xmlNewDocNode(doc, NULL, (xmlChar*) "QALCULATE", NULL);
	xmlNewProp(doc->children, (xmlChar*) "version", (xmlChar*) VERSION);
	cur = doc->children;std::cout << "10517" << endl;
	for(size_t i = 0; i < prefixes.size(); i++) {
		newnode = xmlNewTextChild(cur, NULL, (xmlChar*) "prefix", NULL);
		if(!prefixes[i]->longName(false).empty()) xmlNewTextChild(newnode, NULL, (xmlChar*) "name", (xmlChar*) prefixes[i]->longName(false).c_str());
		if(!prefixes[i]->shortName(false).empty()) xmlNewTextChild(newnode, NULL, (xmlChar*) "abbreviation", (xmlChar*) prefixes[i]->shortName(false).c_str());
		if(!prefixes[i]->unicodeName(false).empty()) xmlNewTextChild(newnode, NULL, (xmlChar*) "unicode", (xmlChar*) prefixes[i]->unicodeName(false).c_str());
		switch(prefixes[i]->type()) {
			case PREFIX_DECIMAL: {
				xmlNewProp(newnode, (xmlChar*) "type", (xmlChar*) "decimal");
				xmlNewTextChild(newnode, NULL, (xmlChar*) "exponent", (xmlChar*) i2s(((DecimalPrefix*) prefixes[i])->exponent()).c_str());
				break;
			}
			case PREFIX_BINARY: {
				xmlNewProp(newnode, (xmlChar*) "type", (xmlChar*) "binary");
				xmlNewTextChild(newnode, NULL, (xmlChar*) "exponent", (xmlChar*) i2s(((BinaryPrefix*) prefixes[i])->exponent()).c_str());
				break;
			}
			case PREFIX_NUMBER: {
				xmlNewProp(newnode, (xmlChar*) "type", (xmlChar*) "number");
				xmlNewTextChild(newnode, NULL, (xmlChar*) "value", (xmlChar*) prefixes[i]->value().print(save_printoptions).c_str());
				break;
			}
		}
	}
	int returnvalue = xmlSaveFormatFile(file_name, doc, 1);
	xmlFreeDoc(doc);std::cout << "10542" << endl;
	return returnvalue;
}

#define SAVE_NAMES(o)\
				str = "";\
				for(size_t i2 = 1;;)  {\
					ename = &o->getName(i2);std::cout << "10549" << endl;\
					if(ename->abbreviation) {str += 'a';}\
					bool b_cs = (ename->abbreviation || text_length_is_one(ename->name));std::cout << "10551" << endl;\
					if(ename->case_sensitive && !b_cs) {str += 'c';}\
					if(!ename->case_sensitive && b_cs) {str += "-c";}\
					if(ename->avoid_input) {str += 'i';}\
					if(ename->completion_only) {str += 'o';}\
					if(ename->plural) {str += 'p';}\
					if(ename->reference) {str += 'r';}\
					if(ename->suffix) {str += 's';}\
					if(ename->unicode) {str += 'u';}\
					if(str.empty() || str[str.length() - 1] == ',') {\
						if(i2 == 1 && o->countNames() == 1) {\
							if(save_global) {\
								xmlNewTextChild(newnode, NULL, (xmlChar*) "_names", (xmlChar*) ename->name.c_str());\
							} else {\
								xmlNewTextChild(newnode, NULL, (xmlChar*) "names", (xmlChar*) ename->name.c_str());\
							}\
							break;\
						}\
					} else {\
						str += ':';std::cout << "10570" << endl;\
					}\
					str += ename->name;std::cout << "10572" << endl;\
					i2++;std::cout << "10573" << endl;\
					if(i2 > o->countNames()) {\
						if(save_global) {\
							xmlNewTextChild(newnode, NULL, (xmlChar*) "_names", (xmlChar*) str.c_str());\
						} else {\
							xmlNewTextChild(newnode, NULL, (xmlChar*) "names", (xmlChar*) str.c_str());\
						}\
						break;\
					}\
					str += ',';std::cout << "10582" << endl;\
				}

string Calculator::temporaryCategory() const {
	return _("Temporary");
}

int Calculator::saveVariables(const char* file_name, bool save_global) {
	string str;std::cout << "10590" << endl;
	const ExpressionName *ename;std::cout << "10591" << endl;
	xmlDocPtr doc = xmlNewDoc((xmlChar*) "1.0");
	xmlNodePtr cur, newnode, newnode2;std::cout << "10593" << endl;
	doc->children = xmlNewDocNode(doc, NULL, (xmlChar*) "QALCULATE", NULL);
	xmlNewProp(doc->children, (xmlChar*) "version", (xmlChar*) VERSION);
	node_tree_item top;std::cout << "10596" << endl;
	top.category = "";
	top.node = doc->children;std::cout << "10598" << endl;
	node_tree_item *item;std::cout << "10599" << endl;
	string cat, cat_sub;std::cout << "10600" << endl;
	for(size_t i = 0; i < variables.size(); i++) {
		if((save_global || variables[i]->isLocal() || variables[i]->hasChanged()) && variables[i]->category() != _("Temporary") && variables[i]->category() != "Temporary") {
			item = &top;std::cout << "10603" << endl;
			if(!variables[i]->category().empty()) {
				cat = variables[i]->category();std::cout << "10605" << endl;
				size_t cat_i = cat.find("/"); size_t cat_i_prev = 0;
				bool b = false;std::cout << "10607" << endl;
				while(true) {
					if(cat_i == string::npos) {
						cat_sub = cat.substr(cat_i_prev, cat.length() - cat_i_prev);std::cout << "10610" << endl;
					} else {
						cat_sub = cat.substr(cat_i_prev, cat_i - cat_i_prev);std::cout << "10612" << endl;
					}
					b = false;std::cout << "10614" << endl;
					for(size_t i2 = 0; i2 < item->items.size(); i2++) {
						if(cat_sub == item->items[i2].category) {
							item = &item->items[i2];std::cout << "10617" << endl;
							b = true;std::cout << "10618" << endl;
							break;
						}
					}
					if(!b) {
						item->items.resize(item->items.size() + 1);std::cout << "10623" << endl;
						item->items[item->items.size() - 1].node = xmlNewTextChild(item->node, NULL, (xmlChar*) "category", NULL);
						item = &item->items[item->items.size() - 1];std::cout << "10625" << endl;
						item->category = cat_sub;std::cout << "10626" << endl;
						if(save_global) {
							xmlNewTextChild(item->node, NULL, (xmlChar*) "_title", (xmlChar*) item->category.c_str());
						} else {
							xmlNewTextChild(item->node, NULL, (xmlChar*) "title", (xmlChar*) item->category.c_str());
						}
					}
					if(cat_i == string::npos) {
						break;
					}
					cat_i_prev = cat_i + 1;std::cout << "10636" << endl;
					cat_i = cat.find("/", cat_i_prev);
				}
			}
			cur = item->node;std::cout << "10640" << endl;
			if(!save_global && !variables[i]->isLocal() && variables[i]->hasChanged()) {
				if(variables[i]->isActive()) {
					xmlNewTextChild(cur, NULL, (xmlChar*) "activate", (xmlChar*) variables[i]->referenceName().c_str());
				} else {
					xmlNewTextChild(cur, NULL, (xmlChar*) "deactivate", (xmlChar*) variables[i]->referenceName().c_str());
				}
			} else if(save_global || variables[i]->isLocal()) {
				if(variables[i]->isBuiltin()) {
					if(variables[i]->isKnown()) {
						newnode = xmlNewTextChild(cur, NULL, (xmlChar*) "builtin_variable", NULL);
					} else {
						newnode = xmlNewTextChild(cur, NULL, (xmlChar*) "builtin_unknown", NULL);
					}
					xmlNewProp(newnode, (xmlChar*) "name", (xmlChar*) variables[i]->referenceName().c_str());
				} else {
					if(variables[i]->isKnown()) {
						newnode = xmlNewTextChild(cur, NULL, (xmlChar*) "variable", NULL);
					} else {
						newnode = xmlNewTextChild(cur, NULL, (xmlChar*) "unknown", NULL);
					}
				}
				if(!variables[i]->isActive()) xmlNewProp(newnode, (xmlChar*) "active", (xmlChar*) "false");
				if(variables[i]->isHidden()) xmlNewTextChild(newnode, NULL, (xmlChar*) "hidden", (xmlChar*) "true");
				if(!variables[i]->title(false).empty()) {
					if(save_global) {
						xmlNewTextChild(newnode, NULL, (xmlChar*) "_title", (xmlChar*) variables[i]->title(false).c_str());
					} else {
						xmlNewTextChild(newnode, NULL, (xmlChar*) "title", (xmlChar*) variables[i]->title(false).c_str());
					}
				}
				SAVE_NAMES(variables[i])				if(!variables[i]->description().empty()) {
					str = variables[i]->description();std::cout << "10672" << endl;
					if(save_global) {
						xmlNewTextChild(newnode, NULL, (xmlChar*) "_description", (xmlChar*) str.c_str());
					} else {
						xmlNewTextChild(newnode, NULL, (xmlChar*) "description", (xmlChar*) str.c_str());
					}
				}
				if(!variables[i]->isBuiltin()) {
					if(variables[i]->isKnown()) {
						bool is_approx = false;std::cout << "10681" << endl;
						save_printoptions.is_approximate = &is_approx;std::cout << "10682" << endl;
						if(((KnownVariable*) variables[i])->isExpression()) {
							newnode2 = xmlNewTextChild(newnode, NULL, (xmlChar*) "value", (xmlChar*) ((KnownVariable*) variables[i])->expression().c_str());
							bool unc_rel = false;std::cout << "10685" << endl;
							if(!((KnownVariable*) variables[i])->uncertainty(&unc_rel).empty()) xmlNewProp(newnode2, (xmlChar*) (unc_rel ? "relative_uncertainty" : "uncertainty"), (xmlChar*) ((KnownVariable*) variables[i])->uncertainty().c_str());
							if(!((KnownVariable*) variables[i])->unit().empty()) xmlNewProp(newnode2, (xmlChar*) "unit", (xmlChar*) ((KnownVariable*) variables[i])->unit().c_str());
						} else {
							newnode2 = xmlNewTextChild(newnode, NULL, (xmlChar*) "value", (xmlChar*) ((KnownVariable*) variables[i])->get().print(save_printoptions).c_str());
						}
						save_printoptions.is_approximate = NULL;std::cout << "10691" << endl;
						if(variables[i]->isApproximate() || is_approx) xmlNewProp(newnode2, (xmlChar*) "approximate", (xmlChar*) "true");
						if(variables[i]->precision() >= 0) xmlNewProp(newnode2, (xmlChar*) "precision", (xmlChar*) i2s(variables[i]->precision()).c_str());
					} else {
						if(((UnknownVariable*) variables[i])->assumptions()) {
							switch(((UnknownVariable*) variables[i])->assumptions()->type()) {
								case ASSUMPTION_TYPE_INTEGER: {
									newnode2 = xmlNewTextChild(newnode, NULL, (xmlChar*) "type", (xmlChar*) "integer");
									break;
								}
								case ASSUMPTION_TYPE_RATIONAL: {
									newnode2 = xmlNewTextChild(newnode, NULL, (xmlChar*) "type", (xmlChar*) "rational");
									break;
								}
								case ASSUMPTION_TYPE_REAL: {
									newnode2 = xmlNewTextChild(newnode, NULL, (xmlChar*) "type", (xmlChar*) "real");
									break;
								}
								case ASSUMPTION_TYPE_COMPLEX: {
									newnode2 = xmlNewTextChild(newnode, NULL, (xmlChar*) "type", (xmlChar*) "complex");
									break;
								}
								case ASSUMPTION_TYPE_NUMBER: {
									newnode2 = xmlNewTextChild(newnode, NULL, (xmlChar*) "type", (xmlChar*) "number");
									break;
								}
								case ASSUMPTION_TYPE_NONMATRIX: {
									newnode2 = xmlNewTextChild(newnode, NULL, (xmlChar*) "type", (xmlChar*) "non-matrix");
									break;
								}
								case ASSUMPTION_TYPE_NONE: {
									newnode2 = xmlNewTextChild(newnode, NULL, (xmlChar*) "type", (xmlChar*) "none");
									break;
								}
							}
							switch(((UnknownVariable*) variables[i])->assumptions()->sign()) {
								case ASSUMPTION_SIGN_NONZERO: {
									newnode2 = xmlNewTextChild(newnode, NULL, (xmlChar*) "sign", (xmlChar*) "non-zero");
									break;
								}
								case ASSUMPTION_SIGN_NONPOSITIVE: {
									newnode2 = xmlNewTextChild(newnode, NULL, (xmlChar*) "sign", (xmlChar*) "non-positive");
									break;
								}
								case ASSUMPTION_SIGN_NEGATIVE: {
									newnode2 = xmlNewTextChild(newnode, NULL, (xmlChar*) "sign", (xmlChar*) "negative");
									break;
								}
								case ASSUMPTION_SIGN_NONNEGATIVE: {
									newnode2 = xmlNewTextChild(newnode, NULL, (xmlChar*) "sign", (xmlChar*) "non-negative");
									break;
								}
								case ASSUMPTION_SIGN_POSITIVE: {
									newnode2 = xmlNewTextChild(newnode, NULL, (xmlChar*) "sign", (xmlChar*) "positive");
									break;
								}
								case ASSUMPTION_SIGN_UNKNOWN: {
									newnode2 = xmlNewTextChild(newnode, NULL, (xmlChar*) "sign", (xmlChar*) "unknown");
									break;
								}
							}
						}
					}
				}
			}
		}
	}
	int returnvalue = xmlSaveFormatFile(file_name, doc, 1);
	xmlFreeDoc(doc);std::cout << "10759" << endl;
	return returnvalue;
}

int Calculator::saveUnits(const char* file_name, bool save_global) {
	string str;std::cout << "10764" << endl;
	xmlDocPtr doc = xmlNewDoc((xmlChar*) "1.0");
	xmlNodePtr cur, newnode, newnode2, newnode3;std::cout << "10766" << endl;
	doc->children = xmlNewDocNode(doc, NULL, (xmlChar*) "QALCULATE", NULL);
	xmlNewProp(doc->children, (xmlChar*) "version", (xmlChar*) VERSION);
	const ExpressionName *ename;std::cout << "10769" << endl;
	CompositeUnit *cu = NULL;std::cout << "10770" << endl;
	AliasUnit *au = NULL;std::cout << "10771" << endl;
	Unit *u;std::cout << "10772" << endl;
	node_tree_item top;std::cout << "10773" << endl;
	top.category = "";
	top.node = doc->children;std::cout << "10775" << endl;
	node_tree_item *item;std::cout << "10776" << endl;
	string cat, cat_sub;std::cout << "10777" << endl;
	for(size_t i = 0; i < units.size(); i++) {
		u = units[i];std::cout << "10779" << endl;
		if(save_global || u->isLocal() || u->hasChanged()) {
			item = &top;std::cout << "10781" << endl;
			if(!u->category().empty()) {
				cat = u->category();std::cout << "10783" << endl;
				size_t cat_i = cat.find("/"); size_t cat_i_prev = 0;
				bool b = false;std::cout << "10785" << endl;
				while(true) {
					if(cat_i == string::npos) {
						cat_sub = cat.substr(cat_i_prev, cat.length() - cat_i_prev);std::cout << "10788" << endl;
					} else {
						cat_sub = cat.substr(cat_i_prev, cat_i - cat_i_prev);std::cout << "10790" << endl;
					}
					b = false;std::cout << "10792" << endl;
					for(size_t i2 = 0; i2 < item->items.size(); i2++) {
						if(cat_sub == item->items[i2].category) {
							item = &item->items[i2];std::cout << "10795" << endl;
							b = true;std::cout << "10796" << endl;
							break;
						}
					}
					if(!b) {
						item->items.resize(item->items.size() + 1);std::cout << "10801" << endl;
						item->items[item->items.size() - 1].node = xmlNewTextChild(item->node, NULL, (xmlChar*) "category", NULL);
						item = &item->items[item->items.size() - 1];std::cout << "10803" << endl;
						item->category = cat_sub;std::cout << "10804" << endl;
						if(save_global) {
							xmlNewTextChild(item->node, NULL, (xmlChar*) "_title", (xmlChar*) item->category.c_str());
						} else {
							xmlNewTextChild(item->node, NULL, (xmlChar*) "title", (xmlChar*) item->category.c_str());
						}
					}
					if(cat_i == string::npos) {
						break;
					}
					cat_i_prev = cat_i + 1;std::cout << "10814" << endl;
					cat_i = cat.find("/", cat_i_prev);
				}
			}
			cur = item->node;std::cout << "10818" << endl;
			if(!save_global && !u->isLocal() && u->hasChanged()) {
				if(u->isActive()) {
					xmlNewTextChild(cur, NULL, (xmlChar*) "activate", (xmlChar*) u->referenceName().c_str());
				} else {
					xmlNewTextChild(cur, NULL, (xmlChar*) "deactivate", (xmlChar*) u->referenceName().c_str());
				}
			} else if(save_global || u->isLocal()) {
				if(u->isBuiltin()) {
					newnode = xmlNewTextChild(cur, NULL, (xmlChar*) "builtin_unit", NULL);
					xmlNewProp(newnode, (xmlChar*) "name", (xmlChar*) u->referenceName().c_str());
				} else {
					newnode = xmlNewTextChild(cur, NULL, (xmlChar*) "unit", NULL);
					switch(u->subtype()) {
						case SUBTYPE_BASE_UNIT: {
							xmlNewProp(newnode, (xmlChar*) "type", (xmlChar*) "base");
							break;
						}
						case SUBTYPE_ALIAS_UNIT: {
							au = (AliasUnit*) u;std::cout << "10837" << endl;
							xmlNewProp(newnode, (xmlChar*) "type", (xmlChar*) "alias");
							break;
						}
						case SUBTYPE_COMPOSITE_UNIT: {
							cu = (CompositeUnit*) u;std::cout << "10842" << endl;
							xmlNewProp(newnode, (xmlChar*) "type", (xmlChar*) "composite");
							break;
						}
					}
				}
				if(!u->isActive()) xmlNewProp(newnode, (xmlChar*) "active", (xmlChar*) "false");
				if(u->isHidden()) xmlNewTextChild(newnode, NULL, (xmlChar*) "hidden", (xmlChar*) "true");
				if(!u->system().empty()) {
					xmlNewTextChild(newnode, NULL, (xmlChar*) "system", (xmlChar*) u->system().c_str());
				}
				if(!u->isSIUnit() || !u->useWithPrefixesByDefault()) {
					xmlNewTextChild(newnode, NULL, (xmlChar*) "use_with_prefixes", u->useWithPrefixesByDefault() ? (xmlChar*) "true" : (xmlChar*) "false");
				}
				if(!u->title(false).empty()) {
					if(save_global) {
						xmlNewTextChild(newnode, NULL, (xmlChar*) "_title", (xmlChar*) u->title(false).c_str());
					} else {
						xmlNewTextChild(newnode, NULL, (xmlChar*) "title", (xmlChar*) u->title(false).c_str());
					}
				}
				if(save_global && u->subtype() == SUBTYPE_COMPOSITE_UNIT) {
					save_global = false;std::cout << "10864" << endl;
					SAVE_NAMES(u)					save_global = true;std::cout << "10865" << endl;
				} else {
					SAVE_NAMES(u)				}
				if(!u->description().empty()) {
					str = u->description();std::cout << "10869" << endl;
					if(save_global) {
						xmlNewTextChild(newnode, NULL, (xmlChar*) "_description", (xmlChar*) str.c_str());
					} else {
						xmlNewTextChild(newnode, NULL, (xmlChar*) "description", (xmlChar*) str.c_str());
					}
				}
				if(!u->isBuiltin()) {
					if(u->subtype() == SUBTYPE_COMPOSITE_UNIT) {
						for(size_t i2 = 1; i2 <= cu->countUnits(); i2++) {
							newnode2 = xmlNewTextChild(newnode, NULL, (xmlChar*) "part", NULL);
							int exp = 1;std::cout << "10880" << endl;
							Prefix *p = NULL;std::cout << "10881" << endl;
							Unit *u = cu->get(i2, &exp, &p);std::cout << "10882" << endl;
							xmlNewTextChild(newnode2, NULL, (xmlChar*) "unit", (xmlChar*) u->referenceName().c_str());
							if(p) {
								switch(p->type()) {
									case PREFIX_DECIMAL: {
										xmlNewTextChild(newnode2, NULL, (xmlChar*) "prefix", (xmlChar*) i2s(((DecimalPrefix*) p)->exponent()).c_str());
										break;
									}
									case PREFIX_BINARY: {
										newnode3 = xmlNewTextChild(newnode2, NULL, (xmlChar*) "prefix", (xmlChar*) i2s(((BinaryPrefix*) p)->exponent()).c_str());
										xmlNewProp(newnode3, (xmlChar*) "type", (xmlChar*) "binary");
										break;
									}
									case PREFIX_NUMBER: {
										newnode3 = xmlNewTextChild(newnode2, NULL, (xmlChar*) "prefix", (xmlChar*) p->value().print(save_printoptions).c_str());
										xmlNewProp(newnode3, (xmlChar*) "type", (xmlChar*) "number");
										break;
									}
								}
							}
							xmlNewTextChild(newnode2, NULL, (xmlChar*) "exponent", (xmlChar*) i2s(exp).c_str());
						}
					}
					if(u->subtype() == SUBTYPE_ALIAS_UNIT) {
						newnode2 = xmlNewTextChild(newnode, NULL, (xmlChar*) "base", NULL);
						xmlNewTextChild(newnode2, NULL, (xmlChar*) "unit", (xmlChar*) au->firstBaseUnit()->referenceName().c_str());
						newnode3 = xmlNewTextChild(newnode2, NULL, (xmlChar*) "relation", (xmlChar*) au->expression().c_str());
						if(au->isApproximate()) xmlNewProp(newnode3, (xmlChar*) "approximate", (xmlChar*) "true");
						if(au->precision() >= 0) xmlNewProp(newnode3, (xmlChar*) "precision", (xmlChar*) i2s(u->precision()).c_str());
						bool unc_rel = false;std::cout << "10911" << endl;
						if(!au->uncertainty(&unc_rel).empty()) xmlNewProp(newnode3, (xmlChar*) (unc_rel ? "relative_uncertainty" : "uncertainty"), (xmlChar*) au->uncertainty().c_str());
						if(!au->inverseExpression().empty()) {
							xmlNewTextChild(newnode2, NULL, (xmlChar*) "inverse_relation", (xmlChar*) au->inverseExpression().c_str());
						}
						xmlNewTextChild(newnode2, NULL, (xmlChar*) "exponent", (xmlChar*) i2s(au->firstBaseExponent()).c_str());
						if(au->mixWithBase() > 0) {
							newnode3 = xmlNewTextChild(newnode2, NULL, (xmlChar*) "mix", (xmlChar*) i2s(au->mixWithBase()).c_str());
							if(au->mixWithBaseMinimum() > 1) xmlNewProp(newnode3, (xmlChar*) "min", (xmlChar*) i2s(au->mixWithBaseMinimum()).c_str());
						}
					}
				}
			}
		}
	}
	int returnvalue = xmlSaveFormatFile(file_name, doc, 1);
	xmlFreeDoc(doc);std::cout << "10927" << endl;
	return returnvalue;
}

int Calculator::saveFunctions(const char* file_name, bool save_global) {
	xmlDocPtr doc = xmlNewDoc((xmlChar*) "1.0");
	xmlNodePtr cur, newnode, newnode2;std::cout << "10933" << endl;
	doc->children = xmlNewDocNode(doc, NULL, (xmlChar*) "QALCULATE", NULL);
	xmlNewProp(doc->children, (xmlChar*) "version", (xmlChar*) VERSION);
	const ExpressionName *ename;std::cout << "10936" << endl;
	node_tree_item top;std::cout << "10937" << endl;
	top.category = "";
	top.node = doc->children;std::cout << "10939" << endl;
	node_tree_item *item;std::cout << "10940" << endl;
	string cat, cat_sub;std::cout << "10941" << endl;
	Argument *arg;std::cout << "10942" << endl;
	IntegerArgument *iarg;std::cout << "10943" << endl;
	NumberArgument *farg;std::cout << "10944" << endl;
	string str;std::cout << "10945" << endl;
	for(size_t i = 0; i < functions.size(); i++) {
		if(functions[i]->subtype() != SUBTYPE_DATA_SET && (save_global || functions[i]->isLocal() || functions[i]->hasChanged())) {
			item = &top;std::cout << "10948" << endl;
			if(!functions[i]->category().empty()) {
				cat = functions[i]->category();std::cout << "10950" << endl;
				size_t cat_i = cat.find("/"); size_t cat_i_prev = 0;
				bool b = false;std::cout << "10952" << endl;
				while(true) {
					if(cat_i == string::npos) {
						cat_sub = cat.substr(cat_i_prev, cat.length() - cat_i_prev);std::cout << "10955" << endl;
					} else {
						cat_sub = cat.substr(cat_i_prev, cat_i - cat_i_prev);std::cout << "10957" << endl;
					}
					b = false;std::cout << "10959" << endl;
					for(size_t i2 = 0; i2 < item->items.size(); i2++) {
						if(cat_sub == item->items[i2].category) {
							item = &item->items[i2];std::cout << "10962" << endl;
							b = true;std::cout << "10963" << endl;
							break;
						}
					}
					if(!b) {
						item->items.resize(item->items.size() + 1);std::cout << "10968" << endl;
						item->items[item->items.size() - 1].node = xmlNewTextChild(item->node, NULL, (xmlChar*) "category", NULL);
						item = &item->items[item->items.size() - 1];std::cout << "10970" << endl;
						item->category = cat_sub;std::cout << "10971" << endl;
						if(save_global) {
							xmlNewTextChild(item->node, NULL, (xmlChar*) "_title", (xmlChar*) item->category.c_str());
						} else {
							xmlNewTextChild(item->node, NULL, (xmlChar*) "title", (xmlChar*) item->category.c_str());
						}
					}
					if(cat_i == string::npos) {
						break;
					}
					cat_i_prev = cat_i + 1;std::cout << "10981" << endl;
					cat_i = cat.find("/", cat_i_prev);
				}
			}
			cur = item->node;std::cout << "10985" << endl;
			if(!save_global && !functions[i]->isLocal() && functions[i]->hasChanged()) {
				if(functions[i]->isActive()) {
					xmlNewTextChild(cur, NULL, (xmlChar*) "activate", (xmlChar*) functions[i]->referenceName().c_str());
				} else {
					xmlNewTextChild(cur, NULL, (xmlChar*) "deactivate", (xmlChar*) functions[i]->referenceName().c_str());
				}
			} else if(save_global || functions[i]->isLocal()) {
				if(functions[i]->isBuiltin()) {
					newnode = xmlNewTextChild(cur, NULL, (xmlChar*) "builtin_function", NULL);
					xmlNewProp(newnode, (xmlChar*) "name", (xmlChar*) functions[i]->referenceName().c_str());
				} else {
					newnode = xmlNewTextChild(cur, NULL, (xmlChar*) "function", NULL);
				}
				if(!functions[i]->isActive()) xmlNewProp(newnode, (xmlChar*) "active", (xmlChar*) "false");
				if(functions[i]->isHidden()) xmlNewTextChild(newnode, NULL, (xmlChar*) "hidden", (xmlChar*) "true");
				if(!functions[i]->title(false).empty()) {
					if(save_global) {
						xmlNewTextChild(newnode, NULL, (xmlChar*) "_title", (xmlChar*) functions[i]->title(false).c_str());
					} else {
						xmlNewTextChild(newnode, NULL, (xmlChar*) "title", (xmlChar*) functions[i]->title(false).c_str());
					}
				}
				SAVE_NAMES(functions[i])				if(!functions[i]->description().empty()) {
					str = functions[i]->description();std::cout << "11009" << endl;
					if(save_global) {
						xmlNewTextChild(newnode, NULL, (xmlChar*) "_description", (xmlChar*) str.c_str());
					} else {
						xmlNewTextChild(newnode, NULL, (xmlChar*) "description", (xmlChar*) str.c_str());
					}
				}
				if(!functions[i]->example(true).empty()) newnode2 = xmlNewTextChild(newnode, NULL, (xmlChar*) "example", (xmlChar*) functions[i]->example(true).c_str());
				if(functions[i]->isBuiltin()) {
					cur = newnode;std::cout << "11018" << endl;
					for(size_t i2 = 1; i2 <= functions[i]->lastArgumentDefinitionIndex(); i2++) {
						arg = functions[i]->getArgumentDefinition(i2);std::cout << "11020" << endl;
						if(arg && !arg->name().empty()) {
							newnode = xmlNewTextChild(cur, NULL, (xmlChar*) "argument", NULL);
							if(save_global) {
								xmlNewTextChild(newnode, NULL, (xmlChar*) "_title", (xmlChar*) arg->name().c_str());
							} else {
								xmlNewTextChild(newnode, NULL, (xmlChar*) "title", (xmlChar*) arg->name().c_str());
							}
							xmlNewProp(newnode, (xmlChar*) "index", (xmlChar*) i2s(i2).c_str());
						}
					}
				} else {
					for(size_t i2 = 1; i2 <= ((UserFunction*) functions[i])->countSubfunctions(); i2++) {
						newnode2 = xmlNewTextChild(newnode, NULL, (xmlChar*) "subfunction", (xmlChar*) ((UserFunction*) functions[i])->getSubfunction(i2).c_str());
						if(((UserFunction*) functions[i])->subfunctionPrecalculated(i2)) xmlNewProp(newnode2, (xmlChar*) "precalculate", (xmlChar*) "true");
						else xmlNewProp(newnode2, (xmlChar*) "precalculate", (xmlChar*) "false");

					}
					newnode2 = xmlNewTextChild(newnode, NULL, (xmlChar*) "expression", (xmlChar*) ((UserFunction*) functions[i])->formula().c_str());
					if(functions[i]->isApproximate()) xmlNewProp(newnode2, (xmlChar*) "approximate", (xmlChar*) "true");
					if(functions[i]->precision() >= 0) xmlNewProp(newnode2, (xmlChar*) "precision", (xmlChar*) i2s(functions[i]->precision()).c_str());
					if(!functions[i]->condition().empty()) {
						xmlNewTextChild(newnode, NULL, (xmlChar*) "condition", (xmlChar*) functions[i]->condition().c_str());
					}
					cur = newnode;std::cout << "11044" << endl;
					for(size_t i2 = 1; i2 <= functions[i]->lastArgumentDefinitionIndex(); i2++) {
						arg = functions[i]->getArgumentDefinition(i2);std::cout << "11046" << endl;
						if(arg) {
							newnode = xmlNewTextChild(cur, NULL, (xmlChar*) "argument", NULL);
							if(!arg->name().empty()) {
								if(save_global) {
									xmlNewTextChild(newnode, NULL, (xmlChar*) "_title", (xmlChar*) arg->name().c_str());
								} else {
									xmlNewTextChild(newnode, NULL, (xmlChar*) "title", (xmlChar*) arg->name().c_str());
								}
							}
							switch(arg->type()) {
								case ARGUMENT_TYPE_TEXT: {xmlNewProp(newnode, (xmlChar*) "type", (xmlChar*) "text"); break;}
								case ARGUMENT_TYPE_SYMBOLIC: {xmlNewProp(newnode, (xmlChar*) "type", (xmlChar*) "symbol"); break;}
								case ARGUMENT_TYPE_DATE: {xmlNewProp(newnode, (xmlChar*) "type", (xmlChar*) "date"); break;}
								case ARGUMENT_TYPE_INTEGER: {xmlNewProp(newnode, (xmlChar*) "type", (xmlChar*) "integer"); break;}
								case ARGUMENT_TYPE_NUMBER: {xmlNewProp(newnode, (xmlChar*) "type", (xmlChar*) "number"); break;}
								case ARGUMENT_TYPE_VECTOR: {xmlNewProp(newnode, (xmlChar*) "type", (xmlChar*) "vector"); break;}
								case ARGUMENT_TYPE_MATRIX: {xmlNewProp(newnode, (xmlChar*) "type", (xmlChar*) "matrix"); break;}
								case ARGUMENT_TYPE_BOOLEAN: {xmlNewProp(newnode, (xmlChar*) "type", (xmlChar*) "boolean"); break;}
								case ARGUMENT_TYPE_FUNCTION: {xmlNewProp(newnode, (xmlChar*) "type", (xmlChar*) "function"); break;}
								case ARGUMENT_TYPE_UNIT: {xmlNewProp(newnode, (xmlChar*) "type", (xmlChar*) "unit"); break;}
								case ARGUMENT_TYPE_VARIABLE: {xmlNewProp(newnode, (xmlChar*) "type", (xmlChar*) "variable"); break;}
								case ARGUMENT_TYPE_EXPRESSION_ITEM: {xmlNewProp(newnode, (xmlChar*) "type", (xmlChar*) "object"); break;}
								case ARGUMENT_TYPE_ANGLE: {xmlNewProp(newnode, (xmlChar*) "type", (xmlChar*) "angle"); break;}
								case ARGUMENT_TYPE_DATA_OBJECT: {xmlNewProp(newnode, (xmlChar*) "type", (xmlChar*) "data-object"); break;}
								case ARGUMENT_TYPE_DATA_PROPERTY: {xmlNewProp(newnode, (xmlChar*) "type", (xmlChar*) "data-property"); break;}
								default: {xmlNewProp(newnode, (xmlChar*) "type", (xmlChar*) "free");}
							}
							xmlNewProp(newnode, (xmlChar*) "index", (xmlChar*) i2s(i2).c_str());
							if(!arg->tests()) {
								xmlNewTextChild(newnode, NULL, (xmlChar*) "test", (xmlChar*) "false");
							}
							if(!arg->alerts()) {
								xmlNewTextChild(newnode, NULL, (xmlChar*) "alert", (xmlChar*) "false");
							}
							if(arg->zeroForbidden()) {
								xmlNewTextChild(newnode, NULL, (xmlChar*) "zero_forbidden", (xmlChar*) "true");
							}
							if(arg->matrixAllowed()) {
								xmlNewTextChild(newnode, NULL, (xmlChar*) "matrix_allowed", (xmlChar*) "true");
							}
							switch(arg->type()) {
								case ARGUMENT_TYPE_INTEGER: {
									iarg = (IntegerArgument*) arg;std::cout << "11089" << endl;
									if(iarg->min()) {
										xmlNewTextChild(newnode, NULL, (xmlChar*) "min", (xmlChar*) iarg->min()->print(save_printoptions).c_str());
									}
									if(iarg->max()) {
										xmlNewTextChild(newnode, NULL, (xmlChar*) "max", (xmlChar*) iarg->max()->print(save_printoptions).c_str());
									}
									break;
								}
								case ARGUMENT_TYPE_NUMBER: {
									farg = (NumberArgument*) arg;std::cout << "11099" << endl;
									if(farg->min()) {
										newnode2 = xmlNewTextChild(newnode, NULL, (xmlChar*) "min", (xmlChar*) farg->min()->print(save_printoptions).c_str());
										if(farg->includeEqualsMin()) {
											xmlNewProp(newnode2, (xmlChar*) "include_equals", (xmlChar*) "true");
										} else {
											xmlNewProp(newnode2, (xmlChar*) "include_equals", (xmlChar*) "false");
										}
									}
									if(farg->max()) {
										newnode2 = xmlNewTextChild(newnode, NULL, (xmlChar*) "max", (xmlChar*) farg->max()->print(save_printoptions).c_str());
										if(farg->includeEqualsMax()) {
											xmlNewProp(newnode2, (xmlChar*) "include_equals", (xmlChar*) "true");
										} else {
											xmlNewProp(newnode2, (xmlChar*) "include_equals", (xmlChar*) "false");
										}
									}
									if(!farg->complexAllowed()) {
										xmlNewTextChild(newnode, NULL, (xmlChar*) "complex_allowed", (xmlChar*) "false");
									}
									break;
								}
							}
							if(!arg->getCustomCondition().empty()) {
								xmlNewTextChild(newnode, NULL, (xmlChar*) "condition", (xmlChar*) arg->getCustomCondition().c_str());
							}
						}
					}
				}
			}
		}
	}
	int returnvalue = xmlSaveFormatFile(file_name, doc, 1);
	xmlFreeDoc(doc);std::cout << "11132" << endl;
	return returnvalue;
}
int Calculator::saveDataSets(const char* file_name, bool save_global) {
	xmlDocPtr doc = xmlNewDoc((xmlChar*) "1.0");
	xmlNodePtr cur, newnode, newnode2;std::cout << "11137" << endl;
	doc->children = xmlNewDocNode(doc, NULL, (xmlChar*) "QALCULATE", NULL);
	xmlNewProp(doc->children, (xmlChar*) "version", (xmlChar*) VERSION);
	const ExpressionName *ename;std::cout << "11140" << endl;
	node_tree_item top;std::cout << "11141" << endl;
	top.category = "";
	top.node = doc->children;std::cout << "11143" << endl;
	node_tree_item *item;std::cout << "11144" << endl;
	string cat, cat_sub;std::cout << "11145" << endl;
	Argument *arg;std::cout << "11146" << endl;
	DataSet *ds;std::cout << "11147" << endl;
	DataProperty *dp;std::cout << "11148" << endl;
	string str;std::cout << "11149" << endl;
	for(size_t i = 0; i < functions.size(); i++) {
		if(functions[i]->subtype() == SUBTYPE_DATA_SET && (save_global || functions[i]->isLocal() || functions[i]->hasChanged())) {
			item = &top;std::cout << "11152" << endl;
			ds = (DataSet*) functions[i];std::cout << "11153" << endl;
			if(!ds->category().empty()) {
				cat = ds->category();std::cout << "11155" << endl;
				size_t cat_i = cat.find("/"); size_t cat_i_prev = 0;
				bool b = false;std::cout << "11157" << endl;
				while(true) {
					if(cat_i == string::npos) {
						cat_sub = cat.substr(cat_i_prev, cat.length() - cat_i_prev);std::cout << "11160" << endl;
					} else {
						cat_sub = cat.substr(cat_i_prev, cat_i - cat_i_prev);std::cout << "11162" << endl;
					}
					b = false;std::cout << "11164" << endl;
					for(size_t i2 = 0; i2 < item->items.size(); i2++) {
						if(cat_sub == item->items[i2].category) {
							item = &item->items[i2];std::cout << "11167" << endl;
							b = true;std::cout << "11168" << endl;
							break;
						}
					}
					if(!b) {
						item->items.resize(item->items.size() + 1);std::cout << "11173" << endl;
						item->items[item->items.size() - 1].node = xmlNewTextChild(item->node, NULL, (xmlChar*) "category", NULL);
						item = &item->items[item->items.size() - 1];std::cout << "11175" << endl;
						item->category = cat_sub;std::cout << "11176" << endl;
						if(save_global) {
							xmlNewTextChild(item->node, NULL, (xmlChar*) "_title", (xmlChar*) item->category.c_str());
						} else {
							xmlNewTextChild(item->node, NULL, (xmlChar*) "title", (xmlChar*) item->category.c_str());
						}
					}
					if(cat_i == string::npos) {
						break;
					}
					cat_i_prev = cat_i + 1;std::cout << "11186" << endl;
					cat_i = cat.find("/", cat_i_prev);
				}
			}
			cur = item->node;std::cout << "11190" << endl;
			if(save_global || ds->isLocal() || ds->hasChanged()) {
				if(save_global || ds->isLocal()) {
					newnode = xmlNewTextChild(cur, NULL, (xmlChar*) "dataset", NULL);
				} else {
					newnode = xmlNewTextChild(cur, NULL, (xmlChar*) "builtin_dataset", NULL);
					xmlNewProp(newnode, (xmlChar*) "name", (xmlChar*) ds->referenceName().c_str());
				}
				if(!ds->isActive()) xmlNewProp(newnode, (xmlChar*) "active", (xmlChar*) "false");
				if(ds->isHidden()) xmlNewTextChild(newnode, NULL, (xmlChar*) "hidden", (xmlChar*) "true");
				if(!ds->title(false).empty()) {
					if(save_global) {
						xmlNewTextChild(newnode, NULL, (xmlChar*) "_title", (xmlChar*) ds->title(false).c_str());
					} else {
						xmlNewTextChild(newnode, NULL, (xmlChar*) "title", (xmlChar*) ds->title(false).c_str());
					}
				}
				if((save_global || ds->isLocal()) && !ds->defaultDataFile().empty()) {
					xmlNewTextChild(newnode, NULL, (xmlChar*) "datafile", (xmlChar*) ds->defaultDataFile().c_str());
				}
				SAVE_NAMES(ds)				if(!ds->description().empty()) {
					str = ds->description();std::cout << "11211" << endl;
					if(save_global) {
						xmlNewTextChild(newnode, NULL, (xmlChar*) "_description", (xmlChar*) str.c_str());
					} else {
						xmlNewTextChild(newnode, NULL, (xmlChar*) "description", (xmlChar*) str.c_str());
					}
				}
				if((save_global || ds->isLocal()) && !ds->copyright().empty()) {
					if(save_global) {
						xmlNewTextChild(newnode, NULL, (xmlChar*) "_copyright", (xmlChar*) ds->copyright().c_str());
					} else {
						xmlNewTextChild(newnode, NULL, (xmlChar*) "copyright", (xmlChar*) ds->copyright().c_str());
					}
				}
				arg = ds->getArgumentDefinition(1);std::cout << "11225" << endl;
				if(arg && ((!save_global && !ds->isLocal()) || arg->name() != _("Object"))) {
					newnode2 = xmlNewTextChild(newnode, NULL, (xmlChar*) "object_argument", NULL);
					if(save_global) {
						xmlNewTextChild(newnode2, NULL, (xmlChar*) "_title", (xmlChar*) arg->name().c_str());
					} else {
						xmlNewTextChild(newnode2, NULL, (xmlChar*) "title", (xmlChar*) arg->name().c_str());
					}
				}
				arg = ds->getArgumentDefinition(2);std::cout << "11234" << endl;
				if(arg && ((!save_global && !ds->isLocal()) || arg->name() != _("Property"))) {
					newnode2 = xmlNewTextChild(newnode, NULL, (xmlChar*) "property_argument", NULL);
					if(save_global) {
						xmlNewTextChild(newnode2, NULL, (xmlChar*) "_title", (xmlChar*) arg->name().c_str());
					} else {
						xmlNewTextChild(newnode2, NULL, (xmlChar*) "title", (xmlChar*) arg->name().c_str());
					}
				}
				if((!save_global && !ds->isLocal()) || ds->getDefaultValue(2) != _("info")) {
					xmlNewTextChild(newnode, NULL, (xmlChar*) "default_property", (xmlChar*) ds->getDefaultValue(2).c_str());
				}
				DataPropertyIter it;std::cout << "11246" << endl;
				dp = ds->getFirstProperty(&it);std::cout << "11247" << endl;
				while(dp) {
					if(save_global || ds->isLocal() || dp->isUserModified()) {
						newnode2 = xmlNewTextChild(newnode, NULL, (xmlChar*) "property", NULL);
						if(!dp->title(false).empty()) {
							if(save_global) {
								xmlNewTextChild(newnode2, NULL, (xmlChar*) "_title", (xmlChar*) dp->title().c_str());
							} else {
								xmlNewTextChild(newnode2, NULL, (xmlChar*) "title", (xmlChar*) dp->title().c_str());
							}
						}
						switch(dp->propertyType()) {
							case PROPERTY_STRING: {
								xmlNewTextChild(newnode2, NULL, (xmlChar*) "type", (xmlChar*) "text");
								break;
							}
							case PROPERTY_NUMBER: {
								xmlNewTextChild(newnode2, NULL, (xmlChar*) "type", (xmlChar*) "number");
								break;
							}
							case PROPERTY_EXPRESSION: {
								xmlNewTextChild(newnode2, NULL, (xmlChar*) "type", (xmlChar*) "expression");
								break;
							}
						}
						if(dp->isHidden()) {
							xmlNewTextChild(newnode2, NULL, (xmlChar*) "hidden", (xmlChar*) "true");
						}
						if(dp->isKey()) {
							xmlNewTextChild(newnode2, NULL, (xmlChar*) "key", (xmlChar*) "true");
						}
						if(dp->isApproximate()) {
							xmlNewTextChild(newnode2, NULL, (xmlChar*) "approximate", (xmlChar*) "true");
						}
						if(dp->usesBrackets()) {
							xmlNewTextChild(newnode2, NULL, (xmlChar*) "brackets", (xmlChar*) "true");
						}
						if(dp->isCaseSensitive()) {
							xmlNewTextChild(newnode2, NULL, (xmlChar*) "case_sensitive", (xmlChar*) "true");
						}
						if(!dp->getUnitString().empty()) {
							xmlNewTextChild(newnode2, NULL, (xmlChar*) "unit", (xmlChar*) dp->getUnitString().c_str());
						}
						str = "";
						for(size_t i2 = 1;;)  {
							if(dp->nameIsReference(i2)) {str += 'r';}
							if(str.empty() || str[str.length() - 1] == ',') {
								if(i2 == 1 && dp->countNames() == 1) {
									if(save_global) {
										xmlNewTextChild(newnode2, NULL, (xmlChar*) "_names", (xmlChar*) dp->getName(i2).c_str());
									} else {
										xmlNewTextChild(newnode2, NULL, (xmlChar*) "names", (xmlChar*) dp->getName(i2).c_str());
									}
									break;
								}
							} else {
								str += ':';std::cout << "11303" << endl;
							}
							str += dp->getName(i2);std::cout << "11305" << endl;
							i2++;std::cout << "11306" << endl;
							if(i2 > dp->countNames()) {
								if(save_global) {
									xmlNewTextChild(newnode2, NULL, (xmlChar*) "_names", (xmlChar*) str.c_str());
								} else {
									xmlNewTextChild(newnode2, NULL, (xmlChar*) "names", (xmlChar*) str.c_str());
								}
								break;
							}
							str += ',';std::cout << "11315" << endl;
						}
						if(!dp->description().empty()) {
							str = dp->description();std::cout << "11318" << endl;
							if(save_global) {
								xmlNewTextChild(newnode2, NULL, (xmlChar*) "_description", (xmlChar*) str.c_str());
							} else {
								xmlNewTextChild(newnode2, NULL, (xmlChar*) "description", (xmlChar*) str.c_str());
							}
						}
					}
					dp = ds->getNextProperty(&it);std::cout << "11326" << endl;
				}
			}
		}
	}
	int returnvalue = xmlSaveFormatFile(file_name, doc, 1);
	xmlFreeDoc(doc);std::cout << "11332" << endl;
	return returnvalue;
}

bool Calculator::importCSV(MathStructure &mstruct, const char *file_name, int first_row, string delimiter, vector<string> *headers) {
	FILE *file = fopen(file_name, "r");
	if(file == NULL) {
		return false;
	}
	if(first_row < 1) {
		first_row = 1;std::cout << "11342" << endl;
	}
	char line[10000];std::cout << "11344" << endl;
	string stmp, str1, str2;std::cout << "11345" << endl;
	long int row = 0, rows = 1;std::cout << "11346" << endl;
	int columns = 1;std::cout << "11347" << endl;
	int column;std::cout << "11348" << endl;
	mstruct = m_empty_matrix;std::cout << "11349" << endl;
	size_t is, is_n;std::cout << "11350" << endl;
	bool v_added = false;std::cout << "11351" << endl;
	while(fgets(line, 10000, file)) {
		row++;std::cout << "11353" << endl;
		if(row >= first_row) {
			stmp = line;std::cout << "11355" << endl;
			remove_blank_ends(stmp);std::cout << "11356" << endl;
			if(row == first_row) {
				if(stmp.empty()) {
					row--;std::cout << "11359" << endl;
				} else {
					is = 0;std::cout << "11361" << endl;
					while((is_n = stmp.find(delimiter, is)) != string::npos) {
						columns++;std::cout << "11363" << endl;
						if(headers) {
							str1 = stmp.substr(is, is_n - is);std::cout << "11365" << endl;
							remove_blank_ends(str1);std::cout << "11366" << endl;
							headers->push_back(str1);std::cout << "11367" << endl;
						}
						is = is_n + delimiter.length();std::cout << "11369" << endl;
					}
					if(headers) {
						str1 = stmp.substr(is, stmp.length() - is);std::cout << "11372" << endl;
						remove_blank_ends(str1);std::cout << "11373" << endl;
						headers->push_back(str1);std::cout << "11374" << endl;
					}
					mstruct.resizeMatrix(1, columns, m_undefined);std::cout << "11376" << endl;
				}
			}
			if((!headers || row > first_row) && !stmp.empty()) {
				is = 0;std::cout << "11380" << endl;
				column = 1;std::cout << "11381" << endl;
				if(v_added) {
					mstruct.addRow(m_undefined);std::cout << "11383" << endl;
					rows++;std::cout << "11384" << endl;
				}
				while(column <= columns) {
					is_n = stmp.find(delimiter, is);std::cout << "11387" << endl;
					if(is_n == string::npos) {
						str1 = stmp.substr(is, stmp.length() - is);std::cout << "11389" << endl;
					} else {
						str1 = stmp.substr(is, is_n - is);std::cout << "11391" << endl;
						is = is_n + delimiter.length();std::cout << "11392" << endl;
					}
					parse(&mstruct[rows - 1][column - 1], str1);std::cout << "11394" << endl;
					column++;std::cout << "11395" << endl;
					if(is_n == string::npos) {
						break;
					}
				}
				v_added = true;std::cout << "11400" << endl;
			}
		}
	}
	return true;
}

bool Calculator::importCSV(const char *file_name, int first_row, bool headers, string delimiter, bool to_matrix, string name, string title, string category) {
	FILE *file = fopen(file_name, "r");
	if(file == NULL) {
		return false;
	}
	if(first_row < 1) {
		first_row = 1;std::cout << "11413" << endl;
	}
	string filestr = file_name;std::cout << "11415" << endl;
	remove_blank_ends(filestr);std::cout << "11416" << endl;
	size_t i = filestr.find_last_of("/");
	if(i != string::npos) {
		filestr = filestr.substr(i + 1, filestr.length() - (i + 1));std::cout << "11419" << endl;
	}
	remove_blank_ends(name);std::cout << "11421" << endl;
	if(name.empty()) {
		name = filestr;std::cout << "11423" << endl;
		i = name.find_last_of("/");
		if(i != string::npos) name = name.substr(i + 1, name.length() - i);
		i = name.find_last_of(".");
		if(i != string::npos) name = name.substr(0, i);
	}

	char line[10000];std::cout << "11430" << endl;
	string stmp, str1, str2;std::cout << "11431" << endl;
	int row = 0;std::cout << "11432" << endl;
	int columns = 1, rows = 1;std::cout << "11433" << endl;
	int column;std::cout << "11434" << endl;
	vector<string> header;std::cout << "11435" << endl;
	vector<MathStructure> vectors;std::cout << "11436" << endl;
	MathStructure mstruct = m_empty_matrix;std::cout << "11437" << endl;
	size_t is, is_n;std::cout << "11438" << endl;
	bool v_added = false;std::cout << "11439" << endl;
	while(fgets(line, 10000, file)) {
		row++;std::cout << "11441" << endl;
		if(row >= first_row) {
			stmp = line;std::cout << "11443" << endl;
			remove_blank_ends(stmp);std::cout << "11444" << endl;
			if(row == first_row) {
				if(stmp.empty()) {
					row--;std::cout << "11447" << endl;
				} else {
					is = 0;std::cout << "11449" << endl;
					while((is_n = stmp.find(delimiter, is)) != string::npos) {
						columns++;std::cout << "11451" << endl;
						if(headers) {
							str1 = stmp.substr(is, is_n - is);std::cout << "11453" << endl;
							remove_blank_ends(str1);std::cout << "11454" << endl;
							header.push_back(str1);std::cout << "11455" << endl;
						}
						if(!to_matrix) {
							vectors.push_back(m_empty_vector);std::cout << "11458" << endl;
						}
						is = is_n + delimiter.length();std::cout << "11460" << endl;
					}
					if(headers) {
						str1 = stmp.substr(is, stmp.length() - is);std::cout << "11463" << endl;
						remove_blank_ends(str1);std::cout << "11464" << endl;
						header.push_back(str1);std::cout << "11465" << endl;
					}
					if(to_matrix) {
						mstruct.resizeMatrix(1, columns, m_undefined);std::cout << "11468" << endl;
					} else {
						vectors.push_back(m_empty_vector);std::cout << "11470" << endl;
					}
				}
			}
			if((!headers || row > first_row) && !stmp.empty()) {
				if(to_matrix && v_added) {
					mstruct.addRow(m_undefined);std::cout << "11476" << endl;
					rows++;std::cout << "11477" << endl;
				}
				is = 0;std::cout << "11479" << endl;
				column = 1;std::cout << "11480" << endl;
				while(column <= columns) {
					is_n = stmp.find(delimiter, is);std::cout << "11482" << endl;
					if(is_n == string::npos) {
						str1 = stmp.substr(is, stmp.length() - is);std::cout << "11484" << endl;
					} else {
						str1 = stmp.substr(is, is_n - is);std::cout << "11486" << endl;
						is = is_n + delimiter.length();std::cout << "11487" << endl;
					}
					if(to_matrix) {
						parse(&mstruct[rows - 1][column - 1], str1);std::cout << "11490" << endl;
					} else {
						vectors[column - 1].addChild(parse(str1));std::cout << "11492" << endl;
					}
					column++;std::cout << "11494" << endl;
					if(is_n == string::npos) {
						break;
					}
				}
				for(; column <= columns; column++) {
					if(!to_matrix) {
						vectors[column - 1].addChild(m_undefined);std::cout << "11501" << endl;
					}
				}
				v_added = true;std::cout << "11504" << endl;
			}
		}
	}
	if(to_matrix) {
		addVariable(new KnownVariable(category, name, mstruct, title));std::cout << "11509" << endl;
	} else {
		if(vectors.size() > 1) {
			if(!category.empty()) {
				category += "/";
			}
			category += name;std::cout << "11515" << endl;
		}
		for(size_t i = 0; i < vectors.size(); i++) {
			str1 = "";
			str2 = "";
			if(vectors.size() > 1) {
				str1 += name;std::cout << "11521" << endl;
				str1 += "_";
				if(title.empty()) {
					str2 += name;std::cout << "11524" << endl;
					str2 += " ";
				} else {
					str2 += title;std::cout << "11527" << endl;
					str2 += " ";
				}
				if(i < header.size()) {
					str1 += header[i];std::cout << "11531" << endl;
					str2 += header[i];std::cout << "11532" << endl;
				} else {
					str1 += _("column");
					str1 += "_";
					str1 += i2s(i + 1);std::cout << "11536" << endl;
					str2 += _("Column ");
					str2 += i2s(i + 1);std::cout << "11538" << endl;
				}
				gsub(" ", "_", str1);
			} else {
				str1 = name;std::cout << "11542" << endl;
				str2 = title;std::cout << "11543" << endl;
				if(i < header.size()) {
					str2 += " (";
					str2 += header[i];std::cout << "11546" << endl;
					str2 += ")";
				}
			}
			addVariable(new KnownVariable(category, str1, vectors[i], str2));std::cout << "11550" << endl;
		}
	}
	return true;
}
bool Calculator::exportCSV(const MathStructure &mstruct, const char *file_name, string delimiter) {
	FILE *file = fopen(file_name, "w+");
	if(file == NULL) {
		return false;
	}
	MathStructure mcsv(mstruct);std::cout << "11560" << endl;
	PrintOptions po;std::cout << "11561" << endl;
	po.number_fraction_format = FRACTION_DECIMAL;
	po.decimalpoint_sign = ".";
	po.comma_sign = ",";
	if(mcsv.isMatrix()) {
		for(size_t i = 0; i < mcsv.size(); i++) {
			for(size_t i2 = 0; i2 < mcsv[i].size(); i2++) {
				if(i2 > 0) fputs(delimiter.c_str(), file);
				mcsv[i][i2].format(po);
				fputs(mcsv[i][i2].print(po).c_str(), file);std::cout << "11570" << endl;
			}
			fputs("\n", file);
		}
	} else if(mcsv.isVector()) {
		for(size_t i = 0; i < mcsv.size(); i++) {
			mcsv[i].format(po);
			fputs(mcsv[i].print(po).c_str(), file);std::cout << "11577" << endl;
			fputs("\n", file);
		}
	} else {
		mcsv.format(po);
		fputs(mcsv.print(po).c_str(), file);std::cout << "11582" << endl;
		fputs("\n", file);
	}
	fclose(file);std::cout << "11585" << endl;
	return true;
}
int Calculator::testCondition(string expression) {
	MathStructure mstruct = calculate(expression);std::cout << "11589" << endl;
	if(mstruct.isNumber()) {
		if(mstruct.number().isPositive()) {
			return 1;
		} else {
			return 0;
		}
	}
	return -1;
}

void Calculator::startPrintControl(int milli_timeout) {
	startControl(milli_timeout);std::cout << "11601" << endl;
}
void Calculator::abortPrint() {
	abort();std::cout << "11604" << endl;
}
bool Calculator::printingAborted() {
	return aborted();
}
string Calculator::printingAbortedMessage() const {
	return abortedMessage();
}
string Calculator::timedOutString() const {
	return _("timed out");
}
bool Calculator::printingControlled() const {
	return isControlled();
}
void Calculator::stopPrintControl() {
	stopControl();std::cout << "11619" << endl;
}

void Calculator::startControl(int milli_timeout) {
	b_controlled = true;std::cout << "11623" << endl;
	i_aborted = 0;std::cout << "11624" << endl;
	i_timeout = milli_timeout;std::cout << "11625" << endl;
	if(i_timeout > 0) {
#ifndef CLOCK_MONOTONIC
		gettimeofday(&t_end, NULL);std::cout << "11628" << endl;
#else
		struct timespec ts;std::cout << "11630" << endl;
		clock_gettime(CLOCK_MONOTONIC, &ts);std::cout << "11631" << endl;
		t_end.tv_sec = ts.tv_sec;std::cout << "11632" << endl;
		t_end.tv_usec = ts.tv_nsec / 1000;std::cout << "11633" << endl;
#endif
		long int usecs = t_end.tv_usec + (long int) milli_timeout * 1000;std::cout << "11635" << endl;
		t_end.tv_usec = usecs % 1000000;std::cout << "11636" << endl;
		t_end.tv_sec += usecs / 1000000;std::cout << "11637" << endl;
	}
}
bool Calculator::aborted() {
	if(!b_controlled) return false;
	if(i_aborted > 0) return true;
	if(i_timeout > 0) {
#ifndef CLOCK_MONOTONIC
		struct timeval tv;std::cout << "11645" << endl;
		gettimeofday(&tv, NULL);std::cout << "11646" << endl;
		if(tv.tv_sec > t_end.tv_sec || (tv.tv_sec == t_end.tv_sec && tv.tv_usec > t_end.tv_usec)) {
#else
		struct timespec tv;std::cout << "11649" << endl;
		clock_gettime(CLOCK_MONOTONIC, &tv);std::cout << "11650" << endl;
		if(tv.tv_sec > t_end.tv_sec || (tv.tv_sec == t_end.tv_sec && tv.tv_nsec / 1000 > t_end.tv_usec)) {
#endif
			i_aborted = 2;std::cout << "11653" << endl;
			return true;
		}
	}
	return false;
}
string Calculator::abortedMessage() const {
	if(i_aborted == 2) return _("timed out");
	return _("aborted");
}
bool Calculator::isControlled() const {
	return b_controlled;
}
void Calculator::stopControl() {
	b_controlled = false;std::cout << "11667" << endl;
	i_aborted = 0;std::cout << "11668" << endl;
	i_timeout = 0;std::cout << "11669" << endl;
}


bool Calculator::loadExchangeRates() {
	xmlDocPtr doc = NULL;std::cout << "11674" << endl;
	xmlNodePtr cur = NULL;std::cout << "11675" << endl;
	xmlChar *value;std::cout << "11676" << endl;
	bool global_file = false;std::cout << "11677" << endl;
	string currency, rate, sdate;std::cout << "11678" << endl;
	string filename = buildPath(getLocalDataDir(), "eurofxref-daily.xml");
	if(fileExists(filename)) {
		doc = xmlParseFile(filename.c_str());std::cout << "11681" << endl;
	} else {
#ifndef _WIN32
		string filename_old = buildPath(getOldLocalDir(), "eurofxref-daily.xml");
		if(fileExists(filename)) {
			doc = xmlParseFile(filename_old.c_str());std::cout << "11686" << endl;
			if(doc) {
				recursiveMakeDir(getLocalDataDir());std::cout << "11688" << endl;
				move_file(filename_old.c_str(), filename.c_str());std::cout << "11689" << endl;
				removeDir(getOldLocalDir());std::cout << "11690" << endl;
			}
		}
#endif
	}
	if(doc) cur = xmlDocGetRootElement(doc);
	if(!cur) {
		if(doc) xmlFreeDoc(doc);
		filename = buildPath(getGlobalDefinitionsDir(), "eurofxref-daily.xml");
		doc = xmlParseFile(filename.c_str());std::cout << "11699" << endl;
		if(!doc) return false;
		cur = xmlDocGetRootElement(doc);std::cout << "11701" << endl;
		if(!cur) return false;
		global_file = true;std::cout << "11703" << endl;
	}
	Unit *u;std::cout << "11705" << endl;
	while(cur) {
		if(!xmlStrcmp(cur->name, (const xmlChar*) "Cube")) {
			if(global_file && sdate.empty()) {
				XML_GET_STRING_FROM_PROP(cur, "time", sdate);
				QalculateDateTime qdate;std::cout << "11710" << endl;
				if(qdate.set(sdate)) {
					exchange_rates_time[0] = (time_t) qdate.timestamp().ulintValue();std::cout << "11712" << endl;
					if(exchange_rates_time[0] > exchange_rates_check_time[0]) exchange_rates_check_time[0] = exchange_rates_time[0];
				} else {
					sdate.clear();std::cout << "11715" << endl;
				}
			}
			XML_GET_STRING_FROM_PROP(cur, "currency", currency);
			if(!currency.empty()) {
				XML_GET_STRING_FROM_PROP(cur, "rate", rate);
				if(!rate.empty()) {
					rate = "1/" + rate;
					u = getUnit(currency);std::cout << "11723" << endl;
					if(!u) {
						u = addUnit(new AliasUnit(_("Currency"), currency, "", "", "", u_euro, rate, 1, "", false, true));
					} else if(u->subtype() == SUBTYPE_ALIAS_UNIT) {
						((AliasUnit*) u)->setExpression(rate);std::cout << "11727" << endl;
					}
					if(u) {
						u->setApproximate();std::cout << "11730" << endl;
						u->setPrecision(-2);std::cout << "11731" << endl;
						u->setChanged(false);std::cout << "11732" << endl;
					}
				}
			}
		}
		if(cur->children) {
			cur = cur->children;std::cout << "11738" << endl;
		} else if(cur->next) {
			cur = cur->next;std::cout << "11740" << endl;
		} else {
			cur = cur->parent;std::cout << "11742" << endl;
			if(cur) {
				cur = cur->next;std::cout << "11744" << endl;
			}
		}
	}
	xmlFreeDoc(doc);std::cout << "11748" << endl;
	if(sdate.empty()) {
		struct stat stats;std::cout << "11750" << endl;
		if(stat(filename.c_str(), &stats) == 0) {
			if(exchange_rates_time[0] >= stats.st_mtime) {
#ifdef _WIN32
				struct _utimbuf new_times;std::cout << "11754" << endl;
#else
				struct utimbuf new_times;std::cout << "11756" << endl;
#endif
				struct tm *temptm = localtime(&exchange_rates_time[0]);std::cout << "11758" << endl;
				if(temptm) {
					struct tm extm = *temptm;std::cout << "11760" << endl;
					time_t time_now = time(NULL);std::cout << "11761" << endl;
					struct tm *newtm = localtime(&time_now);std::cout << "11762" << endl;
					if(newtm && newtm->tm_mday != extm.tm_mday) {
						newtm->tm_hour = extm.tm_hour;std::cout << "11764" << endl;
						newtm->tm_min = extm.tm_min;std::cout << "11765" << endl;
						newtm->tm_sec = extm.tm_sec;std::cout << "11766" << endl;
						exchange_rates_time[0] = mktime(newtm);std::cout << "11767" << endl;
					} else {
						time(&exchange_rates_time[0]);std::cout << "11769" << endl;
					}
				} else {
					time(&exchange_rates_time[0]);std::cout << "11772" << endl;
				}
				new_times.modtime = exchange_rates_time[0];std::cout << "11774" << endl;
				new_times.actime = exchange_rates_time[0];std::cout << "11775" << endl;
#ifdef _WIN32
				_utime(filename.c_str(), &new_times);std::cout << "11777" << endl;
#else
				utime(filename.c_str(), &new_times);std::cout << "11779" << endl;
#endif
			} else {
				exchange_rates_time[0] = stats.st_mtime;std::cout << "11782" << endl;
				if(exchange_rates_time[0] > exchange_rates_check_time[0]) exchange_rates_check_time[0] = exchange_rates_time[0];
			}
		}
	}

	filename = buildPath(getLocalDataDir(), "btc.json");
	ifstream file2(filename.c_str());
	if(file2.is_open()) {
		std::stringstream ssbuffer2;std::cout << "11791" << endl;
		ssbuffer2 << file2.rdbuf();std::cout << "11792" << endl;
		string sbuffer = ssbuffer2.str();std::cout << "11793" << endl;
		size_t i = sbuffer.find("\"amount\":");
		if(i != string::npos) {
			i = sbuffer.find("\"", i + 9);
			if(i != string::npos) {
				size_t i2 = sbuffer.find("\"", i + 1);
				((AliasUnit*) u_btc)->setExpression(sbuffer.substr(i + 1, i2 - (i + 1)));std::cout << "11799" << endl;
			}
		}
		file2.close();std::cout << "11802" << endl;
		struct stat stats;std::cout << "11803" << endl;
		if(stat(filename.c_str(), &stats) == 0) {
			exchange_rates_time[1] = stats.st_mtime;std::cout << "11805" << endl;
			if(exchange_rates_time[1] > exchange_rates_check_time[1]) exchange_rates_check_time[1] = exchange_rates_time[1];
		}
	} else {
		exchange_rates_time[1] = ((time_t) 1531087L) * 1000;std::cout << "11809" << endl;
		if(exchange_rates_time[1] > exchange_rates_check_time[1]) exchange_rates_check_time[1] = exchange_rates_time[1];
	}

	Unit *u_usd = getUnit("USD");
	if(!u_usd) return true;

	string sbuffer;std::cout << "11816" << endl;
	filename = buildPath(getLocalDataDir(), "rates.html");
	ifstream file(filename.c_str());
	if(file.is_open()) {
		std::stringstream ssbuffer;std::cout << "11820" << endl;
		ssbuffer << file.rdbuf();std::cout << "11821" << endl;
		sbuffer = ssbuffer.str();std::cout << "11822" << endl;
	}
	if(sbuffer.empty()) {
		if(file.is_open()) file.close();
		file.clear();std::cout << "11826" << endl;
		filename = buildPath(getGlobalDefinitionsDir(), "rates.json");
		file.open(filename.c_str());std::cout << "11828" << endl;
		if(!file.is_open()) return true;
		std::stringstream ssbuffer;std::cout << "11830" << endl;
		ssbuffer << file.rdbuf();std::cout << "11831" << endl;
		sbuffer = ssbuffer.str();std::cout << "11832" << endl;
		string sname;std::cout << "11833" << endl;
		size_t i = sbuffer.find("\"currency_code\":");
		while(i != string::npos) {
			i += 16;std::cout << "11836" << endl;
			size_t i2 = sbuffer.find("\"", i);
			if(i2 == string::npos) break;
			size_t i3 = sbuffer.find("\"", i2 + 1);
			if(i3 != string::npos && i3 - (i2 + 1) == 3) {
				currency = sbuffer.substr(i2 + 1, i3 - (i2 + 1));std::cout << "11841" << endl;
				if(currency.length() == 3 && currency[0] >= 'A' && currency[0] <= 'Z') {
					u = getUnit(currency);std::cout << "11843" << endl;
					if(!u || (u->subtype() == SUBTYPE_ALIAS_UNIT && ((AliasUnit*) u)->firstBaseUnit() == u_usd)) {
						i2 = sbuffer.find("\"rate\":", i3 + 1);
						size_t i4 = sbuffer.find("}", i3 + 1);
						if(i2 != string::npos && i2 < i4) {
							i3 = sbuffer.find(",", i2 + 7);
							rate = sbuffer.substr(i2 + 7, i3 - (i2 + 7));std::cout << "11849" << endl;
							rate = "1/" + rate;
							if(!u) {
								i2 = sbuffer.find("\"name\":\"", i3 + 1);
								if(i2 != string::npos && i2 < i4) {
									i3 = sbuffer.find("\"", i2 + 8);
									if(i3 != string::npos) {
										sname = sbuffer.substr(i2 + 8, i3 - (i2 + 8));std::cout << "11856" << endl;
										remove_blank_ends(sname);std::cout << "11857" << endl;
									}
								} else {
									sname = "";
								}
								u = addUnit(new AliasUnit(_("Currency"), currency, "", "", sname, u_usd, rate, 1, "", false, true), false, true);
								if(u) u->setHidden(true);
							} else {
								((AliasUnit*) u)->setBaseUnit(u_usd);std::cout << "11865" << endl;
								((AliasUnit*) u)->setExpression(rate);std::cout << "11866" << endl;
							}
							if(u) {
								u->setApproximate();std::cout << "11869" << endl;
								u->setPrecision(-2);std::cout << "11870" << endl;
								u->setChanged(false);std::cout << "11871" << endl;
							}
						}
					}
				}
			}
			i = sbuffer.find("\"currency_code\":", i);
		}
		file.close();std::cout << "11879" << endl;
		exchange_rates_time[2] = ((time_t) 1527199L) * 1000;std::cout << "11880" << endl;
		if(exchange_rates_time[2] > exchange_rates_check_time[2]) exchange_rates_check_time[2] = exchange_rates_time[2];
	} else {
		string sname;std::cout << "11883" << endl;
		size_t i = sbuffer.find("class=\'country\'");
		while(i != string::npos) {
			currency = ""; sname = ""; rate = "";
			i += 15;std::cout << "11887" << endl;
			size_t i2 = sbuffer.find("data-currency-code=\"", i);
			if(i2 != string::npos) {
				i2 += 19;std::cout << "11890" << endl;
				size_t i3 = sbuffer.find("\"", i2 + 1);
				if(i3 != string::npos) {
					currency = sbuffer.substr(i2 + 1, i3 - (i2 + 1));std::cout << "11893" << endl;
					remove_blank_ends(currency);std::cout << "11894" << endl;
				}
			}
			i2 = sbuffer.find("data-currency-name=\'", i);
			if(i2 != string::npos) {
				i2 += 19;std::cout << "11899" << endl;
				size_t i3 = sbuffer.find("|", i2 + 1);
				if(i3 != string::npos) {
					sname = sbuffer.substr(i2 + 1, i3 - (i2 + 1));std::cout << "11902" << endl;
					remove_blank_ends(sname);std::cout << "11903" << endl;
				}
			}
			i2 = sbuffer.find("data-rate=\'", i);
			if(i2 != string::npos) {
				i2 += 10;std::cout << "11908" << endl;
				size_t i3 = sbuffer.find("'", i2 + 1);
				if(i3 != string::npos) {
					rate = sbuffer.substr(i2 + 1, i3 - (i2 + 1));std::cout << "11911" << endl;
					remove_blank_ends(rate);std::cout << "11912" << endl;
				}
			}
			if(currency.length() == 3 && currency[0] >= 'A' && currency[0] <= 'Z' && !rate.empty()) {
				u = getUnit(currency);std::cout << "11916" << endl;
				if(!u || (u->subtype() == SUBTYPE_ALIAS_UNIT && ((AliasUnit*) u)->firstBaseUnit() == u_usd)) {
					rate = "1/" + rate;
					if(!u) {
						u = addUnit(new AliasUnit(_("Currency"), currency, "", "", sname, u_usd, rate, 1, "", false, true), false, true);
						if(u) u->setHidden(true);
					} else {
						((AliasUnit*) u)->setBaseUnit(u_usd);std::cout << "11923" << endl;
						((AliasUnit*) u)->setExpression(rate);std::cout << "11924" << endl;
					}
					if(u) {
						u->setApproximate();std::cout << "11927" << endl;
						u->setPrecision(-2);std::cout << "11928" << endl;
						u->setChanged(false);std::cout << "11929" << endl;
					}
				}
			}
			i = sbuffer.find("class=\'country\'", i);
		}
		file.close();std::cout << "11935" << endl;
		struct stat stats;std::cout << "11936" << endl;
		if(stat(filename.c_str(), &stats) == 0) {
			exchange_rates_time[2] = stats.st_mtime;std::cout << "11938" << endl;
			if(exchange_rates_time[2] > exchange_rates_check_time[2]) exchange_rates_check_time[2] = exchange_rates_time[2];
		}
	}

	return true;

}
bool Calculator::hasGVFS() {
	return false;
}
bool Calculator::hasGnomeVFS() {
	return hasGVFS();
}
bool Calculator::canFetch() {
#ifdef HAVE_LIBCURL
	return true;
#else
	return false;
#endif
}
string Calculator::getExchangeRatesFileName(int index) {
	switch(index) {
		case 1: {return buildPath(getLocalDataDir(), "eurofxref-daily.xml");}
		case 2: {return buildPath(getLocalDataDir(), "btc.json");}
		//case 3: {return buildPath(getLocalDataDir(), "rates.json");}
		case 3: {return buildPath(getLocalDataDir(), "rates.html");}
		default: {}
	}
	return "";
}
time_t Calculator::getExchangeRatesTime(int index) {
	if(index > 3) return 0;
	if(index < 1) {
		if(exchange_rates_time[1] < exchange_rates_time[0]) {
			if(exchange_rates_time[2] < exchange_rates_time[1]) return exchange_rates_time[2];
			return exchange_rates_time[1];
		}
		if(exchange_rates_time[2] < exchange_rates_time[0]) return exchange_rates_time[2];
		return exchange_rates_time[0];
	}
	index--;std::cout << "11979" << endl;
	return exchange_rates_time[index];
}
string Calculator::getExchangeRatesUrl(int index) {
	switch(index) {
		case 1: {return "https://www.ecb.europa.eu/stats/eurofxref/eurofxref-daily.xml";}
		case 2: {return "https://api.coinbase.com/v2/prices/spot?currency=EUR";}
		//case 2: {return "http://www.mycurrency.net/service/rates";}
		case 3: {return "https://www.mycurrency.net/=US";}
		default: {}
	}
	return "";
}
bool Calculator::fetchExchangeRates(int timeout, string) {return fetchExchangeRates(timeout);}
size_t write_data(void *ptr, size_t size, size_t nmemb, string *sbuffer) {
	sbuffer->append((char*) ptr, size * nmemb);std::cout << "11994" << endl;
	return size * nmemb;
}
#define FETCH_FAIL_CLEANUP curl_easy_cleanup(curl); curl_global_cleanup(); time(&exchange_rates_check_time[0]); time(&exchange_rates_check_time[1]); time(&exchange_rates_check_time[2]);
bool Calculator::fetchExchangeRates(int timeout, int n) {
#ifdef HAVE_LIBCURL
	if(n <= 0) n = 3;

	recursiveMakeDir(getLocalDataDir());std::cout << "12002" << endl;
	string sbuffer;std::cout << "12003" << endl;
	char error_buffer[CURL_ERROR_SIZE];std::cout << "12004" << endl;
	CURL *curl;std::cout << "12005" << endl;
	CURLcode res;std::cout << "12006" << endl;
	long int file_time = 0;std::cout << "12007" << endl;
	curl_global_init(CURL_GLOBAL_DEFAULT);std::cout << "12008" << endl;
	curl = curl_easy_init();std::cout << "12009" << endl;
	if(!curl) {return false;}
	curl_easy_setopt(curl, CURLOPT_URL, getExchangeRatesUrl(1).c_str());std::cout << "12011" << endl;
	curl_easy_setopt(curl, CURLOPT_TIMEOUT, timeout);std::cout << "12012" << endl;
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);std::cout << "12013" << endl;
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, &sbuffer);std::cout << "12014" << endl;
	curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, error_buffer);std::cout << "12015" << endl;
	error_buffer[0] = 0;std::cout << "12016" << endl;
	curl_easy_setopt(curl, CURLOPT_FILETIME, &file_time);std::cout << "12017" << endl;
#ifdef _WIN32
	char exepath[MAX_PATH];std::cout << "12019" << endl;
	GetModuleFileName(NULL, exepath, MAX_PATH);std::cout << "12020" << endl;
	string datadir(exepath);std::cout << "12021" << endl;
	datadir.resize(datadir.find_last_of('\\'));std::cout << "12022" << endl;
	if(datadir.substr(datadir.length() - 4) != "\\bin" && datadir.substr(datadir.length() - 6) != "\\.libs") {
		string cainfo = buildPath(datadir, "ssl", "certs", "ca-bundle.crt");
		gsub("\\", "/", cainfo);
		curl_easy_setopt(curl, CURLOPT_CAINFO, cainfo.c_str());std::cout << "12026" << endl;
	}
#endif
	res = curl_easy_perform(curl);

	if(res != CURLE_OK) {
		if(strlen(error_buffer)) error(true, _("Failed to download exchange rates from %s: %s."), "ECB", error_buffer, NULL);
		else error(true, _("Failed to download exchange rates from %s: %s."), "ECB", curl_easy_strerror(res), NULL);
		FETCH_FAIL_CLEANUP;std::cout << "12034" << endl;
		return false;
	}
	if(sbuffer.empty()) {error(true, _("Failed to download exchange rates from %s: %s."), "ECB", "Document empty", NULL); FETCH_FAIL_CLEANUP; return false;}
	ofstream file(getExchangeRatesFileName(1).c_str(), ios::out | ios::trunc | ios::binary);std::cout << "12038" << endl;
	if(!file.is_open()) {
		error(true, _("Failed to download exchange rates from %s: %s."), "ECB", strerror(errno), NULL);
		FETCH_FAIL_CLEANUP
		return false;
	}
	file << sbuffer;std::cout << "12044" << endl;
	file.close();std::cout << "12045" << endl;
	if(file_time > 0) {
#ifdef _WIN32
		struct _utimbuf new_times;std::cout << "12048" << endl;
#else
		struct utimbuf new_times;std::cout << "12050" << endl;
#endif
		new_times.modtime = (time_t) file_time;std::cout << "12052" << endl;
		new_times.actime = (time_t) file_time;std::cout << "12053" << endl;
#ifdef _WIN32
		_utime(getExchangeRatesFileName(1).c_str(), &new_times);std::cout << "12055" << endl;
#else
		utime(getExchangeRatesFileName(1).c_str(), &new_times);std::cout << "12057" << endl;
#endif
	}

	if(n >= 2) {

		sbuffer = "";
		curl_easy_setopt(curl, CURLOPT_URL, getExchangeRatesUrl(2).c_str());std::cout << "12064" << endl;
		curl_easy_setopt(curl, CURLOPT_TIMEOUT, timeout);std::cout << "12065" << endl;
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);std::cout << "12066" << endl;
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &sbuffer);std::cout << "12067" << endl;
		curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, error_buffer);std::cout << "12068" << endl;

		res = curl_easy_perform(curl);

		if(res != CURLE_OK) {error(true, _("Failed to download exchange rates from %s: %s."), "coinbase.com", error_buffer, NULL); FETCH_FAIL_CLEANUP; return false;}
		if(sbuffer.empty()) {error(true, _("Failed to download exchange rates from %s: %s."), "coinbase.com", "Document empty", NULL); FETCH_FAIL_CLEANUP; return false;}
		ofstream file3(getExchangeRatesFileName(2).c_str(), ios::out | ios::trunc | ios::binary);std::cout << "12074" << endl;
		if(!file3.is_open()) {
			error(true, _("Failed to download exchange rates from %s: %s."), "coinbase.com", strerror(errno), NULL);
			FETCH_FAIL_CLEANUP
			return false;
		}
		file3 << sbuffer;std::cout << "12080" << endl;
		file3.close();std::cout << "12081" << endl;

	}

	if(n >= 3) {

		sbuffer = "";
		curl_easy_setopt(curl, CURLOPT_URL, getExchangeRatesUrl(3).c_str());std::cout << "12088" << endl;
		curl_easy_setopt(curl, CURLOPT_TIMEOUT, timeout);std::cout << "12089" << endl;
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);std::cout << "12090" << endl;
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &sbuffer);std::cout << "12091" << endl;
		curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, error_buffer);std::cout << "12092" << endl;
		res = curl_easy_perform(curl);

		if(res != CURLE_OK) {error(true, _("Failed to download exchange rates from %s: %s."), "mycurrency.net", error_buffer, NULL); FETCH_FAIL_CLEANUP; return false;}
		if(sbuffer.empty() || sbuffer.find("Internal Server Error") != string::npos) {error(true, _("Failed to download exchange rates from %s: %s."), "mycurrency.net", "Document empty", NULL); FETCH_FAIL_CLEANUP; return false;}
		ofstream file2(getExchangeRatesFileName(3).c_str(), ios::out | ios::trunc | ios::binary);std::cout << "12097" << endl;
		if(!file2.is_open()) {
			error(true, _("Failed to download exchange rates from %s: %s."), "mycurrency.net", strerror(errno), NULL);
			FETCH_FAIL_CLEANUP
			return false;
		}
		file2 << sbuffer;std::cout << "12103" << endl;
		file2.close();std::cout << "12104" << endl;

	}

	curl_easy_cleanup(curl);std::cout << "12108" << endl; curl_global_cleanup();std::cout << "12108" << endl;

	return true;
#else
	return false;
#endif
}
bool Calculator::checkExchangeRatesDate(unsigned int n_days, bool force_check, bool send_warning, int n) {
	if(n <= 0) n = 3;
	time_t extime = exchange_rates_time[0];std::cout << "12117" << endl;
	if(n > 1 && exchange_rates_time[1] < extime) extime = exchange_rates_time[1];
	if(n > 2 && exchange_rates_time[2] < extime) extime = exchange_rates_time[2];
	time_t cextime = exchange_rates_check_time[0];std::cout << "12120" << endl;
	if(n > 1 && exchange_rates_check_time[1] < cextime) cextime = exchange_rates_check_time[1];
	if(n > 2 && exchange_rates_check_time[2] < cextime) cextime = exchange_rates_check_time[2];
	if(extime > 0 && ((!force_check && cextime > 0 && difftime(time(NULL), cextime) < 86400 * n_days) || difftime(time(NULL), extime) < (86400 * n_days) + 3600)) return true;
	time(&exchange_rates_check_time[0]);std::cout << "12124" << endl;
	if(n > 1) time(&exchange_rates_check_time[1]);
	if(n > 2) time(&exchange_rates_check_time[2]);
	if(send_warning) error(false, _("It has been %s day(s) since the exchange rates last were updated."), i2s((int) floor(difftime(time(NULL), extime) / 86400)).c_str(), NULL);
	return false;
}
void Calculator::setExchangeRatesWarningEnabled(bool enable) {
	b_exchange_rates_warning_enabled = enable;std::cout << "12131" << endl;
}
bool Calculator::exchangeRatesWarningEnabled() const {
	return b_exchange_rates_warning_enabled;
}
int Calculator::exchangeRatesUsed() const {
	return b_exchange_rates_used;
}
void Calculator::resetExchangeRatesUsed() {
	b_exchange_rates_used = 0;std::cout << "12140" << endl;
}
void Calculator::setExchangeRatesUsed(int index) {
	if(index > b_exchange_rates_used) b_exchange_rates_used = index;
	if(b_exchange_rates_warning_enabled) checkExchangeRatesDate(7, false, true);
}

bool Calculator::canPlot() {
#ifdef _WIN32
	LPSTR lpFilePart;std::cout << "12149" << endl;
	char filename[MAX_PATH];std::cout << "12150" << endl;
	return SearchPath(NULL, "gnuplot", ".exe", MAX_PATH, filename, &lpFilePart);
#else
	FILE *pipe = popen("gnuplot - 2>/dev/null", "w");
	if(!pipe) return false;
	return pclose(pipe) == 0;
#endif
}

// extern bool fix_intervals(MathStructure &mstruct, const EvaluationOptions &eo, bool *failed = NULL, long int min_precision = 2, bool function_middle = false);
void parse_and_precalculate_plot(string &expression, MathStructure &mstruct, const ParseOptions &po, EvaluationOptions &eo) {
	eo.approximation = APPROXIMATION_APPROXIMATE;std::cout << "12161" << endl;
	ParseOptions po2 = po;std::cout << "12162" << endl;
	po2.read_precision = DONT_READ_PRECISION;std::cout << "12163" << endl;
	eo.parse_options = po2;std::cout << "12164" << endl;
	eo.interval_calculation = INTERVAL_CALCULATION_NONE;std::cout << "12165" << endl;
	mstruct = CALCULATOR->parse(expression, po2);std::cout << "12166" << endl;
	MathStructure mbak(mstruct);std::cout << "12167" << endl;
	eo.calculate_functions = false;std::cout << "12168" << endl;
	eo.expand = false;std::cout << "12169" << endl;
	CALCULATOR->beginTemporaryStopMessages();std::cout << "12170" << endl;
	mstruct.eval(eo);std::cout << "12171" << endl;
	int im = 0;std::cout << "12172" << endl;
	if(CALCULATOR->endTemporaryStopMessages(NULL, &im) > 0 || im > 0) mstruct = mbak;
	eo.calculate_functions = true;std::cout << "12174" << endl;
	eo.expand = true;std::cout << "12175" << endl;
}

MathStructure Calculator::expressionToPlotVector(string expression, const MathStructure &min, const MathStructure &max, int steps, MathStructure *x_vector, string x_var, const ParseOptions &po, int msecs) {
	Variable *v = getActiveVariable(x_var);std::cout << "12179" << endl;
	MathStructure x_mstruct;std::cout << "12180" << endl;
	if(v) x_mstruct = v;
	else x_mstruct = x_var;std::cout << "12182" << endl;
	EvaluationOptions eo;std::cout << "12183" << endl;
	MathStructure mparse;std::cout << "12184" << endl;
	if(msecs > 0) startControl(msecs);
	beginTemporaryStopIntervalArithmetic();std::cout << "12186" << endl;
	parse_and_precalculate_plot(expression, mparse, po, eo);std::cout << "12187" << endl;
	beginTemporaryStopMessages();std::cout << "12188" << endl;
	MathStructure y_vector(mparse.generateVector(x_mstruct, min, max, steps, x_vector, eo));std::cout << "12189" << endl;
	endTemporaryStopMessages();std::cout << "12190" << endl;
	endTemporaryStopIntervalArithmetic();std::cout << "12191" << endl;
	if(msecs > 0) {
		if(aborted()) error(true, _("It took too long to generate the plot data."), NULL);
		stopControl();std::cout << "12194" << endl;
	}
	if(y_vector.size() == 0) {
		error(true, _("Unable to generate plot data with current min, max and sampling rate."), NULL);
	}
	return y_vector;
}
MathStructure Calculator::expressionToPlotVector(string expression, float min, float max, int steps, MathStructure *x_vector, string x_var, const ParseOptions &po, int msecs) {
	MathStructure min_mstruct(min), max_mstruct(max);std::cout << "12202" << endl;
	ParseOptions po2 = po;std::cout << "12203" << endl;
	po2.read_precision = DONT_READ_PRECISION;std::cout << "12204" << endl;
	MathStructure y_vector(expressionToPlotVector(expression, min_mstruct, max_mstruct, steps, x_vector, x_var, po2, msecs));std::cout << "12205" << endl;
	return y_vector;
}
MathStructure Calculator::expressionToPlotVector(string expression, const MathStructure &min, const MathStructure &max, const MathStructure &step, MathStructure *x_vector, string x_var, const ParseOptions &po, int msecs) {
	Variable *v = getActiveVariable(x_var);std::cout << "12209" << endl;
	MathStructure x_mstruct;std::cout << "12210" << endl;
	if(v) x_mstruct = v;
	else x_mstruct = x_var;std::cout << "12212" << endl;
	EvaluationOptions eo;std::cout << "12213" << endl;
	MathStructure mparse;std::cout << "12214" << endl;
	if(msecs > 0) startControl(msecs);
	beginTemporaryStopIntervalArithmetic();std::cout << "12216" << endl;
	parse_and_precalculate_plot(expression, mparse, po, eo);std::cout << "12217" << endl;
	beginTemporaryStopMessages();std::cout << "12218" << endl;
	MathStructure y_vector(mparse.generateVector(x_mstruct, min, max, step, x_vector, eo));std::cout << "12219" << endl;
	endTemporaryStopMessages();std::cout << "12220" << endl;
	endTemporaryStopIntervalArithmetic();std::cout << "12221" << endl;
	if(msecs > 0) {
		if(aborted()) error(true, _("It took too long to generate the plot data."), NULL);
		stopControl();std::cout << "12224" << endl;
	}
	if(y_vector.size() == 0) {
		error(true, _("Unable to generate plot data with current min, max and step size."), NULL);
	}
	return y_vector;
}
MathStructure Calculator::expressionToPlotVector(string expression, float min, float max, float step, MathStructure *x_vector, string x_var, const ParseOptions &po, int msecs) {
	MathStructure min_mstruct(min), max_mstruct(max), step_mstruct(step);std::cout << "12232" << endl;
	ParseOptions po2 = po;std::cout << "12233" << endl;
	po2.read_precision = DONT_READ_PRECISION;std::cout << "12234" << endl;
	MathStructure y_vector(expressionToPlotVector(expression, min_mstruct, max_mstruct, step_mstruct, x_vector, x_var, po2, msecs));std::cout << "12235" << endl;
	return y_vector;
}
MathStructure Calculator::expressionToPlotVector(string expression, const MathStructure &x_vector, string x_var, const ParseOptions &po, int msecs) {
	Variable *v = getActiveVariable(x_var);std::cout << "12239" << endl;
	MathStructure x_mstruct;std::cout << "12240" << endl;
	if(v) x_mstruct = v;
	else x_mstruct = x_var;std::cout << "12242" << endl;
	EvaluationOptions eo;std::cout << "12243" << endl;
	MathStructure mparse;std::cout << "12244" << endl;
	if(msecs > 0) startControl(msecs);
	beginTemporaryStopIntervalArithmetic();std::cout << "12246" << endl;
	parse_and_precalculate_plot(expression, mparse, po, eo);std::cout << "12247" << endl;
	beginTemporaryStopMessages();std::cout << "12248" << endl;
	MathStructure y_vector(mparse.generateVector(x_mstruct, x_vector, eo).eval(eo));std::cout << "12249" << endl;
	endTemporaryStopMessages();std::cout << "12250" << endl;
	endTemporaryStopIntervalArithmetic();std::cout << "12251" << endl;
	if(msecs > 0) {
		if(aborted()) error(true, _("It took too long to generate the plot data."), NULL);
		stopControl();std::cout << "12254" << endl;
	}
	return y_vector;
}

extern bool testComplexZero(const Number *this_nr, const Number *i_nr);

bool Calculator::plotVectors(PlotParameters *param, const vector<MathStructure> &y_vectors, const vector<MathStructure> &x_vectors, vector<PlotDataParameters*> &pdps, bool persistent, int msecs) {

	string homedir = getLocalTmpDir();std::cout << "12263" << endl;
	recursiveMakeDir(homedir);std::cout << "12264" << endl;

	string commandline_extra;std::cout << "12266" << endl;
	string title;std::cout << "12267" << endl;

	if(!param) {
		PlotParameters pp;std::cout << "12270" << endl;
		param = &pp;std::cout << "12271" << endl;
	}

	string plot;std::cout << "12274" << endl;

	if(param->filename.empty()) {
		if(!param->color) {
			commandline_extra += " -mono";
		}
		plot += "set terminal pop\n";
	} else {
		persistent = true;std::cout << "12282" << endl;
		if(param->filetype == PLOT_FILETYPE_AUTO) {
			size_t i = param->filename.rfind(".");
			if(i == string::npos) {
				param->filetype = PLOT_FILETYPE_PNG;std::cout << "12286" << endl;
				error(false, _("No extension in file name. Saving as PNG image."), NULL);
			} else {
				string ext = param->filename.substr(i + 1, param->filename.length() - (i + 1));std::cout << "12289" << endl;
				if(ext == "png") {
					param->filetype = PLOT_FILETYPE_PNG;std::cout << "12291" << endl;
				} else if(ext == "ps") {
					param->filetype = PLOT_FILETYPE_PS;std::cout << "12293" << endl;
				} else if(ext == "pdf") {
					param->filetype = PLOT_FILETYPE_PDF;std::cout << "12295" << endl;
				} else if(ext == "eps") {
					param->filetype = PLOT_FILETYPE_EPS;std::cout << "12297" << endl;
				} else if(ext == "svg") {
					param->filetype = PLOT_FILETYPE_SVG;std::cout << "12299" << endl;
				} else if(ext == "fig") {
					param->filetype = PLOT_FILETYPE_FIG;std::cout << "12301" << endl;
				} else if(ext == "tex") {
					param->filetype = PLOT_FILETYPE_LATEX;std::cout << "12303" << endl;
				} else {
					param->filetype = PLOT_FILETYPE_PNG;std::cout << "12305" << endl;
					error(false, _("Unknown extension in file name. Saving as PNG image."), NULL);
				}
			}
		}
		plot += "set terminal ";
		switch(param->filetype) {
			case PLOT_FILETYPE_FIG: {
				plot += "fig ";
				if(param->color) {
					plot += "color";
				} else {
					plot += "monochrome";
				}
				break;
			}
			case PLOT_FILETYPE_SVG: {
				plot += "svg";
				break;
			}
			case PLOT_FILETYPE_LATEX: {
				plot += "latex ";
				break;
			}
			case PLOT_FILETYPE_PS: {
				plot += "postscript ";
				if(param->color) {
					plot += "color";
				} else {
					plot += "monochrome";
				}
				plot += " \"Times\"";
				break;
			}
			case PLOT_FILETYPE_PDF: {
				plot += "pdf ";
				if(param->color) {
					plot += "color";
				} else {
					plot += "monochrome";
				}
				break;
			}
			case PLOT_FILETYPE_EPS: {
				plot += "postscript eps ";
				if(param->color) {
					plot += "color";
				} else {
					plot += "monochrome";
				}
				plot += " \"Times\"";
				break;
			}
			default: {
				plot += "png ";
				break;
			}

		}
		plot += "\nset output \"";
		plot += param->filename;std::cout << "12365" << endl;
		plot += "\"\n";
	}

	switch(param->legend_placement) {
		case PLOT_LEGEND_NONE: {plot += "set nokey\n"; break;}
		case PLOT_LEGEND_TOP_LEFT: {plot += "set key top left\n"; break;}
		case PLOT_LEGEND_TOP_RIGHT: {plot += "set key top right\n"; break;}
		case PLOT_LEGEND_BOTTOM_LEFT: {plot += "set key bottom left\n"; break;}
		case PLOT_LEGEND_BOTTOM_RIGHT: {plot += "set key bottom right\n"; break;}
		case PLOT_LEGEND_BELOW: {plot += "set key below\n"; break;}
		case PLOT_LEGEND_OUTSIDE: {plot += "set key outside\n"; break;}
	}
	if(!param->x_label.empty()) {
		title = param->x_label;std::cout << "12379" << endl;
		gsub("\"", "\\\"", title);
		plot += "set xlabel \"";
		plot += title;std::cout << "12382" << endl;
		plot += "\"\n";
	}
	if(!param->y_label.empty()) {
		string title = param->y_label;std::cout << "12386" << endl;
		gsub("\"", "\\\"", title);
		plot += "set ylabel \"";
		plot += title;std::cout << "12389" << endl;
		plot += "\"\n";
	}
	if(!param->title.empty()) {
		title = param->title;std::cout << "12393" << endl;
		gsub("\"", "\\\"", title);
		plot += "set title \"";
		plot += title;std::cout << "12396" << endl;
		plot += "\"\n";
	}
	if(param->grid) {
		plot += "set grid\n";

	}
	if(!param->auto_y_min || !param->auto_y_max) {
		plot += "set yrange [";
		if(!param->auto_y_min) plot += d2s(param->y_min);
		plot += ":";
		if(!param->auto_y_max) plot += d2s(param->y_max);
		plot += "]";
		plot += "\n";
	}
	if(param->x_log) {
		plot += "set logscale x ";
		plot += i2s(param->x_log_base);std::cout << "12413" << endl;
		plot += "\n";
	}
	if(param->show_all_borders) {
		plot += "set border 15\n";
	} else {
		bool xaxis2 = false, yaxis2 = false;std::cout << "12419" << endl;
		for(size_t i = 0; i < pdps.size(); i++) {
			if(pdps[i] && pdps[i]->xaxis2) {
				xaxis2 = true;std::cout << "12422" << endl;
			}
			if(pdps[i] && pdps[i]->yaxis2) {
				yaxis2 = true;std::cout << "12425" << endl;
			}
		}
		if(xaxis2 && yaxis2) {
			plot += "set border 15\nset x2tics\nset y2tics\n";
		} else if(xaxis2) {
			plot += "set border 7\nset x2tics\n";
		} else if(yaxis2) {
			plot += "set border 11\nset y2tics\n";
		} else {
			plot += "set border 3\n";
		}
		plot += "set xtics nomirror\nset ytics nomirror\n";
	}
	size_t samples = 1000;std::cout << "12439" << endl;
	for(size_t i = 0; i < y_vectors.size(); i++) {
		if(!y_vectors[i].isUndefined()) {
			if(y_vectors[i].size() > 3000) {
				samples = 6000;std::cout << "12443" << endl;
				break;
			}
			if(y_vectors[i].size() * 2 > samples) samples = y_vectors[i].size() * 2;
		}
	}
	plot += "set samples ";
	plot += i2s(samples);std::cout << "12450" << endl;
	plot += "\n";
	plot += "plot ";
	for(size_t i = 0; i < y_vectors.size(); i++) {
		if(!y_vectors[i].isUndefined()) {
			if(i != 0) {
				plot += ",";
			}
			string filename = "gnuplot_data";
			filename += i2s(i + 1);std::cout << "12459" << endl;
			filename = buildPath(homedir, filename);std::cout << "12460" << endl;
#ifdef _WIN32
			gsub("\\", "\\\\", filename);
#endif
			plot += "\"";
			plot += filename;std::cout << "12465" << endl;
			plot += "\"";
			if(i < pdps.size()) {
				switch(pdps[i]->smoothing) {
					case PLOT_SMOOTHING_UNIQUE: {plot += " smooth unique"; break;}
					case PLOT_SMOOTHING_CSPLINES: {plot += " smooth csplines"; break;}
					case PLOT_SMOOTHING_BEZIER: {plot += " smooth bezier"; break;}
					case PLOT_SMOOTHING_SBEZIER: {plot += " smooth sbezier"; break;}
					default: {}
				}
				if(pdps[i]->xaxis2 && pdps[i]->yaxis2) {
					plot += " axis x2y2";
				} else if(pdps[i]->xaxis2) {
					plot += " axis x2y1";
				} else if(pdps[i]->yaxis2) {
					plot += " axis x1y2";
				}
				if(!pdps[i]->title.empty()) {
					title = pdps[i]->title;std::cout << "12483" << endl;
					gsub("\"", "\\\"", title);
					plot += " title \"";
					plot += title;std::cout << "12486" << endl;
					plot += "\"";
				}
				switch(pdps[i]->style) {
					case PLOT_STYLE_LINES: {plot += " with lines"; break;}
					case PLOT_STYLE_POINTS: {plot += " with points"; break;}
					case PLOT_STYLE_POINTS_LINES: {plot += " with linespoints"; break;}
					case PLOT_STYLE_BOXES: {plot += " with boxes"; break;}
					case PLOT_STYLE_HISTOGRAM: {plot += " with histeps"; break;}
					case PLOT_STYLE_STEPS: {plot += " with steps"; break;}
					case PLOT_STYLE_CANDLESTICKS: {plot += " with candlesticks"; break;}
					case PLOT_STYLE_DOTS: {plot += " with dots"; break;}
				}
				if(param->linewidth < 1) {
					plot += " lw 2";
				} else {
					plot += " lw ";
					plot += i2s(param->linewidth);std::cout << "12503" << endl;
				}
			}
		}
	}
	plot += "\n";

	string plot_data;std::cout << "12510" << endl;
	PrintOptions po;std::cout << "12511" << endl;
	po.number_fraction_format = FRACTION_DECIMAL;
	po.interval_display = INTERVAL_DISPLAY_MIDPOINT;std::cout << "12513" << endl;
	po.decimalpoint_sign = ".";
	po.comma_sign = ",";
	for(size_t serie = 0; serie < y_vectors.size(); serie++) {
		if(!y_vectors[serie].isUndefined()) {
			string filename = "gnuplot_data";
			filename += i2s(serie + 1);std::cout << "12519" << endl;
			string filepath = buildPath(homedir, filename);std::cout << "12520" << endl;
			FILE *fdata = fopen(filepath.c_str(), "w+");
			if(!fdata) {
				error(true, _("Could not create temporary file %s"), filepath.c_str(), NULL);
				return false;
			}
			plot_data = "";
			int non_numerical = 0, non_real = 0;std::cout << "12527" << endl;
			//string str = "";
			if(msecs > 0) startControl(msecs);
			ComparisonResult ct1 = COMPARISON_RESULT_EQUAL, ct2 = COMPARISON_RESULT_EQUAL;std::cout << "12530" << endl;
			size_t last_index = string::npos, last_index2 = string::npos;std::cout << "12531" << endl;
			bool check_continuous = pdps[serie]->test_continuous && (pdps[serie]->style == PLOT_STYLE_LINES || pdps[serie]->style == PLOT_STYLE_POINTS_LINES);std::cout << "12532" << endl;
			bool prev_failed = false;std::cout << "12533" << endl;
			for(size_t i = 1; i <= y_vectors[serie].countChildren(); i++) {
				ComparisonResult ct = COMPARISON_RESULT_UNKNOWN;std::cout << "12535" << endl;
				bool invalid_nr = false, b_imagzero_x = false, b_imagzero_y = false;std::cout << "12536" << endl;
				if(!y_vectors[serie].getChild(i)->isNumber()) {
					invalid_nr = true;std::cout << "12538" << endl;
					non_numerical++;std::cout << "12539" << endl;
					//if(non_numerical == 1) str = y_vectors[serie].getChild(i)->print(po);
				} else if(!y_vectors[serie].getChild(i)->number().isReal()) {
					b_imagzero_y = testComplexZero(&y_vectors[serie].getChild(i)->number(), y_vectors[serie].getChild(i)->number().internalImaginary());std::cout << "12542" << endl;
					if(!b_imagzero_y) {
						invalid_nr = true;std::cout << "12544" << endl;
						non_real++;std::cout << "12545" << endl;
						//if(non_numerical + non_real == 1) str = y_vectors[serie].getChild(i)->print(po);
					}
				}
				if(serie < x_vectors.size() && !x_vectors[serie].isUndefined() && x_vectors[serie].countChildren() == y_vectors[serie].countChildren()) {
					if(!x_vectors[serie].getChild(i)->isNumber()) {
						invalid_nr = true;std::cout << "12551" << endl;
						non_numerical++;std::cout << "12552" << endl;
						//if(non_numerical == 1) str = x_vectors[serie].getChild(i)->print(po);
					} else if(!x_vectors[serie].getChild(i)->number().isReal()) {
						b_imagzero_x = testComplexZero(&x_vectors[serie].getChild(i)->number(), x_vectors[serie].getChild(i)->number().internalImaginary());std::cout << "12555" << endl;
						if(!b_imagzero_x) {
							invalid_nr = true;std::cout << "12557" << endl;
							non_real++;std::cout << "12558" << endl;
							//if(non_numerical + non_real == 1) str = x_vectors[serie].getChild(i)->print(po);
						}
					}
					if(!invalid_nr) {
						if(b_imagzero_y) plot_data += x_vectors[serie].getChild(i)->number().realPart().print(po);
						else plot_data += x_vectors[serie].getChild(i)->print(po);std::cout << "12564" << endl;
						plot_data += " ";
					}
				}
				if(!invalid_nr) {
					if(check_continuous && !prev_failed) {
						if(i == 1 || ct2 == COMPARISON_RESULT_UNKNOWN) ct = COMPARISON_RESULT_EQUAL;
						else ct = y_vectors[serie].getChild(i - 1)->number().compare(y_vectors[serie].getChild(i)->number());std::cout << "12571" << endl;
						if((ct == COMPARISON_RESULT_GREATER || ct == COMPARISON_RESULT_LESS) && (ct1 == COMPARISON_RESULT_GREATER || ct1 == COMPARISON_RESULT_LESS) && (ct2 == COMPARISON_RESULT_GREATER || ct2 == COMPARISON_RESULT_LESS) && ct1 != ct2 && ct != ct2) {
							if(last_index2 != string::npos) plot_data.insert(last_index2 + 1, "  \n");
						}
					}
					if(b_imagzero_x) plot_data += y_vectors[serie].getChild(i)->number().realPart().print(po);
					else plot_data += y_vectors[serie].getChild(i)->print(po);std::cout << "12577" << endl;
					plot_data += "\n";
					prev_failed = false;std::cout << "12579" << endl;
				} else if(!prev_failed) {
					ct = COMPARISON_RESULT_UNKNOWN;std::cout << "12581" << endl;
					plot_data += "  \n";
					prev_failed = true;std::cout << "12583" << endl;
				}
				last_index2 = last_index;std::cout << "12585" << endl;
				last_index = plot_data.length() - 1;std::cout << "12586" << endl;
				ct1 = ct2;std::cout << "12587" << endl;
				ct2 = ct;std::cout << "12588" << endl;
				if(aborted()) {
					fclose(fdata);std::cout << "12590" << endl;
					if(msecs > 0) {
						error(true, _("It took too long to generate the plot data."), NULL);
						stopControl();std::cout << "12593" << endl;
					}
					return false;
				}
			}
			if(msecs > 0) stopControl();
			/*if(non_numerical > 0 || non_real > 0) {
				string stitle;std::cout << "12600" << endl;
				if(serie < pdps.size() && !pdps[serie]->title.empty()) {
					stitle = pdps[serie]->title.c_str();std::cout << "12602" << endl;
				} else {
					stitle = i2s(serie).c_str();std::cout << "12604" << endl;
				}
				if(non_numerical > 0) {
					error(true, _("Series %s contains non-numerical data (\"%s\" first of %s) which can not be properly plotted."), stitle.c_str(), str.c_str(), i2s(non_numerical).c_str(), NULL);
				} else {
					error(true, _("Series %s contains non-real data (\"%s\" first of %s) which can not be properly plotted."), stitle.c_str(), str.c_str(), i2s(non_real).c_str(), NULL);
				}
			}*/
			fputs(plot_data.c_str(), fdata);std::cout << "12612" << endl;
			fflush(fdata);std::cout << "12613" << endl;
			fclose(fdata);std::cout << "12614" << endl;
		}
	}

	return invokeGnuplot(plot, commandline_extra, persistent);
}
bool Calculator::invokeGnuplot(string commands, string commandline_extra, bool persistent) {
	FILE *pipe = NULL;std::cout << "12621" << endl;
	if(!b_gnuplot_open || !gnuplot_pipe || persistent || commandline_extra != gnuplot_cmdline) {
		if(!persistent) {
			closeGnuplot();std::cout << "12624" << endl;
		}
		string commandline = "gnuplot";
		if(persistent) {
			commandline += " -persist";
		}
		commandline += commandline_extra;std::cout << "12630" << endl;
#ifdef _WIN32
		commandline += " - 2>nul";
		pipe = _popen(commandline.c_str(), "w");
#else
		commandline += " - 2>/dev/null";
		pipe = popen(commandline.c_str(), "w");
#endif
		if(!pipe) {
			error(true, _("Failed to invoke gnuplot. Make sure that you have gnuplot installed in your path."), NULL);
			return false;
		}
		if(!persistent && pipe) {
			gnuplot_pipe = pipe;std::cout << "12643" << endl;
			b_gnuplot_open = true;std::cout << "12644" << endl;
			gnuplot_cmdline = commandline_extra;std::cout << "12645" << endl;
		}
	} else {
		pipe = gnuplot_pipe;std::cout << "12648" << endl;
	}
	if(!pipe) {
		return false;
	}
	if(!persistent) {
		fputs("clear\n", pipe);
		fputs("reset\n", pipe);
	}
	fputs(commands.c_str(), pipe);std::cout << "12657" << endl;
	fflush(pipe);std::cout << "12658" << endl;
	if(persistent) {
		return pclose(pipe) == 0;
	}
	return true;
}
bool Calculator::closeGnuplot() {
	if(gnuplot_pipe) {
#ifdef _WIN32
		int rv = _pclose(gnuplot_pipe);std::cout << "12667" << endl;
#else
		int rv = pclose(gnuplot_pipe);std::cout << "12669" << endl;
#endif
		gnuplot_pipe = NULL;std::cout << "12671" << endl;
		b_gnuplot_open = false;std::cout << "12672" << endl;
		return rv == 0;
	}
	gnuplot_pipe = NULL;std::cout << "12675" << endl;
	b_gnuplot_open = false;std::cout << "12676" << endl;
	return true;
}
bool Calculator::gnuplotOpen() {
	return b_gnuplot_open && gnuplot_pipe;
}


#endif