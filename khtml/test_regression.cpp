// programm to test the new khtml implementation


#include <stdlib.h>
#include <kapp.h>
#include <qfile.h>
// to be able to delete a static protected member pointer in kbrowser...
// just for memory debugging
#define protected public
#undef protected

#include "test_regression.h"

#include <stdio.h>

#include "css/cssstyleselector.h"
#include <dom_string.h>
#include "rendering/render_style.h"

#include <kaction.h>
#include <kcmdlineargs.h>
#include <khtml_factory.h>
#include <kio/job.h>
#include <kmainwindow.h>

#include <qcolor.h>
#include <qcursor.h>
#include <qdir.h>
#include <qobject.h>
#include <qpushbutton.h>
#include <qpushbutton.h>
#include <qscrollview.h>
#include <qstring.h>
#include <qtextstream.h>
#include <qvaluelist.h>
#include <qwidget.h>
#include <qfileinfo.h>

#include "decoder.h"
#include "dom/dom2_range.h"
#include "dom/dom_exception.h"
#include "dom/html_document.h"
#include "html_document.h"
#include "htmltokenizer.h"
#include "khtml_part.h"
#include "khtmlview.h"
#include "rendering/render_object.h"
#include "xml/dom_docimpl.h"
#include "dom/dom_doc.h"
#include "misc/loader.h"


using namespace khtml;
using namespace DOM;

static KCmdLineOptions options[] = { { "+source_dir", "Directory containing html files to prcoess", 0 } ,
				     { "+result_dir", "Directory for comparison/storage of processing results", 0 } ,
				     {0, 0, 0}, };

int main(int argc, char *argv[])
{

    KCmdLineArgs::init(argc, argv, "test_regression", "Regression tester for khtml", "1.0");
    KCmdLineArgs::addCmdLineOptions(options);

    KApplication a;
    KCmdLineArgs *args = KCmdLineArgs::parsedArgs( );
    if ( args->count() < 2 ) {
	KCmdLineArgs::usage();
	::exit( 1 );
    }

    QFileInfo sourceDir(args->arg(0));
    if (!sourceDir.exists() || !sourceDir.isDir() || !sourceDir.isDir()) {
	fprintf(stderr,"ERROR: Source directory \"%s\": no such directory.\n",args->arg(0));
	exit(1);
    }

    QFileInfo resultDir(args->arg(0));
    if (!resultDir.exists() || !resultDir.isDir() || !resultDir.isDir()) {
	fprintf(stderr,"ERROR: Result directory \"%s\": no such directory.\n",args->arg(1));
	exit(1);
    }

    // create widgets
    KHTMLFactory *fac = new KHTMLFactory();
    KMainWindow *toplevel = new KMainWindow();
    KHTMLPart *part = new KHTMLPart( toplevel, 0, toplevel, 0, KHTMLPart::BrowserViewGUI );

    toplevel->setCentralWidget( part->widget() );
    toplevel->resize( 800, 600);
    part->setJScriptEnabled(true);
//    part->setJavaEnabled(true);

    a.setTopWidget(part->widget());

    // run the tests
    RegressionTest *regressionTest = new RegressionTest(part,
							args->arg(0),
							args->arg(1));
    regressionTest->runTests();
    delete regressionTest;


    // cleanup
    delete part;
    delete toplevel;
    delete fac;

    khtml::Cache::clear();
    khtml::CSSStyleSelector::clear();
    khtml::RenderStyle::cleanup();

    return 0;
}

RegressionTest::RegressionTest(KHTMLPart *part, QString _sourceFilesDir, QString _resultFilesDir) : QObject(part)
{
    m_part = part;

    m_sourceFilesDir = _sourceFilesDir;
    m_resultFilesDir = _resultFilesDir;
};

void RegressionTest::runTests()
{
    QDir sourceDir(m_sourceFilesDir);
    for (uint fileno = 0; fileno < sourceDir.count(); fileno++) {
	QString filename = sourceDir[fileno];
	if (filename.findRev(".html") == int(filename.length())-5) {
	    testFile(filename);
	}
    }
}

bool RegressionTest::testFile(QString filename)
{
    // load in test file and write to part

    QString fullSourceName = m_sourceFilesDir+"/"+filename;
    QFile sourceFile(fullSourceName);
    m_part->begin(KURL(fullSourceName));
    if (!sourceFile.open(IO_ReadOnly)) {
	fprintf(stderr,"Error reading file %s\n",fullSourceName.ascii());
	exit(-1);
    }


    char buf[1024];
    int bytesread;
    while ((bytesread = sourceFile.readBlock(buf,1024)) > 0) {
	m_part->write(buf,bytesread);
    }
    sourceFile.close();
    m_part->end();

    // dump out the contents of the rendering & DOM trees
    QByteArray array;
    QString fullResultName = m_resultFilesDir+"/"+filename+"-result";
    QFile resultFile(fullResultName);
    if (!resultFile.open(IO_WriteOnly)) {
	fprintf(stderr,"Error writing to file %s\n",fullResultName.ascii());
	exit(-1);
    }

    QTextStream outputStream(&resultFile);

    outputStream << "Rendering Tree" << endl;
    outputStream << "--------------" << endl;
    m_part->document().handle()->renderer()->dump(&outputStream);
    outputStream << "DOM Tree" << endl;
    outputStream << "--------" << endl;
    m_part->document().handle()->dump(&outputStream);
    resultFile.close();
}

bool RegressionTest::checkFile(QString filename, QByteArray data)
{
    QFile file(filename);
    if (!file.open(IO_ReadOnly)) {
	fprintf(stderr,"Error reading file %s\n",filename.ascii());
	exit(-1);
    }

    QByteArray fileData;
    QTextStream stream(fileData,IO_WriteOnly);

    char buf[1024];
    int bytesread;
    while ((bytesread = file.readBlock(buf,1024)) > 0)
	stream.writeRawBytes(buf,bytesread);

    file.close();

    return (fileData == data);
}

#include "test_regression.moc"
