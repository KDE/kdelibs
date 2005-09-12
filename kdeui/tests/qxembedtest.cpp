#include <stdlib.h>
#include <stdio.h>

#include "qapplication.h"
#include "qpushbutton.h"
#include "qlineedit.h"
#include "q3hbox.h"
#include "q3vbox.h"
#include "qxembed.h"
#include <QX11Info>

WId windowWithName(const char *);


int
main(int argc, char**argv)
{
  if (argc != 2)
    {
      fprintf(stderr,
              "usage: qxembedtest [qtoptions] windowid\n"
              "       qxembedtest [qtoptions] windowTitle\n");
      exit(10);
    }


  QApplication a(argc,argv);

#ifdef Q_WS_X11
  QWidget *main = new Q3VBox(NULL,"main",Qt::WDestructiveClose);
  QWidget *top = new Q3HBox(main);
  QPushButton *quit = new QPushButton("Quit", top);
  QObject::connect( quit, SIGNAL(clicked()), main, SLOT(close()) );
  QLineEdit *edit = new QLineEdit(top);
  edit->setText( "Just to see focus changes");
  QXEmbed *embed = new QXEmbed(main);
  embed->setProtocol(QXEmbed::XPLAIN);
  a.setMainWidget(main);
  main->show();

  WId wid = strtol(argv[1], NULL, 0);
  if (! wid)
    wid = windowWithName(argv[1]);
  if (! wid)
    {
      fprintf(stderr,"qxembedtest: window not found\n");
      exit(10);
    }

  fprintf(stderr,"qxembedtest: embedding wid=0x%08x\n", (unsigned int)wid);

  embed->embed(wid);

  return a.exec();
#else
  return 0;
#endif
}



#ifdef Q_WS_X11

#include <X11/Xlib.h>
#include <qpaintdevice.h>

// This is lifted from X11 xprop.

Window Window_With_Name(Display *dpy, Window top, const char *name)
{
  Window *children, dummy;
  unsigned int nchildren;
  Window w=0;
  char *window_name;
  if (XFetchName(dpy, top, &window_name) && !strcmp(window_name, name))
    return(top);
  if (!XQueryTree(dpy, top, &dummy, &dummy, &children, &nchildren))
    return(0);
  for (unsigned int i=0; i<nchildren; i++) {
    w = Window_With_Name(dpy, children[i], name);
    if (w)
      break;
  }
  if (children)
    XFree ((char *)children);
  return(w);
}


WId windowWithName(const char *name)
{
  return Window_With_Name(QX11Info::display(), QX11Info::appRootWindow(), name);
}

#endif
