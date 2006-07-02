#include "speller.h"

namespace KSpell2
{

class Speller::Private
{
public:
    QString language;
};

Speller::Speller(const QString &lang)
    : d(new Private)
{
    d->language = lang;
}

Speller::~Speller()
{
    delete d; d = 0;
}

QString Speller::language() const
{
    return d->language;
}

bool Speller::isMisspelled(const QString &word) const
{
    return !isCorrect(word);
}

bool Speller::checkAndSuggest(const QString &word,
                              QStringList &suggestions) const
{
    bool c = isCorrect(word);
    if (!c)
        suggestions = suggest(word);
    return c;
}

}
