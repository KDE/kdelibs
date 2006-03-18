/**
 * This file is part of the KDE project
 *
 * Copyright (C) 2006 Nikolas Zimmermann <zimmermann@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 *
 */

#include <assert.h>
#include <signal.h>

#include <QFile>
#include <QTimer>
#include <QProcess>
#include <QFileInfo>
#include <QTextStream>
#include <QMainWindow>

#include <kiconloader.h>
#include <kmessagebox.h>
#include <ksimpleconfig.h>
#include <kdirselectdialog.h>

#include "test_regression_gui_window.moc"

TestRegressionWindow::TestRegressionWindow(QWidget *parent)
: QMainWindow(parent), m_flags(None), m_runCounter(0), m_testCounter(0), m_lastResult(Unknown),
					   m_activeProcess(0), m_activeTreeItem(0), m_suspended(false), m_justProcessingQueue(false)
{
	m_ui.setupUi(this);

	// Setup actions/connections
	connect(m_ui.actionOnly_run_JS_tests, SIGNAL(toggled(bool)), SLOT(toggleJSTests(bool)));
	connect(m_ui.actionOnly_run_HTML_tests, SIGNAL(toggled(bool)), SLOT(toggleHTMLTests(bool)));
	connect(m_ui.actionDo_not_supress_debug_output, SIGNAL(toggled(bool)), SLOT(toggleDebugOutput(bool)));
	connect(m_ui.actionDo_not_use_Xvfb, SIGNAL(toggled(bool)), SLOT(toggleNoXvfbUse(bool)));
	connect(m_ui.actionSpecify_tests_directory, SIGNAL(triggered(bool)), SLOT(setTestsDirectory()));
	connect(m_ui.actionSpecify_khtml_directory, SIGNAL(triggered(bool)), SLOT(setKHTMLDirectory()));
	connect(m_ui.actionSpecify_output_directory, SIGNAL(triggered(bool)), SLOT(setOutputDirectory()));
	connect(m_ui.actionRun_tests, SIGNAL(triggered(bool)), SLOT(runTests()));

	connect(m_ui.pauseContinueButton, SIGNAL(clicked(bool)), SLOT(pauseContinueButtonClicked()));
	connect(m_ui.saveLogButton, SIGNAL(clicked(bool)), SLOT(saveLogButtonClicked()));

	connect(m_ui.treeWidget, SIGNAL(customContextMenuRequested(const QPoint &)),
			this, SLOT(treeWidgetContextMenuRequested(const QPoint &)));

	// Setup actions' default state
	m_ui.progressBar->setValue(0);
	m_ui.textEdit->setReadOnly(true);
	m_ui.actionRun_tests->setEnabled(false);
	m_ui.pauseContinueButton->setEnabled(false);

	m_ui.treeWidget->headerItem()->setTextAlignment(0, Qt::AlignLeft);
	m_ui.treeWidget->headerItem()->setText(0, i18n("Available Tests: 0"));

	initLegend();

	// Load default values for tests directory/khtml directory...
	KSimpleConfig config("testregressiongui");

	m_testsUrl = KUrl::fromPath(config.readPathEntry("TestsDirectory"));
	m_khtmlUrl = KUrl::fromPath(config.readPathEntry("KHTMLDirectory"));

	initTestsDirectory();
}

TestRegressionWindow::~TestRegressionWindow()
{
	if(m_activeProcess)
	{
		m_activeProcess->kill();

		/* This leads to: 
		 * QProcess object destroyed while process is still running.
		 * Any idea why??
		delete m_activeProcess;
		*/

		m_activeProcess = 0;
	}
}

void TestRegressionWindow::toggleJSTests(bool checked)
{
	if(checked)
		m_flags |= (1 << JSTests);
	else
		m_flags &= (1 << JSTests);
}

void TestRegressionWindow::toggleHTMLTests(bool checked)
{
	if(checked)
		m_flags |= (1 << HTMLTests);
	else
		m_flags &= (1 << HTMLTests);
}

void TestRegressionWindow::toggleDebugOutput(bool checked)
{
	if(checked)
		m_flags |= (1 << DebugOutput);
	else
		m_flags &= (1 << DebugOutput);
}

void TestRegressionWindow::toggleNoXvfbUse(bool checked)
{
	if(checked)
		m_flags |= (1 << NoXvfbUse);
	else
		m_flags &= (1 << NoXvfbUse);
}

void TestRegressionWindow::setTestsDirectory()
{
	m_testsUrl = KDirSelectDialog::selectDirectory(QString(), true /* local only */);
	initTestsDirectory();
}

void TestRegressionWindow::setOutputDirectory()
{
	m_outputUrl = KDirSelectDialog::selectDirectory(QString(), true /* local only */);
}

void TestRegressionWindow::initTestsDirectory()
{
	bool okay = !m_testsUrl.isEmpty();
	if(okay)
	{
		const char *subdirs[] = { "tests", "baseline", "output", "resources" };
		for(int i = 0; i <= 3; i++)
		{
			QFileInfo sourceDir(m_testsUrl.path() + "/" + subdirs[i]);
			if(!sourceDir.exists() || !sourceDir.isDir())
			{
				KMessageBox::error(0, i18n("Please choose a valid 'khtmltests/regression/' directory!"));

				okay = false;
				m_testsUrl = KUrl();
				break;
			}
		}
	}

	if(okay)
	{
		// Clean up...
		m_itemMap.clear();
		m_ignoreMap.clear();
		m_failureMap.clear();
		m_directoryMap.clear();

		m_ui.treeWidget->clear();

		if(!m_khtmlUrl.isEmpty())
			m_ui.actionRun_tests->setEnabled(true);

		// Initialize map (to prevent assert below)...
		m_directoryMap.insert(QString(), QStringList()); 

		// Setup root tree widget item...
		(void) new QTreeWidgetItem(m_ui.treeWidget, QStringList(m_testsUrl.path() + "/tests"));

		// Check for ignore & failure file in root directory...
		QString ignoreFile = m_testsUrl.path() + "/tests/ignore";
		QString failureFile = m_testsUrl.path() + "/tests/KNOWN_FAILURES";

		QStringList ignoreFileList = readListFile(ignoreFile);
		QStringList failureFileList = readListFile(failureFile);

		if(!ignoreFileList.isEmpty())
			m_ignoreMap.insert(QString(), ignoreFileList);

		if(!failureFileList.isEmpty())
			m_failureMap.insert(QString(), failureFileList);

		// Remember directory...
		KSimpleConfig config("testregressiongui");
		config.writePathEntry("TestsDirectory", m_testsUrl.path());

		// Start listing directory...
		KUrl listUrl = m_testsUrl; listUrl.addPath("tests");
		KIO::ListJob *job = KIO::listRecursive(listUrl, false /* no progress */, false /* no hidden files */);

		connect(job, SIGNAL(result(KIO::Job *)), SLOT(directoryListingFinished(KIO::Job *)));

		connect(job, SIGNAL(entries(KIO::Job *, const KIO::UDSEntryList &)),
				this, SLOT(directoryListingResult(KIO::Job *, const KIO::UDSEntryList &)));
	}
}

void TestRegressionWindow::setKHTMLDirectory()
{
	m_khtmlUrl = KDirSelectDialog::selectDirectory(QString(), true /* local only */);

	if(!m_khtmlUrl.isEmpty())
	{
		const char *subdirs[] = { "css", "dom", "xml", "html" }; // That's enough ;-)
		for(int i = 0; i <= 3; i++)
		{
			QFileInfo sourceDir(m_khtmlUrl.path() + "/" + subdirs[i]);
			if(!sourceDir.exists() || !sourceDir.isDir())
			{
				KMessageBox::error(0, i18n("Please choose a valid 'khtml/' build directory!"));

				m_khtmlUrl = KUrl();
				break;
			}
		}

		// Remember directory...
		KSimpleConfig config("testregressiongui");
		config.writePathEntry("KHTMLDirectory", m_khtmlUrl.path());

		if(!m_testsUrl.isEmpty() && !m_khtmlUrl.isEmpty())
			m_ui.actionRun_tests->setEnabled(true);
	}
}

void TestRegressionWindow::directoryListingResult(KIO::Job *, const KIO::UDSEntryList &list)
{
	KIO::UDSEntryList::ConstIterator it = list.constBegin();
	const KIO::UDSEntryList::ConstIterator end = list.constEnd();

	for(; it != end; ++it)
	{
		const KIO::UDSEntry &entry = *it;

		QString name = entry.stringValue(KIO::UDS_NAME);
		if(entry.isDir()) // Create new map entry...
		{
			assert(m_directoryMap.constFind(name) == m_directoryMap.constEnd());
			m_directoryMap.insert(name, QStringList());

			QString ignoreFile = m_testsUrl.path() + "/tests/" + name + "/ignore";
			QString failureFile = m_testsUrl.path() + "/tests/" + name + "/KNOWN_FAILURES";

			QStringList ignoreFileList = readListFile(ignoreFile);
			QStringList failureFileList = readListFile(failureFile);

			if(!ignoreFileList.isEmpty())
				m_ignoreMap.insert(name, ignoreFileList);

			if(!failureFileList.isEmpty())
				m_failureMap.insert(name, failureFileList);
		}
		else if(name.endsWith(".html") || name.endsWith(".htm") ||
				name.endsWith(".xhtml") || name.endsWith(".xml") || name.endsWith(".js"))
		{
			int lastSlashPos = name.lastIndexOf('/');

			QString cachedDirectory = (lastSlashPos > 0 ? name.mid(0, lastSlashPos) : QString());
			QString cachedFilename = name.mid(lastSlashPos + 1);
	
			assert(m_directoryMap.constFind(cachedDirectory) != m_directoryMap.constEnd());
			m_directoryMap[cachedDirectory].append(cachedFilename);
		}
	}
}

void TestRegressionWindow::directoryListingFinished(KIO::Job *)
{
	QTreeWidgetItem *topLevelItem = m_ui.treeWidget->topLevelItem(0);

	QMap<QString, QStringList>::const_iterator it = m_directoryMap.constBegin();
	const QMap<QString, QStringList>::const_iterator end = m_directoryMap.constEnd();

	unsigned long availableTests = 0;

	for(; it != end; ++it)
	{
		QString directory = it.key();
		QStringList filenames = it.value();

		bool hasIgnores = (m_ignoreMap.constFind(directory) != m_directoryMap.constEnd());
		bool hasFailures = (m_failureMap.constFind(directory) != m_failureMap.constEnd());

		if(filenames.isEmpty()) // Do not add empty directories at all...
			continue;

		// Sort in ascending order...
		filenames.sort();

		QStringList::const_iterator it2 = filenames.constBegin();
		const QStringList::const_iterator end2 = filenames.constEnd();

		// Create new tree widget item for the active directory...
		QTreeWidgetItem *parent = topLevelItem;

		if(!directory.isEmpty())
		{
			parent = new QTreeWidgetItem(topLevelItem, QStringList(directory));

			// Extract parent directory...
			int position = directory.lastIndexOf('/');

			QString parentDirectory = directory.mid(0, (position == -1 ? 0 : position));
			QString parentDirectoryItem = directory.mid(position + 1);

			bool hasParentIgnores = (m_ignoreMap.constFind(parentDirectory) != m_directoryMap.constEnd());
			bool hasParentFailures = (m_failureMap.constFind(parentDirectory) != m_failureMap.constEnd());

			// Directory is completely ignored, mark it 'yellow'...
			if(hasParentIgnores && m_ignoreMap[parentDirectory].contains(parentDirectoryItem))
				parent->setIcon(0, m_ignorePixmap);

			// Directory is completely known to fail, mark it 'red'...
			if(hasParentFailures && m_failureMap[parentDirectory].contains(parentDirectoryItem))
				parent->setIcon(0, m_failKnownPixmap);
		}

		// Add all contained files as new items below 'parent'...
		for(; it2 != end2; ++it2)
		{
			QString test = (*it2);
			QString cacheName = directory + "/" + test;

			QTreeWidgetItem *testItem = new QTreeWidgetItem(parent, QStringList(KUrl(test).path()));

			// Remember name <-> item pair...
			assert(m_itemMap.constFind(cacheName) != m_directoryMap.constEnd());
			m_itemMap.insert(cacheName, testItem);

			// Ignored tests are marked 'yellow'...
			if(hasIgnores && m_ignoreMap[directory].contains(test))
				testItem->setIcon(0, m_ignorePixmap);

			// Tests, known to fail, are marked 'red'...
			if(hasFailures && m_failureMap[directory].contains(test))
				testItem->setIcon(0, m_failKnownPixmap);

			availableTests++;
		}
	}

/* MAP DEBUGGING
	unsigned long ignoreMapSize = countMapItems(m_ignoreMap);
	unsigned long failureMapSize = countMapItems(m_failureMap);
	unsigned long directoryMapSize = countMapItems(m_directoryMap);

	kDebug() << "---------------------------------------" << endl
			 << "Map statistics after directory listing:" << endl
			 << "Available tests:    " << availableTests << endl
			 << "Item Map Size:      " << m_itemMap.size() << endl
			 << "Ignore Map Size:    " << m_ignoreMap.size() << " Items: " << ignoreMapSize << endl
			 << "Failure Map Size:   " << m_failureMap.size() << " Items: " << failureMapSize << endl
			 << "Directory Map Size: " << m_directoryMap.size() << " Items: " << directoryMapSize << endl
			 << "---------------------------------------" << endl << endl;
*/

	// This is an estimation of the number of tests which will be run.
	// Most tests have RENDER/PAINT output, that's why 2 is a quite good guess.
	const unsigned long progressBarRange = availableTests * 2;
	m_ui.progressBar->setRange(0, progressBarRange);

	m_ui.treeWidget->headerItem()->setText(0, i18n("Available Tests: %1").arg(availableTests));
}

void TestRegressionWindow::pauseContinueButtonClicked()
{
	assert(m_activeProcess != 0);

	if(!m_suspended)
	{
		// Suspend process
		kill(m_activeProcess->pid(), SIGSTOP);

		m_suspended = true;
		m_ui.pauseContinueButton->setText(i18n("Continue"));
	}
	else
	{
		// Continue process
		kill(m_activeProcess->pid(), SIGCONT);

		m_suspended = false;
		m_ui.pauseContinueButton->setText(i18n("Pause"));
	}
}

void TestRegressionWindow::saveLogButtonClicked()
{
	assert(m_activeProcess == 0);
	m_saveLogUrl = KDirSelectDialog::selectDirectory(QString(), true /* local only */);

	QString fileName = m_saveLogUrl.path() + "/logOutput.html";
	if(QFileInfo(fileName).exists())
	{
		// Remove file if already existant...
		QFile file(fileName);
		if(!file.remove())
		{
			kError() << " Can't remove " << fileName << endl;
			exit(1);
		}
	}
}

void TestRegressionWindow::runTests()
{
	// Run in all-in-one mode...
	m_runCounter = 0;
	m_testCounter = 0;

	initRegressionTesting(QString());
}

void TestRegressionWindow::runSingleTest()
{
	assert(m_activeTreeItem != 0);

	QString testFileName = pathFromItem(m_activeTreeItem);

	// Run in single-test mode...
	m_runCounter = 0;
	m_testCounter = -1;

	initRegressionTesting(testFileName);
}

void TestRegressionWindow::initRegressionTesting(const QString &testFileName)
{
	assert(m_activeProcess == 0);

	m_activeProcess = new QProcess();
	m_activeProcess->setReadChannelMode(QProcess::MergedChannels);

	QStringList environment = QProcess::systemEnvironment();
	environment << "KDE_DEBUG=false"; // No Dr. Konqi please!

	QString program = m_khtmlUrl.path() + "/.libs/testregression";

	QStringList arguments;
	arguments << "--base" << m_testsUrl.path();

	if(!m_outputUrl.isEmpty())
		arguments << "--output" << m_outputUrl.path();

	if(!testFileName.isEmpty())
		arguments << "--test" << testFileName;

	if(m_flags & (1 << JSTests))
		arguments << "--js";
	if(m_flags & (1 << HTMLTests))
		arguments << "--html";
	if(m_flags & (1 << DebugOutput))
		arguments << "--debug";
	if(m_flags & (1 << NoXvfbUse))
		arguments << "--noxvfb";

	connect(m_activeProcess, SIGNAL(finished(int, QProcess::ExitStatus)), SLOT(testerExited(int, QProcess::ExitStatus)));
	connect(m_activeProcess, SIGNAL(readyReadStandardOutput()), SLOT(testerReceivedData()));

	// Clear processing queue before starting...
	m_processingQueue.clear();

	// Clean up gui...
	m_ui.textEdit->clear();
	m_ui.progressBar->reset();
	m_ui.saveLogButton->setEnabled(false);
	m_ui.actionRun_tests->setEnabled(false);
	m_ui.pauseContinueButton->setEnabled(true);

	// Start regression testing process...
	m_activeProcess->setEnvironment(environment);
	m_activeProcess->start(program, arguments, QIODevice::ReadOnly);
}

void TestRegressionWindow::updateItemStatus(TestResult result, QTreeWidgetItem *item, const QString &testFileName)
{
	if(!item)
		return;

	// Ensure item is visible...
	QTreeWidgetItem *parent = item;
	while(parent != 0)
	{
		m_ui.treeWidget->setItemExpanded(parent, true);
		parent = parent->parent();
	}

	m_ui.treeWidget->scrollToItem(item);

	bool updateIcon = true;
	if(m_lastName == testFileName && !m_lastName.isEmpty())
	{
		if(m_lastResult == result)
			updateIcon = false;
		else if((m_lastResult == Pass || m_lastResult == PassUnexpected) &&
				(result == Fail || result == FailKnown || result == Crash))
		{
			// If one part of the test (render/dom/paint) passed,
			// and the current part fails, update to 'failed' icon...
			updateIcon = true;
		}
		else if((m_lastResult == Fail || m_lastResult == FailKnown || m_lastResult == Crash) &&
				(result == Pass || result == PassUnexpected))
		{
			// If one part of the test (render/dom/paint) failed,
			// and the current part passes, don't update to 'passed' icon...
			updateIcon = false;
		}
	}

	// Update icon, if necessary...
	if(updateIcon)
	{
		if(result == Crash)
			item->setIcon(0, m_crashPixmap);
		else if(result == Fail)
			item->setIcon(0, m_failPixmap);
		else if(result == FailKnown)
			item->setIcon(0, m_failKnownPixmap);
		else if(result == Pass)
			item->setIcon(0, m_passPixmap);
		else if(result == PassUnexpected)
			item->setIcon(0, m_passUnexpectedPixmap);
		else // Unhandled state...
			assert(false);
	}

	// Remember test & result...
	m_lastResult = result;
	m_lastName = testFileName;
	m_activeTreeItem = item;
}

void TestRegressionWindow::initLegend()
{
	// Init pixmaps...
	m_failPixmap = QPixmap(":/test/pics/fail.xpm");
	m_failKnownPixmap = QPixmap(":/test/pics/failKnown.xpm");
	m_passPixmap = QPixmap(":/test/pics/pass.xpm");
	m_passUnexpectedPixmap = QPixmap(":/test/pics/passUnexpected.xpm");
	m_ignorePixmap = QPixmap(":/test/pics/ignore.xpm");
	m_crashPixmap = QPixmap(":/test/pics/crash.xpm");

	QString legend = QLatin1String("<body><center><font size='8'>Welcome to the khtml<br/>") +
					 QLatin1String("regression testing tool!</font></center><br/><br/>") +
					 QLatin1String("<table border='0' align='center' cellspacing='15'>") +
					 QLatin1String("<tr valign='top'><td colspan='2'><center><b>Legend</b></center></td></tr>") +
					 QLatin1String("<tr valign='middle'><td>Pass</td><td><img src=':/test/pics/pass.xpm'></td></tr>") +
					 QLatin1String("<tr valign='middle'><td>Pass unexpected</td><td><img src=':/test/pics/passUnexpected.xpm'></td></tr>") +
					 QLatin1String("<tr valign='middle'><td>Fail</td><td><img src=':/test/pics/fail.xpm'></td></tr>") +
					 QLatin1String("<tr valign='middle'><td>Fail known</td><td><img src=':/test/pics/failKnown.xpm'></td></tr>") +
					 QLatin1String("<tr valign='middle'><td>Ignore</td><td><img src=':/test/pics/ignore.xpm'></td></tr>") +
					 QLatin1String("<tr valign='middle'><td>Crash</td><td><img src=':/test/pics/crash.xpm'></td></tr>") +
					 QLatin1String("</table></body>");

	m_ui.textEdit->setHtml(legend);
}

void TestRegressionWindow::testerExited(int /* exitCode */, QProcess::ExitStatus exitStatus)
{
	assert(m_activeProcess != 0);
	assert(m_activeTreeItem != 0);

	if(exitStatus == QProcess::CrashExit) // Special case: crash!
	{
		QTreeWidgetItem *useItem = m_activeTreeItem;

		if(m_testCounter >= 0 || m_runCounter > 0) // Single-tests mode invoked on a directory OR All-test-mode
		{
			QTreeWidgetItem *parent = useItem->parent();
			assert(parent != 0);

			useItem = parent->child(parent->indexOfChild(useItem) + 1);
			assert(useItem != 0);
		}

		// Reflect crashed test...
		updateItemStatus(Crash, useItem, QString());
	}

	if(m_testCounter >= 0) // All-tests mode
		m_ui.progressBar->setValue(m_ui.progressBar->maximum());

	// Eventually save log output...
	if(!m_saveLogUrl.isEmpty())
	{
		// We should close our written log with </body></html>.
		m_processingQueue.enqueue(QString::fromLatin1("\n</body>\n</html>"));

		if(!m_justProcessingQueue)
		{
			m_justProcessingQueue = true;
			QTimer::singleShot(50, this, SLOT(processQueue()));
		}
	}

	// Cleanup gui...
	m_ui.saveLogButton->setEnabled(true);
	m_ui.actionRun_tests->setEnabled(true);
	m_ui.pauseContinueButton->setEnabled(false);

	// Cleanup data..
	delete m_activeProcess;
	m_activeProcess = 0;

	m_runCounter = 0;
	m_testCounter = 0;
	m_activeTreeItem = 0;
}

void TestRegressionWindow::testerReceivedData()
{
	assert(m_activeProcess != 0);

	QString data(m_activeProcess->readAllStandardOutput());
	QStringList list = data.split('\n');

	QStringList::const_iterator it = list.constBegin();
	const QStringList::const_iterator end = list.constEnd();

	for(; it != end; ++it)
	{
		QString temp = *it;
		if(!temp.isEmpty())
			m_processingQueue.enqueue(temp);
	}

	if(!m_justProcessingQueue)
	{
		m_justProcessingQueue = true;
		QTimer::singleShot(50, this, SLOT(processQueue()));
	}
}

void TestRegressionWindow::processQueue()
{
	while(!m_processingQueue.isEmpty())
	{
		QString data = m_processingQueue.dequeue();
		TestResult result = Unknown;

		QString cacheName = extractTestNameFromData(data, result);

		if(result != Unknown) // Yes, we're dealing with a test result...
		{
			if(cacheName.isEmpty()) // Make sure everything is alright!
			{
				kError() << "Couldn't extract cacheName from data=\"" << data << "\"! Ignoring!" << endl;
				continue;
			}
		}

		parseRegressionTestingOutput(data, result, cacheName);
	}

	m_justProcessingQueue = false;
}

void TestRegressionWindow::addToIgnores()
{
	assert(m_activeTreeItem != 0);

	QString treeItemText = pathFromItem(m_activeTreeItem);

	// Extract directory/file name...
	int position = treeItemText.lastIndexOf('/');

	QString directory = treeItemText.mid(0, (position == -1 ? 0 : position));
	QString fileName = treeItemText.mid(position + 1);

	// Read corresponding ignore file..
	QString ignoreFile = m_testsUrl.path() + "/tests/" + directory + "/ignore";
	QStringList ignoreFileList = readListFile(ignoreFile);

	if(!ignoreFileList.contains(fileName))
		ignoreFileList.append(fileName);

	// Commit changes...
	writeListFile(ignoreFile, ignoreFileList);

	// Reset icon status...
	m_activeTreeItem->setIcon(0, m_ignorePixmap);
}

void TestRegressionWindow::removeFromIgnores()
{
	assert(m_activeTreeItem != 0);

	QString treeItemText = pathFromItem(m_activeTreeItem);

	// Extract directory/file name...
	int position = treeItemText.lastIndexOf('/');

	QString directory = treeItemText.mid(0, (position == -1 ? 0 : position));
	QString fileName = treeItemText.mid(position + 1);

	// Read corresponding ignore file..
	QString ignoreFile = m_testsUrl.path() + "/tests/" + directory + "/ignore";
	QStringList ignoreFileList = readListFile(ignoreFile);

	if(ignoreFileList.contains(fileName))
		ignoreFileList.removeAll(fileName);

	// Commit changes...
	writeListFile(ignoreFile, ignoreFileList);

	// Reset icon status...
	m_activeTreeItem->setIcon(0, QPixmap());
}

QString TestRegressionWindow::pathFromItem(const QTreeWidgetItem *item) const
{
	QString path = item->text(0);

	QTreeWidgetItem *parent = item->parent();
	while(parent != 0)
	{
		if(parent->parent() != 0)
			path.prepend(parent->text(0) + "/");

		parent = parent->parent();
	}

	return path;
}

QString TestRegressionWindow::extractTestNameFromData(QString &data, TestResult &result) const
{
	if(data.indexOf("PASS") >= 0 || data.indexOf("FAIL") >= 0)
	{
		// Name extraction regexps...
		QString bracesSelector("[0-9a-zA-Z-_<>\\* +-,.:!?$'\"=/\\[\\]\\(\\)]*");

		QRegExp expPass("PASS: (" + bracesSelector + ")");
		QRegExp expPassUnexpected("PASS \\(unexpected!\\): (" + bracesSelector + ")");

		QRegExp expFail("FAIL: (" + bracesSelector + ")");
		QRegExp expFailKnown("FAIL \\(known\\): (" + bracesSelector + ")");

		// Extract name of test... (while using regexps as rare as possible!)
		int pos = -1;
		QString test;

		QRegExp cleanTest(" \\[" + bracesSelector + "\\]");

		pos = expPass.indexIn(data);
		if(pos > -1) { test = expPass.cap(1); result = Pass; }

		if(result == Unknown)
		{
			pos = expPassUnexpected.indexIn(data);
			if(pos > -1) { test = expPassUnexpected.cap(1); result = PassUnexpected; }
		}

		if(result == Unknown)
		{
			pos = expFail.indexIn(data);
			if(pos > -1) { test = expFail.cap(1); result = Fail; }
		}

		if(result == Unknown)
		{
			pos = expFailKnown.indexIn(data);
			if(pos > -1) { test = expFailKnown.cap(1); result = FailKnown; }
		}

		if(!test.isEmpty() && result != Unknown) // Got information about test...
		{
			// Clean up first, so we only get the file name...
			test.replace(cleanTest, QString());

			// Extract cached directory/filename pair...
			int lastSlashPos = test.lastIndexOf('/');

			QString cachedDirectory = (lastSlashPos > 0 ? test.mid(0, lastSlashPos) : QString());
			QString cachedFilename = test.mid(lastSlashPos + 1);

			if(cachedDirectory == ".") // Handle cases like "./empty.html"
				cachedDirectory = QString();

			assert(m_directoryMap.constFind(cachedDirectory) != m_directoryMap.constEnd());

			QString cacheName = cachedDirectory + "/" + cachedFilename;
			if(m_itemMap.constFind(cacheName) != m_itemMap.constEnd())
			{
				// Highlight test...
				data.replace(expPass, "<b><font color='green'>PASS:\t\\1</font></b>");
				data.replace(expPassUnexpected, "<b><font color='green'>PASS (unexpected!):\t\\1</font></b>");
				data.replace(expFail, "<b><font color='red'>FAIL:\t\\1</font></b>");
				data.replace(expFailKnown, "<b><font color='red'>FAIL (known):\t\\1</font></b>");

				return cacheName;
			}
		}
	}

	return QString();
}

void TestRegressionWindow::parseRegressionTestingOutput(QString data, TestResult result, const QString &cacheName)
{
	if(!cacheName.isEmpty())
	{
		if(m_testCounter >= 0) // Only increment in all-tests mode...
			m_testCounter++;

		m_runCounter++; // Always increment...

		// Update the icon...
		updateItemStatus(result, m_itemMap[cacheName], cacheName);
	}

	// Apply some nice formatting for the statistics...
	if(data.indexOf("Total") >= 0 || data.indexOf("Passes") >= 0 || data.indexOf("Tests completed") >= 0 ||
	   data.indexOf("Errors:") >= 0 || data.indexOf("Failures:") >= 0)
	{
		QRegExp expTotal("Total:    ([0-9]*)");
		QRegExp expPasses("Passes:   ([0-9 a-z\\(\\)]*)");
		QRegExp expErrors("Errors:   ([0-9 a-z]*)");
		QRegExp expFailures("Failures: ([0-9 a-z\\(\\)]*)");

		data.replace("Tests completed.", "<br><center><h2>Tests completed.</h2></center>");
		data.replace(expTotal, "<b><font size='4'>Total:&nbsp;\\1</font></b>");
		data.replace(expPasses, "<b><font size='4' color='green'>Passes:&nbsp;\\1</font></b>");
		data.replace(expErrors, "<b><font size='4' color='blue'>Errors:&nbsp;\\1</font></b>");
		data.replace(expFailures, "<b><font size='4' color='red'>Failures:&nbsp;\\1</font></b>");
	}

	if(!data.contains("</body>\n</html>")) // Don't put <br> behind </html>!
		data.append("<br>");

	// Update text edit...
	updateLogOutput(data);

	// Update progressbar...
	if(m_testCounter >= 0)
		m_ui.progressBar->setValue(m_testCounter);
}

void TestRegressionWindow::treeWidgetContextMenuRequested(const QPoint &pos)
{
	if((m_testCounter == -1 && m_activeProcess) || (m_testCounter > 0 && m_activeProcess) ||
	   m_testsUrl.isEmpty() || m_khtmlUrl.isEmpty()) // Still processing/not ready yet...
	{
		return;
	}

	QTreeWidgetItem *item = m_ui.treeWidget->itemAt(pos);
	if(item && item != m_ui.treeWidget->topLevelItem(0))
	{
		m_activeTreeItem = item;

		// Build & show popup menu...
		QMenu menu(m_ui.treeWidget);

		menu.addAction(SmallIcon("player_play"), i18n("Run test..."), this, SLOT(runSingleTest()));
		menu.addSeparator();
		menu.addAction(SmallIcon("add"), i18n("Add to ignores..."), this, SLOT(addToIgnores()));
		menu.addAction(SmallIcon("button_cancel"), i18n("Remove from ignores..."), this, SLOT(removeFromIgnores()));
		
		if(!menu.exec(m_ui.treeWidget->mapToGlobal(pos)))
			m_activeTreeItem = 0; // Needs reset...
	}	
}

void TestRegressionWindow::updateLogOutput(const QString &data)
{
	QTextCursor cursor = m_ui.textEdit->textCursor();

	// Append 'data'...
	m_ui.textEdit->insertHtml(data);

	// Keep a maximum of 100 lines in the log...
	const int maxLogLines = 100;

	long logLines = countLogLines();
	if(logLines > maxLogLines)
	{
		cursor.movePosition(QTextCursor::Start);
		cursor.movePosition(QTextCursor::Down, QTextCursor::KeepAnchor, logLines - maxLogLines);
		cursor.removeSelectedText();
	}

	// Go to end...
	cursor.movePosition(QTextCursor::End);
	m_ui.textEdit->setTextCursor(cursor);

	// Eventually save log output...
	if(!m_saveLogUrl.isEmpty())
	{
		QString fileName = m_saveLogUrl.path() + "/logOutput.html";
		QIODevice::OpenMode fileFlags = QIODevice::WriteOnly;

		bool fileExists = QFileInfo(fileName).exists();
		if(fileExists)
			fileFlags |= QIODevice::Append;

		QFile file(fileName);
		if(!file.open(fileFlags))
		{
			kError() << " Can't open " << fileName << endl;
			exit(1);
		}

		if(!fileExists)
			file.write(QString::fromLatin1("<html>\n<body>\n").toAscii());

		file.write((data + "\n").toAscii());
		file.close();

		// Reset save log url, if we reached the end...
		if(data.contains("</body>\n</html>"))
			m_saveLogUrl = KUrl();
	}
}

unsigned long TestRegressionWindow::countLogLines() const
{
	QTextCursor cursor = m_ui.textEdit->textCursor();
	cursor.movePosition(QTextCursor::Start);
	
	unsigned long lines = 0;
	while(cursor.movePosition(QTextCursor::Down, QTextCursor::MoveAnchor))
		lines++;

	return lines;
}

unsigned long TestRegressionWindow::countMapItems(const QMap<QString, QStringList > &map) const
{
	unsigned long counter = 0;

	QMap<QString, QStringList>::const_iterator it = map.constBegin();
	const QMap<QString, QStringList>::const_iterator end = map.constEnd();

	for(; it != end; ++it)
	{
		QStringList list = it.value();

		QStringList::const_iterator it2 = list.constBegin();
		const QStringList::const_iterator end2 = list.constEnd();

		for(; it2 != end2; ++it2)
			counter++;
	}

	return counter;
}

QStringList TestRegressionWindow::readListFile(const QString &fileName) const
{
	QStringList files;

	QFileInfo fileInfo(fileName);
	if(fileInfo.exists())
	{
		QFile file(fileName);
		if(!file.open(QIODevice::ReadOnly))
		{
			kError() << " Can't open " << fileName << endl;
			exit(1);
		}

		QString line;

		QTextStream fileStream(&file);
		while(!(line = fileStream.readLine()).isNull())
			files.append(line);

		file.close();
	}

	return files;
}

void TestRegressionWindow::writeListFile(const QString &fileName, const QStringList &content) const
{
	QFile file(fileName);
	if(!file.open(QIODevice::WriteOnly | QIODevice::Truncate))
	{
		kError() << " Can't open " << fileName << endl;
		exit(1);
	}

	file.write(content.join("\n").toAscii());
	file.close();
}

// vim:ts=4:tw=4:noet
