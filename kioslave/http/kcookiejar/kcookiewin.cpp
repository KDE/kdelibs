/*
This file is part of KDE

  Copyright (C) 2000- Waldo Bastian <bastian@kde.org>
  Copyright (C) 2000- Dawit Alemayehu <adawit@kde.org>

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

// The purpose of the QT_NO_TOOLTIP and QT_NO_WHATSTHIS ifdefs is because
// this file is also used in Konqueror/Embedded. One of the aims of
// Konqueror/Embedded is to be a small as possible to fit on embedded
// devices. For this it's also useful to strip out unneeded features of
// Qt, like for example QToolTip or QWhatsThis. The availability (or the
// lack thereof) can be determined using these preprocessor defines.
// The same applies to the QT_NO_ACCEL ifdef below. I hope it doesn't make
// too much trouble... (Simon)

#include <qhbox.h>
#include <qvbox.h>
#include <qaccel.h>
#include <qlabel.h>
#include <qwidget.h>
#include <qlayout.h>
#include <qgroupbox.h>
#include <qdatetime.h>
#include <qmessagebox.h>
#include <qpushbutton.h>
#include <qradiobutton.h>
#include <qvbuttongroup.h>

#ifndef QT_NO_TOOLTIP
#include <qtooltip.h>
#endif

#ifndef QT_NO_WHATSTHIS
#include <qwhatsthis.h>
#endif

#include <kwin.h>
#include <klocale.h>
#include <kglobal.h>
#include <kurllabel.h>
#include <klineedit.h>
#include <kiconloader.h>
#include <kapplication.h>

#include "kcookiejar.h"
#include "kcookiewin.h"

KCookieWin::KCookieWin( QWidget *parent, KHttpCookieList cookieList,
                        int defaultButton, bool showDetails )
           :KDialog( parent, "cookiealert", true )
{
#ifndef Q_WS_QWS //FIXME(E): Implement for Qt Embedded
    KWin::setState( winId(), NET::StaysOnTop );
    KWin::setOnDesktop(winId(), KWin::currentDesktop());
    setCaption( i18n("Cookie Alert") );
    setIcon( SmallIcon("cookie") );
#endif
    // Main widget's layout manager...
    QVBoxLayout* vlayout = new QVBoxLayout( this, KDialog::marginHint(), KDialog::spacingHint() );
    vlayout->setResizeMode( QLayout::Fixed );

    // Cookie image and message to user
    QHBox* hBox = new QHBox( this );
    hBox->setSpacing( KDialog::spacingHint() );
    QLabel* icon = new QLabel( hBox );
#if QT_VERSION < 300
    icon->setPixmap( QMessageBox::standardIcon(QMessageBox::Warning, kapp->style().guiStyle()) );
#else
    icon->setPixmap( QMessageBox::standardIcon(QMessageBox::Warning) );
#endif
    icon->setAlignment( Qt::AlignCenter );
    icon->setFixedSize( 2*icon->sizeHint() );

    int count = cookieList.count();

    QVBox* vBox = new QVBox( hBox );
    QString txt = (count == 1) ? i18n("You received a cookie from"):
                  i18n("You received %1 cookies from").arg(count);
    QLabel* lbl = new QLabel( txt, vBox );
    lbl->setAlignment( Qt::AlignCenter );
    KHttpCookiePtr cookie = cookieList.first();
    txt = i18n("<b>%1</b>").arg( cookie->host() );
    if (cookie->isCrossDomain())
       txt += i18n(" <b>[Cross Domain!]</b>");
    lbl = new QLabel( txt, vBox );
    lbl->setAlignment( Qt::AlignCenter );
    lbl = new QLabel( i18n("Do you want to accept or reject?"), vBox );
    lbl->setAlignment( Qt::AlignCenter );
    vlayout->addWidget( hBox, 0, Qt::AlignLeft );

    // Cookie Details dialog...
    m_detailView = new KCookieDetail( cookieList, count, this );
    vlayout->addWidget( m_detailView );
    m_showDetails = showDetails;
    m_showDetails ? m_detailView->show():m_detailView->hide();

    // Cookie policy choice...
    m_btnGrp = new QVButtonGroup( i18n("Apply Choice To"), this );
    m_btnGrp->setRadioButtonExclusive( true );

    txt = (count == 1)? i18n("&Only this cookie") : i18n("&Only these cookies");
    QRadioButton* rb = new QRadioButton( txt, m_btnGrp );
#ifndef QT_NO_WHATSTHIS
    QWhatsThis::add( rb, i18n("Select this option to accept/reject only this cookie. "
                              "You will be prompted if another cookie is received. "
                              "<em>(see WebBrowsing/Cookies in the Control Center)</em>." ) );
#endif
    m_btnGrp->insert( rb );
    rb = new QRadioButton( i18n("All cookies from this do&main"), m_btnGrp );
#ifndef QT_NO_WHATSTHIS
    QWhatsThis::add( rb, i18n("Select this option to accept/reject all cookies from "
                              "this site. Choosing this option will add a new policy for "
                              "the site this cookie originated from. This policy will be "
                              "permanent until you manually change it from the Control Center "
                              "<em>(see WebBrowsing/Cookies in the Control Center)</em>.") );
#endif
    m_btnGrp->insert( rb );
    rb = new QRadioButton( i18n("All &cookies"), m_btnGrp );
#ifndef QT_NO_WHATSTHIS
    QWhatsThis::add( rb, i18n("Select this option to accept/reject all cookies from "
                              "anywhere. Choosing this option will change the global "
                              "cookie policy set in the Control Center for all cookies "
                              "<em>(see WebBrowsing/Cookies in the Control Center)</em>.") );
#endif
    m_btnGrp->insert( rb );
    vlayout->addWidget( m_btnGrp );

    if ( defaultButton > -1 && defaultButton < 3 )
        m_btnGrp->setButton( defaultButton );
    else
        m_btnGrp->setButton( 0 );

    // Accept/Reject buttons
    QWidget* bbox = new QWidget( this );
    QBoxLayout* bbLay = new QHBoxLayout( bbox );
    bbLay->setSpacing( KDialog::spacingHint() );
    QPushButton* btn = new QPushButton( i18n("&Accept"), bbox );
    btn->setDefault( true );
    connect( btn, SIGNAL(clicked()), SLOT(accept()) );
    bbLay->addWidget( btn );
    btn = new QPushButton( i18n("&Reject"), bbox );
    connect( btn, SIGNAL(clicked()), SLOT(reject()) );
    bbLay->addWidget( btn );
    bbLay->addStretch( 1 );
    m_button = new QPushButton( bbox );
    m_button->setText( m_showDetails ? i18n("&Details <<"):i18n("&Details >>") );
    connect( m_button, SIGNAL(clicked()), SLOT(slotCookieDetails()) );
    bbLay->addWidget( m_button );
#ifndef QT_NO_ACCEL
    QAccel* a = new QAccel( this );
    a->connectItem( a->insertItem(Qt::Key_Escape), btn, SLOT(animateClick()) );
#endif


#ifndef QT_NO_WHATSTHIS
    QWhatsThis::add( btn, i18n("See or modify the cookie information") );
#endif
    vlayout->addWidget( bbox );
    setFixedSize( sizeHint() );
}

KCookieWin::~KCookieWin()
{
}

void KCookieWin::slotCookieDetails()
{
    if ( m_detailView->isVisible() )
    {
        m_detailView->setMaximumSize( 0, 0 );
        m_detailView->adjustSize();
        m_detailView->hide();
        m_button->setText( i18n( "&Details >>" ) );
        m_showDetails = false;
    }
    else
    {
        m_detailView->setMaximumSize( 1000, 1000 );
        m_detailView->adjustSize();
        m_detailView->show();
        m_button->setText( i18n( "&Details <<" ) );
        m_showDetails = true;
    }
}

KCookieAdvice KCookieWin::advice( KCookieJar *cookiejar, KHttpCookie* cookie )
{
    int result = exec();
    
    cookiejar->setShowCookieDetails ( m_showDetails );
    
    KCookieAdvice advice = (result==QDialog::Accepted) ? KCookieAccept:KCookieReject;
    
    int preferredPolicy = m_btnGrp->id( m_btnGrp->selected() );
    cookiejar->setPreferredDefaultPolicy( preferredPolicy );
    
    switch ( preferredPolicy )
    {
        case 2:
            cookiejar->setGlobalAdvice( advice );
            break;
        case 1:
            cookiejar->setDomainAdvice( cookie, advice );
            break;
        case 0:
        default:
            break;
    }
    return advice;
}

KCookieDetail::KCookieDetail( KHttpCookieList cookieList, int cookieCount,
                              QWidget* parent, const char* name )
              :QGroupBox( parent, name )
{
    KHttpCookiePtr cookie = cookieList.first();
    setTitle( i18n("Cookie Details") );
    QGridLayout* grid = new QGridLayout( this, 9, 2,
                                         KDialog::spacingHint(),
                                         KDialog::marginHint() );
    grid->addRowSpacing( 0, fontMetrics().lineSpacing() );
    grid->setColStretch( 1, 3 );

    QLabel* label = new QLabel( i18n("Name:"), this );
    grid->addWidget( label, 1, 0 );
    m_name = new KLineEdit( this );
    m_name->setReadOnly( true );
    m_name->setText( cookie->name() );
    m_name->setMaximumWidth( fontMetrics().width('W') * 25 );
    grid->addWidget( m_name, 1 ,1 );

    //Add the value
    label = new QLabel( i18n("Value:"), this );
    grid->addWidget( label, 2, 0 );
    m_value = new KLineEdit( this );
    m_value->setReadOnly( true );
    m_value->setText( cookie->value() );
    m_value->setMaximumWidth( fontMetrics().width('W') * 25 );
    grid->addWidget( m_value, 2, 1);

    label = new QLabel( i18n("Expires:"), this );
    grid->addWidget( label, 3, 0 );
    m_expires = new KLineEdit( this );
    m_expires->setReadOnly( true );
    QDateTime cookiedate;
    cookiedate.setTime_t( cookie->expireDate() );
    if ( cookie->expireDate() )
      m_expires->setText( KGlobal::locale()->formatDateTime(cookiedate) );
    else
      m_expires->setText( i18n("End Of Session") );
    m_expires->setMaximumWidth(fontMetrics().width('W') * 25 );
    grid->addWidget( m_expires, 3, 1);

    label = new QLabel( i18n("Path:"), this );
    grid->addWidget( label, 4, 0 );
    m_path = new KLineEdit( this );
    m_path->setReadOnly( true );
    m_path->setText( cookie->path() );
    m_path->setMaximumWidth( fontMetrics().width('W') * 25 );
    grid->addWidget( m_path, 4, 1);

    label = new QLabel( i18n("Domain:"), this );
    grid->addWidget( label, 5, 0 );
    m_domain = new KLineEdit( this );
    m_domain->setReadOnly( true );
    QString val = cookie->domain();
    m_domain->setText( val.isEmpty()?i18n("Not specified"):val );
    m_domain->setMaximumWidth( fontMetrics().width('W') * 25 );
    grid->addWidget( m_domain, 5, 1);

    label = new QLabel( i18n("Is Secure:"), this );
    grid->addWidget( label, 6, 0 );
    m_secure = new KLineEdit( this );
    m_secure->setReadOnly( true );
    m_secure->setText( cookie->isSecure() ? i18n("True"):i18n("False") );
    m_secure->setMaximumWidth( fontMetrics().width('W') * 25 );
    grid->addWidget( m_secure, 6, 1 );

    if ( cookieCount > 1 )
    {
        QPushButton* btnNext = new QPushButton( i18n("&Next >>"), this );
        btnNext->setFixedSize( btnNext->sizeHint() );
        grid->addMultiCellWidget( btnNext, 8, 8, 0, 1 );
        connect( btnNext, SIGNAL(clicked()), SLOT(slotNextCookie()) );
#ifndef QT_NO_TOOLTIP
        QToolTip::add( btnNext, i18n("Show details of the next cookie") );
#endif
    }
    m_cookieList = cookieList;
    m_cookie = cookie;
}

KCookieDetail::~KCookieDetail()
{
}

void KCookieDetail::slotNextCookie()
{
    KHttpCookiePtr cookie = m_cookieList.first();
    while(cookie)
    {
       if (cookie == m_cookie)
       {
          cookie = m_cookieList.next();
          break;
       }
       cookie = m_cookieList.next();
    }
    m_cookie = cookie;
    if (!m_cookie)
        m_cookie = m_cookieList.first();

    if ( m_cookie )
    {
        m_name->setText( m_cookie->name() );
        m_value->setText( ( m_cookie->value() ) );
        if ( m_cookie->domain().isEmpty() )
          m_domain->setText( i18n("Not specified") );
        else
          m_domain->setText( m_cookie->domain() );
        m_path->setText( m_cookie->path() );
        QDateTime cookiedate;
        cookiedate.setTime_t( m_cookie->expireDate() );
        if ( m_cookie->expireDate() )
          m_expires->setText( KGlobal::locale()->formatDateTime(cookiedate) );
        else
          m_expires->setText( i18n("End of Session") );
        m_secure->setText( m_cookie->isSecure() ? i18n("True"):i18n("False") );
    }
}

#include "kcookiewin.moc"
