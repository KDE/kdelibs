
#include <kaboutdata.h>
#include <kcmdlineargs.h>
#include <kapplication.h>
#include <qwidget.h>
#include <qtimer.h>
#include <stdlib.h>
#include "kpalette.h"
#include "kledtest.h"
#include <stdio.h>

#include <qstringlist.h>


int main( int argc, char **argv )
{
    KAboutData about("KPaletteTest", "KPaletteTest", "version");
    KCmdLineArgs::init(argc, argv, &about);
    KApplication a;

    QStringList palettes = KPalette::getPaletteList();
    for(QStringList::ConstIterator it = palettes.begin(); 
	it != palettes.end(); it++) 
    {
       printf("Palette = %s\n", (*it).toAscii().constData());

       KPalette myPalette = KPalette(*it);
    
       printf("Palette Name = \"%s\"\n", myPalette.name().toAscii().constData());
       printf("Description:\n\"%s\"\n", myPalette.description().toAscii().constData());
       printf("Nr of Colors = %d\n", myPalette.nrColors());
       for(int i = 0; i < myPalette.nrColors(); i++)
       {
         int r,g,b;
         myPalette.color(i).getRgb(&r, &g, &b);
         printf("#%d Name = \"%s\" #%02x%02x%02x\n", 
		i, myPalette.colorName(i).toAscii().constData(), r,g,b);
       }
    }
}


