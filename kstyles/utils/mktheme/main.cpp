#include <stdio.h>
#include <kapp.h>
#include <kthemebase.h>
#include <klocale.h>
#include <stdio.h>

int main(int argc, char **argv)
{
    KApplication app(argc, argv);

    puts("mktheme: (C)1999 KDE Artisic License - D.M. Duley (mosfet@kde.org)");
    if(argc < 2){
        puts("mktheme: You must specify a output file!\n");
        return(1);
    }
    //KThemeBase::writeConfigFile(argv[1]);
    return(0);
}
