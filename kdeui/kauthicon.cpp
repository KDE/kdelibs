/*
 * KAuthIcon - an icon which shows whether privileges are in effect
 * Part of the KDE Project
 * Copyright (c) 1999 Preston Brown <pbrown@kde.org>
 */

#include <unistd.h>

#include <qlayout.h>
#include <qtimer.h>

#include <klocale.h>

#include "kauthicon.h"
#include "kauthicon.moc"

/* XPM */
static const char * lock_xpm[] = {
"22 22 5 1",
"       c None",
".      c #808080",
"+      c #000000",
"@      c #FFFFFF",
"#      c #C0C0C0",
"                      ",
"                      ",
"                      ",
"                      ",
"        .+++.         ",
"        .@@@.+        ",
"      ..@+++@..       ",
"      +@+...+@+       ",
"      +@+.  +@+.      ",
"      +@+.  +@+.      ",
"     +++++++++++      ",
"     +#########+.     ",
"     +#.......#+.     ",
"     +#@@@@@@@#+.     ",
"     +#.......#+.     ",
"     +#########+.     ",
"     +++++++++++.     ",
"      ...........     ",
"                      ",
"                      ",
"                      ",
"                      "};

/* XPM */
static const char * openlock_xpm[] = {
"22 22 5 1",
"       c None",
".      c #808080",
"+      c #000000",
"@      c #FFFFFF",
"#      c #C0C0C0",
"                      ",
"                      ",
"        .+++.         ",
"        .@@@.+        ",
"      ..@+++@..       ",
"      +@+...+@+       ",
"      +@+.  +@+.      ",
"      +@+.  +@+.      ",
"      +++.  +@+.      ",
"       ...  +@+.      ",
"            +@+.      ",
"     +++++++++++      ",
"     +#########+.     ",
"     +#.......#+.     ",
"     +#@@@@@@@#+.     ",
"     +#.......#+.     ",
"     +#########+.     ",
"     +++++++++++.     ",
"      ...........     ",
"                      ",
"                      ",
"                      "};

KAuthIcon::KAuthIcon(QWidget *parent, const char *name)
  : QWidget(parent, name), lockPM(lock_xpm), openLockPM(openlock_xpm)
{
  lockText = i18n("Editing disabled");
  openLockText = i18n("Editing enabled");

  lockBox = new QLabel(this);
  lockBox->setFrameStyle(QFrame::WinPanel|QFrame::Raised);
  lockBox->setPixmap(lockPM);
  lockBox->setFixedSize(lockBox->sizeHint());

  lockLabel = new QLabel(this);
  lockLabel->setFrameStyle(QFrame::NoFrame);

  // set fixed size of this frame to whichever phrase is longer
  if (lockLabel->fontMetrics().boundingRect(lockText).width() >
      lockLabel->fontMetrics().boundingRect(openLockText).width())
    lockLabel->setText(lockText);
  else
    lockLabel->setText(openLockText);
  lockLabel->setAlignment(AlignCenter);
  lockLabel->setFixedSize(lockLabel->sizeHint());
  lockLabel->setText(lockText);

  topLayout = new QGridLayout(this, 3, 5);

  topLayout->setRowStretch(0, 1);
  topLayout->setRowStretch(2, 1);

  topLayout->setColStretch(0, 1);
  topLayout->addColSpacing(2, 5);
  topLayout->setColStretch(4, 1);

  topLayout->addWidget(lockBox, 1, 1, AlignLeft|AlignVCenter);
  topLayout->addWidget(lockLabel, 1, 3, AlignRight|AlignVCenter);

  resize(sizeHint());
  topLayout->activate();
}

KAuthIcon::~KAuthIcon()
{
}


QSize KAuthIcon::sizeHint() const
{
  return topLayout->minimumSize();
}


/************************************************************************/

KRootPermsIcon::KRootPermsIcon(QWidget *parent, const char *name)
  : KAuthIcon(parent, name)
{
  updateStatus();
}


KRootPermsIcon::~KRootPermsIcon()
{
}

void KRootPermsIcon::updateStatus()
{
  bool newRoot;
  newRoot = (geteuid() == 0);
  lockBox->setPixmap(newRoot ? openLockPM : lockPM);
  lockLabel->setText(newRoot ? openLockText : lockText);
  update();
  if (root != newRoot) {
    root = newRoot;
    emit authChanged(newRoot);
  }
}

/************************************************************************/

KWritePermsIcon::KWritePermsIcon(QString fileName,
				 QWidget *parent, const char *name)
  : KAuthIcon(parent, name)
{
  fi.setFile(fileName);
  updateStatus();
}


KWritePermsIcon::~KWritePermsIcon()
{
}

void KWritePermsIcon::updateStatus()
{
  bool newwrite;
  newwrite = fi.isWritable();
  lockBox->setPixmap(newwrite ? openLockPM : lockPM);
  lockLabel->setText(newwrite ? openLockText : lockText);
  update();
  if (writable != newwrite) {
    writable = newwrite;
    emit authChanged(newwrite);
  }
}
