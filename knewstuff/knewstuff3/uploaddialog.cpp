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

#include <kdebug.h>
#include <kconfiggroup.h>

using namespace KNS3;

void UploadDialog::Private::init()
{
    q->connect(ui.providerComboBox, SIGNAL(currentIndexChanged(QString)), q, SLOT(_k_providerChanged(QString)));
}

void UploadDialog::Private::_k_showPage(int page)
{
    ui.stackedWidget->setCurrentIndex(page);

    switch (ui.stackedWidget->currentIndex()) {
    case UserPasswordPage:
        ui.username->setFocus();
        break;

    case FileNewUpdatePage:
        currentProvider().saveCredentials(ui.username->text(), ui.password->text());
        ui.uploadButton->setFocus();
        break;

    case Details1Page:
        ui.mNameEdit->setFocus();
        break;
    }

    _k_updatePage();
}

void UploadDialog::Private::_k_updatePage()
{
    bool firstPage = ui.stackedWidget->currentIndex() == 0;
    q->enableButton(BackButton, !firstPage);

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
            nextEnabled = true;
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
        q->button(NextButton)->setEnabled(false);
        Attica::PostJob* checkLoginJob = currentProvider().checkLogin(ui.username->text(), ui.password->text());
        q->connect(checkLoginJob, SIGNAL(finished(Attica::BaseJob*)), q, SLOT(_k_checkCredentialsFinished(Attica::BaseJob*)));
        checkLoginJob->start();
    } else {
        _k_showPage(ui.stackedWidget->currentIndex()+1);
    }
}

void UploadDialog::Private::_k_checkCredentialsFinished(Attica::BaseJob* baseJob)
{
    if (baseJob->metadata().error() == Attica::Metadata::NoError) {
        currentProvider().saveCredentials(ui.username->text(), ui.password->text());
        _k_showPage(FileNewUpdatePage);


    } else {
        // TODO check what the actual error is
        KMessageBox::error(q, i18n("Could not verify login, please try again."), i18n("Error"));
    }
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
    // populate dialog with stuff
    QWidget* _mainWidget = new QWidget(this);
    setMainWidget(_mainWidget);
    d->ui.setupUi(_mainWidget);
    connect(d->ui.mPreviewUrl, SIGNAL(urlSelected(const KUrl&)), SLOT(previewChanged(const KUrl&)));

    connect(&d->providerManager, SIGNAL(providerAdded(const Attica::Provider&)), SLOT(_k_providerAdded(const Attica::Provider&)));
    d->providerManager.loadDefaultProviders();

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
    connect(d->ui.priceCheckBox, SIGNAL(toggled(bool)), this, SLOT(priceToggled(bool)));

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

void UploadDialog::priceToggled(bool priceEnabled)
{
    d->ui.priceGroupBox->setEnabled(priceEnabled);
}

void UploadDialog::Private::_k_providerAdded(const Attica::Provider& provider)
{
    providers.insert(provider.name(), provider);
    ui.providerComboBox->addItem(provider.name());
    _k_updatePage(); // manually

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
    // FIXME: this only works if categories are set in the .knsrc file
    // TODO: ask for confirmation when closing the dialog

    q->button(BackButton)->setEnabled(false);
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

    ui.mProgressLabel->setText(i18n("Creating Content on Server..."));

    // fill in the content object
    Attica::Content content;
    content.setName(ui.mNameEdit->text());
    QString summary = ui.mSummaryEdit->toPlainText();
    content.addAttribute("description", summary);
    content.addAttribute("version", ui.mVersionEdit->text());
    content.addAttribute("license", ui.mLicenseCombo->currentText());
    content.addAttribute("changelog", ui.changelog->toPlainText());

    // TODO: add additional attributes
    //content.addAttribute("downloadlink1", ui.link1->text());
    //content.addAttribute("downloadlink2", ui.link2->text());
    //content.addAttribute("homepage1", ui.homepage->text());
    //content.addAttribute("blog1", ui.blog->text());

    if (ui.priceCheckBox->isChecked()) {
        content.addAttribute("downloadbuy1", "1");
        content.addAttribute("downloadbuyprice1", QString::number(ui.priceSpinBox->value()));
        content.addAttribute("downloadbuyreason1", ui.priceReasonLineEdit->text());
    }

    Attica::ItemPostJob<Attica::Content>* job = currentProvider().addNewContent(category, content);
    q->connect(job, SIGNAL(finished(Attica::BaseJob*)), q, SLOT(contentAdded(Attica::BaseJob*)));
    job->start();
}

void UploadDialog::previewChanged(const KUrl& url)
{
    d->previewFile = url;
    QPixmap img(url.toLocalFile());
    d->ui.mImagePreview->setPixmap(img.scaled(d->ui.mImagePreview->size(), Qt::KeepAspectRatio));
}

void UploadDialog::contentAdded(Attica::BaseJob* baseJob)
{
    if (baseJob->metadata().error()) {
        if (baseJob->metadata().error() == Attica::Metadata::NetworkError) {
            KMessageBox::error(this, i18n("There was a network error."), i18n("Uploading Failed"));
            return;
        }
        if (baseJob->metadata().error() == Attica::Metadata::OcsError) {
            if (baseJob->metadata().statusCode() == 102)
            KMessageBox::error(this, i18n("Authentication error."), i18n("Uploading Failed"));
        }
        return;
    }

    d->ui.mProgressLabel->setText(d->ui.mProgressLabel->text() + '\n' + i18n("Uploading preview and content..."));

    Attica::ItemPostJob<Attica::Content> * job = static_cast<Attica::ItemPostJob<Attica::Content> *>(baseJob);
    QString id = job->result().id();

    kDebug() << "content added " << id;

    d->contentId = id;

    d->ui.mProgressLabel->setText(d->ui.mProgressLabel->text() + '\n' + i18n("Uploading content..."));
    doUpload(QString(), d->uploadFile.toLocalFile());

    if (!d->previewFile.isEmpty()) {
        d->ui.mProgressLabel->setText(d->ui.mProgressLabel->text() + '\n' + i18n("Uploading preview image and content..."));
        doUpload("1", d->previewFile.toLocalFile());
    }
}

void UploadDialog::doUpload(const QString& index, const QString& path)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        KMessageBox::error(this, i18n("File not found: %1",d->uploadFile.url(), i18n("Upload Failed")));
        reject();
        return;
    }

    QByteArray fileContents;
    fileContents.append(file.readAll());
    file.close();

    QString fileName = QFileInfo(path).fileName();

    Attica::PostJob* job;
    if (index.isEmpty()) {
        job = d->currentProvider().setDownloadFile(d->contentId, fileName, fileContents);
        connect(job, SIGNAL(finished(Attica::BaseJob*)), SLOT(fileUploadFinished(Attica::BaseJob*)));
    } else {
        job = d->currentProvider().setPreviewImage(d->contentId, index, fileName, fileContents);
        connect(job, SIGNAL(finished(Attica::BaseJob*)), SLOT(previewUploadFinished(Attica::BaseJob*)));
    }

    job->start();
}

void UploadDialog::fileUploadFinished(Attica::BaseJob* )
{
    d->ui.mProgressLabel->setText(d->ui.mProgressLabel->text() + "\n\n" + i18n("Content successfully uploaded."));
    d->finished = true;

    d->ui.uploadProgressBar->setMinimum(0);
    d->ui.uploadProgressBar->setMaximum(100);
    d->ui.uploadProgressBar->setValue(100);
    d->_k_updatePage();
}

#include "uploaddialog.moc"
