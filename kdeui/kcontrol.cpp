/*
  kcontrol - Base for KDE Control Applications

  written 1997 by Matthias Hoelzer

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
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/


#include <klocale.h>
#include "kcontrol.h"

static int minimum_width_;

KControlDialog::KControlDialog()
  : QTabDialog(0, 0, FALSE)
{
  // Create help button
  helpBtn = new QPushButton(i18n("Help"), this);
  helpBtn->resize(helpBtn->sizeHint());
  helpBtn->move(7, height() - helpBtn->height() - 6);


  // Create default button
  defaultBtn = new QPushButton(i18n("Default"), this);
  defaultBtn->resize(defaultBtn->sizeHint());
  defaultBtn->move(helpBtn->width()+16, height() - defaultBtn->height() - 6);

  // set the default buttons
  setOKButton(i18n("OK"));
  setApplyButton(i18n("Apply"));
  setCancelButton(i18n("Cancel"));

  //geometry hack.
  defaultBtn->setText(i18n("OK"));
  int w = defaultBtn->sizeHint().width();
  defaultBtn->setText(i18n("Apply"));
  w  = QMAX(w, defaultBtn->sizeHint().width());
  defaultBtn->setText(i18n("Cancel"));
  w  = QMAX(w, defaultBtn->sizeHint().width());

  defaultBtn->setText(i18n("Default"));


  minimum_width_ = w*3+20+ defaultBtn->width() + 30 + helpBtn->width();
}

void KControlDialog::done(int result)
{
  hide();
  setResult(result);
  kapp->quit();
}


void KControlDialog::resizeEvent(QResizeEvent *event)
{
  QTabDialog::resizeEvent(event);

  if (helpBtn)
    helpBtn->move(7, height() - helpBtn->height() - 6);
  if (defaultBtn)
    defaultBtn->move(16+helpBtn->width(), height() - helpBtn->height() - 6);
}


KControlApplication::KControlApplication(int &argc, char **argv, const QString &name)
  : KApplication(argc, argv, name)
{
  dialog = 0;
  pages = 0;

  // called to initialize?
  if (argc == 2 && strcmp("-init", argv[1]) == 0)
    {
      init();
      justInit = TRUE;
      return;
    }

  // run the setup dialog
  justInit = FALSE;

  // create setup dialog
  dialog = new KControlDialog();
  if (!dialog)
    return;
  // Stephan: this is not possible (or better: not practical)
  // dialog->setCaption(i18n(title));

  // connect the buttons
  connect(dialog, SIGNAL(applyButtonPressed()), this, SLOT(apply()));
  connect(dialog->helpBtn, SIGNAL(pressed()), this, SLOT(help()));
  connect(dialog->defaultBtn, SIGNAL(pressed()), this, SLOT(defaultValues()));

  // set dialog as main widget
  setMainWidget(dialog);

  // detect, if swallowing
  int start=1;
  if (argc >= 3 && strcmp(argv[1],"-swallow") == 0)
    {
      swallowCaption = argv[2];
      start = 3;
    }

  dialog->setCaption( swallowCaption );

  // parse the command line parameters, if any
  if (argc > start)
    {
      pages = new QStrList();
      if (pages)
	{
	  for (int i=start; i<argc; i++)
	    pages->append(argv[i]);
	}
    }

}


void KControlApplication::setTitle(const QString& title)
{
  if (dialog && swallowCaption.isEmpty())
      dialog->setCaption(title);
}

KControlApplication::~KControlApplication()
{
  if (dialog)
    delete dialog;
  if (pages)
    delete pages;
}


void KControlApplication::addPage(QWidget *page, const QString &name, const QString &help_name)
{
  if (dialog)
    {
      dialog->addTab(page, name);
      helpNames.append(help_name.data());
      // set the default size
      dialog->resize(QMAX(dialog->sizeHint().width(), minimum_width_), dialog->height());
    }
}


void KControlApplication::help()
{
  QString name("index.html");

  if (dialog)
    name = helpNames.at(dialog->tabBar()->currentTab());

  kapp->invokeHTMLHelp(QString("kcontrol/")+kapp->appName()+"/"+name, "");
}

#include "kcontrol.moc"
