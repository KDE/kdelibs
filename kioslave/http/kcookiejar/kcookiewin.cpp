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

// The purpose of the QT_NO_TOOLTIP and QT_NO_WHATSTHIS ifdefs is because
// this file is also used in Konqueror/Embedded. One of the aims of
// Konqueror/Embedded is to be a small as possible to fit on embedded
// devices. For this it's also useful to strip out unneeded features of
// Qt, like for example QToolTip or QWhatsThis. The availability (or the
// lack thereof) can be determined using these preprocessor defines.
// The same applies to the QT_NO_ACCEL ifdef below. I hope it doesn't make
// too much trouble... (Simon)

#include "kcookiewin.h"
#include "kcookiejar.h"

#include <QtGui/QLabel>
#include <QtGui/QLayout>
#include <QtGui/QGroupBox>
#include <QtGui/QPushButton>
#include <QtGui/QRadioButton>
#include <QtGui/QShortcut>

#include <kwindowsystem.h>
#include <klocale.h>
#include <kglobal.h>
#include <klineedit.h>
#include <kiconloader.h>
#include <kapplication.h>
#include <kvbox.h>
#include <kdatetime.h>

KCookieWin::KCookieWin( QWidget *parent, KHttpCookieList cookieList,
                        int defaultButton, bool showDetails )
           :KDialog( parent )
{
    setModal(true);
    setObjectName("cookiealert");
    setButtons(Yes|No|Details);
#ifndef Q_WS_QWS //FIXME(E): Implement for Qt Embedded
    setCaption( i18n("Cookie Alert") );
    setWindowIcon( KIcon("preferences-web-browser-cookies") );
    // all cookies in the list should have the same window at this time, so let's take the first
    if( cookieList.first().windowIds().count() > 0 )
    {
#ifdef Q_WS_WIN
        KWindowSystem::setMainWindow( this, reinterpret_cast<WId>( cookieList.first().windowIds().first() ) );
#else
        KWindowSystem::setMainWindow( this, cookieList.first().windowIds().first());
#endif
    }
    else
    {
        // No window associated... make sure the user notices our dialog.
#ifdef Q_WS_X11
        KWindowSystem::setState( winId(), NET::KeepAbove );
#endif
        kapp->updateUserTimestamp();
    }
#endif
    KVBox* vBox1 = new KVBox( this );
    vBox1->setSpacing( -1 );
    setMainWidget(vBox1);
    // Cookie image and message to user
    KHBox* hBox = new KHBox( vBox1 );
    QLabel* icon = new QLabel( hBox );
    icon->setPixmap(KIcon("dialog-warning").pixmap(IconSize(KIconLoader::Desktop)));
    icon->setAlignment( Qt::AlignCenter );
    icon->setFixedSize( 2*icon->sizeHint() );

    int count = cookieList.count();

    KVBox* vBox = new KVBox( hBox );
    QString txt = i18np("You received a cookie from",
                       "You received %1 cookies from", count);
    QLabel* lbl = new QLabel( txt, vBox );
    lbl->setAlignment( Qt::AlignCenter );
    const KHttpCookie& cookie = cookieList.first();

    QString host (cookie.host());
    int pos = host.indexOf(':');
    if ( pos > 0 )
    {
        QString portNum = host.left(pos);
        host.remove(0, pos+1);
        host += ':';
        host += portNum;
    }

    txt = QString("<b>%1</b>").arg( QUrl::fromAce(host.toLatin1()) );
    if (cookie.isCrossDomain())
       txt += i18n(" <b>[Cross Domain]</b>");
    lbl = new QLabel( txt, vBox );
    lbl->setAlignment( Qt::AlignCenter );
    lbl = new QLabel( i18n("Do you want to accept or reject?"), vBox );
    lbl->setAlignment( Qt::AlignCenter );

    // Cookie Details dialog...
    m_detailView = new KCookieDetail( cookieList, count, vBox1 );
    setDetailsWidget(m_detailView);

    // Cookie policy choice...
    QGroupBox *m_btnGrp = new QGroupBox(i18n("Apply Choice To"),vBox1);
    QVBoxLayout *vbox = new QVBoxLayout;
    txt = (count == 1)? i18n("&Only this cookie") : i18n("&Only these cookies");
    m_onlyCookies = new QRadioButton( txt, m_btnGrp );
    vbox->addWidget(m_onlyCookies);
#ifndef QT_NO_WHATSTHIS
    m_onlyCookies->setWhatsThis(i18n("Select this option to accept/reject only this cookie. "
                              "You will be prompted if another cookie is received. "
                              "<em>(see WebBrowsing/Cookies in the System Settings)</em>." ) );
#endif
    m_allCookiesDomain = new QRadioButton( i18n("All cookies from this do&main"), m_btnGrp );
    vbox->addWidget(m_allCookiesDomain);
#ifndef QT_NO_WHATSTHIS
    m_allCookiesDomain->setWhatsThis(i18n("Select this option to accept/reject all cookies from "
                              "this site. Choosing this option will add a new policy for "
                              "the site this cookie originated from. This policy will be "
                              "permanent until you manually change it from the System Settings "
                              "<em>(see WebBrowsing/Cookies in the System Settings)</em>.") );
#endif
    m_allCookies = new QRadioButton( i18n("All &cookies"), m_btnGrp);
    vbox->addWidget(m_allCookies);
#ifndef QT_NO_WHATSTHIS
    m_allCookies->setWhatsThis(i18n("Select this option to accept/reject all cookies from "
                              "anywhere. Choosing this option will change the global "
                              "cookie policy set in the System Settings for all cookies "
                              "<em>(see WebBrowsing/Cookies in the System Settings)</em>.") );
#endif
    m_btnGrp->setLayout(vbox);
    if (defaultButton == KCookieJar::ApplyToShownCookiesOnly )
        m_onlyCookies->setChecked(true);
    else if (defaultButton == KCookieJar::ApplyToCookiesFromDomain)
        m_allCookiesDomain->setChecked(true);
    else if (defaultButton == KCookieJar::ApplyToAllCookies)
        m_allCookies->setChecked(true);
    else
        m_onlyCookies->setChecked(true);
    setButtonText(KDialog::Yes, i18n("&Accept"));
    setButtonText(KDialog::No, i18n("&Reject"));
    //QShortcut( Qt::Key_Escape, btn, SLOT(animateClick()) );
    setButtonToolTip(Details, i18n("See or modify the cookie information") );
    setDefaultButton(Yes);

    setDetailsWidgetVisible(showDetails);
}

KCookieWin::~KCookieWin()
{
}

KCookieAdvice KCookieWin::advice( KCookieJar *cookiejar, const KHttpCookie& cookie )
{
    int result = exec();

    cookiejar->setShowCookieDetails ( isDetailsWidgetVisible() );

    KCookieAdvice advice = (result==KDialog::Yes) ? KCookieAccept : KCookieReject;

    KCookieJar::KCookieDefaultPolicy preferredPolicy = KCookieJar::ApplyToShownCookiesOnly;
    if (m_allCookiesDomain->isChecked()) {
        preferredPolicy = KCookieJar::ApplyToCookiesFromDomain;
        cookiejar->setDomainAdvice( cookie, advice );
    } else if (m_allCookies->isChecked()) {
        preferredPolicy = KCookieJar::ApplyToAllCookies;
        cookiejar->setGlobalAdvice( advice );
    }
    cookiejar->setPreferredDefaultPolicy( preferredPolicy );

    return advice;
}

KCookieDetail::KCookieDetail( KHttpCookieList cookieList, int cookieCount,
                              QWidget* parent )
              :QGroupBox( parent )
{
    setTitle( i18n("Cookie Details") );
    QGridLayout* grid = new QGridLayout( this );
    grid->addItem( new QSpacerItem(0, fontMetrics().lineSpacing()), 0, 0 );
    grid->setColumnStretch( 1, 3 );

    QLabel* label = new QLabel( i18n("Name:"), this );
    grid->addWidget( label, 1, 0 );
    m_name = new KLineEdit( this );
    m_name->setReadOnly( true );
    m_name->setMaximumWidth( fontMetrics().maxWidth() * 25 );
    grid->addWidget( m_name, 1 ,1 );

    //Add the value
    label = new QLabel( i18n("Value:"), this );
    grid->addWidget( label, 2, 0 );
    m_value = new KLineEdit( this );
    m_value->setReadOnly( true );
    m_value->setMaximumWidth( fontMetrics().maxWidth() * 25 );
    grid->addWidget( m_value, 2, 1);

    label = new QLabel( i18n("Expires:"), this );
    grid->addWidget( label, 3, 0 );
    m_expires = new KLineEdit( this );
    m_expires->setReadOnly( true );
    m_expires->setMaximumWidth(fontMetrics().maxWidth() * 25 );
    grid->addWidget( m_expires, 3, 1);

    label = new QLabel( i18n("Path:"), this );
    grid->addWidget( label, 4, 0 );
    m_path = new KLineEdit( this );
    m_path->setReadOnly( true );
    m_path->setMaximumWidth( fontMetrics().maxWidth() * 25 );
    grid->addWidget( m_path, 4, 1);

    label = new QLabel( i18n("Domain:"), this );
    grid->addWidget( label, 5, 0 );
    m_domain = new KLineEdit( this );
    m_domain->setReadOnly( true );
    m_domain->setMaximumWidth( fontMetrics().maxWidth() * 25 );
    grid->addWidget( m_domain, 5, 1);

    label = new QLabel( i18n("Exposure:"), this );
    grid->addWidget( label, 6, 0 );
    m_secure = new KLineEdit( this );
    m_secure->setReadOnly( true );
    m_secure->setMaximumWidth( fontMetrics().maxWidth() * 25 );
    grid->addWidget( m_secure, 6, 1 );

    if ( cookieCount > 1 )
    {
        QPushButton* btnNext = new QPushButton( i18nc("Next cookie","&Next >>"), this );
        btnNext->setFixedSize( btnNext->sizeHint() );
        grid->addWidget( btnNext, 8, 0, 1, 2 );
        connect( btnNext, SIGNAL(clicked()), SLOT(slotNextCookie()) );
#ifndef QT_NO_TOOLTIP
        btnNext->setToolTip(i18n("Show details of the next cookie") );
#endif
    }
    m_cookieList = cookieList;
    m_cookieNumber = 0;
    slotNextCookie();
}

KCookieDetail::~KCookieDetail()
{
}

void KCookieDetail::slotNextCookie()
{
    if (m_cookieNumber == m_cookieList.count() - 1)
        m_cookieNumber = 0;
    else
        ++m_cookieNumber;
    displayCookieDetails();
}

void KCookieDetail::displayCookieDetails()
{
    const KHttpCookie& cookie = m_cookieList.at(m_cookieNumber);
    m_name->setText(cookie.name());
    m_value->setText((cookie.value()));
    if (cookie.domain().isEmpty())
        m_domain->setText(i18n("Not specified"));
    else
        m_domain->setText(cookie.domain());
    m_path->setText(cookie.path());
    KDateTime cookiedate;
    cookiedate.setTime_t(cookie.expireDate());
    if (cookie.expireDate())
        m_expires->setText(KGlobal::locale()->formatDateTime(cookiedate));
    else
        m_expires->setText(i18n("End of Session"));
    QString sec;
    if (cookie.isSecure())
    {
        if (cookie.isHttpOnly())
            sec = i18n("Secure servers only");
        else
            sec = i18n("Secure servers, page scripts");
    }
    else
    {
        if (cookie.isHttpOnly())
            sec = i18n("Servers");
        else
            sec = i18n("Servers, page scripts");
    }
    m_secure->setText(sec);
}

#include "kcookiewin.moc"
