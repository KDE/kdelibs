/* -*- C++ -*-

   This file implements the SMIVView class.

   $ Author: Mirko Boehm $
   $ Copyright: (C) 2005, Mirko Boehm $
   $ Contact: mirko@kde.org
         http://www.kde.org
         http://www.hackerbuero.org $
   $ License: LGPL with the following explicit clarification:
         This code may be linked against any version of the Qt toolkit
         from Trolltech, Norway. $

   $Id: SMIVView.cpp 30 2005-08-16 16:16:04Z mirko $
*/

#include "SMIVView.h"

SMIVView::SMIVView ( QWidget *parent )
    : QListView ( parent )
{
    setAlternatingRowColors ( true );
}

SMIVView::~SMIVView ()
{
}

#include "SMIVView.moc"
