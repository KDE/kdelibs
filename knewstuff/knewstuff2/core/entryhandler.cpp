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

#include "entryhandler.h"

using namespace KNS;

EntryHandler::EntryHandler(const Entry& entry)
{
  mValid = false;
  mEntry = entry;
  mEntryXML = serializeElement(entry);
}

EntryHandler::EntryHandler(const QDomElement& entryxml)
{
  mValid = false;
  mEntryXML = entryxml;
  mEntry = deserializeElement(entryxml);
}

bool EntryHandler::isValid()
{
  return mValid;
}

QDomElement EntryHandler::entryXML()
{
  return mEntryXML;
}

Entry EntryHandler::entry()
{
  return mEntry;
}

Entry *EntryHandler::entryptr()
{
  Entry *entry = new Entry();
  entry->setName(mEntry.name());
  entry->setAuthor(mEntry.author());
  entry->setType(mEntry.type());
  entry->setLicense(mEntry.license());
  entry->setSummary(mEntry.summary());
  entry->setVersion(mEntry.version());
  entry->setRelease(mEntry.release());
  entry->setReleaseDate(mEntry.releaseDate());
  entry->setPayload(mEntry.payload());
  entry->setPreview(mEntry.preview());
  entry->setRating(mEntry.rating());
  entry->setDownloads(mEntry.downloads());
  return entry;
}

QDomElement EntryHandler::serializeElement(const Entry& entry)
{
  QDomDocument doc;

  QDomElement el = doc.createElement("stuff");
  el.setAttribute("type", entry.type());

//  (void)addElement(doc, el, "author", entry.author());
//  FIXME: rather include author XML element/reference here created by AuthorHandler!
  (void)addElement(doc, el, "licence", entry.license());
  (void)addElement(doc, el, "version", entry.version());
  (void)addElement(doc, el, "release", QString::number(entry.release()));
  (void)addElement(doc, el, "rating", QString::number(entry.rating()));
  (void)addElement(doc, el, "downloads", QString::number(entry.downloads()));

  (void)addElement(doc, el, "releasedate",
    entry.releaseDate().toString(Qt::ISODate));

  KTranslatable name = entry.name();
  KTranslatable summary = entry.summary();
  KTranslatable preview = entry.preview();
  KTranslatable payload = entry.payload();

  QStringList::ConstIterator it;
  QDomElement e;
  QStringList langs;

  langs = name.languages();
  for(it = langs.begin(); it != langs.end(); ++it)
  {
    e = addElement(doc, el, "name", name.translated(*it));
    e.setAttribute("xml:lang", *it);
  }

  langs = summary.languages();
  for(it = langs.begin(); it != langs.end(); ++it)
  {
    e = addElement(doc, el, "name", summary.translated(*it));
    e.setAttribute("xml:lang", *it);
  }

  langs = preview.languages();
  for(it = langs.begin(); it != langs.end(); ++it)
  {
    e = addElement(doc, el, "name", preview.translated(*it));
    e.setAttribute("xml:lang", *it);
  }

  langs = payload.languages();
  for(it = langs.begin(); it != langs.end(); ++it)
  {
    e = addElement(doc, el, "name", payload.translated(*it));
    e.setAttribute("xml:lang", *it);
  }

  return el;
}

Entry EntryHandler::deserializeElement(const QDomElement& entryxml)
{
  Entry entry;

  KTranslatable name, summary, preview, payload;

  if(entryxml.tagName() != "stuff") return Entry();

  QString type = entryxml.attribute("type");
  entry.setType(type);

  QDomNode n;
  for(n = entryxml.firstChild(); !n.isNull(); n = n.nextSibling())
  {
    QDomElement e = n.toElement();
    if(e.tagName() == "name")
    {
      QString lang = e.attribute("lang");
      name.addString(lang, e.text().stripWhiteSpace());
    }
    else if(e.tagName() == "author")
    {
      Author author;
      QString email = e.attribute("email");
      author.setName(e.text().stripWhiteSpace());
      author.setEmail(email);
      entry.setAuthor(author);
    }
    else if(e.tagName() == "licence")
    {
      entry.setLicense(e.text().stripWhiteSpace());
    }
    else if(e.tagName() == "summary")
    {
      QString lang = e.attribute("lang");
      summary.addString(lang, e.text().stripWhiteSpace());
    }
    else if(e.tagName() == "version")
    {
      entry.setVersion(e.text().stripWhiteSpace());
    }
    else if(e.tagName() == "release")
    {
      entry.setRelease(e.text().toInt());
    }
    else if(e.tagName() == "releasedate")
    {
      QDate date = QDate::fromString(e.text().stripWhiteSpace(), Qt::ISODate);
      entry.setReleaseDate(date);
    }
    else if(e.tagName() == "preview")
    {
      QString lang = e.attribute("lang");
      preview.addString(lang, e.text().stripWhiteSpace());
    }
    else if(e.tagName() == "payload")
    {
      QString lang = e.attribute("lang");
      payload.addString(lang, e.text().stripWhiteSpace());
    }
    else if(e.tagName() == "rating")
    {
      entry.setRating(e.text().toInt());
    }
    else if(e.tagName() == "downloads")
    {
      entry.setDownloads(e.text().toInt());
    }
  }

  entry.setName(name);
  entry.setSummary(summary);
  entry.setPreview(preview);
  entry.setPayload(payload);

  // FIXME: validate here

  mValid = true;
  return entry;
}

QDomElement EntryHandler::addElement(QDomDocument& doc, QDomElement& parent,
  const QString& tag, const QString& value)
{
  QDomElement n = doc.createElement(tag);
  n.appendChild(doc.createTextNode(value));
  parent.appendChild(n);

  return n;
}
