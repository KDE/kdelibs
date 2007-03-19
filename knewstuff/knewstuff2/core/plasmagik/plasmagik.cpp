/***************************************************************************
 *   Copyright (C) 2006 by Riccardo Iaconelli  <ruphy@fsfe.org>            *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#include <QDomDocument>
#include <QDomImplementation>
#include <QtGui>

#include <klocale.h>
#include <kzip.h>
#include <kuser.h>
#include <kdirselectdialog.h>
#include <kmessage.h>
#include <kmessageboxmessagehandler.h>
#include <ktemporaryfile.h>
#include <kmessagebox.h>
#include <kicon.h>
#include <kseparator.h>
#include <kfiledialog.h>
#include <kstdguiitem.h>
#include <kstandarddirs.h>
#include <kdesktopfile.h>
#include <kpassivepopupmessagehandler.h>

#include "plasmagik.h"
#include "plasmagik.moc"

#include <kdebug.h>

PlasmagikPrivate::PlasmagikPrivate(Plasmagik *p /*parent*/)
{
    ui.setupUi(p);
    simpleSeparator = new KSeparator(Qt::Horizontal, p);
    user = new KUser;
    mainLayout = new QVBoxLayout;

    extension = "";
    m_mimetype = 0L;

    contentTypeModel = new QStringListModel();
//     contentType << "" << "" << "" << "" << "" << "";
    contentTypeModel->setStringList(contentType);
    ui.plasmoidLanguage->setModel(contentTypeModel);

    widgetsRemoved = false;

    numPages = 3;
    currentPage = 0;
    
}

Plasmagik::Plasmagik(const QString &appName, QWidget *parent)
   : QDialog(parent)
{
    d = new PlasmagikPrivate(this);

    d->m_appName = appName;
    d->extension = ".kns"; // Default extension for packages
    KDesktopFile *temp = new KDesktopFile(QString(KStandardDirs::locate("mime", "application/x-kns")));
    d->m_mimetype = new KMimeType(temp);
    delete temp;

    initializeButtonsAndLayout();

    d->mainLayout->addWidget(d->ui.mainWidget);
    d->mainLayout->addWidget(d->simpleSeparator);
    d->mainLayout->addLayout(d->buttonLayout);

    connectSignalsAndSlots();
    initializeSpecialWidgets();

    KMessageBoxMessageHandler *handler = new KMessageBoxMessageHandler;
    KMessage::setMessageHandler(handler);
//     KMessage::message(KMessage::Information, "Test KMessageBox");
//     handler->message(KMessage::Information, "Test KMessageBox", "");
//     KMessageBoxMessageHandler *handler= new KMessageBoxMessageHandler(this);
//     KMessage::setMessageHandler(handler);
//     KMessage::message(KMessage::Information, "KMessage test");
//     KMessage::setMessageHandler(new KMessageBoxMessageHandler(this));
//     KMessage::setMessageHandler(new KPassivePopupMessageHandler(this));

    setLayout(d->mainLayout);
    setWindowIcon(KIcon("plasmagik"));

    switchPage(); // We move to the first page

    resize(minimumSizeHint());
}

Plasmagik::~Plasmagik()
{
//     delete Label;
    kDebug() << "Bang" << endl;
    delete d;
    kDebug() << "Bangv2" << endl;
}

#warning remove this function before release.
void Plasmagik::fillFields() //WARNING: remove this before release
{
    d->ui.plasmoidName->setText("FooPlasmoid");
    d->ui.plasmoidAuthor->setText("Mr. Konqui");
    d->ui.plasmoidVersion->setText("0.1");
    d->ui.plasmoidHomepage->setText("http://www.kde.org");
    d->ui.plasmoidEmail->setText("konqui@kde.org");
    d->ui.plasmoidLicence->setCurrentIndex(1); //GPL
    d->ui.plasmoidLanguage->setCurrentIndex(1); //C++
    d->ui.plasmoidDescription->setText("The cooler plasmoid I've ever written.");
    d->ui.plasmoidNotes->setText("Just have fun! =D");
}

void Plasmagik::initializeButtonsAndLayout()
{
    d->cancelButton = new KPushButton(KIcon("no"), i18n("Quit"), this);
    d->backButton = new KPushButton(KIcon("arrow-left"), i18n("&Back"), this);
    d->nextButton = new KPushButton(KIcon("arrow-right"), i18n("&Next"), this);
    d->finishButton = new KPushButton(KIcon("ok"), i18n("&Package!"), this);

    d->buttonLayout = new QHBoxLayout;
    d->buttonLayout->addWidget(d->cancelButton);
    d->buttonLayout->insertStretch(1, 10);
    d->buttonLayout->addWidget(d->backButton);
    d->buttonLayout->insertStretch(3, 1);
    d->buttonLayout->addWidget(d->nextButton);
    d->buttonLayout->insertStretch(5, 2);
    d->buttonLayout->addWidget(d->finishButton);
}

void Plasmagik::initializeSpecialWidgets()
{
    //Second page
    d->ui.plasmoidHomepage->setText("http://");
    d->ui.plasmoidAuthor->setText(d->user->fullName());
    d->ui.plasmoidIcon->setFilter("image/png|image/svg");

    //Third page
    d->ui.destFile->setUrl(d->user->homeDir());
    d->ui.firstFile->setUrl(d->user->homeDir());
    d->ui.sourcePathRequester->setUrl(d->user->homeDir());
    d->ui.sourcePathRequester->setMode(KFile::Directory);
    d->ui.destFile->fileDialog()->setOperationMode(KFileDialog::Saving);
    d->ui.destFile->setMode(KFile::LocalOnly);
}

void Plasmagik::connectSignalsAndSlots()
{
    connect(d->ui.plasmoidName, SIGNAL(textChanged(const QString &)), this,
                             SLOT(plasmoidNameChanged(const QString &)));
    connect(d->ui.plasmoidVersion, SIGNAL(textChanged(const QString &)), this,
                                SLOT(plasmoidVersionChanged(const QString &)));

    connect(d->ui.destFile, SIGNAL(urlSelected(const KUrl &)), this,
                         SLOT(checkNotToBeOverwriting(const KUrl &)));

    connect(d->cancelButton, SIGNAL(clicked()), this, SLOT(reject()));
    connect(d->backButton, SIGNAL(clicked()), this, SLOT(backButtonClicked()));
    connect(d->nextButton, SIGNAL(clicked()), this, SLOT(nextButtonClicked()));
    connect(d->finishButton, SIGNAL(clicked()), this, SLOT(checkInfos()));
}

void Plasmagik::plasmoidNameChanged(const QString &newtext)
{
    d->plasmoidSuggestedFilename = d->user->homeDir();
    d->plasmoidSuggestedFilename += '/';
    d->plasmoidSuggestedFilename += newtext;
    d->plasmoidSuggestedFilename += '-';
    d->plasmoidSuggestedFilename += d->ui.plasmoidVersion->text();
    d->plasmoidSuggestedFilename += d->extension;
    d->ui.destFile->lineEdit()->setText(d->plasmoidSuggestedFilename);
}

void Plasmagik::plasmoidVersionChanged(const QString &newtext)
{
    d->plasmoidSuggestedFilename = d->user->homeDir();
    d->plasmoidSuggestedFilename += '/';
    d->plasmoidSuggestedFilename += d->ui.plasmoidName->text();
    d->plasmoidSuggestedFilename += '-';
    d->plasmoidSuggestedFilename += newtext;
    d->plasmoidSuggestedFilename += d->extension;
    d->ui.destFile->lineEdit()->setText(d->plasmoidSuggestedFilename);
}


void Plasmagik::backButtonClicked()
{
    d->currentPage--;
    switchPage();
}

void Plasmagik::nextButtonClicked()
{
//     d->nextButton->setEnabled(true);
//     d->finishButton->setEnabled(d->history.size() == d->numPages - 1);

    if (d->currentPage == 1 && /* We are moving from the second to the third page */
        d->ui.plasmoidLicence->currentIndex() == 10 /* Licence is 'custom' */) {
        KMessage::message(KMessage::Information,
                          i18n("You choose to use a custom licence. Please remember to add "
                               "a file in the root directory of your plasmoid called "
                               "\"COPYING\" that contains your licence"));
    }

    d->currentPage++;
    switchPage();
}


void Plasmagik::switchPage()
{
    //NOTE that currentPage has already been changed by
    //nextButtonClicked and backButtonClicked.
    d->ui.mainWidget->setCurrentIndex(d->currentPage);
    updateButtons();
    setWindowTitle(i18n("Plasmagik - Step %1 of %2",
                       d->currentPage + 1,
                       d->numPages + 1));
}

void Plasmagik::updateButtons()
{
    //Set the status of the button 'Back'.
    if (d->currentPage == 0) {
        d->backButton->setEnabled(false);
        d->nextButton->setDefault(true);
    } else {
        d->backButton->setEnabled(true);
    }
    //Set the status of 'Next' and 'Finish' buttons.
    if (d->currentPage == d->numPages) {
        d->finishButton->setEnabled(true);
        d->nextButton->setEnabled(false);
    } else {
        d->nextButton->setEnabled(true);
        d->finishButton->setEnabled(false);
    }
}

void Plasmagik::checkNotToBeOverwriting(const KUrl &url)
{
   QFile f(url.path());

    if (f.exists()) {
        warningOverwrite(url.path());
        d->alreadyAnsweredOverwrite = true;
    }
}

//This just displays a warning that the chosen output file qould be overwritten.
void Plasmagik::warningOverwrite(const QString &url)
{
//     KMessageBox::information(this, i18n("The file %1 already exists. "
//               "If you continue, this file will be silently overwritten.", url));
}

// Called in checkInfos()
bool Plasmagik::isOkToOverwrite()
{
//     return ( (KMessageBox::warningContinueCancel(this, i18n("The file %1 already exists. "
//               "Do you want to overwrite it?", d->ui.destFile->lineEdit()->text()),
//               QString(), KGuiItem(i18n("Overwrite")), QString())) 
//               == KMessageBox::Ok);
    return false;
#warning FIXME
}

QString Plasmagik::generateMetadata()
{
    QString pathToFirstFile;
    //FIXME check
    if (!d->widgetsRemoved) {
        // use path() because we are not sure to parse things in the right way if
        // 'file://' (or anything else) is prepended.
        pathToFirstFile = d->ui.firstFile->url().path();
        QString basePath = d->ui.sourcePathRequester->url().path();

        // If the plasmoid is C++ remove any extension to the name
        // stored in metadata.xml. We will NOT rename the file.
       if (d->ui.plasmoidLanguage->currentText() == "C++") {
            QString *tempFileName = new QString(d->ui.firstFile->url().fileName());
            QString *tempDirName = new QString(d->ui.firstFile->url().path());

            // remove the file name from the string
            tempDirName->truncate(tempDirName->lastIndexOf(*tempFileName));
            // remove any extesion from the filename (again, _we will NOT rename the file itself_,
            // the extension is just removed from the filename stored in metadata.xml)
            tempFileName->truncate(tempFileName->lastIndexOf('.'));

            pathToFirstFile = *tempDirName + *tempFileName;

            delete tempFileName;
            delete tempDirName;
        }

        // extract the relative path to the first file to be loaded from the absolute path
        // with a '/' as first char. At this moment / is the root directory of the sources of the plasmoid.
        pathToFirstFile.remove(pathToFirstFile.indexOf(basePath),
                          basePath.size());

        if (pathToFirstFile.indexOf('/') != 0) {
            pathToFirstFile.prepend('/');
        }

        // to have the relative path form the point of view the file metadata.xml
        // prepend the name of the folder that contains the plasmoid sources.
        // that is the name of the plasmoid itself.
        pathToFirstFile.prepend(d->ui.plasmoidName->text());
    }

    KTemporaryFile tempFile;
    KDesktopFile desktopFile(tempFile.fileName());

    desktopFile.writeEntry("Name", d->ui.plasmoidName->text());
    desktopFile.writeEntry("Description", d->ui.plasmoidName->text());
    desktopFile.writeEntry("Author", d->ui.plasmoidName->text());
    desktopFile.writeEntry("Email", d->ui.plasmoidName->text());
    desktopFile.writeEntry("Version", d->ui.plasmoidName->text());
    desktopFile.writeEntry("Website", d->ui.plasmoidName->text());
    desktopFile.writeEntry("License", d->ui.plasmoidName->text());
    desktopFile.writeEntry("FirstFile", d->ui.plasmoidName->text());
    desktopFile.writeEntry("ApplicationName", d->m_appName);

    QString code;

//     code = "[Desktop Entry]\n"
//            "Name="+d->ui.plasmoidName->text()+"-"+d->ui.plasmoidVersion->text()+"\n"
//            "Comment="+d->ui.plasmoidDescription->text()+"\n"
//            "\n"
//            "[X-KDE Plugin Info]"
//            "Author="+d->ui.plasmoidAuthor->text()+"\n"
//            "Email="+d->ui.plasmoidEmail->text()+"\n"
//            "PluginName="
//            "Version="+1.1
//            "Website="+http://www.plugin.org/
//            "Category="+playlist
// //            Depends=plugin1,plugin3
//            "License="+d->m_licence+"\n"
// //            EnabledByDefault=true
//            "FirstFile="+pathToFirstFile+"\n"

    //We store all the data into a DOM document extracting them
    //directly from the second and the third page.
    QDomImplementation docType;
    QDomDocument doc(docType.createDocumentType("Plasmoid", QString(), QString()));
    QDomElement root = doc.createElement("magicpkg");

    QDomElement nameTag = doc.createElement("name");
    QDomText name = doc.createTextNode(d->ui.plasmoidName->text());

    QDomElement descriptionTag = doc.createElement("description");
    QDomText description = doc.createTextNode(d->ui.plasmoidDescription->text());

    //When I talk about language, I mean programming language ;-)
    QDomElement languageTag = doc.createElement("language");
    QDomText language = doc.createTextNode(d->ui.plasmoidLanguage->currentText());

    QDomElement firstLoadedFileTag = doc.createElement("first_file");
    QDomText firstLoadedFile = doc.createTextNode(pathToFirstFile);

    QDomElement authorTag = doc.createElement("author");
    QDomText author = doc.createTextNode(d->ui.plasmoidAuthor->text());

    QDomElement versionTag = doc.createElement("version");
    QDomText version = doc.createTextNode(d->ui.plasmoidVersion->text());

    QDomElement homepageTag = doc.createElement("homepage");
    QDomText homepage = doc.createTextNode(d->ui.plasmoidHomepage->text());

    QDomElement emailTag = doc.createElement("email");
    QDomText email = doc.createTextNode(d->ui.plasmoidEmail->text());

    QDomElement licenceTag = doc.createElement("licence");
    //Here we will use m_licence, that is the name of the licence untranslated.
    //We have set it when we was checking for an acceptable licence.
    QDomText licence = doc.createTextNode(d->m_licence);

    QDomElement releaseNotesTag = doc.createElement("release_notes");
    QDomText releaseNotes = doc.createTextNode(d->ui.plasmoidNotes->toPlainText());

    doc.appendChild(root);

    root.appendChild(nameTag);
    nameTag.appendChild(name);

    root.appendChild(descriptionTag);
    descriptionTag.appendChild(description);

    root.appendChild(languageTag);
    languageTag.appendChild(language);

    root.appendChild(firstLoadedFileTag);
    firstLoadedFileTag.appendChild(firstLoadedFile);

    root.appendChild(authorTag);
    authorTag.appendChild(author);

    root.appendChild(versionTag);
    versionTag.appendChild(version);

    root.appendChild(homepageTag);
    homepageTag.appendChild(homepage);

    root.appendChild(emailTag);
    emailTag.appendChild(email);

    root.appendChild(licenceTag);
    licenceTag.appendChild(licence);

    root.appendChild(releaseNotesTag);
    releaseNotesTag.appendChild(releaseNotes);

    code = doc.toString();
    return code;
}

void Plasmagik::accept()
{
    QString metadata;
    metadata = generateMetadata();

    KTemporaryFile *tmpXML = new KTemporaryFile;
    QFile data(tmpXML->fileName());

    //We just write the content of the QString containing the metadata in an
    //empty temporary file that we will package with the name metadata.xml
    if (data.open(QFile::WriteOnly | QFile::Truncate)) 
    {
        QTextStream out(&data);
        out << metadata;
        data.close();
    }

    //OK, we've got the temporary file with the metadata in it.
    //Now we just need to put everything into a zip archive.
    KZip plasmoid(d->ui.destFile->lineEdit()->text());
    plasmoid.setCompression(KZip::NoCompression);

    if (plasmoid.open(QIODevice::WriteOnly))
    {
        // put the metadata.xml file first to provide a faster reading.
        plasmoid.addLocalFile(tmpXML->fileName(),
                              "metadata.xml");

        // add the icon if the user provided it.
        if (!d->ui.plasmoidIcon->url().fileName().isEmpty())
        {
            plasmoid.addLocalFile(d->ui.plasmoidIcon->lineEdit()->text(),
                              "icon.png");
        }

        plasmoid.addLocalDirectory(d->ui.sourcePathRequester->lineEdit()->text(),
                                   d->ui.plasmoidName->text());
        plasmoid.close();

        KMessage::message(KMessage::Information, 
                          "<qt>"+i18nc("The argument is a filename", "Your plasmoid has been "
                          "successfully packaged!<br>You can find it at: <b>%1</b>",
                          d->ui.destFile->lineEdit()->text())+"</qt>",
                          i18n("Plasmoid Packaged!") );
    }
    else
    {
        KMessage::message(KMessage::Error, i18n("Some problem occoured during the packaging operation."));
    }

    delete tmpXML;

    QDialog::accept();
}

void Plasmagik::checkInfos()
{
    // Note that I didn't use the "else if" structure, because is harder to
    // show popups to be sure the user knows what (s)he is doing. ;-)
    // Note that is the same, because we return if the user decides to interrupt.
    // Just some test.
    if (!fieldsNotEmpty()) 
    {
        KMessage::message( KMessage::Sorry, i18n("Sorry, you must fill in all the fields before we can "
                           "package your plasmoid. Please fill in all the informations."),
                           i18n("Have you filled in all the fields?"));
        return;
    }

    // Always show this message.
    if (KMessageBox::warningContinueCancel(this,
                     "<qt>"+i18n("You are going to package the %1 folder <br>"
                     "Are you sure you want to continue?",
                     d->ui.sourcePathRequester->lineEdit()->text())+"</qt>") == KMessageBox::Cancel)
    {
        return;
    }

    if (!d->alreadyAnsweredOverwrite)
    {
        if (plasmoidExists())
        {
            if (!isOkToOverwrite())
            return;
        }
    }

    if (!licenceIsAcceptable()) 
    {
        KMessage::message(KMessage::Sorry, "<qt>"+i18nc("Do _NOT_ translate 'COPYING'",
                          "Sorry, we cannot create the package of your plasmoid because we cannot find "
                          "any acceptable licence.\n Have you specified a valid licence? If you wish to "
                          "use a custom licence, you must make sure to include it in a file called <b>COPYING</b>.")+"</qt>",
                          i18n("Could not find any acceptable licence") );
        return;
    }

    if (!fileToLoadExists())
    {
        KMessage::message(KMessage::Error, i18n("The file you have specified does not exists. Please "
                          "make sure that you've chosen the right file."));
        return;
    }

    if (packagingHomeDir())
    {
        #warning use the standard GUI item for 'continue' (line 496)
        if ((KMessageBox::warningContinueCancel(this,"<qt>"+
            i18n("<b style=\"color: red\">***WARNING, WARNING, WARNING!!!***</b><br>"
            "It seems that you are going to package your ENTIRE home directory. This is not what "
            "you want in the 99.999% of the cases, and in any case it will take a lot of space "
            "on the disk and a lot of time to complete.<br>"
            "Are you sure you want to continue?")
            +"<qt>", QString(), KGuiItem(i18n("continue"))/*KStdGuiItem::cont()*/, QString(),
            KMessageBox::Dangerous)) == KMessageBox::Cancel)
           return;
    }

    // The line beow is executed only if all the conditions are met
    // (else you will find a 'return' instruction in your way)
    accept(); //This is Plasmagik::accept(), not QDialog::accept()
}

bool Plasmagik::licenceIsAcceptable()
{
    int indexLicence = d->ui.plasmoidLicence->currentIndex();

    //m_licence is a string that contains the name of the chosen licence untraslated.
    //we store it to make use in the generateXML() function.
    switch (indexLicence) {
    case 0: { //Empty
        return false;
    }
    case 1: {
        d->m_licence = "GPL";
        return true;
    }
    case 2: {
        d->m_licence = "LGPL";
        return true;
    }
    case 3: {
        d->m_licence = "BSD";
        return true;
    }
    case 4: {
        d->m_licence = "MPL";
        return true;
    }
    case 5: {
        d->m_licence = "Creative Commons BY";
        return true;
    }
    case 6: {
        d->m_licence = "Creative Commons BY-SA";
        return true;
    }
    case 7: {
        d->m_licence = "Creative Commons BY-ND";
        return true;
    }
    case 8: {
        d->m_licence = "Creative Commons BY-ND-NC";
        return true;
    }
    case 9: {
        d->m_licence = "Creative Commons BY-NC-SA";
        return true;
    }
    case 10: { //Custom licence
        KUrl sourcePath = d->ui.sourcePathRequester->url();
        sourcePath.addPath("COPYING");
        QFile copyingFile(sourcePath.path());

        if (!copyingFile.exists()) {
            d->m_licence = QString();
            return false;
        }

        d->m_licence = "Custom";
        return true;
    }
    default: {//This shouldn't happen. Never.
        return false;
    }
    }
}

bool Plasmagik::fileToLoadExists()
{
    KUrl filePath = d->ui.firstFile->url();
    QFile f(filePath.path());
    if (f.exists()) return true;

    return false;
}

inline bool Plasmagik::packagingHomeDir()
{
    //We clean the path wiht those 2 lines as much as we can to ensure
    //that the comparison happens in a right way.
    //(KUser::homeDir() is QString, not KUrl)
    d->ui.sourcePathRequester->url().adjustPath(KUrl::RemoveTrailingSlash);
    d->ui.sourcePathRequester->url().cleanPath();

    if (d->ui.sourcePathRequester->url() == KUrl(d->user->homeDir()))
        return true;
    else return false;
}

bool Plasmagik::fieldsNotEmpty()
{
    //TODO: Make it smarter if we can.
    if (d->ui.plasmoidName->text().isEmpty() ||
        d->ui.plasmoidAuthor->text().isEmpty() ||
        d->ui.plasmoidVersion->text().isEmpty() ||
        d->ui.plasmoidEmail->text().isEmpty() ||
        d->ui.plasmoidLicence->currentIndex() == 0 ||
        d->ui.plasmoidLanguage->currentIndex() == 0 ||
        d->ui.plasmoidDescription->text().isEmpty() ||
        d->ui.sourcePathRequester->url().prettyUrl().isEmpty() ||
        d->ui.destFile->url().fileName().isEmpty() ||
        (d->ui.firstFile->url().fileName().isEmpty() && !d->widgetsRemoved)
        )
    return false;

    else return true;
}

bool Plasmagik::plasmoidExists()
{
    QFile plasmoid(d->ui.destFile->url().path());

    if (!plasmoid.exists())
    {
        return false;
    }

    return true;
    inline void setContentTypes(const QStringList &items, int position = 0); //provided for convenience
}

void Plasmagik::reAddHiddenWidgets()
{
    d->ui.firstFileFrame->setVisible(true);
    d->widgetsRemoved = false;
}

//Public functions for the API not to be moved to PlasmagikPrivate.

void Plasmagik::clearContentTypes()
{
    int count = d->contentTypeModel->rowCount();
    if (count > 0) {
        d->contentTypeModel->removeRows(0, count);
    }
}

void Plasmagik::addContentTypes(const QString &items)
{
//     QString temp_items = items;
//     if (position == 0 && temp_items.indexOf('|') != 1)
//     {
//         temp_items.prepend('|');
//     }
    addContentTypes(items.split('|'));
}

void Plasmagik::addContentTypes(const QStringList &items)
{
//     int numNeededRows = position + items.size();
    if (items.size() > 0) {
        d->contentTypeModel->insertRows(d->contentTypeModel->rowCount(), items.size());
        QString item;
        int i = 0;
        QModelIndex index;

        foreach(item, items) {
            index = d->contentTypeModel->index(i);
            d->contentTypeModel->setData(index, item);
            i++;
        }

    } else {
        kWarning() << "No content type has been added" << endl;
    }
}

void Plasmagik::setContentTypes(const QString &items)
{
    setContentTypes(items.split('|'));
}

void Plasmagik::setContentTypes(const QStringList &items)
{
    clearContentTypes();
    addContentTypes(items);
}

void Plasmagik::setPackageExtension(const QString &extension)
{
    d->extension = extension;
}

void Plasmagik::setIcon(const QIcon &icon)
{
    setIcon(icon);
}

void Plasmagik::setText(Plasmagik::Label label, const QString &text)
{
    if(label.testFlag(Plasmagik::WelcomeText))  {
        d->ui.firstLabel->setText(text);
    } else if(label.testFlag(Plasmagik::ReleaseNotes)) {
        d->ui.releaseNotesLabel->setText(text);
    }
}

//TODO: maybe add some more packages/proprieties to set?
void Plasmagik::setPackageType(Plasmagik::PackageType type, bool clearFirst)
{
    if (type.testFlag(Plasmagik::QtScriptPackage))
    {
        kDebug() << "Package type is: QtScript" << endl;
        if (clearFirst) {
            clearContentTypes();
        }
        addContentTypes(QString("|QtScript - Javascript"));
        d->packageType = "QtScript";

        if (d->widgetsRemoved) {
            d->ui.firstFileFrame->setVisible(true);
            d->widgetsRemoved = false;
        }
    }
    else if (type.testFlag(Plasmagik::KrossPackage))
    {
        kDebug() << "Package type is: Kross Package" << endl;
        clearContentTypes();
        setContentTypes("|Kross - Javascript (KJS)|Kross - Python|Kross - Ruby");
        d->packageType = "Kross";

        if (d->widgetsRemoved) {
            d->ui.firstFileFrame->setVisible(true);
            d->widgetsRemoved = false;
        }
    }
    else if (type.testFlag(Plasmagik::Artwork))
    {
        kDebug() << "Package type is: Artwork" << endl;
        clearContentTypes();
        setContentTypes(QString("|Generic Artwork"));
        d->packageType = "Artwork";

        d->ui.firstFileFrame->setVisible(false);

        //this is set because the application could decide to change the package type
        //and we are aware that widgets need to be re-added.
        d->widgetsRemoved = true;
    }
    else if (type.testFlag(Plasmagik::Custom))
    {
        kDebug() << "Package type is: Custom" << endl;
        kDebug() << "Remember, you are responsible to set the other needed proprieties!" << endl;
        d->packageType = "Custom";

        if (d->widgetsRemoved) {
            d->ui.firstFileFrame->setVisible(true);
            d->widgetsRemoved = false;
        }
    }
}







