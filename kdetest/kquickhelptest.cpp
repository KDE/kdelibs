#include <kapp.h>
#include <qdialog.h>
#include <kquickhelp.h>
#include <qlayout.h>
#include <kbuttonbox.h>
#include <qlabel.h>
#include <qpushbt.h>
#include <stdlib.h>

#define SIZE(w)  w->setFrameStyle(QFrame::Raised|QFrame::Panel); w->setMinimumSize(w->sizeHint());

int main(int argc, char **argv) {
  KApplication a(argc, argv, "kquickhelptest");

  QDialog *dlg = new QDialog(0, 0, true);
  dlg->setCaption("QuickHelp - Demo");

  QGridLayout *tl = new QGridLayout(dlg, 6, 2, 10);
  QLabel *l;
  l = new QLabel("Right click to show simple help", dlg);
  SIZE(l);
  tl->addWidget(l, 0, 0);
  KQuickHelp::add(l, "This is a very simple, tooltip like Quickhelp");

  l = new QLabel("Colors", dlg);
  SIZE(l);
  tl->addWidget(l, 1, 0);
  KQuickHelp::add(l, "Different colors:<br><i+><red>red\n<green>green\n<blue>blue<black><i->");

  l = new QLabel("Font sizes", dlg);
  SIZE(l);
  tl->addWidget(l, 2, 0);
  KQuickHelp::add(l, "normal<+> bigger <+> bigger <+> huge <+> monumenthal");

  l = new QLabel("Font styles", dlg);
  SIZE(l);
  tl->addWidget(l, 3, 0);
  KQuickHelp::add(l, "<b><red>Red<black> bold text</b>\n<i>Italic text</i>\n<u>Underlined</u>");

  l = new QLabel("RGB colors", dlg);
  SIZE(l);
  tl->addWidget(l, 4, 0);
  KQuickHelp::add(l, "<color #ff0000>This is pure red\n<color #808080>Gray text\n<color #e37726>Strange color");

  l = new QLabel("Weblinks", dlg);
  SIZE(l);
  tl->addWidget(l, 0, 1);
  KQuickHelp::add(l, "<+>Click <link http://www.kde.org>here</link> to go\nto the KDE homepage!");

  l = new QLabel("FTP - links", dlg);
  SIZE(l);
  tl->addWidget(l, 1, 1);
  KQuickHelp::add(l, "<+>Click <link ftp://ftp.kde.org>here</link> to go\nto the KDE FTP server!");

  l = new QLabel("Local links", dlg);
  SIZE(l);
  tl->addWidget(l, 2, 1);
  QString h(getenv("HOME"));
  QString s;
  s.sprintf("<+>Click <link file:%s>here</link> to open\nyour home directory!",
	    h.data());
  KQuickHelp::add(l, s.data());

  l = new QLabel("Links to the application HTML help", dlg);
  SIZE(l);
  tl->addWidget(l, 3, 1);
  KQuickHelp::add(l, "<link #anchor>This</link> would open the applications\nHTML help, however I'm too lazy to\nwrite one!");

  KButtonBox *bbox = new KButtonBox(dlg);
  bbox->addStretch(1);
  dlg->connect(bbox->addButton("Close"), SIGNAL(clicked()),
	       dlg, SLOT(accept()));
  bbox->layout();
  tl->addMultiCellWidget(bbox, 5, 5, 0, 1);

  tl->freeze();

  dlg->exec();
  return 0;
}
