/*
 * This file is part of the KDE Libraries
 * Copyright (C) 2007 Krzysztof Lichota (lichota@mimuw.edu.pl)
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 *
 */

#include "kswitchlanguagedialog.h"

#include <qlayout.h>
#include <qtooltip.h>
#include <qlabel.h>
#include <qmap.h>

#include <klanguagebutton.h>
#include <kconfig.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kdebug.h>
#include <kpushbutton.h>

struct LanguageRowData
{
    QLabel *label;
    KLanguageButton *languageButton;
    KPushButton *removeButton;
    
    void setRowWidgets(
        QLabel *label,
        KLanguageButton *languageButton,
        KPushButton *removeButton
        )
    {
        this->label = label;
        this->languageButton = languageButton;
        this->removeButton = removeButton;
    }
    
};

class KSwitchLanguageDialogPrivate
{
public:
    KSwitchLanguageDialogPrivate(KSwitchLanguageDialog *parent);
    
    KSwitchLanguageDialog *p; //parent class
    
    /**
        Fills language button with names of languages for which given application has translation.
    */
    void fillApplicationLanguages(KLanguageButton *button);
    
    /**
        Adds one button with language to widget.
    */
    void addLanguageButton(const QString & languageCode, bool primaryLanguage);
    
    /**
        Returns list of languages chosen for application or default languages is they are not set.
    */
    QStringList applicationLanguageList();
    
    QMap<KPushButton*, LanguageRowData> languageRows;
    QPtrList<KLanguageButton> languageButtons;
    QGridLayout *languagesLayout;
    QWidget *page;
};

/*************************** KSwitchLanguageDialog **************************/

KSwitchLanguageDialog::KSwitchLanguageDialog(
    QWidget *parent, 
    const char *name, 
    bool modal
    ):
    KDialogBase(parent, name, modal, i18n("Switch application language"), Ok|Cancel, Ok, true ),
    d(new KSwitchLanguageDialogPrivate(this))
{
    d->page = new QWidget( this );
    setMainWidget(d->page);
    QVBoxLayout *topLayout = new QVBoxLayout( d->page, 0, spacingHint() );
    QLabel *label = new QLabel( i18n("Please choose language which should be used for this application"), d->page, "label1" );
    topLayout->addWidget( label );
    
    QHBoxLayout *languageHorizontalLayout = new QHBoxLayout();
    topLayout->addLayout(languageHorizontalLayout);
    
    d->languagesLayout = new QGridLayout(0 , 2);
    languageHorizontalLayout->addLayout(d->languagesLayout);
    languageHorizontalLayout->addStretch();
    
    QStringList defaultLanguages = d->applicationLanguageList();
    
    for ( QStringList::ConstIterator it = defaultLanguages.begin(); it != defaultLanguages.end(); ++it )
    {
        QString language = *it;
        bool primaryLanguage = (it == defaultLanguages.begin());
        d->addLanguageButton(language, primaryLanguage);
    }
    
    if (defaultLanguages.count() == 0)
    {
        d->addLanguageButton(KGlobal::locale()->defaultLanguage(), true);
    }
    
    QHBoxLayout *addButtonHorizontalLayout = new QHBoxLayout();
    topLayout->addLayout(addButtonHorizontalLayout);
    
    KPushButton *addLangButton = new KPushButton(i18n("Add fallback language"), d->page, "addLangButton");
    QToolTip::add(addLangButton, i18n("Adds one more language which will be used if other translations do not contain proper translation"));
    connect(addLangButton, SIGNAL(clicked()), this, SLOT(slotAddLanguageButton()));
    addButtonHorizontalLayout->addWidget(addLangButton);
    addButtonHorizontalLayout->addStretch();
    
    topLayout->addStretch(10);
}

KSwitchLanguageDialog::~KSwitchLanguageDialog()
{
    delete this->d;
}

void KSwitchLanguageDialog::slotAddLanguageButton()
{
    //adding new button with en_US as it should always be present
    d->addLanguageButton("en_US", d->languageButtons.isEmpty() ? true : false);
}

void KSwitchLanguageDialog::removeButtonClicked()
{
    QObject const *signalSender = sender();
    
    if (signalSender == NULL)
    {
        kdError() << "KSwitchLanguageDialog::removeButtonClicked() called directly, not using signal";
        return;
    }
    
    KPushButton *removeButton = const_cast<KPushButton*>(::qt_cast<const KPushButton*>(signalSender));
    
    if (removeButton == NULL)
    {
        kdError() << "KSwitchLanguageDialog::removeButtonClicked() called from something else than KPushButton";
        return;
    }
    
    QMap<KPushButton *, LanguageRowData>::iterator it = d->languageRows.find(removeButton);
    
    if (it == d->languageRows.end())
    {
        kdError() << "KSwitchLanguageDialog::removeButtonClicked called from unknown KPushButton";
        return;
    }

    LanguageRowData languageRowData = it.data();
    
    d->languageButtons.removeRef(languageRowData.languageButton);
    
    languageRowData.label->deleteLater();
    languageRowData.languageButton->deleteLater();
    languageRowData.removeButton->deleteLater();
    d->languageRows.erase(it);
}

void KSwitchLanguageDialog::languageOnButtonChanged(const QString & languageCode)
{
    for ( QPtrList<KLanguageButton>::ConstIterator it = d->languageButtons.begin(); it != d->languageButtons.end(); ++it )
    {
        KLanguageButton *languageButton = *it;
        if (languageButton->current() == languageCode)
        {
            //update all buttons which have matching id
            //might update buttons which were not changed, but well...
            languageButton->setText(KGlobal::locale()->twoAlphaToLanguageName(languageCode));
        }
    }
}

void KSwitchLanguageDialog::slotOk()
{
    QString languageString;
    bool first = true;
    
    for ( QPtrList<KLanguageButton>::ConstIterator it = d->languageButtons.begin(); it != d->languageButtons.end(); ++it )
    {
        KLanguageButton *languageButton = *it;
        
        if (first == false)
        {
            languageString += ':';
        }
        languageString += languageButton->current();
        first = false;
    }
    
    KConfig *config = KGlobal::config();
    
    if (d->applicationLanguageList().join(":") != languageString)
    {
        //list is different from defaults or saved languages list
        KConfigGroup group(config, "Locale");
    
        group.writeEntry("Language", languageString);
        config->sync();
        
        KMessageBox::information(
            this,
            i18n("Language for this application has been changed. The change will take effect upon next start of application"), //text
            i18n("Application language changed"), //caption
            "ApplicationLanguageChangedWarning" //dontShowAgainName
            );
    }

    emit okClicked();
    accept();
}

/************************ KSwitchLanguageDialogPrivate ***********************/

KSwitchLanguageDialogPrivate::KSwitchLanguageDialogPrivate(
    KSwitchLanguageDialog *parent
    ):
    p(parent)
{
    //NOTE: do NOT use "p" in constructor, it is not fully constructed
}

void KSwitchLanguageDialogPrivate::fillApplicationLanguages(KLanguageButton *button)
{
    KLocale *locale = KGlobal::locale();
    QStringList allLanguages = locale->allLanguagesTwoAlpha();
    for ( QStringList::ConstIterator it = allLanguages.begin(); it != allLanguages.end(); ++it )
    {
        QString languageCode = *it;
        if (locale->isApplicationTranslatedInto(languageCode))
        {
            button->insertItem(
                locale->twoAlphaToLanguageName(languageCode),
                languageCode
                );
        }
    }
}

QStringList KSwitchLanguageDialogPrivate::applicationLanguageList()
{
    KConfig *config = KGlobal::config();
    QStringList languagesList;
    
    if (config->hasGroup("Locale"))
    {
        KConfigGroupSaver saver(config, "Locale");
        
        if (config->hasKey("Language"))
        {
            languagesList = config->readListEntry("Language", ':');
        }
    }
    if (languagesList.empty())
    {
        languagesList = KGlobal::locale()->languageList();
    }
    return languagesList;
}

void KSwitchLanguageDialogPrivate::addLanguageButton(const QString & languageCode, bool primaryLanguage)
{
    QString labelText = primaryLanguage ? i18n("Primary language:") : i18n("Fallback language:");
    
    KLanguageButton *languageButton = new KLanguageButton(page);
    
    languageButton->setText(KGlobal::locale()->twoAlphaToLanguageName(languageCode));
    
    fillApplicationLanguages(languageButton);
    
    languageButton->setCurrentItem(languageCode);
    
    QObject::connect(
        languageButton, 
        SIGNAL(activated( const QString &)), 
        p,
        SLOT(languageOnButtonChanged(const QString &))
        );
    
    LanguageRowData languageRowData;
    KPushButton *removeButton = NULL;
    
    if (primaryLanguage == false)
    {
        removeButton = new KPushButton(i18n("Remove"), page);
        
        QObject::connect(
            removeButton, 
            SIGNAL(clicked()),
            p,
            SLOT(removeButtonClicked())
            );
    }
        
    if (primaryLanguage)
    {
        QToolTip::add(languageButton, i18n("This is main application language which will be used first before any other languages"));
    }
    else
    {
        QToolTip::add(languageButton, i18n("This is language which will be used if any previous languages does not contain proper translation"));
    }
    
    int numRows = languagesLayout->numRows();
    
    QLabel *languageLabel = new QLabel(labelText, page);
    languagesLayout->addWidget( languageLabel, numRows + 1, 1, Qt::AlignAuto );
    languagesLayout->addWidget( languageButton, numRows + 1, 2, Qt::AlignAuto );
    
    if (primaryLanguage == false)
    {
        languagesLayout->addWidget( removeButton, numRows + 1, 3, Qt::AlignAuto );
        
        languageRowData.setRowWidgets(
            languageLabel,
            languageButton,
            removeButton
            );
        removeButton->show();
    }
    
    languageRows.insert(removeButton, languageRowData);
    
    languageButtons.append(languageButton);
    languageButton->show();
    languageLabel->show();
}

#include "kswitchlanguagedialog.moc"

