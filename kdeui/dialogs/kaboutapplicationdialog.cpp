/* This file is part of the KDE libraries
   Copyright (C) 2007 Urs Wolfer <uwolfer at kde.org>

   Parts of this class have been take from the KAboutApplication class, which was
   Copyright (C) 2000 Waldo Bastian (bastian@kde.org) and Espen Sand (espen@kde.org)

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include <QLabel>
#include <QLayout>
#include <QTabWidget>

#include <kaboutdata.h>
#include <kapplication.h>
#include <kglobal.h>
#include <kglobalsettings.h>
#include <kiconloader.h>
#include <klocale.h>
#include <ktextbrowser.h>

#include "kaboutapplicationdialog.h"

KAboutApplicationDialog::KAboutApplicationDialog(const KAboutData *aboutData, QWidget *parent)
  : KDialog(parent)
{
    setPlainCaption(i18n("About %1", aboutData->programName()));
    setButtons(KDialog::Close);
    setDefaultButton(KDialog::Close);
    setModal(false);

    if (aboutData == 0)
        aboutData = KGlobal::mainComponent().aboutData();

    if (!aboutData)
    {
        QLabel *errorLabel = new QLabel(i18n("No information available.<br />"
                                             "The supplied KAboutData object does not exist."), this);
        setMainWidget(errorLabel);
        return;
    }

    QFrame* titleLabel = new QFrame();
    titleLabel->setAutoFillBackground(true);
    titleLabel->setFrameShape(QFrame::StyledPanel);
    titleLabel->setFrameShadow(QFrame::Plain);
    titleLabel->setBackgroundRole(QPalette::Base);

    QLabel *iconLabel = new QLabel(titleLabel);
    iconLabel->setFixedSize(64, 64);
    iconLabel->setPixmap(qApp->windowIcon().pixmap(64, 64));
    if (aboutData->programLogo().canConvert<QPixmap>())
        iconLabel->setPixmap(aboutData->programLogo().value<QPixmap>());
    else if (aboutData->programLogo().canConvert<QImage>())
        iconLabel->setPixmap(QPixmap::fromImage(aboutData->programLogo().value<QImage>()));

    QLabel *headerLabel = new QLabel(titleLabel);
    headerLabel->setAlignment(Qt::AlignLeft);
    headerLabel->setText(i18n("<html><font size=\"5\">%1</font><br><b>version %2</b><br>Using KDE %3</html>", aboutData->programName(),
                         aboutData->version(), QString(KDE_VERSION_STRING)));

    QTabWidget *tabWidget = new QTabWidget;
    tabWidget->setUsesScrollButtons(false);

    QString aboutPageText = aboutData->shortDescription() + "<br />";

    if (!aboutData->otherText().isEmpty())
        aboutPageText += "<br />" + aboutData->otherText() + "<br />";

    if (!aboutData->copyrightStatement().isEmpty())
        aboutPageText += "<br />" + aboutData->copyrightStatement() + "<br />";

    if (!aboutData->homepage().isEmpty())
        aboutPageText += "<br />" + QString("<a href=\"%1\">%1</a>").arg(aboutData->homepage()) + "<br />";

    QLabel *aboutLabel = new QLabel;
    aboutLabel->setMargin(10);
    aboutLabel->setWordWrap(true);
    aboutLabel->setOpenExternalLinks(true);
    aboutLabel->setText(aboutPageText.replace('\n', "<br />"));
    tabWidget->addTab(aboutLabel, i18n("&About"));

    int authorCount = aboutData->authors().count();
    if (authorCount)
    {
        QString authorPageText;

        QString authorPageTitle = authorCount == 1 ? i18n("A&uthor") : i18n("A&uthors");

        if (!aboutData->customAuthorTextEnabled() || !aboutData->customAuthorRichText().isEmpty())
        {
            if (!aboutData->customAuthorTextEnabled())
            {
                if (aboutData->bugAddress().isEmpty() || aboutData->bugAddress() == "submit@bugs.kde.org")
                    authorPageText = i18n("Please use <a href=\"http://bugs.kde.org\">http://bugs.kde.org</a> to report bugs.\n");
                else
                {
                    if(aboutData->authors().count() == 1 && (aboutData->authors().first().emailAddress() == aboutData->bugAddress()))
                    {
                        authorPageText = i18n("Please report bugs to <a href=\"mailto:%1\">%2</a>.\n",
                                              aboutData->authors().first().emailAddress(),
                                              aboutData->authors().first().emailAddress());
                    }
                    else
                    {
                        authorPageText = i18n("Please report bugs to <a href=\"mailto:%1\">%2</a>.\n",
                                              aboutData->bugAddress(), aboutData->bugAddress());
                    }
                }
            }
            else
                authorPageText = aboutData->customAuthorRichText();
        }

        authorPageText += "<br />";

        QList<KAboutPerson> lst = aboutData->authors();
        for (int i = 0; i < lst.size(); ++i)
        {
            authorPageText += QString("<br />%1<br />").arg(lst.at(i).name());
            if (!lst.at(i).emailAddress().isEmpty())
                authorPageText += QString("&nbsp;&nbsp;<a href=\"mailto:%1\">%1</a><br />").arg(lst.at(i).emailAddress());
            if (!lst.at(i).webAddress().isEmpty())
                authorPageText += QString("&nbsp;&nbsp;<a href=\"%3\">%3</a><br />").arg(lst.at(i).webAddress());
            if (!lst.at(i).task().isEmpty())
                authorPageText += QString("&nbsp;&nbsp;%4<br />").arg(lst.at(i).task());
        }

        KTextBrowser *authorTextBrowser = new KTextBrowser;
        authorTextBrowser->setFrameStyle(QFrame::NoFrame);
        authorTextBrowser->setHtml(authorPageText);
        tabWidget->addTab(authorTextBrowser, authorPageTitle);
    }

    int creditsCount = aboutData->credits().count();
    if (creditsCount)
    {
        QString creditsPageText;

        QList<KAboutPerson> lst = aboutData->credits();
        for (int i = 0; i < lst.size(); ++i)
        {
            creditsPageText += QString("<br />%1<br />").arg(lst.at(i).name());
            if (!lst.at(i).emailAddress().isEmpty())
                creditsPageText += QString("&nbsp;&nbsp;<a href=\"mailto:%1\">%1</a><br />").arg(lst.at(i).emailAddress());
            if (!lst.at(i).webAddress().isEmpty())
                creditsPageText += QString("&nbsp;&nbsp;<a href=\"%3\">%3</a><br />").arg(lst.at(i).webAddress());
            if (!lst.at(i).task().isEmpty())
                creditsPageText += QString("&nbsp;&nbsp;%4<br />").arg(lst.at(i).task());
        }

        KTextBrowser *creditsTextBrowser = new KTextBrowser;
        creditsTextBrowser->setFrameStyle( QFrame::NoFrame );
        creditsTextBrowser->setHtml(creditsPageText);
        tabWidget->addTab(creditsTextBrowser, i18n("&Thanks To"));
    }

    const QList<KAboutTranslator> translatorList = aboutData->translators();

    if(translatorList.count() > 0)
    {
        QString translatorPageText = QString();

        QList<KAboutTranslator>::ConstIterator it;
        for(it = translatorList.begin(); it != translatorList.end(); ++it)
        {
            translatorPageText += QString("<br />%1<br />").arg((*it).name());
            if (!(*it).emailAddress().isEmpty())
                translatorPageText += QString("&nbsp;&nbsp;<a href=\"mailto:%1\">%1</a><br />").arg((*it).emailAddress());
        }

        translatorPageText += KAboutData::aboutTranslationTeam();

        KTextBrowser *translatorTextBrowser = new KTextBrowser;
        translatorTextBrowser->setFrameStyle( QFrame::NoFrame );
        translatorTextBrowser->setHtml(translatorPageText);
        tabWidget->addTab(translatorTextBrowser, i18n("T&ranslation"));
    }

    if (!aboutData->license().isEmpty())
    {
        KTextBrowser *licenseBrowser = new KTextBrowser;
        licenseBrowser->setFrameStyle( QFrame::NoFrame );
        licenseBrowser->setFont(KGlobalSettings::fixedFont());
        licenseBrowser->setLineWrapMode(QTextEdit::NoWrap);
        licenseBrowser->setText(aboutData->license());
        
        tabWidget->addTab(licenseBrowser, i18n("&License Agreement"));
    }

    QVBoxLayout *mainLayout = new QVBoxLayout;
         
    QHBoxLayout *titleLayout = new QHBoxLayout;
    titleLayout->setMargin(3);
    titleLayout->setSpacing(KDialog::spacingHint());
    titleLayout->addWidget(iconLabel,0,Qt::AlignVCenter);
    titleLayout->addWidget(headerLabel,0,Qt::AlignVCenter);
    titleLayout->addStretch(1);

    titleLabel->setLayout(titleLayout);

    mainLayout->addWidget(titleLabel);
    mainLayout->addWidget(tabWidget);
    mainLayout->setMargin(0);

    QWidget *mainWidget = new QWidget;
    mainWidget->setLayout(mainLayout);

    setMainWidget(mainWidget);
}

#include "kaboutapplicationdialog.moc"
