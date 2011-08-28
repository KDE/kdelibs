/* -*- C++ -*-

   This file declares the SMIVView class. 

   $ Author: Mirko Boehm $
   $ Copyright: (C) 2005, Mirko Boehm $
   $ Contact: mirko@kde.org
         http://www.kde.org
         http://www.hackerbuero.org $
   $ License: LGPL with the following explicit clarification:
         This code may be linked against any version of the Qt toolkit
         from Trolltech, Norway. $

   $Id: SMIVView.h 30 2005-08-16 16:16:04Z mirko $
*/

#ifndef SMIVView_H
#define SMIVView_H

#include <QtGui/QListView>

class SMIVView : public QListView
{
    Q_OBJECT
public:
    SMIVView (QWidget *parent = 0);
    ~SMIVView();
};


#endif // SMIVView_H
