
#include <iostream>

#include <kaboutdata.h>
#include <kapp.h>
#include <kcmdlineargs.h>

#include "kurifilter.h"

int main(int argc, char **argv) {
    KAboutData aboutData("kurifiltertest", "KURIFilter Test",
                        "1.0");

    KCmdLineArgs::init(argc, argv, &aboutData);
    KApplication app;

    QString a("linuxtoday.com");

    cout << "Filtering: " << a << endl;
    if (KURIFilter::filter()->filterURI(a)) {
	cout << "After filtering: " << a << endl;
    }

    a = "http://www.kde.org";
    cout << "Filtering: " << a << endl;
    if (KURIFilter::filter()->filterURI(a)) {
	cout << "After filtering: " << a << endl;
    }

    return 0;
}

