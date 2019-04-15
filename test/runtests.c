/* File: runtests.c*/

#include <stdio.h>
#include "minunit.h"
#include "../src/locator.c"


mu_suite_start();
int foo = 0;
tests_run = 0;

static char * test_alsaSetup() {
	mu_assert("error, foo != 7", foo == 7);
	return 0;
}

static char * test_wiringPiSetup() {
	mu_assert("error, foo != 7", foo == 7);
	return 0;
}

static char * test_16bitToSignedIntConv() {
	mu_assert("error, foo != 7", foo == 7);
	return 0;
}

static char * test_motorSafety() {
	mu_assert("error, foo != 7", foo == 7);
	return 0;
}

static char * all_tests() {
	mu_run_test(test_alsaSetup);
	mu_run_test(test_wiringPiSetup);
	mu_run_test(test_16bitToSignedIntConv);
	mu_run_test(test_motorSafety);
	return 0;
}


