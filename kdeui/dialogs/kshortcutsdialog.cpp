/* This file is part of the KDE libraries Copyright (C) 1998 Mark Donohoe <donohoe@kde.org>
    Copyright (C) 1997 Nicolas Hadacek <hadacek@kde.org>
    Copyright (C) 1998 Matthias Ettrich <ettrich@kde.org>
    Copyright (C) 2001 Ellis Whitehead <ellis@kde.org>
    Copyright (C) 2006 Hamish Rodda <rodda@kde.org>
    Copyright (C) 2007 Roberto Raggi <roberto@kdevelop.org>
    Copyright (C) 2007 Andreas Hartmetz <ahartmetz@gmail.com>
    Copyright (C) 2008 Michael Jansen <kde@michael-jansen.biz>
    Copyright (C) 2008 Alexander Dymo <adymo@kdevelop.org>
    Copyright (C) 2009 Chani Armitage <chani@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "kshortcutsdialog.h"
#include "kshortcutsdialog_p.h"
#include "kshortcutschemeshelper_p.h"

#include "kdebug.h"
#include "klocalizedstring.h"

#include <QApplication>
#include <QDialogButtonBox>
#include <QDomDocument>

#include <kconfiggroup.h>
#include <kmessagebox.h>
#include <kxmlguiclient.h>
#include <kxmlguifactory.h>
#include <kactioncollection.h>
#include <kiconloader.h>

/************************************************************************/
/* KShortcutsDialog                                                     */
/*                                                                      */
/* Originally by Nicolas Hadacek <hadacek@via.ecp.fr>                   */
/*                                                                      */
/* Substantially revised by Mark Donohoe <donohoe@kde.org>              */
/*                                                                      */
/* And by Espen Sand <espen@kde.org> 1999-10-19                         */
/* (by using KDialog there is almost no code left ;)                    */
/*                                                                      */
/************************************************************************/

class KShortcutsDialog::KShortcutsDialogPrivate
{
public:

    KShortcutsDialogPrivate(KShortcutsDialog *q)
        : q(q),
          m_keyChooser(0),
          m_schemeEditor(0),
          m_detailsButton(0)
    {
    }

    QList<KActionCollection*> m_collections;

    void changeShortcutScheme(const QString &scheme)
    {
        if (m_keyChooser->isModified() && KMessageBox::questionYesNo(q,
                i18n("The current shortcut scheme is modified. Save before switching to the new one?")) == KMessageBox::Yes) {
            m_keyChooser->save();
        } else {
            m_keyChooser->undoChanges();
        }

        QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
        m_keyChooser->clearCollections();

        foreach (KActionCollection *collection, m_collections) {
            // passing an empty stream forces the clients to reread the XML
            KXMLGUIClient *client = const_cast<KXMLGUIClient *>(collection->parentGUIClient());
            if (client) {
                client->setXMLGUIBuildDocument( QDomDocument() );
            }
        }

        //get xmlguifactory
        if (!m_collections.isEmpty()) {
            const KXMLGUIClient *client = m_collections.first()->parentGUIClient();
            if (client) {
                KXMLGUIFactory *factory = client->factory();
                if (factory) {
                    factory->changeShortcutScheme(scheme);
                }
            }
        }

        foreach (KActionCollection *collection, m_collections) {
            m_keyChooser->addCollection(collection);
        }

        QApplication::restoreOverrideCursor();
     }

    void undoChanges()
    {
        m_keyChooser->undoChanges();
    }

    void toggleDetails()
    {
        const bool isVisible = m_schemeEditor->isVisible();

        m_schemeEditor->setVisible(!isVisible);
        m_detailsButton->setText(i18n("&Details") + (isVisible ? " >>" : " <<"));
    }

    void save()
    {
        m_keyChooser->save();
        emit q->saved();
    }

    KShortcutsDialog *q;
    KShortcutsEditor* m_keyChooser; // ### move
    KShortcutSchemesEditor* m_schemeEditor;
    QPushButton *m_detailsButton;
};


KShortcutsDialog::KShortcutsDialog( KShortcutsEditor::ActionTypes types, KShortcutsEditor::LetterShortcuts allowLetterShortcuts, QWidget *parent )
    : QDialog(parent), d(new KShortcutsDialogPrivate(this))
{
    setWindowTitle(i18n("Configure Shortcuts"));
    setModal(true);

    QVBoxLayout *layout = new QVBoxLayout;
    setLayout(layout);

    d->m_keyChooser = new KShortcutsEditor( this, types, allowLetterShortcuts );
    layout->addWidget(d->m_keyChooser);

    d->m_schemeEditor = new KShortcutSchemesEditor(this);
    connect( d->m_schemeEditor, SIGNAL(shortcutsSchemeChanged(QString)),
             this, SLOT(changeShortcutScheme(QString)) );
    d->m_schemeEditor->hide();
    layout->addWidget(d->m_schemeEditor);

    d->m_detailsButton = new QPushButton;
    d->m_detailsButton->setText(i18n("&Details") + " >>");

    QPushButton *printButton = new QPushButton;
    printButton->setText(i18n("Print"));
    printButton->setIcon(KDE::icon("document-print"));

    QDialogButtonBox *buttonBox = new QDialogButtonBox(this);
    buttonBox->addButton(d->m_detailsButton, QDialogButtonBox::ActionRole);
    buttonBox->addButton(printButton, QDialogButtonBox::ActionRole);
    buttonBox->setStandardButtons(QDialogButtonBox::Ok | QDialogButtonBox::Cancel | QDialogButtonBox::Reset);
    buttonBox->button(QDialogButtonBox::Reset)->setText(i18n("Reset to Defaults"));
    layout->addWidget(buttonBox);

    connect(buttonBox->button(QDialogButtonBox::Reset), SIGNAL(clicked()),
            d->m_keyChooser, SLOT(allDefault()));
    connect(d->m_detailsButton, SIGNAL(clicked()), this, SLOT(toggleDetails()));
    connect(printButton, SIGNAL(clicked()), d->m_keyChooser, SLOT(printShortcuts()) );
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(undoChanges()));

    connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));

    KConfigGroup group( KSharedConfig::openConfig(), "KShortcutsDialog Settings" );
    resize( group.readEntry( "Dialog Size", sizeHint() ) );
}


KShortcutsDialog::~KShortcutsDialog()
{
    KConfigGroup group( KSharedConfig::openConfig(), "KShortcutsDialog Settings" );
    group.writeEntry( "Dialog Size", size(), KConfigGroup::Persistent|KConfigGroup::Global );
    delete d;
}


void KShortcutsDialog::addCollection(KActionCollection *collection, const QString &title)
{
    d->m_keyChooser->addCollection(collection, title);
    d->m_collections << collection;
}


QList<KActionCollection*> KShortcutsDialog::actionCollections() const
{
    return d->m_collections;
}

//FIXME should there be a setSaveSettings method?
bool KShortcutsDialog::configure(bool saveSettings)
{
    disconnect(this, SIGNAL(okClicked()), this, SLOT(save()));
    if (saveSettings) {
        connect(this, SIGNAL(okClicked()), this, SLOT(save()));
    }
    if (isModal()) {
        int retcode = exec();
        return retcode;
    } else {
        show();
        return false;
    }
}

QSize KShortcutsDialog::sizeHint() const
{
    return QSize(600, 480);
}

int KShortcutsDialog::configure(KActionCollection *collection, KShortcutsEditor::LetterShortcuts allowLetterShortcuts,
                          QWidget *parent, bool saveSettings)
{
    kDebug(125) << "KShortcutsDialog::configureKeys( KActionCollection*, " << saveSettings << " )";
    KShortcutsDialog dlg(KShortcutsEditor::AllActions, allowLetterShortcuts, parent);
    dlg.d->m_keyChooser->addCollection(collection);
    return dlg.configure(saveSettings);
}

#include "moc_kshortcutsdialog.cpp"
#include "moc_kshortcutsdialog_p.cpp"

//kate: space-indent on; indent-width 4; replace-tabs on;tab-width 4;
