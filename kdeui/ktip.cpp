/*****************************************************************

Copyright (c) 2000, 2001 Matthias Hoelzer-Kluepfel
                         Tobias Koenig <tokoe82@yahoo.de>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

******************************************************************/

#include <qcheckbox.h>
#include <qfile.h>
#include <qhbox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qpushbutton.h>
#include <qtextstream.h>
#include <qimage.h>

#include <kaboutdata.h>
#include <kapplication.h>
#include <kconfig.h>
#include <kdebug.h>
#include <kglobal.h>
#include <kglobalsettings.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kpushbutton.h>
#include <kseparator.h>
#include <kstandarddirs.h>
#include <kstdguiitem.h>
#include <ktextbrowser.h>
#include <kiconeffect.h>
#include <kglobalsettings.h>

#include "ktip.h"


KTipDatabase::KTipDatabase(const QString &_tipFile)
{
    QString tipFile = _tipFile;
    if (tipFile.isEmpty())
	tipFile = QString::fromLatin1(KGlobal::instance()->aboutData()->appName()) + "/tips";

    loadTips(tipFile);

    if (tips.count())
	current = kapp->random() % tips.count();
}


// if you change something here, please update the script
// preparetips, which depends on extracting exactly the same
// text as done here.
void KTipDatabase::loadTips(const QString &tipFile)
{
    QString fileName = locate("data", tipFile);

    if (fileName.isEmpty())
    {
	kdDebug() << "can't find '" << tipFile << "' in standard dirs" << endl;
        return;
    }

    QFile file(fileName);
    if (!file.open(IO_ReadOnly))
    {
	kdDebug() << "can't open '" << fileName << "' for reading" << endl;
	return;
    }

    tips.clear();

    QString content = file.readAll();

    int pos = -1;
    while ((pos = content.find("<html>", pos + 1, false)) != -1)
    {
	QString tip = content.mid(pos + 6, content.find("</html>", pos, false) - pos - 6);
	if (tip.startsWith("\n"))
		tip = tip.mid(1);
	tips.append(tip);
    }

    file.close();    
}

void KTipDatabase::nextTip()
{
    if (tips.count() == 0)
	return ;
    current += 1;
    if (current >= (int) tips.count())
	current = 0;
}


void KTipDatabase::prevTip()
{
    if (tips.count() == 0)
	return ;
    current -= 1;
    if (current < 0)
	current = tips.count() - 1;
}


QString KTipDatabase::tip() const
{
    return tips[current];
}

KTipDialog *KTipDialog::_instance = 0;


KTipDialog::KTipDialog(KTipDatabase *db, QWidget *parent, const char *name)
  : KDialog(parent, name)
{
    /**
     * Parent is 0L when TipDialog is used as a mainWidget. This should
     * be the case only in ktip, so let's use the ktip layout.
     */
    bool isTipDialog = (parent != 0);
 
    QImage img;
    int h,s,v;

    _blendedColor = KGlobalSettings::activeTitleColor();
    _blendedColor.hsv(&h,&s,&v);
    _blendedColor.setHsv(h,s*(71/76.0),v*(67/93.0));

    if (!isTipDialog)
    {
	img = QImage(locate("data", "kdewizard/pics/wizard_small.png"));
	// colorize and check to figure the correct color
	KIconEffect::colorize(img, _blendedColor, 1.0);
	QRgb colPixel( img.pixel(0,0) );

	_blendedColor = QColor(qRed(colPixel),qGreen(colPixel),qBlue(colPixel));
    }

    _baseColor = KGlobalSettings::alternateBackgroundColor();
    _baseColor.hsv(&h,&s,&v);
    _baseColor.setHsv(h,s*(10/6.0),v*(93/99.0));

    _textColor = KGlobalSettings::textColor();
    
	
    _database = db;

    setCaption(i18n("Tip of the Day"));
    setIcon(KGlobal::iconLoader()->loadIcon("ktip", KIcon::Small));

    QVBoxLayout *vbox = new QVBoxLayout(this, marginHint(), spacingHint());

   if (isTipDialog)
    {
	QHBoxLayout *pl = new QHBoxLayout(vbox, 0, 0);

	QLabel *bulb = new QLabel(this);
	bulb->setPixmap(locate("data", "kdeui/pics/ktip-bulb.png"));
	pl->addWidget(bulb);

	QLabel *titlePane = new QLabel(this);
	titlePane->setBackgroundPixmap(locate("data", "kdeui/pics/ktip-background.png"));
	titlePane->setText(i18n("Did you know...?\n"));
	titlePane->setFont(QFont(KGlobalSettings::generalFont().family(), 20, QFont::Bold));
	titlePane->setAlignment(QLabel::AlignCenter);
	pl->addWidget(titlePane, 100);
    }

    QHBox *hbox = new QHBox(this);
    hbox->setSpacing(0);
    hbox->setFrameStyle(QFrame::Panel | QFrame::Sunken);
    vbox->addWidget(hbox);

    QHBox *tl = new QHBox(hbox);
    tl->setMargin(7);
    tl->setBackgroundColor(_blendedColor);

    QHBox *topLeft = new QHBox(tl);
    topLeft->setMargin(15);
    topLeft->setBackgroundColor(_baseColor);

    _tipText = new KTextBrowser(topLeft);
    
    _tipText->mimeSourceFactory()->addFilePath(
	KGlobal::dirs()->findResourceDir("data", "kdewizard/pics")+"kdewizard/pics/");
    _tipText->setFrameStyle(QFrame::NoFrame | QFrame::Plain);
    _tipText->setHScrollBarMode(QScrollView::AlwaysOff);
    _tipText->setLinkUnderline(false);

    QStyleSheet *sheet = _tipText->styleSheet();
    QStyleSheetItem *item = sheet->item("a");
    item->setFontWeight(QFont::Bold);
    _tipText->setStyleSheet(sheet);
    QPalette pal = _tipText->palette();
    pal.setColor( QPalette::Active, QColorGroup::Link, _blendedColor );
    pal.setColor( QPalette::Inactive, QColorGroup::Link, _blendedColor );
    _tipText->setPalette(pal);

    QStringList icons = KGlobal::dirs()->resourceDirs("icon");
    QStringList::Iterator it;
    for (it = icons.begin(); it != icons.end(); ++it)
        _tipText->mimeSourceFactory()->addFilePath(*it);

    if (!isTipDialog)
    {
	QLabel *l = new QLabel(hbox);
	l->setPixmap(img);
	l->setBackgroundColor(_blendedColor);
	l->setAlignment(Qt::AlignRight | Qt::AlignBottom);

	resize(550, 230);
        QSize sh = size();
	QRect rect = KApplication::desktop()->screenGeometry();
        move(rect.x() + (rect.width() - sh.width())/2,
	rect.y() + (rect.height() - sh.height())/2);
    }

    KSeparator* sep = new KSeparator( KSeparator::HLine, this);
    vbox->addWidget(sep);

    QHBoxLayout *hbox2 = new QHBoxLayout(vbox, 4);

    _tipOnStart = new QCheckBox(i18n("&Show tips on startup"), this);
    hbox2->addWidget(_tipOnStart, 1);

    KPushButton *prev = new KPushButton( KStdGuiItem::back( 
            KStdGuiItem::UseRTL ), this );
    prev->setText( i18n("&Previous") );
    hbox2->addWidget(prev);

    KPushButton *next = new KPushButton( KStdGuiItem::forward( 
            KStdGuiItem::UseRTL ), this );
    next->setText( i18n("&Next") );
    hbox2->addWidget(next);

    KPushButton *ok = new KPushButton(KStdGuiItem::close(), this);
    ok->setDefault(true);
    hbox2->addWidget(ok);

    KConfigGroup config(kapp->config(), "TipOfDay");
    _tipOnStart->setChecked(config.readBoolEntry("RunOnStart", true));

    connect(next, SIGNAL(clicked()), this, SLOT(nextTip()));
    connect(prev, SIGNAL(clicked()), this, SLOT(prevTip()));
    connect(ok, SIGNAL(clicked()), this, SLOT(accept()));
    connect(_tipOnStart, SIGNAL(toggled(bool)), this, SLOT(showOnStart(bool)));

    ok->setFocus();

    nextTip();
}


void KTipDialog::showTip(const QString &tipFile, bool force)
{
    showTip(kapp->mainWidget(), tipFile, force);
}

void KTipDialog::showTip(QWidget *parent,const QString &tipFile, bool force)
{
    if (!force)
    {
	KConfigGroup config(kapp->config(), "TipOfDay");
	if (!config.readBoolEntry("RunOnStart", true))
	    return;
    }

    if (!_instance)
	_instance = new KTipDialog(new KTipDatabase(tipFile), parent);

    _instance->nextTip();
    _instance->show();
    _instance->raise();
}

void KTipDialog::prevTip()
{
    _database->prevTip();
    _tipText->setText(QString::fromLatin1(
	 "<qt text=\"%1\" bgcolor=\"%2\">%3</qt>")
	 .arg(_textColor.name())
	 .arg(_baseColor.name())
	 .arg(i18n(_database->tip().utf8())));
}

void KTipDialog::nextTip()
{
    _database->nextTip();
    _tipText->setText(QString::fromLatin1("<qt text=\"%1\" bgcolor=\"%2\">%3</qt>")
			.arg(_textColor.name())
			.arg(_baseColor.name())
			.arg(i18n(_database->tip().utf8())));
}

void KTipDialog::showOnStart(bool on)
{
    setShowOnStart(on);
}

void KTipDialog::setShowOnStart(bool on)
{
    KConfigGroup config(kapp->config(), "TipOfDay");
    config.writeEntry("RunOnStart", on);
    config.sync();
}

bool KTipDialog::eventFilter(QObject *o, QEvent *e)
{
	if (o == _tipText && e->type()== QEvent::KeyPress &&
		(((QKeyEvent *)e)->key() == Key_Return ||
		((QKeyEvent *)e)->key() == Key_Space ))
		accept();
		    
	// If the user presses Return or Space, we close the dialog as if the
	// default button was pressed even if the KTextBrowser has the keyboard
	// focus. This could have the bad side-effect that the user cannot use the
	// keyboard to open urls in the KTextBrowser, so we just let it handle
	// the key event _additionally_. (Antonio)

	return QWidget::eventFilter( o, e );
}
				
void KTipDialog::virtual_hook( int id, void* data )
{
	KDialog::virtual_hook( id, data );
}
				
#include "ktip.moc"
