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

#include "k3aboutdialog.h"
#include "k3aboutdialog_p.h"
#include <kdeversion.h>
#include <QtGui/QClipboard>
#include <QtGui/QImage>
#include <QtGui/QLabel>
#include <QtCore/QDebug>
#include <QtGui/QLayout>
#include <QtCore/QObject>
#include <QtGui/QPainter>
#include <QtCore/QRect>
#include <QtGui/QTabWidget>
#include <QtGui/QTabBar>
#include <QScrollArea>
#include <QtGui/QApplication>
#include <QtGui/QTextBrowser>
#include <QtGui/QStyle>

#include <kglobal.h>
#include <kglobalsettings.h>
#include <klocale.h>
#include <kiconloader.h>
#include <kdebug.h>
#include <ktextedit.h>

#ifndef Q_MOC_RUN
template class QList<K3AboutContributor *>;
#endif

#define WORKTEXT_IDENTATION 16
#define Grid 3

// ##############################################################
// MOC OUTPUT FILES:
#include "k3aboutdialog.moc"
#include "k3aboutdialog_p.moc"
// ##############################################################

class K3AboutContributor::Private
{
  public:
    Private(K3AboutContributor *_parent)
      : parent(_parent)
    {
    }

    K3AboutContributor *parent;
    QLabel *label[4];
    QLabel *text[4];
    bool showHeader;
    bool showBold;

    void updateLayout();
};

void K3AboutContributor::Private::updateLayout()
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

K3AboutContributor::K3AboutContributor(QWidget *_parent,
                                      const QString &_name,const QString &_email,
                                      const QString &_url, const QString &_work,
                                      bool showHeader, bool showFrame,
                                      bool showBold)
  : QFrame(_parent), d(new Private(this))
{
  if (showFrame)
    setFrameStyle(QFrame::Panel | QFrame::Plain);

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

K3AboutContributor::~K3AboutContributor()
{
  delete d;
}

void K3AboutContributor::setName(const QString &_text, const QString &_header,
                                 bool _update)
{
  d->label[0]->setText(_header);
  d->text[0]->setText(_text);
  if (_update) { d->updateLayout(); }
}


void K3AboutContributor::setEmail(const QString &_text, const QString &_header,
                                  bool _update)
{
  if (_text.isEmpty())
      return;
  d->label[1]->setText(_header);
  d->text[1]->setText(QString("<a href=\"mailto:%1\">%1</a>").arg(_text));
  if (_update) { d->updateLayout(); }
}

void K3AboutContributor::setUrl(const QString &_text, const QString &_header,
                                bool _update)
{
    if (_text.isEmpty())
      return;
  d->label[2]->setText(_header);
  d->text[2]->setText(QString("<a href=\"%1\">%1</a>").arg(_text));
  if (_update) { d->updateLayout(); }
}

void K3AboutContributor::setWork(const QString &_text, const QString &_header,
                                 bool _update)
{
  d->label[3]->setText(_header);
  d->text[3]->setText(_text);
  if (_update) { d->updateLayout(); }
}

QString K3AboutContributor::name() const
{
  return d->text[0]->text();
}

QString K3AboutContributor::email() const
{
  return d->text[1]->text();
}

QString K3AboutContributor::url() const
{
  return d->text[2]->text();
}

QString K3AboutContributor::work() const
{
  return d->text[3]->text();
}

void K3AboutContributor::fontChange(const QFont &/*oldFont*/)
{
  if (d->showBold)
  {
    QFont f(font());
    f.setBold(true);
    d->text[0]->setFont(f);
  }
  update();
}

QSize K3AboutContributor::sizeHint(void) const
{
  return minimumSizeHint();
}

//
// Internal widget for the K3AboutDialog class.
//
K3AboutContainerBase::K3AboutContainerBase(int layoutType, QWidget *_parent)
  : QWidget(_parent),
    mImageLabel(0), mTitleLabel(0), mIconLabel(0),mVersionLabel(0),
    mAuthorLabel(0), mImageFrame(0),mPageTab(0),mPlainSpace(0),d(0)
{
  mTopLayout = new QVBoxLayout(this);
  mTopLayout->setMargin(0);
  mTopLayout->setSpacing(KDialog::spacingHint());

  if (layoutType & K3AboutDialog::ImageOnly)
  {
    layoutType &= ~(K3AboutDialog::ImageLeft|K3AboutDialog::ImageRight|K3AboutDialog::Tabbed|K3AboutDialog::Plain);
  }
  if (layoutType & K3AboutDialog::ImageLeft)
  {
    layoutType &= ~K3AboutDialog::ImageRight;
  }

  if (layoutType & K3AboutDialog::Title)
  {
    mTitleLabel = new QLabel(this);
    mTitleLabel->setObjectName("title");
    mTitleLabel->setAlignment(Qt::AlignCenter);
    mTopLayout->addWidget(mTitleLabel);
    mTopLayout->addSpacing(KDialog::spacingHint());
  }

  if (layoutType & K3AboutDialog::Product)
  {
      QFrame* const productArea = new  QFrame(this);
      productArea->setFrameStyle( QFrame::StyledPanel );
      productArea->setFrameShadow( QFrame::Plain );
      productArea->setBackgroundRole(QPalette::Base);
      productArea->setAutoFillBackground(true);


      QHBoxLayout* const hbox = new QHBoxLayout();
      hbox->setMargin(3);
      hbox->setSpacing(KDialog::spacingHint());

      mIconLabel = new QLabel(productArea);
      hbox->addWidget(mIconLabel, 0, Qt::AlignLeft|Qt::AlignHCenter);

      QVBoxLayout* const vbox = new QVBoxLayout();
      if (!vbox) { return; }

      mVersionLabel = new QLabel(productArea);
      mVersionLabel->setObjectName("version");
      mAuthorLabel  = new QLabel(productArea);
      mAuthorLabel->setObjectName("author");
      vbox->addWidget(mVersionLabel);
      vbox->addWidget(mAuthorLabel);
      vbox->setMargin(3);

      hbox->addLayout(vbox);

      if ( QApplication::isRightToLeft() )
        hbox->insertStretch(0  /* add stretch at start */ , 1);
      else 
        hbox->insertStretch(-1 /* add stretch at end   */ , 1);

      productArea->setLayout(hbox);

      mTopLayout->addWidget(productArea, 0); 
  }

  QHBoxLayout* const hbox = new QHBoxLayout();
  if (!hbox) { return; }
  mTopLayout->addLayout(hbox, 10);

  if (layoutType & K3AboutDialog::ImageLeft)
  {
    QVBoxLayout* vbox = new QVBoxLayout();
    hbox->addLayout(vbox);
    vbox->addSpacing(1);
    mImageFrame = new QFrame(this);
    mImageFrame->setAutoFillBackground(true);
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

  if (layoutType & K3AboutDialog::Tabbed)
  {
    mPageTab = new QTabWidget(this);
    if (!mPageTab) { return; }
    mPageTab->setUsesScrollButtons(false);
    hbox->addWidget(mPageTab, 10);
  }
  else if (layoutType & K3AboutDialog::ImageOnly)
  {
    mImageFrame = new QFrame(this);
    mImageFrame->setAutoFillBackground(true);
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

  if (layoutType & K3AboutDialog::ImageRight)
  {
    QVBoxLayout *vbox = new QVBoxLayout();
    hbox->addLayout(vbox);
    vbox->addSpacing(1);
    mImageFrame = new QFrame(this);
    mImageFrame->setAutoFillBackground(true);
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

QSize K3AboutContainerBase::sizeHint() const
{
    return minimumSize().expandedTo(QSize(QWidget::sizeHint().width(), 0));
}

void K3AboutContainerBase::fontChange(const QFont &/*oldFont*/)
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
    mVersionLabel->setFont(f);
    mAuthorLabel->setFont(f);
    mVersionLabel->parentWidget()->layout()->activate();
  }

  update();
}

QFrame *K3AboutContainerBase::addTextPage(const QString &title,
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
    browser->setFrameStyle( QFrame::NoFrame );
    browser->setOpenExternalLinks(true);
    browser->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    browser->setHtml(text);
    browser->setMinimumHeight(fontMetrics().lineSpacing()*numLines);

    vbox->addWidget(browser);
  }
  else
  {
    KTextEdit *const textEdit = new KTextEdit(page);
    textEdit->setFrameStyle( QFrame::NoFrame );
    textEdit->setObjectName("text");
    textEdit->setReadOnly(true);
    textEdit->setMinimumHeight(fontMetrics().lineSpacing()*numLines);
    textEdit->setLineWrapMode(QTextEdit::NoWrap);
    vbox->addWidget(textEdit);
  }

  return page;
}

QFrame *K3AboutContainerBase::addLicensePage(const QString &title,
					  const QString &text, int numLines)
{
  QFrame *const page = addEmptyPage(title);
  if (!page) { return 0; }
  if (numLines <= 0) { numLines = 10; }

  QVBoxLayout* const vbox = new QVBoxLayout(page);
  vbox->setSpacing(KDialog::spacingHint());

  vbox->setMargin(0);

  KTextEdit* const textEdit = new KTextEdit(page);
  textEdit->setFrameStyle(QFrame::NoFrame);
  textEdit->setObjectName("license");
  textEdit->setFont(KGlobalSettings::fixedFont());
  textEdit->setReadOnly(true);
  textEdit->setLineWrapMode(QTextEdit::NoWrap);
  textEdit->setPlainText(text);
  textEdit->setMinimumHeight(fontMetrics().lineSpacing()*numLines);
  vbox->addWidget(textEdit);
  return page;
}


K3AboutContainer *K3AboutContainerBase::addContainerPage(const QString &title,
							Qt::Alignment childAlignment,
							Qt::Alignment innerAlignment)
{
  if (!mPageTab)
  {
    kDebug(291) << "addPage: " << "Invalid layout";
    return 0;
  }

  K3AboutContainer* const container = new K3AboutContainer(0,
    KDialog::spacingHint(), KDialog::spacingHint(), childAlignment,
						  innerAlignment);
  mPageTab->addTab(container, title);
  return container;
}


K3AboutContainer *K3AboutContainerBase::addScrolledContainerPage(
				      const QString &title,
				      Qt::Alignment childAlignment,
				      Qt::Alignment innerAlignment)
{
  if (!mPageTab)
  {
    kDebug(291) << "addPage: " << "Invalid layout";
    return 0;
  }

  QFrame *const page = addEmptyPage(title);
  QVBoxLayout* const vbox = new QVBoxLayout(page);
  vbox->setSpacing(KDialog::spacingHint());
  vbox->setMargin(0);

  QScrollArea* const scrollView = new QScrollArea(page);
  scrollView->setFrameStyle(QFrame::Plain);
  vbox->addWidget(scrollView);

  K3AboutContainer* const container = new K3AboutContainer(scrollView,
    KDialog::spacingHint(), KDialog::spacingHint(), childAlignment,
    innerAlignment);
  scrollView->setWidget(container);
  return container;
}


QFrame *K3AboutContainerBase::addEmptyPage(const QString &title)
{
  if (!mPageTab)
  {
    kDebug(291) << "addPage: " << "Invalid layout";
    return 0;
  }

  QFrame *const page = new QFrame();
  page->setObjectName(title.toLatin1());
  page->setFrameStyle(QFrame::NoFrame);

  mPageTab->addTab(page, title);
  return page;
}


K3AboutContainer *K3AboutContainerBase::addContainer(Qt::Alignment childAlignment,
						    Qt::Alignment innerAlignment)
{
  K3AboutContainer* const container = new K3AboutContainer(this,
    0, KDialog::spacingHint(), childAlignment, innerAlignment);
  container->setObjectName("container");
  mTopLayout->addWidget(container, 0, childAlignment);
  return container;
}



void K3AboutContainerBase::setTitle(const QString &title)
{
  if (!mTitleLabel)
  {
    kDebug(291) << "setTitle: " << "Invalid layout";
    return;
  }
  mTitleLabel->setText(title);
}


void K3AboutContainerBase::setImage(const QString &fileName)
{
  if (!mImageLabel)
  {
    kDebug(291) << "setImage: " << "Invalid layout";
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

void K3AboutContainerBase::setProgramLogo(const QString &fileName)
{
  if (fileName.isNull())
  {
    return;
  }

  const QPixmap logo(fileName);
  setProgramLogo(logo);
}

void K3AboutContainerBase::setProgramLogo(const QPixmap &pixmap)
{
  if (!mIconLabel)
  {
    kDebug(291) << "setProgramLogo: " << "Invalid layout";
    return;
  }
  if (!pixmap.isNull())
  {
    mIconLabel->setPixmap(pixmap);
  }
}

void K3AboutContainerBase::setImageBackgroundColor(const QColor &color)
{
  if (mImageFrame)
  {
    QPalette palette(mImageFrame->palette());
    palette.setColor(QPalette::Window, color);
    mImageFrame->setPalette(palette);
  }
}


void K3AboutContainerBase::setImageFrame(bool state)
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


void K3AboutContainerBase::setProduct(const QString &appName,
				      const QString &version,
				      const QString &author,
				      const QString &year)
{
  if (!mIconLabel)
  {
    kDebug(291) << "setProduct: " << "Invalid layout";
    return;
  }

  int size = IconSize(KIconLoader::Desktop);
  mIconLabel->setPixmap(qApp->windowIcon().pixmap(size,size));

  const QString msg1 = i18n("<html><font size=\"5\">%1</font><br/><b>version %2</b><br/>Using KDE %3</html>", appName, version,
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

class K3AboutContainer::Private
{
  public:
    QVBoxLayout *vbox;
    Qt::Alignment alignment;
};

K3AboutContainer::K3AboutContainer(QWidget *parent,
                                  int margin, int spacing,
                                  Qt::Alignment childAlignment,
                                  Qt::Alignment innerAlignment)
  : QFrame(parent), d(new Private)
{
  d->alignment = innerAlignment;

  setFrameStyle(QFrame::NoFrame);

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

K3AboutContainer::~K3AboutContainer()
{
  delete d;
}

QSize K3AboutContainer::sizeHint(void) const
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


QSize K3AboutContainer::minimumSizeHint(void) const
{
  return sizeHint();
}

void K3AboutContainer::addWidget(QWidget *widget)
{
  widget->setParent(this);

  d->vbox->addWidget(widget, 0, d->alignment);
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

void K3AboutContainer::addPerson(const QString &_name, const QString &_email,
				 const QString &_url, const QString &_task,
				 bool showHeader, bool showFrame,bool showBold)
{

  K3AboutContributor* const cont = new K3AboutContributor(this,
    _name, _email, _url, _task, showHeader, showFrame, showBold);
  cont->setObjectName("pers");
  addWidget(cont);
}


void K3AboutContainer::addTitle(const QString &title, Qt::Alignment alignment,
				bool showFrame, bool showBold)
{

  QLabel* const label = new QLabel(title, this);
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
  addWidget(label);
}


void K3AboutContainer::addImage(const QString &fileName, Qt::Alignment alignment)
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
  addWidget(label);
}

K3AboutWidget::K3AboutWidget(QWidget *_parent)
  : QWidget(_parent),
    version(new QLabel(this)),
    cont(new QLabel(this)),
    logo(new QLabel(this)),
    author(new K3AboutContributor(this)),
    maintainer(new K3AboutContributor(this)),
    showMaintainer(false),
    d(0)
{
  cont->setText(i18n("Other Contributors:"));
  logo->setText(i18n("(No logo available)"));
  logo->setFrameStyle(QFrame::Panel | QFrame::Raised);
  version->setAlignment(Qt::AlignCenter);
}


void K3AboutWidget::adjust()
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
      foreach(K3AboutContributor* currEntry, contributors)
	{
	  cy+=currEntry->sizeHint().height();
	}
    }
  // -----
  setMinimumSize(cx, cy);
}

void K3AboutWidget::setLogo(const QPixmap& i)
{
  logo->setPixmap(i);
}

void K3AboutWidget::setAuthor(const QString &_name, const QString &_email,
			const QString &_url, const QString &_w)
{
  author->setName(_name);
  author->setEmail(_email);
  author->setUrl(_url);
  author->setWork(_w);
}

void K3AboutWidget::setMaintainer(const QString &_name, const QString &_email,
			    const QString &_url, const QString &_w)
{
  maintainer->setName(_name);
  maintainer->setEmail(_email);
  maintainer->setWork(_w);
  maintainer->setUrl(_url);
  showMaintainer=true;
}

void K3AboutWidget::addContributor(const QString &_name, const QString &_email,
			     const QString &_url, const QString &_w)
{
  K3AboutContributor* const c=new K3AboutContributor(this);
  // -----
  c->setName(_name);
  c->setEmail(_email);
  c->setUrl(_url);
  c->setWork(_w);
  contributors.append(c);
}

void K3AboutWidget::setVersion(const QString &_name)
{
  version->setText(_name);
}

void K3AboutWidget::resizeEvent(QResizeEvent*)
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
  foreach(K3AboutContributor *currEntry, contributors)
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

K3AboutDialog::K3AboutDialog(QWidget *_parent)
  : KDialog(_parent),
    mAbout(new K3AboutWidget(this)), mContainerBase(0), d(0)
{
  setButtons(Ok);
  setModal(true);
  setMainWidget(mAbout);
}

K3AboutDialog::K3AboutDialog(int layoutType, const QString &_caption, QWidget *_parent)
  : KDialog(_parent),
    mAbout(0), d(0)
{
  setModal(true);
  setPlainCaption(i18n("About %1", _caption));

  mContainerBase = new K3AboutContainerBase(layoutType, this);
  setMainWidget(mContainerBase);
}


void K3AboutDialog::show(void)
{
  adjust();
  if (mContainerBase) { mContainerBase->show(); }
  QDialog::show();
}


void K3AboutDialog::show(QWidget * /*centerParent*/)
{
  adjust();
  if (mContainerBase) { mContainerBase->show(); }
  QDialog::show();
}

void K3AboutDialog::adjust()
{
  if (!mAbout)
    return;

  mAbout->adjust();
  resize(sizeHint());
}

void K3AboutDialog::setLogo(const QPixmap& i)
{
  if (!mAbout)
    return;

  mAbout->setLogo(i);
}

void K3AboutDialog::setMaintainer(const QString &_name, const QString &_email,
				 const QString &_url, const QString &_w)
{
  if (!mAbout)
    return;

  mAbout->setMaintainer(_name, _email, _url, _w);
}

void K3AboutDialog::setAuthor(const QString &_name, const QString &_email,
			     const QString &_url, const QString &_work)
{
  if (!mAbout)
    return;

  mAbout->setAuthor(_name, _email, _url, _work);
}

void K3AboutDialog::addContributor(const QString &_name, const QString &_email,
				  const QString &_url, const QString &_w)
{
  if (!mAbout)
    return;

  mAbout->addContributor(_name, _email, _url, _w);
}

void K3AboutDialog::setVersion(const QString &_name)
{
  if (!mAbout)
    return;

  mAbout->setVersion(_name);
}

QFrame *K3AboutDialog::addTextPage(const QString &title, const QString &text,
				   bool richText, int numLines)
{
  if (!mContainerBase) { return 0; }
  return mContainerBase->addTextPage(title, text, richText, numLines);
}

QFrame *K3AboutDialog::addLicensePage(const QString &title, const QString &text,
				   int numLines)
{
  if (!mContainerBase) { return 0; }
  return mContainerBase->addLicensePage(title, text, numLines);
}

K3AboutContainer *K3AboutDialog::addContainerPage(const QString &title,
				  Qt::Alignment childAlignment, Qt::Alignment innerAlignment)
{
  if (!mContainerBase) { return 0; }
  return mContainerBase->addContainerPage(title, childAlignment,
					    innerAlignment);
}

K3AboutContainer *K3AboutDialog::addScrolledContainerPage(const QString &title,
				  Qt::Alignment childAlignment, Qt::Alignment innerAlignment)
{
  if (!mContainerBase) { return 0; }
  return mContainerBase->addScrolledContainerPage(title, childAlignment,
						    innerAlignment);
}

QFrame *K3AboutDialog::addPage(const QString &title)
{
  if (!mContainerBase) { return 0; }
  return mContainerBase->addEmptyPage(title);
}


K3AboutContainer *K3AboutDialog::addContainer(Qt::Alignment childAlignment,
					     Qt::Alignment innerAlignment)
{
  if (!mContainerBase) { return 0; }
  return mContainerBase->addContainer(childAlignment, innerAlignment);
}


void K3AboutDialog::setTitle(const QString &title)
{
  if (!mContainerBase) { return; }
  mContainerBase->setTitle(title);
}

void K3AboutDialog::setImage(const QString &fileName)
{
  if (!mContainerBase) { return; }
  mContainerBase->setImage(fileName);
}

void K3AboutDialog::setProgramLogo(const QString &fileName)
{
  if (!mContainerBase) { return; }
  mContainerBase->setProgramLogo(fileName);
}

void K3AboutDialog::setProgramLogo(const QPixmap &pixmap)
{
  if (!mContainerBase) { return; }
  mContainerBase->setProgramLogo(pixmap);
}

void K3AboutDialog::setImageBackgroundColor(const QColor &color)
{
  if (!mContainerBase) { return; }
  mContainerBase->setImageBackgroundColor(color);
}

void K3AboutDialog::setImageFrame(bool state)
{
  if (!mContainerBase) { return; }
  mContainerBase->setImageFrame(state);
}

void K3AboutDialog::setProduct(const QString &appName, const QString &version,
			       const QString &author, const QString &year)
{
  if (!mContainerBase) { return; }
  mContainerBase->setProduct(appName, version, author, year);
}

void K3AboutDialog::imageUrl(QWidget *_parent, const QString &_caption,
			     const QString &_path, const QColor &_imageColor,
			     const QString &_url)
{
  K3AboutDialog a(ImageOnly, QString(), _parent);
  a.setButtons(Close);
  a.setDefaultButton(Close);
  a.setObjectName("image");
  a.setPlainCaption(_caption);
  a.setImage(_path);
  a.setImageBackgroundColor(_imageColor);

  K3AboutContainer* const c = a.addContainer(Qt::AlignCenter, Qt::AlignCenter);
  if (c)
  {
    c->addPerson(QString(), QString(), _url, QString());
  }
  a.exec();
}

