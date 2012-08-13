/*
    Requires the Qt widget libraries, available at no cost at
    http://www.troll.no

    Copyright (C) 1996 Bernd Johannes Wuebben
                       wuebben@math.cornell.edu

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

*/


#include "kfontdialog.h"
#include <kconfig.h>

#include <QApplication>

int main( int argc, char **argv )
{
  QApplication::setApplicationName("KFontDialogTest");

  QApplication app(argc, argv);

  app.setFont(QFont("Helvetica",12));

  //  QFont font = QFont("Times",18,QFont::Bold);

  QFont font;
  int nRet = KFontDialog::getFont(font);
  KFontChooser::FontDiffFlags diffFlags;
  nRet = KFontDialog::getFontDiff(font, diffFlags);

  return nRet;
}
