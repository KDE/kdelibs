/*
 * $Id$
 *
 * $Log$
 * Revision 1.1.1.1  1997/04/13 14:42:43  cvsuser
 * Source imported
 *
 * Revision 1.1.1.1  1997/04/09 00:28:08  cvsuser
 * Sources imported
 *
 * Revision 1.1  1997/03/09 16:41:56  kalle
 * Initial revision
 *
 */

#include <qpainter.h>
#include <qcursor.h>
#include <qframe.h>
#include <kpanner.h>
#include "kpanner.moc"

KPanner::KPanner(QWidget *parent, const char *name, unsigned flags, int d)
    : QWidget(parent, name)
{
    u_flags = flags;

    /*
     * create the child widgets and the divider widget
     */
    
    cw0 = new QWidget(this, "_pchild0");
    cw1 = new QWidget(this, "_pchild1");
    divider = new QFrame(this, "_pdivider", 0, TRUE);
    divider->setFrameStyle(QFrame::Panel | QFrame::Raised);
    divider->setLineWidth(1);

    if(d < 0 || d > 100)
        d = 50;
    
    percent = d;

    
    /*
     * set the cursor shape
     */
    
    if(u_flags & O_HORIZONTAL) {
        divider->setCursor(QCursor(sizeVerCursor));
        u_limit = parentWidget()->height();
    }
    if(u_flags & O_VERTICAL) {
        divider->setCursor(QCursor(sizeHorCursor));
        u_limit = parentWidget()->width();
    }

    l_limit = 0;
    p0 = 0; p1 = 100;
    
    divider->installEventFilter(this);
}

KPanner::~KPanner() {

    delete cw0;
    delete cw1;
    delete divider;
}

/*
 * resize the panner.
 * this calculates the new geometry information for the client widgets
 * and the separator widget
 */

void KPanner::resizeEvent(QResizeEvent *rev)
{
    int max_value, old_max = 0;

    max_value = getMaxValue();
    
    if(u_flags & O_VERTICAL) {
        if(rev) {
            old_max = rev->oldSize().width();
            pos = (percent * max_value) / 100;
        }
        cw0->setGeometry(0, 0, pos, height());
        cw1->setGeometry(pos + 4, 0, width() - pos - 4, height());
    }
    if(u_flags & O_HORIZONTAL) {
        if(rev) {
            old_max = rev->oldSize().height();
            pos = (percent * max_value) / 100;
        }
        cw0->setGeometry(0, 0, width(), pos);
        cw1->setGeometry(0, pos + 4, width(), height() - pos - 4);
    }
    setDividerGeometry(pos);

    if(rev && (old_max != max_value)) {
        l_limit = (p0 * max_value) / 100;
        u_limit = (p1 * max_value) / 100;
    }

}

void KPanner::setDividerGeometry(int pos)
{
    if(u_flags & O_VERTICAL) {
        divider->setGeometry(pos, 0, 4, height());
    }
    if(u_flags & O_HORIZONTAL) {
        divider->setGeometry(0, pos, width(), 4);
    }
}

/*
 * dragging the mouse on the separator initiates the drag action. It does not check
 * for any mouse button (yet).
 */

bool KPanner::eventFilter(QObject *obj, QEvent *ev)
{
    int max_value = getMaxValue();
    
    /*
     * initiate divider drag action. record current position
     */
    
    if(ev->type() == Event_MouseButtonPress) {
        if(u_flags & O_VERTICAL)
            old_coord = divider->x();
        if(u_flags & O_HORIZONTAL)
            old_coord = divider->y();
    }
    
    if(ev->type() == Event_MouseMove) {

        if(obj == (QObject *)divider) {
            QMouseEvent *mev = (QMouseEvent *)ev;

            /*
             * get the new coordinate, depending on our orientation.
             */

            if(u_flags & O_VERTICAL) {
                delta = mev->pos().x();
            }
            if(u_flags & O_HORIZONTAL) {
                delta = mev->pos().y();
            }

            old_coord += delta;
            if(old_coord < 0)
                old_coord = 0;

            /*
             * check for limits
             */
            
            if(old_coord > max_value)
                old_coord = max_value - 4;
            
            if(old_coord < l_limit)
                old_coord = l_limit;

            if(old_coord > u_limit)
                old_coord = u_limit;
            
            if(u_flags & O_VERTICAL)
                divider->move(old_coord, 0);

            if(u_flags & O_HORIZONTAL)
                divider->move(0, old_coord);

            return TRUE;
        }
    }
    if(ev->type() == Event_MouseButtonRelease) {
        u_coord = old_coord;
        pos = u_coord;
        percent = (100 * u_coord) / max_value;
        setDividerGeometry(pos);
        resizeEvent(0);
        emit(positionChanged());
        return TRUE;
    }
    return FALSE;
}

QWidget *KPanner::child0()
{
    return cw0;
}

QWidget *KPanner::child1()
{
    return cw1;
}

void KPanner::setSeparator(int size)
{
    if(size < 0 || size > 100)
        size = 50;
    
    percent = size;
    resizeEvent(0);
    emit(positionChanged());
}

void KPanner::setAbsSeparator(int size)
{
    int m = getMaxValue(), p;

    if(size < l_limit || size > u_limit)
        size = l_limit + ((u_limit - l_limit) / 2);

    p = (100 * size) / m;
    setSeparator(p);
}

void KPanner::setLimits(int l, int u)
{
    int m = getMaxValue();
    
    l_limit = (l < 0 ? 0 : l);

    if(u < 0) {
        u_limit = u >= -m ? m + u : m;
    } else
        u_limit = (u > m - 4 ? m - 4 : u);

    if(l_limit >= u_limit) {
        l_limit = 0;
        u_limit = m;
    }

    p0 = (100 * l_limit) / m;
    p1 = (100 * u_limit) / m;
    
    if(checkRange(pos)) {
        percent = (100 * pos) / m;
        resizeEvent(0);
    }
}

int KPanner::getMaxValue()
{
    if(u_flags & O_VERTICAL) {
        pos = divider->x();
        return width();
    }
    if(u_flags & O_HORIZONTAL) {
        pos = divider->y();
        return height();
    }
   return 0;
}

bool KPanner::checkRange(int & value)
{
    if(!(value >= l_limit && value <= u_limit)) {
        value = l_limit;
        return TRUE;
    }
    return FALSE;
}
