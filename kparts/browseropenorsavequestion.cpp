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
#include <kconfiggroup.h>
#include <QDebug>
#include <klocalizedstring.h>
#include <kfileitemactions.h>
#include <ksqueezedtextlabel.h>
#include <knotification.h>
#include <kmimetypetrader.h>
#include <kstandardguiitem.h>
#include <kguiitem.h>
#include <kmessagebox.h>
#include <qmimedatabase.h>
#include <QAction>
#include <QDialog>
#include <QDialogButtonBox>
#include <QMenu>
#include <QPushButton>
#include <QStyle>
#include <QStyleOption>
#include <QVBoxLayout>
#include <QCheckBox>
#include <QLabel>

using namespace KParts;
Q_DECLARE_METATYPE(KService::Ptr)

class KParts::BrowserOpenOrSaveQuestionPrivate : public QDialog
{
    Q_OBJECT
public:
    enum {
        Save = QDialog::Accepted,
        OpenDefault = Save + 1,
        OpenWith = OpenDefault + 1,
        Cancel = QDialog::Rejected
    };

    BrowserOpenOrSaveQuestionPrivate(QWidget* parent, const QUrl& url, const QString& mimeType)
        : QDialog(parent), url(url), mimeType(mimeType),
          features(0)
    {
        const int spacingHint = style()->pixelMetric(QStyle::PM_DefaultLayoutSpacing);

        // Use askSave or askEmbedOrSave from filetypesrc
        dontAskConfig = KSharedConfig::openConfig(QStringLiteral("filetypesrc"), KConfig::NoGlobals);

        setWindowTitle(url.host());
        setObjectName(QStringLiteral("questionYesNoCancel"));

        QVBoxLayout *mainLayout = new QVBoxLayout(this);
        mainLayout->setSpacing(spacingHint * 2); // provide extra spacing
        mainLayout->setMargin(0);

        QHBoxLayout *hLayout = new QHBoxLayout();
        hLayout->setMargin(0);
        hLayout->setSpacing(-1); // use default spacing
        mainLayout->addLayout(hLayout, 5);

        QLabel *iconLabel = new QLabel(this);
        QStyleOption option;
        option.initFrom(this);
        QIcon icon = QIcon::fromTheme(QStringLiteral("dialog-information"));
        iconLabel->setPixmap(icon.pixmap(style()->pixelMetric(QStyle::PM_MessageBoxIconSize, &option, this)));

        hLayout->addWidget(iconLabel, 0, Qt::AlignCenter);
        hLayout->addSpacing(spacingHint);

        QVBoxLayout* textVLayout = new QVBoxLayout;
        questionLabel = new KSqueezedTextLabel(this);
        textVLayout->addWidget(questionLabel);

        fileNameLabel = new QLabel(this);
        fileNameLabel->hide();
        textVLayout->addWidget(fileNameLabel);

        QMimeDatabase db;
        mime = db.mimeTypeForName(mimeType);
        QString mimeDescription (mimeType);
        if (mime.isValid()) {
            // Always prefer the mime-type comment over the raw type for display
            mimeDescription = (mime.comment().isEmpty() ? mime.name() : mime.comment());
        }
        QLabel* mimeTypeLabel = new QLabel(this);
        mimeTypeLabel->setText(i18nc("@label Type of file", "Type: %1", mimeDescription));
        mimeTypeLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
        textVLayout->addWidget(mimeTypeLabel);

        hLayout->addLayout(textVLayout,5);

        mainLayout->addStretch(15);
        dontAskAgainCheckBox = new QCheckBox(this);
        dontAskAgainCheckBox->setText(i18nc("@label:checkbox", "Remember action for files of this type"));
        mainLayout->addWidget(dontAskAgainCheckBox);

        buttonBox = new QDialogButtonBox(this);

        saveButton = buttonBox->addButton(QDialogButtonBox::Yes);
        saveButton->setObjectName(QStringLiteral("saveButton"));
        KGuiItem::assign(saveButton, KStandardGuiItem::saveAs());
        saveButton->setDefault(true);

        openDefaultButton = new QPushButton;
        openDefaultButton->setObjectName(QStringLiteral("openDefaultButton"));
        buttonBox->addButton(openDefaultButton, QDialogButtonBox::ActionRole);

        openWithButton = new QPushButton;
        openWithButton->setObjectName(QStringLiteral("openWithButton"));
        buttonBox->addButton(openWithButton, QDialogButtonBox::ActionRole);

        QPushButton *cancelButton = buttonBox->addButton(QDialogButtonBox::Cancel);
        cancelButton->setObjectName(QStringLiteral("cancelButton"));

        connect(saveButton, SIGNAL(clicked()), this, SLOT(slotYesClicked()));
        connect(openDefaultButton, SIGNAL(clicked()), this, SLOT(slotOpenDefaultClicked()));
        connect(openWithButton, SIGNAL(clicked()), this, SLOT(slotOpenWithClicked()));
        connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));

        mainLayout->addWidget(buttonBox);
    }

    bool autoEmbedMimeType(int flags);

    int executeDialog(const QString& dontShowAgainName)
    {
        KConfigGroup cg(dontAskConfig, "Notification Messages"); // group name comes from KMessageBox
        const QString dontAsk = cg.readEntry(dontShowAgainName, QString()).toLower();
        if (dontAsk == QLatin1String("yes") || dontAsk == QLatin1String("true")) {
            return Save;
        } else if (dontAsk == QLatin1String("no") || dontAsk == QLatin1String("false")) {
            return OpenDefault;
        }

        KNotification::event(QStringLiteral("messageQuestion"), // why does KMessageBox uses Information for questionYesNoCancel?
                             questionLabel->text(), // also include mimetype?
                             QPixmap(),
                             window());
        const int result = exec();

        if (dontAskAgainCheckBox->isChecked()) {
            cg.writeEntry(dontShowAgainName, result == BrowserOpenOrSaveQuestion::Save);
            cg.sync();
        }
        return result;
    }

    void showService(KService::Ptr selectedService)
    {
        KGuiItem openItem(i18nc("@label:button", "&Open with %1", selectedService->name()), selectedService->icon());
        KGuiItem::assign(openWithButton, openItem);
    }

    QUrl url;
    QString mimeType;
    QMimeType mime;
    KService::Ptr selectedService;
    KSqueezedTextLabel* questionLabel;
    BrowserOpenOrSaveQuestion::Features features;
    QLabel* fileNameLabel;
    QDialogButtonBox *buttonBox;
    QPushButton *saveButton;
    QPushButton *openDefaultButton;
    QPushButton *openWithButton;

private:
    QCheckBox* dontAskAgainCheckBox;
    KSharedConfig::Ptr dontAskConfig;

public Q_SLOTS:
    virtual void reject()
    {
        selectedService = 0;
        QDialog::reject();
    }

    void slotYesClicked()
    {
        selectedService = 0;
        done(Save);
    }

    void slotOpenDefaultClicked()
    {
        done(OpenDefault);
    }

    void slotOpenWithClicked()
    {
        if (!openWithButton->menu()) {
            selectedService = 0;
            done(OpenWith);
        }
    }

    void slotAppSelected(QAction* action)
    {
        selectedService = action->data().value<KService::Ptr>();
        //showService(selectedService);
        done(OpenDefault);
    }
};


BrowserOpenOrSaveQuestion::BrowserOpenOrSaveQuestion(QWidget* parent, const QUrl& url, const QString& mimeType)
    : d(new BrowserOpenOrSaveQuestionPrivate(parent, url, mimeType))
{
}

BrowserOpenOrSaveQuestion::~BrowserOpenOrSaveQuestion()
{
    delete d;
}

static QAction* createAppAction(const KService::Ptr& service, QObject* parent)
{
    QString actionName(service->name().replace(QLatin1Char('&'), QStringLiteral("&&")));
    actionName = i18nc("@action:inmenu", "Open &with %1", actionName);

    QAction *act = new QAction(parent);
    act->setIcon(QIcon::fromTheme(service->icon()));
    act->setText(actionName);
    act->setData(QVariant::fromValue(service));
    return act;
}

BrowserOpenOrSaveQuestion::Result BrowserOpenOrSaveQuestion::askOpenOrSave()
{
    d->questionLabel->setText(i18nc("@info", "Open '%1'?", d->url.toDisplayString(QUrl::PreferLocalFile)));
    d->questionLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
    d->openWithButton->hide();

    KGuiItem openWithDialogItem(i18nc("@label:button", "&Open with..."), QStringLiteral("document-open"));

    // I thought about using KFileItemActions, but we don't want a submenu, nor the slots....
    // and we want no menu at all if there's only one offer.
    // TODO: we probably need a setTraderConstraint(), to exclude the current application?
    const KService::List apps = KFileItemActions::associatedApplications(QStringList() << d->mimeType,
                                                                         QString() /* TODO trader constraint */);
    if (apps.isEmpty()) {
        KGuiItem::assign(d->openDefaultButton, openWithDialogItem);
    } else {
        KService::Ptr offer = apps.first();
        KGuiItem openItem(i18nc("@label:button", "&Open with %1", offer->name()), offer->icon());
        KGuiItem::assign(d->openDefaultButton, openItem);
        if (d->features & ServiceSelection) {
            // OpenDefault shall use this service
            d->selectedService = apps.first();
            d->openWithButton->show();
            QMenu* menu = new QMenu(d);
            if (apps.count() > 1) {
                // Provide an additional button with a menu of associated apps
                KGuiItem openWithItem(i18nc("@label:button", "&Open with"), QStringLiteral("document-open"));
                KGuiItem::assign(d->openWithButton, openWithItem);
                d->openWithButton->setMenu(menu);
                QObject::connect(menu, SIGNAL(triggered(QAction*)), d, SLOT(slotAppSelected(QAction*)));
                for (KService::List::const_iterator it = apps.begin(); it != apps.end(); ++it) {
                    QAction* act = createAppAction(*it, d);
                    menu->addAction(act);
                }
                QAction* openWithDialogAction = new QAction(d);
                openWithDialogAction->setIcon(QIcon::fromTheme(QStringLiteral("document-open")));
                openWithDialogAction->setText(openWithDialogItem.text());
                menu->addAction(openWithDialogAction);
            } else {
                // Only one associated app, already offered by the other menu -> add "Open With..." button
                KGuiItem::assign(d->openWithButton, openWithDialogItem);
            }
        } else {
            // qDebug() << "Not using new feature ServiceSelection; port the caller to BrowserOpenOrSaveQuestion::setFeature(ServiceSelection)";
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
    if (flags != (int)BrowserRun::AttachmentDisposition && mime.isValid() && (
            mime.inherits(QStringLiteral("text/html")) ||
            mime.inherits(QStringLiteral("application/xml")) ||
            mime.inherits(QStringLiteral("inode/directory")) ||
            mimeType.startsWith(QLatin1String("image")) ||
            mime.inherits(QStringLiteral("multipart/x-mixed-replace")) ||
            mime.inherits(QStringLiteral("multipart/replace"))))
        return true;
    return false;
}

BrowserOpenOrSaveQuestion::Result BrowserOpenOrSaveQuestion::askEmbedOrSave(int flags)
{
    if (d->autoEmbedMimeType(flags))
        return Embed;

    // don't use KStandardGuiItem::open() here which has trailing ellipsis!
    KGuiItem::assign(d->openDefaultButton, KGuiItem(i18nc("@label:button", "&Open"), QStringLiteral("document-open")));
    d->openWithButton->hide();

    d->questionLabel->setText(i18nc("@info", "Open '%1'?", d->url.toDisplayString(QUrl::PreferLocalFile)));
    d->questionLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);

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
    if (suggestedFileName.isEmpty()) {
        return;
    }

    d->fileNameLabel->setText(i18nc("@label File name", "Name: %1", suggestedFileName));
    d->fileNameLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
    d->fileNameLabel->setWhatsThis(i18nc("@info:whatsthis", "This is the file name suggested by the server"));
    d->fileNameLabel->show();
}

#include "browseropenorsavequestion.moc"
