/*
This file is part of KDE

  Copyright (C) 2000 Waldo Bastian (bastian@kde.org)

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/
//----------------------------------------------------------------------------
//
// KDE File Manager -- HTTP Cookie Dialogs
// $Id$

#include "kcookiewin.h"
#include "kcookiejar.h"

#include <klocale.h>
#include <kapp.h>

#include <qwidget.h>
#include <qlayout.h>
#include <qlabel.h>
#include <qradiobutton.h>
#include <qvbuttongroup.h>

#include <qmessagebox.h>
        
KCookieWin::KCookieWin(QWidget *parent, KHttpCookie *_cookie) :
    KDialogBase( i18n("Cookie Alert"), KDialogBase::Yes | KDialogBase::No,
		 KDialogBase::Yes, KDialogBase::No, 
		 parent, 
		 "cookiealert", true, true,
                 i18n("&Accept"), i18n("&Reject")),
    cookie(_cookie)
{
    QWidget *contents = new QWidget(this);

    QGridLayout *layout = new QGridLayout(contents, 5, 3, 
	KDialog::marginHint(), 
        KDialog::spacingHint());

    layout->setColStretch(0, 0);
    layout->setColStretch(1, 1);
    layout->setRowStretch(0, 0);
    layout->setRowStretch(1, 1);

    layout->addColSpacing(2, KDialog::spacingHint());
    layout->addRowSpacing(3, KDialog::spacingHint());

    QLabel *icon = new QLabel( contents );
    icon->setPixmap(QMessageBox::standardIcon(QMessageBox::Warning, kapp->style().guiStyle()));
    layout->addMultiCellWidget(icon, 0, 2, 0, 0);

    QLabel *text1 = new QLabel( 
	i18n("You received a cookie from host:"),
	contents);
    layout->addWidget(text1, 0, 2, AlignLeft | AlignTop);

    QLabel *text2 = new QLabel( cookie->host(), contents);
    layout->addWidget(text2, 1, 2, AlignCenter);

    QLabel *text3 = new QLabel( i18n("Do you want to accept or reject this cookie?"), contents);
    layout->addWidget( text3, 2, 2, AlignLeft | AlignTop);

    QVButtonGroup *bg = new QVButtonGroup( i18n("Apply to:"), contents);
    bg->setExclusive( true );
    layout->addMultiCellWidget(bg , 4, 4, 0, 2);
    
    rb1 = new QRadioButton( i18n("&This cookie only"), bg );
    rb1->adjustSize();
    rb1->setChecked( true );
     
    rb2 = new QRadioButton( i18n("All cookies from this &domain"), bg );
    rb2->adjustSize();

    rb3 = new QRadioButton( i18n("All &cookies"), bg );
    rb3->adjustSize();

    bg->adjustSize();

    setMainWidget(contents);
    enableButtonSeparator(false);
 
// new QLabel( i18n("Do you want to accept or reject this cookie?"), this, "_msg" );
}

KCookieWin::~KCookieWin()
{
}

KCookieAdvice
KCookieWin::advice(KCookieJar *cookiejar)
{
   int result = exec();
   if (result == Yes)
   {
      if (rb2->isChecked())
         cookiejar->setDomainAdvice( cookie, KCookieAccept);
      else if (rb3->isChecked())
         cookiejar->setGlobalAdvice( KCookieAccept );
      return KCookieAccept;
   }
   else
   {
      if (rb2->isChecked())
         cookiejar->setDomainAdvice( cookie, KCookieReject);
      else if (rb3->isChecked())
         cookiejar->setGlobalAdvice( KCookieReject );
      return KCookieReject;
   } 
   return KCookieReject; // Never reached
}

