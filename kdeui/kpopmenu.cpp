#include "kpopmenu.h"
#include "kpopmenu.moc"
#include <qpainter.h>
#include <klocale.h>
#include <kapp.h>

KPopupMenu::KPopupMenu(QWidget *parent, const char *name)
    : QPopupMenu(parent, name)
{
    initialize(klocale->translate("Untitled"));
}

KPopupMenu::KPopupMenu(const char *title, QWidget *parent, const char *name)
    : QPopupMenu(parent, name)
{
    initialize(title);
}


KPopupMenu::~KPopupMenu()
{
}


const char *KPopupMenu::title() const
{
    return text(0);
}

void KPopupMenu::setTitle(const char *title)
{
    changeItem(title, 0);
}
    
void KPopupMenu::initialize(const char *title)
{
    insertItem(title);
    insertSeparator();
    insertSeparator();
}
   
void KPopupMenu::paintCell(QPainter *p, int row, int col)
{
    if (row != 0)
        QPopupMenu::paintCell(p, row, col);
    else if (!row && col==1) {
        int cellh = cellHeight(0);
        int cellw = cellWidth(0);
        QColorGroup cg = colorGroup();
        
        p->setPen(cg.light());
		p->drawText(6, 3, cellw, cellh-4, 
					DontClip|AlignVCenter|ShowPrefix|SingleLine, text(0));
        p->setPen(cg.text());
		p->drawText(5, 2, cellw, cellh-4, 
					DontClip|AlignVCenter|ShowPrefix|SingleLine, text(0));
    }
}
    
