/**************************************************************************
**
** This file is part of QMime
**
** Based on Qt Creator source code
**
** Qt Creator Copyright (c) 2011 Nokia Corporation and/or its subsidiary(-ies).
**
**
** GNU Lesser General Public License Usage
**
** This file may be used under the terms of the GNU Lesser General Public
** License version 2.1 as published by the Free Software Foundation and
** appearing in the file LICENSE.LGPL included in the packaging of this file.
** Please review the following information to ensure the GNU Lesser General
** Public License version 2.1 requirements will be met:
** http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
**************************************************************************/

#include "qmimemagicrulematcher_p.h"

#include "qmimetype_p.h"
#include <QtCore/QDebug>

#include <qendian.h>

QT_BEGIN_NAMESPACE

/*!
    \internal
    \class QMimeMagicRuleMatcher

    \brief The QMimeMagicRuleMatcher class checks a number of rules based on operator "or".

    It is used for rules parsed from XML files.

    \sa QMimeType, QMimeDatabase, MagicRule, MagicStringRule, MagicByteRule, GlobPattern
    \sa BaseMimeTypeParser, MimeTypeParser
*/

QMimeMagicRuleMatcher::QMimeMagicRuleMatcher(const QString &mime, unsigned thePriority) :
    m_list(),
    m_priority(thePriority),
    m_mimetype(mime)
{
}

bool QMimeMagicRuleMatcher::operator==(const QMimeMagicRuleMatcher &other)
{
    return m_list == other.m_list &&
           m_priority == other.m_priority;
}

void QMimeMagicRuleMatcher::addRule(const QMimeMagicRule &rule)
{
    m_list.append(rule);
}

void QMimeMagicRuleMatcher::addRules(const QList<QMimeMagicRule> &rules)
{
    m_list.append(rules);
}

QList<QMimeMagicRule> QMimeMagicRuleMatcher::magicRules() const
{
    return m_list;
}

// Check for a match on contents of a file
bool QMimeMagicRuleMatcher::matches(const QByteArray &data) const
{
    foreach (const QMimeMagicRule &magicRule, m_list) {
        if (magicRule.matches(data))
            return true;
    }

    return false;
}

// Return a priority value from 1..100
unsigned QMimeMagicRuleMatcher::priority() const
{
    return m_priority;
}

#if 0 // unused
void QMimeMagicRuleMatcher::setPriority(unsigned thePriority)
{
    m_priority = thePriority;
}
#endif

QT_END_NAMESPACE
