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

#ifndef KNEWSTUFF3_ATTICAHELPER_H
#define KNEWSTUFF3_ATTICAHELPER_H

#include <QtCore/QStringList>

#include <attica/providermanager.h>
#include <attica/provider.h>

#include <attica/category.h>
#include <attica/content.h>
#include <attica/license.h>

class KJob;
namespace KIO {
    class Job;
    class TransferJob;
}

namespace KNS3
{

class AtticaHelper : public QObject
{
Q_OBJECT
public:
    explicit AtticaHelper(QObject *parent = 0);
    void init();

    void setCurrentProvider(const QString& provider);

    Attica::Provider provider();

    void checkLogin(const QString& name, const QString& password);
    bool loadCredentials(QString& name, QString& password);
    bool saveCredentials(const QString& name, const QString& password);
    void loadCategories(const QStringList& configuredCategories);
    void loadContentByCurrentUser();
    void loadLicenses();
    void loadDetailsLink(const QString& contentId);
    void loadContent(const QString& contentId);
    void loadCurrency();
    void loadPreviews(const QString& contentId);

Q_SIGNALS:
    void loginChecked(bool);
    void providersLoaded(const QStringList&);
    void categoriesLoaded(Attica::Category::List);
    void contentByCurrentUserLoaded(const Attica::Content::List&);
    void licensesLoaded(const Attica::License::List&);
    void detailsLinkLoaded(const QUrl&);
    void contentLoaded(const Attica::Content&);
    void currencyLoaded(const QString&);
    void previewLoaded(int index, const QImage& image);

private Q_SLOTS:
    void checkLoginFinished(Attica::BaseJob* baseJob);
    void defaultProvidersLoaded();
    void categoriesLoaded(Attica::BaseJob* baseJob);
    void contentByCurrentUserLoaded(Attica::BaseJob* baseJob);
    void licensesLoaded(Attica::BaseJob* baseJob);
    void detailsLinkLoaded(Attica::BaseJob* baseJob);
    void contentLoaded(Attica::BaseJob* baseJob);
    void currencyLoaded(Attica::BaseJob* baseJob);

    void slotPreviewData(KIO::Job* job, const QByteArray& buf);
    void slotPreviewDownload(KJob *job);


private:
    Attica::ProviderManager providerManager;
    Attica::Provider currentProvider;
    Attica::Category::List m_validCategories;

    QString m_username;
    QStringList m_configuredCategories;
    Attica::Content::List m_userCreatedContent;

    QByteArray m_previewBuffer[3];
    KIO::TransferJob* m_previewJob[3];

    Q_DISABLE_COPY(AtticaHelper)
};
}

#endif
