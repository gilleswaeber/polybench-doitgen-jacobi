#include <iostream>
#include <check.h>


START_TEST(test_money_create)
{
	ck_assert(1);
	std::cout << "test run!" << std::endl;
}
END_TEST

Suite* money_suite(void)
{
	Suite* s;
	TCase* tc_core;

	s = suite_create("Money");

	/* Core test case */
	tc_core = tcase_create("Core");

	tcase_add_test(tc_core, test_money_create);
	suite_add_tcase(s, tc_core);

	return s;
}

int main(void)
{

	std::cout << "### Test Jacobi ###" << std::endl;

	int number_failed;
	Suite* s;
	SRunner* sr;

	s = money_suite();
	sr = srunner_create(s);

	srunner_run_all(sr, CK_NORMAL);
	number_failed = srunner_ntests_failed(sr);
	srunner_free(sr);
	return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
