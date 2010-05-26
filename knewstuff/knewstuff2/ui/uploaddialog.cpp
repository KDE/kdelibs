/*
    This file is part of KNewStuff2.
    Copyright (c) 2002 Cornelius Schumacher <schumacher@kde.org>

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

#include <kdebug.h>

//#include "engine.h"
#include "knewstuff2/core/entry.h"
#include "knewstuff2/core/author.h"

#include <kconfiggroup.h>

using namespace KNS;

UploadDialog::UploadDialog(/*Engine *engine,*/ QWidget *parent) :
        KDialog(parent)
{
    m_entry = NULL;

    // popuplate dialog with stuff
    QWidget* _mainWidget = new QWidget(this);
    setMainWidget(_mainWidget);
    setupUi(_mainWidget);

    setCaption(i18n("Share Hot New Stuff"));
    setButtons(Ok | Cancel);
    setDefaultButton(Cancel);
    setModal(false);

    mTitleWidget->setText(i18nc("Program name followed by 'Add On Uploader'",
                                 "%1 Add-On Uploader",
                                 KGlobal::activeComponent().aboutData()->programName()));
    mTitleWidget->setPixmap(KIcon(KGlobal::activeComponent().aboutData()->programIconName()));

    QStringList languagecodes = KGlobal::locale()->languageList();
    for (int i = 0; i < languagecodes.count(); i++) {
        QString languagecode = languagecodes.at(i);
        QString language = KGlobal::locale()->languageCodeToName(languagecode);
        mLanguageCombo->addItem(language);
        m_languages.insert(language, languagecode);
    }

    KUser user;
    mAuthorEdit->setText(user.property(KUser::FullName).toString());

    connect(this, SIGNAL(okClicked()), this, SLOT(slotOk()));
}

UploadDialog::~UploadDialog()
{
//qDeleteAll(mEntryList);
//mEntryList.clear();
}

void UploadDialog::slotOk()
{
    if (mNameEdit->text().isEmpty()) {
        KMessageBox::error(this, i18n("Please put in a name."));
        //return;
        reject(); // FIXME - huh? return should work here but it accept()s!
    }

    QString language = m_languages.value(mLanguageCombo->currentText());

    Author author;
    author.setName(mAuthorEdit->text());
    author.setEmail(mEmailEdit->text());

    KTranslatable previewurl;
    KUrl purl = mPreviewUrl->url();
    //purl.setFileName(QString());
    // FIXME: what does this do?
    previewurl.addString(language, purl.url());

    KTranslatable summary;
    summary.addString(language, mSummaryEdit->toPlainText());

    KTranslatable name;
    name.addString(language, mNameEdit->text());

    m_entry = new Entry;
    m_entry->setName(name);
    m_entry->setAuthor(author);
    m_entry->setVersion(mVersionEdit->text());
    m_entry->setLicense(mLicenseCombo->currentText());
    m_entry->setPreview(previewurl);
    m_entry->setSummary(summary);

    if (mPayloadUrl.isValid()) {
        KConfigGroup cg(KGlobal::config(), QString("KNewStuffUpload:%1").arg(mPayloadUrl.fileName()));
        cg.writeEntry("name", mNameEdit->text());
        cg.writeEntry("author", mAuthorEdit->text());
        cg.writeEntry("author-email", mEmailEdit->text());
        cg.writeEntry("version", mVersionEdit->text());
        cg.writeEntry("license", mLicenseCombo->currentText());
        cg.writeEntry("preview", mPreviewUrl->url().url());
        cg.writeEntry("summary", mSummaryEdit->toPlainText());
        cg.writeEntry("language", mLanguageCombo->currentText());
        KGlobal::config()->sync();
    }

    accept();
}

void UploadDialog::setPreviewFile(const KUrl& previewFile)
{
    mPreviewUrl->setUrl(previewFile);
}

void UploadDialog::setPayloadFile(const KUrl& payloadFile)
{
    mPayloadUrl = payloadFile;

    KConfigGroup cg(KGlobal::config(), QString("KNewStuffUpload:%1").arg(mPayloadUrl.fileName()));
    QString name = cg.readEntry("name");
    QString author = cg.readEntry("author");
    QString email = cg.readEntry("author-email");
    QString version = cg.readEntry("version");
    KUrl preview(cg.readEntry("preview"));
    QString summary = cg.readEntry("summary");
    QString lang = cg.readEntry("language");
    QString license = cg.readEntry("license");

    if (!name.isNull()) {
        int prefill = KMessageBox::questionYesNo(this,
                      i18n("Old upload information found, fill out fields?"),
                      QString(),
                      KGuiItem(i18n("Fill Out")),
                      KGuiItem(i18n("Do Not Fill Out")));
        if (prefill == KMessageBox::Yes) {
            mNameEdit->setText(name);
            mAuthorEdit->setText(author);
            mEmailEdit->setText(email);
            mVersionEdit->setText(version);
            //mReleaseSpin->setValue(release.toInt());
            mPreviewUrl->setUrl(preview);
            mSummaryEdit->setPlainText(summary);
            if (!lang.isEmpty()) mLanguageCombo->setCurrentIndex(mLanguageCombo->findText(lang));
            if (!license.isEmpty()) mLicenseCombo->setCurrentIndex(mLicenseCombo->findText(license));
        }
    }
}

Entry *UploadDialog::entry() const
{
    return m_entry;
}

#include "uploaddialog.moc"
