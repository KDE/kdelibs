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
#include <qhbox.h>
#include <qvbox.h>
#include <qlabel.h>
#include <qwidget.h>
#include <qlayout.h>
#include <qtooltip.h>
#include <qwhatsthis.h>
#include <qgroupbox.h>
#include <qdatetime.h>
#include <qmessagebox.h>
#include <qradiobutton.h>
#include <qvbuttongroup.h>

#include <kapp.h>
#include <kwin.h>
#include <kdebug.h>
#include <klocale.h>
#include <kglobal.h>
#include <kurllabel.h>
#include <kbuttonbox.h>

#include "kcookiejar.h"
#include "kcookiewin.h"

KCookieWin::KCookieWin( QWidget *parent, KHttpCookie* cookie,
                        int defaultButton, bool showDetails )
           :KDialog( parent, "cookiealert", true )
{
    KWin::setState( winId(), NET::StaysOnTop );
    KWin::setOnDesktop(winId(), KWin::currentDesktop());
    setCaption( i18n("Cookie Alert") );

    // Main widget's layout manager...
    QVBoxLayout* vlayout = new QVBoxLayout( this );
    vlayout->setMargin( KDialog::marginHint() );
    vlayout->setSpacing( KDialog::spacingHint() );
    vlayout->setResizeMode( QLayout::Fixed );

    // Cookie image and message to user
    QHBox* hBox = new QHBox( this );
    hBox->setSpacing( KDialog::marginHint() );

    QVBox* vBox = new QVBox( hBox );
    vBox->setSpacing( KDialog::marginHint() );
    QLabel* icon = new QLabel( vBox );
    icon->setPixmap( QMessageBox::standardIcon(QMessageBox::Warning, kapp->style().guiStyle()) );
    icon->setAlignment( Qt::AlignCenter );
    icon->setFixedSize( 2 * icon->sizeHint() );

    int count = 0;
    KHttpCookie* nextCookie = cookie;
    while ( nextCookie )
    {
        count++;
        nextCookie = nextCookie->next();
    }

    vBox = new QVBox( hBox );
    QString txt = (count == 1) ? i18n("You received a cookie from"):
                  i18n("You received %1 cookies from").arg(count);
    QLabel* lbl = new QLabel( txt, vBox );
    lbl->setAlignment( Qt::AlignCenter );
    txt = i18n("<b>%1</b>").arg( cookie->host() );
    lbl = new QLabel( txt, vBox );
    lbl->setAlignment( Qt::AlignCenter );
    lbl = new QLabel( i18n("Do you want to accept or reject ?"), vBox );
    lbl->setAlignment( Qt::AlignCenter );
    vlayout->addWidget( hBox, 0, Qt::AlignLeft );

    // Cookie Details dialog...
    m_detailView = new KCookieDetail( cookie, count, this );
    vlayout->addWidget( m_detailView );
    m_showDetails = showDetails;
    m_showDetails ? m_detailView->show():m_detailView->hide();

    // Cookie policy choice...
    m_btnGrp = new QVButtonGroup( i18n("Apply choice to"), this );
    m_btnGrp->setRadioButtonExclusive( true );

    txt = (count == 1)? i18n("&Only this cookie") : i18n("&Only these cookies");
    QRadioButton* rb = new QRadioButton( txt, m_btnGrp );
    QWhatsThis::add( rb, i18n("Select this option to accept/reject only this cookie. "
                              "You will be prompted if another cookie is received. "
                              "<em>(see WebBrowsing/Cookies in the control panel)</em>." ) );
    m_btnGrp->insert( rb );
    rb = new QRadioButton( i18n("All cookies from this &domain"), m_btnGrp );
    QWhatsThis::add( rb, i18n("Select this option to accept/reject all cookies from "
                              "this site. Choosing this option will add a new policy for "
                              "the site this cookie originated from.  This policy will be "
                              "permanent until you manually change it from the control panel "
                              "<em>(see WebBrowsing/Cookies in the control panel)</em>.") );
    m_btnGrp->insert( rb );
    rb = new QRadioButton( i18n("All &cookies"), m_btnGrp );
    QWhatsThis::add( rb, i18n("Select this option to accept/reject all cookies from "
                              "anywhere. Choosing this option will change the global "
                              "cookie policy set in the control panel for all cookies "
                              "<em>(see WebBrowsing/Cookies in the control panel)</em>.") );
    m_btnGrp->insert( rb );
    vlayout->addWidget( m_btnGrp );

    if ( defaultButton > -1 && defaultButton < 3 )
        m_btnGrp->setButton( defaultButton );
    else
        m_btnGrp->setButton( 0 );

    // Accept/Reject buttons
    KButtonBox* bbox = new KButtonBox( this );
    m_button = bbox->addButton( i18n("&Accept"), this, SLOT(accept()), false );
    m_button->setDefault( true );
    m_button = bbox->addButton( i18n("&Reject"), this, SLOT(reject()), false );
    bbox->addStretch();
    m_button = bbox->addButton(m_showDetails ? i18n("&Details <<"):i18n("&Details >>"),
                                this, SLOT(slotCookieDetails()), false );
    QWhatsThis::add( m_button, i18n("Click this button to show/hide detailed "
                                  "cookie information") );
    bbox->layout();
    vlayout->addWidget( bbox, 0, Qt::AlignCenter );
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
    KCookieAdvice advice = (result==QDialog::Accepted) ? KCookieAccept:KCookieReject;
    cookiejar->defaultRadioButton = m_btnGrp->id( m_btnGrp->selected() );
    cookiejar->showCookieDetails = m_showDetails;
    kdDebug(7104) << "Show cookie details: " << cookiejar->showCookieDetails << endl;
    switch ( cookiejar->defaultRadioButton )
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

KCookieDetail::KCookieDetail( KHttpCookie* cookie, int cookieCount,
                              QWidget* parent, const char* name )
              :QGroupBox( parent, name )
{
    QVBoxLayout *vlayout = new QVBoxLayout( this );
    vlayout->addSpacing( 2 * KDialog::marginHint() );
    vlayout->setSpacing( KDialog::spacingHint() );
    vlayout->setMargin( 2 * KDialog::marginHint() );
    setTitle( i18n("Cookie details") );

    QString val = cookie->value();
    val.truncate(100);
    m_value = new QLabel( i18n("Value: %1").arg( val ), this );
    QToolTip::add( m_value, cookie->value() );
    vlayout->addWidget( m_value );

    val = cookie->domain();
    m_domain = new QLabel( i18n("Domain: %1").arg(val.isEmpty()?"Unspecified":val), this );
    vlayout->addWidget( m_domain );

    m_path = new QLabel( i18n("Path: %1").arg(cookie->path()), this );
    vlayout->addWidget( m_path );

    QDateTime cookiedate;
    cookiedate.setTime_t( cookie->expireDate() );
    QString sdate = i18n("Expires On: %1").arg( cookie->expireDate() ?
                   KGlobal::locale()->formatDateTime(cookiedate):"Unspecified" );
    m_expires = new QLabel( sdate, this );
    vlayout->addWidget( m_expires );

    m_protocol = new QLabel( i18n("Protocol Version: %1").arg(cookie->protocolVersion()), this );
    vlayout->addWidget( m_protocol );

    m_secure = new QLabel( i18n("Is Secure: %1").arg(cookie->isSecure() ? "True":"False"), this );
    vlayout->addWidget( m_secure );

    if ( cookieCount > 1 )
    {
        QPushButton* btnNext = new QPushButton( i18n("&Next >"), this );
        btnNext->setFlat( true );
        btnNext->setFixedSize( btnNext->sizeHint() );
        QToolTip::add( btnNext, i18n("Click here to see the details for the next cookie") );
        vlayout->addWidget( btnNext, 0, Qt::AlignCenter );
        connect( btnNext, SIGNAL(clicked()), SLOT(slotNextCookie()) );
    }

    m_cookie = cookie;
    m_cookie_orig = cookie;
}

KCookieDetail::~KCookieDetail()
{
}

void KCookieDetail::slotNextCookie()
{
    m_cookie = m_cookie->next();
    if ( !m_cookie )
        m_cookie = m_cookie_orig;

    if ( m_cookie )
    {
        QString val = m_cookie->value();
        val.truncate( 100 );
        m_value->setText( i18n("Value: %1").arg( m_cookie->value() ) );
        QToolTip::add( m_value, m_cookie->value() );

        val = m_cookie->domain();
        m_domain->setText( i18n("Domain: %1").arg( val.isEmpty() ? "Unspecified":val ) );
        m_path->setText( i18n("Path: %1").arg(m_cookie->path()) );
        QDateTime cookiedate;
        cookiedate.setTime_t( m_cookie->expireDate() );
        QString sdate = i18n("Expires On: %1").arg( m_cookie->expireDate() ?
                        KGlobal::locale()->formatDateTime(cookiedate):"Unspecified" );

        m_expires->setText( sdate );
        m_protocol->setText( i18n("Protocol Version: %1").arg(m_cookie->protocolVersion()) );
        m_secure->setText( i18n("Is Secure: %1").arg(m_cookie->isSecure() ? "True":"False") );
    }
}

#include "kcookiewin.moc"
