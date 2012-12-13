/* This file is part of the KDE libraries
   Copyright (C) 2007 Urs Wolfer <uwolfer at kde.org>

   Parts of this class have been take from the KAboutKDE class, which was
   Copyright (C) 2000 Espen Sand <espen@kde.org>

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

#include "kaboutkdedialog_p.h"

#include <QDialogButtonBox>
#include <QFrame>
#include <QLabel>
#include <QLayout>
#include <QTabWidget>

#include <kdeversion.h>
#include <kglobalsettings.h>
#include <klocalizedstring.h>
#include <ktitlewidget.h>
#include <kiconloader.h>

namespace KDEPrivate {


KAboutKdeDialog::KAboutKdeDialog(QWidget *parent)
  : QDialog(parent),
    d( 0 )
{
    setWindowTitle(i18n("About KDE"));

    KTitleWidget *titleWidget = new KTitleWidget(this);
    titleWidget->setText(i18n("<html><font size=\"5\">KDE - Be Free!</font><br /><b>Platform Version %1</b></html>",
                         QString(KDE_VERSION_STRING)));
    titleWidget->setPixmap(KDE::icon("kde").pixmap(48), KTitleWidget::ImageLeft);

    QLabel *about = new QLabel;
    about->setMargin(10);
    about->setAlignment(Qt::AlignTop);
    about->setWordWrap(true);
    about->setOpenExternalLinks(true);
    about->setTextInteractionFlags(Qt::TextBrowserInteraction);
    about->setText(i18n("<html>"
        "<b>KDE</b> is a world-wide network of software engineers, artists, writers, translators and facilitators "
        "who are committed to <a href=\"%1\">Free Software</a> development. "
        "This community has created hundreds of Free Software applications as part of the KDE "
        "Development Platform and KDE Software Distribution.<br /><br />"
        "KDE is a cooperative enterprise in which no single entity controls the "
        "efforts or products of KDE to the exclusion of others. Everyone is welcome to join and "
        "contribute to KDE, including you.<br /><br />"
        "Visit <a href=\"%2\">%2</a> for "
        "more information about the KDE community and the software we produce.</html>",
        QLatin1String("http://www.gnu.org/philosophy/free-sw.html"),
        QLatin1String("http://www.kde.org/")));

    QLabel *report = new QLabel;
    report->setMargin(10);
    report->setAlignment(Qt::AlignTop);
    report->setWordWrap(true);
    report->setOpenExternalLinks(true);
    report->setTextInteractionFlags(Qt::TextBrowserInteraction);
    report->setText(i18n("<html>"
        "Software can always be improved, and the KDE team is ready to "
        "do so. However, you - the user - must tell us when "
        "something does not work as expected or could be done better.<br /><br />"
        "KDE has a bug tracking system. Visit "
        "<a href=\"%1\">%1</a> or "
        "use the \"Report Bug...\" dialog from the \"Help\" menu to report bugs.<br /><br />"
        "If you have a suggestion for improvement then you are welcome to use "
        "the bug tracking system to register your wish. Make sure you use the "
        "severity called \"Wishlist\".</html>",
        QLatin1String("https://bugs.kde.org/")));

    QLabel *join = new QLabel;
    join->setMargin(10);
    join->setAlignment(Qt::AlignTop);
    join->setWordWrap(true);
    join->setOpenExternalLinks(true);
    join->setTextInteractionFlags(Qt::TextBrowserInteraction);
    join->setText(i18n("<html>"
        "You do not have to be a software developer to be a member of the "
        "KDE team. You can join the national teams that translate "
        "program interfaces. You can provide graphics, themes, sounds, and "
        "improved documentation. You decide!"
        "<br /><br />"
        "Visit "
        "<a href=\"%1\">%1</a> "
        "for information on some projects in which you can participate."
        "<br /><br />"
        "If you need more information or documentation, then a visit to "
        "<a href=\"%2\">%2</a> "
        "will provide you with what you need.</html>",
        QLatin1String("http://www.kde.org/community/getinvolved/"),
        QLatin1String("http://techbase.kde.org/")));

    QLabel *support = new QLabel;
    support->setMargin(10);
    support->setAlignment(Qt::AlignTop);
    support->setWordWrap(true);
    support->setOpenExternalLinks(true);
    support->setTextInteractionFlags(Qt::TextBrowserInteraction);
    support->setText(i18n("<html>"
        "KDE software is and will always be available free of charge, however creating it is not free.<br /><br />"
        "To support development the KDE community has formed the KDE e.V., a non-profit organization "
        "legally founded in Germany. KDE e.V. represents the KDE community in legal and financial matters. "
        "See <a href=\"%1\">%1</a>"
        " for information on KDE e.V.<br /><br />"
        "KDE benefits from many kinds of contributions, including financial. "
        "We use the funds to reimburse members and others for expenses "
        "they incur when contributing. Further funds are used for legal "
        "support and organizing conferences and meetings. <br /> <br />"
        "We would like to encourage you to support our efforts with a "
        "financial donation, using one of the ways described at "
        "<a href=\"%2\">%2</a>."
        "<br /><br />Thank you very much in advance for your support.</html>",
        QLatin1String("http://ev.kde.org/"),
        QLatin1String("http://www.kde.org/community/donations/")) + "<br /><br />"); // FIXME: ugly <br /> at the end...

    QTabWidget *tabWidget = new QTabWidget;
    tabWidget->setUsesScrollButtons(false);
    tabWidget->addTab(about, i18nc("About KDE","&About"));
    tabWidget->addTab(report, i18n("&Report Bugs or Wishes"));
    tabWidget->addTab(join, i18n("&Join KDE"));
    tabWidget->addTab(support, i18n("&Support KDE"));

    QLabel *image = new QLabel;
    image->setPixmap(QStandardPaths::locate(QStandardPaths::GenericDataLocation, "kdeui/pics/aboutkde.png"));

    QHBoxLayout *midLayout = new QHBoxLayout;
    midLayout->addWidget(image);
    midLayout->addWidget(tabWidget);

    QDialogButtonBox *buttonBox = new QDialogButtonBox;
    buttonBox->setStandardButtons(QDialogButtonBox::Close);
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(titleWidget);
    mainLayout->addLayout(midLayout);
    mainLayout->addWidget(buttonBox);

    setLayout(mainLayout);
}

}

