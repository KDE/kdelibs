/*
  $Id$
 
         kedit+ , a simple text editor for the KDE project

  Copyright 1997 Bernd Johannes Wuebben, wuebben@math.cornell.edu
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
  
  KEdit, simple editor class, hacked version of the original by 
  Nov 96, Alexander Sanda <alex@darkstar.ping.at>
 
  */

#include <qpopmenu.h>
#include <qmenubar.h>
#include <qapp.h>
#include <qkeycode.h>
#include <qaccel.h>
#include <qobject.h>
#include <qmlined.h>
#include <qradiobt.h>
#include <qfiledlg.h>
#include <qchkbox.h>
#include <qmsgbox.h>
#include <qcombo.h>
#include <qpushbt.h>
#include <qgrpbox.h>
#include <qregexp.h>
#include <qkeycode.h>
#include <qfileinf.h> 

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>

#include <pwd.h>

  

#ifndef __KEDIT_H__
#define __KEDIT_H__


class KEdSrch : public QDialog
{
    Q_OBJECT;

public:

    KEdSrch ( QWidget *parent = 0, const char *name=0);

    QString getText();
    QLineEdit *values;

private:

    QPushButton *ok, *cancel;
    QCheckBox *sensitive;
    QGroupBox *frame1;
    void resizeEvent(QResizeEvent *);

public slots:

    void selected(int);

};



class KEdit : public QMultiLineEdit
{
    Q_OBJECT;
    
public:

    KEdit (QWidget *parent=0, const char *name=0, const char *filename=0, 
	   unsigned flags = 0);

    ~KEdit();


    enum { ALLOW_OPEN = 1, ALLOW_SAVE = 2, ALLOW_SAVEAS = 4, ALLOW_SEARCH = 8,
            HAS_POPUP = 16,
            SHOW_ERRORS = 32
    };

    enum { READ_ONLY ,READ_WRITE};

    enum { MENU_ID_OPEN = 1,
            MENU_ID_INSERT = 2,
            MENU_ID_SAVE = 3,
            MENU_ID_SAVEAS = 4,
            MENU_ID_SEARCH = 5,
            MENU_ID_SEARCHAGAIN = 6
    };

    enum { KEDIT_OK = 0, KEDIT_OS_ERROR = 1, KEDIT_USER_CANCEL = 2 ,KEDIT_RETRY =3
	   ,KEDIT_NOPERMISSIONS = 4};
    enum { OPEN_READWRITE = 1, OPEN_READONLY = 2, OPEN_INSERT = 4 };
    
  
    int loadFile(const char *, int);
    int saveFile();
    int saveAs();
    int openFile(int);
    bool isModified();
    const char *getName();

    void setName( const char *_name );
    int doSave();
    int insertFile();

    int doSave( const char *_filename );
    int setEditMode(int);
    int doSearch(const char *, int);
    int repeatSearch();
    void initSearch();
    void selectFont();
    int  newFile(int);
    void toggleModified( bool );
    void keyPressEvent ( QKeyEvent *e);
    void mousePressEvent (QMouseEvent*) ;
    void mouseReleaseEvent (QMouseEvent* e);
    void mouseMoveEvent (QMouseEvent* e);
    QString markedText();

private:

    QWidget* p_parent;
    bool modified;
    QFileDialog *fbox;
    int edit_mode;
    char filename[1024];
    char pattern[256];
    bool eventFilter(QObject *, QEvent *);
    void setContextSens();
    KEdSrch *srchdialog;
    unsigned k_flags;

signals:

    void fileChanged();
    void update_status_bar();
    void saving();
    void loading();

public slots:

    void setModified();

};



#endif
