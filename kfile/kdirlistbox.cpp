/* This file is part of the KDE libraries
    Copyright (C) 1997, 1998 Richard Moore <rich@kde.org>
                  1998 Mario Weilguni <mweilguni@sime.com>
                  1998 Stephan Kulow <coolo@kde.org>
                  1998 Daniel Grana <grana@ie.iwi.unibe.ch>
    
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
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

#include "kfileinfo.h"
#include "kdirlistbox.h"
#include <kapp.h>
#include <qpainter.h>
#include <qlistbox.h>
#include <kstddirs.h>
#include <kglobal.h>

class QPixmap;
class QPainter;
class KFileInfo;

/**
 * Class to allow pixmaps and text in QListBox.
 * Taken from the Qt library documentation.
 */
class KDirListBoxItem : public QListBoxItem 
{
public:
    KDirListBoxItem( const KFileInfo *i);
    
    void setItalic(bool);
    bool isItalic() const;
    
protected:
    virtual void paint( QPainter * );        
    virtual int height( const QListBox * ) const;
    virtual int width( const QListBox * ) const;        
    virtual const QPixmap *pixmap() const { return pm; }

private:
    static QPixmap *folder_pixmap;
    static QPixmap *locked_folder;
    static QPixmap *file_pixmap;
    static QPixmap *locked_file;
    QPixmap *pm;
    bool italic;
};

QPixmap *KDirListBoxItem::folder_pixmap = 0;
QPixmap *KDirListBoxItem::locked_folder = 0;
QPixmap *KDirListBoxItem::file_pixmap = 0;
QPixmap *KDirListBoxItem::locked_file = 0;

KDirListBoxItem::KDirListBoxItem(const KFileInfo *i)
    : QListBoxItem() 
{
    if (!folder_pixmap) // don't use IconLoader to always get the same icon
      folder_pixmap = new QPixmap(locate("mini", "folder.png"));
				    
    if (!locked_folder)
      locked_folder = new QPixmap(locate("mini", "lockedfolder.png"));
    
    if (!file_pixmap)
      file_pixmap = new QPixmap(locate("mini", "unknown.png"));
				
    if (!locked_file)
      locked_file = new QPixmap(locate("mini", "locked.png"));
     
    
    if (i->isDir())
      pm = (i->isReadable()) ? folder_pixmap : locked_folder;
    else
      pm = (i->isReadable()) ? file_pixmap : locked_file;

    italic = FALSE;
    setText(i->fileName());
}

void KDirListBoxItem::paint( QPainter *p )
{
    if(italic) {
	p->save();
	QFont f = p->font();
	f.setItalic(true);
	p->setFont(f);
    }

    p->drawPixmap( 3, 2, *pm );
    QFontMetrics fm = p->fontMetrics();
    int yPos;                       // vertical text position
    if ( (pm->height()) < fm.height() )            
	yPos = fm.ascent() + fm.leading()/2; 
    else
	yPos = pm->height()/2 - fm.height()/2 + fm.ascent();
    
    yPos= yPos+2;
    p->drawText( pm->width() + 5, yPos, text() );

    if(italic)
	p->restore();
}

int KDirListBoxItem::height(const QListBox *lb ) const
{
    int retval;
    
    retval= QMAX( pm->height(), lb->fontMetrics().lineSpacing() + 1);
    retval= retval+2;
    return retval;
}     

int KDirListBoxItem::width(const QListBox *lb ) const    
{
    return pm->width() + lb->fontMetrics().width( text() ) + 6;    
}

void KDirListBoxItem::setItalic(bool b) {
    italic = b;
}

bool KDirListBoxItem::isItalic() const {
    return italic;
}

void KDirListBox::mousePressEvent ( QMouseEvent *inEvent )
{
    int index = this->findItem(inEvent->pos().y());
    if (index == -1 || inEvent->button() != LeftButton)
        return;
  
    if ( useSingle() && isDir(index)) 
        select( index );
    else
        highlight( index );
  
}

KDirListBox::KDirListBox( bool accepts, bool s, QDir::SortSpec sorting,
                          QWidget * parent , const char * name ) 
    : QListBox(parent, name) , KFileInfoContents(s,sorting)
{
    _acceptFiles = accepts;
    setSortMode(Increasing);
    setSorting(QDir::Name);
}

KDirListBox::KDirListBox( bool s, QDir::SortSpec sorting,
                          QWidget * parent , const char * name ) 
    : QListBox(parent, name) , KFileInfoContents(s,sorting)
{
   _acceptFiles = false;
    setSortMode(Increasing);
    setSorting(QDir::Name);
}

void KDirListBox::mouseDoubleClickEvent ( QMouseEvent *inEvent )
{
   if ( !useSingle() && inEvent->button() == LeftButton ) {
       int newItem = this->findItem(inEvent->pos().y());
       if ( newItem != -1 )
	   select(newItem);
   }
}

void KDirListBox::setAutoUpdate(bool f)
{
    QListBox::setAutoUpdate(f);
}

bool KDirListBox::insertItem(const KFileInfo *i, int index) 
{
    KDirListBoxItem *li = new KDirListBoxItem(i);
    li->setItalic(i->isSymLink());
    QListBox::insertItem(li, index);
    return true;
}

void KDirListBox::clearView()
{
    QListBox::clear();
}

void KDirListBox::highlightItem(unsigned int i)
{
    QListBox::setCurrentItem(i);
}

#include "kdirlistbox.moc"

