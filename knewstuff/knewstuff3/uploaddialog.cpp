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
        bool finished;

        Private()
            :finished(false)
        {
        }
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
    d->ui.mFileNameLabel->setText(payloadFile.url());

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

bool UploadDialog::init(const QString &configfile)
{
    // populate dialog with stuff
    QWidget* _mainWidget = new QWidget(this);
    setMainWidget(_mainWidget);
    d->ui.setupUi(_mainWidget);
    connect(d->ui.mPreviewUrl, SIGNAL(urlSelected(const KUrl&)), SLOT(previewChanged(const KUrl&)));

    connect(&d->providerManager, SIGNAL(providerAdded(const Attica::Provider&)), SLOT(providerAdded(const Attica::Provider&)));
    d->providerManager.loadDefaultProviders();

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
    d->ui.mCategoryCombo->addItems(d->categoryNames);

    if (d->categoryNames.size() == 1) {
        d->ui.mCategoryLabel->setVisible(false);
        d->ui.mCategoryCombo->setVisible(false);
    }

    kDebug() << "Categories: " << d->categoryNames;

    connect(d->ui.priceCheckBox, SIGNAL(toggled(bool)), this, SLOT(priceToggled(bool)));
    priceToggled(false);

    return true;
}

void UploadDialog::priceToggled(bool priceEnabled)
{
    d->ui.priceLabel->setVisible(priceEnabled);
    d->ui.priceSpinBox->setVisible(priceEnabled);
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

    Q_FOREACH(const Attica::Category &category, categories) {
        if (d->categoryNames.contains(category.name())) {
            d->categories.append(category);
            kDebug() << "found category: " << category.name();
        }
        else {
            //kDebug() << "found invalid category: " << category.name();
        }
    }

    // at least one category is needed
    if (d->categories.count() == 0) {
        if (d->categoryNames.size() > 0) {
            KMessageBox::error(this,
                               i18n("The server does not know the category that you try to upload: %1", d->categoryNames.join(", ")),
                               i18n("Error"));
            // close the dialog
            reject();
        } else {
            kWarning() << "No category was set in knsrc file. Adding all categories.";
            Q_FOREACH(const Attica::Category &category, categories) {
                d->ui.mCategoryCombo->addItem(category.name());
                d->categoryNames.append(category.name());
            }
        }
    }

    button(Ok)->setEnabled(true);
    d->ui.mProgressLabel->clear();
}

void UploadDialog::accept()
{
    if (d->finished) {
        KDialog::accept();
        return;
    }

    if (!d->provider.isValid()) {
        KMessageBox::error(this, i18n("Provider could not be initialized."));
        return;
    }

    if (d->ui.mNameEdit->text().isEmpty()) {
        KMessageBox::error(this, i18n("Please fill out the name field."));
        return;
    }

    // check the category
    QString categoryName = d->ui.mCategoryCombo->currentText();
    QList<Attica::Category>::const_iterator iter = d->categories.constBegin();
    Attica::Category category;
    while (iter != d->categories.constEnd()) {
        if (iter->name() == categoryName) {
            category = *iter;
            break;
        }
        ++iter;
    }
    if (!category.isValid()) {
        KMessageBox::error(this, i18n("The selected category \"%1\" is invalid.", categoryName), i18n("Upload Failed"));
        return;
    }

    // go to the next page
    d->ui.stackedWidget->setCurrentIndex(1);
    d->ui.mProgressLabel->setText(i18n("Creating Content on Server..."));

    // fill in the content object
    Attica::Content content;
    content.setName(d->ui.mNameEdit->text());
    QString summary = d->ui.mSummaryEdit->toPlainText();
    content.addAttribute("description", summary);
    content.addAttribute("version", d->ui.mVersionEdit->text());
    content.addAttribute("license", d->ui.mLicenseCombo->currentText());

    // TODO: add additional attributes
    //content.addAttribute("changelog", ui.changelog->text());
    //content.addAttribute("downloadlink1", ui.link1->text());
    //content.addAttribute("downloadlink2", ui.link2->text());
    //content.addAttribute("homepage1", ui.homepage->text());
    //content.addAttribute("blog1", ui.blog->text());

    if (d->ui.priceCheckBox->isChecked()) {
        content.addAttribute("downloadbuy1", "1");
        content.addAttribute("downloadbuyprice1", QString::number(d->ui.priceSpinBox->value()));
        // TODO in the next version:
        // content.addAttribute("downloadbuyreason1", "the description why is content is not for free");
    }

    Attica::ItemPostJob<Attica::Content>* job = d->provider.addNewContent(category, content);
    connect(job, SIGNAL(finished(Attica::BaseJob*)), SLOT(contentAdded(Attica::BaseJob*)));
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
    //QMessageBox::information(0, i18n("Content Added"), id);
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
    d->ui.mProgressLabel->setText(i18n(d->ui.mProgressLabel->text() + "\n\n" + "Content successfully uploaded."));
    d->ui.mProgress->setVisible(false);
    d->finished = true;
    setButtons(KDialog::Ok);
}


#include "uploaddialog.moc"
