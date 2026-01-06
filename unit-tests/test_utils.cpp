#include "test_utils.hpp"
#include <cstdlib>
void mark_test_failure() {
	count_failed++;
}
void test_any_fails() {
	if (count_failed) {
		std::cerr << RED << "SOME (" << count_failed << ") TESTS HAVE FAILED\n";
		exit(1);
	}
	else {
		std::cerr << GREEN << "ALL TESTS PASSED 🎉\n";
	}
}
static int _ = (atexit(test_any_fails), 0);
