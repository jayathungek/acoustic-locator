/* File: runtests.c*/
#define TEST
#include "../src/utils.h"
#include "../src/servo.h"

#include "../src/utils.c"
#include "../src/servo.c"
#include "minunit.h"

int tests_run = 0;


// Testing the convertValueFunction at limits and intermediate values. 
MU_TEST(test_16bitConversion_0xFFFF) {
	char msb = 0xff;
	char lsb = 0xff;
	int correctval = -1;
	int testval = convertValue(msb, lsb);
	mu_check(testval == correctval);
}

MU_TEST(test_16bitConversion_0x0000) {
	char msb = 0x00;
	char lsb = 0x00;
	int correctval = 0;
	int testval = convertValue(msb, lsb);
	mu_check(testval == correctval);
}

MU_TEST(test_16bitConversion_0xC77F) {
	char msb = 0xc7;
	char lsb = 0x7f;
	int correctval = -14465;
	int testval = convertValue(msb, lsb);
	mu_check(testval == correctval);
}

MU_TEST(test_16bitConversion_0x34FE) {
	char msb = 0x34;
	char lsb = 0xfe;
	int correctval = 13566;
	int testval = convertValue(msb, lsb);
	mu_check(testval == correctval);
}

//Testing the turnMotorBy() function to see whether it limits movement

// Test azimuth motor safety (has wider range than elevation)
MU_TEST(test_AZMotor_M90_PLUS_180){
	int curr = -90;
	turnMotorBy(180, AZIMUTH, &curr);
	mu_check(curr == AZ_MAX);
	
}

MU_TEST(test_AZMotor_0_PLUS_180){
	int curr = 0;
	turnMotorBy(180, AZIMUTH, &curr);
	mu_check(curr == AZ_MAX);
	
}

MU_TEST(test_AZMotor_0_PLUS_M180){
	int curr = 0;
	turnMotorBy(-180, AZIMUTH, &curr);
	mu_check(curr == AZ_MIN);
	
}


// Test elevation motor safety 
MU_TEST(test_ELMotor_0_PLUS_70){
	int curr = 0;
	turnMotorBy(70, ELEVATION, &curr);
	mu_check(curr == EL_MAX);
	
}

MU_TEST(test_ELMotor_0_PLUS_M70){
	int curr = 0;
	turnMotorBy(-70, ELEVATION, &curr);
	mu_check(curr == EL_MIN);
}

MU_TEST(test_ELMotor_45_PLUS_M30){
	int curr = 45;
	turnMotorBy(-30, ELEVATION, &curr);
	mu_check(curr == 15);
}

MU_TEST_SUITE(test_conversion) {
	MU_RUN_TEST(test_16bitConversion_0xFFFF);
	MU_RUN_TEST(test_16bitConversion_0xC77F);
	MU_RUN_TEST(test_16bitConversion_0x34FE);
	MU_RUN_TEST(test_16bitConversion_0x0000);
}

MU_TEST_SUITE(test_azimuth) {
	MU_RUN_TEST(test_AZMotor_M90_PLUS_180);
	MU_RUN_TEST(test_AZMotor_0_PLUS_180);
	MU_RUN_TEST(test_AZMotor_0_PLUS_M180);
}

MU_TEST_SUITE(test_elevation) {
	MU_RUN_TEST(test_ELMotor_0_PLUS_70);
	MU_RUN_TEST(test_ELMotor_0_PLUS_M70);
	MU_RUN_TEST(test_ELMotor_45_PLUS_M30);
}

int main(int argc, char *argv[]) {
	MU_RUN_SUITE(test_conversion);
	MU_RUN_SUITE(test_azimuth);
	MU_RUN_SUITE(test_elevation);
	MU_REPORT();
	return minunit_status;
}
