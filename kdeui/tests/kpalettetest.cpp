
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
    KApplication a( argc, argv, "KPalettetest" );

    QStringList palettes = KPalette::getPaletteList();
    for(QStringList::ConstIterator it = palettes.begin(); 
	it != palettes.end(); it++) 
    {
       printf("Palette = %s\n", (*it).ascii());

       KPalette myPalette = KPalette(*it);
    
       printf("Palette Name = \"%s\"\n", myPalette.name().ascii());
       printf("Description:\n\"%s\"\n", myPalette.description().ascii());
       printf("Nr of Colors = %d\n", myPalette.nrColors());
       for(int i = 0; i < myPalette.nrColors(); i++)
       {
         int r,g,b;
         myPalette.color(i).rgb(&r, &g, &b);
         printf("#%d Name = \"%s\" #%02x%02x%02x\n", 
		i, myPalette.colorName(i).ascii(), r,g,b);
       }
    }
}


