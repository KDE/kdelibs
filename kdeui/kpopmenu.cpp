#include "kpopmenu.h"
#include "kpopmenu.moc"
#include <qpainter.h>

KPopupMenu::KPopupMenu(QWidget *parent, const char *name)
    : QPopupMenu(parent, name)
{
    initialize("Nameless");
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
    if (row)
        QPopupMenu::paintCell(p, row, col);
    else {
        int cellh = cellHeight(0);
        int cellw = cellWidth(0);
        QColorGroup cg = colorGroup();
        
        p->setPen(cg.light());
        p->drawText(1, 1, cellw, cellh, AlignCenter, text(0));
        p->setPen(cg.text());
        p->drawText(0, 0, cellw, cellh, AlignCenter, text(0));
    }
}
    
