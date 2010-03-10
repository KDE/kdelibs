/*
    Copyright (C) 2010 Frederik Gladhorn <gladhorn@kde.org>

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

#ifndef KNEWSTUFF3_UI_UPLOADDIALOG_P_H
#define KNEWSTUFF3_UI_UPLOADDIALOG_P_H

#include <attica/providermanager.h>
#include <attica/provider.h>
#include <attica/category.h>
#include <attica/content.h>
#include <attica/listjob.h>
#include <attica/postjob.h>

#include "ui_uploaddialog.h"

#define FinishButton KDialog::User1
#define NextButton KDialog::User2
#define BackButton KDialog::User3

namespace KNS3 {
    class UploadDialog::Private
    {
    public:
        Private(UploadDialog* q)
            :q(q), currentPage(UserPasswordPage), finished(false)
        {
        }

        Attica::Provider currentProvider()
        {
            QString name(ui.providerComboBox->currentText());
            if (name.isEmpty()) {
                return Attica::Provider();
            }
            return providers[name];
        }

        UploadDialog* q;

        enum WizardPage {
            UserPasswordPage,
            FileNewUpdatePage,
            Details1Page,
            Details2Page,
            UploadFinalPage
        };
        WizardPage currentPage;

        Ui::UploadDialog ui;

        Attica::ProviderManager providerManager;
        QHash<QString, Attica::Provider> providers;
        Attica::Category::List categories;
        KUrl uploadFile;
        KUrl previewFile;
        QStringList categoryNames;
        QString contentId;
        bool finished;

        void init();

        // change to page, set the focus also calls updatePage()
        void _k_showPage(int page);

        // check after user input - for example enable the next button
        void _k_updatePage();

        // next wizard page (next button clicked)
        void _k_nextPage();
        // go back one page
        void _k_backPage();

        // after all has been done and said, do the uploading
        void _k_startUpload();

        void _k_providerChanged(const QString& providerName);

        // validation of login is done, go to next page if successfull, otherwise ask again
        void _k_checkCredentialsFinished(Attica::BaseJob*);
        void _k_categoriesLoaded(Attica::BaseJob* job);
        void _k_providerAdded(const Attica::Provider& provider);

    };
}

#endif
