#include "kpopmenu.h"
#include <qpainter.h>
#include <qdrawutil.h>
#include <klocale.h>
#include <kglobal.h>
#include <kconfig.h>
#include <kstddirs.h>
#include <kiconloader.h>
#include <kapp.h>

KPopupTitle::KPopupTitle(QWidget *parent, const char *name)
    : QWidget(parent, name)
{
    KConfig *config = KGlobal::config();
    QString oldGroup = config->group();
    QString tmpStr;

    config->setGroup("PopupTitle");
    bgColor = config->readColorEntry("Color", &colorGroup().dark());
    grHigh = bgColor.light(150);
    grLow = bgColor.dark(150);
    fgColor = config->readColorEntry("TextColor", &colorGroup().highlightedText());
    
    tmpStr = config->readEntry("Pixmap", "");
    if(!tmpStr.isEmpty())
        fill.load(KGlobal::dirs()->findResource("wallpaper", tmpStr));
    if(!fill.isNull()){
        useGradient = false;
        return;
    }
    
    tmpStr = config->readEntry("Gradient", "");
    if(tmpStr.isEmpty() && QPixmap::defaultDepth() >= 15)
        tmpStr = "Diagonal";
    else{
        useGradient = false;
        return;
    }
    
    if(tmpStr == "Horizontal")
        grType = KPixmapEffect::HorizontalGradient;
    else if(tmpStr == "Vertical")
        grType = KPixmapEffect::VerticalGradient;
    else if(tmpStr == "Diagonal")
        grType = KPixmapEffect::DiagonalGradient;
    else if(tmpStr == "Pyramid")
        grType = KPixmapEffect::PyramidGradient;
    else if(tmpStr == "Rectangle")
        grType = KPixmapEffect::RectangleGradient;
    else if(tmpStr == "Elliptic")
        grType = KPixmapEffect::EllipticGradient;
    else{
        warning("KPopupMenu: Unknown gradient type %s for title item",
                tmpStr.latin1());
        grType = KPixmapEffect::HorizontalGradient;
    }

    useGradient = true;
    setMinimumSize(16, fontMetrics().height()+8);
    config->setGroup(oldGroup);
}

KPopupTitle::KPopupTitle(KPixmapEffect::GradientType gradient,
                         const QColor &color, const QColor &textColor,
                         QWidget *parent, const char *name)
   : QWidget(parent, name)
{
    grType = gradient;
    fgColor = textColor;
    bgColor = color;
    grHigh = bgColor.light(150);
    grLow = bgColor.dark(150);
    useGradient = true;
    setMinimumSize(16, fontMetrics().height()+8);
}

KPopupTitle::KPopupTitle(const KPixmap &background, const QColor &color,
                         const QColor &textColor, QWidget *parent,
                         const char *name)
    : QWidget(parent, name)
{
    if(!background.isNull())
        fill = background;
    else
        warning("KPopupMenu: Empty pixmap used for title.");
    useGradient = false;

    fgColor = textColor;
    bgColor = color;
    grHigh = bgColor.light(150);
    grLow = bgColor.dark(150);
    setMinimumSize(16, fontMetrics().height()+8);
}

void KPopupTitle::setTitle(const QString &text, const QPixmap *icon)
{
    titleStr = text;
    if(icon){
        miniicon = *icon;
    }
    else
        miniicon.resize(0, 0);
    setMinimumSize(miniicon.width()+fontMetrics().width(text)+16,
                   fontMetrics().height()+8);
}

void KPopupTitle::paintEvent(QPaintEvent *)
{
    QRect r(rect());
    QPainter p(this);

    if(useGradient){

        if(fill.width() != r.width()-4 || fill.height() != r.height()-4){
            fill.resize(r.width()-4, r.height()-4);
            KPixmapEffect::gradient(fill, grHigh, grLow, grType);
        }
        p.drawPixmap(2, 2, fill);
    }
    else if(!fill.isNull())
        p.drawTiledPixmap(2, 2, r.width()-4, r.height()-4, fill);
    else{
        p.fillRect(2, 2, r.width()-4, r.height()-4, QBrush(bgColor));
    }

    if(!miniicon.isNull())
        p.drawPixmap(4, (r.height()-miniicon.height())/2, miniicon);
    if(!titleStr.isNull()){
        p.setPen(fgColor);
        if(!miniicon.isNull())
            p.drawText(miniicon.width()+8, 0, width()-(miniicon.width()+8),
                       height(), AlignLeft | AlignVCenter | SingleLine,
                       titleStr);
        else
            p.drawText(0, 0, width(), height(),
                       AlignCenter | SingleLine, titleStr);
    }
    p.setPen(Qt::black);
    p.drawRect(r);
    p.setPen(grLow);
    p.drawLine(r.x()+1, r.y()+1, r.right()-1, r.y()+1);
    p.drawLine(r.x()+1, r.y()+1, r.x()+1, r.bottom()-1);
    p.setPen(grHigh);
    p.drawLine(r.x()+1, r.bottom()-1, r.right()-1, r.bottom()-1);
    p.drawLine(r.right()-1, r.y()+1, r.right()-1, r.bottom()-1);
}

QSize KPopupTitle::sizeHint() const
{
    return(minimumSize());
}

KPopupMenu::KPopupMenu(QWidget *parent, const char *name)
    : QPopupMenu(parent, name)
{
    ;
}

int KPopupMenu::insertTitle(const QString &text, int id, int index)
{
    KPopupTitle *titleItem = new KPopupTitle();
    titleItem->setTitle(text);
    return(insertItem(titleItem, id, index));
}

int KPopupMenu::insertTitle(const QPixmap &icon, const QString &text, int id,
                            int index)
{
    KPopupTitle *titleItem = new KPopupTitle();
    titleItem->setTitle(text, &icon);
    return(insertItem(titleItem, id, index));
}

void KPopupMenu::changeTitle(int id, const QString &text)
{
    QMenuItem *item = findItem(id);
    if(item){
        if(item->widget())
            ((KPopupTitle *)item->widget())->setTitle(text);
        else
            warning("KPopupMenu: changeTitle() called with non-title id %d.", id);
    }
    else
        warning("KPopupMenu: changeTitle() called with invalid id %d.", id);
}

void KPopupMenu::changeTitle(int id, const QPixmap &icon, const QString &text)
{
    QMenuItem *item = findItem(id);
    if(item){
        if(item->widget())
            ((KPopupTitle *)item->widget())->setTitle(text, &icon);
        else
            warning("KPopupMenu: changeTitle() called with non-title id %d.", id);
    }
    else
        warning("KPopupMenu: changeTitle() called with invalid id %d.", id);
}

QString KPopupMenu::title(int id)
{
    if(id == -1) // obselete
        return(lastTitle);
    QMenuItem *item = findItem(id);
    if(item){
        if(item->widget())
            return(((KPopupTitle *)item->widget())->title());
        else
            warning("KPopupMenu: title() called with non-title id %d.", id);
    }
    else
        warning("KPopupMenu: title() called with invalid id %d.", id);
    return(QString::null);
}

QPixmap KPopupMenu::titlePixmap(int id)
{
    QMenuItem *item = findItem(id);
    if(item){
        if(item->widget())
            return(((KPopupTitle *)item->widget())->icon());
        else
            warning("KPopupMenu: titlePixmap() called with non-title id %d.", id);
    }
    else
        warning("KPopupMenu: titlePixmap() called with invalid id %d.", id);
    QPixmap tmp;
    return(tmp);
}
    

// Obselete
KPopupMenu::KPopupMenu(const QString& title, QWidget *parent, const char *name)
    : QPopupMenu(parent, name)
{
    setTitle(title);
}

// Obselete
void KPopupMenu::setTitle(const QString &title)
{
    KPopupTitle *titleItem = new KPopupTitle();
    titleItem->setTitle(title);
    insertItem(titleItem);
    lastTitle = title;
}

#include "kpopmenu.moc"
