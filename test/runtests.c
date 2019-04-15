/* File: runtests.c*/

#include <stdio.h>
#include "minunit.h"
#include "../src/utils.h"
#include "../src/servo.h"

#include "../src/utils.c"
#include "../src/servo.c"


mu_suite_start();
int foo = 0;
tests_run = 0;


static char * test_16bitToSignedIntConv() {
	mu_assert("error, foo != 7", foo == 7);
	return 0;
}

static char * test_motorSafety() {
	mu_assert("error, foo != 7", foo == 7);
	return 0;
}

static char * test_parabolaSafety() {
	mu_assert("error, foo != 7", foo == 7);
	return 0;
}

static char * all_tests() {
	mu_run_test(test_16bitToSignedIntConv);
	mu_run_test(test_motorSafety);
	mu_run_test(test_parabolaSafety);
	return 0;
}

int main(int argc, char **argv) {
	char *result = all_tests();
	if (result != 0) {
	 printf("%s\n", result);
	}
	else {
	 printf("ALL TESTS PASSED\n");
	}
	printf("Tests run: %d\n", tests_run);

	return result != 0;
}

