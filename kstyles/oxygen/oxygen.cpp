/* Oxygen widget style for KDE 4
   Copyright (C) 2006-2007 Thomas Luebking <thomas.luebking@web.de>

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

/**================== Qt4 includes ======================*/
#include <QDir>
#include <QFile>
#include <QTextStream>
#include <QSettings>
#include <QEvent>
#include <QList>
#include <QResizeEvent>
#include <QApplication>
#include <QWidget>
#include <QPainter>
#include <QGroupBox>
#include <QPixmap>
#include <QImage>
#include <QDesktopWidget>
#include <QX11Info>
#include <QStylePlugin>
#include <QProgressBar>
#include <QMenu>
#include <QMenuBar>
#include <QStyleOptionProgressBarV2>
#include <QLayout>
#include <QListWidget>
#include <QAbstractButton>
#include <QPushButton>
#include <QScrollBar>
#include <QTabBar>
#include <QTabWidget>
#include <QComboBox>
#include <QCheckBox>
#include <QRadioButton>
#include <QSplitterHandle>
#include <QToolBar>
#include <QFrame>
#include <QLabel>
#include <QLineEdit>
#include <QAbstractScrollArea>
#include <QProcess>
/**============= Qt3 support includes ======================*/
#include <Q3ScrollView>
/**========================================================*/

/**============= System includes ==========================*/

#ifdef Q_WS_X11
// #include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/keysym.h>
#include <X11/extensions/XTest.h>
// #include <fixx11h.h>
#endif
#ifndef QT_NO_XRENDER
#include <X11/extensions/Xrender.h>
#endif

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <cmath>
/**========================================================*/

/**============= DEBUG includes ==========================*/
#undef DEBUG
#ifdef DEBUG
#define MOUSEDEBUG 1
#include <QtDebug>
#include "debug.h"
#define oDEBUG qDebug()
#include <QTime>
#include <QTimer>
#define _PROFILESTART_ QTime timer; int time; timer.start();
#define _PROFILERESTART_ timer.restart();
#define _PROFILESTOP_(_STRING_) time = timer.elapsed(); qDebug("%s: %d",_STRING_,time);
#else
#define oDEBUG //
#undef MOUSEDEBUG
#endif
/**========================================================*/

/**============= Oxygen includes ==========================*/
#include "oxygen.h"
#include "makros.h"
#ifndef QT_NO_XRENDER
#include "oxrender.h"
#endif
#include "visualframe.h"
/**=========================================================*/


/**============= extern C stuff ==========================*/
class OxygenStylePlugin : public QStylePlugin
{
public:
   QStringList keys() const {
      return QStringList() << "Oxygen";
   }
   
   QStyle *create(const QString &key) {
      if (key == "oxygen")
         return new Oxygen::OxygenStyle;
      return 0;
   }
};

Q_EXPORT_PLUGIN2(OxygenStyle, OxygenStylePlugin)
/**=========================================================*/
using namespace Oxygen;

/** static config object */
Config config;
Dpi dpi;

/** Let's try if we can supply round frames that shape their content */

/** Get some excluded code */
#include "inlinehelp.cpp"

/**Some static variables*/

// static const int windowsItemFrame	= 1; // menu item frame width
// static const int windowsSepHeight	= 2; // separator item height
// static const int windowsItemHMargin	= 3; // menu item hor text margin
// static const int windowsItemVMargin	= 1; // menu item ver text margin
// static const int windowsArrowHMargin	= 6; // arrow horizontal margin
// static const int windowsTabSpacing	= 12; // space between text and tab
// static const int windowsCheckMarkHMargin= 2; // horiz. margins of check mark
// static const int windowsRightBorder	= 12; // right border on windows
// static const int windowsCheckMarkWidth	= 14; // checkmarks width on windows

static bool invColorRole(QPalette::ColorRole &from, QPalette::ColorRole &to,
                         QPalette::ColorRole defFrom = QPalette::WindowText, QPalette::ColorRole defTo = QPalette::Window)
{
   switch (from)
   {
   case QPalette::WindowText: //0
      to = QPalette::Window; break;
   case QPalette::Window: //10
      to = QPalette::WindowText; break;
   case QPalette::Base: //9
      to = QPalette::Text; break;
   case QPalette::Text: //6
      to = QPalette::Base; break;
   case QPalette::Button: //1
      to = QPalette::ButtonText; break;
   case QPalette::ButtonText: //8
      to = QPalette::Button; break;
   case QPalette::Highlight: //12
      to = QPalette::HighlightedText; break;
   case QPalette::HighlightedText: //13
      to = QPalette::Highlight; break;
   default:
      from = defFrom;
      to = defTo;
      return false;
   }
   return true;
}

void OxygenStyle::makeStructure(int num, const QColor &c)
{
   if (!_scanlines[num])
      _scanlines[num] = new QPixmap(64, 64);
   QPainter p(_scanlines[num]);
   switch (config.structure)
   {
   default:
   case 0: { // scanlines
      _scanlines[num]->fill( c.light(110).rgb() );
      p.setPen( (num == 1) ? c.light(106) : c.light(103) );
      int i;
      for ( i = 1; i < 64; i += 4 )
      {
         p.drawLine( 0, i, 63, i );
         p.drawLine( 0, i+2, 63, i+2 );
      }
      p.setPen( c );
      for ( i = 2; i < 63; i += 4 )
         p.drawLine( 0, i, 63, i );
      break;
   }
   case 1: { //checkboard
      p.setPen(Qt::NoPen);
      p.setBrush(c.light(102));
      if (num == 1) {
         p.drawRect(0,0,16,16); p.drawRect(32,0,16,16);
         p.drawRect(16,16,16,16); p.drawRect(48,16,16,16);
         p.drawRect(0,32,16,16); p.drawRect(32,32,16,16);
         p.drawRect(16,48,16,16); p.drawRect(48,48,16,16);
      }
      else {
         p.drawRect(0,0,32,32);
         p.drawRect(32,32,32,32);
      }
      p.setBrush(c.dark(102));
      if (num == 1) {
         p.drawRect(16,0,16,16); p.drawRect(48,0,16,16);
         p.drawRect(0,16,16,16); p.drawRect(32,16,16,16);
         p.drawRect(16,32,16,16); p.drawRect(48,32,16,16);
         p.drawRect(0,48,16,16); p.drawRect(32,48,16,16);
      }
      else {
         p.drawRect(32,0,32,32);
         p.drawRect(0,32,32,32);
      }
      break;
   }
   case 2: // bricks - sucks...
      p.setPen(c.dark(105));
      p.setBrush(c.light(102));
      p.drawRect(0,0,32,16); p.drawRect(32,0,32,16);
      p.drawRect(0,16,16,16); p.drawRect(16,16,32,16); p.drawRect(48,16,16,16);
      p.drawRect(0,32,32,16); p.drawRect(32,32,32,16);
      p.drawRect(0,48,16,16); p.drawRect(16,48,32,16); p.drawRect(48,48,16,16);
      break;
   }
   p.end();
}

void OxygenStyle::readSettings()
{
   QSettings settings("Oxygen", "Style");
   settings.beginGroup("Style");
   
   config.bgMode = (BGMode) settings.value("BackgroundMode", BevelV).toInt();
#ifndef QT_NO_XRENDER
   if (config.bgMode > LightH)
      config.bgMode = BevelV;
   else if(config.bgMode == ComplexLights &&
           !QFile::exists(QDir::tempPath() + "oxygenPP.lock"))
      QProcess::startDetached ( settings.value("BgDaemon", "oxygenPP").toString() );
#else
   if (config.bgMode == ComplexLights) config.bgMode = BevelV;
#endif
   config.structure = settings.value("Structure", 0).toInt();
   
   config.scale = settings.value("Scale", 1.0).toDouble();
   config.checkType = settings.value("CheckType", 1).toInt();
   
   
   config.gradientIntensity = settings.value("GradientIntensity",70).toInt();
   
   config.tabTransition =
      (TabAnimInfo::TabTransition) settings.value("TabTransition",
         TabAnimInfo::ScanlineBlend).toInt();
   
   config.gradButton =
      (Gradients::Type) settings.value("GradButton", Gradients::None).toInt();
   config.gradChoose =
      (Gradients::Type) settings.value("GradChoose", Gradients::Glass).toInt();
   config.gradProgress =
      (Gradients::Type) settings.value("GradProgress", Gradients::Gloss).toInt();

   config.roundButtons = settings.value("RoundButtons", false).toBool();
   
   config.hoverImpact = 166 - settings.value("HoverImpact", 0).toInt();
   config.hoverImpact = CLAMP(config.hoverImpact, 6, 166);
   
   config.showMenuIcons = settings.value("ShowMenuIcons", false).toBool();
   config.menuShadow = settings.value("MenuShadow", false).toBool();
   
   config.glassProgress = settings.value("GlassProgress", true).toBool();
   
   // color roles
   
   config.role_progress[0] =
      (QPalette::ColorRole) settings.value("role_progressGroove", QPalette::WindowText).toInt();
   
   config.role_progress[1] =
      (QPalette::ColorRole) settings.value("role_progress", config.role_progress[1]).toInt();
   
   config.role_tab[0] =
      (QPalette::ColorRole) settings.value("role_tab", QPalette::Button).toInt();
   invColorRole(config.role_tab[0], config.role_tab[1],
                QPalette::Button, QPalette::ButtonText);
   
   config.role_btn[0] =
      (QPalette::ColorRole) settings.value("role_button", QPalette::Window).toInt();
   invColorRole(config.role_btn[0], config.role_btn[1],
                 QPalette::Window, QPalette::WindowText);
   
   config.role_btnHover[0] =
      (QPalette::ColorRole) settings.value("role_buttonHover", QPalette::Highlight).toInt();
   if (config.hoverImpact < 13)
      invColorRole(config.role_btnHover[0], config.role_btnHover[1],
                   QPalette::Highlight, QPalette::HighlightedText);
   else
      config.role_btnHover[1] = config.role_btn[1];
   
   config.role_popup[0] =
      (QPalette::ColorRole) settings.value("role_popup", QPalette::Window).toInt();
   invColorRole(config.role_popup[0], config.role_popup[1],
                QPalette::Window, QPalette::WindowText);

   settings.endGroup();
}

#define SCALE(_N_) lround(_N_*config.scale)

#include "genpixmaps.cpp"

void OxygenStyle::initMetrics()
{
   dpi.f1 = SCALE(1); dpi.f2 = SCALE(2);
   dpi.f3 = SCALE(3); dpi.f4 = SCALE(4);
   dpi.f5 = SCALE(5); dpi.f6 = SCALE(6);
   dpi.f7 = SCALE(7); dpi.f8 = SCALE(8);
   dpi.f9 = SCALE(9); dpi.f10 =SCALE(10);
   
   dpi.f12 = SCALE(12); dpi.f13 = SCALE(13);
   dpi.f16 = SCALE(16); dpi.f18 = SCALE(18);
   dpi.f20 = SCALE(20); dpi.f32 = SCALE(32);
   dpi.f80 = SCALE(80);
   
   dpi.ScrollBarExtent = SCALE(20);
   dpi.ScrollBarSliderMin = SCALE(40);
   dpi.SliderThickness = SCALE(24);
   dpi.SliderControl = SCALE(17);
   dpi.Indicator = SCALE(20);
   dpi.ExclusiveIndicator = SCALE(19);
}

#undef SCALE

/**THE STYLE ITSELF*/
OxygenStyle::OxygenStyle() : QCommonStyle(), mouseButtonPressed_(false),
internalEvent_(false) {
   _scanlines[0] = _scanlines[1] = 0L;
   readSettings();
   initMetrics();
   generatePixmaps();
   Gradients::init(config.bgMode > ComplexLights ?
                   (Gradients::BgMode)config.bgMode : Gradients::BevelV);
   //====== TOOLTIP ======================
//    tooltipPalette = qApp->palette();
//    tooltipPalette.setBrush( QPalette::Background, QColor( 255, 255, 220 ) );
//    tooltipPalette.setBrush( QPalette::Foreground, Qt::black );
   //=======================================
   

   // start being animated
   animator = new StyleAnimator(this, config.tabTransition);

}

OxygenStyle::~OxygenStyle() {
   Gradients::wipe();
//    bfi.clear();
}

void OxygenStyle::fillWithMask(QPainter *painter, const QPoint &xy, const QBrush &brush, const QPixmap &mask, QPoint offset) const
{
   QPixmap qPix(mask.size());
   if (brush.texture().isNull())
      qPix.fill(brush.color());
   else {
      QPainter p(&qPix);
      p.drawTiledPixmap(mask.rect(),brush.texture(),offset);
      p.end();
   }
#ifndef QT_NO_XRENDER
   qPix = OXRender::applyAlpha(qPix, mask);
#else
#warning no XRender - performance will suffer!
   qPix.setAlphaChannel(mask);
#endif
   painter->drawPixmap(xy, qPix);
}

void OxygenStyle::fillWithMask(QPainter *painter, const QRect &rect, const QBrush &brush, const Tile::Mask *mask, Tile::PosFlags pf, bool justClip, QPoint offset, bool inverse, const QRect *outerRect) const
{
   // TODO: get rid of this?! - masks now render themselves!
   bool pixmode = !brush.texture().isNull();
   if (!mask) {
      if (pixmode)
         painter->drawTiledPixmap(rect, brush.texture(), offset);
      else
         painter->fillRect(rect, brush.color());
      return;
   }
   mask->render(rect, painter, brush, pf, justClip, offset, inverse, outerRect);
}

/**======================================*/

/**QStyle reimplementation ========================================= */

void OxygenStyle::polish ( QApplication * app ) {
//    if (timer && !timer->isActive())
//       timer->start(50);
   QPalette pal = app->palette();
   polish(pal);
   app->setPalette(pal);
   app->installEventFilter(this);
}

#define _SHIFTCOLOR_(clr) clr = QColor(CLAMP(clr.red()-10,0,255),CLAMP(clr.green()-10,0,255),CLAMP(clr.blue()-10,0,255))

void OxygenStyle::polish( QPalette &pal )
{
   QColor c = pal.color(QPalette::Active, QPalette::Background);
   if (config.bgMode > Scanlines) {
      int h,s,v;
      c.getHsv(&h,&s,&v);
      if (v < 70) // very dark colors won't make nice backgrounds ;)
         c.setHsv(h,s,70);
      pal.setColor( QPalette::Window, c );
   }
   if (_scanlines[0]) delete _scanlines[0]; _scanlines[0] = 0;
   if (_scanlines[1]) delete _scanlines[1]; _scanlines[1] = 0;
   QLinearGradient lg; QPainter p;
   if (config.bgMode == Scanlines) {
      QColor c = pal.color(QPalette::Active, QPalette::Background);
      makeStructure(0, c);
      QBrush brush( c, *_scanlines[0] );
      pal.setBrush( QPalette::Background, brush );
   }
   
   int highlightGray = qGray(pal.color(QPalette::Active, QPalette::Highlight).rgb());
   pal.setColor(QPalette::Disabled, QPalette::Highlight,
                QColor(highlightGray,highlightGray,highlightGray));
   
   //inactive palette
   pal.setColor(QPalette::Inactive, QPalette::WindowText,
                midColor(pal.color(QPalette::Active, QPalette::Window),
                         pal.color(QPalette::Active, QPalette::WindowText)));
   pal.setColor(QPalette::Inactive, QPalette::Text,
                midColor(pal.color(QPalette::Active, QPalette::Base),
                         pal.color(QPalette::Active, QPalette::Text),1,3));
   pal.setColor(QPalette::Inactive, QPalette::Highlight,
                midColor(pal.color(QPalette::Active, QPalette::Highlight),
                         pal.color(QPalette::Disabled, QPalette::Highlight),3,1));
   pal.setColor(QPalette::Inactive, QPalette::AlternateBase,
                midColor(pal.color(QPalette::Active, QPalette::AlternateBase),
                         pal.color(QPalette::Active, QPalette::Base),3,1));
   
}

#include <QtDebug>
#ifdef Q_WS_X11
static Atom winTypePopup = XInternAtom(QX11Info::display(), "_NET_WM_WINDOW_TYPE_POPUP_MENU", False);
static Atom winType = XInternAtom(QX11Info::display(), "_NET_WM_WINDOW_TYPE", False);
#endif
void OxygenStyle::polish( QWidget * widget) {
   
   // installs dynamic brush to all widgets, taking care of a correct bg pixmap size
   //TODO maybe we can exclude some more widgets here... (currently only popup menus)
//    if (_bgBrush && !(
//          qobject_cast<QMenu*>(widget) ||
//          widget->inherits("QAlphaWidget") ||
//          widget->inherits("QComboBoxListView") ||
//          widget->inherits("QComboBoxPrivateContainer") ||
//          // Kwin stuff ===========================
//          widget->topLevelWidget()->objectName() == "decoration widget" ||
//          widget->topLevelWidget()->inherits("QDesktopWidget") ||
//          widget->topLevelWidget()->objectName() == "desktop_widget"
//         ))
//       widget->installEventFilter(_bgBrush);
   if (widget->isWindow() && config.bgMode > Scanlines) {
      widget->setAutoFillBackground(true);
      widget->setAttribute(Qt::WA_StyledBackground);
   }

#ifdef MOUSEDEBUG
   widget->installEventFilter(this);
#endif
   
   if (false
//         qobject_cast<QPushButton *>(widget)
// #ifndef QT_NO_COMBOBOX
//        || qobject_cast<QComboBox *>(widget)
// #endif
#ifndef QT_NO_SPINBOX
       || qobject_cast<QAbstractSpinBox *>(widget)
#endif
//        || qobject_cast<QCheckBox *>(widget)
       || qobject_cast<QScrollBar *>(widget)
       || widget->inherits("QHeaderView")
//        || qobject_cast<QRadioButton *>(widget)
#ifndef QT_NO_SPLITTER
       || qobject_cast<QSplitterHandle *>(widget)
#endif
#ifndef QT_NO_TABBAR
       || qobject_cast<QTabBar *>(widget)
#endif
       || widget->inherits("QWorkspaceTitleBar")
       || widget->inherits("QToolButton")
       || widget->inherits("QDockWidget")
       || widget->inherits("QToolBar")
       || widget->inherits("QToolBarHandle")
       || widget->inherits("QDockSeparator")
       || widget->inherits("QToolBoxButton")
       || widget->inherits("QAbstractSlider")
       || widget->inherits("QDockWidgetSeparator")
       || widget->inherits("Q3DockWindowResizeHandle")
      )
         widget->setAttribute(Qt::WA_Hover);
   
   if (qobject_cast<QAbstractButton*>(widget)) {
      if (widget->inherits("QToolBoxButton"))
         widget->setForegroundRole ( QPalette::WindowText );
      else {
         widget->setBackgroundRole ( config.role_btn[0] );
         widget->setForegroundRole ( config.role_btn[1] );
         widget->installEventFilter(this);
      }
   }
   if (qobject_cast<QComboBox *>(widget)) {
      widget->setBackgroundRole ( QPalette::Base );
      widget->setForegroundRole ( QPalette::Text );
      widget->installEventFilter(this);
   }
   if (qobject_cast<QAbstractSlider *>(widget)) {
      widget->installEventFilter(this);
      if (qobject_cast<QScrollBar *>(widget) &&
         !(widget->parentWidget() &&
            widget->parentWidget()->parentWidget() &&
            widget->parentWidget()->parentWidget()->inherits("QComboBoxListView")))
         widget->setAttribute(Qt::WA_OpaquePaintEvent, false);
   }
   
   if (qobject_cast<QProgressBar*>(widget)) {
      widget->setBackgroundRole ( config.role_progress[0] );
      widget->setForegroundRole ( config.role_progress[1] );
      widget->installEventFilter(this);
   }
   
   if (qobject_cast<QTabWidget*>(widget))
      widget->installEventFilter(this);

   if (qobject_cast<QTabBar *>(widget)) {
      widget->setBackgroundRole ( config.role_tab[0] );
      widget->setForegroundRole ( config.role_tab[1] );
      widget->installEventFilter(this);
   }
   
   
   if (qobject_cast<QAbstractScrollArea*>(widget) || qobject_cast<Q3ScrollView*>(widget) ||
       widget->inherits("QWorkspaceTitleBar"))
      widget->installEventFilter(this);
   
   if (widget->inherits("QWorkspace"))
      connect(this, SIGNAL(MDIPopup(QPoint)), widget, SLOT(_q_popupOperationMenu(QPoint)));

   
   if (false // to simplify the #ifdefs
#ifndef QT_NO_MENUBAR
       || qobject_cast<QMenuBar *>(widget)
#endif
#ifdef QT3_SUPPORT
       || widget->inherits("Q3ToolBar")
#endif
#ifndef QT_NO_TOOLBAR
       || qobject_cast<QToolBar *>(widget)
       || (widget && qobject_cast<QToolBar *>(widget->parent()))
#endif
      ) {
      widget->setBackgroundRole(QPalette::Window);
      if (config.bgMode == Scanlines) {
         widget->setAutoFillBackground ( true );
         QPalette pal = widget->palette();
         QColor c = pal.color(QPalette::Active, QPalette::Window);
         
         if (!_scanlines[1])
            makeStructure(1, c);
         QBrush brush( c, *_scanlines[1] );
         pal.setBrush( QPalette::Window, brush );
         widget->setPalette(pal);
      }
   }
   
   if (!widget->isWindow())
   if (QFrame *frame = qobject_cast<QFrame *>(widget)) {
      // kill ugly winblows frames...
      if (frame->frameShape() == QFrame::Box ||
          frame->frameShape() == QFrame::Panel ||
          frame->frameShape() == QFrame::WinPanel)
         frame->setFrameShape(QFrame::StyledPanel);
      
      // overwrite ugly lines
      if (frame->frameShape() == QFrame::HLine ||
          frame->frameShape() == QFrame::VLine)
         widget->installEventFilter(this);
      
      // toolbox handling - a shame they look that crap by default!
      else if (widget->inherits("QToolBox")) {
         widget->setBackgroundRole(QPalette::Window);
         widget->setForegroundRole(QPalette::WindowText);
         if (widget->layout()) {
            widget->layout()->setMargin ( 0 );
            widget->layout()->setSpacing ( 0 );
         }
      }

//        && !(
//          widget->inherits("QComboBoxListView") ||
//          widget->inherits("QComboBoxPrivateContainer"))
      else if (frame->frameShape() == QFrame::StyledPanel) {
         if (widget->inherits("QTextEdit") && frame->lineWidth() == 1)
            frame->setLineWidth(dpi.f4);
         else {
            QWidget *grampa = frame->parentWidget();
            if (!grampa) grampa = frame;
            QList<VisualFrame*> vfs = grampa->findChildren<VisualFrame*>();
            bool addVF = true;
            foreach (VisualFrame* vf, vfs)
               if (vf->frame() == frame) { addVF = false; break; }
            if (addVF) {
               int f2 = dpi.f2, f3 = dpi.f3, f4 = dpi.f4, f6 = dpi.f6;
               int s[4]; uint t[4]; // t/b/l/r
               if (frame->frameShadow() == QFrame::Sunken) {
                  s[0] = s[2] = s[3] = 0; s[1] = f3;
                  t[0] = t[1] = t[2] = t[3] = f4;
               }
               else if (frame->frameShadow() == QFrame::Raised) {
                  s[0] = f2; s[1] = f4; s[2] = s[3] = f2;
                  t[0] = t[2] = t[3] = f4; t[1] = f6;
               }
               else { // plain
                  s[0] = s[1] = s[2] = s[3] = f2;
                  t[0] = t[1] = t[2] = t[3] = f2;
               }
               new VisualFrame(grampa, frame, VisualFrame::North,
                               t[0], s[0], s[2], s[3]);
               new VisualFrame(grampa, frame, VisualFrame::South,
                               t[1], s[1], s[2], s[3]);
               new VisualFrame(grampa, frame, VisualFrame::West,
                               t[2], s[2], t[0]-s[0], t[1]-s[1], t[0], t[1]);
               new VisualFrame(grampa, frame, VisualFrame::East,
                               t[3], s[3], t[0]-s[0], t[1]-s[1], t[0], t[1]);
            }
         }
      }
   }
   
   if (widget->autoFillBackground() &&
       // dad
       widget->parentWidget() &&
       ( widget->parentWidget()->objectName() == "qt_scrollarea_viewport" ) &&
       //grampa
       widget->parentWidget()->parentWidget() &&
       qobject_cast<QAbstractScrollArea*>(widget->parentWidget()->parentWidget()) &&
       // grangrampa
       widget->parentWidget()->parentWidget()->parentWidget() &&
       widget->parentWidget()->parentWidget()->parentWidget()->inherits("QToolBox")
      ) {
      widget->parentWidget()->setAutoFillBackground(false);
      widget->setAutoFillBackground(false);
   }
   
   // swap qmenu colors
   if (qobject_cast<QMenu *>(widget)) {
#ifdef Q_WS_X11
      // this should tell beryl et. al this is a popup - doesn't work... yet
      XChangeProperty(QX11Info::display(), widget->winId(), winType,
                      XA_CARDINAL, 32, PropModeReplace, (const unsigned char*)&winTypePopup, 1L);
#endif
      // WARNING: compmgrs like e.g. beryl deny to shadow shaped windows,
      // if we cannot find a way to get ARGB menus independent from the app settings, the compmgr must handle the round corners here
      widget->installEventFilter(this); // for the round corners
      widget->setAutoFillBackground (true);
      widget->setBackgroundRole ( config.role_popup[0] );
      widget->setForegroundRole ( config.role_popup[1] );
      if (qGray(widget->palette().color(QPalette::Active, widget->backgroundRole()).rgb()) < 100) {
         QFont tmpFont = widget->font();
         tmpFont.setBold(true);
         widget->setFont(tmpFont);
      }
   }
   
   //========================

}

bool OxygenStyle::eventFilter( QObject *object, QEvent *ev ) {
   switch (ev->type()) {
   case QEvent::Paint: {
      if (QFrame *frame = qobject_cast<QFrame*>(object)) {
         if (frame->frameShape() == QFrame::HLine ||
             frame->frameShape() == QFrame::VLine) {
            if (frame->isVisible()) {
               QPainter p(frame);
               Orientation3D o3D =
                  (frame->frameShadow() == QFrame::Sunken) ? Sunken:
                  (frame->frameShadow() == QFrame::Raised) ? Raised : Relief;
               const bool v = frame->frameShape() == QFrame::VLine;
               shadows.line[v][o3D].render(frame->rect(), &p);
               p.end();
            }
            return true;
         }
      }
      else if (QTabBar *tabBar = qobject_cast<QTabBar*>(object)) {
         if (tabBar->parentWidget() &&
             qobject_cast<QTabWidget*>(tabBar->parentWidget()))
            return false; // no extra tabbar here please...
         QPainter p(tabBar);
         QStyleOptionTabBarBase opt;
         opt.initFrom(tabBar);
         drawPrimitive ( PE_FrameTabBarBase, &opt, &p);
         p.end();
      }
      return false;
   }
//    case QEvent::Resize: {
//       if (QMenu *menu = qobject_cast<QMenu*>(object)) {
//                   QResizeEvent *rev = (QResizeEvent*)ev;
//          int w = ((QResizeEvent*)ev)->size().width(),
//             h = ((QResizeEvent*)ev)->size().height();
//          QRegion mask(0,0,w,h);
//          mask -= masks.popupCorner[0]; // tl
//          QRect br = masks.popupCorner[1].boundingRect();
//          mask -= masks.popupCorner[1].translated(w-br.width(), 0); // tr
//          br = masks.popupCorner[2].boundingRect();
//          mask -= masks.popupCorner[2].translated(0, h-br.height()); // bl
//          br = masks.popupCorner[3].boundingRect();
//          mask -= masks.popupCorner[3].translated(w-br.width(), h-br.height()); // br
//          menu->setMask(mask);
//       }
//       return false;
//    }
   case QEvent::MouseButtonPress: {
      QMouseEvent *mev = (QMouseEvent*)ev;
#ifdef MOUSEDEBUG
      qDebug() << object;
#endif
      if (( mev->button() == Qt::LeftButton) &&
          object->inherits("QWorkspaceTitleBar")) {
         //TODO this is a hack to get the popupmenu to the right side. bug TT to query the position with a SH
         QWidget *widget = (QWidget*)object;
         // check for menu button
         QWidget *MDI = qobject_cast<QWidget*>(widget->parent()); if (!MDI) return false; //this is elsewhat...
         /// this does not work as TT keeps the flag in a private to the titlebar (for no reason?)
//             if (!(widget->windowFlags() & Qt::WindowSystemMenuHint)) return false;
         // check if we clicked it..
         if (mev->x() < widget->width()-widget->height()-2) return false;
         // find popup
         MDI = qobject_cast<QWidget*>(MDI->parent()); if (!MDI) return false; //this is elsewhat...
         MDI = MDI->findChild<QMenu *>("qt_internal_mdi_popup");
         if (!MDI) {
            qWarning("MDI popup not found, unable to calc menu position");
            return false;
         }
         // calc menu position
         emit MDIPopup(widget->mapToGlobal( QPoint(widget->width() - MDI->sizeHint().width(), widget->height())));
         return true;
      }
      return false;
   }
   case QEvent::Show: {
      if (QProgressBar *progress = qobject_cast<QProgressBar*>(object))
      if (progress->isEnabled()) {
         animator->addProgressBar(progress);
         return false;
      }
      if (QTabWidget* tab = qobject_cast<QTabWidget*>(object)) {
         animator->addTab(tab, tab->currentIndex());
         return false;
      }
      return false;
   }
   case QEvent::Hide: {
      if (QProgressBar *progress = qobject_cast<QProgressBar*>(object)) {
         animator->removeProgressBar(progress);
         return false;
      }
      if (QTabWidget* tab = qobject_cast<QTabWidget*>(object)) {
         animator->removeTab(tab);
         return false;
      }
      return false;
   }
#define HANDLE_SCROLL_AREA_EVENT \
         if (area->horizontalScrollBar()->isVisible())\
            animator->fadeIn(area->horizontalScrollBar());\
         if (area->verticalScrollBar()->isVisible())\
            animator->fadeIn(area->verticalScrollBar());
   case QEvent::Enter:
      if (qobject_cast<QAbstractButton*>(object) ||
          qobject_cast<QComboBox*>(object)) {
         QWidget *widget = (QWidget*)object;
         if (!widget->isEnabled())
            return false;
         animator->fadeIn(widget);
         return false;
      }
      else if (QAbstractScrollArea* area =
          qobject_cast<QAbstractScrollArea*>(object)) {
         if (!area->isEnabled()) return false;
         HANDLE_SCROLL_AREA_EVENT
         return false;
      }
      else if (Q3ScrollView* area =
               qobject_cast<Q3ScrollView*>(object)) {
         if (!area->isEnabled()) return false;
         HANDLE_SCROLL_AREA_EVENT
         return false;
      }
      return false;

#undef HANDLE_SCROLL_AREA_EVENT
#define HANDLE_SCROLL_AREA_EVENT \
         if (area->horizontalScrollBar()->isVisible())\
            animator->fadeOut(area->horizontalScrollBar());\
         if (area->verticalScrollBar()->isVisible())\
            animator->fadeOut(area->verticalScrollBar());
   case QEvent::Leave:
      if (qobject_cast<QAbstractButton*>(object) || 
          qobject_cast<QComboBox*>(object)) {
         QWidget *widget = (QWidget*)object;
         if (!widget->isEnabled())
            return false;
         animator->fadeOut(widget);
         return false;
      }
      else if (QAbstractScrollArea* area =
          qobject_cast<QAbstractScrollArea*>(object)) {
         if (!area->isEnabled()) return false;
         HANDLE_SCROLL_AREA_EVENT
         return false;
      }
      else if (Q3ScrollView* area =
               qobject_cast<Q3ScrollView*>(object)) {
         HANDLE_SCROLL_AREA_EVENT
         return false;
      }
      return false;
#undef HANDLE_SCROLL_AREA_EVENT
      
   case QEvent::FocusIn:
      if (qobject_cast<QAbstractButton*>(object) ||
          qobject_cast<QComboBox*>(object)) {
         QWidget *widget = (QWidget*)object;
         if (!widget->isEnabled()) return false;
         if (widget->testAttribute(Qt::WA_UnderMouse))
            widget->repaint();
         else
            animator->fadeIn(widget);
         return false;
      }
      return false;
   case QEvent::FocusOut:
      if (qobject_cast<QAbstractButton*>(object) || 
          qobject_cast<QComboBox*>(object)) {
         QWidget *widget = (QWidget*)object;
         if (!widget->isEnabled()) return false;
         if (widget->testAttribute(Qt::WA_UnderMouse))
            widget->repaint();
         else
            animator->fadeOut((QWidget*)(object));
         return false;
      }
      return false;
   case QEvent::EnabledChange:
      if (QWidget* progress = qobject_cast<QProgressBar*>(object)) {
         if (progress->isEnabled())
            animator->addProgressBar(progress);
         else
            animator->removeProgressBar(progress);
         return false;
      }
      if (QTabWidget* tab = qobject_cast<QTabWidget*>(object)) {
         if (tab->isEnabled())
            animator->addTab(tab, tab->currentIndex());
         else
            animator->removeTab(tab);
         return false;
      }
      return false;
   default:
      return false;
   }
}

void OxygenStyle::unPolish( QApplication */*app */)
{
}

void OxygenStyle::unPolish( QWidget *widget )
{
   if (qobject_cast<QProgressBar*>(widget)) {
      widget->removeEventFilter(this);
      animator->removeProgressBar(widget);
   }
   if (qobject_cast<QAbstractScrollArea*>(widget) || qobject_cast<Q3ScrollView*>(widget))
      widget->removeEventFilter(this);
   if (qobject_cast<VisualFrame*>(widget))
      widget->deleteLater();
//    w->removeEventFilter(this);
//    if (w->isTopLevel() || qobject_cast<QGroupBox*>(w) || w->inherits("KActiveLabel"))
//       w->setPalette(QPalette());
}

QPalette OxygenStyle::standardPalette () const
{
   QPalette pal ( Qt::black, QColor(30,31,32), // windowText, button
                     Qt::white, QColor(200,201,202), QColor(221,222,223), //light, dark, mid
                     Qt::black, Qt::white, //text, bright_text
                     QColor(251,254,255), QColor(234,236,238) ); //base, window
   pal.setColor(QPalette::ButtonText, QColor(234,236,238));
   return pal;
}

/** eventcontrol slots*/
#if 0
void OxygenStyle::fakeMouse()
{
   if (mouseButtonPressed_) // delayed mousepress for move event
   {
      QCursor::setPos ( cursorPos_ );
      XTestFakeButtonEvent(QX11Info::display(),1, false, 0);
      XTestFakeKeyEvent(QX11Info::display(),XKeysymToKeycode(QX11Info::display(), XK_Alt_L), true, 0);
      XTestFakeButtonEvent(QX11Info::display(),1, true, 0);
      XTestFakeKeyEvent(QX11Info::display(),XKeysymToKeycode(QX11Info::display(), XK_Alt_L), false, 0);
      XFlush(QX11Info::display());
   }
}
#endif
