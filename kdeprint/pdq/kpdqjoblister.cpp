#include "kpdqjoblister.h"
#include "pdqclass.h"

#include <stdlib.h>
#include <qfile.h>
#include <qtextstream.h>
#include <qfileinfo.h>
#include <qdir.h>

QString jobDir()
{
	PDQMain	main_;
	main_.parseFile(getenv("HOME")+QString::fromLatin1("/.printrc"),false);
	QString	dir_ = main_.jobdir_;
	if (dir_.isEmpty())
		dir_ = getenv("HOME") + QString::fromLatin1("/.printjobs/");
	return dir_;
}

bool readJobFile(const QString& filename, QMap<QString,QString>& items)
{
	QFile	f(filename);
	if (!f.exists() || !f.open(IO_ReadOnly))
		return false;
	QTextStream	t(&f);
	QString	line;
	QString	word, value;
	while (!t.eof())
	{
		word = "";
		value = "";
		line = t.readLine().simplifyWhiteSpace();
		if (line.isEmpty()) continue;
		int	p = line.find(' '), p2(-1);
		if (p != -1)
			word = line.left(p);
		else
			return false;
		p = line.find('{');
		p2 = line.findRev('}');
		if (p != -1 && p2 != -1)
			value = line.mid(p+1,p2-p-1).stripWhiteSpace();
		else
			return false;
		if (!word.isEmpty())
			items[word] = value;
		else
			return false;
	}
	return true;
}

int stateFromString(const QString& s)
{
	if (s == "finished" || s.startsWith("cancelled")) return KJob::Held;
	else if (s == "aborted" || s.startsWith("ABORT")) return KJob::Error;
	else return KJob::Printing;
}

bool readJob(KJob *job, const QString& stfile)
{
	QMap<QString,QString>	items;
	if (readJobFile(stfile,items))
	{
		QFileInfo	fi(stfile);
		job->ID = fi.baseName().toInt();
		job->printer = items["printer"];
		job->name = QFileInfo(items["input_filename"]).fileName();
		job->user = getenv("USER");
		job->size = 0;
		job->state = stateFromString(items["status"]);
		return true;
	}
	return false;
}

//****************************************************************************************************

KPdqJobLister::KPdqJobLister()
: KJobLister()
{
}

int KPdqJobLister::actions()
{
	return (KJob::Remove|KJob::Move);
}

bool KPdqJobLister::sendCommand(const QList<KJob>&, int, const QString&)
{
	return false;
}

bool KPdqJobLister::listJobs()
{
	// try to find job dir: first look in local printrc file, then try the default location
	QDir	jobdir(jobDir());
	if (!jobdir.exists())
		return false;

	QStringList	joblist = jobdir.entryList(QString::fromLatin1("*.status"),QDir::Files,QDir::Name);
	for (QStringList::ConstIterator it=joblist.begin(); it!=joblist.end(); ++it)
	{
		KJob	*job = new KJob();
		// only keep job if read OK, ID > 0 and job not held ("Held" stands for "Finished" in this case)
		if (readJob(job,jobdir.absFilePath(*it)) && job->ID > 0 && job->state != KJob::Held &&
		    printers_.contains(job->printer) > 0)
		{
			KJob	*aJob = findJob(job->ID);
			if (aJob)
			{
				jobCopy(job,aJob);
				delete job;
			}
			else
			{
				job->discarded = false;
				jobs_.append(job);
			}
		}
		else
			delete job;
	}

	return true;
}
