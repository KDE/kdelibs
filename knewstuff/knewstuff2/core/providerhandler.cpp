/*
    This file is part of KNewStuff2.
    Copyright (c) 2002 Cornelius Schumacher <schumacher@kde.org>
    Copyright (c) 2003 - 2006 Josef Spillner <spillner@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

//#include <kconfig.h>
#include <kdebug.h>
//#include <kio/job.h>
#include <kglobal.h>
//#include <kmessagebox.h>
#include <klocale.h>

#include "providerhandler.h"

using namespace KNS;

ProviderHandler::ProviderHandler(const Provider& provider)
{
  mValid = false;
  mProvider = provider;
  mProviderXML = serializeElement(provider);
}

ProviderHandler::ProviderHandler(const QDomElement& providerxml)
{
  mValid = false;
  mProviderXML = providerxml;
  mProvider = deserializeElement(providerxml);
}

bool ProviderHandler::isValid()
{
  return mValid;
}

QDomElement ProviderHandler::providerXML()
{
  return mProviderXML;
}

Provider *ProviderHandler::providerptr()
{
  Provider *provider = new Provider();
  provider->setName(mProvider.name());
  provider->setDownloadUrl(mProvider.downloadUrl());
  provider->setUploadUrl(mProvider.uploadUrl());
  provider->setNoUploadUrl(mProvider.noUploadUrl());
  provider->setNoUpload(mProvider.noUpload());
  provider->setIcon(mProvider.icon());
  // FIXME: download url variants
  return provider;
}

Provider ProviderHandler::provider()
{
  return mProvider;
}

QDomElement ProviderHandler::serializeElement(const Provider& provider)
{
  //QDomDocument doc;

// FIXME: not supported right now (not needed?)
// FIXME: see EntryHandler::serializeElement()
  Q_UNUSED(provider);

  return QDomElement();
}

Provider ProviderHandler::deserializeElement(const QDomElement& providerxml)
{
  Provider provider;

  if(providerxml.tagName() != "provider") return Provider();

  QString downloadurl = providerxml.attribute("downloadurl");
  QString uploadurl = providerxml.attribute("uploadurl");
  QString nouploadurl = providerxml.attribute("nouploadurl");
  provider.setDownloadUrl(KURL(downloadurl));
  provider.setUploadUrl(KURL(uploadurl));
  provider.setNoUploadUrl(KURL(nouploadurl));

  QString downloadlatest = providerxml.attribute("downloadurl-latest");
  QString downloadscore = providerxml.attribute("downloadurl-score");
  QString downloaddownloads = providerxml.attribute("downloadurl-downloads");
  // FIXME: we throw those away
 
  // FIXME: what exactly is the following condition supposed to do?
  // FIXME: make sure new KUrl in KDE 4 handles this right
  KURL iconurl(providerxml.attribute("icon"));
  if(!iconurl.isValid()) iconurl.setPath(providerxml.attribute("icon"));
  provider.setIcon(iconurl);

  QDomNode n;
  for(n = providerxml.firstChild(); !n.isNull(); n = n.nextSibling())
  {
    QDomElement e = n.toElement();
    if(e.tagName() == "noupload")
    {
      provider.setNoUpload(true);
    }
    else if(e.tagName() == "title")
    {
      provider.setName(e.text().stripWhiteSpace());
    }
  }

  // FIXME: validate here

  mValid = true;
  return provider;
}

// FIXME: also used in EntryHandler - make common method?
QDomElement ProviderHandler::addElement(QDomDocument& doc, QDomElement& parent,
  const QString& tag, const QString& value)
{
  QDomElement n = doc.createElement(tag);
  n.appendChild(doc.createTextNode(value));
  parent.appendChild(n);

  return n;
}
