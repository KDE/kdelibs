/* This file is part of the KDE libraries
    Copyright (C) 2001,2002,2003 Carsten Pfeiffer <pfeiffer@kde.org>
    Copyright (C) 2007 Kevin Ottens <ervin@kde.org>

    library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation, version 2.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "kfileplaceeditdialog.h"

#include <kaboutdata.h>
#include <kconfig.h>
#include <kdebug.h>
#include <kglobal.h>
#include <kicondialog.h>
#include <kiconloader.h>
#include <kcomponentdata.h>
#include <klineedit.h>
#include <klocale.h>
#include <kmimetype.h>
#include <kio/global.h>
#include <kprotocolinfo.h>
#include <kstringhandler.h>
#include <kurlrequester.h>

#include <QtCore/QMimeData>
#include <QtGui/QApplication>
#include <QtGui/QCheckBox>
#include <QtGui/qdrawutil.h>
#include <QtGui/QFontMetrics>
#include <QtGui/QGridLayout>
#include <QtGui/QItemDelegate>
#include <QtGui/QLabel>
#include <QtGui/QMenu>
#include <QtGui/QPainter>
#include <QtGui/QStyle>

#include <unistd.h>
#include <kvbox.h>
#include <kconfiggroup.h>


bool KFilePlaceEditDialog::getInformation(bool allowGlobal, KUrl& url,
                                          QString& description, QString& icon,
                                          bool& appLocal, int iconSize,
                                          QWidget *parent )
{
    KFilePlaceEditDialog *dialog = new KFilePlaceEditDialog(allowGlobal, url,
                                                            description, icon,
                                                            appLocal,
                                                            iconSize, parent );
    if ( dialog->exec() == QDialog::Accepted ) {
        // set the return parameters
        url         = dialog->url();
        description = dialog->description();
        icon        = dialog->icon();
        appLocal    = dialog->applicationLocal();

        delete dialog;
        return true;
    }

    delete dialog;
    return false;
}

KFilePlaceEditDialog::KFilePlaceEditDialog(bool allowGlobal, const KUrl& url,
                                           const QString& description,
                                           const QString &icon, bool appLocal,
                                           int iconSize,
                                           QWidget *parent)
    : KDialog( parent )
{
    setCaption( i18n("Edit Places Entry") );
    setButtons( Ok | Cancel );
    setModal(true);
    setDefaultButton(Ok);
    showButtonSeparator(true);

    QWidget *wdg = new QWidget( this );
    QVBoxLayout *box = new QVBoxLayout( wdg );

    QGridLayout *layout = new QGridLayout();
    box->addLayout( layout );

    QString whatsThisText = i18n("<qt>This is the text that will appear in the Places panel.<br /><br />"
                                 "The description should consist of one or two words "
                                 "that will help you remember what this entry refers to.</qt>");
    QLabel *label = new QLabel( i18n("&Description:"), wdg );
    layout->addWidget( label, 0, 0 );

    m_edit = new KLineEdit( wdg );
    layout->addWidget( m_edit, 0, 1 );
    m_edit->setText( description.isEmpty() ? url.fileName() : description );
    label->setBuddy( m_edit );
    label->setWhatsThis(whatsThisText );
    m_edit->setWhatsThis(whatsThisText );

    whatsThisText = i18n("<qt>This is the location associated with the entry. Any valid URL may be used. For example:<br /><br />"
                         "%1<br />http://www.kde.org<br />ftp://ftp.kde.org/pub/kde/stable<br /><br />"
                         "By clicking on the button next to the text edit box you can browse to an "
                         "appropriate URL.</qt>", QDir::homePath());
    label = new QLabel( i18n("&Location:"), wdg );
    layout->addWidget( label, 1, 0 );
    m_urlEdit = new KUrlRequester( url.prettyUrl(), wdg );
    m_urlEdit->setMode( KFile::Directory );
    layout->addWidget( m_urlEdit, 1, 1 );
    label->setBuddy( m_urlEdit );
    label->setWhatsThis(whatsThisText );
    m_urlEdit->setWhatsThis(whatsThisText );

    whatsThisText = i18n("<qt>This is the icon that will appear in the Places panel.<br /><br />"
                         "Click on the button to select a different icon.</qt>");
    label = new QLabel( i18n("Choose an &icon:"), wdg );
    layout->addWidget( label, 2, 0 );
    m_iconButton = new KIconButton( wdg );
    layout->addWidget( m_iconButton, 2, 1 );
    m_iconButton->setObjectName( QLatin1String( "icon button" ) );
    m_iconButton->setIconSize( iconSize );
    if ( icon.isEmpty() )
        m_iconButton->setIcon( KMimeType::iconNameForUrl( url ) );
    else
        m_iconButton->setIcon( icon );
    label->setBuddy( m_iconButton );
    label->setWhatsThis(whatsThisText );
    m_iconButton->setWhatsThis(whatsThisText );

    if ( allowGlobal ) {
        QString appName;
        if ( KGlobal::mainComponent().aboutData() )
            appName = KGlobal::mainComponent().aboutData()->programName();
        if ( appName.isEmpty() )
            appName = KGlobal::mainComponent().componentName();
        m_appLocal = new QCheckBox( i18n("&Only show when using this application (%1)",  appName ), wdg );
        m_appLocal->setChecked( appLocal );
        m_appLocal->setWhatsThis(i18n("<qt>Select this setting if you want this "
                              "entry to show only when using the current application (%1).<br /><br />"
                              "If this setting is not selected, the entry will be available in all "
                              "applications.</qt>",
                               appName));
        box->addWidget(m_appLocal);
    }
    else
        m_appLocal = 0L;
    connect(m_urlEdit->lineEdit(),SIGNAL(textChanged ( const QString & )),this,SLOT(urlChanged(const QString & )));
    m_edit->setFocus();
    setMainWidget( wdg );
}

KFilePlaceEditDialog::~KFilePlaceEditDialog()
{
}

void KFilePlaceEditDialog::urlChanged(const QString & text )
{
    enableButtonOk( !text.isEmpty() );
}

KUrl KFilePlaceEditDialog::url() const
{
    return m_urlEdit->url();
}

QString KFilePlaceEditDialog::description() const
{
    return m_edit->text();
}

const QString &KFilePlaceEditDialog::icon() const
{
    return m_iconButton->icon();
}

bool KFilePlaceEditDialog::applicationLocal() const
{
    if ( !m_appLocal )
        return true;

    return m_appLocal->isChecked();
}


#include "kfileplaceeditdialog.moc"
