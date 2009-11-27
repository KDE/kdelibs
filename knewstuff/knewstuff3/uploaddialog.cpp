/*
    knewstuff3/ui/uploaddialog.cpp.
    Copyright (c) 2002 Cornelius Schumacher <schumacher@kde.org>
    Copyright (c) 2009 Jeremy Whiting <jpwhiting@kde.org>
    Copyright (C) 2009 Frederik Gladhorn <gladhorn@kde.org>

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

#include <QtGui/QLabel>
#include <QtGui/QLayout>
#include <QtGui/QDoubleSpinBox>
#include <QtCore/QString>

#include <kaboutdata.h>
#include <kcombobox.h>
#include <kcomponentdata.h>
#include <kconfig.h>
#include <kglobal.h>
#include <klineedit.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <ktextedit.h>
#include <kurlrequester.h>
#include <kuser.h>
#include <kstandarddirs.h>

#include <kdebug.h>
#include <kconfiggroup.h>

#include <kwallet.h>

#include <attica/providermanager.h>
#include <attica/provider.h>
#include <attica/category.h>
#include <attica/content.h>
#include <attica/listjob.h>
#include <attica/postjob.h>

#include "ui_uploaddialog.h"

namespace KNS3 {
    class UploadDialog::Private
    {
    public:
        Ui::UploadDialog ui;

        Attica::ProviderManager providerManager;
        Attica::Provider provider;
        Attica::Category::List categories;
        KUrl uploadFile;
        KUrl previewFile;
        QStringList categoryNames;
        QString contentId;
        KWallet::Wallet* wallet;
    };
}

using namespace KNS3;

UploadDialog::UploadDialog(QWidget *parent)
    : KDialog(parent), d(new Private)
{
    KComponentData component = KGlobal::activeComponent();
    QString name = component.componentName();
    init(name + ".knsrc");
}

UploadDialog::UploadDialog(const QString& configFile, QWidget *parent)
    : KDialog(parent), d(new Private)
{
    init(configFile);
}

UploadDialog::~UploadDialog()
{
    delete d;
}

void UploadDialog::setUploadFile(const KUrl& payloadFile)
{
    d->uploadFile = payloadFile;
    d->ui.mFileNameLabel->setText(i18nc("Label showing the file name of a file that is about to be uploaded", "Uploading: %1", payloadFile.toLocalFile()));
}

bool UploadDialog::init(const QString &configfile)
{
    // popuplate dialog with stuff
    QWidget* _mainWidget = new QWidget(this);
    setMainWidget(_mainWidget);
    d->ui.setupUi(_mainWidget);
    connect(d->ui.mPreviewUrl, SIGNAL(urlSelected(const KUrl&)), SLOT(previewChanged(const KUrl&)));

    connect(&d->providerManager, SIGNAL(providerAdded(const Attica::Provider&)), SLOT(providerAdded(const Attica::Provider&)));
    d->providerManager.loadDefaultProviders();

    d->wallet = KWallet::Wallet::openWallet(KWallet::Wallet::NetworkWallet(), 0);
    d->wallet->createFolder("Attica");
    d->wallet->setFolder("Attica");

    setCaption(i18n("Share Hot New Stuff"));
    setButtons(Ok | Cancel);
    showButtonSeparator(true);

    setButtonText(Ok, i18n("Upload..."));
    button(Ok)->setEnabled(false);

    d->ui.mTitleWidget->setText(i18nc("Program name followed by 'Add On Uploader'",
                                 "%1 Add-On Uploader",
                                 KGlobal::activeComponent().aboutData()->programName()));
    d->ui.mTitleWidget->setPixmap(KIcon(KGlobal::activeComponent().aboutData()->programIconName()));
    d->ui.mProgress->setVisible(false);
    d->ui.mProgressLabel->setText(i18n("Fetching provider information..."));

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

    kDebug() << "Categories: " << d->categoryNames;

    //d->providerFileUrl = group.readEntry("ProvidersUrl", QString());
    //d->applicationName = QFileInfo(KStandardDirs::locate("config", configfile)).baseName() + ':';

    return true;
}

void UploadDialog::providerAdded(const Attica::Provider& provider)
{
    // we only care about opendesktop for now
    if (provider.baseUrl() != QUrl("https://api.opendesktop.org/v1/")) {
        return;
    }
    d->provider = provider;
    Attica::ListJob<Attica::Category>* job = d->provider.requestCategories();
    connect(job, SIGNAL(finished(Attica::BaseJob*)), SLOT(categoriesLoaded(Attica::BaseJob*)));
    job->start();

    d->ui.mServerNameLabel->setText(d->provider.name());
}

void UploadDialog::categoriesLoaded(Attica::BaseJob* job)
{
    kDebug() << "Loading Categories..." << d->categoryNames;

    Attica::ListJob<Attica::Category>* listJob = static_cast<Attica::ListJob<Attica::Category>*>(job);
    Attica::Category::List categories = listJob->itemList();

    Q_FOREACH(Attica::Category category, categories) {
        if (d->categoryNames.contains(category.name())) {
            d->categories.append(category);
            kDebug() << "found category: " << category.name();
        }
    }
    button(Ok)->setEnabled(true);
    d->ui.mProgressLabel->clear();
}

void UploadDialog::accept()
{
    if (!d->provider.isValid()) {
        KMessageBox::error(this, i18n("Provider could not be initialized."));
        return;
    }

    if (d->ui.mNameEdit->text().isEmpty()) {
        KMessageBox::error(this, i18n("Please put in a name."));
        return;
    }

    d->ui.mProgress->setVisible(true);
    d->ui.mProgress->setMinimum(0);
    d->ui.mProgress->setMaximum(0);
    d->ui.mProgressLabel->setText(i18n("Creating Content on Server..."));
    d->ui.uploadGroup->setEnabled(false);

    // fill in the content object
    Attica::Content content;
    content.setName(d->ui.mNameEdit->text());
    QString summary = d->ui.mSummaryEdit->toPlainText();
    content.addAttribute("description", summary);
    content.addAttribute("version", d->ui.mVersionEdit->text());
    content.addAttribute("license", d->ui.mLicenseCombo->currentText());

    Attica::ItemPostJob<Attica::Content>* job = d->provider.addNewContent(d->categories.first(), content);
    connect(job, SIGNAL(finished(Attica::BaseJob*)), SLOT(contentAdded(Attica::BaseJob*)));
    job->start();

    /*
    details.insert("user", m_settingsWidget.userEdit->text());
    details.insert("password", m_settingsWidget.passwordEdit->text());
    m_wallet->writeMap(m_provider.baseUrl().toString(), details);
    */
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


    d->ui.mProgressLabel->setText(i18n("Uploading preview and content..."));

    Attica::ItemPostJob<Attica::Content> * job = static_cast<Attica::ItemPostJob<Attica::Content> *>(baseJob);
    QString id = job->result().id();
    QMessageBox::information(0, "Content Added", id);

    d->contentId = id;

    d->ui.mProgressLabel->setText(i18n("Uploading content..."));
    doUpload(QString(), d->uploadFile.toLocalFile());

    if (!d->previewFile.isEmpty()) {
        d->ui.mProgressLabel->setText(i18n("Uploading preview image and content..."));
        doUpload("1", d->previewFile.toLocalFile());
    }
}

void UploadDialog::doUpload(const QString& index, const QString& path)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        KMessageBox::information(this, "Upload", QString("File not found: %1").arg(d->uploadFile.url()));
        return;
    }

    QByteArray fileContents;
    fileContents.append(file.readAll());
    file.close();

    QString fileName = QFileInfo(path).fileName();

    Attica::PostJob* job;
    if (index.isEmpty()) {
        job = d->provider.setDownloadFile(d->contentId, fileName, fileContents);
        connect(job, SIGNAL(finished(Attica::BaseJob*)), SLOT(fileUploadFinished(Attica::BaseJob*)));
    } else {
        job = d->provider.setPreviewImage(d->contentId, index, fileName, fileContents);
        connect(job, SIGNAL(finished(Attica::BaseJob*)), SLOT(previewUploadFinished(Attica::BaseJob*)));
    }

    job->start();
}

void UploadDialog::fileUploadFinished(Attica::BaseJob* )
{
    KMessageBox::information(0, "Content Added", "File Uploaded");
    d->ui.mProgress->setVisible(false);

}


#include "uploaddialog.moc"
