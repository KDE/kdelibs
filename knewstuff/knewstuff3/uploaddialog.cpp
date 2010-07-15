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
#include <QtCore/QSignalMapper>

#include <kaboutdata.h>
#include <kcomponentdata.h>
#include <kfiledialog.h>
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

    atticaHelper = new AtticaHelper(q);
    q->connect(atticaHelper, SIGNAL(providersLoaded(QStringList)), q, SLOT(_k_providersLoaded(QStringList)));
    q->connect(atticaHelper, SIGNAL(loginChecked(bool)), q, SLOT(_k_checkCredentialsFinished(bool)));
    q->connect(atticaHelper, SIGNAL(licensesLoaded(Attica::License::List)), q, SLOT(_k_licensesLoaded(Attica::License::List)));
    q->connect(atticaHelper, SIGNAL(categoriesLoaded(Attica::Category::List)), q, SLOT(_k_categoriesLoaded(Attica::Category::List)));
    q->connect(atticaHelper, SIGNAL(contentByCurrentUserLoaded(Attica::Content::List)), q, SLOT(_k_contentByCurrentUserLoaded(Attica::Content::List)));
    q->connect(atticaHelper, SIGNAL(contentLoaded(Attica::Content)), q, SLOT(_k_updatedContentFetched(Attica::Content)));
    q->connect(atticaHelper, SIGNAL(detailsLinkLoaded(QUrl)), q, SLOT(_k_detailsLinkLoaded(QUrl)));
    q->connect(atticaHelper, SIGNAL(currencyLoaded(QString)), q, SLOT(_k_currencyLoaded(QString)));
    q->connect(atticaHelper, SIGNAL(previewLoaded(int, QImage)), q, SLOT(_k_previewLoaded(int, QImage)));
    atticaHelper->init();

    q->connect(ui.changePreview1Button, SIGNAL(clicked()), q, SLOT(_k_changePreview1()));
    q->connect(ui.changePreview2Button, SIGNAL(clicked()), q, SLOT(_k_changePreview2()));
    q->connect(ui.changePreview3Button, SIGNAL(clicked()), q, SLOT(_k_changePreview3()));

    q->connect(ui.providerComboBox, SIGNAL(currentIndexChanged(QString)), q, SLOT(_k_providerChanged(QString)));
    q->connect(ui.radioUpdate, SIGNAL(toggled(bool)), q, SLOT(_k_updateContentsToggled(bool)));

    //Busy widget
    busyWidget = new KPixmapSequenceWidget();
    busyWidget->setSequence(KPixmapSequence("process-working", 22));
    busyWidget->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));
    ui.busyWidget->setLayout(new QHBoxLayout());
    ui.busyWidget->layout()->addWidget(busyWidget);
    busyWidget->setVisible(false);

    //ui.previewImage1->showPreview(KUrl("invalid"));
    //ui.previewImage2->showPreview(KUrl("invalid"));
    //ui.previewImage3->showPreview(KUrl("invalid"));
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
        // TODO 4.6 enable new string: setBusy(i18n("Fetching provider information..."));
        break;

    case FileNewUpdatePage:
        atticaHelper->loadLicenses();
        atticaHelper->loadCurrency();
        ui.uploadButton->setFocus();
        setBusy(i18n("Fetching license data from server..."));
        break;

    case Details1Page:
        if (ui.radioUpdate->isChecked()) {
            // Fetch
            atticaHelper->loadContent(ui.userContentList->currentItem()->data(Qt::UserRole).toString());
            setBusy(i18n("Fetching content data from server..."));
        }

        ui.mNameEdit->setFocus();
        break;

    case UploadFinalPage:
        if (previewFile1.isEmpty()) {
            ui.uploadPreview1ImageLabel->setVisible(false);
            ui.uploadPreview1Label->setVisible(false);
        }
        if (previewFile2.isEmpty()) {
            ui.uploadPreview2ImageLabel->setVisible(false);
            ui.uploadPreview2Label->setVisible(false);
        }
        if (previewFile3.isEmpty()) {
            ui.uploadPreview3ImageLabel->setVisible(false);
            ui.uploadPreview3Label->setVisible(false);
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
        if (!uploadFile.isEmpty() || ui.uploadFileRequester->url().isLocalFile()) {
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
        nextEnabled = true;
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

void UploadDialog::Private::_k_providersLoaded(const QStringList& providers)
{
    if (providers.size() == 0) {
        // TODO 4.6 enable new string: setIdle(i18n("Could not fetch provider information."));
        ui.stackedWidget->setEnabled(false);
        kWarning() << "Could not load providers.";
        return;
    }
    setIdle(QString());
    ui.providerComboBox->addItems(providers);
    ui.providerComboBox->setCurrentIndex(0);
    atticaHelper->setCurrentProvider(providers.at(0));

    QString user;
    QString pass;
    if (atticaHelper->loadCredentials(user, pass)) {
        ui.username->setText(user);
        ui.password->setText(pass);
    }
    _k_updatePage();
}

void UploadDialog::Private::_k_providerChanged(const QString& providerName)
{
    atticaHelper->setCurrentProvider(providerName);
    ui.username->clear();
    ui.password->clear();
    QString user;
    QString pass;
    if (atticaHelper->loadCredentials(user, pass)) {
        ui.username->setText(user);
        ui.password->setText(pass);
    }
    _k_updatePage();
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
        atticaHelper->checkLogin(ui.username->text(), ui.password->text());
    } else {
        _k_showPage(ui.stackedWidget->currentIndex()+1);
    }
}

void UploadDialog::Private::_k_checkCredentialsFinished(bool success)
{
    ui.providerComboBox->setEnabled(true);
    ui.username->setEnabled(true);
    ui.password->setEnabled(true);

    if (success) {
        atticaHelper->saveCredentials(ui.username->text(), ui.password->text());
        _k_showPage(FileNewUpdatePage);

        atticaHelper->loadCategories(categoryNames);
        setBusy(i18n("Fetching your previously updated content..."));
    } else {
        // TODO check what the actual error is
        setIdle(i18n("Could not verify login, please try again."));
    }
}

void UploadDialog::Private::_k_licensesLoaded(const Attica::License::List& licenses)
{
    ui.mLicenseCombo->clear();
    foreach (Attica::License license, licenses) {
        ui.mLicenseCombo->addItem(license.name(), license.id());
    }
}

void UploadDialog::Private::_k_currencyLoaded(const QString& currency)
{
    ui.priceCurrency->setText(currency);
}

void UploadDialog::Private::_k_contentByCurrentUserLoaded(const Attica::Content::List& contentList)
{
    setIdle(i18n("Fetching your previously updated content finished."));

    foreach(Attica::Content content, contentList) {
        QListWidgetItem *contentItem = new QListWidgetItem(content.name());
        contentItem->setData(Qt::UserRole, content.id());
        ui.userContentList->addItem(contentItem);
    }

    if (ui.userContentList->count() > 0) {
        ui.userContentList->setCurrentRow(0);
        ui.radioUpdate->setEnabled(true);
        _k_updatePage();
    }

}

void UploadDialog::Private::_k_updatedContentFetched(const Attica::Content& content)
{
    setIdle(i18n("Fetching content data from server finished."));

    contentId = content.id();
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

void UploadDialog::Private::_k_previewLoaded(int index, const QImage& image)
{
    switch (index) {
    case 1:
        ui.previewImage1->setPixmap(QPixmap::fromImage(image));
        break;
    case 2:
        ui.previewImage2->setPixmap(QPixmap::fromImage(image));
        break;
    case 3:
        ui.previewImage3->setPixmap(QPixmap::fromImage(image));
        break;
    }
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
    d->_k_updatePage();

    connect(d->ui.username, SIGNAL(textChanged(QString)), this, SLOT(_k_updatePage()));

    connect(d->ui.password, SIGNAL(textChanged(QString)), this, SLOT(_k_updatePage()));
    connect(d->ui.mNameEdit, SIGNAL(textChanged(QString)), this, SLOT(_k_updatePage()));
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
    // fall back to download categories
    if (d->categoryNames.isEmpty()) {
        d->categoryNames = group.readEntry("Categories", QStringList());
    }

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
        KMessageBox::error(this, i18n("File not found: %1", d->uploadFile.url()), i18n("Upload Failed"));
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

void UploadDialog::setPriceEnabled(bool enabled)
{
    d->ui.priceCheckBox->setVisible(enabled);
    d->ui.priceGroupBox->setVisible(enabled);
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

void UploadDialog::setPreviewImageFile(uint number, const KUrl& file)
{
    QPixmap preview(file.toLocalFile());
    switch(number) {
    case 0 :
        d->previewFile1 = file;
        d->ui.previewImage1->setPixmap(preview.scaled(d->ui.previewImage1->size()));
        break;
    case 1 :
        d->previewFile2 = file;
        d->ui.previewImage2->setPixmap(preview.scaled(d->ui.previewImage2->size()));
        break;
    case 2 :
        d->previewFile3 = file;
        d->ui.previewImage3->setPixmap(preview.scaled(d->ui.previewImage3->size()));
        break;
    default :
        kError() << "Wrong preview image file number";
        break;
    }
}

void UploadDialog::Private::_k_priceToggled(bool priceEnabled)
{
    ui.priceGroupBox->setEnabled(priceEnabled);
}

void UploadDialog::Private::_k_categoriesLoaded(const Attica::Category::List& loadedCategories)
{
    categories = loadedCategories;

    // at least one category is needed
    if (categories.count() == 0) {
            KMessageBox::error(q,
                     i18np("The server does not recognize the category %2 to which you are trying to upload.",
                           "The server does not recognize any of the categories to which you are trying to upload: %2",
                           categoryNames.size(), categoryNames.join(", ")),
                           i18n("Error"));
            // close the dialog
            q->reject();
            return;
    }
    foreach(Attica::Category c, categories) {
        ui.mCategoryCombo->addItem(c.name(), c.id());
    }
    atticaHelper->loadContentByCurrentUser();
}

void UploadDialog::accept()
{
    KDialog::accept();
}

void UploadDialog::Private::_k_startUpload()
{
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

void UploadDialog::Private::_k_changePreview1()
{
    KUrl url = KFileDialog::getImageOpenUrl(KUrl(), q, i18n("Select preview image"));
    previewFile1 = url;
    kDebug() << "preview is: " << url.url();
    QPixmap preview(url.toLocalFile());
    ui.previewImage1->setPixmap(preview.scaled(ui.previewImage1->size()));
}

void UploadDialog::Private::_k_changePreview2()
{
    KUrl url = KFileDialog::getImageOpenUrl(KUrl(), q, i18n("Select preview image"));
    previewFile2 = url;
    QPixmap preview(url.toLocalFile());
    ui.previewImage2->setPixmap(preview.scaled(ui.previewImage1->size()));
}

void UploadDialog::Private::_k_changePreview3()
{
    KUrl url = KFileDialog::getImageOpenUrl(KUrl(), q, i18n("Select preview image"));
    previewFile3 = url;
    QPixmap preview(url.toLocalFile());
    ui.previewImage3->setPixmap(preview.scaled(ui.previewImage1->size()));
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
    if (job->metadata().error() != Attica::Metadata::NoError) {
        KMessageBox::error(q, i18n("Upload failed: %1", job->metadata().message()));
        return;
    }

    // only when adding new content we get an id returned, otherwise stick with the old one
    QString id = job->result().id();
    if (!id.isEmpty()) {
        contentId = id;
    }

    if (!uploadFile.isEmpty()) {
        doUpload(QString(), uploadFile);
    } else {
        doUpload(QString(), ui.uploadFileRequester->url());
    }

    // FIXME: status labels need to accomodate 3 previews
    if (!previewFile1.isEmpty()) {
        doUpload("1", previewFile1);
    }
    if (!previewFile2.isEmpty()) {
        doUpload("2", previewFile2);
    }
    if (!previewFile3.isEmpty()) {
        doUpload("3", previewFile3);
    }

    if (ui.radioNewUpload->isChecked()) {
        atticaHelper->loadDetailsLink(contentId);
    }
}

void UploadDialog::Private::doUpload(const QString& index, const KUrl& path)
{
    QFile file(path.toLocalFile());
    if (!file.open(QIODevice::ReadOnly)) {
        KMessageBox::error(q, i18n("File not found: %1", uploadFile.url(), i18n("Upload Failed")));
        q->reject();
        return;
    }

    QByteArray fileContents;
    fileContents.append(file.readAll());
    file.close();

    QString fileName = QFileInfo(path.toLocalFile()).fileName();

    Attica::PostJob* job;
    if (index.isEmpty()) {
        job = currentProvider().setDownloadFile(contentId, fileName, fileContents);
        q->connect(job, SIGNAL(finished(Attica::BaseJob*)), q, SLOT(_k_fileUploadFinished(Attica::BaseJob*)));
    } else if (index == QLatin1String("1")) {
        job = currentProvider().setPreviewImage(contentId, index, fileName, fileContents);
        q->connect(job, SIGNAL(finished(Attica::BaseJob*)), q, SLOT(_k_preview1UploadFinished(Attica::BaseJob*)));
    } else if (index == QLatin1String("2")) {
        job = currentProvider().setPreviewImage(contentId, index, fileName, fileContents);
        q->connect(job, SIGNAL(finished(Attica::BaseJob*)), q, SLOT(_k_preview2UploadFinished(Attica::BaseJob*)));
    } else if (index == QLatin1String("3")) {
        job = currentProvider().setPreviewImage(contentId, index, fileName, fileContents);
        q->connect(job, SIGNAL(finished(Attica::BaseJob*)), q, SLOT(_k_preview3UploadFinished(Attica::BaseJob*)));
    }

    job->start();
}

void UploadDialog::Private::_k_fileUploadFinished(Attica::BaseJob* )
{
    ui.uploadContentImageLabel->setPixmap(KIcon("dialog-ok").pixmap(16));
    finishedContents = true;
    uploadFileFinished();
}

void UploadDialog::Private::_k_preview1UploadFinished(Attica::BaseJob* )
{
    ui.uploadPreview1ImageLabel->setPixmap(KIcon("dialog-ok").pixmap(16));
    finishedPreview1 = true;
    uploadFileFinished();
}

void UploadDialog::Private::_k_preview2UploadFinished(Attica::BaseJob* )
{
    ui.uploadPreview2ImageLabel->setPixmap(KIcon("dialog-ok").pixmap(16));
    finishedPreview2 = true;
    uploadFileFinished();
}

void UploadDialog::Private::_k_preview3UploadFinished(Attica::BaseJob* )
{
    ui.uploadPreview3ImageLabel->setPixmap(KIcon("dialog-ok").pixmap(16));
    finishedPreview3 = true;
    uploadFileFinished();
}

void UploadDialog::Private::uploadFileFinished()
{
    // FIXME multiple previews
    if (finishedContents && (previewFile1.isEmpty() || finishedPreview1)
            && (previewFile2.isEmpty() || finishedPreview2)
            && (previewFile3.isEmpty() || finishedPreview3)) {
        finished = true;
        ui.uploadProgressBar->setMinimum(0);
        ui.uploadProgressBar->setMaximum(100);
        ui.uploadProgressBar->setValue(100);
        _k_updatePage();
    }
}

void UploadDialog::Private::_k_detailsLinkLoaded(const QUrl& url)
{
    ui.contentWebsiteLink->setText(QLatin1String("<a href=\"") + url.toString() + QLatin1String("\">")
                                       + i18nc("A link to the website where the get hot new stuff upload can be seen", "Visit website") + QLatin1String("</a>"));
    ui.fetchContentLinkImageLabel->setPixmap(KIcon("dialog-ok").pixmap(16));
}

#include "uploaddialog.moc"
