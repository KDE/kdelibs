#ifndef QMIMEGLOBPATTERN_H
#define QMIMEGLOBPATTERN_H

#include <QStringList>
#include <QHash>

struct QMimeGlobMatchResult
{
    QMimeGlobMatchResult()
    : m_weight(0), m_matchingPatternLength(0)
    {}

    void addMatch(const QString& mimeType, int weight, const QString &pattern);

    QStringList m_matchingMimeTypes;
    int m_weight;
    int m_matchingPatternLength;
    QString m_foundSuffix;
};

class QMimeGlobPattern
{
public:
    static const unsigned MaxWeight = 100;
    static const unsigned DefaultWeight = 50;
    static const unsigned MinWeight = 1;

    explicit QMimeGlobPattern(const QString &thePattern, const QString &theMimeType, unsigned theWeight = DefaultWeight, Qt::CaseSensitivity s = Qt::CaseInsensitive) :
        m_pattern(thePattern), m_mimeType(theMimeType), m_weight(theWeight), m_caseSensitivity(s)
    {
        if (s == Qt::CaseInsensitive) {
            m_pattern = m_pattern.toLower();
        }
    }
    ~QMimeGlobPattern() {}

    bool matchFileName(const QString& filename) const;

    inline const QString& pattern() const
    { return m_pattern; }
    inline unsigned weight() const
    { return m_weight; }
    inline const QString& mimeType() const
    { return m_mimeType; }
    inline bool isCaseSensitive() const
    { return m_caseSensitivity == Qt::CaseSensitive; }

private:
    QString m_pattern;
    QString m_mimeType;
    int m_weight;
    Qt::CaseSensitivity m_caseSensitivity;
};

class QMimeGlobPatternList : public QList<QMimeGlobPattern>
{
public:
    bool hasPattern(const QString& mimeType, const QString& pattern) const
    {
        const_iterator it = begin();
        const const_iterator myend = end();
        for (; it != myend; ++it)
            if ((*it).pattern() == pattern && (*it).mimeType() == mimeType)
                return true;
        return false;
    }

    /*!
        "noglobs" is very rare occurrence, so it's ok if it's slow
     */
    void removeMimeType(const QString& mimeType)
    {
        QMutableListIterator<QMimeGlobPattern> it(*this);
        while (it.hasNext()) {
            if (it.next().mimeType() == mimeType)
                it.remove();
        }
    }

    void match(QMimeGlobMatchResult &result, const QString &fileName) const;
};

/*!
    Result of the globs parsing, as data structures ready for efficient MIME type matching.
    This contains:
    1) a map of fast regular patterns (e.g. *.txt is stored as "txt" in a qhash's key)
    2) a linear list of high-weight globs
    3) a linear list of low-weight globs
 */
class QMimeAllGlobPatterns
{
public:
    typedef QHash<QString, QStringList> PatternsMap; // MIME type -> patterns

    void addGlob(const QMimeGlobPattern &glob);
    void removeMimeType(const QString &mimeType);
    QStringList matchingGlobs(const QString &fileName, QString *foundSuffix) const;

    PatternsMap m_fastPatterns; // example: "doc" -> "application/msword", "text/plain"
    QMimeGlobPatternList m_highWeightGlobs;
    QMimeGlobPatternList m_lowWeightGlobs; // <= 50, including the non-fast 50 patterns
};

#endif // QMIMEGLOBPATTERN_H
