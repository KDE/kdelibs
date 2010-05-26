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
#include <QtGui/QFormLayout>
#include <QtGui/QItemDelegate>
#include <QtGui/QLabel>
#include <QtGui/QMenu>
#include <QtGui/QPainter>
#include <QtGui/QStyle>

#include <unistd.h>
#include <kvbox.h>
#include <kconfiggroup.h>


bool KFilePlaceEditDialog::getInformation(bool allowGlobal, KUrl& url,
                                          QString& label, QString& icon,
                                          bool& appLocal, int iconSize,
                                          QWidget *parent )
{
    KFilePlaceEditDialog *dialog = new KFilePlaceEditDialog(allowGlobal, url,
                                                            label, icon,
                                                            appLocal,
                                                            iconSize, parent );
    if ( dialog->exec() == QDialog::Accepted ) {
        // set the return parameters
        url         = dialog->url();
        label       = dialog->label();
        icon        = dialog->icon();
        appLocal    = dialog->applicationLocal();

        delete dialog;
        return true;
    }

    delete dialog;
    return false;
}

KFilePlaceEditDialog::KFilePlaceEditDialog(bool allowGlobal, const KUrl& url,
                                           const QString& label,
                                           const QString &icon, bool appLocal,
                                           int iconSize,
                                           QWidget *parent)
    : KDialog( parent )
{
    setCaption( i18n("Edit Places Entry") );
    setButtons( Ok | Cancel );
    setModal(true);
    setDefaultButton(Ok);

    QWidget *wdg = new QWidget( this );
    QVBoxLayout *box = new QVBoxLayout( wdg );

    QFormLayout *layout = new QFormLayout();
    box->addLayout( layout );

    QString whatsThisText = i18n("<qt>This is the text that will appear in the Places panel.<br /><br />"
                                 "The label should consist of one or two words "
                                 "that will help you remember what this entry refers to. "
                                 "If you do not enter a label, it will be derived from "
                                 "the location's URL.</qt>");
    m_labelEdit = new KLineEdit(wdg);
    layout->addRow(i18n("L&abel:"), m_labelEdit);
    m_labelEdit->setText(label);
    m_labelEdit->setClickMessage(i18n("Enter descriptive label here"));
    m_labelEdit->setWhatsThis(whatsThisText);
    layout->labelForField(m_labelEdit)->setWhatsThis(whatsThisText);

    whatsThisText = i18n("<qt>This is the location associated with the entry. Any valid URL may be used. For example:<br /><br />"
                         "%1<br />http://www.kde.org<br />ftp://ftp.kde.org/pub/kde/stable<br /><br />"
                         "By clicking on the button next to the text edit box you can browse to an "
                         "appropriate URL.</qt>", QDir::homePath());
    m_urlEdit = new KUrlRequester( url.prettyUrl(), wdg );
    m_urlEdit->setMode( KFile::Directory );
    layout->addRow( i18n("&Location:"), m_urlEdit );
    m_urlEdit->setWhatsThis( whatsThisText );
    layout->labelForField(m_urlEdit)->setWhatsThis( whatsThisText );

    whatsThisText = i18n("<qt>This is the icon that will appear in the Places panel.<br /><br />"
                         "Click on the button to select a different icon.</qt>");
    m_iconButton = new KIconButton( wdg );
    layout->addRow( i18n("Choose an &icon:"), m_iconButton );
    m_iconButton->setObjectName( QLatin1String( "icon button" ) );
    m_iconButton->setIconSize( iconSize );
    m_iconButton->setIconType( KIconLoader::NoGroup, KIconLoader::Place );
    if ( icon.isEmpty() )
        m_iconButton->setIcon( KMimeType::iconNameForUrl( url ) );
    else
        m_iconButton->setIcon( icon );
    m_iconButton->setWhatsThis( whatsThisText );
    layout->labelForField(m_iconButton)->setWhatsThis( whatsThisText );

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
    if (!label.isEmpty()) {
        // editing existing entry
        m_labelEdit->setFocus();
    } else {
        // new entry
        m_urlEdit->setFocus();
    }
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

QString KFilePlaceEditDialog::label() const
{
    if (!m_labelEdit->text().isEmpty()) {
        return m_labelEdit->text();
    }

    // derive descriptive label from the URL
    KUrl url = m_urlEdit->url();
    if (!url.fileName().isEmpty()) {
        return url.fileName();
    }
    if (!url.host().isEmpty()) {
        return url.host();
    }
    return url.scheme();
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
