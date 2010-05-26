/*
    Copyright (c) 2009, 2010 David Faure <faure@kde.org>

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
    // Mapping to KDialog button codes
    static const KDialog::ButtonCode Save = KDialog::Yes;
    static const KDialog::ButtonCode OpenDefault = KDialog::User2;
    static const KDialog::ButtonCode OpenWith = KDialog::User1;
    static const KDialog::ButtonCode Cancel = KDialog::Cancel;
        
    BrowserOpenOrSaveQuestionPrivate(QWidget* parent, const KUrl& url, const QString& mimeType)
        : KDialog(parent), url(url), mimeType(mimeType),
          features(0)
    {
        // Use askSave or askEmbedOrSave from filetypesrc
        dontAskConfig = KSharedConfig::openConfig("filetypesrc", KConfig::NoGlobals);

        setCaption(url.host());
        setButtons(Save | OpenDefault | OpenWith | Cancel);
        setObjectName("questionYesNoCancel");
        setButtonGuiItem(Save, KStandardGuiItem::saveAs());
        setButtonGuiItem(Cancel, KStandardGuiItem::cancel());
        setDefaultButton(Save);

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

        fileNameLabel = new QLabel(mainWidget());
        fileNameLabel->hide();
        textVLayout->addWidget(fileNameLabel);

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
        dontAskAgainCheckBox->setText(i18nc("@label:checkbox", "Remember action for files of this type"));
        mainLayout->addWidget(dontAskAgainCheckBox);
    }

    bool autoEmbedMimeType(int flags);

    int executeDialog(const QString& dontShowAgainName)
    {
        KConfigGroup cg(dontAskConfig, "Notification Messages"); // group name comes from KMessageBox
        const QString dontAsk = cg.readEntry(dontShowAgainName, QString()).toLower();
        if (dontAsk == "yes" || dontAsk == "true") {
            return Save;
        } else if (dontAsk == "no" || dontAsk == "false") {
            return OpenDefault;
        }

        KNotification::event("messageQuestion", // why does KMessageBox uses Information for questionYesNoCancel?
                             questionLabel->text(), // also include mimetype?
                             QPixmap(),
                             window());
        const int result = exec();

        if (dontAskAgainCheckBox->isChecked()) {
            cg.writeEntry(dontShowAgainName, result == Save);
            cg.sync();
        }
        return result;
    }

    void showService(KService::Ptr selectedService)
    {
        KGuiItem openItem(i18nc("@label:button", "&Open with %1", selectedService->name()), selectedService->icon());
        setButtonGuiItem(OpenWith, openItem);
    }

    KUrl url;
    QString mimeType;
    KMimeType::Ptr mime;
    KService::Ptr selectedService;
    KSqueezedTextLabel* questionLabel;
    BrowserOpenOrSaveQuestion::Features features;
    QLabel* fileNameLabel;

protected:
    virtual void slotButtonClicked(int buttonId)
    {
        if (buttonId != OpenDefault)
            selectedService = 0;
        KPushButton* button = KDialog::button(KDialog::ButtonCode(buttonId));
        if (button && !button->menu()) {
            done(buttonId);
        }
    }
private:
    QCheckBox* dontAskAgainCheckBox;
    KSharedConfig::Ptr dontAskConfig;

public Q_SLOTS:
    void slotAppSelected(QAction* action)
    {
        selectedService = action->data().value<KService::Ptr>();
        //showService(selectedService);
        done(OpenDefault);
    }
};


BrowserOpenOrSaveQuestion::BrowserOpenOrSaveQuestion(QWidget* parent, const KUrl& url, const QString& mimeType)
    : d(new BrowserOpenOrSaveQuestionPrivate(parent, url, mimeType))
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
    d->showButton(BrowserOpenOrSaveQuestionPrivate::OpenWith, false);

    KGuiItem openWithDialogItem(i18nc("@label:button", "&Open with..."), "document-open");

    // I thought about using KFileItemActions, but we don't want a submenu, nor the slots....
    // and we want no menu at all if there's only one offer.
    // TODO: we probably need a setTraderConstraint(), to exclude the current application?
    const KService::List apps = KFileItemActions::associatedApplications(QStringList() << d->mimeType,
                                                                         QString() /* TODO trader constraint */);
    if (apps.isEmpty()) {
        d->setButtonGuiItem(BrowserOpenOrSaveQuestionPrivate::OpenDefault, openWithDialogItem);
    } else {
        KService::Ptr offer = apps.first();
        KGuiItem openItem(i18nc("@label:button", "&Open with %1", offer->name()), offer->icon());
        d->setButtonGuiItem(BrowserOpenOrSaveQuestionPrivate::OpenDefault, openItem);
        if (d->features & ServiceSelection) {
            // OpenDefault shall use this service
            d->selectedService = apps.first();
            d->showButton(BrowserOpenOrSaveQuestionPrivate::OpenWith, true);
            KMenu* menu = new KMenu(d);
            if (apps.count() > 1) {
                // Provide an additional button with a menu of associated apps
                KGuiItem openWithItem(i18nc("@label:button", "&Open with"), "document-open");
                d->setButtonGuiItem(BrowserOpenOrSaveQuestionPrivate::OpenWith, openWithItem);
                d->setButtonMenu(BrowserOpenOrSaveQuestionPrivate::OpenWith, menu, KDialog::InstantPopup);
                QObject::connect(menu, SIGNAL(triggered(QAction*)), d, SLOT(slotAppSelected(QAction*)));
                for (KService::List::const_iterator it = apps.begin(); it != apps.end(); ++it) {
                    KAction* act = createAppAction(*it, d);
                    menu->addAction(act);
                }
                KAction* openWithDialogAction = new KAction(d);
                openWithDialogAction->setIcon(KIcon("document-open"));
                openWithDialogAction->setText(openWithDialogItem.text());
                menu->addAction(openWithDialogAction);
            } else {
                // Only one associated app, already offered by the other menu -> add "Open With..." button
                d->setButtonGuiItem(BrowserOpenOrSaveQuestionPrivate::OpenWith, openWithDialogItem);
            }
        } else {
            kDebug() << "Not using new feature ServiceSelection; port the caller to BrowserOpenOrSaveQuestion::setFeature(ServiceSelection)";
            //kDebug() << kBacktrace();
        }
    }

    // KEEP IN SYNC with kdebase/runtime/keditfiletype/filetypedetails.cpp!!!
    const QString dontAskAgain = QLatin1String("askSave") + d->mimeType;

    const int choice = d->executeDialog(dontAskAgain);
    return choice == BrowserOpenOrSaveQuestionPrivate::Save ? Save
        : (choice == BrowserOpenOrSaveQuestionPrivate::Cancel ? Cancel : Open);
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
            mimeType.startsWith(QLatin1String("image")) ||
            mime->is("multipart/x-mixed-replace") ||
            mime->is("multipart/replace")))
        return true;
    return false;
}

BrowserOpenOrSaveQuestion::Result BrowserOpenOrSaveQuestion::askEmbedOrSave(int flags)
{
    if (d->autoEmbedMimeType(flags))
        return Embed;

    // don't use KStandardGuiItem::open() here which has trailing ellipsis!
    d->setButtonGuiItem(BrowserOpenOrSaveQuestionPrivate::OpenDefault, KGuiItem(i18nc("@label:button", "&Open"), "document-open"));
    d->showButton(BrowserOpenOrSaveQuestionPrivate::OpenWith, false);

    d->questionLabel->setText(i18nc("@info", "Open '%1'?", d->url.pathOrUrl()));

    const QString dontAskAgain = QLatin1String("askEmbedOrSave")+ d->mimeType; // KEEP IN SYNC!!!
    // SYNC SYNC SYNC SYNC SYNC SYNC SYNC SYNC SYNC SYNC SYNC SYNC SYNC SYNC

    const int choice = d->executeDialog(dontAskAgain);
    return choice == BrowserOpenOrSaveQuestionPrivate::Save ? Save
        : (choice == BrowserOpenOrSaveQuestionPrivate::Cancel ? Cancel : Embed);
}

void BrowserOpenOrSaveQuestion::setFeatures(Features features)
{
    d->features = features;
}

void BrowserOpenOrSaveQuestion::setSuggestedFileName(const QString& suggestedFileName)
{
    if (!suggestedFileName.isEmpty()) {
        d->fileNameLabel->setText(i18nc("@label File name", "Name: %1", suggestedFileName));
        d->fileNameLabel->setWhatsThis(i18nc("@info:whatsthis", "This is the file name suggested by the server"));
        d->fileNameLabel->show();
    }
}

#include "browseropenorsavequestion.moc"
