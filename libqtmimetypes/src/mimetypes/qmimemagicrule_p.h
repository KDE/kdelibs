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

#ifndef QMIMEMAGICRULE_P_H
#define QMIMEMAGICRULE_P_H

#include <QtCore/QByteArray>
#include <QtCore/QScopedPointer>
#include <QtCore/QList>

QT_BEGIN_NAMESPACE

class QMimeMagicRulePrivate;
class QMimeMagicRule
{
public:
    enum Type { Invalid = 0, String, Host16, Host32, Big16, Big32, Little16, Little32, Byte };

    QMimeMagicRule(Type type, const QByteArray &value, int startPos, int endPos, const QByteArray &mask = QByteArray());
    QMimeMagicRule(const QMimeMagicRule &other);
    ~QMimeMagicRule();

    QMimeMagicRule &operator=(const QMimeMagicRule &other);

    bool operator==(const QMimeMagicRule &other) const;

    Type type() const;
    QByteArray value() const;
    int startPos() const;
    int endPos() const;
    QByteArray mask() const;

    bool isValid() const;

    bool matches(const QByteArray &data) const;

    QList<QMimeMagicRule> m_subMatches;

    static Type type(const QByteArray &type);
    static QByteArray typeName(Type type);

    static bool matchSubstring(const char* dataPtr, int dataSize, int rangeStart, int rangeLength, int valueLength, const char* valueData, const char* mask);

private:
    const QScopedPointer<QMimeMagicRulePrivate> d;
};

QT_END_NAMESPACE

#endif // QMIMEMAGICRULE_H
