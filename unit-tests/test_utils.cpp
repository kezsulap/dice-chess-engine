#include "test_utils.hpp"
#include <cstdlib>
void mark_test_failure() {
	any_failed = true;
}
void test_any_fails() {
	if (any_failed) {
		std::cerr << RED << "SOME TESTS HAVE FAILED\n";
		exit(1);
	}
	else {
		std::cerr << GREEN << "ALL TESTS PASSED 🎉\n";
	}
}
static int _ = (atexit(test_any_fails), 0);
