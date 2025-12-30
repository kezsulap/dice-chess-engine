#ifndef TEST_UTILS_H
#define TEST_UTILS_H
#define ASSERT_EQUAL(a, b) assert_equal_impl(__LINE__, a, b, #a, #b)
#define ASSERT_THROWS(e, f) assert_throws_impl<e>(__LINE__, []{f;}, #f, #e)
#define ASSERT_THROWS_WITH_CONTENT(e, f, field, field_value) assert_throws_with_content_impl<e>(__LINE__, []{f;}, #f, #field, #e, [](const auto &__x){return __x.field;}, field_value)
#include <iostream>
#include <string>
#include "../output_operators.hpp"
inline bool any_failed = false;
static const std::string GREEN = "\u001b[32m", RED = "\u001b[31m", CLEAR_COLOURS = "\u001b[0m";
void mark_test_failure();
template <class c> void assert_equal_impl(int line, c a, c b, const std::string &a_name, const std::string &b_name) {
	if (a != b) {
		std::cout << RED << "ERROR IN LINE " << line << ": " << a_name << " = " << a << " not equal to " << b_name << " = " << b << CLEAR_COLOURS << std::endl;
		mark_test_failure();
	}
	else {
		std::cout << GREEN << "PASSED IN LINE " << line << ": " << a_name << " = " << b_name << CLEAR_COLOURS << std::endl;
	}
}
template <class Exception, class F> void assert_throws_impl(int line, F f, const std::string &command, const std::string &exception_name) {
	try {
		f();
	}
	catch(Exception) {
		std::cout << GREEN << "PASSED IN LINE " << line << ": " << command << " raised " << exception_name << CLEAR_COLOURS << std::endl;
		return;
	}
	std::cout << RED << "ERROR IN LINE " << line << ": " << command << " didn't raise anything, expected to raise " << exception_name << CLEAR_COLOURS << std::endl;
	mark_test_failure();
}
template <class Exception, class F, class Extractor>
void assert_throws_with_content_impl(int line, F f, std::string command, const std::string &field_name, const std::string &exception_name, Extractor &&extractor, const std::string &field_value) {
	try {
		f();
	}
	catch(Exception e) {
		if (extractor(e) == field_value) {
			std::cout << GREEN << "PASSED IN LINE " << line << ": " << command << " raised " << exception_name << " with " << field_name << " = " << field_value << CLEAR_COLOURS << std::endl;
			return;
		}
		else {
			std::cout << RED << "ERROR IN LINE " << line << ": " << command << " raised " << exception_name << " with " << field_name << " = " << extractor(e) << " expected " << field_value << CLEAR_COLOURS << std::endl;
			mark_test_failure();
			return;
		}
	}
	std::cout << RED << "ERROR IN LINE " << line << ": " << command << " didn't raise anything, excepted to raise " << exception_name << " with " << field_name << " = " << field_value << CLEAR_COLOURS << std::endl;
	mark_test_failure();
}
#endif
