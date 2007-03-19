/***************************************************************************
 *   Copyright (C) 2006 by Riccardo Iaconelli  <ruphy@fsfe.org>            *
 *                                                                         *
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

#ifndef PLASMAGIK_H
#define PLASMAGIK_H

#include <qdialog.h>
#include "ui_plasmagikdesigned.h"


class PlasmagikPrivate;

class QIcon;
class QLabel;
class QGridLayout;
class KLineEdit;
class KTextEdit;
class KUrlRequester;
class QComboBox;
class QGroupBox;
class KUser;
class QStringList;
class QStackedLayout;
class QHBoxLayout;
class QPushButton;
class QVBoxLayout;
class KSeparator;
class KIcon;
class QLayoutItem;
class QStringListModel;
class KMimeType;
class KMessageBoxMessageHandler;

// TODO: Document the class

class Plasmagik : public QDialog
{
    Q_OBJECT
public:
    Plasmagik(const QString &appName, QWidget *parent = 0);
    ~Plasmagik();
    void fillFields();
    
    enum LabelFlag {
        WelcomeText     = 0x00000001,
        ReleaseNotes    = 0x00000002,
        //TODO: add text of other strings and popups here
    };
    Q_DECLARE_FLAGS(Label, LabelFlag);

    //This sets some 
    enum PackageTypeFlag {
        Plasmoid        = 0x00000001,
        KrossPackage    = 0x00000002,
        Artwork         = 0x00000004,
        QtScriptPackage = 0x00000008,
        Custom          = 0x00000010,
        GenericDocument = 0x00000020
    };
    Q_DECLARE_FLAGS(PackageType, PackageTypeFlag);

    //Set the text of the string "string" to the text "text"
    void setText(Plasmagik::Label label, const QString &text);

    // those methods are very similar. here you can set the mimetype or the extension
    // to save the package. NOTE that the extension won't be checked. (extension will be of the type '.ext')
    void setPackageExtension(const QString &extension);
    // Similar Function, but it takes a 
    void setPackageMimeType(const KMimeType &mimeType);

    //Sets the program icon. Default is app/plasmagik
    void setIcon(const QIcon &icon);

    // This adjust Plasmagik to handle the type of package to be created. You can specify more
    // than one package type. If you use Plasmagik::Custom you will need to define also
    // the following proprieties: 1) Whether to show or not the "what file should be loaded first" widget.
    // (TODO finish to write what needs to be defined is Plasmagik::Custom is set).
    // If clearFirst is set, clearProgrammingLanguages() will be called before setting the languages.
    // Don't set it if you specify more than one type at the same moment,
    // call clearContentList() instead.
    void setPackageType(Plasmagik::PackageType type, bool clearFirst);

    //TODO rename in setContentType, setLanguage or something like that.
    //Sets supported contents.
    //NOTE: This function will automatically clear any language previously set.
    //If you use the method that accepts a QString, languages must be separated by '|'
    //Ex: C++|Javascript|Python. NOTE: Languages for specific package types are autommatically set by setPackageType().
    void setContentTypes(const QString &items);
    void setContentTypes(const QStringList &items); //provided for convenience

    //Add supported contents at the end of the list.
    //SEEALSO: clearProgrammingLanguages();
    //If you use the method that accepts a QString, languages must be separated by '|'
    //Ex: C++|Javascript|Python. NOTE: Languages for specific package types are autommatically set by setPackageType().
    void addContentTypes(const QString &items);
    void addContentTypes(const QStringList &items); //provided for convenience

    //It deletes all the previously set languages (including the one autommatically set
    //by setPackageType()).
    void clearContentTypes();

protected:
    void accept();

private:
    PlasmagikPrivate *d;

    void initializeButtonsAndLayout();
    void initializeSpecialWidgets();
    void connectSignalsAndSlots();
    void updateButtons();
    void switchPage();
    QString generateMetadata();

    bool isOkToOverwrite();
    void warningOverwrite(const QString &url);

    bool licenceIsAcceptable();
    bool fieldsNotEmpty();
    bool fileToLoadExists();
    bool plasmoidExists();
    inline bool packagingHomeDir();

    void reAddHiddenWidgets();

private Q_SLOTS:
    void checkNotToBeOverwriting(const KUrl & url);
    void checkInfos();
    void plasmoidNameChanged(const QString &newtext);
    void plasmoidVersionChanged(const QString &newtext);

    void backButtonClicked();
    void nextButtonClicked();
    
    friend class PlasmagikPrivate;
};

class PlasmagikPrivate
{
private:
    PlasmagikPrivate(Plasmagik *parent);

    KMessageBoxMessageHandler *handler;

    QString m_appName;

    QStringList contentType;
    QStringListModel *contentTypeModel;

    QString m_licence;
    QString plasmoidSuggestedFilename;

    bool alreadyAnsweredOverwrite;
    int numPages;
    KUser *user;

    int currentPage;
    QPushButton *cancelButton;
    QPushButton *backButton;
    QPushButton *nextButton;
    QPushButton *finishButton;

    QHBoxLayout *buttonLayout;
    QVBoxLayout *mainLayout;
    KSeparator *simpleSeparator;

    KMimeType *m_mimetype;
    QString extension;
    QString packageType;

    bool widgetsRemoved;
    QSpacerItem *spacer;
    QWidgetItem *label, *urlRequester;

    Ui::PlasmagikDesignedWidget ui;

    friend class Plasmagik;
};
#endif
