/*
 *  This file is part of the KDE Libraries
 *  Copyright (C) 1999-2001 Mirko Boehm <mirko@kde.org> and
 *  Espen Sand <espensa@online.no>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 *
 */

#include "kaboutdialog.h"
#include "kaboutdialog_p.h"
#include <kdeversion.h>
#include <qclipboard.h>
#include <qimage.h>
#include <qlabel.h>
#include <qdebug.h>
#include <qlayout.h>
#include <qobject.h>
#include <qpainter.h>
#include <qrect.h>
#include <qtabwidget.h>
#include <qtabbar.h>
#include <QScrollArea>
#include <qapplication.h>
#include <qtextbrowser.h>
#include <qstyle.h>

#include <kglobal.h>
#include <kglobalsettings.h>
#include <klocale.h>
#include <qlabel.h>
#include <kiconloader.h>
#include <kdebug.h>
#include <ktextedit.h>

//MOC_SKIP_BEGIN
template class QList<KAboutContributor *>;
//MOC_SKIP_END

#define WORKTEXT_IDENTATION 16
#define Grid 3

// ##############################################################
// MOC OUTPUT FILES:
#include "kaboutdialog.moc"
#include "kaboutdialog_p.moc"
// ##############################################################

class KAboutContributor::Private
{
  public:
    Private(KAboutContributor *_parent)
      : parent(_parent)
    {
    }

    KAboutContributor *parent;
    QLabel *label[4];
    QLabel *text[4];
    bool showHeader;
    bool showBold;

    void updateLayout();
};

void KAboutContributor::Private::updateLayout()
{
  delete parent->layout();

  int row = 0;
  if (!text[0]->text().isEmpty()) { ++row; }
  if (!text[1]->text().isEmpty()) { ++row; }
  if (!text[2]->text().isEmpty()) { ++row; }
  if (!text[3]->text().isEmpty()) { ++row; }

  QGridLayout *gbox;
  if (row == 0)
  {
    gbox = new QGridLayout(parent);
    gbox->setSpacing(1);
    for (int i=0; i<4; ++i)
    {
      label[i]->hide();
      text[i]->hide();
    }
  }
  else
  {
    if (text[0]->text().isEmpty() && !showHeader)
    {
      gbox = new QGridLayout(parent);
      gbox->setMargin(parent->frameWidth()+1);
      gbox->setSpacing(2);
    }
    else
    {
      gbox = new QGridLayout(parent);
      gbox->setMargin(parent->frameWidth()+1);
      gbox->setSpacing(2);
      if (!showHeader)
      {
        gbox->addItem(new QSpacerItem(KDialog::spacingHint()*2, 0), 0, 0);
      }
      gbox->setColumnStretch(1, 10);
    }

    for (int i=0, r=0; i<4; ++i)
    {
      label[i]->setFixedHeight(parent->fontMetrics().lineSpacing());
      if (i != 3)
      {
        text[i]->setFixedHeight(parent->fontMetrics().lineSpacing());
      }

      if (!text[i]->text().isEmpty())
      {
        if (showHeader)
        {
          gbox->addWidget(label[i], r, 0, Qt::AlignLeft);
          gbox->addWidget(text[i], r, 1, Qt::AlignLeft );
          label[i]->show();
          text[i]->show();
        }
        else
        {
          label[i]->hide();
          if (!i)
          {
            gbox->addWidget(text[i], r, 0, 1, 2, Qt::AlignLeft);
          }
          else
          {
            gbox->addWidget(text[i], r, 1, Qt::AlignLeft );
          }
          text[i]->show();
        }
        ++r;
      }
      else
      {
        label[i]->hide();
        text[i]->hide();
      }
    }
  }

  gbox->activate();
  parent->setMinimumSize(parent->sizeHint());
}

KAboutContributor::KAboutContributor(QWidget *_parent,
                                      const QString &_name,const QString &_email,
                                      const QString &_url, const QString &_work,
                                      bool showHeader, bool showFrame,
                                      bool showBold)
  : QFrame(_parent), d(new Private(this))
{
  if (showFrame)
    setFrameStyle(QFrame::Panel | QFrame::Raised);

  d->showHeader = showHeader;
  d->showBold = showBold;
  for (int i=0; i < 4; ++i) {
    d->label[i] = new QLabel(this);
    d->label[i]->setOpenExternalLinks(true);
    d->label[i]->setTextInteractionFlags(Qt::LinksAccessibleByMouse);
    d->text[i] = new QLabel(this);
    d->text[i]->setOpenExternalLinks(true);
    d->text[i]->setTextInteractionFlags(Qt::LinksAccessibleByMouse);
  }

  setName(_name, i18n("Author"), false);
  setEmail(_email, i18n("Email"), false);
  setUrl(_url, i18n("Homepage"), false);
  setWork(_work, i18n("Task"), false);

  d->label[3]->setAlignment(Qt::AlignTop);

  fontChange(font());
  d->updateLayout();
}

KAboutContributor::~KAboutContributor()
{
  delete d;
}

void KAboutContributor::setName(const QString &_text, const QString &_header,
                                 bool _update)
{
  d->label[0]->setText(_header);
  d->text[0]->setText(_text);
  if (_update) { d->updateLayout(); }
}


void KAboutContributor::setEmail(const QString &_text, const QString &_header,
                                  bool _update)
{
  if (_text.isEmpty())
      return;
  d->label[1]->setText(_header);
  d->text[1]->setText(QString("<a href=\"mailto:%1\">%1</a>").arg(_text));
  if (_update) { d->updateLayout(); }
}

void KAboutContributor::setUrl(const QString &_text, const QString &_header,
                                bool _update)
{
    if (_text.isEmpty())
      return;
  d->label[2]->setText(_header);
  d->text[2]->setText(QString("<a href=\"%1\">%1</a>").arg(_text));
  if (_update) { d->updateLayout(); }
}

void KAboutContributor::setWork(const QString &_text, const QString &_header,
                                 bool _update)
{
  d->label[3]->setText(_header);
  d->text[3]->setText(_text);
  if (_update) { d->updateLayout(); }
}

QString KAboutContributor::name() const
{
  return d->text[0]->text();
}

QString KAboutContributor::email() const
{
  return d->text[1]->text();
}

QString KAboutContributor::url() const
{
  return d->text[2]->text();
}

QString KAboutContributor::work() const
{
  return d->text[3]->text();
}

void KAboutContributor::fontChange(const QFont &/*oldFont*/)
{
  if (d->showBold)
  {
    QFont f(font());
    f.setBold(true);
    d->text[0]->setFont(f);
  }
  update();
}

QSize KAboutContributor::sizeHint(void) const
{
  return minimumSizeHint();
}

//
// Internal widget for the KAboutDialog class.
//
KAboutContainerBase::KAboutContainerBase(int layoutType, QWidget *_parent)
  : QWidget(_parent),
    mImageLabel(0), mTitleLabel(0), mIconLabel(0),mVersionLabel(0),
    mAuthorLabel(0), mImageFrame(0),mPageTab(0),mPlainSpace(0),d(0)
{
  mTopLayout = new QVBoxLayout(this);
  mTopLayout->setMargin(0);
  mTopLayout->setSpacing(KDialog::spacingHint());
  if (!mTopLayout) { return; }

  if (layoutType & KAboutDialog::ImageOnly)
  {
    layoutType &= ~(KAboutDialog::ImageLeft|KAboutDialog::ImageRight|KAboutDialog::Tabbed|KAboutDialog::Plain);
  }
  if (layoutType & KAboutDialog::ImageLeft)
  {
    layoutType &= ~KAboutDialog::ImageRight;
  }

  if (layoutType & KAboutDialog::Title)
  {
    mTitleLabel = new QLabel(this);
    mTitleLabel->setObjectName("title");
    mTitleLabel->setAlignment(Qt::AlignCenter);
    mTopLayout->addWidget(mTitleLabel);
    mTopLayout->addSpacing(KDialog::spacingHint());
  }

  if (layoutType & KAboutDialog::Product)
  {
      QWidget* const productArea = new  QWidget(this);
      mTopLayout->addWidget(productArea, 0, QApplication::isRightToLeft() ? Qt::AlignRight : Qt::AlignLeft);

      QHBoxLayout* const hbox = new QHBoxLayout(productArea);
      hbox->setMargin(0);
      hbox->setSpacing(KDialog::spacingHint());

      mIconLabel = new QLabel(productArea);
      hbox->addWidget(mIconLabel, 0, Qt::AlignLeft|Qt::AlignHCenter);

      QVBoxLayout* const vbox = new QVBoxLayout();
      if (!vbox) { return; }
      hbox->addLayout(vbox);

      mVersionLabel = new QLabel(productArea);
      mVersionLabel->setObjectName("version");
      mAuthorLabel  = new QLabel(productArea);
      mAuthorLabel->setObjectName("author");
      vbox->addWidget(mVersionLabel);
      vbox->addWidget(mAuthorLabel);
      hbox->activate();

      mTopLayout->addSpacing(KDialog::spacingHint());
  }

  QHBoxLayout* const hbox = new QHBoxLayout();
  if (!hbox) { return; }
  mTopLayout->addLayout(hbox, 10);

  if (layoutType & KAboutDialog::ImageLeft)
  {
    QVBoxLayout* vbox = new QVBoxLayout();
    hbox->addLayout(vbox);
    vbox->addSpacing(1);
    mImageFrame = new QFrame(this);
    setImageFrame(true);
    vbox->addWidget(mImageFrame);
    vbox->addSpacing(1);

    vbox = new QVBoxLayout(mImageFrame);
    vbox->setSpacing(1);
    mImageLabel = new QLabel(mImageFrame);
    vbox->addStretch(10);
    vbox->addWidget(mImageLabel);
    vbox->addStretch(10);
    vbox->activate();
  }

  if (layoutType & KAboutDialog::Tabbed)
  {
    mPageTab = new QTabWidget(this);
    if (!mPageTab) { return; }
    hbox->addWidget(mPageTab, 10);
  }
  else if (layoutType & KAboutDialog::ImageOnly)
  {
    mImageFrame = new QFrame(this);
    setImageFrame(true);
    hbox->addWidget(mImageFrame, 10);

    QGridLayout* const gbox = new QGridLayout(mImageFrame);
    gbox->setMargin(1);
    gbox->setSpacing(0);
    gbox->setRowStretch(0, 10);
    gbox->setRowStretch(2, 10);
    gbox->setColumnStretch(0, 10);
    gbox->setColumnStretch(2, 10);

    mImageLabel = new QLabel(mImageFrame);
    gbox->addWidget(mImageLabel, 1, 1);
    gbox->activate();
  }
  else
  {
    mPlainSpace = new QFrame(this);
    if (!mPlainSpace) { return; }
    hbox->addWidget(mPlainSpace, 10);
  }

  if (layoutType & KAboutDialog::ImageRight)
  {
    QVBoxLayout *vbox = new QVBoxLayout();
    hbox->addLayout(vbox);
    vbox->addSpacing(1);
    mImageFrame = new QFrame(this);
    setImageFrame(true);
    vbox->addWidget(mImageFrame);
    vbox->addSpacing(1);

    vbox = new QVBoxLayout(mImageFrame);
    vbox->setSpacing(1);
    mImageLabel = new QLabel(mImageFrame);
    vbox->addStretch(10);
    vbox->addWidget(mImageLabel);
    vbox->addStretch(10);
    vbox->activate();
  }

  fontChange(font());
}

QSize KAboutContainerBase::sizeHint() const
{
    return minimumSize().expandedTo(QSize(QWidget::sizeHint().width(), 0));
}

void KAboutContainerBase::fontChange(const QFont &/*oldFont*/)
{
  if (mTitleLabel)
  {
    QFont f(KGlobalSettings::generalFont());
    f.setBold(true);
    int fs = f.pointSize();
    if (fs == -1)
       fs = QFontInfo(f).pointSize();
    f.setPointSize(fs+2); // Lets not make it too big
    mTitleLabel->setFont(f);
  }

  if (mVersionLabel)
  {
    QFont f(KGlobalSettings::generalFont());
    f.setBold(true);
    mVersionLabel->setFont(f);
    mAuthorLabel->setFont(f);
    mVersionLabel->parentWidget()->layout()->activate();
  }

  update();
}

QFrame *KAboutContainerBase::addTextPage(const QString &title,
					  const QString &text,
					  bool richText, int numLines)
{
  QFrame *const page = addEmptyPage(title);
  if (!page) { return 0; }
  if (numLines <= 0) { numLines = 10; }

  QVBoxLayout* const vbox = new QVBoxLayout(page);
  vbox->setSpacing(KDialog::spacingHint());

  if (richText)
  {
    QTextBrowser *browser = new QTextBrowser(page);
    browser->setOpenExternalLinks(true);
    browser->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    browser->setHtml(text);
    browser->setMinimumHeight(fontMetrics().lineSpacing()*numLines);

    vbox->addWidget(browser);
  }
  else
  {
    KTextEdit *const textEdit = new KTextEdit(page);
    textEdit->setObjectName("text");
    textEdit->setReadOnly(true);
    textEdit->setMinimumHeight(fontMetrics().lineSpacing()*numLines);
    textEdit->setLineWrapMode(QTextEdit::NoWrap);
    vbox->addWidget(textEdit);
  }

  return page;
}

QFrame *KAboutContainerBase::addLicensePage(const QString &title,
					  const QString &text, int numLines)
{
  QFrame *const page = addEmptyPage(title);
  if (!page) { return 0; }
  if (numLines <= 0) { numLines = 10; }

  QVBoxLayout* const vbox = new QVBoxLayout(page);
  vbox->setSpacing(KDialog::spacingHint());

  KTextEdit* const textEdit = new KTextEdit(page);
  textEdit->setObjectName("license");
  textEdit->setFont(KGlobalSettings::fixedFont());
  textEdit->setReadOnly(true);
  textEdit->setLineWrapMode(QTextEdit::NoWrap);
  textEdit->setPlainText(text);
  textEdit->setMinimumHeight(fontMetrics().lineSpacing()*numLines);
  vbox->addWidget(textEdit);
  return page;
}


KAboutContainer *KAboutContainerBase::addContainerPage(const QString &title,
							Qt::Alignment childAlignment,
							Qt::Alignment innerAlignment)
{
  if (!mPageTab)
  {
    kDebug(291) << "addPage: " << "Invalid layout" << endl;
    return 0;
  }

  KAboutContainer* const container = new KAboutContainer(0,
    KDialog::spacingHint(), KDialog::spacingHint(), childAlignment,
						  innerAlignment);
  mPageTab->addTab(container, title);
  return container;
}


KAboutContainer *KAboutContainerBase::addScrolledContainerPage(
				      const QString &title,
				      Qt::Alignment childAlignment,
				      Qt::Alignment innerAlignment)
{
  if (!mPageTab)
  {
    kDebug(291) << "addPage: " << "Invalid layout" << endl;
    return 0;
  }

  QFrame *const page = addEmptyPage(title);
  QVBoxLayout* const vbox = new QVBoxLayout(page);
  vbox->setSpacing(KDialog::spacingHint());
  QScrollArea* const scrollView = new QScrollArea(page);
  vbox->addWidget(scrollView);

  KAboutContainer* const container = new KAboutContainer(scrollView,
    KDialog::spacingHint(), KDialog::spacingHint(), childAlignment,
    innerAlignment);
  scrollView->setWidget(container);
  return container;
}


QFrame *KAboutContainerBase::addEmptyPage(const QString &title)
{
  if (!mPageTab)
  {
    kDebug(291) << "addPage: " << "Invalid layout" << endl;
    return 0;
  }

  QFrame *const page = new QFrame();
  page->setObjectName(title.toLatin1());
  page->setFrameStyle(QFrame::NoFrame);

  mPageTab->addTab(page, title);
  return page;
}


KAboutContainer *KAboutContainerBase::addContainer(Qt::Alignment childAlignment,
						    Qt::Alignment innerAlignment)
{
  KAboutContainer* const container = new KAboutContainer(this,
    0, KDialog::spacingHint(), childAlignment, innerAlignment);
  container->setObjectName("container");
  mTopLayout->addWidget(container, 0, childAlignment);
  return container;
}



void KAboutContainerBase::setTitle(const QString &title)
{
  if (!mTitleLabel)
  {
    kDebug(291) << "setTitle: " << "Invalid layout" << endl;
    return;
  }
  qDebug() << "KAboutContainerBase::setTitl" << title;
  mTitleLabel->setText(title);
}


void KAboutContainerBase::setImage(const QString &fileName)
{
  if (!mImageLabel)
  {
    kDebug(291) << "setImage: " << "Invalid layout" << endl;
    return;
  }
  if (fileName.isNull())
  {
    return;
  }

  const QPixmap logo(fileName);
  if (!logo.isNull())
    mImageLabel->setPixmap(logo);

  mImageFrame->layout()->activate();
}

void KAboutContainerBase::setProgramLogo(const QString &fileName)
{
  if (fileName.isNull())
  {
    return;
  }

  const QPixmap logo(fileName);
  setProgramLogo(logo);
}

void KAboutContainerBase::setProgramLogo(const QPixmap &pixmap)
{
  if (!mIconLabel)
  {
    kDebug(291) << "setProgramLogo: " << "Invalid layout" << endl;
    return;
  }
  if (!pixmap.isNull())
  {
    mIconLabel->setPixmap(pixmap);
  }
}

void KAboutContainerBase::setImageBackgroundColor(const QColor &color)
{
  if (mImageFrame)
  {
    QPalette palette(mImageFrame->palette());
    palette.setColor(QPalette::Window, color);
    mImageFrame->setPalette(palette);
  }
}


void KAboutContainerBase::setImageFrame(bool state)
{
  if (mImageFrame)
  {
    if (state)
    {
      mImageFrame->setFrameStyle(QFrame::Panel | QFrame::Sunken);
      mImageFrame->setLineWidth(1);
    }
    else
    {
      mImageFrame->setFrameStyle(QFrame::NoFrame);
      mImageFrame->setLineWidth(0);
    }
  }
}


void KAboutContainerBase::setProduct(const QString &appName,
				      const QString &version,
				      const QString &author,
				      const QString &year)
{
  if (!mIconLabel)
  {
    kDebug(291) << "setProduct: " << "Invalid layout" << endl;
    return;
  }

  int size = IconSize(K3Icon::Desktop);
  mIconLabel->setPixmap(qApp->windowIcon().pixmap(size,size));

  const QString msg1 = i18n("%1 %2 (Using KDE %3)", appName, version,
    QLatin1String(KDE_VERSION_STRING));
  const QString msg2 = !year.isEmpty() ? i18n("%1 %2, %3", QChar(0xA9), year,
    author) : QLatin1String("");

  //if (!year.isEmpty())
  //  msg2 = i18n("%1 %2, %3").arg(QChar(0xA9)).arg(year).arg(author);

  mVersionLabel->setText(msg1);
  mAuthorLabel->setText(msg2);
  if (msg2.isEmpty())
  {
    mAuthorLabel->hide();
  }

  mIconLabel->parentWidget()->layout()->activate();
}

class KAboutContainer::Private
{
  public:
    QVBoxLayout *vbox;
    Qt::Alignment alignment;
};

KAboutContainer::KAboutContainer(QWidget *parent,
                                  int margin, int spacing,
                                  Qt::Alignment childAlignment,
                                  Qt::Alignment innerAlignment)
  : QFrame(parent), d(new Private)
{
  d->alignment = innerAlignment;

  QGridLayout* const gbox = new QGridLayout(this);
  gbox->setMargin(margin);
  gbox->setMargin(spacing);
  if (childAlignment & Qt::AlignHCenter)
  {
    gbox->setColumnStretch(0, 10);
    gbox->setColumnStretch(2, 10);
  }
  else if (childAlignment & Qt::AlignRight)
  {
    gbox->setColumnStretch(0, 10);
  }
  else
  {
    gbox->setColumnStretch(2, 10);
  }

  if (childAlignment & Qt::AlignVCenter)
  {
    gbox->setRowStretch(0, 10);
    gbox->setRowStretch(2, 10);
  }
  else if (childAlignment & Qt::AlignRight)
  {
    gbox->setRowStretch(0, 10);
  }
  else
  {
    gbox->setRowStretch(2, 10);
  }

  d->vbox = new QVBoxLayout();
  d->vbox->setSpacing(spacing);
  gbox->addLayout(d->vbox, 1, 1);
  gbox->activate();
}

KAboutContainer::~KAboutContainer()
{
  delete d;
}

void KAboutContainer::childEvent(QChildEvent *e)
{
  if (!e->added() || !e->child()->isWidgetType())
  {
    return;
  }

  QWidget* const w = static_cast<QWidget *>(e->child());
  d->vbox->addWidget(w, 0, d->alignment);
  const QSize s(sizeHint());
  setMinimumSize(s);

  const QList<QObject*> l = children(); // silence please
  foreach (QObject *o, l) {
	if (o->isWidgetType())
	{
        static_cast<QWidget *>(o)->setMinimumWidth(s.width());
	}
  }
}


QSize KAboutContainer::sizeHint(void) const
{
  //
  // The size is computed by adding the sizeHint().height() of all
  // widget children and taking the width of the widest child and adding
  // layout()->margin() and layout()->spacing()
  //

  QSize total_size;

  int numChild = 0;
  const QList<QObject*> l = children(); // silence please
  foreach (QObject *o, l) {
	  if (o->isWidgetType())
	  {
		  ++numChild;
		  QWidget* const w= static_cast<QWidget *>(o);

		  QSize s = w->minimumSize();
		  if (s.isEmpty())
		  {
			  s = w->minimumSizeHint();
			  if (s.isEmpty())
			  {
				  s = w->sizeHint();
				  if (s.isEmpty())
				  {
					  s = QSize(100, 100); // Default size
				  }
			  }
		  }
		  total_size.setHeight(total_size.height() + s.height());
		  if (s.width() > total_size.width()) { total_size.setWidth(s.width()); }
	  }
  }

  if (numChild > 0)
  {
    //
    // Seems I have to add 1 to the height to properly show the border
    // of the last entry if layout()->margin() is 0
    //

    total_size.setHeight(total_size.height() + layout()->spacing()*(numChild-1));
    total_size += QSize(layout()->margin()*2, layout()->margin()*2 + 1);
  }
  else
  {
    total_size = QSize(1, 1);
  }
  return total_size;
}


QSize KAboutContainer::minimumSizeHint(void) const
{
  return sizeHint();
}

void KAboutContainer::addWidget(QWidget *widget)
{
  widget->setParent(this);
  widget->move(QPoint(0,0));
}

void KAboutContainer::addPerson(const QString &_name, const QString &_email,
				 const QString &_url, const QString &_task,
				 bool showHeader, bool showFrame,bool showBold)
{

  KAboutContributor* const cont = new KAboutContributor(this,
    _name, _email, _url, _task, showHeader, showFrame, showBold);
  cont->setObjectName("pers");
}


void KAboutContainer::addTitle(const QString &title, Qt::Alignment alignment,
				bool showFrame, bool showBold)
{

  QLabel* const label = new QLabel(title, this);
  qDebug() << "addTitle" << title;
  label->setObjectName("title");
  if (showBold )
  {
    QFont labelFont(font());
    labelFont.setBold(true);
    label->setFont(labelFont);
  }
  if (showFrame)
    label->setFrameStyle(QFrame::Panel | QFrame::Raised);

  label->setAlignment(alignment);
}


void KAboutContainer::addImage(const QString &fileName, Qt::Alignment alignment)
{
  if (fileName.isNull())
  {
    return;
  }

  QLabel* const label = new QLabel(this);
  label->setObjectName("image");
  const QImage logo(fileName);
  if (!logo.isNull())
  {
    QPixmap pix;
    pix = QPixmap::fromImage(logo, 0);
    label->setPixmap(pix);
  }
  label->setAlignment(alignment);
}

KAboutWidget::KAboutWidget(QWidget *_parent)
  : QWidget(_parent),
    version(new QLabel(this)),
    cont(new QLabel(this)),
    logo(new QLabel(this)),
    author(new KAboutContributor(this)),
    maintainer(new KAboutContributor(this)),
    showMaintainer(false),
    d(0)
{
  cont->setText(i18n("Other Contributors:"));
  logo->setText(i18n("(No logo available)"));
  logo->setFrameStyle(QFrame::Panel | QFrame::Raised);
  version->setAlignment(Qt::AlignCenter);
}


void KAboutWidget::adjust()
{
  int cx, cy, tempx;
  int maintWidth, maintHeight;
  QSize total_size;
  // -----
  if (showMaintainer)
    {
      total_size=maintainer->sizeHint();
      maintWidth=total_size.width();
      maintHeight=total_size.height();
    } else {
      maintWidth=0;
      maintHeight=0;
    }
  total_size=author->sizeHint();
  logo->adjustSize();
  cy=version->sizeHint().height()+Grid;
  cx=logo->width();
  tempx=qMax(total_size.width(), maintWidth);
  cx+=Grid+tempx;
  cx=qMax(cx, version->sizeHint().width());
  cy+=qMax(logo->height(),
	   total_size.height()+(showMaintainer ? Grid+maintHeight : 0));
  // -----
  if (!contributors.isEmpty())
    {
      cx=qMax(cx, cont->sizeHint().width());
      cy+=cont->sizeHint().height()+Grid;
      foreach(KAboutContributor* currEntry, contributors)
	{
	  cy+=currEntry->sizeHint().height();
	}
    }
  // -----
  setMinimumSize(cx, cy);
}

void KAboutWidget::setLogo(const QPixmap& i)
{
  logo->setPixmap(i);
}

void KAboutWidget::setAuthor(const QString &_name, const QString &_email,
			const QString &_url, const QString &_w)
{
  author->setName(_name);
  author->setEmail(_email);
  author->setUrl(_url);
  author->setWork(_w);
}

void KAboutWidget::setMaintainer(const QString &_name, const QString &_email,
			    const QString &_url, const QString &_w)
{
  maintainer->setName(_name);
  maintainer->setEmail(_email);
  maintainer->setWork(_w);
  maintainer->setUrl(_url);
  showMaintainer=true;
}

void KAboutWidget::addContributor(const QString &_name, const QString &_email,
			     const QString &_url, const QString &_w)
{
  KAboutContributor* const c=new KAboutContributor(this);
  // -----
  c->setName(_name);
  c->setEmail(_email);
  c->setUrl(_url);
  c->setWork(_w);
  contributors.append(c);
}

void KAboutWidget::setVersion(const QString &_name)
{
  version->setText(_name);
}

void KAboutWidget::resizeEvent(QResizeEvent*)
{
  int _x=0, _y, cx, tempx, tempy;
  // ----- set version label geometry:
  version->setGeometry(0, 0, width(), version->sizeHint().height());
  _y=version->height()+Grid;
  // ----- move logo to correct position:
  logo->adjustSize();
  logo->move(0, _y);
  // ----- move author and maintainer right to it:
  tempx=logo->width()+Grid;
  cx=width()-tempx;
  author->setGeometry
    (tempx, _y, cx, author->sizeHint().height());
  maintainer->setGeometry
    (tempx, _y+author->height()+Grid, cx, maintainer->sizeHint().height());

  _y+=qMax(logo->height(),
	  author->height()+(showMaintainer ? Grid+maintainer->height() : 0));
  // -----
  if (!contributors.isEmpty())
    {
      tempy=cont->sizeHint().height();
      cont->setGeometry(0, _y, width(), tempy);
      cont->show();
      _y+=tempy+Grid;
    } else {
      cont->hide();
    }
  foreach(KAboutContributor *currEntry, contributors)
    {
      tempy=currEntry->sizeHint().height();
      // y+=Grid;
      currEntry->setGeometry(_x, _y, width(), tempy);
      _y+=tempy;
    }
  if (showMaintainer)
    {
      maintainer->show();
    } else {
      maintainer->hide();
    }
}

KAboutDialog::KAboutDialog(QWidget *_parent)
  : KDialog(_parent),
    mAbout(new KAboutWidget(this)), mContainerBase(0), d(0)
{
  setButtons(Ok);
  setModal(true);
  setMainWidget(mAbout);
}

KAboutDialog::KAboutDialog(int layoutType, const QString &_caption, QWidget *_parent)
  : KDialog(_parent),
    mAbout(0), d(0)
{
  setModal(true);
  showButtonSeparator(false);
  setPlainCaption(i18n("About %1", _caption));

  mContainerBase = new KAboutContainerBase(layoutType, this);
  setMainWidget(mContainerBase);
}


void KAboutDialog::show(void)
{
  adjust();
  if (mContainerBase) { mContainerBase->show(); }
  QDialog::show();
}


void KAboutDialog::show(QWidget * /*centerParent*/)
{
  adjust();
  if (mContainerBase) { mContainerBase->show(); }
  QDialog::show();
}

void KAboutDialog::adjust()
{
  if (!mAbout)
    return;

  mAbout->adjust();
  resize(sizeHint());
}

void KAboutDialog::setLogo(const QPixmap& i)
{
  if (!mAbout)
    return;

  mAbout->setLogo(i);
}

void KAboutDialog::setMaintainer(const QString &_name, const QString &_email,
				 const QString &_url, const QString &_w)
{
  if (!mAbout)
    return;

  mAbout->setMaintainer(_name, _email, _url, _w);
}

void KAboutDialog::setAuthor(const QString &_name, const QString &_email,
			     const QString &_url, const QString &_work)
{
  if (!mAbout)
    return;

  mAbout->setAuthor(_name, _email, _url, _work);
}

void KAboutDialog::addContributor(const QString &_name, const QString &_email,
				  const QString &_url, const QString &_w)
{
  if (!mAbout)
    return;

  mAbout->addContributor(_name, _email, _url, _w);
}

void KAboutDialog::setVersion(const QString &_name)
{
  if (!mAbout)
    return;

  mAbout->setVersion(_name);
}

QFrame *KAboutDialog::addTextPage(const QString &title, const QString &text,
				   bool richText, int numLines)
{
  if (!mContainerBase) { return 0; }
  return mContainerBase->addTextPage(title, text, richText, numLines);
}

QFrame *KAboutDialog::addLicensePage(const QString &title, const QString &text,
				   int numLines)
{
  if (!mContainerBase) { return 0; }
  return mContainerBase->addLicensePage(title, text, numLines);
}

KAboutContainer *KAboutDialog::addContainerPage(const QString &title,
				  Qt::Alignment childAlignment, Qt::Alignment innerAlignment)
{
  if (!mContainerBase) { return 0; }
  return mContainerBase->addContainerPage(title, childAlignment,
					    innerAlignment);
}

KAboutContainer *KAboutDialog::addScrolledContainerPage(const QString &title,
				  Qt::Alignment childAlignment, Qt::Alignment innerAlignment)
{
  if (!mContainerBase) { return 0; }
  return mContainerBase->addScrolledContainerPage(title, childAlignment,
						    innerAlignment);
}

QFrame *KAboutDialog::addPage(const QString &title)
{
  if (!mContainerBase) { return 0; }
  return mContainerBase->addEmptyPage(title);
}


KAboutContainer *KAboutDialog::addContainer(Qt::Alignment childAlignment,
					     Qt::Alignment innerAlignment)
{
  if (!mContainerBase) { return 0; }
  return mContainerBase->addContainer(childAlignment, innerAlignment);
}


void KAboutDialog::setTitle(const QString &title)
{
  if (!mContainerBase) { return; }
  mContainerBase->setTitle(title);
}

void KAboutDialog::setImage(const QString &fileName)
{
  if (!mContainerBase) { return; }
  mContainerBase->setImage(fileName);
}

void KAboutDialog::setProgramLogo(const QString &fileName)
{
  if (!mContainerBase) { return; }
  mContainerBase->setProgramLogo(fileName);
}

void KAboutDialog::setProgramLogo(const QPixmap &pixmap)
{
  if (!mContainerBase) { return; }
  mContainerBase->setProgramLogo(pixmap);
}

void KAboutDialog::setImageBackgroundColor(const QColor &color)
{
  if (!mContainerBase) { return; }
  mContainerBase->setImageBackgroundColor(color);
}

void KAboutDialog::setImageFrame(bool state)
{
  if (!mContainerBase) { return; }
  mContainerBase->setImageFrame(state);
}

void KAboutDialog::setProduct(const QString &appName, const QString &version,
			       const QString &author, const QString &year)
{
  if (!mContainerBase) { return; }
  mContainerBase->setProduct(appName, version, author, year);
}

void KAboutDialog::imageUrl(QWidget *_parent, const QString &_caption,
			     const QString &_path, const QColor &_imageColor,
			     const QString &_url)
{
  KAboutDialog a(ImageOnly, QString(), _parent);
  a.setButtons(Close);
  a.setDefaultButton(Close);
  a.setObjectName("image");
  a.setPlainCaption(_caption);
  a.setImage(_path);
  a.setImageBackgroundColor(_imageColor);

  KAboutContainer* const c = a.addContainer(Qt::AlignCenter, Qt::AlignCenter);
  if (c)
  {
    c->addPerson(QString(), QString(), _url, QString());
  }
  a.exec();
}

