/****************************************************************************
**
** Copyright (C) 1992-2006 Trolltech ASA. All rights reserved.
**
** This file is part of the QtGui module of the Qt Toolkit.
**
** This file may be used under the terms of the GNU General Public
** License version 2.0 as published by the Free Software Foundation
** and appearing in the file LICENSE.GPL included in the packaging of
** this file.  Please review the following information to ensure GNU
** General Public Licensing requirements will be met:
** http://www.trolltech.com/products/qt/opensource.html
**
** If you are unsure which license is appropriate for your use, please
** review the following information:
** http://www.trolltech.com/products/qt/licensing.html or contact the
** sales department at sales@trolltech.com.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QKEYSEQUENCE_H
#define QKEYSEQUENCE_H

#include <QtCore/qnamespace.h>
#include <QtCore/qstring.h>

QT_BEGIN_HEADER

QT_MODULE(Gui)

#ifndef QT_NO_SHORTCUT

/*****************************************************************************
  QKeySequence stream functions
 *****************************************************************************/
#ifndef QT_NO_DATASTREAM
class QKeySequence;
Q_GUI_EXPORT QDataStream &operator<<(QDataStream &in, const QKeySequence &ks);
Q_GUI_EXPORT QDataStream &operator>>(QDataStream &out, QKeySequence &ks);
#endif

class QVariant;
class QKeySequencePrivate;

class Q_GUI_EXPORT QKeySequence
{
public:
    enum StandardKey {
        UnknownKey,
        HelpContents,
        WhatsThis,
        Open,
        Close,
        Save,
        New,
        Delete,
        Cut,
        Copy,
        Paste,
        Undo,
        Redo,
        Back,
        Forward,
        Refresh,
        ZoomIn,
        ZoomOut,
        Print,
        AddTab,
        NextChild,
        PreviousChild,
        Find,
        FindNext,
        FindPrevious,
        Replace,
        SelectAll,
        Bold,
        Italic,
        Underline,
        MoveToNextChar,
        MoveToPreviousChar,
        MoveToNextWord,
        MoveToPreviousWord,
        MoveToNextLine,
        MoveToPreviousLine,
        MoveToNextPage,
        MoveToPreviousPage,
        MoveToStartOfLine,
        MoveToEndOfLine,
        MoveToStartOfBlock,
        MoveToEndOfBlock,
        MoveToStartOfDocument,
        MoveToEndOfDocument,
        SelectNextChar,
        SelectPreviousChar,
        SelectNextWord,
        SelectPreviousWord,
        SelectNextLine,
        SelectPreviousLine,
        SelectNextPage,
        SelectPreviousPage,
        SelectStartOfLine,
        SelectEndOfLine,
        SelectStartOfBlock,
        SelectEndOfBlock,
        SelectStartOfDocument,
        SelectEndOfDocument,
        DeleteStartOfWord,
        DeleteEndOfWord,
        DeleteEndOfLine
     };

    QKeySequence();
    QKeySequence(const QString &key);
    QKeySequence(int k1, int k2 = 0, int k3 = 0, int k4 = 0);
    QKeySequence(const QKeySequence &ks);
    QKeySequence(StandardKey key);
    ~QKeySequence();

    uint count() const;
    bool isEmpty() const;

    enum SequenceMatch {
        NoMatch,
        PartialMatch,
        ExactMatch
#ifdef QT3_SUPPORT
        , Identical = ExactMatch
#endif
    };

    enum SequenceFormat {
        NativeText,
        PortableText
    };

    QString toString(SequenceFormat format = PortableText) const;
    static QKeySequence fromString(const QString &str, SequenceFormat format = PortableText);

    SequenceMatch matches(const QKeySequence &seq) const;
    static QKeySequence mnemonic(const QString &text);
    static QList<QKeySequence> keyBindings(StandardKey key);

    // ### Qt 5: kill 'operator QString' - it's evil
    operator QString() const;
    operator QVariant() const;
    operator int() const;
    int operator[](uint i) const;
    QKeySequence &operator=(const QKeySequence &other);
    bool operator==(const QKeySequence &other) const;
    inline bool operator!= (const QKeySequence &other) const
    { return !(*this == other); }
    bool operator< (const QKeySequence &ks) const;
    inline bool operator> (const QKeySequence &other) const
    { return other < *this; }
    inline bool operator<= (const QKeySequence &other) const
    { return !(other < *this); }
    inline bool operator>= (const QKeySequence &other) const
    { return !(*this < other); }

    bool isDetached() const;
private:
    static int decodeString(const QString &ks);
    static QString encodeString(int key);
    int assign(const QString &str);
    void setKey(int key, int index);

    QKeySequencePrivate *d;

    friend Q_GUI_EXPORT QDataStream &operator<<(QDataStream &in, const QKeySequence &ks);
    friend Q_GUI_EXPORT QDataStream &operator>>(QDataStream &in, QKeySequence &ks);
    friend class Q3AccelManager;
    friend class QShortcutMap;
    friend class QShortcut;
};
Q_DECLARE_TYPEINFO(QKeySequence, Q_MOVABLE_TYPE);
Q_DECLARE_SHARED(QKeySequence)

#ifndef QT_NO_DEBUG_STREAM
Q_GUI_EXPORT QDebug operator<<(QDebug, const QKeySequence &);
#endif

#else

class Q_GUI_EXPORT QKeySequence
{
public:
    QKeySequence() {}
    QKeySequence(int) {}
};

#endif // QT_NO_SHORTCUT

QT_END_HEADER

#endif // QKEYSEQUENCE_H
