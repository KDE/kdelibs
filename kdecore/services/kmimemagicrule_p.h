/*  This file is part of the KDE libraries
 *  Copyright 2007 David Faure <faure@kde.org>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 */

#ifndef KMIMEMAGICRULE_H
#define KMIMEMAGICRULE_H

#include <QList>
#include <QString>
class QIODevice;

/**
 * @internal
 * @see KMimeMagicRule
 */
struct KMimeMagicMatch
{
    bool match(QIODevice* device, QByteArray& availableData, const QString& mimeType) const;

    qint64 m_rangeStart;
    qint64 m_rangeLength;
    QByteArray m_data;
    QByteArray m_mask;
    QList<KMimeMagicMatch> m_subMatches;
};

/**
 * @internal
 *
 * Representation of a magic rule for a mimetype.
 * For instance the following 4 lines from the magic file define _one_ rule
 * for matching application/docbook+xml:
 * [90:application/docbook+xml]
 * >0=^@^E<?xml
 * 1>0=^@^Y-//OASIS//DTD DocBook XML+101
 * 1>0=^@^W-//KDE//DTD DocBook XML+101
 */
class KMimeMagicRule
{
public:
    KMimeMagicRule(const QString& mimetype, int priority, const QList<KMimeMagicMatch>& matches)
        : m_mimetype(mimetype), m_priority(priority), m_matches(matches) {}

    bool match(QIODevice* device, QByteArray& availableData) const;

    QString mimetype() const { return m_mimetype; }
    int priority() const { return m_priority; }
    // For unit test only
    QList<KMimeMagicMatch> matches() const { return m_matches; }

private:
    QString m_mimetype;
    int m_priority;
    QList<KMimeMagicMatch> m_matches;
};

#endif /* KMIMEMAGICRULE_H */
