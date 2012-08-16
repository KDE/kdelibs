/* This file is part of the KDE libraries
   Copyright (C) 2007 Urs Wolfer <uwolfer at kde.org>
   Copyright (C) 2008 Friedrich W. H. Kossebau <kossebau@kde.org>
   Copyright (C) 2010 Teo Mrnjavac <teo@kde.org>

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

#include "kaboutapplicationdialog.h"

#include "kaboutapplicationpersonmodel_p.h"
#include "kaboutapplicationpersonlistview_p.h"
#include "kaboutapplicationpersonlistdelegate_p.h"
#include "kdeversion.h"
#include "kiconloader.h"
#include "kapplication.h"
#include "kglobalsettings.h"
#include "ktextbrowser.h"
#include "ktitlewidget.h"

#include <kaboutdata.h>
#include <kglobal.h>
#include <klocalizedstring.h>

#include <QLabel>
#include <QLayout>
#include <QPushButton>
#include <QScrollBar>
#include <QTabWidget>

class KAboutApplicationDialog::Private
{
public:
    Private(KAboutApplicationDialog *parent)
        : q(parent),
          aboutData(0)
    {}

    void init( const KAboutData *aboutData, Options opt );

    void _k_showLicense( const QString &number );

    KAboutApplicationDialog *q;

    const KAboutData *aboutData;
};

KAboutApplicationDialog::KAboutApplicationDialog(const KAboutData *aboutData, QWidget *parent)
  : KDialog(parent),
    d(new Private(this))
{
    d->init( aboutData, NoOptions );
}

KAboutApplicationDialog::KAboutApplicationDialog(const KAboutData *aboutData, Options opt, QWidget *parent)
  : KDialog(parent),
    d(new Private(this))
{
    d->init( aboutData, opt );
}

void KAboutApplicationDialog::Private::init( const KAboutData *ad, Options opt )
{
    if (ad == 0)
        ad = KGlobal::mainComponent().aboutData();

    aboutData = ad;

    if (!aboutData) {
        QLabel *errorLabel = new QLabel(i18n("<qt>No information available.<br />"
                                             "The supplied KAboutData object does not exist.</qt>"), q);

        errorLabel->setTextInteractionFlags(Qt::TextBrowserInteraction);
        q->setMainWidget(errorLabel);
        return;
    }

    q->setPlainCaption(i18n("About %1", aboutData->programName()));
    q->setButtons(KDialog::Close);
    q->setDefaultButton(KDialog::Close);
    q->setModal(false);

    //Set up the title widget...
    KTitleWidget *titleWidget = new KTitleWidget(q);

    QIcon windowIcon;
    if (!aboutData->programIconName().isEmpty()) {
        windowIcon = KDE::icon(aboutData->programIconName());
    } else {
        windowIcon = qApp->windowIcon();
    }
    titleWidget->setPixmap(windowIcon.pixmap(64, 64), KTitleWidget::ImageLeft);
    if (aboutData->programLogo().canConvert<QPixmap>())
        titleWidget->setPixmap(aboutData->programLogo().value<QPixmap>(), KTitleWidget::ImageLeft);
    else if (aboutData->programLogo().canConvert<QImage>())
        titleWidget->setPixmap(QPixmap::fromImage(aboutData->programLogo().value<QImage>()), KTitleWidget::ImageLeft);

    if ( opt & HideKdeVersion )
        titleWidget->setText(i18n("<html><font size=\"5\">%1</font><br /><b>Version %2</b><br />&nbsp;</html>",
                                  aboutData->programName(), aboutData->version()));
    else
        titleWidget->setText(i18nc("Program name, version and KDE platform version; do not translate 'Development Platform'",
                                   "<html><font size=\"5\">%1</font><br /><b>Version %2</b><br />Using KDE Development Platform %3</html>",
                                   aboutData->programName(), aboutData->version(), QString(KDE_VERSION_STRING)));

    //Then the tab bar...
    QTabWidget *tabWidget = new QTabWidget;
    tabWidget->setUsesScrollButtons(false);

    //Set up the first page...
    QString aboutPageText = aboutData->shortDescription() + '\n';

    if (!aboutData->otherText().isEmpty())
        aboutPageText += '\n' + aboutData->otherText() + '\n';

    if (!aboutData->copyrightStatement().isEmpty())
        aboutPageText += '\n' + aboutData->copyrightStatement() + '\n';

    if (!aboutData->homepage().isEmpty())
        aboutPageText += '\n' + QString("<a href=\"%1\">%1</a>").arg(aboutData->homepage()) + '\n';
    aboutPageText = aboutPageText.trimmed();

    QLabel *aboutLabel = new QLabel;
    aboutLabel->setWordWrap(true);
    aboutLabel->setOpenExternalLinks(true);
    aboutLabel->setText(aboutPageText.replace('\n', "<br />"));
    aboutLabel->setTextInteractionFlags(Qt::TextBrowserInteraction);

    QVBoxLayout *aboutLayout = new QVBoxLayout;
    aboutLayout->addStretch();
    aboutLayout->addWidget(aboutLabel);

    const int licenseCount = aboutData->licenses().count();
    for (int i = 0; i < licenseCount; ++i) {
        const KAboutLicense &license = aboutData->licenses().at(i);

        QLabel *showLicenseLabel = new QLabel;
        showLicenseLabel->setText(QString("<a href=\"%1\">%2</a>").arg(QString::number(i),
                                                                       i18n("License: %1",
                                                                            license.name(KAboutData::FullName))));
        showLicenseLabel->setTextInteractionFlags(Qt::TextBrowserInteraction);
        connect(showLicenseLabel, SIGNAL(linkActivated(QString)), q, SLOT(_k_showLicense(QString)));

        aboutLayout->addWidget(showLicenseLabel);
    }

    aboutLayout->addStretch();

    QWidget *aboutWidget = new QWidget(q);
    aboutWidget->setLayout(aboutLayout);

    tabWidget->addTab(aboutWidget, i18n("&About"));

    //Palette needed at least for translators...
    QPalette transparentBackgroundPalette;
    transparentBackgroundPalette.setColor(QPalette::Base, Qt::transparent);
    transparentBackgroundPalette.setColor(QPalette::Text, transparentBackgroundPalette.color(QPalette::WindowText));

    //And here we go, authors page...
    const int authorCount = aboutData->authors().count();
    if (authorCount) {
        QWidget *authorWidget = new QWidget( q );
        QVBoxLayout *authorLayout = new QVBoxLayout( authorWidget );
        authorLayout->setMargin( 0 );

        if (!aboutData->customAuthorTextEnabled() || !aboutData->customAuthorRichText().isEmpty()) {
            QLabel *bugsLabel = new QLabel( authorWidget );
            bugsLabel->setContentsMargins( 4, 2, 0, 4 );
            bugsLabel->setOpenExternalLinks( true );
            if (!aboutData->customAuthorTextEnabled()) {
                if (aboutData->bugAddress().isEmpty() || aboutData->bugAddress() == "submit@bugs.kde.org")
                    bugsLabel->setText( i18n("Please use <a href=\"http://bugs.kde.org\">http://bugs.kde.org</a> to report bugs.\n") );
                else {
                    if( ( aboutData->authors().count() == 1 ) &&
                        ( aboutData->authors().first().emailAddress() == aboutData->bugAddress() ) ) {
                        bugsLabel->setText( i18n("Please report bugs to <a href=\"mailto:%1\">%2</a>.\n",
                                              aboutData->authors().first().emailAddress(),
                                              aboutData->authors().first().emailAddress() ) );
                    }
                    else {
                        bugsLabel->setText( i18n("Please report bugs to <a href=\"mailto:%1\">%2</a>.\n",
                                              aboutData->bugAddress(), aboutData->bugAddress()));
                    }
                }
            }
            else
                bugsLabel->setText( aboutData->customAuthorRichText() );
            bugsLabel->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Minimum );
            authorLayout->addWidget( bugsLabel );
        }

        KDEPrivate::KAboutApplicationPersonModel *authorModel =
                new KDEPrivate::KAboutApplicationPersonModel( aboutData->authors(),
                                                              aboutData->ocsProviderUrl(),
                                                              authorWidget );

        KDEPrivate::KAboutApplicationPersonListView *authorView =
                new KDEPrivate::KAboutApplicationPersonListView( authorWidget );

        KDEPrivate::KAboutApplicationPersonListDelegate *authorDelegate =
                new KDEPrivate::KAboutApplicationPersonListDelegate( authorView, authorView );

        authorView->setModel( authorModel );
        authorView->setItemDelegate( authorDelegate );
        authorView->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding );
        authorLayout->addWidget( authorView );

        QString authorPageTitle = QString( ( authorCount == 1 ) ? i18n("A&uthor") : i18n("A&uthors") );
        tabWidget->addTab( authorWidget, authorPageTitle );
    }

    //And credits page...
    const int creditsCount = aboutData->credits().count();
    if (creditsCount) {
        QWidget *creditWidget = new QWidget( q );
        QVBoxLayout *creditLayout = new QVBoxLayout( creditWidget );
        creditLayout->setMargin( 0 );

        KDEPrivate::KAboutApplicationPersonModel *creditModel =
                new KDEPrivate::KAboutApplicationPersonModel( aboutData->credits(),
                                                              aboutData->ocsProviderUrl(),
                                                              creditWidget );

        KDEPrivate::KAboutApplicationPersonListView *creditView =
                new KDEPrivate::KAboutApplicationPersonListView( creditWidget );

        KDEPrivate::KAboutApplicationPersonListDelegate *creditDelegate =
                new KDEPrivate::KAboutApplicationPersonListDelegate( creditView, creditView );

        creditView->setModel( creditModel );
        creditView->setItemDelegate( creditDelegate );
        creditView->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding );
        creditLayout->addWidget( creditView );

        tabWidget->addTab( creditWidget, i18n("&Thanks To"));
    }

    //Finally, the optional translators page...
    if ( !( opt & HideTranslators ) ) {
        const int translatorsCount = aboutData->translators().count();
        if( translatorsCount ) {
            QWidget *translatorWidget = new QWidget( q );
            QVBoxLayout *translatorLayout = new QVBoxLayout( translatorWidget );
            translatorLayout->setMargin( 0 );

            KDEPrivate::KAboutApplicationPersonModel *translatorModel =
                    new KDEPrivate::KAboutApplicationPersonModel( aboutData->translators(),
                                                                  aboutData->ocsProviderUrl(),
                                                                  translatorWidget );

            KDEPrivate::KAboutApplicationPersonListView *translatorView =
                    new KDEPrivate::KAboutApplicationPersonListView( translatorWidget );

            KDEPrivate::KAboutApplicationPersonListDelegate *translatorDelegate =
                    new KDEPrivate::KAboutApplicationPersonListDelegate( translatorView, translatorView );

            translatorView->setModel( translatorModel );
            translatorView->setItemDelegate( translatorDelegate );
            translatorView->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding );
            translatorLayout->addWidget( translatorView );

            QString aboutTranslationTeam = KAboutData::aboutTranslationTeam();
            if( !aboutTranslationTeam.isEmpty() ) {
                QLabel *translationTeamLabel = new QLabel( translatorWidget );
                translationTeamLabel->setContentsMargins( 4, 2, 4, 4 );
                translationTeamLabel->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Minimum );
                translationTeamLabel->setWordWrap( true );
                translationTeamLabel->setText( aboutTranslationTeam );
                translatorLayout->addWidget( translationTeamLabel );
                //TODO: this could be displayed as a view item to save space
            }

            tabWidget->addTab( translatorWidget, i18n("T&ranslation"));
        }
    }

    //And we jam everything together in a layout...
    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(titleWidget);
    mainLayout->addWidget(tabWidget);
    mainLayout->setMargin(0);

    QWidget *mainWidget = new QWidget;
    mainWidget->setLayout(mainLayout);

    q->setMainWidget(mainWidget);
}

KAboutApplicationDialog::~KAboutApplicationDialog()
{
    delete d;
    // The delegate wants to be deleted before the items it created, otherwise
    // complains bitterly about it
    qDeleteAll(findChildren<KWidgetItemDelegate*>());
}

void KAboutApplicationDialog::Private::_k_showLicense( const QString &number )
{
    KDialog *dialog = new KDialog(q);

    dialog->setCaption(i18n("License Agreement"));
    dialog->setButtons(KDialog::Close);
    dialog->setDefaultButton(KDialog::Close);

    const QFont font = KGlobalSettings::fixedFont();
    QFontMetrics metrics(font);

    const QString licenseText = aboutData->licenses().at(number.toInt()).text();
    KTextBrowser *licenseBrowser = new KTextBrowser;
    licenseBrowser->setFont(font);
    licenseBrowser->setLineWrapMode(QTextEdit::NoWrap);
    licenseBrowser->setText(licenseText);

    dialog->setMainWidget(licenseBrowser);

    // try to set up the dialog such that the full width of the
    // document is visible without horizontal scroll-bars being required
    const qreal idealWidth = licenseBrowser->document()->idealWidth() + (2 * dialog->marginHint())
        + licenseBrowser->verticalScrollBar()->width() * 2;

    // try to allow enough height for a reasonable number of lines to be shown
    const int idealHeight = metrics.height() * 30;

    dialog->setInitialSize(dialog->sizeHint().expandedTo(QSize((int)idealWidth,idealHeight)));
    dialog->show();
}

#include "moc_kaboutapplicationdialog.cpp"
