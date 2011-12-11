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

#ifndef MAGICMATCHER_H
#define MAGICMATCHER_H

#include "qmime_global.h"

#include <QtCore/QByteArray>
#include <QtCore/QHash>
#include <QtCore/QList>
#include <QtCore/QSharedPointer>
#include <QtCore/QString>

#include "qmimemagicrule_p.h"

QT_BEGIN_NAMESPACE

class QMIME_EXPORT QMimeMagicRuleMatcher
{
public:
    explicit QMimeMagicRuleMatcher(const QString &mime, unsigned priority = 65535);

    bool operator==(const QMimeMagicRuleMatcher &other);

    void addRule(const QMimeMagicRule &rule);
    void addRules(const QList<QMimeMagicRule> &rules);
    QList<QMimeMagicRule> magicRules() const;

    bool matches(const QByteArray &data) const;

    unsigned priority() const;
#if 0
    void setPriority(unsigned priority);
#endif

    QString mimetype() const { return m_mimetype; }

private:
    QList<QMimeMagicRule> m_list;
    unsigned m_priority;
    QString m_mimetype;
};

QT_END_NAMESPACE

#endif // MAGICMATCHER_H
