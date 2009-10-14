/*
    Copyright (c) 2009 David Faure <faure@kde.org>

    This library is free software; you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation; either version 2 of the License or ( at
    your option ) version 3 or, at the discretion of KDE e.V. ( which shall
    act as a proxy as in section 14 of the GPLv3 ), any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "browseropenorsavequestion.h"
#include <kdebug.h>
#include <kaction.h>
#include <kfileitemactions.h>
#include <kpushbutton.h>
#include <kmenu.h>
#include <ksqueezedtextlabel.h>
#include <knotification.h>
#include <kdialog.h>
#include <kmimetypetrader.h>
#include <kstandardguiitem.h>
#include <kguiitem.h>
#include <kmessagebox.h>
#include <kmimetype.h>
#include <QStyle>
#include <QStyleOption>
#include <QVBoxLayout>
#include <QCheckBox>
#include <QLabel>

using namespace KParts;
Q_DECLARE_METATYPE(KService::Ptr)

class KParts::BrowserOpenOrSaveQuestionPrivate : public KDialog
{
    Q_OBJECT
public:
    BrowserOpenOrSaveQuestionPrivate(QWidget* parent, const KUrl& url, const QString& mimeType, const QString& suggestedFileName)
        : KDialog(parent), url(url), mimeType(mimeType), suggestedFileName(suggestedFileName)
    {
        // Use askSave or askEmbedOrSave from filetypesrc
        dontAskConfig = KSharedConfig::openConfig("filetypesrc", KConfig::NoGlobals);

        setCaption(url.host());
        setButtons(KDialog::Yes | KDialog::No | KDialog::Cancel);
        setObjectName("questionYesNoCancel");
        setButtonGuiItem(KDialog::Cancel, KStandardGuiItem::cancel());
        setDefaultButton(KDialog::Yes);

        QVBoxLayout *mainLayout = new QVBoxLayout(mainWidget());
        mainLayout->setSpacing(KDialog::spacingHint() * 2); // provide extra spacing
        mainLayout->setMargin(0);

        QHBoxLayout *hLayout = new QHBoxLayout();
        hLayout->setMargin(0);
        hLayout->setSpacing(-1); // use default spacing
        mainLayout->addLayout(hLayout, 5);

        QLabel *iconLabel = new QLabel(mainWidget());
        QStyleOption option;
        option.initFrom(this);
        KIcon icon("dialog-information");
        iconLabel->setPixmap(icon.pixmap(style()->pixelMetric(QStyle::PM_MessageBoxIconSize, &option, this)));

        hLayout->addWidget(iconLabel, 0, Qt::AlignCenter);
        hLayout->addSpacing(KDialog::spacingHint());

        QVBoxLayout* textVLayout = new QVBoxLayout;
        questionLabel = new KSqueezedTextLabel(mainWidget());
        textVLayout->addWidget(questionLabel);

        if (!suggestedFileName.isEmpty()) {
            QLabel* label = new QLabel(mainWidget());
            label->setText(i18nc("@label File name", "Name: %1", suggestedFileName));
            label->setWhatsThis(i18nc("@info:whatsthis", "This is the file name suggested by the server"));
            textVLayout->addWidget(label);
        }

        mime = KMimeType::mimeType(mimeType, KMimeType::ResolveAliases);
        QString mimeDescription = mimeType;
        if (mime) { // The mime-type is known so display the comment instead of mime-type
            mimeDescription = mime->comment();
        }
        QLabel* mimeTypeLabel = new QLabel(mainWidget());
        mimeTypeLabel->setText(i18nc("@label Type of file", "Type: %1", mimeDescription));
        textVLayout->addWidget(mimeTypeLabel);

        hLayout->addLayout(textVLayout,5);

        mainLayout->addStretch(15);
        dontAskAgainCheckBox = new QCheckBox(mainWidget());
        dontAskAgainCheckBox->setText(i18nc("@label:checkbox", "Do not ask again for files of this type"));
        mainLayout->addWidget(dontAskAgainCheckBox);

        showButtonSeparator(true);
    }

    bool autoEmbedMimeType(int flags);

    int execute(const QString& dontShowAgainName)
    {
        KConfigGroup cg(dontAskConfig, "Notification Messages"); // group name comes from KMessageBox
        const QString dontAsk = cg.readEntry(dontShowAgainName, QString()).toLower();
        if (dontAsk == "yes" || dontAsk == "true") {
            return KDialog::Yes; // i.e. "Save"
        } else if (dontAsk == "no" || dontAsk == "false") {
            return KDialog::No; // i.e. "Open"
        }

        KNotification::event("messageQuestion", // why does KMessageBox uses Information for questionYesNoCancel?
                             questionLabel->text(), // also include mimetype?
                             QPixmap(),
                             window());
        const int result = exec();

        if (dontAskAgainCheckBox->isChecked()) {
            cg.writeEntry(dontShowAgainName, result == KDialog::Yes);
            cg.sync();
        }
        return result;
    }

    void showService(KService::Ptr selectedService)
    {
        KGuiItem openItem(i18nc("@label:button", "&Open with %1", selectedService->name()), selectedService->icon());
        setButtonGuiItem(KDialog::No, openItem);
    }

    KUrl url;
    QString mimeType;
    KMimeType::Ptr mime;
    QString suggestedFileName;
    KService::Ptr selectedService;
    KSqueezedTextLabel* questionLabel;

private:
    QCheckBox* dontAskAgainCheckBox;
    KSharedConfig::Ptr dontAskConfig;

public Q_SLOTS:
    void slotAppSelected(QAction* action)
    {
        selectedService = action->data().value<KService::Ptr>();
        //showService(selectedService);
        done(KDialog::No);
    }
};


BrowserOpenOrSaveQuestion::BrowserOpenOrSaveQuestion(QWidget* parent, const KUrl& url, const QString& mimeType, const QString& suggestedFileName)
    : d(new BrowserOpenOrSaveQuestionPrivate(parent, url, mimeType, suggestedFileName))
{
}

BrowserOpenOrSaveQuestion::~BrowserOpenOrSaveQuestion()
{
    delete d;
}

static KAction* createAppAction(const KService::Ptr& service, QObject* parent)
{
    QString actionName(service->name().replace('&', "&&"));
    actionName = i18nc("@action:inmenu", "Open &with %1", actionName);

    KAction *act = new KAction(parent);
    act->setIcon(KIcon(service->icon()));
    act->setText(actionName);
    act->setData(QVariant::fromValue(service));
    return act;
}

BrowserOpenOrSaveQuestion::Result BrowserOpenOrSaveQuestion::askOpenOrSave()
{
    d->questionLabel->setText(i18nc("@info", "Open '%1'?", d->url.pathOrUrl()));

    // I thought about using KFileItemActions, but we don't want a submenu, nor the slots....
    // and we want no menu at all if there's only one offer.
    // TODO: we probably need a setTraderConstraint(), to exclude the current application?
    bool first = true;
    const KService::List apps = KFileItemActions::associatedApplications(QStringList() << d->mimeType,
                                                                         QString() /* TODO trader constraint */);
    if (apps.isEmpty()) {
        KGuiItem openItem(i18nc("@label:button", "&Open with..."), "system-run");
        d->setButtonGuiItem(KDialog::No, openItem);
    } else if (apps.count() == 1) {
        KService::Ptr offer = apps.first();
        KGuiItem openItem(i18nc("@label:button", "&Open with %1", offer->name()), offer->icon());
        d->setButtonGuiItem(KDialog::No, openItem);
    } else {
        KPushButton* openButton = d->button(KDialog::No);
        KMenu* menu = new KMenu(openButton);
        openButton->setDelayedMenu(menu);
        QObject::connect(menu, SIGNAL(triggered(QAction*)), d, SLOT(slotAppSelected(QAction*)));
        for (KService::List::const_iterator it = apps.begin(); it != apps.end(); ++it) {
            KAction* act = createAppAction(*it, d);
            menu->addAction(act);
            if (first) {
                first = false;
                d->showService(*it); // sets the guiitem
            }
        }
    }

    d->setButtonGuiItem(KDialog::Yes, KStandardGuiItem::saveAs());

    // KEEP IN SYNC with kdebase/runtime/keditfiletype/filetypedetails.cpp!!!
    const QString dontAskAgain = QLatin1String("askSave") + d->mimeType;

    const int choice = d->execute(dontAskAgain);
    return choice == KDialog::Yes ? Save : (choice == KDialog::No ? Open : Cancel);
}

KService::Ptr BrowserOpenOrSaveQuestion::selectedService() const
{
    return d->selectedService;
}

bool BrowserOpenOrSaveQuestionPrivate::autoEmbedMimeType(int flags)
{
    // SYNC SYNC SYNC SYNC SYNC SYNC SYNC SYNC SYNC SYNC SYNC SYNC SYNC SYNC
    // NOTE: Keep this function in sync with
    // kdebase/runtime/keditfiletype/filetypedetails.cpp
    //       FileTypeDetails::updateAskSave()

    // Don't ask for:
    // - html (even new tabs would ask, due to about:blank!)
    // - dirs obviously (though not common over HTTP :),
    // - images (reasoning: no need to save, most of the time, because fast to see)
    // e.g. postscript is different, because takes longer to read, so
    // it's more likely that the user might want to save it.
    // - multipart/* ("server push", see kmultipart)
    // KEEP IN SYNC!!!
    if (flags != (int)BrowserRun::AttachmentDisposition && mime && (
            mime->is("text/html") ||
            mime->is("application/xml") ||
            mime->is("inode/directory") ||
            mimeType.startsWith("image") ||
            mime->is("multipart/x-mixed-replace") ||
            mime->is("multipart/replace")))
        return true;
    return false;
}

BrowserOpenOrSaveQuestion::Result BrowserOpenOrSaveQuestion::askEmbedOrSave(int flags)
{
    if (d->autoEmbedMimeType(flags))
        return Embed;

    d->setButtonGuiItem(KDialog::Yes, KStandardGuiItem::saveAs());
    // don't use KStandardGuiItem::open() here which has trailing ellipsis!
    d->setButtonGuiItem(KDialog::No, KGuiItem(i18nc("@label:button", "&Open"), "document-open"));

    d->questionLabel->setText(i18nc("@info", "Open '%1'?", d->url.pathOrUrl()));

    const QString dontAskAgain = QLatin1String("askEmbedOrSave")+ d->mimeType; // KEEP IN SYNC!!!

    const int choice = d->execute(dontAskAgain);
    return choice == KDialog::Yes ? Save : (choice == KDialog::No ? Embed : Cancel);
    // SYNC SYNC SYNC SYNC SYNC SYNC SYNC SYNC SYNC SYNC SYNC SYNC SYNC SYNC
}

#include "browseropenorsavequestion.moc"
