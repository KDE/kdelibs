#! /usr/bin/env bash
# Must be run in the source directory
kdelibsdir=`pwd`
cd ../qt-copy/src
find . -name "*.cpp" | fgrep -v moc_ > list
for file in qfiledialog qcolordialog qprintdialog \
	qurloperator qftp qhttp qlocal qerrormessage; do
	fgrep -v $file list > list.new && mv list.new list
done
# Important: tr has a comment in second position that KDE uses as context
#  but QT_TRANSLATE_NOOP has the context at first position
xgettext --omit-header -C --qt -ktr:1,1t -ktr:1,2c,2t -kQT_TRANSLATE_NOOP:1,1t -kQT_TRANSLATE_NOOP:1c,2,2t --files-from=list -o$kdelibsdir/qt-messages.pot
cd $kdelibsdir
