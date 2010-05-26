/* This file is part of the KDE libraries
    Copyright (C) 1997 Martin Jones (mjones@kde.org)
    Copyright (C) 2007 Roberto Raggi (roberto@kdevelop.org)

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
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/
//-----------------------------------------------------------------------------
// KDE color selection dialog.
//
// 1999-09-27 Espen Sand <espensa@online.no>
// KColorDialog is now subclassed from KDialog. I have also extended
// KColorDialog::getColor() so that it contains a parent argument. This
// improves centering capability.
//
// layout management added Oct 1997 by Mario Weilguni
// <mweilguni@sime.com>
//

#include "kcolordialog.h"
#include "kcolordialog_p.h"


#include <stdio.h>
#include <stdlib.h>

#include <QtGui/QButtonGroup>
#include <QtGui/QCheckBox>
#include <QtGui/QDesktopWidget>
#include <QtGui/QRadioButton>
#include <QtGui/qdrawutil.h>
#include <QtGui/QActionEvent>
#include <QtCore/QFile>
#include <QtGui/QHeaderView>
#include <QtGui/QImage>
#include <QtGui/QStyledItemDelegate>
#include <QtGui/QLabel>
#include <QtGui/QLayout>
#include <QtGui/QPainter>
#include <QtGui/QPushButton>
#include <QtGui/QScrollBar>
#include <QtCore/QTimer>

#include <kapplication.h>
#include <kcombobox.h>
#include <kconfig.h>
#include <kglobal.h>
#include <kglobalsettings.h>
#include <khbox.h>
#include <kiconloader.h>
#include <klineedit.h>
#include <klistwidget.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <knuminput.h>
#include <kseparator.h>
#include <kstandarddirs.h>
#include <kcolorcollection.h>
#include <kcolorutils.h>

#include "kcolormimedata.h"
#include <config.h>
#include <kdebug.h>

#include "kcolorchoosermode_p.h"
#include "kcolorhelpers_p.h"
#include "kselector.h"
#include "kcolorvalueselector.h"
#include "khuesaturationselect.h"
#include "kxyselector.h"
#include <kconfiggroup.h>

#ifdef Q_WS_X11
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <QX11Info>
#include <fixx11h.h>
#endif

using namespace KDEPrivate;

using KDEPrivate::KColorTable;

struct ColorCollectionNameType {
    const char* const m_fileName;
    const char* const m_displayName;
};

static const ColorCollectionNameType colorCollectionName[] = {
    { "Recent_Colors", I18N_NOOP2("palette name", "* Recent Colors *") },
    { "Custom_Colors", I18N_NOOP2("palette name", "* Custom Colors *") },
    { "40.colors",     I18N_NOOP2("palette name", "Forty Colors") },
    { "Oxygen.colors", I18N_NOOP2("palette name", "Oxygen Colors") },
    { "Rainbow.colors", I18N_NOOP2("palette name", "Rainbow Colors") },
    { "Royal.colors",  I18N_NOOP2("palette name", "Royal Colors") },
    { "Web.colors",    I18N_NOOP2("palette name", "Web Colors") },
    { 0, 0 } // end of data
};

enum ColorCollectionIndices
{
    recentColorIndex,
    customColorIndex,
    fortyColorIndex
};

//-----------------------------------------------------------------------------

class KColorCells::KColorCellsPrivate
{
public:
    KColorCellsPrivate(KColorCells *q): q(q) {
        inMouse = false;
        selected = -1;
        shade = false;
    }

    KColorCells *q;
    QPoint mousePos;
    int selected;
    bool shade;
    bool inMouse;
};

class KColorCellsItemDelegate: public QStyledItemDelegate
{
public:
    KColorCellsItemDelegate(KColorCells *parent): QStyledItemDelegate(parent) {}
    virtual void paint(QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index) const
    {
        QStyleOptionViewItemV4 opt(option);
        initStyleOption(&opt,index);

        //Get the current cell color
        QColor backgroundColor = index.data(Qt::BackgroundRole).value<QColor>();
        if (backgroundColor.isValid()) {
            //Paint the general background
            painter->fillRect(opt.rect, backgroundColor);
            //Paint the selection mark (circle)
            if (opt.state & QStyle::State_Selected) {
                //Use black or white, depending on the contrast
                QColor color = QColor(0, 0, 0, 220);
                if (KColorUtils::contrastRatio(color, backgroundColor) < 5) {
                    color = QColor(255, 255, 255, 220);
                }
                //Draw the selection (radiobutton-like) circle
                painter->save();
                painter->setRenderHint(QPainter::Antialiasing, true);
                painter->setRenderHint(QPainter::HighQualityAntialiasing, true);
                painter->setPen(QPen(color, 1.2, Qt::SolidLine));
                painter->setBrush(QBrush());
                painter->drawEllipse(opt.rect.adjusted(2,2,-2,-2));
                painter->restore();
            }
        } else {
            //Paint the "X" (missing) cross on empty background color
            backgroundColor = opt.palette.color(QPalette::Window);
            painter->fillRect(opt.rect, backgroundColor);
            painter->save();
            QColor crossColor = qGray(backgroundColor.rgb()) > 192 ? backgroundColor.darker(106) :
                                                                     backgroundColor.lighter(106);
            painter->setPen(QPen(crossColor, 1.5));
            painter->drawLine(opt.rect.topLeft(), opt.rect.bottomRight());
            painter->drawLine(opt.rect.topRight(), opt.rect.bottomLeft());
            painter->restore();
        }
    }
};

KColorCells::KColorCells(QWidget *parent, int rows, int cols)
        : QTableWidget(parent), d(new KColorCellsPrivate(this))
{
    setItemDelegate(new KColorCellsItemDelegate(this));

    setFrameShape(QFrame::NoFrame);
    d->shade = true;
    setRowCount(rows);
    setColumnCount(cols);

    verticalHeader()->hide();
    horizontalHeader()->hide();

    d->selected = 0;
    d->inMouse = false;

    // Drag'n'Drop
    setAcceptDrops(true);

    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    viewport()->setBackgroundRole(QPalette::Background);
    setBackgroundRole(QPalette::Background);


    setSelectionMode(QAbstractItemView::SingleSelection);
    setDragEnabled(false);
}

KColorCells::~KColorCells()
{
    delete d;
}

QColor KColorCells::color(int index) const
{
    QTableWidgetItem * tmpItem = item(index / columnCount(), index % columnCount());

    if (tmpItem != 0)
        return tmpItem->data(Qt::BackgroundRole).value<QColor>();

    return QColor();
}

int KColorCells::count() const
{
    return rowCount() * columnCount();
}

void KColorCells::setShading(bool _shade)
{
    d->shade = _shade;
}

void KColorCells::setAcceptDrags(bool _acceptDrags)
{
    this->setDragEnabled(_acceptDrags);
}

void KColorCells::setSelected(int index)
{
    Q_ASSERT(index >= 0 && index < count());

    d->selected = index;
}

int KColorCells::selectedIndex() const
{
    return d->selected;
}

void KColorCells::setColor(int column, const QColor &color)
{
    const int tableRow = column / columnCount();
    const int tableColumn = column % columnCount();

    Q_ASSERT(tableRow >= 0 && tableRow < rowCount());
    Q_ASSERT(tableColumn >= 0 && tableColumn < columnCount());

    QTableWidgetItem * tableItem = item(tableRow, tableColumn);

    if (tableItem == 0) {
        tableItem = new QTableWidgetItem();
        setItem(tableRow, tableColumn, tableItem);
    }

    tableItem->setData(Qt::BackgroundRole , color);
}

/*void KColorCells::paintCell( QPainter *painter, int row, int col )
{
    painter->setRenderHint( QPainter::Antialiasing , true );

 QBrush brush;
 int w = 1;

 if (shade)
 {
  qDrawShadePanel( painter, 1, 1, cellWidth()-2,
       cellHeight()-2, palette(), true, 1, &brush );
  w = 2;
 }
 QColor color = colors[ row * numCols() + col ];
 if (!color.isValid())
 {
  if (!shade) return;
  color = palette().color(backgroundRole());
 }

 const QRect colorRect( w, w, cellWidth()-w*2, cellHeight()-w*2 );
 painter->fillRect( colorRect, color );

 if ( row * numCols() + col == selected ) {
  painter->setPen( qGray(color.rgb())>=127 ? Qt::black : Qt::white );
  painter->drawLine( colorRect.topLeft(), colorRect.bottomRight() );
  painter->drawLine( colorRect.topRight(), colorRect.bottomLeft() );
 }
}*/

void KColorCells::resizeEvent(QResizeEvent*)
{
    // According to the Qt doc:
    //   If you need to set the width of a given column to a fixed value, call
    //   QHeaderView::resizeSection() on the table's {horizontal,vertical}
    //   header.
    // Therefore we iterate over each row and column and set the header section
    // size, as the sizeHint does indeed appear to be ignored in favor of a
    // minimum size that is larger than what we want.
    for (int index = 0 ; index < columnCount() ; index++)
        horizontalHeader()->resizeSection(index, sizeHintForColumn(index));
    for (int index = 0 ; index < rowCount() ; index++)
        verticalHeader()->resizeSection(index, sizeHintForRow(index));
}

int KColorCells::sizeHintForColumn(int /*column*/) const
{
    return width() / columnCount() ;
}

int KColorCells::sizeHintForRow(int /*row*/) const
{
    return height() / rowCount() ;
}

void KColorCells::mousePressEvent(QMouseEvent *e)
{
    d->inMouse = true;
    d->mousePos = e->pos();

    QTableWidget::mousePressEvent(e);
}


int KColorCells::positionToCell(const QPoint &pos, bool ignoreBorders) const
{
    //TODO ignoreBorders not yet handled
    Q_UNUSED(ignoreBorders)

    QTableWidgetItem* tableItem = itemAt(pos);

    if (!tableItem)
        return -1;

    const int itemRow = row(tableItem);
    const int itemColumn = column(tableItem);
    int cell = itemRow * columnCount() + itemColumn;

    /*if (!ignoreBorders)
    {
       int border = 2;
       int x = pos.x() - col * cellWidth();
       int y = pos.y() - row * cellHeight();
       if ( (x < border) || (x > cellWidth()-border) ||
            (y < border) || (y > cellHeight()-border))
          return -1;
    }*/

    return cell;
}

void KColorCells::mouseMoveEvent(QMouseEvent *e)
{
    if (this->dragEnabled() || this->acceptDrops()) {
        if (!(e->buttons() & Qt::LeftButton)) return;

        if (d->inMouse) {
            int delay = KGlobalSettings::dndEventDelay();
            if (e->x() > d->mousePos.x() + delay || e->x() < d->mousePos.x() - delay ||
                    e->y() > d->mousePos.y() + delay || e->y() < d->mousePos.y() - delay) {
                // Drag color object
                QTableWidgetItem * tableItem = itemAt(d->mousePos);

                if (tableItem) {
                    QVariant var = tableItem->data(Qt::BackgroundRole);
                    QColor tmpCol = var.value<QColor>();
                    if (tmpCol.isValid())
                        KColorMimeData::createDrag(tmpCol, this)->start();
                }
            }
        }
    } else
        QTableWidget::mouseMoveEvent(e);
}

void KColorCells::dragEnterEvent(QDragEnterEvent *event)
{
    kDebug() << "KColorCells::dragEnterEvent() acceptDrags="
    << this->dragEnabled()
    << " canDecode=" << KColorMimeData::canDecode(event->mimeData())
    << endl;
    event->setAccepted(this->dragEnabled() && KColorMimeData::canDecode(event->mimeData()));
}

// Reimplemented to override QTableWidget's override.  Else dropping doesn't work.
void KColorCells::dragMoveEvent(QDragMoveEvent *event)
{
    kDebug() << "KColorCells::dragMoveEvent() acceptDrags="
    << this->dragEnabled()
    << " canDecode=" << KColorMimeData::canDecode(event->mimeData())
    << endl;
    event->setAccepted(this->dragEnabled() && KColorMimeData::canDecode(event->mimeData()));
}

void KColorCells::dropEvent(QDropEvent *event)
{
    QColor c = KColorMimeData::fromMimeData(event->mimeData());

    kDebug() << "KColorCells::dropEvent() color.isValid=" << c.isValid();
    if (c.isValid()) {
        QTableWidgetItem * tableItem = itemAt(event->pos());

        if (tableItem)
            tableItem->setData(Qt::BackgroundRole , c);
    }
}

void KColorCells::mouseReleaseEvent(QMouseEvent *e)
{
    if (selectionMode() != QAbstractItemView::NoSelection) {
        int cell = positionToCell(d->mousePos);
        int currentCell = positionToCell(e->pos());

        // If we release the mouse in another cell and we don't have
        // a drag we should ignore this event.
        if (currentCell != cell)
            cell = -1;

        if ((cell != -1) && (d->selected != cell)) {
            d->selected = cell;

            const int newRow = cell / columnCount();
            const int newColumn = cell % columnCount();

            clearSelection(); // we do not want old violet selected cells

            item(newRow, newColumn)->setSelected(true);
        }

        d->inMouse = false;
        if (cell != -1)
            emit colorSelected(cell , color(cell));
    }

    QTableWidget::mouseReleaseEvent(e);
}

void KColorCells::mouseDoubleClickEvent(QMouseEvent * /*e*/)
{
    int cell = positionToCell(d->mousePos);

    if (cell != -1)
        emit colorDoubleClicked(cell , color(cell));
}


//-----------------------------------------------------------------------------

class KColorPatch::KColorPatchPrivate
{
public:
    KColorPatchPrivate(KColorPatch *q): q(q) {}

    KColorPatch *q;
    QColor color;
};

KColorPatch::KColorPatch(QWidget *parent) : QFrame(parent), d(new KColorPatchPrivate(this))
{
    setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);
    setAcceptDrops(true);
    setMinimumSize(12, 12);
}

KColorPatch::~KColorPatch()
{
    delete d;
}

QColor KColorPatch::color() const
{
    return d->color;
}

void KColorPatch::setColor(const QColor &col)
{
    d->color = col.toRgb();

    update();
}

void KColorPatch::paintEvent(QPaintEvent* pe)
{
    QFrame::paintEvent(pe);
    QPainter painter(this);

    fillOpaqueRect(&painter, contentsRect(), d->color);
}

void KColorPatch::mouseMoveEvent(QMouseEvent *e)
{
    // Drag color object
    if (!(e->buttons() & Qt::LeftButton))
        return;
    KColorMimeData::createDrag(d->color, this)->start();
}

void KColorPatch::dragEnterEvent(QDragEnterEvent *event)
{
    event->setAccepted(KColorMimeData::canDecode(event->mimeData()));
}

void KColorPatch::dropEvent(QDropEvent *event)
{
    QColor c = KColorMimeData::fromMimeData(event->mimeData());
    if (c.isValid()) {
        setColor(c);
        emit colorChanged(c);
    }
}

class KColorTable::KColorTablePrivate
{
public:
    KColorTablePrivate(KColorTable *q): q(q) {}

    void slotColorCellSelected(int index , const QColor&);
    void slotColorCellDoubleClicked(int index , const QColor&);
    void slotColorTextSelected(const QString &colorText);
    void slotSetColors(const QString &_collectionName);
    void slotShowNamedColorReadError(void);

    KColorTable *q;
    QString i18n_namedColors;
    KComboBox *combo;
    KColorCells *cells;
    QScrollArea *sv;
    KListWidget *mNamedColorList;
    KColorCollection *mPalette;
    int mMinWidth;
    int mCols;
    QMap<QString, QColor> m_namedColorMap;
};

KColorTable::KColorTable(QWidget *parent, int minWidth, int cols)
        : QWidget(parent), d(new KColorTablePrivate(this))
{
    d->cells = 0;
    d->mPalette = 0;
    d->mMinWidth = minWidth;
    d->mCols = cols;
    d->i18n_namedColors  = i18n("Named Colors");

    QStringList diskPaletteList = KColorCollection::installedCollections();
    QStringList paletteList;

    // We must replace the untranslated file names by translate names (of course only for KDE's standard palettes)
    for (int i = 0; colorCollectionName[i].m_fileName; ++i) {
        diskPaletteList.removeAll(colorCollectionName[i].m_fileName);
        paletteList.append(i18nc("palette name", colorCollectionName[i].m_displayName));
    }
    paletteList += diskPaletteList;
    paletteList.append(d->i18n_namedColors);

    QVBoxLayout *layout = new QVBoxLayout(this);

    d->combo = new KComboBox(this);
    d->combo->setEditable(false);
    d->combo->addItems(paletteList);
    layout->addWidget(d->combo);

    d->sv = new QScrollArea(this);
    QSize cellSize = QSize(d->mMinWidth, 120);
    d->sv->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    d->sv->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    QSize minSize = QSize(d->sv->verticalScrollBar()->sizeHint().width(), 0);
    minSize += QSize(d->sv->frameWidth() * 2, 0);
    minSize += QSize(cellSize);
    d->sv->setFixedSize(minSize);
    layout->addWidget(d->sv);

    d->mNamedColorList = new KListWidget(this);
    d->mNamedColorList->setObjectName("namedColorList");
    d->mNamedColorList->setFixedSize(minSize);
    d->mNamedColorList->hide();
    layout->addWidget(d->mNamedColorList);
    connect(d->mNamedColorList, SIGNAL(currentTextChanged(const QString &)),
            this, SLOT(slotColorTextSelected(const QString &)));

    setFixedSize(sizeHint());
    connect(d->combo, SIGNAL(activated(const QString &)),
            this, SLOT(slotSetColors(const QString &)));
}

KColorTable::~KColorTable()
{
    delete d->mPalette;
    delete d;
}

QString
KColorTable::name() const
{
    return d->combo->currentText();
}


static const char * const *namedColorFilePath(void)
{
    //
    // 2000-02-05 Espen Sand.
    // Add missing filepaths here. Make sure the last entry is 0, 0!
    //
    // 2009-06-16 Pino Toscano
    //
    // You can specify either absolute paths or relative locations
    // wrt KStandardDirs resources. In either way, there should be two
    // "strings" for each path.
    // - absolute path: specify it directly, then add 0 as second item
    //   * example: "/usr/share/X11/rgb.txt", 0,
    // - KStandardDirs location: specify the filename as first item,
    //   then add the resource as second
    //   * example: "kdeui/rgb.txt", "data",
    //
    static const char * const path[] = {
#ifdef Q_WS_X11
#ifdef X11_RGBFILE
        X11_RGBFILE, 0,
#endif
        "/usr/share/X11/rgb.txt", 0,
        "/usr/X11R6/lib/X11/rgb.txt", 0,
        "/usr/openwin/lib/X11/rgb.txt", 0, // for Solaris.
#else /* systems without X11 */
        "kdeui/rgb.txt", "data",
#endif
        0, 0
    };
    return path;
}




void
KColorTable::readNamedColor(void)
{
    if (d->mNamedColorList->count() != 0) {
        return; // Strings already present
    }

    KGlobal::locale()->insertCatalog("kdelibs_colors4");

    //
    // Code somewhat inspired by KColorCollection.
    //

    const char * const *path = namedColorFilePath();
    for (int i = 0; path[i]; i += 2) {
        QString file;
        if (path[i + 1]) {
            file = KStandardDirs::locate(path[i + 1], QString::fromLatin1(path[i]));
            if (file.isEmpty()) {
                continue;
            }
        } else {
            file = QString::fromLatin1(path[i]);
        }

        QFile paletteFile(file);
        if (!paletteFile.open(QIODevice::ReadOnly)) {
            continue;
        }

        QByteArray line;
        QStringList list;
        while (!paletteFile.atEnd()) {
            line = paletteFile.readLine();

            int red, green, blue;
            int pos = 0;

            if (sscanf(line, "%d %d %d%n", &red, &green, &blue, &pos) == 3) {
                //
                // Remove duplicates. Every name with a space and every name
                // that start with "gray".
                //
                QString name = line.mid(pos).trimmed();
                QByteArray s1 = line.mid(pos);
                if (name.isNull() || name.indexOf(' ') != -1 ||
                        name.indexOf("gray") != -1 ||  name.indexOf("grey") != -1) {
                    continue;
                }

                const QColor color(red, green, blue);
                if (color.isValid()) {
                    const QString colorName(i18nc("color", name.toLatin1().data()));
                    list.append(colorName);
                    d->m_namedColorMap[ colorName ] = color;
                }
            }
        }

        list.sort();
        d->mNamedColorList->addItems(list);
        break;
    }

    if (d->mNamedColorList->count() == 0) {
        //
        // Give the error dialog box a chance to center above the
        // widget (or dialog). If we had displayed it now we could get a
        // situation where the (modal) error dialog box pops up first
        // preventing the real dialog to become visible until the
        // error dialog box is removed (== bad UI).
        //
        QTimer::singleShot(10, this, SLOT(slotShowNamedColorReadError()));
    }
}


void
KColorTable::KColorTablePrivate::slotShowNamedColorReadError(void)
{
    if (mNamedColorList->count() == 0) {
        QString pathMsg;
        int pathCount = 0;

        const char * const *path = namedColorFilePath();
        for (int i = 0; path[i]; i += 2, ++pathCount) {
            if (path[i + 1]) {
                pathMsg += QLatin1String(path[i + 1]) + ", " + QString::fromLatin1(path[i]);
            } else {
                pathMsg += QLatin1String(path[i]);
            }
            pathMsg += '\n';
        }

        QString finalMsg  = i18ncp("%1 is the number of paths, %2 is the list of paths (with newlines between them)",
                                   "Unable to read X11 RGB color strings. The following "
                                   "file location was examined:\n%2",
                                   "Unable to read X11 RGB color strings. The following "
                                   "file locations were examined:\n%2",
                                   pathCount, pathMsg );

        KMessageBox::sorry(q, finalMsg);
    }
}


//
// 2000-02-12 Espen Sand
// Set the color in two steps. The setColors() slot will not emit a signal
// with the current color setting. The reason is that setColors() is used
// by the color selector dialog on startup. In the color selector dialog
// we normally want to display a startup color which we specify
// when the dialog is started. The slotSetColors() slot below will
// set the palette and then use the information to emit a signal with the
// new color setting. It is only used by the combobox widget.
//
void
KColorTable::KColorTablePrivate::slotSetColors(const QString &_collectionName)
{
    q->setColors(_collectionName);
    if (mNamedColorList->count() && mNamedColorList->isVisible()) {
        int item = mNamedColorList->currentRow();
        mNamedColorList->setCurrentRow(item < 0 ? 0 : item);
        slotColorTextSelected(mNamedColorList->currentItem()->text());
    } else {
        slotColorCellSelected(0, QColor()); // FIXME: We need to save the current value!!
    }
}


void
KColorTable::setColors(const QString &_collectionName)
{
    QString collectionName(_collectionName);

    if (d->combo->currentText() != collectionName) {
        bool found = false;
        for (int i = 0; i < d->combo->count(); i++) {
            if (d->combo->itemText(i) == collectionName) {
                d->combo->setCurrentIndex(i);
                found = true;
                break;
            }
        }
        if (!found) {
            d->combo->addItem(collectionName);
            d->combo->setCurrentIndex(d->combo->count() - 1);
        }
    }

    // We must again find the file name of the palette from the eventual translation
    for (int i = 0; colorCollectionName[i].m_fileName; ++i) {
        if (collectionName == i18nc("palette name", colorCollectionName[i].m_displayName)) {
            collectionName = colorCollectionName[i].m_fileName;
            break;
        }
    }


    //
    // 2000-02-12 Espen Sand
    // The palette mode "i18n_namedColors" does not use the KColorCollection
    // class. In fact, 'mPalette' and 'cells' are 0 when in this mode. The reason
    // for this is maninly that KColorCollection reads from and writes to files
    // using "locate()". The colors used in "i18n_namedColors" mode comes from
    // the X11 diretory and is not writable. I don't think this fit in
    // KColorCollection.
    //
    if (!d->mPalette || d->mPalette->name() != collectionName) {
        if (collectionName == d->i18n_namedColors) {
            d->sv->hide();
            d->mNamedColorList->show();
            readNamedColor();

            delete d->cells; d->cells = 0;
            delete d->mPalette; d->mPalette = 0;
        } else {
            d->mNamedColorList->hide();
            d->sv->show();

            delete d->cells;
            delete d->mPalette;
            d->mPalette = new KColorCollection(collectionName);
            int rows = (d->mPalette->count() + d->mCols - 1) / d->mCols;
            if (rows < 1) rows = 1;
            d->cells = new KColorCells(d->sv->viewport(), rows, d->mCols);
            d->cells->setShading(false);
            d->cells->setAcceptDrags(false);
            QSize cellSize = QSize(d->mMinWidth, d->mMinWidth * rows / d->mCols);
            d->cells->setFixedSize(cellSize);
            for (int i = 0; i < d->mPalette->count(); i++) {
                d->cells->setColor(i, d->mPalette->color(i));
            }
            connect(d->cells, SIGNAL(colorSelected(int , const QColor&)),
                    SLOT(slotColorCellSelected(int , const QColor&)));
            connect(d->cells, SIGNAL(colorDoubleClicked(int , const QColor&)),
                    SLOT(slotColorCellDoubleClicked(int , const QColor&)));
            d->sv->setWidget(d->cells);
            d->cells->show();

            //d->sv->updateScrollBars();
        }
    }
}



void
KColorTable::KColorTablePrivate::slotColorCellSelected(int index , const QColor& /*color*/)
{
    if (!mPalette || (index >= mPalette->count()))
        return;
    emit q->colorSelected(mPalette->color(index), mPalette->name(index));
}

void
KColorTable::KColorTablePrivate::slotColorCellDoubleClicked(int index , const QColor& /*color*/)
{
    if (!mPalette || (index >= mPalette->count()))
        return;
    emit q->colorDoubleClicked(mPalette->color(index), mPalette->name(index));
}


void
KColorTable::KColorTablePrivate::slotColorTextSelected(const QString &colorText)
{
    emit q->colorSelected(m_namedColorMap[ colorText ], colorText);
}


void
KColorTable::addToCustomColors(const QColor &color)
{
    setColors(i18nc("palette name", colorCollectionName[customColorIndex].m_displayName));
    d->mPalette->addColor(color);
    d->mPalette->save();
    delete d->mPalette;
    d->mPalette = 0;
    setColors(i18nc("palette name", colorCollectionName[customColorIndex].m_displayName));
}

void
KColorTable::addToRecentColors(const QColor &color)
{
    //
    // 2000-02-12 Espen Sand.
    // The 'mPalette' is always 0 when current mode is i18n_namedColors
    //
    bool recentIsSelected = false;
    if (d->mPalette && d->mPalette->name() == colorCollectionName[ recentColorIndex ].m_fileName) {
        delete d->mPalette;
        d->mPalette = 0;
        recentIsSelected = true;
    }
    KColorCollection *recentPal = new KColorCollection(colorCollectionName[ recentColorIndex ].m_fileName);
    if (recentPal->findColor(color) == -1) {
        recentPal->addColor(color);
        recentPal->save();
    }
    delete recentPal;
    if (recentIsSelected)
        setColors(i18nc("palette name", colorCollectionName[ recentColorIndex ].m_displayName));
}

class KCDPickerFilter;

class KColorDialog::KColorDialogPrivate
{
public:
    KColorDialogPrivate(KColorDialog *q): q(q) {}

    void setRgbEdit(const QColor &col);
    void setHsvEdit(const QColor &col);
    void setHtmlEdit(const QColor &col);
    void _setColor(const QColor &col, const QString &name = QString());
    void showColor(const QColor &color, const QString &name);

    void slotRGBChanged(void);
    void slotAlphaChanged(void);
    void slotHSVChanged(void);
    void slotHtmlChanged(void);
    void slotHSChanged(int, int);
    void slotVChanged(int);
    void slotAChanged(int);
    void slotModeChanged(int);

    void slotColorSelected(const QColor &col);
    void slotColorSelected(const QColor &col, const QString &name);
    void slotColorDoubleClicked(const QColor &col, const QString &name);
    void slotColorPicker();
    void slotAddToCustomColors();
    void slotDefaultColorClicked();
    /**
      * Write the settings of the dialog to config file.
     **/
    void slotWriteSettings();

    /**
     * Returns the mode.
     */
    KColorChooserMode chooserMode();

    /**
     * Sets a mode. Updates the color picker and the color bar.
     */
    void setChooserMode(KColorChooserMode c);

    KColorDialog *q;
    KColorTable *table;
    QString originalPalette;
    bool bRecursion;
    bool bEditRgb;
    bool bEditHsv;
    bool bEditHtml;
    bool bColorPicking;
    bool bAlphaEnabled;
    QLabel *colorName;
    KLineEdit *htmlName;
    KIntSpinBox *hedit;
    KIntSpinBox *sedit;
    KIntSpinBox *vedit;
    KIntSpinBox *redit;
    KIntSpinBox *gedit;
    KIntSpinBox *bedit;
    QWidget *alphaLabel;
    KIntSpinBox *aedit;

    KColorPatch *patch;
    KColorPatch *comparePatch;

    KColorChooserMode _mode;
    QButtonGroup *modeGroup;

    KHueSaturationSelector *hsSelector;
    KColorCollection *palette;
    KColorValueSelector *valuePal;
    KGradientSelector *alphaSelector;
    QVBoxLayout* l_right;
    QGridLayout* tl_layout;
    QCheckBox *cbDefaultColor;
    QColor defaultColor;
    QColor selColor;
#ifdef Q_WS_X11
    KCDPickerFilter* filter;
#endif
};

#ifdef Q_WS_X11
class KCDPickerFilter: public QWidget
{
public:
    KCDPickerFilter(QWidget* parent): QWidget(parent) {}

    virtual bool x11Event(XEvent* event) {
        if (event->type == ButtonRelease) {
            QMouseEvent e(QEvent::MouseButtonRelease, QPoint(),
                          QPoint(event->xmotion.x_root, event->xmotion.y_root) , Qt::NoButton, Qt::NoButton, Qt::NoModifier);
            QApplication::sendEvent(parentWidget(), &e);
            return true;
        } else return false;
    }
};

#endif


KColorDialog::KColorDialog(QWidget *parent, bool modal)
        : KDialog(parent), d(new KColorDialogPrivate(this))
{
    setCaption(i18n("Select Color"));
    setButtons(modal ? Ok | Cancel : Close);
    setModal(modal);
    d->bRecursion = true;
    d->bColorPicking = false;
    d->bAlphaEnabled = false;
#ifdef Q_WS_X11
    d->filter = 0;
#endif
    d->cbDefaultColor = 0L;
    d->_mode = ChooserClassic;
    connect(this, SIGNAL(okClicked(void)), this, SLOT(slotWriteSettings(void)));
    connect(this, SIGNAL(closeClicked(void)), this, SLOT(slotWriteSettings(void)));

    QLabel *label;

    //
    // Create the top level page and its layout
    //
    QWidget *page = new QWidget(this);
    setMainWidget(page);

    QGridLayout *tl_layout = new QGridLayout(page);
    tl_layout->setMargin(0);
    d->tl_layout = tl_layout;
    tl_layout->addItem(new QSpacerItem(spacingHint()*2, 0), 0, 1);

    //
    // the more complicated part: the left side
    // add a V-box
    //
    QVBoxLayout *l_left = new QVBoxLayout();
    tl_layout->addLayout(l_left, 0, 0);

    //
    // add a H-Box for the XY-Selector and a grid for the
    // entry fields
    //
    QHBoxLayout *l_ltop = new QHBoxLayout();
    l_left->addLayout(l_ltop);

    //
    // the palette and value selector go into the H-box
    //
    d->hsSelector = new KHueSaturationSelector(page);
    d->hsSelector->setMinimumSize(256, 256);
    l_ltop->addWidget(d->hsSelector, 8);
    connect(d->hsSelector, SIGNAL(valueChanged(int, int)),
            SLOT(slotHSChanged(int, int)));

    d->valuePal = new KColorValueSelector(page);
    d->valuePal->setMinimumSize(26, 70);
    d->valuePal->setIndent(false);
    d->valuePal->setArrowDirection(Qt::RightArrow);
    l_ltop->addWidget(d->valuePal, 1);
    connect(d->valuePal, SIGNAL(valueChanged(int)),
            SLOT(slotVChanged(int)));

    d->alphaSelector = new KGradientSelector(Qt::Horizontal, page);
    d->alphaSelector->setFixedSize(256, 26);
    d->alphaSelector->setIndent(false);
    d->alphaSelector->setArrowDirection(Qt::DownArrow);
    d->alphaSelector->setRange(0, 255);
    l_left->addWidget(d->alphaSelector, 1);
    connect(d->alphaSelector, SIGNAL(valueChanged(int)),
            SLOT(slotAChanged(int)));

    // a little space between
    l_left->addSpacing(10); // FIXME: remove hardcoded values

    QGridLayout *l_lbot = new QGridLayout();
    l_left->addLayout(l_lbot);

    // button group that manages the radio buttons
    QRadioButton *modeButton;
    d->modeGroup = new QButtonGroup(page);
    connect(d->modeGroup, SIGNAL(buttonClicked(int)), SLOT(slotModeChanged(int)));

    //
    // add the HSV fields
    //
    l_lbot->setColumnStretch(2, 10);

    modeButton = new QRadioButton(i18n("Hue:"), page);
    l_lbot->addWidget(modeButton, 0, 0);
    d->modeGroup->addButton(modeButton, ChooserHue);

    d->hedit = new KIntSpinBox(page);
    d->hedit->setMaximum(359);
    d->hedit->setSuffix(i18nc("The angular degree unit (for hue)", "\302\260")); //  U+00B0 DEGREE SIGN
    l_lbot->addWidget(d->hedit, 0, 1);
    connect(d->hedit, SIGNAL(valueChanged(int)),
            SLOT(slotHSVChanged()));

    modeButton = new QRadioButton(i18n("Saturation:"), page);
    l_lbot->addWidget(modeButton, 1, 0);
    d->modeGroup->addButton(modeButton, ChooserSaturation);

    d->sedit = new KIntSpinBox(page);
    d->sedit->setMaximum(255);
    l_lbot->addWidget(d->sedit, 1, 1);
    connect(d->sedit, SIGNAL(valueChanged(int)),
            SLOT(slotHSVChanged()));

    modeButton = new QRadioButton(i18nc("This is the V of HSV", "Value:"), page);
    l_lbot->addWidget(modeButton, 2, 0);
    d->modeGroup->addButton(modeButton, ChooserValue);

    d->vedit = new KIntSpinBox(page);
    d->vedit->setMaximum(255);
    l_lbot->addWidget(d->vedit, 2, 1);
    connect(d->vedit, SIGNAL(valueChanged(int)),
            SLOT(slotHSVChanged()));


    //
    // add the RGB fields
    //
    modeButton = new QRadioButton(i18n("Red:"), page);
    l_lbot->addWidget(modeButton, 0, 3);
    d->modeGroup->addButton(modeButton, ChooserRed);

    d->redit = new KIntSpinBox(page);
    d->redit->setMaximum(255);
    l_lbot->addWidget(d->redit, 0, 4);
    connect(d->redit, SIGNAL(valueChanged(int)),
            SLOT(slotRGBChanged()));

    modeButton = new QRadioButton(i18n("Green:"), page);
    l_lbot->addWidget(modeButton, 1, 3);
    d->modeGroup->addButton(modeButton, ChooserGreen);

    d->gedit = new KIntSpinBox(page);
    d->gedit->setMaximum(255);
    l_lbot->addWidget(d->gedit, 1, 4);
    connect(d->gedit, SIGNAL(valueChanged(int)),
            SLOT(slotRGBChanged()));

    modeButton = new QRadioButton(i18n("Blue:"), page);
    l_lbot->addWidget(modeButton, 2, 3);
    d->modeGroup->addButton(modeButton, ChooserBlue);

    d->bedit = new KIntSpinBox(page);
    d->bedit->setMaximum(255);
    l_lbot->addWidget(d->bedit, 2, 4);
    connect(d->bedit, SIGNAL(valueChanged(int)),
            SLOT(slotRGBChanged()));

    d->alphaLabel = new KHBox(page);
    QWidget *spacer = new QWidget(d->alphaLabel);
    label = new QLabel(i18n("Alpha:"), d->alphaLabel);
    QStyleOptionButton option;
    option.initFrom(modeButton);
    QRect labelRect = modeButton->style()->subElementRect(QStyle::SE_RadioButtonContents, &option, modeButton);
    int indent = layoutDirection() == Qt::LeftToRight ? labelRect.left() : modeButton->geometry().right() - labelRect.right();
    spacer->setFixedWidth(indent);
    l_lbot->addWidget(d->alphaLabel, 3, 3);

    d->aedit = new KIntSpinBox(page);
    d->aedit->setMaximum(255);
    label->setBuddy(d->aedit);
    l_lbot->addWidget(d->aedit, 3, 4);
    connect(d->aedit, SIGNAL(valueChanged(int)),
            SLOT(slotAlphaChanged()));

    d->aedit->setVisible(false);
    d->alphaLabel->setVisible(false);
    d->alphaSelector->setVisible(false);

    //
    // add a layout for the right side
    //
    d->l_right = new QVBoxLayout;
    tl_layout->addLayout(d->l_right, 0, 2);

    //
    // Add the palette table
    //
    d->table = new KColorTable(page);
    d->l_right->addWidget(d->table, 10);

    connect(d->table, SIGNAL(colorSelected(const QColor &, const QString &)),
            SLOT(slotColorSelected(const QColor &, const QString &)));

    connect(
        d->table,
        SIGNAL(colorDoubleClicked(const QColor &, const QString &)),
        SLOT(slotColorDoubleClicked(const QColor &, const QString &))
    );
    // Store the default value for saving time.
    d->originalPalette = d->table->name();

    //
    // a little space between
    //
    d->l_right->addSpacing(10);

    QHBoxLayout *l_hbox = new QHBoxLayout();
    d->l_right->addItem(l_hbox);

    //
    // The add to custom colors button
    //
    QPushButton *addButton = new QPushButton(page);
    addButton->setText(i18n("&Add to Custom Colors"));
    l_hbox->addWidget(addButton, 0, Qt::AlignLeft);
    connect(addButton, SIGNAL(clicked()), SLOT(slotAddToCustomColors()));

    //
    // The color picker button
    //
    QPushButton* button = new QPushButton(page);
    button->setIcon(KIcon("color-picker"));
    int commonHeight = addButton->sizeHint().height();
    button->setFixedSize(commonHeight, commonHeight);
    l_hbox->addWidget(button, 0, Qt::AlignHCenter);
    connect(button, SIGNAL(clicked()), SLOT(slotColorPicker()));

    //
    // a little space between
    //
    d->l_right->addSpacing(10);

    //
    // and now the entry fields and the patch (=colored box)
    //
    QGridLayout *l_grid = new QGridLayout();
    d->l_right->addLayout(l_grid);

    l_grid->setColumnStretch(2, 1);

    label = new QLabel(page);
    label->setText(i18n("Name:"));
    l_grid->addWidget(label, 0, 1, Qt::AlignLeft);

    d->colorName = new QLabel(page);
    l_grid->addWidget(d->colorName, 0, 2, Qt::AlignLeft);

    label = new QLabel(page);
    label->setText(i18n("HTML:"));
    l_grid->addWidget(label, 1, 1, Qt::AlignLeft);

    d->htmlName = new KLineEdit(page);
    d->htmlName->setMaxLength(13);   // Qt's QColor allows 12 hexa-digits
    d->htmlName->setText("#FFFFFF"); // But HTML uses only 6, so do not worry about the size
    int w = d->htmlName->fontMetrics().width(QLatin1String("#DDDDDDD"));
    d->htmlName->setFixedWidth(w);
    l_grid->addWidget(d->htmlName, 1, 2, Qt::AlignLeft);

    connect(d->htmlName, SIGNAL(textChanged(const QString &)),
            SLOT(slotHtmlChanged()));

    d->patch = new KColorPatch(page);
    d->patch->setFixedSize(48, 48);
    l_grid->addWidget(d->patch, 0, 0, 2, 1, Qt::AlignHCenter | Qt::AlignVCenter);
    connect(d->patch, SIGNAL(colorChanged(const QColor&)),
            SLOT(setColor(const QColor&)));

    //
    // chain fields together
    //
    setTabOrder(d->hedit, d->sedit);
    setTabOrder(d->sedit, d->vedit);
    setTabOrder(d->vedit, d->redit);
    setTabOrder(d->redit, d->gedit);
    setTabOrder(d->gedit, d->bedit);
    setTabOrder(d->bedit, d->aedit);

    tl_layout->activate();
    page->setMinimumSize(page->sizeHint());

    readSettings();
    d->bRecursion = false;
    d->bEditHsv = false;
    d->bEditRgb = false;
    d->bEditHtml = false;

    setFixedSize(sizeHint());
    QColor col;
    col.setHsv(0, 0, 255);
    d->_setColor(col);

// FIXME: with enabled event filters, it crashes after ever enter of a drag.
// better disable drag and drop than crashing it...
//   d->htmlName->installEventFilter(this);
//   d->hsSelector->installEventFilter(this);
    d->hsSelector->setAcceptDrops(true);

    d->setChooserMode(ChooserValue);
}

KColorDialog::~KColorDialog()
{
#ifdef Q_WS_X11
    if (d->bColorPicking && kapp)
        kapp->removeX11EventFilter(d->filter);
#endif
    delete d;
}

bool
KColorDialog::eventFilter(QObject *obj, QEvent *ev)
{
    if ((obj == d->htmlName) || (obj == d->hsSelector))
        switch (ev->type()) {
        case QEvent::DragEnter:
        case QEvent::DragMove:
        case QEvent::DragLeave:
        case QEvent::Drop:
        case QEvent::DragResponse:
            qApp->sendEvent(d->patch, ev);
            return true;
        default:
            break;
        }
    return KDialog::eventFilter(obj, ev);
}

void
KColorDialog::setDefaultColor(const QColor& col)
{
    if (!d->cbDefaultColor) {
        //
        // a little space between
        //
        d->l_right->addSpacing(10);

        //
        // and the "default color" checkbox, under all items on the right side
        //
        d->cbDefaultColor = new QCheckBox(i18n("Default color"), mainWidget());

        d->l_right->addWidget(d->cbDefaultColor);

        mainWidget()->setMaximumSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX);   // cancel setFixedSize()
        d->tl_layout->activate();
        mainWidget()->setMinimumSize(mainWidget()->sizeHint());
        setFixedSize(sizeHint());

        connect(d->cbDefaultColor, SIGNAL(clicked()), SLOT(slotDefaultColorClicked()));
    }

    d->defaultColor = col;

    d->slotDefaultColorClicked();
}

QColor KColorDialog::defaultColor() const
{
    return d->defaultColor;
}

void KColorDialog::setAlphaChannelEnabled(bool alpha)
{
    if (d->bAlphaEnabled != alpha) {
        d->bAlphaEnabled = alpha;
        d->aedit->setVisible(d->bAlphaEnabled);
        d->alphaLabel->setVisible(d->bAlphaEnabled);
        d->alphaSelector->setVisible(d->bAlphaEnabled);

        mainWidget()->setMaximumSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX);   // cancel setFixedSize()
        d->tl_layout->activate();
        mainWidget()->setMinimumSize(mainWidget()->sizeHint());
        setFixedSize(sizeHint());
    }
}

bool KColorDialog::isAlphaChannelEnabled() const
{
    return d->bAlphaEnabled;
}


void KColorDialog::KColorDialogPrivate::setChooserMode(KColorChooserMode c)
{
    _mode = c;
    hsSelector->setChooserMode(c);
    valuePal->setChooserMode(c);

    modeGroup->button(valuePal->chooserMode())->setChecked(true);
    valuePal->updateContents();
    hsSelector->updateContents();
    valuePal->update();
    hsSelector->update();
    slotHSVChanged();
}


KColorChooserMode KColorDialog::KColorDialogPrivate::chooserMode()
{
    return _mode;
}

void KColorDialog::KColorDialogPrivate::slotDefaultColorClicked()
{
    if (cbDefaultColor->isChecked()) {
        selColor = defaultColor;
        showColor(selColor, i18n("-default-"));
    } else {
        showColor(selColor, QString());
    }
    emit q->colorSelected(selColor);
}

void
KColorDialog::KColorDialogPrivate::slotModeChanged(int id)
{
    setChooserMode(KColorChooserMode(id));
}

void
KColorDialog::readSettings()
{
    KConfigGroup group(KGlobal::config(), "Colors");

    QString collectionName = group.readEntry("CurrentPalette");
    if (collectionName.isEmpty()) {
        collectionName = i18nc("palette name", colorCollectionName[fortyColorIndex].m_displayName);
    } else {
        for (int i = 0; colorCollectionName[i].m_fileName; ++i) {
            if (collectionName == colorCollectionName[i].m_displayName) {
                collectionName = i18nc("palette name", colorCollectionName[i].m_displayName);
                break;
            }
        }
    }

    d->table->setColors(collectionName);
}

void
KColorDialog::KColorDialogPrivate::slotWriteSettings()
{
    KConfigGroup group(KGlobal::config(), "Colors");

    QString collectionName = table->name();
    if (!group.hasDefault("CurrentPalette") && table->name() == originalPalette) {
        group.revertToDefault("CurrentPalette");
    } else {
        QString collectionName(table->name());
        for (int i = 0; colorCollectionName[i].m_fileName; ++i) {
            if (collectionName == i18nc("palette name", colorCollectionName[i].m_displayName)) {
                collectionName = colorCollectionName[i].m_displayName;
                break;
            }
        }
        group.writeEntry("CurrentPalette", collectionName); //Make sure the untranslated name is saved, assuming there is one
    }
}

QColor
KColorDialog::color() const
{
    if (d->cbDefaultColor && d->cbDefaultColor->isChecked())
        return QColor();
    if (d->selColor.isValid())
        d->table->addToRecentColors(d->selColor);
    return d->selColor;
}

void KColorDialog::setColor(const QColor &col)
{
    d->_setColor(col);
}

//
// static function to display dialog and return color
//
int KColorDialog::getColor(QColor &theColor, QWidget *parent)
{
    KColorDialog dlg(parent, true);
    dlg.setObjectName("Color Selector");
    if (theColor.isValid())
        dlg.setColor(theColor);
    int result = dlg.exec();

    if (result == Accepted) {
        theColor = dlg.color();
    }

    return result;
}

//
// static function to display dialog and return color
//
int KColorDialog::getColor(QColor &theColor, const QColor& defaultCol, QWidget *parent)
{
    KColorDialog dlg(parent, true);
    dlg.setObjectName("Color Selector");
    dlg.setDefaultColor(defaultCol);
    dlg.setColor(theColor);
    int result = dlg.exec();

    if (result == Accepted)
        theColor = dlg.color();

    return result;
}

void KColorDialog::KColorDialogPrivate::slotRGBChanged(void)
{
    if (bRecursion) return;
    int red = redit->value();
    int grn = gedit->value();
    int blu = bedit->value();

    if (red > 255 || red < 0) return;
    if (grn > 255 || grn < 0) return;
    if (blu > 255 || blu < 0) return;

    QColor col;
    col.setRgb(red, grn, blu, aedit->value());
    bEditRgb = true;
    _setColor(col);
    bEditRgb = false;
}

void KColorDialog::KColorDialogPrivate::slotAlphaChanged(void)
{
    if (bRecursion) return;
    int alpha = aedit->value();

    if (alpha > 255 || alpha < 0) return;

    QColor col = selColor;
    col.setAlpha(alpha);
    _setColor(col);
}

void KColorDialog::KColorDialogPrivate::slotHtmlChanged(void)
{
    if (bRecursion || htmlName->text().isEmpty()) return;

    QString strColor(htmlName->text());

    // Assume that a user does not want to type the # all the time
    if (strColor[0] != '#') {
        bool signalsblocked = htmlName->blockSignals(true);
        strColor.prepend("#");
        htmlName->setText(strColor);
        htmlName->blockSignals(signalsblocked);
    }

    const QColor color(strColor);

    if (color.isValid()) {
        QColor col(color);
        bEditHtml = true;
        _setColor(col);
        bEditHtml = false;
    }
}

void KColorDialog::KColorDialogPrivate::slotHSVChanged(void)
{
    if (bRecursion) return;
    int hue = hedit->value();
    int sat = sedit->value();
    int val = vedit->value();

    if (hue > 359 || hue < 0) return;
    if (sat > 255 || sat < 0) return;
    if (val > 255 || val < 0) return;

    QColor col;
    col.setHsv(hue, sat, val, aedit->value());
    bEditHsv = true;
    _setColor(col);
    bEditHsv = false;
}

void KColorDialog::KColorDialogPrivate::slotHSChanged(int x, int y)
{
    QColor col = selColor;
    KColorChooserMode xMode = chooserXMode(chooserMode());
    KColorChooserMode yMode = chooserYMode(chooserMode());
    setComponentValue(col, xMode, x / (xMode == ChooserHue ? 360.0 : 255.0));
    setComponentValue(col, yMode, y / (yMode == ChooserHue ? 360.0 : 255.0));
    _setColor(col);
}

void KColorDialog::KColorDialogPrivate::slotVChanged(int v)
{
    QColor col = selColor;
    setComponentValue(col, chooserMode(), v / (chooserMode() == ChooserHue ? 360.0 : 255.0));
    _setColor(col);
}

void KColorDialog::KColorDialogPrivate::slotAChanged(int value)
{
    QColor col = selColor;
    col.setAlpha(value);
    _setColor(col);
}

void KColorDialog::KColorDialogPrivate::slotColorSelected(const QColor &color)
{
    _setColor(color);
}

void KColorDialog::KColorDialogPrivate::slotAddToCustomColors()
{
    table->addToCustomColors(selColor);
}

void KColorDialog::KColorDialogPrivate::slotColorSelected(const QColor &color, const QString &name)
{
    _setColor(color, name);
}

void KColorDialog::KColorDialogPrivate::slotColorDoubleClicked
(
    const QColor  & color,
    const QString & name
)
{
    _setColor(color, name);
    q->accept();
}

void KColorDialog::KColorDialogPrivate::_setColor(const QColor &color, const QString &name)
{
    if (color.isValid()) {
        if (cbDefaultColor && cbDefaultColor->isChecked())
            cbDefaultColor->setChecked(false);
        selColor = color;
    } else {
        if (cbDefaultColor && cbDefaultColor->isChecked())
            cbDefaultColor->setChecked(true);
        selColor = defaultColor;
    }

    showColor(selColor, name);

    emit q->colorSelected(selColor);
}

// show but don't set into selColor, nor emit colorSelected
void KColorDialog::KColorDialogPrivate::showColor(const QColor &color, const QString &name)
{
    bRecursion = true;

    if (name.isEmpty())
        colorName->setText(i18n("-unnamed-"));
    else
        colorName->setText(name);

    patch->setColor(color);

    setRgbEdit(color);
    setHsvEdit(color);
    setHtmlEdit(color);
    aedit->setValue(color.alpha());

    QColor rgbColor = color.toRgb();
    bool ltr = q->layoutDirection() == Qt::LeftToRight;
    rgbColor.setAlpha(ltr ? 0 : 255);
    alphaSelector->setFirstColor(rgbColor);
    rgbColor.setAlpha(ltr ? 255 : 0);
    alphaSelector->setSecondColor(rgbColor);
    alphaSelector->setValue(color.alpha());

    KColorChooserMode xMode = chooserXMode(chooserMode());
    KColorChooserMode yMode = chooserYMode(chooserMode());
    int xValue = qRound(getComponentValue(color, xMode) * (xMode == ChooserHue ? 360.0 : 255.0));
    int yValue = qRound(getComponentValue(color, yMode) * (yMode == ChooserHue ? 360.0 : 255.0));
    int value = qRound(getComponentValue(color, chooserMode()) * (chooserMode() == ChooserHue ? 360.0 : 255.0));
    hsSelector->setValues(xValue, yValue);
    valuePal->setValue(value);

    bool blocked = valuePal->blockSignals(true);

    valuePal->setHue(color.hue());
    valuePal->setSaturation(color.saturation());
    valuePal->setColorValue(color.value());
    valuePal->updateContents();
    valuePal->blockSignals(blocked);
    valuePal->update();

    blocked = hsSelector->blockSignals(true);

    hsSelector->setHue(color.hue());
    hsSelector->setSaturation(color.saturation());
    hsSelector->setColorValue(color.value());
    hsSelector->updateContents();
    hsSelector->blockSignals(blocked);
    hsSelector->update();

    bRecursion = false;
}



void
KColorDialog::KColorDialogPrivate::slotColorPicker()
{
    bColorPicking = true;
#ifdef Q_WS_X11
    filter = new KCDPickerFilter(q);
    kapp->installX11EventFilter(filter);
#endif
    q->grabMouse(Qt::CrossCursor);
    q->grabKeyboard();
}

void
KColorDialog::mouseMoveEvent(QMouseEvent *e)
{
    if (d->bColorPicking) {
        d->_setColor(grabColor(e->globalPos()));
        return;
    }

    KDialog::mouseMoveEvent(e);
}

void
KColorDialog::mouseReleaseEvent(QMouseEvent *e)
{
    if (d->bColorPicking) {
        d->bColorPicking = false;
#ifdef Q_WS_X11
        kapp->removeX11EventFilter(d->filter);
        delete d->filter; d->filter = 0;
#endif
        releaseMouse();
        releaseKeyboard();
        d->_setColor(grabColor(e->globalPos()));
        return;
    }
    KDialog::mouseReleaseEvent(e);
}

QColor
KColorDialog::grabColor(const QPoint &p)
{
#ifdef Q_WS_X11
    // we use the X11 API directly in this case as we are not getting back a valid
    // return from QPixmap::grabWindow in the case where the application is using
    // an argb visual
    if( !qApp->desktop()->geometry().contains( p ))
        return QColor();
    Window root = RootWindow(QX11Info::display(), QX11Info::appScreen());
    XImage *ximg = XGetImage(QX11Info::display(), root, p.x(), p.y(), 1, 1, -1, ZPixmap);
    unsigned long xpixel = XGetPixel(ximg, 0, 0);
    XDestroyImage(ximg);
    XColor xcol;
    xcol.pixel = xpixel;
    xcol.flags = DoRed | DoGreen | DoBlue;
    XQueryColor(QX11Info::display(),
                DefaultColormap(QX11Info::display(), QX11Info::appScreen()),
                &xcol);
    return QColor::fromRgbF(xcol.red / 65535.0, xcol.green / 65535.0, xcol.blue / 65535.0);
#else
    QWidget *desktop = QApplication::desktop();
    QPixmap pm = QPixmap::grabWindow(desktop->winId(), p.x(), p.y(), 1, 1);
    QImage i = pm.toImage();
    return i.pixel(0, 0);
#endif
}

void
KColorDialog::keyPressEvent(QKeyEvent *e)
{
    if (d->bColorPicking) {
        if (e->key() == Qt::Key_Escape) {
            d->bColorPicking = false;
#ifdef Q_WS_X11
            kapp->removeX11EventFilter(d->filter);
            delete d->filter; d->filter = 0;
#endif
            releaseMouse();
            releaseKeyboard();
        }
        e->accept();
        return;
    }
    KDialog::keyPressEvent(e);
}

void KColorDialog::KColorDialogPrivate::setRgbEdit(const QColor &col)
{
    if (bEditRgb) return;
    int r, g, b;
    col.getRgb(&r, &g, &b);

    redit->setValue(r);
    gedit->setValue(g);
    bedit->setValue(b);
}

void KColorDialog::KColorDialogPrivate::setHtmlEdit(const QColor &col)
{
    if (bEditHtml) return;
    int r, g, b;
    col.getRgb(&r, &g, &b);
    QString num;

    num.sprintf("#%02X%02X%02X", r, g, b);
    htmlName->setText(num);
}


void KColorDialog::KColorDialogPrivate::setHsvEdit(const QColor &col)
{
    if (bEditHsv) return;
    int h, s, v;
    col.getHsv(&h, &s, &v);

    hedit->setValue(h);
    sedit->setValue(s);
    vedit->setValue(v);
}

#include "kcolordialog.moc"
#include "kcolordialog_p.moc"
