#include <kbuildsycocaprogressdialog.h>

#include <kapplication.h>
#include <kcmdlineargs.h>

int main(int argc, char *argv[])
{
    KCmdLineArgs::init(argc,argv,"whatever", 0, 0, 0, 0);
    KApplication k;// KMessageBox needs KApp for makeStdCaption

    KBuildSycocaProgressDialog::rebuildKSycoca(0);
    return 0;
}
