
#include <kthemebase.h>
#include <qfileinfo.h>
#include <qdir.h>
#include <kapp.h>

int main(int argc, char **argv)
{
    KApplication app(argc, argv, "settheme"); // needed now because of properties.
    puts("settheme: (C)1999 KDE Artisic License - D.M. Duley (mosfet@kde.org)");
    if(argc < 2){
        puts("settheme: You must specify an input file!");
        return(1);
    }
    QFileInfo fi(argv[1]);
    if(!fi.exists()){
        puts("settheme: The input file does not exist.");
        return(2);
    }
    KThemeBase::applyConfigFile(argv[1]);
    return(0);
}
