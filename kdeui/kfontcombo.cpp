/* This file is part of the KDE libraries
   Copyright (c) 2001 Malte Starostik <malte.starostik@t-online.de>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

// $Id$

#include <qfontdatabase.h>
#include <qlistbox.h>
#include <qpainter.h>

#include <kcharsets.h>
#include <kconfig.h>
#include <kglobal.h>

#include "kfontcombo.h"
#include "kfontcombo.moc"

struct KFontComboPrivate
{
    KFontComboPrivate()
        : bold(false),
          italic(false),
          underline(false),
          strikeOut(false),
          size(0),
          lineSpacing(0)
    {
    };
    
    bool bold : 1;
    bool italic : 1;
    bool underline : 1;
    bool strikeOut : 1;
    bool displayFonts : 1;
    int size;
    int lineSpacing;
};

class KFontListItem : public QListBoxItem
{
public:
    KFontListItem(const QString &fontName, KFontCombo *combo);
    virtual ~KFontListItem();

    virtual int width(const QListBox *) const;
    virtual int height(const QListBox *) const;

    void updateFont();
    
protected:    
    virtual void paint(QPainter *p);

private:
    void createFont();

private:
    KFontCombo *m_combo;
    QString m_fontName;
    QFont *m_font;
    bool m_canPaintName;
};

KFontListItem::KFontListItem(const QString &fontName, KFontCombo *combo)
    : QListBoxItem(combo->listBox()),
      m_combo(combo),
      m_fontName(fontName),
      m_font(0),
      m_canPaintName(true)
{
    setText(fontName);
};

KFontListItem::~KFontListItem()
{
    delete m_font;
}

int KFontListItem::width(const QListBox *lb) const
{
    if (m_font)
       return QFontMetrics(*m_font).width(text()) + 6;
    return lb->fontMetrics().width(text()) + 6;
}

int KFontListItem::height(const QListBox *lb) const
{
    if (m_combo->d->displayFonts)
        return m_combo->d->lineSpacing + 2;
    QFontMetrics fm(lb->fontMetrics());
    return fm.lineSpacing() + 2;
}

void KFontListItem::paint(QPainter *p)
{
    if (m_combo->d->displayFonts)
    {
        if (!m_font)
            createFont();

        QString t = m_fontName;
        if (p->device() != m_combo)
        {
            if (m_canPaintName)
                p->setFont(*m_font);
            else
                t = QString::fromLatin1("(%1)").arg(m_fontName);
        }
        QFontMetrics fm(p->fontMetrics());
        p->drawText(3, (m_combo->d->lineSpacing + fm.ascent() + fm.leading() / 2) / 2, t);
    }
    else
    {
        QFontMetrics fm(p->fontMetrics());
        p->drawText(3, fm.ascent() + fm.leading() / 2, m_fontName);
    }
}

void KFontListItem::updateFont()
{
    if (!m_font)
        return;        

    m_font->setBold(m_combo->d->bold);
    m_font->setItalic(m_combo->d->italic);
    m_font->setUnderline(m_combo->d->underline);
    m_font->setStrikeOut(m_combo->d->strikeOut);
    m_font->setPointSize(m_combo->d->size);
}

void KFontListItem::createFont()
{
    if (m_font)
        return;
        
    m_font = new QFont(m_fontName);
    QFontMetrics fm(*m_font);
    for (unsigned int i = 0; i < m_fontName.length(); ++i)
        if (!fm.inFont(m_fontName[i]))
        {
            m_canPaintName = false;
            break;
        }
    updateFont();
}

KFontCombo::KFontCombo(QWidget *parent, const char *name)
    : KComboBox(true, parent, name)
{
    init();
    QFontDatabase fontdb;
    setFonts(fontdb.families());
}

KFontCombo::KFontCombo(const QStringList &fonts, QWidget *parent, const char *name)
    : KComboBox(true, parent, name)
{
    init();
    setFonts(fonts);
}

void KFontCombo::setFonts(const QStringList &fonts)
{
    clear();
    for (QStringList::ConstIterator it = fonts.begin(); it != fonts.end(); ++it)
        new KFontListItem(*it, this);
}

void KFontCombo::init()
{
    d = new KFontComboPrivate;
    d->displayFonts = displayFonts();
    setInsertionPolicy(NoInsertion);
    setAutoCompletion(true);
    setSize(12);
}

KFontCombo::~KFontCombo()
{
    delete d;
}

void KFontCombo::setBold(bool bold)
{
    if (d->bold == bold)
        return;
    d->bold = bold;
    updateFonts();
}

bool KFontCombo::bold() const
{
    return d->bold;
}

void KFontCombo::setItalic(bool italic)
{
    if (d->italic == italic)
        return;
    d->italic = italic;
    updateFonts();
}

bool KFontCombo::italic() const
{
    return d->italic;
}

void KFontCombo::setUnderline(bool underline)
{
    if (d->underline == underline)
        return;
    d->underline = underline;
    updateFonts();
}

bool KFontCombo::underline() const
{
    return d->underline;
}

void KFontCombo::setStrikeOut(bool strikeOut)
{
    if (d->strikeOut == strikeOut)
        return;
    d->strikeOut = strikeOut;
    updateFonts();
}

bool KFontCombo::strikeOut() const
{
    return d->strikeOut;
}

void KFontCombo::setSize(int size)
{
    if (d->size == size)
        return;
    d->size = size;
    QFont f;
    f.setPointSize(size);
    QFontMetrics fm(f);
    d->lineSpacing = fm.lineSpacing();
    updateFonts();
}

int KFontCombo::size() const
{
    return d->size;
}

void KFontCombo::updateFonts()
{
    if (!d->displayFonts)
        return;

    for (unsigned int i = 0; i < listBox()->count(); ++i)
    {
        KFontListItem *item = static_cast<KFontListItem *>(listBox()->item(i));
        item->updateFont();
    }
}

bool KFontCombo::displayFonts()
{
    KConfigGroupSaver saver(KGlobal::config(), "KDE");
    return KGlobal::config()->readBoolEntry("DisplayFontItems", true);
}

