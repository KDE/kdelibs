/* This file is part of the KDE libraries
   Copyright (c) 2001 Malte Starostik <malte@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/


#include <QAbstractItemView>
#include <QItemDelegate>
#include <qfontdatabase.h>
#include <qpainter.h>

#include <kcharsets.h>
#include <kconfig.h>
#include <kglobal.h>
#include <kfontdialog.h>

#include "kfontcombo.h"
#include "kfontcombo.moc"

class KFontComboDelegate : public QItemDelegate {
public:
    KFontComboDelegate( QObject* o, KFontCombo* combo ) : QItemDelegate( o )
    {
        m_combo = combo;
    }

    void createFont( const QModelIndex& index ) const
    {
        if ( !m_combo->displayFonts() )
            return;

        QVariant v = m_combo->model()->data( index, Qt::FontRole );
        if ( v.isValid() )
            return;
        
        QFont font( m_combo->model()->data( index, Qt::DisplayRole ).toString() );
        font.setBold( m_combo->bold() );
        font.setItalic( m_combo->italic() );
        font.setUnderline( m_combo->underline() );
        font.setStrikeOut( m_combo->strikeOut() );
        font.setPointSize( m_combo->size() );

        QFontMetrics fm( font );
        bool displayFont = true;
        QString fontName = font.family();
        foreach ( QChar c, fontName ) {
            if ( !fm.inFont( c ) )
                displayFont = false;
        }

        if ( displayFont ) {
            m_combo->model()->setData( index, fontName, Qt::DisplayRole );
            m_combo->model()->setData( index, font, Qt::FontRole );
        }
        else
            m_combo->model()->setData( index, QString::fromLatin1( "(%1)" ).arg( fontName ), Qt::DisplayRole );

    }

    virtual void paint( QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index ) const
    {
        createFont( index );
        QItemDelegate::paint( painter, option, index );
    }

    virtual QSize sizeHint( const QStyleOptionViewItem& option, const QModelIndex& index ) const
    {
        // FIXME
        //createFont( index );
        QSize size = QItemDelegate::sizeHint( option, index );
        if ( !m_combo->displayFonts() )
            return size;
        //if ( m_combo->model()->data( index, Qt::FontRole ).isValid() )
        //    return size;
        QFont f;
        f.setPointSize( m_combo->size() );
        QFontMetrics fm( f );

        return QSize( size.width(), fm.lineSpacing() + 2  );
    }
private:
    KFontCombo* m_combo; 
};

struct KFontComboPrivate
{
    KFontComboPrivate()
        : bold(false),
          italic(false),
          underline(false),
          strikeOut(false),
	  modified(false),
          size(0),
          lineSpacing(0)
    {
    };

    bool bold : 1;
    bool italic : 1;
    bool underline : 1;
    bool strikeOut : 1;
    bool displayFonts : 1;
    bool modified : 1;
    int size;
    int lineSpacing;
    QString defaultFamily;
};

KFontCombo::KFontCombo(QWidget *parent)
    : KComboBox(true, parent)
{
    init();
    QStringList families;
    KFontChooser::getFontList(families, 0);
    setFonts(families);
}

KFontCombo::KFontCombo(const QStringList &fonts, QWidget *parent)
    : KComboBox(true, parent)
{
    init();
    setFonts(fonts);
}

void KFontCombo::setFonts(const QStringList &fonts)
{
    clear();
    addItems( fonts );
}

/*
 * Maintenance note: Keep in sync with KFontAction::setFont()
 */
void KFontCombo::setCurrentFont(const QString &family)
{
    QString lowerName = family.toLower();
    int c = count();
    for(int i = 0; i < c; i++)
    {
       if (itemText(i).toLower() == lowerName)
       {
          setCurrentItem(i);
          d->defaultFamily = itemText(i);
	  d->modified = false;
          return;
       }
    }
    int x = lowerName.indexOf(" [");
    if (x>-1)
    {
       lowerName = lowerName.left(x);
       for(int i = 0; i < c; i++)
       {
          if (itemText(i).toLower() == lowerName)
          {
             setCurrentItem(i);
             d->defaultFamily = itemText(i);
	     d->modified = false;
             return;
          }
       }
    }

    lowerName += " [";
    for(int i = 0; i < c; i++)
    {
       if (itemText(i).toLower().startsWith(lowerName))
       {
          setCurrentItem(i);
          d->defaultFamily = itemText(i);
	  d->modified = false;
          return;
       }
    }
}

void KFontCombo::slotModified( int )
{
   d->modified = true;
}

QString KFontCombo::currentFont() const
{
   if (d->modified)
      return currentText();
   return d->defaultFamily;
}

void KFontCombo::setCurrentItem(int i)
{
    d->modified = true;
    QComboBox::setCurrentIndex(i);
}

void KFontCombo::init()
{
    d = new KFontComboPrivate;
    d->displayFonts = displayFonts();
    setItemDelegate( new KFontComboDelegate( view(), this ) );
    setInsertPolicy(NoInsert);
    setAutoCompletion(true);
    setSize(12);
    connect( this, SIGNAL(highlighted(int)), SLOT(slotModified(int)));
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

    for (int i = 0; i < count(); ++i) {
        QVariant v = itemData( i, Qt::FontRole );
        if ( v.isValid() )
            setItemText( i, qvariant_cast<QFont>(v).family() );
            
        setItemData( i, QVariant(), Qt::FontRole );
    }
}

bool KFontCombo::displayFonts()
{
    KConfigGroup cg(KGlobal::config(), "KDE");
    return cg.readBoolEntry("DisplayFontItems", true);
}

void KFontCombo::virtual_hook( int id, void* data )
{ KComboBox::virtual_hook( id, data ); }

