/*

  Copyright (c) 2000 KDE Project

  Permission is hereby granted, free of charge, to any person obtaining a
  copy of this software and associated documentation files (the "Software"),
  to deal in the Software without restriction, including without limitation
  the rights to use, copy, modify, merge, publish, distribute, sublicense,
  and/or sell copies of the Software, and to permit persons to whom the
  Software is furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in
  all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
  THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
  DEALINGS IN THE SOFTWARE.

 */

#include "klegacystyle.h"
#include "klegacystyle.moc"
#include <klocale.h>
#include <kiconloader.h>

#define INCLUDE_MENUITEM_DEF
#include <qapplication.h>
#include <qbitmap.h>
#include <q3buttongroup.h>
#include <q3canvas.h>
#include <qcheckbox.h>
#include <qcolor.h>
#include <qcolordialog.h>
#include <qcombobox.h>
#include <qdial.h>
#include <qdialog.h>
#include <q3dict.h>
#include <qfile.h>
#include <q3filedialog.h>
#include <qfileinfo.h>
#include <qfont.h>
#include <qfontdialog.h>
#include <qframe.h>
#include <qpointer.h>
#include <q3grid.h>
#include <q3groupbox.h>
#include <khbox.h>
#include <qhbuttongroup.h>
#include <q3header.h>
#include <qhgroupbox.h>
#include <q3iconview.h>
#include <qimage.h>
#include <qinputdialog.h>
#include <q3intdict.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qlcdnumber.h>
#include <qlineedit.h>
#include <q3ptrlist.h>
#include <q3listbox.h>
#include <q3listview.h>
#include <q3mainwindow.h>
#include <qmenubar.h>
#include <qmenudata.h>
#include <qmessagebox.h>
#include <q3multilineedit.h>
#include <qobject.h>
#include <qpainter.h>
#include <qpixmap.h>
#include <qpixmapcache.h>
#include <q3popupmenu.h>
#include <qprintdialog.h>
#include <q3progressbar.h>
#include <q3progressdialog.h>
#include <qpushbutton.h>
#include <qradiobutton.h>
#include <qregexp.h>
#include <qscrollbar.h>
#include <q3scrollview.h>
#include <qsemimodal.h>
#include <qsizegrip.h>
#include <qslider.h>
#include <qspinbox.h>
#include <qsplitter.h>
#include <qstatusbar.h>
#include <qstring.h>
#include <qtabbar.h>
#include <q3tabdialog.h>
#include <qtableview.h>
#include <qtabwidget.h>
#include <q3textbrowser.h>
#include <qtextstream.h>
#include <q3textview.h>
#include <q3toolbar.h>
#include <qtoolbutton.h>
#include <qtooltip.h>
#include <kvbox.h>
#include <qvbuttongroup.h>
#include <qvgroupbox.h>
#include <qwidget.h>
#include <q3widgetstack.h>
#include <q3wizard.h>
#include <qworkspace.h>

#include <stdlib.h>
#include <unistd.h>
#include <limits.h>
#include <sys/types.h>

// forward declaration of classes
class KLegacyBorder;
class KLegacyStyleData;
class KLegacyImageData;
class GtkObject;

// declaration of hidden functions
typedef void (QStyle::*QDrawMenuBarItemImpl) (QPainter *, int, int, int, int,
					      QMenuItem *, QColorGroup &, bool, bool);
extern QDrawMenuBarItemImpl qt_set_draw_menu_bar_impl(QDrawMenuBarItemImpl impl);

// the addresses of the integers are used to place things in the
// style data dict
static int listviewitem_ptr   = 0;
static int listboxitem_ptr    = 1;
static int menuitem_ptr       = 2;
static int separator_ptr      = 3;
static int arrow_ptr          = 4;
static int whatsthis_ptr      = 5;
static int checkmenuitem_ptr  = 6;
static int radiomenuitem_ptr  = 7;
static int eventbox_ptr       = 8;

// a QImage cache, since we need to resize some images to different sizes, we
// will cache them, to save the overhead of loading the image from disk each
// time it's needed
static const int imageCacheSize = 61;
static Q3Dict<QImage> *imageCache = 0;


class KLegacy {
public:
    enum Function { Box = 1, FlatBox, Extension, Check, Option,
		    HLine, VLine, BoxGap, Slider, Tab, Arrow, Handle, FShadow, Focus };
    enum State    { Normal = 1, Prelight, Active, Insensitive, Selected };
    enum Shadow   { NoShadow = 0, In, Out, EtchedIn, EtchedOut };
    enum GapSide  { Qt::DockLeft = 1, Qt::DockRight, Qt::DockTop, Qt::DockBottom };
};


class KLegacyBorder : public KLegacy {
private:
    int l, r, t, b;


public:
    KLegacyBorder(int ll = 0, int rr = 0, int tt = 0, int bb = 0)
	: l(ll), r(rr), t(tt), b(bb)
    { }

    KLegacyBorder(const KLegacyBorder &br)
	: l(br.l), r(br.r), t(br.t), b(br.b)
    { }

    inline int left(void) const
    { return l; }
    inline int right(void) const
    { return r; }
    inline int top(void) const
    { return t; }
    inline int bottom(void) const
    { return b; }

    inline void setLeft(int ll)
    { l = ll; }
    inline void setRight(int rr)
    { r = rr; }
    inline void setTop(int tt)
    { t = tt; }
    inline void setBottom(int bb)
    { b = bb; }
};


struct KLegacyImageDataKeyField {
    Q_INT8 function       : 8;
    Q_INT8 state          : 8;
    Q_INT8 shadow         : 4;
    Q_INT8 orientation    : 4;
    Q_INT8 arrowDirection : 4;
    Q_INT8 gapSide        : 4;
};


union KLegacyImageDataKey {
    KLegacyImageDataKeyField data;
    long cachekey;
};


class KLegacyImageData : public KLegacy {
public:
    KLegacyImageDataKey key;

    QString file;
    QString detail;
    QString overlayFile;
    QString gapFile;
    QString gapStartFile;
    QString gapEndFile;

    KLegacyBorder border;
    KLegacyBorder overlayBorder;
    KLegacyBorder gapBorder;
    KLegacyBorder gapStartBorder;
    KLegacyBorder gapEndBorder;

    bool recolorable;
    bool stretch;
    bool overlayStretch;

    KLegacyImageData()
	: recolorable(false),
	  stretch(false),
	  overlayStretch(false)
    { key.cachekey = 0; }
};


class KLegacyStyleData : public KLegacy {
public:
    // name of this style
    QString name;

    // font to use
    QFont *fn;

    // list of image datas (which tell us how to draw things)
    QList<KLegacyImageData> imageList;

    // background, foreground and base colors for the 5 widget
    //states that Gtk defines
    QColor back[5], fore[5], base[5];

    // reference count
    int ref;

    KLegacyStyleData()
	: fn(0), ref(0)
    {
	// have the imageList delete the items it holds when it's deleted
	imageList.setAutoDelete(true);
    }
};


class GtkObject : public QObject {
private:
    KLegacyStyleData *d;

    friend class KLegacyStylePrivate;


public:
    GtkObject(GtkObject *parent, const char *name)
	: QObject(parent, name)
    { d = 0; }

    GtkObject *find(QRegExp &) const;

    QColor backColor(KLegacy::State);
    QColor baseColor(KLegacy::State);
    QColor foreColor(KLegacy::State);

    QFont *font();

    inline QString styleName()
    { return styleData()->name; }

    KLegacyStyleData *styleData();
    KLegacyImageData *getImageData(KLegacyImageDataKey,
				const QString & = QString::null);

    QPixmap *draw(KLegacyImageDataKey, int, int, const QString & = QString::null);
    QPixmap *draw(KLegacyImageData *, int, int);
};


static QPixmap *drawImage(QImage *image, int width, int height,
			  KLegacyBorder border, bool scale)
{
    if ((! image) || (image->isNull()) || (width < 1) || (height < 1)) {
	return (QPixmap *) 0;
    }

    QPixmap *pixmap = new QPixmap(width, height);

    if (scale) {
	if (width < 2) width = 2;
	if (height < 2) height = 2;

	int x[3], y[3], w[3], h[3], x2[3], y2[3], w2[3], h2[3];

	// left
	x[0] = x2[0] = 0;
	w[0] = (border.left() < 1) ? 1 : border.left();

	// middle
	x[1] = border.left();
	w[1] = image->width() - border.left() - border.right();
	if (w[1] < 1) w[1] = 1;

	// right
	x[2] = image->width() - border.right();
	w[2] = (border.right() < 1) ? 1 : border.right();
	if (x[2] < 0) x[2] = 0;

	if ((border.left() + border.right()) > width) {
	    // left
	    x2[0] = 0;
	    w2[0] = (width / 2) + 1;

	    // middle
	    x2[1] = w2[0] - 1;
	    w2[1] = 1;

	    // right
	    x2[2] = x2[1];
	    w2[2] = w2[0];
	} else {
	    // left
	    x2[0] = 0;
	    w2[0] = border.left();

	    // middle
	    x2[1] = w2[0];
	    w2[1] = width - border.left() - border.right() + 1;

	    // right
	    x2[2] = width - border.right();
	    w2[2] = border.right();
	}

	// top
	y[0] = 0;
	h[0] = (border.top() < 1) ? 1 : border.top();

	// middle
	y[1] = border.top();
	h[1] = image->height() - border.top() - border.bottom();
	if (h[1] < 1) h[1] = 1;

	// bottom
	y[2] = image->height() - border.bottom();
	h[2] = (border.bottom() < 1) ? 1 : border.bottom();
	if (y[2] < 0) y[2] = 0;

	if ((border.top() + border.bottom()) > height) {
	    // left
	    y2[0] = 0;
	    h2[0] = height / 2;

	    // middle
	    y2[1] = h2[0];
	    h2[1] = 1;

	    // right
	    y2[2] = y2[1];
	    h2[2] = h2[0];
	} else {
	    // left
	    y2[0] = 0;
	    h2[0] = border.top();

	    // middle
	    y2[1] = h2[0];
	    h2[1] = height - border.top() - border.bottom() + 1;

	    // bottom
	    y2[2] = height - border.bottom();
	    h2[2] = border.bottom();
	}

	// draw the image
	bool mask = image->hasAlphaBuffer();
	QBitmap bm(width, height);
	bm.fill(Qt::color1);

	QImage nimage[3][3];
	int xx = -1, yy = -1;
	while (++yy < 3) {
	    xx = -1;
	    while (++xx < 3) {
		nimage[yy][xx] = image->copy(x[xx], y[yy], w[xx], h[yy]);

		if (nimage[yy][xx].isNull()) continue;

		if ((w[xx] != w2[xx]) || (h[yy] != h2[yy]))
		    nimage[yy][xx] = nimage[yy][xx].smoothScale(w2[xx], h2[yy]);

		if (nimage[yy][xx].isNull()) continue;

		bitBlt(pixmap, x2[xx], y2[yy], &nimage[yy][xx],
		       0, 0, w2[xx], h2[yy], Qt::CopyROP);

		if (mask) {
                    QImage am = nimage[yy][xx].createAlphaMask();
		    bitBlt(&bm, x2[xx], y2[yy], &am,
			   0, 0, w2[xx], h2[yy], Qt::CopyROP);
                }
	    }
	}

	if (mask)
	    pixmap->setMask(bm);
    } else {
	for (int y = 0; y < height; y += image->height())
	    for (int x = 0; x < width; x += image->width())
		bitBlt(pixmap, x, y, image, 0, 0, -1, -1, Qt::CopyROP);

	if (image->hasAlphaBuffer()) {
	    QImage mask = image->createAlphaMask();

	    if (! mask.isNull() && mask.depth() == 1) {
		QBitmap bm(width, height);
		bm.fill(Qt::color1);
		bm = mask;
		pixmap->setMask(bm);
	    }
	}
    }

    return pixmap;
}


// Generate an object tree for all the known Gtk widgets...
// returns a pointer to the bottom of the tree
static GtkObject *initialize(Q3PtrDict<GtkObject> &dict) {
    //
    // auto generated stuff from :
    // --
    // #!/usr/bin/perl -w
    //
    // foreach $line ( <STDIN> ) {
    //     chomp $line;
    //     $line =~ s/[^\sa-zA-Z0-9]/ /g;
    //     $line =~ /^(\s*)(\S*)/;
    //     $prefixlength = length $1;
    //     $classname = $2;
    //     $class{$prefixlength} = $classname;
    //     $prefixlength--;
    //     while( $prefixlength >= 0 && !defined($class{$prefixlength}) ) {
    //       $prefixlength--;
    //     }
    //     $parent = $class{$prefixlength};
    //     $parent = "0" if ( $parent eq $classname );
    //
    //     # for GtkBin:
    //     # myGtkBin = new GtkObject( myGtkWidget, "GtkBin" );
    //
    //     print "GtkObject * my$classname =
    //                new GtkObject( my$parent, \"$classname\" );\n";
    // }
    // --

    GtkObject * myGtkObject =
	new GtkObject( 0, "GtkObject" );
    GtkObject * myGtkWidget =
	new GtkObject( myGtkObject, "GtkWidget" );
    GtkObject * myGtkMisc =
	new GtkObject( myGtkWidget, "GtkMisc" );
    GtkObject * myGtkLabel =
	new GtkObject( myGtkMisc, "GtkLabel" );
    // GtkObject * myGtkAccelLabel =
    //  new GtkObject( myGtkLabel, "GtkAccelLabel" );
    GtkObject * myGtkTipsQuery =
	new GtkObject( myGtkLabel, "GtkTipsQuery" );
    GtkObject * myGtkArrow =
	new GtkObject( myGtkMisc, "GtkArrow" );
    // GtkObject * myGtkImage =
    //  new GtkObject( myGtkMisc, "GtkImage" );
    // GtkObject * myGtkPixmap =
    //  new GtkObject( myGtkMisc, "GtkPixmap" );
    GtkObject * myGtkContainer =
	new GtkObject( myGtkWidget, "GtkContainer" );
    GtkObject * myGtkBin =
	new GtkObject( myGtkContainer, "GtkBin" );
    // GtkObject * myGtkAlignment =
    //  new GtkObject( myGtkBin, "GtkAlignment" );
    GtkObject * myGtkFrame =
	new GtkObject( myGtkBin, "GtkFrame" );
    // GtkObject * myGtkAspectFrame =
    //  new GtkObject( myGtkFrame, "GtkAspectFrame" );
    GtkObject * myGtkButton =
	new GtkObject( myGtkBin, "GtkButton" );
    GtkObject * myGtkToggleButton =
	new GtkObject( myGtkButton, "GtkToggleButton" );
    GtkObject * myGtkCheckButton =
	new GtkObject( myGtkToggleButton, "GtkCheckButton" );
    GtkObject * myGtkRadioButton =
	new GtkObject( myGtkCheckButton, "GtkRadioButton" );
    GtkObject * myGtkOptionMenu =
	new GtkObject( myGtkButton, "GtkOptionMenu" );
    GtkObject * myGtkItem =
	new GtkObject( myGtkBin, "GtkItem" );
    GtkObject * myGtkMenuItem =
	new GtkObject( myGtkItem, "GtkMenuItem" );
    GtkObject * myGtkCheckMenuItem =
	new GtkObject( myGtkMenuItem, "GtkCheckMenuItem" );
    GtkObject * myGtkRadioMenuItem =
	new GtkObject( myGtkCheckMenuItem, "GtkRadioMenuItem" );
    // GtkObject * myGtkTearoffMenuItem =
    //  new GtkObject( myGtkMenuItem, "GtkTearoffMenuItem" );
    GtkObject * myGtkListItem =
	new GtkObject( myGtkItem, "GtkListItem" );
    GtkObject * myGtkTreeItem =
	new GtkObject( myGtkItem, "GtkTreeItem" );
    GtkObject * myGtkWindow =
	new GtkObject( myGtkBin, "GtkWindow" );
    GtkObject * myGtkColorSelectionDialog =
	new GtkObject( myGtkWindow, "GtkColorSelectionDialog" );
    GtkObject * myGtkDialog =
	new GtkObject( myGtkWindow, "GtkDialog" );
    GtkObject * myGtkInputDialog =
	new GtkObject( myGtkDialog, "GtkInputDialog" );
    // GtkObject * myGtkDrawWindow =
    //  new GtkObject( myGtkWindow, "GtkDrawWindow" );
    GtkObject * myGtkFileSelection =
	new GtkObject( myGtkWindow, "GtkFileSelection" );
    GtkObject * myGtkFontSelectionDialog =
	new GtkObject( myGtkWindow, "GtkFontSelectionDialog" );
    // GtkObject * myGtkPlug =
    //  new GtkObject( myGtkWindow, "GtkPlug" );
    GtkObject * myGtkEventBox =
	new GtkObject( myGtkBin, "GtkEventBox" );
    // GtkObject * myGtkHandleBox =
    //  new GtkObject( myGtkBin, "GtkHandleBox" );
    // GtkObject * myGtkScrolledWindow =
    //  new GtkObject( myGtkBin, "GtkScrolledWindow" );
    GtkObject * myGtkViewport =
	new GtkObject( myGtkBin, "GtkViewport" );
    GtkObject * myGtkBox =
	new GtkObject( myGtkContainer, "GtkBox" );
    GtkObject * myGtkButtonBox =
	new GtkObject( myGtkBox, "GtkButtonBox" );
    GtkObject * myGtkHButtonBox =
	new GtkObject( myGtkButtonBox, "GtkHButtonBox" );
    GtkObject * myGtkVButtonBox =
	new GtkObject( myGtkButtonBox, "GtkVButtonBox" );
    GtkObject * myGtkVBox =
	new GtkObject( myGtkBox, "GtkVBox" );
    // GtkObject * myGtkColorSelection =
    //  new GtkObject( myGtkVBox, "GtkColorSelection" );
    // GtkObject * myGtkGammaCurve =
    //  new GtkObject( myGtkVBox, "GtkGammaCurve" );
    GtkObject * myGtkHBox =
	new GtkObject( myGtkBox, "GtkHBox" );


    // CHANGED!  It seems that the gtk optionmenu and gtk combobox aren't related,
    // but in Qt they are the same class... so we have changed gth GtkCombo to inherit
    // from GtkOptionMenu (so that Qt comboboxes look like the optionmenus by default)
    GtkObject * myGtkCombo =
	new GtkObject( myGtkOptionMenu, "GtkCombo" );


    GtkObject * myGtkStatusbar =
	new GtkObject( myGtkHBox, "GtkStatusbar" );
    GtkObject * myGtkCList =
	new GtkObject( myGtkContainer, "GtkCList" );
    GtkObject * myGtkCTree =
	new GtkObject( myGtkCList, "GtkCTree" );
    // GtkObject * myGtkFixed =
    //  new GtkObject( myGtkContainer, "GtkFixed" );
    GtkObject * myGtkNotebook =
	new GtkObject( myGtkContainer, "GtkNotebook" );
    // GtkObject * myGtkFontSelection =
    //  new GtkObject( myGtkNotebook, "GtkFontSelection" );
    GtkObject * myGtkPaned =
	new GtkObject( myGtkContainer, "GtkPaned" );
    // GtkObject * myGtkHPaned =
    //  new GtkObject( myGtkPaned, "GtkHPaned" );
    // GtkObject * myGtkVPaned =
    // new GtkObject( myGtkPaned, "GtkVPaned" );
    // GtkObject * myGtkLayout =
    //  new GtkObject( myGtkContainer, "GtkLayout" );
    // GtkObject * myGtkList =
    //  new GtkObject( myGtkContainer, "GtkList" );
    GtkObject * myGtkMenuShell =
	new GtkObject( myGtkContainer, "GtkMenuShell" );
    GtkObject * myGtkMenuBar =
	new GtkObject( myGtkMenuShell, "GtkMenuBar" );
    GtkObject * myGtkMenu =
	new GtkObject( myGtkMenuShell, "GtkMenu" );
    // GtkObject * myGtkPacker =
    //  new GtkObject( myGtkContainer, "GtkPacker" );
    // GtkObject * myGtkSocket =
    //  new GtkObject( myGtkContainer, "GtkSocket" );
    GtkObject * myGtkTable =
	new GtkObject( myGtkContainer, "GtkTable" );
    GtkObject * myGtkToolbar =
	new GtkObject( myGtkContainer, "GtkToolbar" );
    // GtkObject * myGtkTree =
    // new GtkObject( myGtkContainer, "GtkTree" );
    // GtkObject * myGtkCalendar =
    //  new GtkObject( myGtkWidget, "GtkCalendar" );
    GtkObject * myGtkDrawingArea =
	new GtkObject( myGtkWidget, "GtkDrawingArea");
    // GtkObject * myGtkCurve =
    // new GtkObject( myGtkDrawingArea, "GtkCurve" );
    GtkObject * myGtkEditable =
	new GtkObject( myGtkWidget, "GtkEditable" );
    GtkObject * myGtkEntry =
	new GtkObject( myGtkEditable, "GtkEntry" );
    GtkObject * myGtkSpinButton =
	new GtkObject( myGtkEntry, "GtkSpinButton" );
    GtkObject * myGtkText =
	new GtkObject( myGtkEditable, "GtkText" );
    GtkObject * myGtkRuler =
	new GtkObject( myGtkWidget, "GtkRuler" );
    // GtkObject * myGtkHRuler =
    //  new GtkObject( myGtkRuler, "GtkHRuler" );
    // GtkObject * myGtkVRuler =
    //  new GtkObject( myGtkRuler, "GtkVRuler" );
    GtkObject * myGtkRange =
	new GtkObject( myGtkWidget, "GtkRange" );
    GtkObject * myGtkScale =
	new GtkObject( myGtkRange, "GtkScale" );
    // GtkObject * myGtkHScale =
    //  new GtkObject( myGtkScale, "GtkHScale" );
    // GtkObject * myGtkVScale =
    //  new GtkObject( myGtkScale, "GtkVScale" );
    GtkObject * myGtkScrollbar =
	new GtkObject( myGtkRange, "GtkScrollbar" );
    // GtkObject * myGtkHScrollbar =
    //  new GtkObject( myGtkScrollbar, "GtkHScrollbar" );
    // GtkObject * myGtkVScrollbar =
    //  new GtkObject( myGtkScrollbar, "GtkVScrollbar" );
    GtkObject * myGtkSeparator =
	new GtkObject( myGtkWidget, "GtkSeparator" );
    // GtkObject * myGtkHSeparator =
    //  new GtkObject( myGtkSeparator, "GtkHSeparator" );
    // GtkObject * myGtkVSeparator =
    //  new GtkObject( myGtkSeparator, "GtkVSeparator" );
    // GtkObject * myGtkPreview =
    //  new GtkObject( myGtkWidget, "GtkPreview" );
    GtkObject * myGtkProgress =
	new GtkObject( myGtkWidget, "GtkProgress" );
    GtkObject * myGtkProgressBar =
	new GtkObject( myGtkProgress, "GtkProgressBar" );
    //GtkObject * myGtkData =
    // new GtkObject( myGtkObject, "GtkData" );
    // GtkObject * myGtkAdjustment =
    //	new GtkObject( myGtkData, "GtkAdjustment" );
    // GtkObject * myGtkTooltips =
    //  new GtkObject( myGtkData, "GtkTooltips" );
    // GtkObject * myGtkItemFactory =
    //  new GtkObject( myGtkObject, "GtkItemFactory" );

    // Insert the above Gtk widgets into a dict, using meta data pointers for
    // the different widgets in Qt
    //
    // verify with:
    // --
    // egrep "::staticMetaObject\(\)$" **/*.cpp | fmt -1 | grep :: |
    //    sort | uniq > meta
    //--

    dict.insert(Q3Button::staticMetaObject(), myGtkButton);
    dict.insert(Q3ButtonGroup::staticMetaObject(), myGtkButtonBox);
    dict.insert(Q3Canvas::staticMetaObject(), myGtkDrawingArea);
    dict.insert(Q3CanvasView::staticMetaObject(), myGtkDrawingArea);
    dict.insert(QCheckBox::staticMetaObject(), myGtkCheckButton);
    dict.insert(QColorDialog::staticMetaObject(), myGtkColorSelectionDialog);
    dict.insert(QComboBox::staticMetaObject(), myGtkCombo);
    dict.insert(QDial::staticMetaObject(), myGtkWidget);
    dict.insert(QDialog::staticMetaObject(), myGtkDialog);
    dict.insert(Q3FileDialog::staticMetaObject(), myGtkFileSelection);
    dict.insert(QFontDialog::staticMetaObject(), myGtkFontSelectionDialog);
    dict.insert(QFrame::staticMetaObject(), myGtkFrame);
    dict.insert(Q3Grid::staticMetaObject(), myGtkFrame);
    dict.insert(Q3GroupBox::staticMetaObject(), myGtkBox);
    dict.insert(KHBox::staticMetaObject(), myGtkHBox);
    dict.insert(Q3HButtonGroup::staticMetaObject(), myGtkHButtonBox);
    dict.insert(QHGroupBox::staticMetaObject(), myGtkHBox);
    dict.insert(Q3Header::staticMetaObject(), myGtkRuler);
    dict.insert(Q3IconView::staticMetaObject(), myGtkCTree);
    dict.insert(QInputDialog::staticMetaObject(), myGtkInputDialog);
    dict.insert(QLCDNumber::staticMetaObject(), myGtkFrame);
    dict.insert(QLabel::staticMetaObject(), myGtkLabel);
    dict.insert(QLineEdit::staticMetaObject(), myGtkEntry);
    dict.insert(Q3ListBox::staticMetaObject(), myGtkCList);
    dict.insert(Q3ListView::staticMetaObject(), myGtkCTree);
    dict.insert(Q3MainWindow::staticMetaObject(), myGtkWindow);
    dict.insert(QMenuBar::staticMetaObject(), myGtkMenuBar);
    dict.insert(QMessageBox::staticMetaObject(), myGtkDialog);
    dict.insert(Q3MultiLineEdit::staticMetaObject(), myGtkText);
    dict.insert(Q3PopupMenu::staticMetaObject(), myGtkMenu);
    dict.insert(QPrintDialog::staticMetaObject(), myGtkDialog);
    dict.insert(Q3ProgressBar::staticMetaObject(), myGtkProgressBar);
    dict.insert(Q3ProgressDialog::staticMetaObject(), myGtkDialog);
    dict.insert(QPushButton::staticMetaObject(), myGtkButton);
    dict.insert(QRadioButton::staticMetaObject(), myGtkRadioButton);
    dict.insert(QScrollBar::staticMetaObject(), myGtkScrollbar);
    dict.insert(Q3ScrollView::staticMetaObject(), myGtkViewport);
    dict.insert(QSemiModal::staticMetaObject(), myGtkDialog);
    dict.insert(QSizeGrip::staticMetaObject(), myGtkWidget);
    dict.insert(QSlider::staticMetaObject(), myGtkScale);
    dict.insert(QSpinBox::staticMetaObject(), myGtkSpinButton);
    dict.insert(QSplitter::staticMetaObject(), myGtkPaned);
    dict.insert(QStatusBar::staticMetaObject(), myGtkStatusbar);
    dict.insert(QTabBar::staticMetaObject(), myGtkNotebook);
    dict.insert(Q3TabDialog::staticMetaObject(), myGtkNotebook);
    dict.insert(QTabWidget::staticMetaObject(), myGtkNotebook);
    dict.insert(QTableView::staticMetaObject(), myGtkTable);
    dict.insert(Q3TextBrowser::staticMetaObject(), myGtkText);
    dict.insert(Q3TextView::staticMetaObject(), myGtkText);
    dict.insert(Q3ToolBar::staticMetaObject(), myGtkToolbar);
    dict.insert(QToolButton::staticMetaObject(), myGtkButton);
    dict.insert(KVBox::staticMetaObject(), myGtkVBox);
    dict.insert(Q3VButtonGroup::staticMetaObject(), myGtkVButtonBox);
    dict.insert(QVGroupBox::staticMetaObject(), myGtkVBox);
    dict.insert(QWidget::staticMetaObject(), myGtkWidget);
    dict.insert(Q3WidgetStack::staticMetaObject(), myGtkWidget);
    dict.insert(Q3Wizard::staticMetaObject(), myGtkWindow);
    dict.insert(QWorkspace::staticMetaObject(), myGtkWindow);

    // stuff that we don't have meta data for, but want to know about
    dict.insert(&listboxitem_ptr, myGtkListItem);
    dict.insert(&listviewitem_ptr, myGtkTreeItem);
    dict.insert(&menuitem_ptr, myGtkMenuItem);
    dict.insert(&separator_ptr, myGtkSeparator);
    dict.insert(&arrow_ptr, myGtkArrow);
    dict.insert(&whatsthis_ptr, myGtkTipsQuery);
    dict.insert(&checkmenuitem_ptr, myGtkCheckMenuItem);
    dict.insert(&radiomenuitem_ptr, myGtkRadioMenuItem);
    dict.insert(&eventbox_ptr, myGtkEventBox);

    return myGtkObject;
}


KLegacyImageData *GtkObject::getImageData(KLegacyImageDataKey key, const QString &detail) {
    KLegacyImageData *imagedata = 0;

    if (styleData()) {
	QListIterator<KLegacyImageData> it(styleData()->imageList);

	while ((imagedata = it.current()) != 0) {
	    ++it;

	    if ((((imagedata->key.data.function != 0) &&
		  (imagedata->key.data.function == key.data.function)) ||
		 (imagedata->key.data.function == 0)) &&

		(((imagedata->key.data.state != 0) &&
		  (imagedata->key.data.state == key.data.state)) ||
		 (imagedata->key.data.state == 0)) &&

		(((imagedata->key.data.shadow != 0) &&
		  (imagedata->key.data.shadow == key.data.shadow)) ||
		 (imagedata->key.data.shadow == 0)) &&

		(((imagedata->key.data.orientation != 0) &&
		  (imagedata->key.data.orientation == key.data.orientation)) ||
		 (imagedata->key.data.orientation == 0)) &&

		(((imagedata->key.data.arrowDirection != 0) &&
		  (imagedata->key.data.arrowDirection == key.data.arrowDirection)) ||
		 (imagedata->key.data.arrowDirection == 0)) &&

		(((imagedata->key.data.gapSide != 0) &&
		  (imagedata->key.data.gapSide == key.data.gapSide)) ||
		 (imagedata->key.data.gapSide == 0)) &&

		(((!imagedata->detail.isNull()) &&
		  (detail == imagedata->detail)) ||
		 (imagedata->detail.isNull()))) {
		// we have a winner
		break;
	    }
	}
    }

    if ((! imagedata) && (parent())) {
	imagedata = ((GtkObject *) parent())->getImageData(key, detail);
    }

    return imagedata;
}


KLegacyStyleData *GtkObject::styleData() {
    if ((! d) && parent()) {
	d = ((GtkObject *) parent())->styleData();
    }

    return d;
}


QColor GtkObject::backColor(KLegacy::State s) {
    if ((!  styleData()->back[s].isValid()) && parent()) {
	return ((GtkObject *) parent())->backColor(s);
    }

    if (styleData()->back[s].isValid())
	return  styleData()->back[s];

    return Qt::white;
}


QColor GtkObject::baseColor(KLegacy::State s) {
    if ((! styleData()->base[s].isValid()) && parent()) {
	return ((GtkObject *) parent())->baseColor(s);
    }

    if (styleData()->base[s].isValid())
	return styleData()->base[s];

    return Qt::white;
}


QColor GtkObject::foreColor(KLegacy::State s) {
    if ((! styleData()->fore[s].isValid()) && parent()) {
	return ((GtkObject *) parent())->foreColor(s);
    }

    if (styleData()->fore[s].isValid())
	return styleData()->fore[s];

    return Qt::black;
}


QFont *GtkObject::font() {
    if ((! styleData()->fn) && parent()) {
	return ((GtkObject *) parent())->font();
    }

    return styleData()->fn;
}


GtkObject *GtkObject::find(QRegExp &r) const {
    // if the regular expression matches the name of this widget, return
    if (r.match(name()) != -1) {
	return (GtkObject *) this;
    }

    // regex doesn't match us, and we have no children, return 0
    if (! children()) return 0;

    QObject *o;
    GtkObject *obj, *gobj;

    QObjectListIt ot(*children());

    // search our children to see if any match the regex
    while ((o = ot.current()) != 0) {
	++ot;

	// this would be nice if moc could parse this file :/
	//
	// if (o->className() != "GtkObject") {
	//     qDebug("object is not a GtkObject (className = '%s')",
	// 	      o->className());
	//     continue;
	// }

	obj = (GtkObject *) o;

	// use obj->find(r) instead of r.match(obj->name()) so that this child's
	// children will be searched as well... this allows us to search the entire
	// object tree
	if ((gobj = obj->find(r)) != 0) {
	    // found something!
	    return (GtkObject *) gobj;
	}
    }

    // found nothing
    return 0;
}


QPixmap *GtkObject::draw(KLegacyImageDataKey key, int width, int height,
			 const QString &detail)
{
    KLegacyImageData *imagedata = getImageData(key, detail);
    if (! imagedata) {
	return 0;
    }

    return draw(imagedata, width, height);
}


QPixmap *GtkObject::draw(KLegacyImageData *imagedata, int width, int height) {
    QString pixmapKey;
    QTextOStream(&pixmapKey) << "$KLegacy_Image_" << styleData()->name << "_" <<
	className() << "_" << width << "x" << height << "_" <<
	imagedata->key.cachekey << "_" << (uint) imagedata->recolorable <<
	(uint) imagedata->stretch << (uint) imagedata->overlayStretch;

    QPixmap *pixmap = QPixmapCache::find(pixmapKey);
    if (pixmap) {
	return pixmap;
    }

    QPixmap *main = 0, *overlay = 0;

    if (! imagedata->file.isNull()) {
	QImage *image = imageCache->find(imagedata->file);
	bool found = true;

	if (! image) {
	    image = new QImage(imagedata->file);

	    if (! image || image->isNull()) {
		found = false;
	    } else {
		imageCache->insert(imagedata->file, image);
	    }
	}

	if (found) {
	    int w = ((imagedata->stretch) ? width : image->width()),
		h = ((imagedata->stretch) ? height : image->height());
	    main = drawImage(image, w, h, imagedata->border, imagedata->stretch);
	}
    }

    if (! imagedata->overlayFile.isNull()) {
	QImage *image = imageCache->find(imagedata->overlayFile);
	bool found = true;

	if (! image) {
	    image = new QImage(imagedata->overlayFile);

	    if (! image || image->isNull()) {
		found = false;
	    } else {
		imageCache->insert(imagedata->overlayFile, image);
	    }
	}

	if (found) {
	    int w = ((imagedata->overlayStretch) ? width : image->width()),
		h = ((imagedata->overlayStretch) ? height : image->height());
	    overlay = drawImage(image, w, h, imagedata->overlayBorder,
				imagedata->overlayStretch);
	}
    }

    QSize sz;
    if (main) {
	sz = sz.expandedTo(main->size());
    }

    if (overlay) {
	sz = sz.expandedTo(overlay->size());
    }

    if (sz.isEmpty()) {
	return (QPixmap *) 0;
    }

    pixmap = new QPixmap(sz);
    pixmap->fill(QColor(192,192,176));
    QPainter p(pixmap);

    if (main && (! main->isNull())) {
	p.drawTiledPixmap(0, 0, sz.width(), sz.height(), *main);
    }

    if (overlay && (! overlay->isNull())) {
	QPoint pt((sz.width() - overlay->width()) / 2,
		  (sz.height() - overlay->height()) / 2);
	p.drawPixmap(pt, *overlay);
    }

    p.end();

    if (main) {
	if (main->mask() && (! main->mask()->isNull())) {
	    QBitmap bm(sz);
	    QPainter m(&bm);
	    QRect r(0, 0, width, height);

	    m.drawTiledPixmap(r, *(main->mask()));
	    m.end();

	    pixmap->setMask(bm);
	}
    } else if (overlay) {
	if (overlay->mask() && (! overlay->mask()->isNull())) {
	    QBitmap bm(sz);
	    QPainter m(&bm);
	    QRect r((sz.width() - overlay->width()) / 2,
		    (sz.height() - overlay->height()) / 2,
		    sz.width(), sz.height());
	    m.drawTiledPixmap(r, *(overlay->mask()));
	    m.end();

	    pixmap->setMask(bm);
	}
    }

    if (! QPixmapCache::insert(pixmapKey, pixmap)) {
	delete pixmap;
	pixmap = (QPixmap *) 0;
    }

    return pixmap;
}


class KLegacyStylePrivate : public KLegacy {
private:
    Q3Dict<KLegacyStyleData> styleDict;
    QStringList pixmapPath;
    QTextStream filestream;

    QFont oldfont;
    QPalette oldpalette;

    // pointer to the widget under the pointer
    QPointer<QWidget> lastWidget;

    // current position of the mouse
    QPoint mousePos;
    bool hovering;

    Q3PtrDict<GtkObject> gtkDict;
    GtkObject *gtktree;

    friend class KLegacyStyle;


public:
    KLegacyStylePrivate();
    ~KLegacyStylePrivate();

    // parse the filename passed
    bool parse(const QString &filename);

    bool parseClass();
    bool parseEngine(KLegacyStyleData *);
    bool parseImage(KLegacyStyleData *);
    bool parsePixmapPath();
    bool parseStyle();
};


KLegacyStylePrivate::KLegacyStylePrivate()
    : lastWidget(0), mousePos(-1, -1), hovering(false), gtktree(0)
{
    QPixmapCache::setCacheLimit(8192);

    if (! imageCache) {
	imageCache = new Q3Dict<QImage>(imageCacheSize);
	CHECK_PTR(imageCache);

	imageCache->setAutoDelete(true);
    }

    styleDict.setAutoDelete(true);

    gtktree = initialize(gtkDict);
    CHECK_PTR(gtktree);

    if (! gtktree->d) {
	gtktree->d = new KLegacyStyleData;
 	gtktree->d->name = "Default";
    }

    // get the path to this users .gtkrc
    QString gtkrcFilename = getenv("HOME");
    gtkrcFilename += "/.gtkrc";

    QFile gtkrc(gtkrcFilename);

    if (gtkrc.open(QIODevice::ReadOnly)) {
	filestream.setDevice(&gtkrc);

	while (! filestream.atEnd()) {
	    QString next;
	    filestream >> next;

	    if (next.isNull()) continue;

	    // skip comments
	    if (next[0] == '#') { filestream.readLine(); continue; }

	    if (next == "class" || next == "widget" || next == "widget_class") {
		if (! parseClass())
		    qWarning("\"class\" parse error");
	    } else if (next == "pixmap_path") {
		if (! parsePixmapPath())
		    qWarning("\"pixmap_path\" parse error");
	    } else if (next == "style") {
		if (! parseStyle())
		    qWarning("\"style\" parse error");
	    }
	}

	gtkrc.close();
    } else
	qWarning("%s: failed to open", gtkrcFilename.latin1());
}


KLegacyStylePrivate::~KLegacyStylePrivate() {
    if (imageCache) {
	delete imageCache;
	imageCache = 0;
    }

    if (gtktree) {
	delete gtktree;
	gtktree = 0;
    }
}


bool KLegacyStylePrivate::parseClass() {
    if (filestream.atEnd()) return false;

    QString classname, keyword, stylename;
    filestream >> classname;
    filestream >> keyword;
    filestream >> stylename;

    if (classname.isNull() || keyword.isNull() || stylename.isNull() ||
	keyword != "style" ||
	classname[0] != '\"' || classname[classname.length() - 1] != '\"' ||
	stylename[0] != '\"' || stylename[stylename.length() - 1] != '\"')
	return false;

    classname = classname.mid(1, classname.length() - 2);
    stylename = stylename.mid(1, stylename.length() - 2);

    QRegExp r(classname);
    r.setWildcard(true);
    GtkObject *obj = gtktree->find(r);

    if (! obj) {
	qWarning("unknown object '%s'", classname.latin1());
	return false;
    }

    KLegacyStyleData *styledata = styleDict.find(stylename);

    if (! styledata) {
	qWarning("no such style '%s' for class '%s' (%p)", stylename.latin1(),
		 classname.latin1(), styledata);
	return false;
    }

    obj->d = styledata;
    styledata->ref++;

    return true;
}


bool KLegacyStylePrivate::parseImage(KLegacyStyleData *styledata) {
    if (filestream.atEnd()) {
	qWarning("parseImage: premature end of stream");
	return false;
    }

    QString next, equals, parameter;
    filestream >> next;

    // skip comments
    while (next[0] == '#') {
	filestream.readLine();
	filestream >> next;
    }

    if (next.isNull() || next != "{") {
	qWarning("parseImage: expected '{' after 'image'\n"
		 "  in style '%s', after processing %d previous images\n",
		 styledata->name.latin1(), styledata->imageList.count());
	return false;
    }

    KLegacyImageData *imagedata = new KLegacyImageData;

    int paren_count = 1;
    while (paren_count) {
	filestream >> next;
	if (next.isNull()) continue;

	// skip comments
	if (next[0] == '#') {filestream.readLine(); continue; }

	if (next == "arrow_direction") {
	    filestream >> equals;
	    filestream >> parameter;

	    if (equals.isNull() || parameter.isNull() || equals != "=") continue;

	    if (parameter == "UP")
		imagedata->key.data.arrowDirection = Qt::UpArrow + 1;
	    else if (parameter == "DOWN")
		imagedata->key.data.arrowDirection = Qt::DownArrow + 1;
	    else if (parameter == "LEFT")
		imagedata->key.data.arrowDirection = Qt::LeftArrow + 1;
	    else if (parameter == "RIGHT")
		imagedata->key.data.arrowDirection = Qt::RightArrow + 1;
	} else if (next == "border") {
	    filestream >> equals;
	    filestream >> parameter;

	    if (equals.isNull() || parameter.isNull() || equals != "=" ||
		parameter != "{")
		continue;
	    QString border =filestream.readLine();

	    int lp, rp, tp, bp, l, r, t, b;
	    lp = border.find(',');
	    rp = border.find(',', lp + 1);
	    tp = border.find(',', rp + 1);
	    bp = border.find('}', tp + 1);

	    l = border.left(lp).toUInt();
	    r = border.mid(lp + 1, rp - lp - 1).toUInt();
	    t = border.mid(rp + 1, tp - rp - 1).toUInt();
	    b = border.mid(tp + 1, bp - tp - 1).toUInt();

	    imagedata->border.setLeft(l);
	    imagedata->border.setRight(r);
	    imagedata->border.setTop(t);
	    imagedata->border.setBottom(b);
	} else if (next == "detail") {
	    filestream >> equals;
	    filestream >> parameter;

	    if (equals.isNull() || parameter.isNull() || equals != "=" ||
		parameter[0] != '\"' || parameter[parameter.length() - 1] != '\"')
		continue;

	    parameter = parameter.mid(1, parameter.length() - 2);
	    imagedata->detail = parameter;
	} else if (next == "file") {
	    filestream >> equals;
	    filestream >> parameter;

	    if (equals.isNull() || parameter.isNull() || equals != "=" ||
		parameter[0] != '\"' || parameter[parameter.length() - 1] != '\"') {
		qWarning("image: file parameter malformed");
		continue;
	    }

	    parameter = parameter.mid(1, parameter.length() - 2);

	    QStringList::Iterator it;
	    for (it = pixmapPath.begin(); it != pixmapPath.end(); ++it) {
		QFileInfo fileinfo((*it) + parameter);

		if (fileinfo.exists()) {
		    imagedata->file = fileinfo.filePath();
		    break;
		}
	    }
	} else if (next == "function") {
	    filestream >> equals;
	    filestream >> parameter;

	    if (equals.isNull() || parameter.isNull() || equals != "=") continue;

	    if (parameter == "BOX")
		imagedata->key.data.function = Box;
	    else if (parameter == "FLAT_BOX")
		imagedata->key.data.function = FlatBox;
	    else if (parameter == "EXTENSION")
		imagedata->key.data.function = Extension;
	    else if (parameter == "CHECK")
		imagedata->key.data.function = Check;
	    else if (parameter == "OPTION")
		imagedata->key.data.function = Option;
	    else if (parameter == "HLINE")
		imagedata->key.data.function = HLine;
	    else if (parameter == "VLINE")
		imagedata->key.data.function = VLine;
	    else if (parameter == "BOX_GAP")
		imagedata->key.data.function = BoxGap;
	    else if (parameter == "SLIDER")
		imagedata->key.data.function = Slider;
	    else if (parameter == "TAB")
		imagedata->key.data.function = Tab;
	    else if (parameter == "ARROW")
		imagedata->key.data.function = Arrow;
	    else if (parameter == "HANDLE")
		imagedata->key.data.function = Handle;
	    else if (parameter == "SHADOW")
		imagedata->key.data.function = FShadow;
	    else if (parameter == "FOCUS")
		imagedata->key.data.function = Focus;
	} else if (next == "gap_side" ) {
	    filestream >> equals;
	    filestream >> parameter;

	    if (equals.isNull() || parameter.isNull() || equals != "=") continue;

	    if (parameter == "TOP")
		imagedata->key.data.gapSide = KLegacy::Top;
	    else if (parameter == "BOTTOM")
		imagedata->key.data.gapSide = KLegacy::Bottom;
	} else if (next == "orientation") {
	    filestream >> equals;
	    filestream >> parameter;

	    if (equals.isNull() || parameter.isNull() || equals != "=") continue;

	    if (parameter == "VERTICAL")
		imagedata->key.data.orientation = Qt::Vertical + 1;
	    else if (parameter == "HORIZONTAL")
		imagedata->key.data.orientation = Qt::Horizontal + 1;
	} else if (next == "overlay_border") {
	    filestream >> equals;
	    filestream >> parameter;

	    if (equals.isNull() || parameter.isNull() || equals != "=" ||
		parameter != "{")
		continue;
	    QString border = filestream.readLine();

	    int lp, rp, tp, bp, l, r, t, b;
	    lp = border.find(',');
	    rp = border.find(',', lp + 1);
	    tp = border.find(',', rp + 1);
	    bp = border.find('}', tp + 1);

	    l = border.left(lp).toUInt();
	    r = border.mid(lp + 1, rp - lp - 1).toUInt();
	    t = border.mid(rp + 1, tp - rp - 1).toUInt();
	    b = border.mid(tp + 1, bp - tp - 1).toUInt();

	    imagedata->overlayBorder.setLeft(l);
	    imagedata->overlayBorder.setRight(r);
	    imagedata->overlayBorder.setTop(t);
	    imagedata->overlayBorder.setBottom(b);
	} else if (next == "overlay_file") {
	    filestream >> equals;
	    filestream >> parameter;

	    if (equals.isNull() || parameter.isNull() || equals != "=" ||
		parameter[0] != '\"' || parameter[parameter.length() - 1] != '\"') {
		qWarning("image: overlay_file parameter malformed");
		continue;
	    }

	    parameter = parameter.mid(1, parameter.length() - 2);

	    QStringList::Iterator it;
	    for (it = pixmapPath.begin(); it != pixmapPath.end(); ++it) {
		QFileInfo fileinfo((*it) + parameter);

		if (fileinfo.exists()) {
		    imagedata->overlayFile = fileinfo.filePath();
		    break;
		}
	    }
	} else if (next == "overlay_stretch") {
	    filestream >> equals;
	    filestream >> parameter;

	    if (equals.isNull() || parameter.isNull() || equals != "=") continue;

	    if (parameter == "TRUE")
		imagedata->overlayStretch = true;
	    else
		imagedata->overlayStretch = false;
	} else if (next == "stretch") {
	    filestream >> equals;
	    filestream >> parameter;

	    if (equals.isNull() || parameter.isNull() || equals != "=") continue;

	    if (parameter == "TRUE")
		imagedata->stretch = true;
	    else
		imagedata->stretch = false;
	} else if (next == "shadow") {
	    filestream >> equals;
	    filestream >> parameter;

	    if (equals.isNull() || parameter.isNull() || equals != "=") continue;

	    if (parameter == "NONE")
		imagedata->key.data.shadow = NoShadow;
	    else if (parameter == "IN")
		imagedata->key.data.shadow = In;
	    else if (parameter == "OUT")
		imagedata->key.data.shadow = Out;
	    else if (parameter == "ETCHED_IN")
		imagedata->key.data.shadow = EtchedIn;
	    else if (parameter == "ETCHED_OUT")
		imagedata->key.data.shadow = EtchedOut;
	} else if (next == "state") {
	    filestream >> equals;
	    filestream >> parameter;

	    if (equals.isNull() || parameter.isNull() || equals != "=") continue;

	    if (parameter == "NORMAL")
		imagedata->key.data.state = Normal;
	    else if (parameter == "PRELIGHT")
		imagedata->key.data.state = Prelight;
	    else if (parameter == "ACTIVE")
		imagedata->key.data.state = Active;
	    else if (parameter == "INSENSITIVE")
		imagedata->key.data.state = Insensitive;
	    else if (parameter == "SELECTED")
		imagedata->key.data.state = Selected;
	} else if (next == "{") paren_count++;
	else if (next == "}") paren_count--;
    }

    styledata->imageList.append(imagedata);

    return true;
}


bool KLegacyStylePrivate::parseEngine(KLegacyStyleData *styledata) {
    if (filestream.atEnd()) return false;

    QString enginename, paren;
    filestream >> enginename;
    filestream >> paren;

    if (enginename.isNull() || paren.isNull() ||
	enginename[0] != '\"' || enginename[enginename.length() - 1] != '\"' ||
	paren != "{") {
	return false;
    }

    QString next;
    int paren_count = 1;
    while (paren_count) {
	filestream >> next;

	// skip comments
	if (next[0] == '#') {
	    filestream.readLine();
	    continue;
	}

	if (next == "image") {
	    if (! parseImage(styledata)) {
		qWarning("image parse error");
	    }
	} else if (next == "{") {
	    paren_count++;
	} else if (next == "}") {
	    paren_count--;
	}
    }

    return true;
}


bool KLegacyStylePrivate::parsePixmapPath() {
    if (filestream.atEnd()) {
	return false;
    }

    QString next;
    filestream >> next;

    if (next.isNull() || next[0] != '\"' || next[next.length() - 1] != '\"') {
	return false;
    }

    next = next.mid(1, next.length() - 2);

    int start = 0, end = next.find(":");
    while (end != -1) {
	QString path(next.mid(start, end - start));

	if (path[path.length() - 1] != '/') {
	    path += '/';
	}

	QFileInfo fileinfo(path);
	if (fileinfo.exists() && fileinfo.isDir()) {
	    pixmapPath.append(path);
	}

	start = end + 1;
	end = next.find(":", start);
    }

    // get the straggler
    end = next.length();
    QString path(next.mid(start, end - start));

    if (path[path.length() - 1] != '/') {
	path += '/';
    }

    QFileInfo fileinfo(path);
    if (fileinfo.exists() && fileinfo.isDir()) {
	pixmapPath.append(path);
    }

    return true;
}


bool KLegacyStylePrivate::parseStyle() {
    if (filestream.atEnd()) return false;

    QString stylename, paren;
    filestream >> stylename;
    filestream >> paren;

    if (stylename.isNull() || paren.isNull() ||
	stylename[0] != '\"' || stylename[stylename.length() - 1] != '\"')
	return false;

    stylename = stylename.mid(1, stylename.length() - 2);

    if (paren == "=") {
	QString newstylename;
	filestream >> newstylename;

	if (newstylename.isNull() ||
	    newstylename[0] != '\"' || newstylename[newstylename.length() - 1] != '\"')
	    return false;

	newstylename = newstylename.mid(1, newstylename.length() - 2);

	KLegacyStyleData *styledata = styleDict.find(stylename);

	if (! styledata) return false;

	KLegacyStyleData *newstyledata = new KLegacyStyleData(*styledata);
	newstyledata->name = newstylename;
	styleDict.insert(newstylename, newstyledata);

	return true;
    } else if (paren != "{") {
	qWarning("parseStyle: expected '{' while parsing style %s",
		 stylename.latin1());
	return false;
    }

    KLegacyStyleData *styledata = new KLegacyStyleData;
    styledata->name = stylename;

    QString next, parameter;
    int paren_count = 1;
    while (paren_count) {
	filestream >> next;

	// skip comments
	if (next[0] == '#') {
	    filestream.readLine();
	    continue;
	}

	if (next.left(5) == "base[") {
	    int l = next.find('['), r = next.find(']'), state;

	    if (l < 1 || r < 1 || r < l) continue;

	    QString mode = next.mid(l + 1, r - l - 1);
	    if (mode == "ACTIVE")
		state = Active;
	    else if (mode == "NORMAL")
		state = Normal;
	    else if (mode == "INSENSITIVE")
		state = Insensitive;
	    else if (mode == "PRELIGHT")
		state = Prelight;
	    else if (mode == "SELECTED")
		state = Selected;

	    filestream >> next;
	    filestream >> parameter;

	    if (next.isNull() || parameter.isNull() || next != "=") continue;

	    if (parameter[0] == '\"') { // assume color of the form "#rrggbb"
		QString colorname = parameter.mid(1, parameter.length() - 2);
		if (colorname.isNull()) continue;

		styledata->base[state].setNamedColor(colorname);
	    } else if (parameter == "{") { // assume color of the form  { ri, gi, bi }
		QString color =filestream.readLine();

		int rp, gp, bp;
		float ri, gi, bi;

		rp = color.find(',');
		gp = color.find(',', rp + 1);
		bp = color.find('}', gp + 1);

		ri = color.left(rp).toFloat();
		gi = color.mid(rp + 1, gp - rp - 1).toFloat();
		bi = color.mid(gp + 1, bp - gp - 1).toFloat();

		int Qt::red   = (int) (255 * ri);
		int Qt::green = (int) (255 * gi);
		int Qt::blue  = (int) (255 * bi);
		styledata->base[state].setRgb(Qt::red, Qt::green, Qt::blue);
	    }
	} else if (next.left(3) == "bg[") {
	    int l = next.find('['), r = next.find(']'), state;

	    if (l < 1 || r < 1 || r < l) continue;

	    QString mode = next.mid(l + 1, r - l - 1);
	    if (mode == "ACTIVE")
		state = Active;
	    else if (mode == "NORMAL")
		state = Normal;
	    else if (mode == "INSENSITIVE")
		state = Insensitive;
	    else if (mode == "PRELIGHT")
		state = Prelight;
	    else if (mode == "SELECTED")
		state = Selected;

	    filestream >> next;
	    filestream >> parameter;

	    if (next.isNull() || parameter.isNull() || next != "=") continue;

	    if (parameter[0] == '\"') { // assume color of the form "#rrggbb"
		QString colorname = parameter.mid(1, parameter.length() - 2);
		if (colorname.isNull()) continue;

		styledata->back[state].setNamedColor(colorname);
	    } else if (parameter == "{") { // assume color of the form  { ri, gi, bi }
		QString color =filestream.readLine();

		int rp, gp, bp;
		float ri, gi, bi;

		rp = color.find(',');
		gp = color.find(',', rp + 1);
		bp = color.find('}', gp + 1);

		ri = color.left(rp).toFloat();
		gi = color.mid(rp + 1, gp - rp - 1).toFloat();
		bi = color.mid(gp + 1, bp - gp - 1).toFloat();

		int Qt::red   = (int) (255 * ri);
		int Qt::green = (int) (255 * gi);
		int Qt::blue  = (int) (255 * bi);
		styledata->back[state].setRgb(Qt::red, Qt::green, Qt::blue);
	    }
	} else if (next == "engine") {
	    if (! parseEngine(styledata))
		fprintf(stderr, "engine parse error\n");
	} else if (next.left(3) == "fg[") {
	    int l = next.find('['), r = next.find(']'), state;

	    if (l < 1 || r < 1 || r < l) continue;

	    QString mode = next.mid(l + 1, r - l - 1);
	    if (mode == "ACTIVE")
		state = Active;
	    else if (mode == "NORMAL")
		state = Normal;
	    else if (mode == "INSENSITIVE")
		state = Insensitive;
	    else if (mode == "PRELIGHT")
		state = Prelight;
	    else if (mode == "SELECTED")
		state = Selected;

	    filestream >> next;
	    filestream >> parameter;

	    if (next.isNull() || parameter.isNull() || next != "=") continue;

	    if (parameter[0] == '\"') { // assume color of the form "#rrggbb"
		QString colorname = parameter.mid(1, parameter.length() - 2);
		if (colorname.isNull()) continue;

		styledata->fore[state].setNamedColor(colorname);
	    } else if (parameter == "{") { // assume color of the form  { ri, gi, bi }
		QString color = filestream.readLine();

		int rp, gp, bp;
		float ri, gi, bi;

		rp = color.find(',');
		gp = color.find(',', rp + 1);
		bp = color.find('}', gp + 1);

		ri = color.left(rp).toFloat();
		gi = color.mid(rp + 1, gp - rp - 1).toFloat();
		bi = color.mid(gp + 1, bp - gp - 1).toFloat();

		int Qt::red   = (int) (255 * ri);
		int Qt::green = (int) (255 * gi);
		int Qt::blue  = (int) (255 * bi);
		styledata->fore[state].setRgb(Qt::red, Qt::green, Qt::blue);
	    }
	} else if (next == "font") {
	    filestream >> next;
	    filestream >> parameter;

	    if (next.isNull() || parameter.isNull() || next != "=" ||
		parameter[0] != '\"' || parameter[parameter.length() - 1] != '\"') {
		qWarning("font parameter malformed '%s'", parameter.latin1());
		continue;
	    }

	    parameter = parameter.mid(1, parameter.length() - 2);

	    if (! styledata->fn) {
		styledata->fn = new QFont;
	    }

	    styledata->fn->setRawName(parameter);
	} else if (next == "{") {
	    paren_count++;
	} else if (next == "}") {
	    paren_count--;
	}
    }

    styleDict.insert(styledata->name, styledata);

    return true;
}


KLegacyStyle::KLegacyStyle(void) : KStyle() {
    setButtonDefaultIndicatorWidth(6);
    setScrollBarExtent(15, 15);
    setButtonMargin(3);
    setSliderThickness(15);

    priv = new KLegacyStylePrivate;
}


KLegacyStyle::~KLegacyStyle(void) {
    delete priv;
}


int KLegacyStyle::defaultFrameWidth() const {
    return 2;
}


void KLegacyStyle::polish(QApplication *app) {
    priv->oldfont = app->font();
    priv->oldpalette = app->palette();

    GtkObject *gobj = priv->gtkDict.find(Q3MainWindow::staticMetaObject());

    if (gobj) {
	if (gobj->font()) {
	    app->setFont(*gobj->font(), true);
	}

	QPalette pal = app->palette();
	QBrush brush;

	// background
	brush = pal.brush(QPalette::Active, QColorGroup::Background);
	brush.setColor(gobj->backColor(KLegacy::Active));
	pal.setBrush(QPalette::Active, QColorGroup::Background, brush);

	brush = pal.brush(QPalette::Inactive, QColorGroup::Background);
	brush.setColor(gobj->backColor(KLegacy::Normal));
	pal.setBrush(QPalette::Inactive, QColorGroup::Background, brush);

	brush = pal.brush(QPalette::Disabled, QColorGroup::Background);
	brush.setColor(gobj->backColor(KLegacy::Insensitive));
	pal.setBrush(QPalette::Disabled, QColorGroup::Background, brush);

	// foreground
	brush = pal.brush(QPalette::Active, QColorGroup::Foreground);
	brush.setColor(gobj->foreColor(KLegacy::Active));
	pal.setBrush(QPalette::Active, QColorGroup::Foreground, brush);

	brush = pal.brush(QPalette::Inactive, QColorGroup::Foreground);
	brush.setColor(gobj->foreColor(KLegacy::Normal));
	pal.setBrush(QPalette::Inactive, QColorGroup::Foreground, brush);

	brush = pal.brush(QPalette::Disabled, QColorGroup::Foreground);
	brush.setColor(gobj->foreColor(KLegacy::Insensitive));
	pal.setBrush(QPalette::Disabled, QColorGroup::Foreground, brush);

	// base
	brush = pal.brush(QPalette::Active, QColorGroup::Base);
	brush.setColor(gobj->baseColor(KLegacy::Normal));
	pal.setBrush(QPalette::Active, QColorGroup::Base, brush);

	brush = pal.brush(QPalette::Inactive, QColorGroup::Base);
	brush.setColor(gobj->baseColor(KLegacy::Normal));
	pal.setBrush(QPalette::Inactive, QColorGroup::Base, brush);

	brush = pal.brush(QPalette::Disabled, QColorGroup::Base);
	brush.setColor(gobj->baseColor(KLegacy::Normal));
	pal.setBrush(QPalette::Disabled, QColorGroup::Base, brush);

	// button
	brush = pal.brush(QPalette::Active, QColorGroup::Button);
	brush.setColor(gobj->backColor(KLegacy::Active));
	pal.setBrush(QPalette::Active, QColorGroup::Button, brush);

	brush = pal.brush(QPalette::Normal, QColorGroup::Button);
	brush.setColor(gobj->backColor(KLegacy::Normal));
	pal.setBrush(QPalette::Normal, QColorGroup::Button, brush);

	brush = pal.brush(QPalette::Disabled, QColorGroup::Button);
	brush.setColor(gobj->backColor(KLegacy::Insensitive));
	pal.setBrush(QPalette::Disabled, QColorGroup::Button, brush);

	// text
	brush = pal.brush(QPalette::Active, QColorGroup::Text);
	brush.setColor(gobj->foreColor(KLegacy::Active));
	pal.setBrush(QPalette::Active, QColorGroup::Text, brush);

	brush = pal.brush(QPalette::Inactive, QColorGroup::Text);
	brush.setColor(gobj->foreColor(KLegacy::Normal));
	pal.setBrush(QPalette::Inactive, QColorGroup::Text, brush);

	brush = pal.brush(QPalette::Disabled, QColorGroup::Text);
	brush.setColor(gobj->foreColor(KLegacy::Insensitive));
	pal.setBrush(QPalette::Disabled, QColorGroup::Text, brush);

	// highlight
	brush = pal.brush(QPalette::Active, QColorGroup::Highlight);
	brush.setColor(gobj->backColor(KLegacy::Selected));
	pal.setBrush(QPalette::Active, QColorGroup::Highlight, brush);

	brush = pal.brush(QPalette::Active, QColorGroup::Highlight);
	brush.setColor(gobj->backColor(KLegacy::Selected));
	pal.setBrush(QPalette::Active, QColorGroup::Highlight, brush);

	brush = pal.brush(QPalette::Active, QColorGroup::Highlight);
	brush.setColor(gobj->backColor(KLegacy::Selected));
	pal.setBrush(QPalette::Active, QColorGroup::Highlight, brush);

	// highlight text
	brush = pal.brush(QPalette::Active, QColorGroup::HighlightedText);
	brush.setColor(gobj->foreColor(KLegacy::Selected));
	pal.setBrush(QPalette::Active, QColorGroup::HighlightedText, brush);

	brush = pal.brush(QPalette::Active, QColorGroup::HighlightedText);
	brush.setColor(gobj->foreColor(KLegacy::Active));
	pal.setBrush(QPalette::Active, QColorGroup::HighlightedText, brush);

	brush = pal.brush(QPalette::Active, QColorGroup::HighlightedText);
	brush.setColor(gobj->foreColor(KLegacy::Active));
	pal.setBrush(QPalette::Active, QColorGroup::HighlightedText, brush);

	app->setPalette(pal, true);
    }

    qt_set_draw_menu_bar_impl((QDrawMenuBarItemImpl) &KLegacyStyle::drawMenuBarItem);

    KStyle::polish(app);
}


void KLegacyStyle::polish(QWidget *widget) {
    if (qstrcmp(widget->name(), "qt_viewport") == 0 ||
	widget->testWFlags(Qt::WType_Popup) ||
	widget->inherits("KDesktop"))
	return;

    if (widget->backgroundMode() == Qt::PaletteBackground ||
	widget->backgroundMode() == Qt::PaletteButton &&
	(! widget->ownPalette()))
	widget->setBackgroundMode(Qt::X11ParentRelative);

    QMetaObject *metaobject = 0;
    QString detail;
    KLegacyImageDataKey key;
    key.cachekey = 0;

    bool eventFilter = false;
    bool mouseTrack = false;
    bool immediateRender = false;
    bool bgPixmap = false;

    if (widget->inherits("QButton")) {
	metaobject = Q3Button::staticMetaObject();
	eventFilter = true;
    }

    if (widget->inherits("QComboBox")) {
	metaobject = QComboBox::staticMetaObject();
	eventFilter = true;
    }

    if (widget->inherits("QScrollBar")) {
	metaobject = QScrollBar::staticMetaObject();
	eventFilter = true;
	mouseTrack = true;
    }

    if (widget->inherits("QMenuBar")) {
	eventFilter = true;
	immediateRender = true;

	metaobject = QMenuBar::staticMetaObject();

	detail = "menubar";
	key.data.function = KLegacy::Box;
	key.data.shadow = KLegacy::Out;
	key.data.state = KLegacy::Normal;

	((QMenuBar *) widget)->setFrameShape(QFrame::StyledPanel);
	((QMenuBar *) widget)->setLineWidth(0);
	widget->setBackgroundMode(Qt::PaletteBackground);
    }

    if (widget->inherits("QToolBar")) {
	metaobject = Q3ToolBar::staticMetaObject();

	eventFilter = true;
	immediateRender = true;

	detail = "menubar";
	key.data.function = KLegacy::Box;
	key.data.shadow = KLegacy::Out;
	key.data.state = KLegacy::Normal;

	widget->setBackgroundMode(Qt::PaletteBackground);
    }

    if (widget->inherits("QLineEdit")) {
	metaobject = QLineEdit::staticMetaObject();

	eventFilter = true;
	immediateRender = true;

	detail = "entry_bg";
	key.data.function = KLegacy::FlatBox;
	key.data.shadow = KLegacy::NoShadow;
	key.data.state = (widget->isEnabled()) ? KLegacy::Normal : KLegacy::Insensitive;

	widget->setBackgroundMode(Qt::PaletteBase);
    }

    if (widget->isTopLevel() || widget->inherits("QWorkspaceChild")) {
	immediateRender = true;

	bgPixmap = true;
	metaobject = Q3MainWindow::staticMetaObject();
	key.cachekey = 0;
	key.data.function = KLegacy::FlatBox;
	detail = "base";
    }

    if (widget->inherits("QPopupMenu")) {
	qDebug("polishing popup '%s'", widget->className());
	metaobject = Q3PopupMenu::staticMetaObject();
	widget->setBackgroundMode(Qt::PaletteBackground);
    }

    GtkObject *gobj = gobj = priv->gtkDict.find(((metaobject) ? metaobject :
						 widget->metaObject()));

    if (gobj) {
	if (gobj->font() && (*gobj->font() != QApplication::font()))
	    widget->setFont(*gobj->font());

	if (immediateRender) {
	    QPixmap *pix = gobj->draw(key, widget->width(), widget->height(), detail);

	    if (pix && ! pix->isNull()) {
		if (! bgPixmap) {
		    QPalette pal = widget->palette();
		    QBrush brush;

		    // base
		    // active
		    brush = pal.brush(QPalette::Active,
				      QColorGroup::Base);
		    brush.setPixmap(*pix);
		    pal.setBrush(QPalette::Active,
				 QColorGroup::Base, brush);

		    // inactive
		    brush = pal.brush(QPalette::Inactive,
				      QColorGroup::Base);
		    brush.setPixmap(*pix);
		    pal.setBrush(QPalette::Inactive,
				 QColorGroup::Base, brush);

		    // disabled
		    brush = pal.brush(QPalette::Disabled,
				      QColorGroup::Base);
		    brush.setPixmap(*pix);
		    pal.setBrush(QPalette::Disabled,
				 QColorGroup::Base, brush);

		    // background - button
		    // active
		    brush = pal.brush(QPalette::Active,
				      QColorGroup::Background);
		    brush.setPixmap(*pix);
		    pal.setBrush(QPalette::Active,
				 QColorGroup::Background, brush);

		    brush = pal.brush(QPalette::Active,
				      QColorGroup::Button);
		    brush.setPixmap(*pix);
		    pal.setBrush(QPalette::Active,
				 QColorGroup::Button, brush);

		    // inactive
		    brush = pal.brush(QPalette::Inactive,
				      QColorGroup::Background);
		    brush.setPixmap(*pix);
		    pal.setBrush(QPalette::Inactive,
				 QColorGroup::Background, brush);

		    brush = pal.brush(QPalette::Inactive,
				      QColorGroup::Button);
		    brush.setPixmap(*pix);
		    pal.setBrush(QPalette::Inactive,
				 QColorGroup::Button, brush);

		    // disabled
		    brush = pal.brush(QPalette::Disabled,
				      QColorGroup::Background);
		    brush.setPixmap(*pix);
		    pal.setBrush(QPalette::Disabled,
				 QColorGroup::Background, brush);

		    brush = pal.brush(QPalette::Disabled,
				      QColorGroup::Button);
		    brush.setPixmap(*pix);
		    pal.setBrush(QPalette::Disabled,
				 QColorGroup::Button, brush);

		    widget->setPalette(pal);
		} else
		    widget->setBackgroundPixmap(*pix);
	    }
	}
    }

    if (eventFilter) {
	widget->installEventFilter(this);
    }

    if (mouseTrack) {
 	widget->setMouseTracking(mouseTrack);
    }

    KStyle::polish(widget);
}


void KLegacyStyle::polishPopupMenu(Q3PopupMenu *popup) {
    KStyle::polishPopupMenu(popup);

    popup->setMouseTracking(true);
    popup->setCheckable(true);

    popup->installEventFilter(this);
}


void KLegacyStyle::unPolish(QWidget *widget) {
    if (widget->inherits("KDesktop"))
	return;

    widget->setBackgroundOrigin(QWidget::WidgetOrigin);
    widget->setBackgroundPixmap(QPixmap());
    widget->removeEventFilter(this);
    widget->unsetPalette();
    widget->setAutoMask(false);
    KStyle::unPolish(widget);
}


void KLegacyStyle::unPolish(QApplication *app) {
    app->setFont(priv->oldfont, true);
    app->setPalette(priv->oldpalette, true);

    qt_set_draw_menu_bar_impl(0);

    KStyle::unPolish(app);
}


void KLegacyStyle::drawKMenuItem(QPainter *p, int x, int y, int w, int h, const QColorGroup &g,
				 bool active, QMenuItem *mi, QBrush *)
{
    drawMenuBarItem(p, x, y, w, h, mi, (QColorGroup &) g,
		    (mi) ? mi->isEnabled() : false, active);
}


void KLegacyStyle::drawMenuBarItem(QPainter *p, int x, int y, int w, int h, QMenuItem *mi,
				   QColorGroup &g, bool enabled, bool active)
{
    if (enabled && active) {
	GtkObject *gobj = priv->gtkDict.find(&menuitem_ptr);

	if (gobj) {
	    KLegacyImageDataKey key;
	    key.cachekey = 0;
	    key.data.function = KLegacy::Box;
	    key.data.state = KLegacy::Prelight;
	    key.data.shadow = KLegacy::Out;

	    QPixmap *pix = gobj->draw(key, w, h, "menuitem");

	    if (pix && ! pix->isNull())
		p->drawPixmap(x, y, *pix);
	}
    }

    drawItem(p, x, y, w, h, Qt::AlignCenter|Qt::TextShowMnemonic|Qt::TextDontClip|Qt::TextSingleLine,
	     g, enabled, mi->pixmap(), mi->text(), -1, &g.buttonText());
}


void KLegacyStyle::drawButton(QPainter *p, int x, int y , int w, int h,
			      const QColorGroup &g, bool sunken, const QBrush *fill)
{
    drawBevelButton(p, x, y, w, h, g, sunken, fill);
}


void KLegacyStyle::drawBevelButton(QPainter *p, int x, int y, int w, int h,
				const QColorGroup & g, bool sunken,
				const QBrush *fill)
{
    GtkObject *gobj = priv->gtkDict.find(Q3Button::staticMetaObject());

    if (! gobj) {
	KStyle::drawBevelButton(p, x, y, w, h, g, sunken, fill);
	return;
    }

    KLegacyImageDataKey key;
    key.cachekey = 0;
    key.data.function = KLegacy::Box;
    key.data.shadow = (sunken) ? KLegacy::In : KLegacy::Out;
    key.data.state = (sunken) ? KLegacy::Active : KLegacy::Normal;

    QPixmap *pix = gobj->draw(key, w, h, "button");

    if (pix && (! pix->isNull()))
	p->drawPixmap(x, y, *pix);
    else
	KStyle::drawBevelButton(p, x, y, w, h, g, sunken, fill);
}


void KLegacyStyle::drawPushButton(QPushButton *btn, QPainter *p) {
    GtkObject *gobj = priv->gtkDict.find(QPushButton::staticMetaObject());

    if (! gobj) {
	KStyle::drawPushButton(btn, p);
	return;
    }

    KLegacyImageDataKey key;
    key.cachekey = 0;
    key.data.function = KLegacy::Box;

    QColorGroup g = btn->colorGroup();
    QBrush fill = g.brush(QColorGroup::Button);
    int x1, y1, x2, y2;
    btn->rect().coords(&x1, &y1, &x2, &y2);

    if (btn->isDefault()) {
	// draw default button
	key.data.state = (btn->isEnabled()) ? KLegacy::Normal : KLegacy::Insensitive;
	key.data.shadow = KLegacy::In;

	QPixmap *pix = gobj->draw(key, x2 -x1 + 1, y2 - y1 + 1, "buttondefault");

	if (! pix)
	    pix = gobj->draw(key, x2 - x1 + 1, y2 - y1 + 1, "button");

	if (pix)
	    p->drawPixmap(x1, y1, *pix);
        else
	    KStyle::drawBevelButton(p, x1, y1, x2 - x1 + 1, y2 - y1 + 1,
				    g, true, &fill);
    }

    int diw = buttonDefaultIndicatorWidth();
    if (btn->isDefault() || btn->autoDefault()) {
	x1 += diw;
	y1 += diw;
	x2 -= diw;
	y2 -= diw;
    }

    if (btn->isOn() || btn->isDown()) {
	key.data.state = KLegacy::Active;
	key.data.shadow = KLegacy::In;
    } else {
	key.data.state = ((btn->isEnabled()) ?
			  ((static_cast<QWidget *>( btn ) == priv->lastWidget) ? KLegacy::Prelight : KLegacy::Normal) :
			  KLegacy::Insensitive);
	key.data.shadow = ((btn->isOn() || btn->isDown()) ?
			   KLegacy::In : KLegacy::Out);
    }

    QPixmap *pix = gobj->draw(key, x2 - x1 + 1, y2 - y1 + 1, "button");

    if (pix && ! pix->isNull())
	p->drawPixmap(x1, y1, *pix);
    else {
	KStyle::drawBevelButton(p, x1, y1, x2 - x1 + 1, y2 - y1 + 1,
				g, btn->isOn() || btn->isDown(), &fill);
    }
}


void KLegacyStyle::drawIndicator(QPainter *p, int x, int y, int w, int h,
			      const QColorGroup &g, int state,
			      bool down, bool enabled)
{
    GtkObject *gobj = priv->gtkDict.find(QCheckBox::staticMetaObject());

    if (! gobj) {
	KStyle::drawIndicator(p, x, y, w, h, g, state, down, enabled);
	return;
    }

    KLegacyImageDataKey key;
    key.cachekey = 0;
    key.data.function = KLegacy::Check;
    key.data.state = KLegacy::Normal;
    key.data.shadow = ((state != QCheckBox::Off) || down) ? KLegacy::In : KLegacy::Out;

    QPixmap *pix = gobj->draw(key, w, h, "checkbutton");

    if (pix && (! pix->isNull()))
     	p->drawPixmap(x, y, *pix);
    else
	KStyle::drawIndicator(p, x, y, w, h, g, state, down, enabled);
}


void KLegacyStyle::drawIndicatorMask(QPainter *p, int x, int y, int w, int h, int state) {
    GtkObject *gobj = priv->gtkDict.find(QCheckBox::staticMetaObject());

    if (! gobj) {
	KStyle::drawIndicatorMask(p, x, y, w, h, state);
	return;
    }

    KLegacyImageDataKey key;
    key.cachekey = 0;
    key.data.function = KLegacy::Check;
    key.data.state = KLegacy::Normal;
    key.data.shadow = (state != QCheckBox::Off) ? KLegacy::In : KLegacy::Out;

    QPixmap *pix = gobj->draw(key, w, h, "checkbutton");

    if (pix && pix->mask() && ! pix->mask()->isNull())
	p->drawPixmap(x, y, *(pix->mask()));
    else
	KStyle::drawIndicatorMask(p, x, y, w, h, state);
}


QSize KLegacyStyle::indicatorSize(void) const {
    GtkObject *gobj = priv->gtkDict.find(QCheckBox::staticMetaObject());

    if (! gobj)	return KStyle::indicatorSize();

    KLegacyImageDataKey key;
    key.cachekey = 0;
    key.data.function = KLegacy::Check;
    key.data.shadow = KLegacy::Out;
    KLegacyImageData *id = gobj->getImageData(key, "checkbutton");

    if (! id) return KStyle::indicatorSize();

    QString filename;
    if (! id->file.isNull())
	filename = id->file;
    else if (! id->overlayFile.isNull())
	filename = id->overlayFile;
    else
	return KStyle::indicatorSize();

    QImage *image = imageCache->find(filename);
    if (! image) {
        image = new QImage(filename);

        if (! image) return KStyle::indicatorSize();

        imageCache->insert(filename, image);
    }

    return QSize(image->width(), image->height());
}


void KLegacyStyle::drawExclusiveIndicator(QPainter *p, int x, int y, int w, int h,
				      const QColorGroup &g, bool on,
				       bool down, bool enabled)
{
    GtkObject *gobj = priv->gtkDict.find(QRadioButton::staticMetaObject());

    if (! gobj) {
	drawExclusiveIndicator(p, x, y, w, h, g, on, down, enabled);
	return;
    }

    KLegacyImageDataKey key;
    key.cachekey = 0;
    key.data.function = KLegacy::Option;
    key.data.state = KLegacy::Normal;
    key.data.shadow = (on || down) ? KLegacy::In : KLegacy::Out;

    QPixmap *pix = gobj->draw(key, w, h, "radiobutton");

    if (pix && (! pix->isNull()))
	p->drawPixmap(x, y, *pix);
    else
	KStyle::drawExclusiveIndicator(p, x, y, w, h, g, down, enabled);
}


void KLegacyStyle::drawExclusiveIndicatorMask(QPainter *p, int x, int y, int w, int h,
					   bool on)
{
    GtkObject *gobj = priv->gtkDict.find(QRadioButton::staticMetaObject());

    if (! gobj) {
	KStyle::drawExclusiveIndicatorMask(p, x, y, w, h, on);
	return;
    }

    KLegacyImageDataKey key;
    key.cachekey = 0;
    key.data.function = KLegacy::Option;
    key.data.state = KLegacy::Normal;
    key.data.shadow = (on) ? KLegacy::In : KLegacy::Out;

    QPixmap *pix = gobj->draw(key, w, h, "radiobutton");

    if (pix && pix->mask() && ! pix->mask()->isNull())
	p->drawPixmap(x, y, *(pix->mask()));
    else
	KStyle::drawExclusiveIndicatorMask(p, x, y, w, h, on);
}


QSize KLegacyStyle::exclusiveIndicatorSize(void) const {
    GtkObject *gobj = priv->gtkDict.find(QRadioButton::staticMetaObject());

    if (! gobj) {
	return KStyle::indicatorSize();
    }

    KLegacyImageDataKey key;
    key.cachekey = 0;
    key.data.function = KLegacy::Option;
    key.data.shadow = KLegacy::Out;
    KLegacyImageData *id = gobj->getImageData(key, "radiobutton");

    if (! id) return KStyle::indicatorSize();

    QString filename;
    if (! id->file.isNull()) {
	filename = id->file;
    } else if (! id->overlayFile.isNull()) {
	filename = id->overlayFile;
    } else {
	return KStyle::indicatorSize();
    }

    QImage *image = imageCache->find(filename);
    if (! image) {
        image = new QImage(filename);

        if (! image) {
	    return KStyle::indicatorSize();
	}

        imageCache->insert(filename, image);
    }

    return QSize(image->width(), image->height());
}


void KLegacyStyle::drawPopupMenuItem(QPainter *p, bool checkable, int maxpmw, int tab,
				  QMenuItem *mi, const QPalette &pal, bool act,
				  bool enabled, int x, int y, int w, int h)
{
    const QColorGroup & g = pal.active();
    QColorGroup itemg = (! enabled) ? pal.disabled() : pal.active();

    if (checkable)
	maxpmw = QMAX(maxpmw, 15);

    int checkcol = maxpmw;

    if (mi && mi->isSeparator()) {
	p->setPen( g.dark() );
	p->drawLine( x, y, x+w, y );
	p->setPen( g.light() );
	p->drawLine( x, y+1, x+w, y+1 );
	return;
    }

    if ( act && enabled ) {
	GtkObject *gobj = priv->gtkDict.find(&menuitem_ptr);

	if (gobj) {
	    KLegacyImageDataKey key;
	    key.cachekey = 0;
	    key.data.function = KLegacy::Box;
	    key.data.state = KLegacy::Prelight;
	    key.data.shadow = KLegacy::Out;

	    QPixmap *pix = gobj->draw(key, w, h, "menuitem");

	    if (pix && ! pix->isNull())
		p->drawPixmap(x, y, *pix);
	}
    } else
	p->fillRect(x, y, w, h, g.brush( QColorGroup::Button ));

    if ( !mi )
	return;

    if ( mi->isChecked() ) {
	if ( mi->iconSet() ) {
	    qDrawShadePanel( p, x+2, y+2, checkcol, h-2*2,
			     g, true, 1, &g.brush( QColorGroup::Midlight ) );
	}
    } else if ( !act ) {
	p->fillRect(x+2, y+2, checkcol, h-2*2,
		    g.brush( QColorGroup::Button ));
    }

    if ( mi->iconSet() ) {		// draw iconset
	QIcon::Mode mode = (enabled) ? QIcon::Normal : QIcon::Disabled;

	if (act && enabled)
	    mode = QIcon::Active;

	QPixmap pixmap = mi->iconSet()->pixmap(QIcon::Small, mode);

	int pixw = pixmap.width();
	int pixh = pixmap.height();

	QRect cr( x + 2, y+2, checkcol, h-2*2 );
	QRect pmr( 0, 0, pixw, pixh );

	pmr.moveCenter(cr.center());

	p->setPen( itemg.text() );
	p->drawPixmap( pmr.topLeft(), pixmap );

    } else  if (checkable) {
	int mw = checkcol;
	int mh = h - 4;

	if (mi->isChecked())
	    drawCheckMark(p, x+2, y+2, mw, mh, itemg, act, ! enabled);
    }

    p->setPen( g.buttonText() );

    QColor discol;
    if (! enabled) {
	discol = itemg.text();
	p->setPen( discol );
    }

    if (mi->custom()) {
	p->save();
	mi->custom()->paint(p, itemg, act, enabled, x + checkcol + 4, y + 2,
			    w - checkcol - tab - 3, h - 4);
	p->restore();
    }

    QString s = mi->text();
    if ( !s.isNull() ) {			// draw text
	int t = s.find( '\t' );
	int m = 2;
	const int text_flags = Qt::AlignVCenter|Qt::TextShowMnemonic | Qt::TextDontClip | Qt::TextSingleLine;
	if ( t >= 0 ) {				// draw tab text
	    p->drawText( x+w-tab-2-2,
			 y+m, tab, h-2*m, text_flags, s.mid( t+1 ) );
	}
	p->drawText(x + checkcol + 4, y + 2, w - checkcol -tab - 3, h - 4,
		    text_flags, s, t);
    } else if (mi->pixmap()) {
	QPixmap *pixmap = mi->pixmap();

	if (pixmap->depth() == 1) p->setBackgroundMode(Qt::OpaqueMode);
	p->drawPixmap(x + checkcol + 2, y + 2, *pixmap);
	if (pixmap->depth() == 1) p->setBackgroundMode(Qt::TransparentMode);
    }

    if (mi->popup()) {
	int hh = h / 2;

	drawMenuArrow(p, Qt::RightArrow, (act) ? mi->isEnabled() : false,
		      x + w - hh - 6, y + (hh / 2), hh, hh, g, mi->isEnabled());
    }
}


void KLegacyStyle::drawComboButton(QPainter *p, int x, int y, int w, int h,
				const QColorGroup &g, bool sunken, bool editable,
				bool enabled, const QBrush *b)
{
    GtkObject *gobj = priv->gtkDict.find(QComboBox::staticMetaObject());

    if (! gobj) {
	KStyle::drawComboButton(p, x, y, w, h, g, sunken, editable, enabled, b);
	return;
    }

    KLegacyImageDataKey key;
    key.cachekey = 0;
    key.data.function = KLegacy::Box;
    key.data.state = KLegacy::Normal;
    key.data.shadow = KLegacy::Out;

    if (priv->lastWidget && priv->lastWidget->inherits("QComboBox"))
	key.data.state = KLegacy::Prelight;

    QPixmap *pix = gobj->draw(key, w, h, "optionmenu");

    if (pix && ! pix->isNull()) {
	p->drawPixmap(x, y, *pix);
    } else {
	KStyle::drawComboButton(p, x, y, w, h, g, sunken, editable, enabled, b);
	return;
    }

    QRect rect = comboButtonRect(x, y, w, h);
    int tw = w - rect.width() - rect.right() - rect.left();
    int th = rect.height();

    key.data.function = KLegacy::Tab;
    key.data.state = KLegacy::Normal;
    pix = gobj->draw(key, tw, th, "optionmenutab");

    if (pix && ! pix->isNull())
	p->drawPixmap(x + rect.width() + rect.left() + ((18 - pix->width()) / 2),
		      y + rect.y() + ((rect.height() - pix->height()) / 2), *pix);
}


QRect KLegacyStyle::comboButtonRect(int x, int y, int w, int h) {
    GtkObject *gobj = priv->gtkDict.find(QComboBox::staticMetaObject());

    if (! gobj) {
	return KStyle::comboButtonRect(x, y, w, h);
    }

    KLegacyImageDataKey key;
    key.cachekey = 0;
    key.data.function = KLegacy::Box;
    KLegacyImageData *id = gobj->getImageData(key, "optionmenu");

    if (! id) {
	return KStyle::comboButtonRect(x, y, w, h);
    }

    return QRect(x + id->border.left() + 1, y + id->border.top() + 1,
		 w - id->border.left() - id->border.right() - 18,
		 h - id->border.top() - id->border.bottom() - 2);
}


QRect KLegacyStyle::comboButtonFocusRect(int x, int  y, int w, int h) {
    return comboButtonRect(x, y, w, h);
}


QStyle::ScrollControl KLegacyStyle::scrollBarPointOver(const QScrollBar *scrollbar,
						       int sliderStart, const QPoint &p)
{
    return QCommonStyle::scrollBarPointOver(scrollbar, sliderStart, p);
}


void KLegacyStyle::scrollBarMetrics(const QScrollBar *scrollbar, int &sliderMin,
				 int &sliderMax, int &sliderLength, int &buttonDim)
{
    int maxLength;
    int b = defaultFrameWidth();
    int length = ((scrollbar->orientation() == Qt::Horizontal) ?
		  scrollbar->width() : scrollbar->height());
    int extent = ((scrollbar->orientation() == Qt::Horizontal) ?
		  scrollbar->height() : scrollbar->width());

    if (length > ((extent - (b * 2) - 1) * 2) + (b * 2))
	buttonDim = extent - (b * 2);
    else
	buttonDim = ((length - (b * 2)) / 2) - 1;

    sliderMin = b + buttonDim;
    maxLength = length - (b * 2) - (buttonDim * 2);

    if (scrollbar->maxValue() == scrollbar->minValue()) {
	sliderLength = maxLength - 2;
    } else {
	uint range = scrollbar->maxValue() - scrollbar->minValue();

	sliderLength = (scrollbar->pageStep() * maxLength) /
		       (range + scrollbar->pageStep());

	if (sliderLength < buttonDim || range > (INT_MAX / 2))
	    sliderLength = buttonDim;
	if (sliderLength >= maxLength)
	    sliderLength = maxLength - 2;
    }

    sliderMax = sliderMin + maxLength - sliderLength;

    sliderMin += 1;
    sliderMax -= 1;
}


void KLegacyStyle::drawScrollBarControls(QPainter *p, const QScrollBar *scrollbar,
				     int start, uint controls, uint active)
{
    if (! scrollbar->isVisible()) return;

    GtkObject *gobj = priv->gtkDict.find(QScrollBar::staticMetaObject());

    if (! gobj) {
	KStyle::drawScrollBarControls(p, scrollbar, start, controls, active);
	return;
    }

    KLegacyImageDataKey gkey;
    gkey.cachekey = 0;
    gkey.data.function = KLegacy::Box;
    gkey.data.orientation = scrollbar->orientation() + 1;

    KLegacyImageData *groove_id = gobj->getImageData(gkey, "trough");

    if (! groove_id) {
	KStyle::drawScrollBarControls(p, scrollbar, start, controls, active);
	return;
    }

    int sliderMin;
    int sliderMax;
    int sliderLen;
    int buttonDim;
    scrollBarMetrics(scrollbar, sliderMin, sliderMax, sliderLen, buttonDim);

    // the rectangle for the slider
    QRect slider(
		 // x
		 ((scrollbar->orientation() == Qt::Vertical) ?
		  defaultFrameWidth() : start),

		 // y
		 ((scrollbar->orientation() == Qt::Vertical) ?
		  start : defaultFrameWidth()),

		 // w
		 ((scrollbar->orientation() == Qt::Vertical) ?
		  buttonDim : sliderLen),

		 // h
		 ((scrollbar->orientation() == Qt::Vertical) ?
		  sliderLen : buttonDim));

    KLegacyImageDataKey skey;
    skey.cachekey = 0;
    skey.data.function = KLegacy::Box;
    skey.data.orientation = scrollbar->orientation() + 1;

    if ((active & Slider) || (priv->hovering && slider.contains(priv->mousePos)))
	skey.data.state = KLegacy::Prelight;
    else
	skey.data.state = KLegacy::Normal;

    KLegacyImageData *slider_id = gobj->getImageData(skey, "slider");

    if (! slider_id) {
	KStyle::drawScrollBarControls(p, scrollbar, start, controls, active);
	return;
    }

    QPixmap *groove_pm = gobj->draw(groove_id, scrollbar->width(), scrollbar->height());

    if ((! groove_pm) || (groove_pm->isNull())) {
	groove_pm = 0;
    }

    QPixmap *slider_pm = gobj->draw(slider_id, slider.width(), slider.height());

    if ((! slider_pm) || (slider_pm->isNull())) {
	slider_pm = 0;
    }

    QPixmap buf(scrollbar->size());
    {
	QPainter p2(&buf);

	if (groove_pm) {
	    p2.drawTiledPixmap(scrollbar->rect(), *groove_pm);
	}

	if (slider_pm) {
	    p2.drawTiledPixmap(slider, *slider_pm);
	}

	// arrows
	int x, y;
	x = y = defaultFrameWidth();

	drawArrow(&p2, ((scrollbar->orientation() == Qt::Vertical) ?
			Qt::UpArrow : Qt::LeftArrow),
		  (active & SubLine), x, y,
		  buttonDim,
		  buttonDim,
		  scrollbar->colorGroup(), true);

	if  (scrollbar->orientation() == Qt::Vertical)
	    y = scrollbar->height() - buttonDim - defaultFrameWidth();
	else
	    x = scrollbar->width()  - buttonDim - defaultFrameWidth();

	drawArrow(&p2, ((scrollbar->orientation() == Qt::Vertical) ?
			Qt::DownArrow : Qt::RightArrow),
		  (active & AddLine), x, y,
		  buttonDim,
		  buttonDim,
		  scrollbar->colorGroup(), true);
    }
    p->drawPixmap(0, 0, buf);
}


void KLegacyStyle::drawSlider(QPainter *p, int x, int y, int w, int h, const QColorGroup &g,
			   Qt::Orientation orientation, bool tickAbove, bool tickBelow)
{
    GtkObject *gobj = priv->gtkDict.find(QSlider::staticMetaObject());

    if (! gobj) {
	KStyle::drawSlider(p, x, y, w, h, g, orientation, tickAbove, tickBelow);
	return;
    }

    KLegacyImageDataKey key;
    key.cachekey = 0;
    key.data.function = KLegacy::Box;
    key.data.shadow = KLegacy::Out;
    key.data.state = KLegacy::Normal;
    key.data.orientation = orientation + 1;

    QPixmap *pix = gobj->draw(key, w, h, "slider");

    if (pix && ! pix->isNull())
	p->drawPixmap(x, y, *pix);
    else
	KStyle::drawSlider(p, x, y, w, h, g, orientation, tickAbove, tickBelow);
}



void KLegacyStyle::drawSliderGroove(QPainter *p, int x, int y, int w, int h,
				const QColorGroup &g, QCOORD c, Qt::Orientation o)
{
    GtkObject *gobj = priv->gtkDict.find(QSlider::staticMetaObject());

    if (! gobj) {
	KStyle::drawSliderGroove(p, x, y, w, h, g, c, o);
	return;
    }

    KLegacyImageDataKey key;
    key.cachekey = 0;
    key.data.function = KLegacy::Box;
    key.data.shadow = KLegacy::In;
    key.data.state = KLegacy::Active;
    key.data.orientation = o + 1;

    QPixmap *pix = gobj->draw(key, w, h, "trough");

    if (pix && ! pix->isNull())
	p->drawPixmap(x, y, *pix);
    else
        KStyle::drawSliderGroove(p, x, y, w, h, g, c, o);
}


void KLegacyStyle::drawArrow(QPainter *p, Qt::ArrowType type, bool down,
			 int x, int y, int w, int h,
			 const QColorGroup &g, bool enabled, const QBrush *b)
{
    GtkObject *gobj = priv->gtkDict.find(&arrow_ptr);

    if (! gobj) {
	KStyle::drawArrow(p, type, down, x, y, w, h, g, enabled, b);
	return;
    }

    KLegacyImageDataKey key;
    key.cachekey = 0;
    key.data.function = KLegacy::Arrow;
    key.data.state = (down) ? KLegacy::Active : KLegacy::Normal;
    key.data.shadow = (down) ? KLegacy::In : KLegacy::NoShadow;
    key.data.arrowDirection = type + 1;

    if ((! down) && priv->hovering &&
	QRect(x, y, w, h).contains(priv->mousePos)) {
	key.data.state = KLegacy::Prelight;
    }

    QPixmap *pix = gobj->draw(key, w, h, "arrow");

    if (pix && ! pix->isNull())
	p->drawPixmap(x, y, *pix);
    else
	KStyle::drawArrow(p, type, down, x, y, w, h, g, enabled, b);
}


void KLegacyStyle::drawMenuArrow(QPainter *p, Qt::ArrowType type, bool down,
			 int x, int y, int w, int h,
			 const QColorGroup &g, bool enabled, const QBrush *b)
{
    GtkObject *gobj = priv->gtkDict.find(&menuitem_ptr);

    if (! gobj) {
	KStyle::drawArrow(p, type, down, x, y, w, h, g, enabled, b);
	return;
    }

    KLegacyImageDataKey key;
    key.cachekey = 0;
    key.data.function = KLegacy::Arrow;
    key.data.state = (down) ? KLegacy::Active : KLegacy::Normal;
    key.data.shadow = (down) ? KLegacy::In : KLegacy::NoShadow;
    key.data.arrowDirection = type + 1;

    QPixmap *pix = gobj->draw(key, w, h, "arrow");

    if (pix && ! pix->isNull())
    	p->drawPixmap(x + ((w - pix->width()) / 2),
	              y + ((h - pix->height()) / 2), *pix);
    else
	KStyle::drawArrow(p, type, down, x, y, w, h, g, enabled, b);
}


void KLegacyStyle::drawPanel(QPainter *p, int x, int y, int w, int h,
			  const QColorGroup &g, bool sunken, int, const QBrush *brush)
{
    KStyle::drawPanel(p, x, y, w, h, g, sunken, 1, brush);
}


void KLegacyStyle::drawPopupPanel(QPainter *p, int x, int y, int w, int h,
				  const QColorGroup &g, int, const QBrush *fill)
{
    QBrush brush = (fill) ? *fill : g.brush(QColorGroup::Background);

    p->fillRect(x, y, w, h, brush);
}


void KLegacyStyle::drawCheckMark(QPainter *p, int x, int y, int w, int h,
			      const QColorGroup &g, bool activated, bool disabled)
{
    GtkObject *gobj = priv->gtkDict.find(&checkmenuitem_ptr);

    if (! gobj) {
	KStyle::drawCheckMark(p, x, y, w, h, g, activated, disabled);
	return;
    }

    KLegacyImageDataKey key;
    key.cachekey = 0;
    key.data.function = KLegacy::Check;
    key.data.shadow = (disabled) ? KLegacy::Out : KLegacy::In;

    QPixmap *pix = gobj->draw(key, w, h);

    if (pix && (! pix->isNull())) {
	x += (w - pix->width()) / 2;
	y += (h - pix->height()) / 2;
	p->drawPixmap(x, y, *pix);
    } else {
	KStyle::drawCheckMark(p, x, y, w, h, g, activated, disabled);
    }
}


void KLegacyStyle::drawSplitter(QPainter *p, int x, int y, int w, int h,
			     const QColorGroup &g, Qt::Orientation orientation)
{
    if (orientation == Qt::Horizontal) {
	int xpos = x + (w / 2);
	int kpos = 10;
	int ksize = splitterWidth() - 2;

	qDrawShadeLine(p, xpos, kpos + ksize - 1, xpos, h, g);
	drawBevelButton(p, xpos - (splitterWidth() / 2) + 1, kpos, ksize, ksize,
			g, false, &g.brush(QColorGroup::Button));
	qDrawShadeLine(p, xpos, 0, xpos, kpos, g);
    } else {
	int ypos = y + (h / 2);
	int kpos = w - 10 - splitterWidth();
	int ksize = splitterWidth() - 2;

	qDrawShadeLine(p, 0, ypos, kpos, ypos, g);
	drawBevelButton(p, kpos, ypos - (splitterWidth() / 2) + 1, ksize, ksize,
			g, false, &g.brush(QColorGroup::Button));
	qDrawShadeLine(p, kpos + ksize - 1, ypos, w, ypos, g);
    }
}


void KLegacyStyle::drawTab(QPainter *p, const QTabBar *tabbar, QTab *tab, bool selected)
{
    GtkObject *gobj = priv->gtkDict.find(QTabBar::staticMetaObject());

    if (! gobj) {
	KStyle::drawTab(p, tabbar, tab, selected);
	return;
    }

    KLegacyImageDataKey key;
    key.cachekey = 0;
    key.data.function = KLegacy::Extension;
    key.data.state = (! selected) ? KLegacy::Active : KLegacy::Normal;
    key.data.shadow = KLegacy::Out;
    key.data.gapSide = (tabbar->shape() == QTabBar::RoundedNorth ||
			tabbar->shape() == QTabBar:: TriangularNorth) ?
		       KLegacy::Bottom : KLegacy::Top;

    int ry = tab->r.top(), rh = tab->r.height();

    if (! selected) {
	rh -= 2;

	if (tabbar->shape() == QTabBar::RoundedNorth ||
	    tabbar->shape() == QTabBar:: TriangularNorth)
	    ry += 2;
    }

    QPixmap *pix = gobj->draw(key, tab->r.width(), rh, "tab");


    if (pix && ! pix->isNull())
	p->drawPixmap(tab->r.left(), ry, *pix);
    else
	KStyle::drawTab(p, tabbar, tab, selected);
}


void KLegacyStyle::drawKBarHandle(QPainter *p, int x, int y, int w, int h,
				  const QColorGroup &g, KToolBarPos type, QBrush *fill)
{
    GtkObject *gobj = priv->gtkDict.find(Q3ToolBar::staticMetaObject());

    if (! gobj) {
	KStyle::drawKBarHandle(p, x, y, w, h, g, type, fill);
	return;
    }

    KLegacyImageDataKey key;
    key.cachekey = 0;
    key.data.function = KLegacy::Handle;
    key.data.state = KLegacy::Normal;
    key.data.shadow = KLegacy::Out;
    key.data.orientation = (type == Qt::DockLeft || type == Qt::DockRight) ?
			   Qt::Vertical + 1: Qt::Horizontal + 1;

    QPixmap *pix = gobj->draw(key, w, h, "handle");

    if (pix && ! pix->isNull())
	p->drawPixmap(x, y, *pix);
}


void KLegacyStyle::drawKickerHandle(QPainter *p, int x, int y, int w, int h,
				    const QColorGroup &g, QBrush *fill)
{
    drawKBarHandle(p, x, y, w, h, g, Qt::DockLeft, fill);
}


void KLegacyStyle::drawKickerAppletHandle(QPainter *p, int x, int y, int w, int h,
					  const QColorGroup &g, QBrush *fill)
{
    drawKBarHandle(p, x, y, w, h, g, Qt::DockLeft, fill);
}


void KLegacyStyle::drawKickerTaskButton(QPainter *p, int x, int y, int w, int h,
					const QColorGroup &g, const QString &title,
					bool active, QPixmap *icon, QBrush *fill)
{
    drawBevelButton(p, x, y, w, h, g, active, fill);

    const int pxWidth = 20;
    int textPos = pxWidth;

    QRect br(buttonRect(x, y, w, h));

    if (active)
        p->translate(1,1);

    if (icon && ! icon->isNull()) {
	int dx = (pxWidth - icon->width()) / 2;
	int dy = (h - icon->height()) / 2;

        p->drawPixmap(br.x() + dx, dy, *icon);
    }

    QString s(title);

    static const QString &modStr = KGlobal::staticQString(
        QString::fromUtf8("[") + i18n("modified") + QString::fromUtf8("]"));

    int modStrPos = s.find(modStr);

    if (modStrPos != -1) {
        s.remove(modStrPos, modStr.length()+1);

	QPixmap modPixmap = SmallIcon("modified");

	int dx = (pxWidth - modPixmap.width()) / 2;
	int dy = (h - modPixmap.height()) / 2;

	p->drawPixmap(br.x() + textPos + dx, dy, modPixmap);

	textPos += pxWidth;
    }

    if (! s.isEmpty()) {
	if (p->fontMetrics().width(s) > br.width() - textPos) {
	    int maxLen = br.width() - textPos - p->fontMetrics().width("...");

	    while ( (! s.isEmpty()) && (p->fontMetrics().width(s) > maxLen))
		s.truncate(s.length() - 1);

	    s.append("...");
	}

	p->setPen((active) ? g.foreground() : g.buttonText());

	p->drawText(br.x() + textPos, -1, w - textPos, h, Qt::AlignVCenter | Qt::AlignLeft, s);
    }
}


bool KLegacyStyle::eventFilter(QObject *obj, QEvent *e) {
    switch (e->type()) {
    case QEvent::Resize:
	{
	    QWidget *w = (QWidget *) obj;

	    if (w->inherits("QPopupMenu") && w->width() < 700) {
		GtkObject *gobj = priv->gtkDict.find(Q3PopupMenu::staticMetaObject());

		if (gobj) {
		    KLegacyImageDataKey key;
		    key.cachekey = 0;
		    key.data.function = KLegacy::Box;
		    key.data.state = KLegacy::Normal;
		    key.data.shadow = KLegacy::Out;

		    QPixmap *pix = gobj->draw(key, w->width(), w->height(), "menu");

		    if (pix && ! pix->isNull()) {
			QPalette pal = w->palette();

			// active
			QBrush brush = pal.brush(QPalette::Active,
						 QColorGroup::Background);
			brush.setPixmap(*pix);
			pal.setBrush(QPalette::Active,
				     QColorGroup::Background, brush);

			brush = pal.brush(QPalette::Active,
					  QColorGroup::Button);
			brush.setPixmap(*pix);
			pal.setBrush(QPalette::Active,
				     QColorGroup::Button, brush);

			// inactive
			brush = pal.brush(QPalette::Inactive,
					  QColorGroup::Background);
			brush.setPixmap(*pix);
			pal.setBrush(QPalette::Inactive,
				     QColorGroup::Background, brush);

			brush = pal.brush(QPalette::Inactive,
					  QColorGroup::Button);
			brush.setPixmap(*pix);
			pal.setBrush(QPalette::Inactive,
				     QColorGroup::Button, brush);

			// disabled
			brush = pal.brush(QPalette::Disabled,
					  QColorGroup::Background);
			brush.setPixmap(*pix);
			pal.setBrush(QPalette::Disabled,
				     QColorGroup::Background, brush);

			brush = pal.brush(QPalette::Disabled,
					  QColorGroup::Button);
			brush.setPixmap(*pix);
			pal.setBrush(QPalette::Disabled,
				     QColorGroup::Button, brush);

			w->setPalette(pal);
		    }
		}
	    } else if (w->isTopLevel() || w->inherits("QWorkspaceChild")) {
		GtkObject *gobj = priv->gtkDict.find(Q3MainWindow::staticMetaObject());

		if (gobj) {
		    KLegacyImageDataKey key;
		    key.cachekey = 0;
		    key.data.function = KLegacy::FlatBox;

		    QPixmap *p = gobj->draw(key, w->width(), w->height(), "base");

		    if (p && (! p->isNull()))
			w->setBackgroundPixmap(*p);
		}
	    } else if (w->inherits("QLineEdit")) {
		GtkObject *gobj = priv->gtkDict.find(QLineEdit::staticMetaObject());

		if (gobj) {
		    KLegacyImageDataKey key;
		    key.cachekey = 0;
		    key.data.function = KLegacy::FlatBox;
		    key.data.shadow = KLegacy::NoShadow;
		    key.data.state = (w->isEnabled()) ? KLegacy::Normal : KLegacy::Insensitive;

		    QPixmap *pix = gobj->draw(key, w->width(), w->height(),
					      "entry_bg");

		    if (pix && (! pix->isNull())) {
			QPalette pal = w->palette();

			// active
			QBrush brush = pal.brush(QPalette::Active,
						 QColorGroup::Base);
			brush.setPixmap(*pix);
			pal.setBrush(QPalette::Active,
				     QColorGroup::Base, brush);

			// inactive
			brush = pal.brush(QPalette::Inactive,
					  QColorGroup::Base);
			brush.setPixmap(*pix);
			pal.setBrush(QPalette::Inactive,
				     QColorGroup::Base, brush);

			// disabled
			brush = pal.brush(QPalette::Disabled,
					  QColorGroup::Base);
			brush.setPixmap(*pix);
			pal.setBrush(QPalette::Disabled,
				     QColorGroup::Base, brush);

			w->setPalette(pal);
		    }
		}
	    } else if (w->inherits("QMenuBar") ||
		       w->inherits("QToolBar")) {
		GtkObject *gobj = priv->gtkDict.find(QMenuBar::staticMetaObject());

		if (gobj) {
		    KLegacyImageDataKey key;
		    key.cachekey = 0;
		    key.data.function = KLegacy::Box;
		    key.data.state = KLegacy::Normal;
		    key.data.shadow = KLegacy::Out;

		    QPixmap *pix = gobj->draw(key, w->width(), w->height(),
					      "menubar");
		    if (pix && (! pix->isNull())) {
			QPalette pal = w->palette();

			// active
			QBrush brush = pal.brush(QPalette::Active,
						 QColorGroup::Background);
			brush.setPixmap(*pix);
			pal.setBrush(QPalette::Active,
				     QColorGroup::Background, brush);

			brush = pal.brush(QPalette::Active,
					  QColorGroup::Button);
			brush.setPixmap(*pix);
			pal.setBrush(QPalette::Active,
				     QColorGroup::Button, brush);

			// inactive
			brush = pal.brush(QPalette::Inactive,
					  QColorGroup::Background);
			brush.setPixmap(*pix);
			pal.setBrush(QPalette::Inactive,
				     QColorGroup::Background, brush);

			brush = pal.brush(QPalette::Inactive,
					  QColorGroup::Button);
			brush.setPixmap(*pix);
			pal.setBrush(QPalette::Inactive,
				     QColorGroup::Button, brush);

			// disabled
			brush = pal.brush(QPalette::Disabled,
					  QColorGroup::Background);
			brush.setPixmap(*pix);
			pal.setBrush(QPalette::Disabled,
				     QColorGroup::Background, brush);

			brush = pal.brush(QPalette::Disabled,
					  QColorGroup::Button);
			brush.setPixmap(*pix);
			pal.setBrush(QPalette::Disabled,
				     QColorGroup::Button, brush);

			w->setPalette(pal);
		    }
		}
	    }

	    break;
	}

    case QEvent::Enter:
	{
	    if (obj->inherits("QPushButton") ||
		obj->inherits("QComboBox") ||
		obj->inherits("QSlider") ||
		obj->inherits("QScrollbar")) {
		priv->lastWidget = (QWidget *) obj;
		priv->lastWidget->repaint(false);
	    } else if (obj->inherits("QRadioButton")) {
		QWidget *w = (QWidget *) obj;

		if (! w->isTopLevel() && w->isEnabled()) {
		    GtkObject *gobj = priv->gtkDict.find(QRadioButton::staticMetaObject());

		    if (gobj) {
			KLegacyImageDataKey key;
			key.cachekey = 0;
			key.data.function = KLegacy::FlatBox;

			QPixmap *pix = gobj->draw(key, w->width(), w->height());

			if (pix && (! pix->isNull())) {
			    QPalette pal = w->palette();
			    QBrush brush = pal.brush(QPalette::Normal,
						     QColorGroup::Background);

			    brush.setPixmap(*pix);
			    pal.setBrush(QPalette::Normal,
					 QColorGroup::Background, brush);

			    w->setPalette(pal);
			    w->setBackgroundMode(Qt::PaletteBackground);
			    w->setBackgroundOrigin(QWidget::WidgetOrigin);
			}
		    }
		}
	    } else if (obj->inherits("QCheckBox")) {
		QWidget *w = (QWidget *) obj;

		if (! w->isTopLevel() && w->isEnabled()) {
		    GtkObject *gobj = priv->gtkDict.find(QCheckBox::staticMetaObject());

		    if (gobj) {
			KLegacyImageDataKey key;
			key.cachekey = 0;
			key.data.function = KLegacy::FlatBox;

			QPixmap *pix = gobj->draw(key, w->width(), w->height());

			if (pix && (! pix->isNull())) {
			    QPalette pal = w->palette();
			    QBrush brush = pal.brush(QPalette::Normal,
						     QColorGroup::Background);

			    brush.setPixmap(*pix);
			    pal.setBrush(QPalette::Normal,
					 QColorGroup::Background, brush);

			    w->setPalette(pal);
			    w->setBackgroundMode(Qt::PaletteBackground);
			    w->setBackgroundOrigin(QWidget::WidgetOrigin);
			}
		    }
		}
	    }

	    break;
	}

    case QEvent::Leave:
	{
	    if (obj == priv->lastWidget) {
		priv->lastWidget = 0;
		((QWidget *) obj)->repaint(false);
	    } else if (obj->inherits("QRadioButton") ||
		       obj->inherits("QCheckBox")) {
		QWidget *w = (QWidget *) obj;

		if (! w->isTopLevel()) {
		    w->setBackgroundMode(Qt::X11ParentRelative);
		    w->setBackgroundOrigin(QWidget::WidgetOrigin);
		    w->repaint(true);
		}
	    }

	    break;
	}

    case QEvent::MouseMove:
	{
	    QMouseEvent *me = (QMouseEvent *) e;
	    priv->mousePos = me->pos();
	    if (obj->inherits("QScrollBar") &&
		(! (me->state() & (Qt::LeftButton | Qt::MidButton | Qt::RightButton)))) {
		priv->hovering = true;
		((QWidget *) obj)->repaint(false);
		priv->hovering = false;
	    }

	    break;
	}

    default:
	{
	    break;
	}
    }

    return KStyle::eventFilter(obj, e);
}
