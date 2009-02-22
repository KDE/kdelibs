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

#include <QFrame>
#include <QLabel>
#include <QLayout>
#include <QTabWidget>

#include <kdeversion.h>
#include <kglobalsettings.h>
#include <klocale.h>
#include <kstandarddirs.h>
#include <ktitlewidget.h>

namespace KDEPrivate {


KAboutKdeDialog::KAboutKdeDialog(QWidget *parent)
  : KDialog(parent),
    d( 0 )
{
    setPlainCaption(i18n("About KDE"));
    setButtons(KDialog::Close);

    KTitleWidget *titleWidget = new KTitleWidget(this);
    titleWidget->setText(i18n("<html><font size=\"5\">K Desktop Environment</font><br /><b>Version %1</b></html>",
                         QString(KDE_VERSION_STRING)));
    titleWidget->setPixmap(KIcon("kde").pixmap(48), KTitleWidget::ImageLeft);

    QLabel *about = new QLabel;
    about->setMargin(10);
    about->setAlignment(Qt::AlignTop);
    about->setWordWrap(true);
    about->setOpenExternalLinks(true);
    about->setTextInteractionFlags(Qt::TextBrowserInteraction);
    about->setText(i18n("<html>"
        "The <b>K Desktop Environment</b> is written and maintained by the "
        "KDE Team, a world-wide network of software engineers committed to "
        "<a href=\"http://www.gnu.org/philosophy/free-sw.html\">Free Software</a> development.<br /><br />"
        "No single group, company or organization controls the KDE source "
        "code. Everyone is welcome to contribute to KDE.<br /><br />"
        "Visit <a href=\"http://www.kde.org/\">http://www.kde.org</a> for "
        "more information on the KDE project.</html>"));

    QLabel *report = new QLabel;
    report->setMargin(10);
    report->setAlignment(Qt::AlignTop);
    report->setWordWrap(true);
    report->setOpenExternalLinks(true);
    report->setTextInteractionFlags(Qt::TextBrowserInteraction);
    report->setText(i18n("<html>"
        "Software can always be improved, and the KDE Team is ready to "
        "do so. However, you - the user - must tell us when "
        "something does not work as expected or could be done better.<br /><br />"
        "The K Desktop Environment has a bug tracking system. Visit "
        "<a href=\"http://bugs.kde.org/\">http://bugs.kde.org</a> or "
        "use the \"Report Bug...\" dialog from the \"Help\" menu to report bugs.<br /><br />"
        "If you have a suggestion for improvement then you are welcome to use "
        "the bug tracking system to register your wish. Make sure you use the "
        "severity called \"Wishlist\".</html>" ));

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
        "<a href=\"http://www.kde.org/jobs/\">http://www.kde.org/jobs/</a> "
        "for information on some projects in which you can participate."
        "<br /><br />"
        "If you need more information or documentation, then a visit to "
        "<a href=\"http://techbase.kde.org/\">http://techbase.kde.org</a> "
        "will provide you with what you need.</html>"));

    QLabel *support = new QLabel;
    support->setMargin(10);
    support->setAlignment(Qt::AlignTop);
    support->setWordWrap(true);
    support->setOpenExternalLinks(true);
    support->setTextInteractionFlags(Qt::TextBrowserInteraction);
    support->setText(i18n("<html>"
        "KDE is available free of charge, but making it is not free.<br /><br />"
        "Thus, the KDE team formed the KDE e.V., a non-profit organization"
        " legally founded in Tuebingen, Germany. The KDE e.V. represents"
        " the KDE project in legal and financial matters."
        " See <a href=\"http://www.kde-ev.org/\">http://www.kde-ev.org</a>"
        " for information on the KDE e.V.<br /><br />"
        "The KDE team does need financial support. Most of the money is used to "
        "reimburse members and others on expenses they experienced when "
        "contributing to KDE. You are encouraged to support KDE through a financial "
        "donation, using one of the ways described at "
        "<a href=\"http://www.kde.org/support/\">http://www.kde.org/support/</a>."
        "<br /><br />Thank you very much in advance for your support.</html>") + "<br /><br />"); // FIXME: ugly <br /> at the end...

    QTabWidget *tabWidget = new QTabWidget;
    tabWidget->setUsesScrollButtons(false);
    tabWidget->addTab(about, i18nc("About KDE","&About"));
    tabWidget->addTab(report, i18n("&Report Bugs or Wishes"));
    tabWidget->addTab(join, i18n("&Join the KDE Team"));
    tabWidget->addTab(support, i18n("&Support KDE"));

    QLabel *image = new QLabel;
    image->setPixmap(KStandardDirs::locate("data", "kdeui/pics/aboutkde.png"));

    QHBoxLayout *midLayout = new QHBoxLayout;
    midLayout->addWidget(image);
    midLayout->addWidget(tabWidget);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(titleWidget);
    mainLayout->addLayout(midLayout);
    mainLayout->setMargin(0);

    QWidget *mainWidget = new QWidget;
    mainWidget->setLayout(mainLayout);

    setMainWidget(mainWidget);
}

}

#include "kaboutkdedialog_p.moc"
