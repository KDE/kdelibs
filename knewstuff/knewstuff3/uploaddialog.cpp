/*
    knewstuff3/ui/uploaddialog.cpp.
    Copyright (c) 2002 Cornelius Schumacher <schumacher@kde.org>
    Copyright (c) 2009 Jeremy Whiting <jpwhiting@kde.org>
    Copyright (C) 2009-2010 Frederik Gladhorn <gladhorn@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "uploaddialog.h"
#include "uploaddialog_p.h"

#include <QtGui/QLabel>
#include <QtGui/QLayout>
#include <QtGui/QDoubleSpinBox>
#include <QtCore/QString>

#include <kaboutdata.h>
#include <kcomponentdata.h>
#include <kmessagebox.h>
#include <kstandarddirs.h>
#include <kpixmapsequence.h>
#include <kpixmapsequencewidget.h>

#include <kdebug.h>
#include <kconfiggroup.h>

using namespace KNS3;

void UploadDialog::Private::init()
{
    QWidget* _mainWidget = new QWidget(q);
    q->setMainWidget(_mainWidget);
    ui.setupUi(_mainWidget);
    q->connect(&providerManager, SIGNAL(providerAdded(const Attica::Provider&)), q, SLOT(_k_providerAdded(const Attica::Provider&)));
    providerManager.loadDefaultProviders();

    q->connect(ui.mPreviewUrl, SIGNAL(urlSelected(const KUrl&)), q, SLOT(_k_previewChanged(const KUrl&)));
    q->connect(ui.providerComboBox, SIGNAL(currentIndexChanged(QString)), q, SLOT(_k_providerChanged(QString)));
    q->connect(ui.radioUpdate, SIGNAL(toggled(bool)), q, SLOT(_k_updateContentsToggled(bool)));

    //Busy widget
    busyWidget = new KPixmapSequenceWidget();
    busyWidget->setSequence(KPixmapSequence("process-working", 22));
    busyWidget->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));
    ui.busyWidget->setLayout(new QHBoxLayout());
    ui.busyWidget->layout()->addWidget(busyWidget);
    busyWidget->setVisible(false);
}

void UploadDialog::Private::setBusy(const QString& message)
{
    ui.busyLabel->setText(message);
    busyWidget->setVisible(true);
}

void UploadDialog::Private::setIdle(const QString& message)
{
    ui.busyLabel->setText(message);
    busyWidget->setVisible(false);
}

void UploadDialog::Private::_k_showPage(int page)
{
    ui.stackedWidget->setCurrentIndex(page);
    setIdle(QString());

    switch (ui.stackedWidget->currentIndex()) {
    case UserPasswordPage:
        ui.username->setFocus();
        break;

    case FileNewUpdatePage:
        currentProvider().saveCredentials(ui.username->text(), ui.password->text());
        ui.uploadButton->setFocus();
        fetchLicenses();
        setBusy(i18n("Fetching license data from server..."));
        break;

    case Details1Page:
        // TODO check if old contents should be updated!

        if (ui.radioUpdate->isChecked()) {
            // Fetch
            Attica::ItemJob<Attica::Content> *contentJob = currentProvider().requestContent(ui.userContentList->currentItem()->data(Qt::UserRole).toString());
            q->connect(contentJob, SIGNAL(finished(Attica::BaseJob*)), q, SLOT(_k_updatedContentFetched(Attica::BaseJob*)));
            contentJob->start();

            setBusy(i18n("Fetching content data from server..."));
        }

        ui.mNameEdit->setFocus();
        break;

    case UploadFinalPage:
        if (previewFile.isEmpty()) {
            ui.uploadPreviewImageLabel->setVisible(false);
            ui.uploadPreviewLabel->setVisible(false);
        }
        break;
    }

    _k_updatePage();
}

void UploadDialog::Private::_k_updatePage()
{
    bool firstPage = ui.stackedWidget->currentIndex() == 0;
    q->enableButton(BackButton, !firstPage && !finished);

    bool nextEnabled = false;
    switch (ui.stackedWidget->currentIndex()) {
    case UserPasswordPage:
        if (ui.providerComboBox->count() > 0 && !ui.username->text().isEmpty() && !ui.password->text().isEmpty()) {
            nextEnabled = true;
        }
        break;

    case FileNewUpdatePage:
        // FIXME: check if the file requester contains a valid file
        if (!uploadFile.isEmpty() || !ui.uploadFileRequester->url().isLocalFile()) {
            if (ui.radioNewUpload->isChecked() || ui.userContentList->currentRow() >= 0) {
                nextEnabled = true;
            }
        }
        break;

    case Details1Page:
        if (!ui.mNameEdit->text().isEmpty()) {
            nextEnabled = true;
        }
        break;

    case Details2Page:
        if (ui.copyrightCheckBox->isChecked()) {
            nextEnabled = true;
        }
        break;

    case UploadFinalPage:
        break;
    }

    q->enableButton(NextButton, nextEnabled);
    q->enableButton(FinishButton, finished);

    q->setDefaultButton(nextEnabled ? NextButton : FinishButton);

    if (nextEnabled && q->button(KDialog::Cancel)->hasFocus()) {
        q->button(NextButton)->setFocus();
    }
}

void UploadDialog::Private::_k_providerChanged(const QString& providerName)
{
    // TODO: update username/password
}

void UploadDialog::Private::_k_backPage()
{
    _k_showPage(ui.stackedWidget->currentIndex()-1);
}

void UploadDialog::Private::_k_nextPage()
{
    // TODO: validate credentials after user name/password have been entered
    if (ui.stackedWidget->currentIndex() == UserPasswordPage) {
        setBusy(i18n("Checking login..."));
        q->button(NextButton)->setEnabled(false);
        ui.providerComboBox->setEnabled(false);
        ui.username->setEnabled(false);
        ui.password->setEnabled(false);
        Attica::PostJob* checkLoginJob = currentProvider().checkLogin(ui.username->text(), ui.password->text());
        q->connect(checkLoginJob, SIGNAL(finished(Attica::BaseJob*)), q, SLOT(_k_checkCredentialsFinished(Attica::BaseJob*)));
        checkLoginJob->start();
    } else {
        _k_showPage(ui.stackedWidget->currentIndex()+1);
    }
}

void UploadDialog::Private::_k_checkCredentialsFinished(Attica::BaseJob* baseJob)
{
    ui.providerComboBox->setEnabled(true);
    ui.username->setEnabled(true);
    ui.password->setEnabled(true);

    if (baseJob->metadata().error() == Attica::Metadata::NoError) {
        currentProvider().saveCredentials(ui.username->text(), ui.password->text());
        _k_showPage(FileNewUpdatePage);

        // in case of updates we need the list of stuff that has been uploaded by the user before
        Attica::ListJob<Attica::Content>* userContent = currentProvider().searchContentsByPerson(categories, ui.username->text());
        q->connect(userContent, SIGNAL(finished(Attica::BaseJob*)), q, SLOT(_k_userContentListLoaded(Attica::BaseJob*)));
        userContent->start();
        setBusy(i18n("Fetching your previously updated content..."));
    } else {
        // TODO check what the actual error is
        setIdle(i18n("Could not verify login, please try again."));
    }
}

void UploadDialog::Private::fetchLicenses()
{
    // TODO       
    Attica::ListJob<Attica::License> *licenseJob = currentProvider().requestLicenses();
    q->connect(licenseJob, SIGNAL(finished(Attica::BaseJob*)), q, SLOT(_k_licensesFetched(Attica::BaseJob*)));
    licenseJob->start();
}

void UploadDialog::Private::_k_licensesFetched(Attica::BaseJob* baseJob)
{
    Attica::ListJob<Attica::License>* licenseList = static_cast<Attica::ListJob<Attica::License>*>(baseJob);
    kDebug() << "Licenses size: " << licenseList->itemList().size();
    
    ui.mLicenseCombo->clear();

    foreach(Attica::License license, licenseList->itemList()) {
        ui.mLicenseCombo->addItem(license.name(), license.id());
    }
}

void UploadDialog::Private::_k_userContentListLoaded(Attica::BaseJob* baseJob)
{
    setIdle(i18n("Fetching your previously updated content finished."));
    Attica::ListJob<Attica::Content>* contentList = static_cast<Attica::ListJob<Attica::Content>*>(baseJob);
    kDebug() << "Content size: " << contentList->itemList().size();

    foreach(Attica::Content content, contentList->itemList()) {
        kDebug() << content.name();
        QListWidgetItem *contentItem = new QListWidgetItem(content.name());
        contentItem->setData(Qt::UserRole, content.id());
        ui.userContentList->addItem(contentItem);
    }
    ui.userContentList->setCurrentRow(0);
}

void UploadDialog::Private::_k_updatedContentFetched(Attica::BaseJob* baseJob)
{
    setIdle(i18n("Fetching content data from server finished."));

    Attica::ItemJob<Attica::Content>* contentItemJob = static_cast<Attica::ItemJob<Attica::Content>* >(baseJob);
    Attica::Content content = contentItemJob->result();

    kDebug() << "Content " << content.name();

    // fill in ui
    ui.mNameEdit->setText(content.name());

    ui.mSummaryEdit->setText(content.description());
    ui.mVersionEdit->setText(content.version());
    ui.changelog->setText(content.changelog());
    ui.priceCheckBox->setChecked(content.attribute("downloadbuy1") == "1");
    ui.priceSpinBox->setValue(content.attribute("downloadbuyprice1").toDouble());
    ui.priceReasonLineEdit->setText(content.attribute("downloadbuyreason1"));

    bool conversionOk = false;
    int licenseNumber = content.license().toInt(&conversionOk);
    if (conversionOk) {
        // check if that int is in list
        int row = ui.mLicenseCombo->findData(licenseNumber, Qt::UserRole);
        ui.mLicenseCombo->setCurrentIndex(row);
    } else {
        ui.mLicenseCombo->setEditText(content.license());
    }

    ui.contentWebsiteLink->setText(QLatin1String("<a href=\"") + content.detailpage().toString() + QLatin1String("\">")
                                       + i18nc("A link to the website where the get hot new stuff upload can be seen", "Visit website") + QLatin1String("</a>"));
    ui.fetchContentLinkImageLabel->setPixmap(KIcon("dialog-ok").pixmap(16));
}

void UploadDialog::Private::_k_updateContentsToggled(bool update)
{
    ui.userContentList->setEnabled(update);
}

UploadDialog::UploadDialog(QWidget *parent)
    : KDialog(parent), d(new Private(this))
{
    KComponentData component = KGlobal::activeComponent();
    QString name = component.componentName();
    init(name + ".knsrc");
}

UploadDialog::UploadDialog(const QString& configFile, QWidget *parent)
    : KDialog(parent), d(new Private(this))
{
    init(configFile);
}

UploadDialog::~UploadDialog()
{
    delete d;
}

bool UploadDialog::init(const QString &configfile)
{
    d->init();

    setCaption(i18n("Share Hot New Stuff"));

    setButtons(KDialog::Cancel | KDialog::User1 | KDialog::User2 | KDialog::User3 | KDialog::Help);
    setButtonGuiItem( BackButton, KStandardGuiItem::back(KStandardGuiItem::UseRTL) );

    setButtonText( NextButton, i18nc("Opposite to Back", "Next") );
    setButtonIcon( NextButton, KStandardGuiItem::forward(KStandardGuiItem::UseRTL).icon() );
    setButtonText(FinishButton, i18n("Finish"));
    setButtonIcon( FinishButton, KIcon("dialog-ok-apply") );
    setDefaultButton(NextButton);
    showButtonSeparator(true);
    d->_k_updatePage();

    connect(d->ui.username, SIGNAL(textChanged(QString)), this, SLOT(_k_updatePage()));

    connect(d->ui.password, SIGNAL(textChanged(QString)), this, SLOT(_k_updatePage()));
    connect(d->ui.mNameEdit, SIGNAL(textChanged(QString)), this, SLOT(_k_updatePage()));
    connect(d->ui.copyrightCheckBox, SIGNAL(stateChanged(int)), this, SLOT(_k_updatePage()));
    connect(d->ui.uploadFileRequester, SIGNAL(textChanged(QString)), this, SLOT(_k_updatePage()));
    connect(d->ui.priceCheckBox, SIGNAL(toggled(bool)), this, SLOT(_k_priceToggled(bool)));

    connect(d->ui.uploadButton, SIGNAL(clicked()), this, SLOT(_k_startUpload()));

    connect(this, SIGNAL(user3Clicked()), this, SLOT(_k_backPage()));
    connect(this, SIGNAL(user2Clicked()), this, SLOT(_k_nextPage()));
    connect(this, SIGNAL(user1Clicked()), this, SLOT(accept()));

    d->ui.mTitleWidget->setText(i18nc("Program name followed by 'Add On Uploader'",
                                 "%1 Add-On Uploader",
                                 KGlobal::activeComponent().aboutData()->programName()));
    d->ui.mTitleWidget->setPixmap(KIcon(KGlobal::activeComponent().aboutData()->programIconName()));

    KConfig conf(configfile);
    if (conf.accessMode() == KConfig::NoAccess) {
        kError() << "No knsrc file named '" << configfile << "' was found." << endl;
        return false;
    }
    // FIXME: accessMode() doesn't return NoAccess for non-existing files
    // - bug in kdecore?
    // - this needs to be looked at again until KConfig backend changes for KDE 4
    // the check below is a workaround
    if (KStandardDirs::locate("config", configfile).isEmpty()) {
        kError() << "No knsrc file named '" << configfile << "' was found." << endl;
        return false;
    }

    KConfigGroup group;
    if (conf.hasGroup("KNewStuff3")) {
        kDebug() << "Loading KNewStuff3 config: " << configfile;
        group = conf.group("KNewStuff3");
    } else {
        kError() << "A knsrc file was found but it doesn't contain a KNewStuff3 section." << endl;
        return false;
    }

    d->categoryNames = group.readEntry("UploadCategories", QStringList());
    d->ui.mCategoryCombo->addItems(d->categoryNames);

    if (d->categoryNames.size() == 1) {
        d->ui.mCategoryLabel->setVisible(false);
        d->ui.mCategoryCombo->setVisible(false);
    }

    kDebug() << "Categories: " << d->categoryNames;


    d->_k_showPage(0);

    return true;
}

void UploadDialog::setUploadFile(const KUrl& payloadFile)
{
    d->uploadFile = payloadFile;

    d->ui.uploadFileLabel->setVisible(false);
    d->ui.uploadFileRequester->setVisible(false);

    QFile file(d->uploadFile.toLocalFile());
    if (!file.open(QIODevice::ReadOnly)) {
        KMessageBox::error(this, i18n("File not found: %1",d->uploadFile.url()), i18n("Upload Failed"));
    }
}

void UploadDialog::setUploadName(const QString& name)
{
    d->ui.mNameEdit->setText(name);
}

void UploadDialog::selectCategory(const QString& category)
{
    d->ui.mCategoryCombo->setCurrentIndex(d->ui.mCategoryCombo->findText(category, Qt::MatchFixedString));
}

void UploadDialog::setChangelog(const QString& changelog)
{
    d->ui.changelog->setText(changelog);
}

void UploadDialog::setDescription(const QString& description)
{
    d->ui.mSummaryEdit->setText(description);
}

void UploadDialog::setPrice(double price)
{
    d->ui.priceCheckBox->setEnabled(true);
    d->ui.priceSpinBox->setValue(price);
}

void UploadDialog::setPriceReason(const QString& reason)
{
    d->ui.priceReasonLineEdit->setText(reason);
}

void UploadDialog::setVersion(const QString& version)
{
    d->ui.mVersionEdit->setText(version);
}

void UploadDialog::Private::_k_priceToggled(bool priceEnabled)
{
    ui.priceGroupBox->setEnabled(priceEnabled);
}

void UploadDialog::Private::_k_providerAdded(const Attica::Provider& provider)
{
    providers.insert(provider.name(), provider);
    ui.providerComboBox->addItem(provider.name());
    _k_updatePage();

    Attica::ListJob<Attica::Category>* job = providers[provider.name()].requestCategories();
    q->connect(job, SIGNAL(finished(Attica::BaseJob*)), q, SLOT(_k_categoriesLoaded(Attica::BaseJob*)));
    job->start();

    if (currentProvider().name() == provider.name() && providers[provider.name()].hasCredentials()) {
        QString user;
        QString pass;
        if (providers[provider.name()].loadCredentials(user, pass)) {
            ui.username->setText(user);
            ui.password->setText(pass);
        }
    }
}

void UploadDialog::Private::_k_categoriesLoaded(Attica::BaseJob* job)
{
    kDebug() << "Loading Categories..." << categoryNames;

    Attica::ListJob<Attica::Category>* listJob = static_cast<Attica::ListJob<Attica::Category>*>(job);
    Attica::Category::List newCategories = listJob->itemList();

    Q_FOREACH(const Attica::Category &category, newCategories) {
        if (categoryNames.contains(category.name())) {
            categories.append(category);
            kDebug() << "found category: " << category.name();
        }
        else {
            //kDebug() << "found invalid category: " << category.name();
        }
    }

    // at least one category is needed
    if (newCategories.count() == 0) {
        if (categoryNames.size() > 0) {
            KMessageBox::error(q,
                               i18np("The server does not recognize the category %2 to which you are trying to upload.",
                                     "The server does not recognize any of the categories to which you are trying to upload: %2",
                                     categoryNames.size(), categoryNames.join(", ")),
                               i18n("Error"));
            // close the dialog
            q->reject();
        } else {
            kWarning() << "No category was set in knsrc file. Adding all categories.";
            Q_FOREACH(const Attica::Category &category, newCategories) {
                ui.mCategoryCombo->addItem(category.name());
                categoryNames.append(category.name());
            }
            categories = newCategories;
        }
    }
}

void UploadDialog::accept()
{
    KDialog::accept();
}

void UploadDialog::Private::_k_startUpload()
{
    kDebug() << "Starting upload";

    // FIXME: this only works if categories are set in the .knsrc file
    // TODO: ask for confirmation when closing the dialog

    q->button(BackButton)->setEnabled(false);
    q->button(KDialog::Cancel)->setEnabled(false);

    ui.uploadButton->setEnabled(false);

    // idle back and forth, we need a fix in attica to get at real progress values
    ui.uploadProgressBar->setMinimum(0);
    ui.uploadProgressBar->setMaximum(0);
    ui.uploadProgressBar->setValue(0);

    // check the category
    QString categoryName = ui.mCategoryCombo->currentText();
    QList<Attica::Category>::const_iterator iter = categories.constBegin();
    Attica::Category category;
    while (iter != categories.constEnd()) {
        if (iter->name() == categoryName) {
            category = *iter;
            break;
        }
        ++iter;
    }
    if (!category.isValid()) {
        KMessageBox::error(q, i18n("The selected category \"%1\" is invalid.", categoryName), i18n("Upload Failed"));
        return;
    }

    // fill in the content object
    Attica::Content content;
    content.setName(ui.mNameEdit->text());
    QString summary = ui.mSummaryEdit->toPlainText();
    content.addAttribute("description", summary);
    content.addAttribute("version", ui.mVersionEdit->text());

    // for the license, if one of the licenses coming from the server was used, pass its id, otherwise the string
    QString licenseId = ui.mLicenseCombo->itemData(ui.mLicenseCombo->currentIndex()).toString();
    if (licenseId.isEmpty()) {
        // use other as type and add the string as text
        content.addAttribute("licensetype", "0");
        content.addAttribute("license", ui.mLicenseCombo->currentText());
    } else {
        content.addAttribute("licensetype", licenseId);
    }

    content.addAttribute("changelog", ui.changelog->toPlainText());

    // TODO: add additional attributes
    //content.addAttribute("downloadlink1", ui.link1->text());
    //content.addAttribute("downloadlink2", ui.link2->text());
    //content.addAttribute("homepage1", ui.homepage->text());
    //content.addAttribute("blog1", ui.blog->text());

    content.addAttribute("downloadbuy1", ui.priceCheckBox->isChecked() ? "1" : "0");
    content.addAttribute("downloadbuyprice1", QString::number(ui.priceSpinBox->value()));
    content.addAttribute("downloadbuyreason1", ui.priceReasonLineEdit->text());
    
    if (ui.radioNewUpload->isChecked()) {
        // upload a new content
        Attica::ItemPostJob<Attica::Content>* job = currentProvider().addNewContent(category, content);
        q->connect(job, SIGNAL(finished(Attica::BaseJob*)), q, SLOT(_k_contentAdded(Attica::BaseJob*)));
        job->start();
    } else {
        // update old content
        Attica::ItemPostJob<Attica::Content>* job = currentProvider().editContent(category, ui.userContentList->currentItem()->data(Qt::UserRole).toString(), content);
        q->connect(job, SIGNAL(finished(Attica::BaseJob*)), q, SLOT(_k_contentAdded(Attica::BaseJob*)));
        job->start();
    }
}

void UploadDialog::Private::_k_previewChanged(const KUrl& url)
{
    previewFile = url;
    QPixmap img(url.toLocalFile());
    ui.mImagePreview->setPixmap(img.scaled(ui.mImagePreview->size(), Qt::KeepAspectRatio));
}

void UploadDialog::Private::_k_contentAdded(Attica::BaseJob* baseJob)
{
    if (baseJob->metadata().error()) {
        if (baseJob->metadata().error() == Attica::Metadata::NetworkError) {
            KMessageBox::error(q, i18n("There was a network error."), i18n("Uploading Failed"));
            return;
        }
        if (baseJob->metadata().error() == Attica::Metadata::OcsError) {
            if (baseJob->metadata().statusCode() == 102)
            KMessageBox::error(q, i18n("Authentication error."), i18n("Uploading Failed"));
        }
        return;
    }

    ui.createContentImageLabel->setPixmap(KIcon("dialog-ok").pixmap(16));

    Attica::ItemPostJob<Attica::Content> * job = static_cast<Attica::ItemPostJob<Attica::Content> *>(baseJob);
    QString id = job->result().id();

    kDebug() << "content added " << id;

    contentId = id;

    doUpload(QString(), uploadFile.toLocalFile());

    if (!previewFile.isEmpty()) {
        doUpload("1", previewFile.toLocalFile());
    }

    if (ui.radioNewUpload->isChecked()) {
        fetchDownloadLink(contentId);
    }
}

void UploadDialog::Private::doUpload(const QString& index, const QString& path)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        KMessageBox::error(q, i18n("File not found: %1",uploadFile.url(), i18n("Upload Failed")));
        q->reject();
        return;
    }

    QByteArray fileContents;
    fileContents.append(file.readAll());
    file.close();

    QString fileName = QFileInfo(path).fileName();

    Attica::PostJob* job;
    if (index.isEmpty()) {
        job = currentProvider().setDownloadFile(contentId, fileName, fileContents);
        q->connect(job, SIGNAL(finished(Attica::BaseJob*)), q, SLOT(_k_fileUploadFinished(Attica::BaseJob*)));
    } else {
        job = currentProvider().setPreviewImage(contentId, index, fileName, fileContents);
        q->connect(job, SIGNAL(finished(Attica::BaseJob*)), q, SLOT(_k_previewUploadFinished(Attica::BaseJob*)));
    }

    job->start();
}

void UploadDialog::Private::_k_fileUploadFinished(Attica::BaseJob* )
{
    ui.uploadContentImageLabel->setPixmap(KIcon("dialog-ok").pixmap(16));
    finishedContents = true;
    uploadFileFinished();
}

void UploadDialog::Private::_k_previewUploadFinished(Attica::BaseJob* )
{
    ui.uploadPreviewImageLabel->setPixmap(KIcon("dialog-ok").pixmap(16));
    finishedPreview = true;
    uploadFileFinished();
}

void UploadDialog::Private::uploadFileFinished()
{
    if (finishedContents && (previewFile.isEmpty() || finishedPreview)) {
        finished = true;
        ui.uploadProgressBar->setMinimum(0);
        ui.uploadProgressBar->setMaximum(100);
        ui.uploadProgressBar->setValue(100);
        _k_updatePage();
    }
}

void UploadDialog::Private::fetchDownloadLink(const QString& contentId)
{
    kDebug() << "link for  " << contentId;
    Attica::ItemJob<Attica::Content> *contentJob = currentProvider().requestContent(contentId);
    q->connect(contentJob, SIGNAL(finished(Attica::BaseJob*)), q, SLOT(_k_downloadLinkFetched(Attica::BaseJob*)));
    contentJob->start();
}

void UploadDialog::Private::_k_downloadLinkFetched(Attica::BaseJob* baseJob)
{

    Attica::ItemJob<Attica::Content>* contentItemJob = static_cast<Attica::ItemJob<Attica::Content>* >(baseJob);
    Attica::Content content = contentItemJob->result();

    kDebug() << "link done " << content.detailpage().toString();

    ui.contentWebsiteLink->setText(QLatin1String("<a href=\"") + content.detailpage().toString() + QLatin1String("\">")
                                       + i18nc("A link to the website where the get hot new stuff upload can be seen", "Visit website") + QLatin1String("</a>"));
    ui.fetchContentLinkImageLabel->setPixmap(KIcon("dialog-ok").pixmap(16));
}

#include "uploaddialog.moc"
