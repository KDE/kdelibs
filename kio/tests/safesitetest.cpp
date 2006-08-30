#include <kapplication.h>
#include <safesite.h>
#include <kcmdlineargs.h>

int main(int argc, char **argv) {
	KCmdLineArgs::init(argc, argv, "safesitetest", 0, 0, 0, 0);
	KApplication app;

	Report *r = SafeSite::Agent::obtainReport("http://www.example.com/");
	r = SafeSite::Agent::obtainReport("http://www.example.org/");
	return app.exec();
}

