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

#include "atticahelper.h"

#include <kdebug.h>

#include <attica/listjob.h>
#include <attica/postjob.h>

using namespace KNS3;

AtticaHelper::AtticaHelper(QObject *parent) :
    QObject(parent)
{
}

void AtticaHelper::init()
{
    connect(&providerManager, SIGNAL(defaultProvidersLoaded()), this, SLOT(defaultProvidersLoaded()));
    providerManager.loadDefaultProviders();
}

void AtticaHelper::defaultProvidersLoaded()
{
    QStringList providers;
    foreach(Attica::Provider p, providerManager.providers()) {
        providers.append(p.name());
    }
    emit providersLoaded(providers);
}

void AtticaHelper::setCurrentProvider(const QString &provider)
{
    foreach(Attica::Provider p, providerManager.providers()) {
        if (p.name() == provider) {
            currentProvider = p;
            break;
        }
    }
}

Attica::Provider AtticaHelper::provider()
{
    return currentProvider;
}

void AtticaHelper::checkLogin(const QString &name, const QString &password)
{
    Attica::PostJob* checkLoginJob = currentProvider.checkLogin(name, password);
    connect(checkLoginJob, SIGNAL(finished(Attica::BaseJob*)), this, SLOT(checkLoginFinished(Attica::BaseJob*)));
    checkLoginJob->start();
}

void AtticaHelper::checkLoginFinished(Attica::BaseJob* baseJob)
{
    emit loginChecked(baseJob->metadata().error() == Attica::Metadata::NoError);
}

bool AtticaHelper::loadCredentials(QString &name, QString &password)
{
    if (currentProvider.isValid() && currentProvider.hasCredentials()) {
        if (currentProvider.loadCredentials(name, password)) {
            m_username = name;
            return true;
        }
    }
    return false;
}

bool AtticaHelper::saveCredentials(const QString& name, const QString& password)
{
    return currentProvider.saveCredentials(name, password);
}

void AtticaHelper::loadCategories(const QStringList &configuredCategories)
{
    m_configuredCategories = configuredCategories;
    Attica::ListJob<Attica::Category>* job = currentProvider.requestCategories();
    connect(job, SIGNAL(finished(Attica::BaseJob*)), this, SLOT(categoriesLoaded(Attica::BaseJob*)));
    job->start();
}

void AtticaHelper::categoriesLoaded(Attica::BaseJob *baseJob)
{
    Attica::ListJob<Attica::Category>* listJob = static_cast<Attica::ListJob<Attica::Category>*>(baseJob);
    Attica::Category::List newCategories = listJob->itemList();

    if (m_configuredCategories.isEmpty()) {
        kWarning() << "No category was set in knsrc file. Adding all categories.";
        Q_FOREACH(const Attica::Category& category, newCategories) {
            m_validCategories.append(category);
        }
    } else {
        Q_FOREACH(const Attica::Category& category, newCategories) {
            if (m_configuredCategories.contains(category.name())) {
                m_validCategories.append(category);
            }
        }
    }
    emit categoriesLoaded(m_validCategories);
}

void AtticaHelper::loadContentByCurrentUser()
{
    // in case of updates we need the list of stuff that has been uploaded by the user before
    Attica::ListJob<Attica::Content>* userContent = currentProvider.searchContentsByPerson(m_validCategories, m_username);
    connect(userContent, SIGNAL(finished(Attica::BaseJob*)), this, SLOT(contentByCurrentUserLoaded(Attica::BaseJob*)));
    userContent->start();
}

void AtticaHelper::contentByCurrentUserLoaded(Attica::BaseJob *baseJob)
{
    Attica::ListJob<Attica::Content>* contentList = static_cast<Attica::ListJob<Attica::Content>*>(baseJob);
    kDebug() << "Content size: " << contentList->itemList().size();
    m_userCreatedContent = contentList->itemList();

    emit contentByCurrentUserLoaded(m_userCreatedContent);
}

void AtticaHelper::loadLicenses()
{
    Attica::ListJob<Attica::License> *licenseJob = currentProvider.requestLicenses();
    connect(licenseJob, SIGNAL(finished(Attica::BaseJob*)), this, SLOT(licensesLoaded(Attica::BaseJob*)));
    licenseJob->start();
}

void AtticaHelper::licensesLoaded(Attica::BaseJob* baseJob)
{
    Attica::ListJob<Attica::License>* licenseList = static_cast<Attica::ListJob<Attica::License>*>(baseJob);
    emit licensesLoaded(licenseList->itemList());
}


void AtticaHelper::loadDetailsLink(const QString& contentId)
{
    Attica::ItemJob<Attica::Content> *contentJob = currentProvider.requestContent(contentId);
    connect(contentJob, SIGNAL(finished(Attica::BaseJob*)), this, SLOT(detailsLinkLoaded(Attica::BaseJob*)));
    contentJob->start();
}

void AtticaHelper::detailsLinkLoaded(Attica::BaseJob* baseJob)
{
    Attica::ItemJob<Attica::Content>* contentItemJob = static_cast<Attica::ItemJob<Attica::Content>* >(baseJob);
    Attica::Content content = contentItemJob->result();

    emit detailsLinkLoaded(content.detailpage());
}

void AtticaHelper::loadContent(const QString& contentId)
{
    Attica::ItemJob<Attica::Content> *contentJob = currentProvider.requestContent(contentId);
    connect(contentJob, SIGNAL(finished(Attica::BaseJob*)), this, SLOT(contentLoaded(Attica::BaseJob*)));
    contentJob->start();
}

void AtticaHelper::contentLoaded(Attica::BaseJob* baseJob)
{
    Attica::ItemJob<Attica::Content>* contentItemJob = static_cast<Attica::ItemJob<Attica::Content>* >(baseJob);
    emit contentLoaded(contentItemJob->result());
}


#include "atticahelper.moc"
