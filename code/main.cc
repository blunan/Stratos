#include "stratos.h"

int main(int argc, char *argv[]) {
	Stratos test(argc, argv);
	test.CreateNodes();
	test.CreateDevices();
	test.InstallInternetStack();
	test.InstallApplications();
	test.Run();
	return 0;
}