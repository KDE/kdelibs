/*
    $Id$

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
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
  
    $Log$
    Revision 1.1.1.1  1997/04/09 00:28:09  cvsuser
    Sources imported

    Revision 1.1  1997/03/15 22:40:42  kalle

    Revision 1.5  1997/04/29 02:44:24  wuebben
    and X server fontlist lookup

    Revision 1.4  1997/04/27 01:50:49  wuebben

    Revision 1.3  1997/04/20 14:59:45  wuebben
    fixed a minor bug which caused the last font in the font list to not
    be displayed

    Revision 1.1  1997/04/20 00:18:15  wuebben
    Initial revision

    Revision 1.2  1997/03/02 22:40:59  wuebben

    Revision 1.1  1997/01/04 17:36:44  wuebben
    Initial revision


*/


#ifndef _K_FONT_DIALOG_H_
#define _K_FONT_DIALOG_H_

#include <qmsgbox.h>
#include <qpixmap.h>
#include <qapp.h>
#include <qframe.h> 
#include <qbttngrp.h>
#include <qchkbox.h>
#include <qcombo.h>
/// A font dialog for the KDE
/**
This is a little fontdialog that might come in handy at times.
*/
#include <qpushbt.h>
#include <qradiobt.h>
#include <qscrbar.h>
#include <qtooltip.h>
      // 
    KFontDialog( QWidget *parent = NULL, const char *name = NULL,
			bool modal = FALSE );
      //
      // Case 2) The pointer fontlist is non null. In this cae KFontDialog 
      // will insert the strings of that QStrList into the family combo.
      // 
      // Note: Due to a bug in Qt 1.2 you must 
      // supply at this point at least two fonts in the QStrList that
      // fontlist points to. The bug has been reported and will hopefully
      // be fixed in Qt.1.3. 


public:
    KFontDialog( QWidget *parent = 0L, const char *name = 0L,
			bool modal = FALSE, const QStrList* fontlist = 0L );

    void setFont( const QFont &font );
    QFont font()	{  return selFont; }

    /*
     * This is probably the function you are looking for.
     * Just call this to pop up a dialog to get the selected font.
     * returns result().
     */

    static int getFont( QFont &theFont );

signals:
	/*
	 * connect to this to monitor the font as it as selected if you are
	 * not running modal.
    
private slots:


      void      size_chosen_slot(const char* );
      void      setColors();
private:

    bool loadKDEInstalledFonts();
    void fill_family_combo();
    void setCombos();
   
    QGroupBox	 *box1;
    QGroupBox	 *box2;
    // pointer to an optinally supplied list of fonts to 
    // inserted into the fontdialog font-family combo-box
    QStrList     *fontlist; 

    QLabel	 *family_label;
    QLabel       *style_label;
    QLabel	 *charset_label;

    QLabel	 *actual_family_label;
    QLabel	 *actual_size_label;

    QLabel       *actual_style_label;
    QLabel	 *actual_charset_label;


    
    QLabel	 *actual_size_label_data;
    QLabel       *actual_weight_label_data;
    QLabel       *actual_style_label_data;
    QLabel	 *actual_charset_label_data;
    QComboBox    *family_combo;
    QComboBox    *size_combo;
    QComboBox    *weight_combo;
    QComboBox    *style_combo;
    QComboBox	 *charset_combo;    
    QPushButton	 *ok_button;
    QPushButton	 *cancel_button;

    QLabel       *example_label;
    QFont         selFont;

};


#endif
