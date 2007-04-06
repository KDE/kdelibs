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

#include <QtGui/QApplication>
#include <QtGui/QCheckBox>
#include <QtGui/qdrawutil.h>
#include <QtGui/QFontMetrics>
#include <QtGui/QLabel>
#include <QtCore/QMimeData>
#include <Qt3Support/Q3Grid>
#include <QtGui/QPainter>
#include <QtGui/QMenu>
#include <QtGui/QStyle>
#include <QtGui/QItemDelegate>

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
    setCaption( i18n("Edit Quick Access Entry") );
    setButtons( Ok | Cancel );
    setModal(true);
    setDefaultButton(Ok);
    showButtonSeparator(true);
    KVBox *box = new KVBox( this );
    QString text = i18n("<qt><b>Please provide a description, URL and icon for this Quick Access entry.</b></br></qt>");
    QLabel *label = new QLabel( text, box );
    label->setWordWrap(true);
    box->setSpacing( spacingHint() );

    Q3Grid *grid = new Q3Grid( 2, box );
    grid->setSpacing( spacingHint() );

    QString whatsThisText = i18n("<qt>This is the text that will appear in the Quick Access panel.<p>"
                                 "The description should consist of one or two words "
                                 "that will help you remember what this entry refers to.</qt>");
    label = new QLabel( i18n("&Description:"), grid );
    m_edit = new KLineEdit( grid);
    m_edit->setText( description.isEmpty() ? url.fileName() : description );
    label->setBuddy( m_edit );
    label->setWhatsThis(whatsThisText );
    m_edit->setWhatsThis(whatsThisText );

    whatsThisText = i18n("<qt>This is the location associated with the entry. Any valid URL may be used. For example:<p>"
                         "%1<br>http://www.kde.org<br>ftp://ftp.kde.org/pub/kde/stable<p>"
                         "By clicking on the button next to the text edit box you can browse to an "
                         "appropriate URL.</qt>", QDir::homePath());
    label = new QLabel( i18n("&URL:"), grid );
    m_urlEdit = new KUrlRequester( url.prettyUrl(), grid );
    m_urlEdit->setMode( KFile::Directory );
    label->setBuddy( m_urlEdit );
    label->setWhatsThis(whatsThisText );
    m_urlEdit->setWhatsThis(whatsThisText );

    whatsThisText = i18n("<qt>This is the icon that will appear in the Quick Access panel.<p>"
                         "Click on the button to select a different icon.</qt>");
    label = new QLabel( i18n("Choose an &icon:"), grid );
    m_iconButton = new KIconButton( grid );
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
            appName = QLatin1String( KGlobal::mainComponent().componentName() );
        m_appLocal = new QCheckBox( i18n("&Only show when using this application (%1)",  appName ), box );
        m_appLocal->setChecked( appLocal );
        m_appLocal->setWhatsThis(i18n("<qt>Select this setting if you want this "
                              "entry to show only when using the current application (%1).<p>"
                              "If this setting is not selected, the entry will be available in all "
                              "applications.</qt>",
                               appName));
    }
    else
        m_appLocal = 0L;
    connect(m_urlEdit->lineEdit(),SIGNAL(textChanged ( const QString & )),this,SLOT(urlChanged(const QString & )));
    m_edit->setFocus();
    setMainWidget( box );
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
