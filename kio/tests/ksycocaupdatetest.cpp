#include <kbuildsycocaprogressdialog.h>

#include <kapplication.h>
#include <kcmdlineargs.h>

int main(int argc, char *argv[])
{
    KCmdLineArgs::init(argc,argv, "whatever", 0, qi18n("whatever"), 0);
    KApplication k;

    KBuildSycocaProgressDialog::rebuildKSycoca(0);
    return 0;
}
