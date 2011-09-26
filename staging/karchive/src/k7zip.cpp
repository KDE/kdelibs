/* This file is part of the KDE libraries
   Copyright (C) 2011 Mario Bensi <mbensi@ipsquad.net>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "k7zip.h"

#include <QtCore/QDebug>
#include <QtCore/QDir>
#include <QtCore/QBuffer>
#include <QtCore/QFile>

#include "kcompressiondevice.h"
#include <kfilterbase.h>


#include <time.h> // time()
#include "zlib.h"


////////////////////////////////////////////////////////////////////////
/////////////////////////// K7Zip //////////////////////////////////////
////////////////////////////////////////////////////////////////////////

unsigned char k7zip_signature[6] = {'7', 'z', 0xBC, 0xAF, 0x27, 0x1C};

#define GetUi16(p, offset) (((unsigned char)p[offset+0]) | (((unsigned char)p[1]) << 8))

#define GetUi32(p, offset) ( \
                     ((unsigned char)p[offset+0])        | \
                     (((unsigned char)p[offset+1]) <<  8) | \
                     (((unsigned char)p[offset+2]) << 16) | \
                     (((unsigned char)p[offset+3]) << 24))

#define GetUi64(p, offset) ((quint32)GetUi32(p, offset) | (((quint64)GetUi32(p, offset + 4)) << 32))

#define FILE_ATTRIBUTE_READONLY             1
#define FILE_ATTRIBUTE_HIDDEN               2
#define FILE_ATTRIBUTE_SYSTEM               4
#define FILE_ATTRIBUTE_DIRECTORY           16
#define FILE_ATTRIBUTE_ARCHIVE             32
#define FILE_ATTRIBUTE_DEVICE              64
#define FILE_ATTRIBUTE_NORMAL             128
#define FILE_ATTRIBUTE_TEMPORARY          256
#define FILE_ATTRIBUTE_SPARSE_FILE        512
#define FILE_ATTRIBUTE_REPARSE_POINT     1024
#define FILE_ATTRIBUTE_COMPRESSED        2048
#define FILE_ATTRIBUTE_OFFLINE          0x1000
#define FILE_ATTRIBUTE_ENCRYPTED        0x4000
#define FILE_ATTRIBUTE_UNIX_EXTENSION   0x8000   /* trick for Unix */

enum HeaderType
{
    kEnd,

    kHeader,

    kArchiveProperties,

    kAdditionalStreamsInfo,
    kMainStreamsInfo,
    kFilesInfo,

    kPackInfo,
    kUnpackInfo,
    kSubStreamsInfo,

    kSize,
    kCRC,

    kFolder,

    kCodersUnpackSize,
    kNumUnpackStream,

    kEmptyStream,
    kEmptyFile,
    kAnti,

    kName,
    kCTime,
    kATime,
    kMTime,
    kAttributes,
    kComment,

    kEncodedHeader,

    kStartPos,
    kDummy
};

// Method ID
static const quint64 k_Copy = 0x00;
static const quint64 k_Delta = 0x03;
static const quint64 k_x86 = 0x04; //BCJ
static const quint64 k_PPC = 0x05; // BIG Endian
static const quint64 k_IA64 = 0x06;
static const quint64 k_ARM = 0x07; // little Endian
static const quint64 k_ARM_Thumb = 0x08; // little Endian
static const quint64 k_SPARC = 0x09;
static const quint64 k_LZMA2 = 0x21;
static const quint64 k_Swap2 = 0x020302;
static const quint64 k_Swap4 = 0x020304;
static const quint64 k_LZMA = 0x030101;
static const quint64 k_BCJ = 0x03030103;
static const quint64 k_BCJ2 = 0x0303011B;
static const quint64 k_7zPPC = 0x03030205;
static const quint64 k_Alpha = 0x03030301;
static const quint64 k_7zIA64 = 0x03030401;
static const quint64 k_7zARM = 0x03030501;
static const quint64 k_M68 = 0x03030605; //Big Endian
static const quint64 k_ARMT = 0x03030701;
static const quint64 k_7zSPARC = 0x03030805;
static const quint64 k_PPMD = 0x030401;
static const quint64 k_Experimental = 0x037F01;
static const quint64 k_Shrink = 0x040101;
static const quint64 k_Implode = 0x040106;
static const quint64 k_Deflate = 0x040108;
static const quint64 k_Deflate64 = 0x040109;
static const quint64 k_Imploding = 0x040110;
static const quint64 k_Jpeg = 0x040160;
static const quint64 k_WavPack = 0x040161;
static const quint64 k_PPMd = 0x040162;
static const quint64 k_wzAES = 0x040163;
static const quint64 k_BZip2 = 0x040202;
static const quint64 k_Rar15 = 0x040301;
static const quint64 k_Rar20 = 0x040302;
static const quint64 k_Rar29 = 0x040303;
static const quint64 k_Arj = 0x040401; //1 2 3
static const quint64 k_Arj4 = 0x040402;
static const quint64 k_Z = 0x0405;
static const quint64 k_Lzh = 0x0406;
static const quint64 k_Cab = 0x0408;
static const quint64 k_DeflateNSIS = 0x040901;
static const quint64 k_Bzip2NSIS = 0x040902;
static const quint64 k_AES = 0x06F10701;


class FileInfo
{
public:
    FileInfo()
      : attribDefined(false)
      , attributes(0)
      , hasStream(false)
      , isDir(false)
      , size(0)
      , crc(0)
      , crcDefined(false)
    {}

    QString name;
    bool attribDefined;
    quint32 attributes;
    bool hasStream;
    bool isDir;
    quint64 size;
    quint32 crc;
    bool crcDefined;
};

class Folder
{
public:
    class FolderInfo
    {
    public:
        FolderInfo()
            : numInStreams(0)
            , numOutStreams(0)
            , methodID(0)
        {
        }

        int numInStreams;
        int numOutStreams;
        QVector<unsigned char> properties;
        quint64 methodID;
    };

    Folder()
        : unpackCRCDefined(false)
        , unpackCRC(0)
    {}

    bool unpackCRCDefined;
    quint32 unpackCRC;
    QVector<FolderInfo*> folderInfos;
    QVector<quint64> inIndexes;
    QVector<quint64> outIndexes;
    QVector<quint64> packedStreams;
    QVector<quint64> unpackSizes;
};

class K7Zip::K7ZipPrivate
{
public:
    K7ZipPrivate(K7Zip *parent)
      : q(parent),
        packPos(0),
        numPackStreams(0),
        end(0)
    {
    }

    K7Zip *q;
    QStringList dirList;
    QVector<Folder*> folders;
    QVector<FileInfo*> fileInfos;
    // File informations
    QVector<quint64> cTimes;
    QVector<quint64> aTimes;
    QVector<quint64> mTimes;
    QVector<quint64> startPositions;
    QVector<int> fileInfoPopIDs;

    quint64 packPos;
    quint64 numPackStreams;
    QVector<quint64> packSizes;
    QVector<quint64> unpackSizes;
    QVector<bool> digestsDefined;
    QVector<quint32> digests;

    QVector<bool> isAnti;

    quint64 end;

    int readByte(const char* buffer, int& pos);
    quint32 readUInt32(const char* buffer, int& pos);
    quint64 readUInt64(const char* buffer, int& pos);
    quint64 readNumber(const char*, int& pos);
    QString readString(const char*, int& pos);
    void readHashDigests(int numItems, QVector<bool> &digestsDefined, QVector<quint32> &digests,
                         const char* buffer, int& pos);
    void readBoolVector(int numItems, QVector<bool> &v, const char* buffer, int& pos);
    void readBoolVector2(int numItems, QVector<bool> &v, const char* buffer, int& pos);
    void skipData(int size, const char* buffer, int& pos);
    bool findAttribute(int attribute, const char* buffer, int& pos);
    bool readUInt64DefVector(int numFiles, QVector<quint64>& values, const char* buffer, int& pos);
    Folder* folderItem(const char* buffer, int& pos);
    bool readMainStreamsInfo(const char* buffer, int& pos);
    bool readPackInfo(const char* buffer, int& pos);
    bool readUnpackInfo(const char* buffer, int& pos);
    bool readSubStreamsInfo(const char* buffer, int& pos);
    QByteArray readAndDecodePackedStreams(const char* buffer, int& pos, int& headerSize, bool readMainStreamInfo = true);
};

K7Zip::K7Zip( const QString& fileName )
    : KArchive( fileName ), d(new K7ZipPrivate(this))
{
}

K7Zip::K7Zip( QIODevice * dev )
    : KArchive( dev ), d(new K7ZipPrivate(this))
{
    Q_ASSERT( dev );
}

K7Zip::~K7Zip()
{
    if( isOpen() )
        close();

    delete d;
}

int K7Zip::K7ZipPrivate::readByte(const char* buffer, int& pos)
{
    return buffer[pos++];
}

quint32 K7Zip::K7ZipPrivate::readUInt32(const char* buffer, int& pos)
{
    if ((quint64)(pos + 4) > end) {
        qDebug() << "error size";
        return 0;
    }

    quint32 res = GetUi32(buffer, pos);
    pos += 4;
    return res;
}

quint64 K7Zip::K7ZipPrivate::readUInt64(const char* buffer, int& pos)
{
    if ((quint64)(pos + 8) > end) {
        qDebug() << "error size";
        return 0;
    }

    quint64 res = GetUi64(buffer, pos);
    pos += 8;
    return res;
}

quint64 K7Zip::K7ZipPrivate::readNumber(const char* buffer, int& pos)
{
    unsigned char firstByte = buffer[pos++];
    unsigned char mask = 0x80;
    quint64 value = 0;
    for (int i = 0; i < 8; i++) {
        if ((firstByte & mask) == 0) {
            quint64 highPart = firstByte & (mask - 1);
            value += (highPart << (i * 8));
            return value;
        }
        value |= ((unsigned char)buffer[pos++] << (8 * i));
        mask >>= 1;
    }
    return value;
}

QString K7Zip::K7ZipPrivate::readString(const char* buffer, int& pos)
{
    const char *buf = buffer + pos;
    size_t rem = (end - pos) / 2 * 2;
    {
        size_t i;
        for (i = 0; i < rem; i += 2) {
            if (buf[i] == 0 && buf[i + 1] == 0) {
                break;
            }
        }
        if (i == rem) {
            qDebug() << "read string error";
            return QString();
        }
        rem = i;
    }

    int len = (int)(rem / 2);
    if (len < 0 || (size_t)len * 2 != rem) {
        qDebug() << "read string unsupported";
        return QString();
    }

    QString p;
    for (int i = 0; i < len; i++, buf += 2) {
        p += (wchar_t)GetUi16(buf, 0);
    }

    pos += rem + 2;
    return p;
}

void K7Zip::K7ZipPrivate::skipData(int size, const char* /*buffer*/, int& pos)
{
    // TODO : test the pos 
    pos += size;
}

bool K7Zip::K7ZipPrivate::findAttribute(int attribute, const char* buffer, int& pos)
{
    for (;;)
    {
        int type = readByte(buffer, pos);
        if (type == attribute) {
            return true;
        }
        if (type == kEnd) {
        return false;
        }
        skipData(readNumber(buffer, pos), buffer, pos);
    }
}


void K7Zip::K7ZipPrivate::readBoolVector(int numItems, QVector<bool> &v, const char* buffer, int& pos)
{
    unsigned char b = 0;
    unsigned char mask = 0;
    for (int i = 0; i < numItems; i++) {
        if (mask == 0) {
            b = readByte(buffer, pos);
            mask = 0x80;
        }
        v.append((b & mask) != 0);
        mask >>= 1;
    }
}

void K7Zip::K7ZipPrivate::readBoolVector2(int numItems, QVector<bool> &v, const char* buffer, int& pos)
{
    int allAreDefined = readByte(buffer, pos);
    if (allAreDefined == 0) {
        readBoolVector(numItems, v, buffer, pos);
        return;
    }

    for (int i = 0; i < numItems; i++) {
        v.append(true);
    }
}

void K7Zip::K7ZipPrivate::readHashDigests(int numItems,
                                          QVector<bool> &digestsDefined,
                                          QVector<quint32> &digests,
                                          const char* buffer, int& pos)
{
    readBoolVector2(numItems, digestsDefined, buffer, pos);
    for (int i = 0; i < numItems; i++)
    {
        quint32 crc = 0;
        if (digestsDefined[i]) {
            crc = GetUi32(buffer, pos);
            pos += 4;
        }
        digests.append(crc);
    }
}

Folder* K7Zip::K7ZipPrivate::folderItem(const char* buffer, int& pos)
{
    Folder* folder = new Folder;
    int numCoders = readNumber(buffer, pos);

    quint64 numInStreamsTotal = 0;
    quint64 numOutStreamsTotal = 0;
    for (int i = 0; i < numCoders; i++) {
        Folder::FolderInfo* info = new Folder::FolderInfo();
        //BYTE 
        //    {
        //      0:3 CodecIdSize
        //      4:  Is Complex Coder
        //      5:  There Are Attributes
        //      6:  Reserved
        //      7:  There are more alternative methods. (Not used
        //      anymore, must be 0).
        //    }
        unsigned char coderInfo = readByte(buffer, pos);
        int codecIdSize = (coderInfo & 0xF);
        if (codecIdSize > 8) {
            qDebug() << "unsupported codec id size";
            delete folder;
            return 0;
        }
        unsigned char codecID[codecIdSize];
        for (int i=0; i < codecIdSize; ++i) {
            codecID[i] = readByte(buffer, pos);
        }

        int id = 0;
        for (int j = 0; j < codecIdSize; j++) {
            id |= codecID[codecIdSize - 1 - j] << (8 * j);
        }
        info->methodID = id;

        //if (Is Complex Coder)
        if ((coderInfo & 0x10) != 0) {
            info->numInStreams = readNumber(buffer, pos);
            info->numOutStreams = readNumber(buffer, pos);
        } else {
            info->numInStreams = 1;
            info->numOutStreams = 1;
        }

        //if (There Are Attributes)
        if ((coderInfo & 0x20) != 0) {
            int propertiesSize = readNumber(buffer, pos);
            for (int i=0; i < propertiesSize; ++i) {
                info->properties.append(readByte(buffer, pos));
            }
        }

        if ((coderInfo & 0x80) != 0) {
            qDebug() << "unsupported";
            delete folder;
            return 0;
        }

        numInStreamsTotal += info->numInStreams;
        numOutStreamsTotal += info->numOutStreams;
        folder->folderInfos.append(info);
    }

    int numBindPairs = numOutStreamsTotal - 1;
    for (int i = 0; i < numBindPairs; i++) {
        folder->inIndexes.append(readNumber(buffer, pos));
        folder->outIndexes.append(readNumber(buffer, pos));
    }

    int numPackedStreams = numInStreamsTotal - numBindPairs;
    if (numPackedStreams > 1) {
        for (int i = 0; i < numPackedStreams; ++i) {
            folder->packedStreams.append(readNumber(buffer, pos));
        }
    } else {
        if (numPackedStreams == 1) {
            for (quint64 i = 0; i < numInStreamsTotal; i++) {
                bool found = false;
                for (int j = 0; j < folder->inIndexes.size(); ++j) {
                    if (folder->inIndexes[j] == i) {
                        found = true;
                        break;
                    }
                }
                if (!found) {
                    folder->packedStreams.append(i);
                    break;
                }
            }
            if (folder->packedStreams.size() != 1) {
                delete folder;
                return 0;
            }
        }
    }
    return folder;
}

bool K7Zip::K7ZipPrivate::readUInt64DefVector(int numFiles, QVector<quint64>& values, const char* buffer, int& pos)
{
    QVector<bool> defined;
    readBoolVector2(numFiles, defined, buffer, pos);

    int external = readByte(buffer, pos);
    if (external != 0) {
        int dataIndex = readNumber(buffer, pos);
        if (dataIndex < 0 /*|| dataIndex >= dataVector->Size()*/) {
            qDebug() << "wrong data index";
            return false;
        }

        // TODO : go to the new index
    }

    for (int i = 0; i < numFiles; i++)
    {
        quint64 t = 0;
        if (defined[i]) {
            t = readUInt64(buffer, pos);
        }
        values.append(t);
    }
    return true;
}

bool K7Zip::K7ZipPrivate::readPackInfo(const char* buffer, int& pos)
{
    packPos = readNumber(buffer, pos);
    numPackStreams = readNumber(buffer, pos);
    packSizes.clear();

    if (!findAttribute(kSize, buffer, pos)) {
        qDebug() << "kSize not found";
        return false;
    }

    for (quint64 i = 0; i < numPackStreams; ++i) {
        packSizes.append(readNumber(buffer, pos));
    }

    int type;
    QVector<bool> packCRCsDefined;
    QVector<quint32> packCRCs;
    for (;;) {
        type = readByte(buffer, pos);
        if (type == kEnd) {
            break;
        }
        if (type == kCRC) {
            readHashDigests(numPackStreams, packCRCsDefined, packCRCs, buffer, pos);
            continue;
        }
        skipData(readNumber(buffer, pos), buffer, pos);
    }

    if (packCRCs.isEmpty()) {
        for (quint64 i = 0; i < numPackStreams; ++i) {
            packCRCsDefined.append(false);
            packCRCs.append(0);
        }
    }
    return true;
}

bool K7Zip::K7ZipPrivate::readUnpackInfo(const char* buffer, int& pos)
{
    if (!findAttribute(kFolder, buffer, pos)) {
        qDebug() << "kFolder not found";
        return false;
    }

    int numFolders = readNumber(buffer, pos);
    folders.clear();
    int external = readByte(buffer, pos);
    switch (external) {
    case 0:
    {
        for (int i = 0; i < numFolders; ++i) {
            folders.append(folderItem(buffer, pos));
        }
        break;
    }
    case 1:
    {
        int dataIndex = readNumber(buffer, pos);
        if (dataIndex < 0 /*|| dataIndex >= dataVector->Size()*/) {
            qDebug() << "wrong data index";
        }
        // TODO : go to the new index
        break;
    }
    default:
        qDebug() << "external error";
        return false;
    }


    if(!findAttribute(kCodersUnpackSize, buffer, pos)) {
        qDebug() << "kCodersUnpackSize not found";
        return false;
    }

    for (int i = 0; i < numFolders; ++i) {
        Folder* folder = folders[i];
        int numOutStreams = 0;
        for (int j = 0; j < folder->folderInfos.size(); ++j) {
            numOutStreams += folder->folderInfos[i]->numOutStreams;
        }
        for (int j = 0; j < numOutStreams; ++j) {
            folder->unpackSizes.append(readNumber(buffer, pos));
        }
    }

    for (;;) {
        int type = readByte(buffer, pos);
        if (type == kEnd) {
            break;
        }
        if (type == kCRC) {
            QVector<bool> crcsDefined;
            QVector<quint32> crcs;
            readHashDigests(numFolders, crcsDefined, crcs, buffer, pos);
            for (int i = 0; i < numFolders; i++)
            {
                Folder* folder = folders[i];
                folder->unpackCRCDefined = crcsDefined[i];
                folder->unpackCRC = crcs[i];
            }
            continue;
        }
        skipData(readNumber(buffer, pos), buffer, pos);
    }
    return true;
}

bool K7Zip::K7ZipPrivate::readSubStreamsInfo(const char* buffer, int& pos)
{
    QVector<quint64> numUnpackStreamsInFolders;
    int type;
    for (;;) {
        type = readByte(buffer, pos);
        if (type == kNumUnpackStream) {
            for (int i = 0; i < folders.size(); i++) {
                numUnpackStreamsInFolders.append(readNumber(buffer, pos));
            }
            continue;
        }
        if (type == kCRC || type == kSize)
            break;
        if (type == kEnd)
            break;
        skipData(readNumber(buffer, pos), buffer, pos);
    }

    if (numUnpackStreamsInFolders.isEmpty()) {
        for (int i = 0; i < folders.size(); i++) {
            numUnpackStreamsInFolders.append(1);
        }
    }

    for (int i = 0; i < numUnpackStreamsInFolders.size(); i++)
    {
        quint64 numSubstreams = numUnpackStreamsInFolders[i];
        if (numSubstreams == 0)
            continue;
        quint64 sum = 0;
        for (quint64 j = 1; j < numSubstreams; j++) {
            if (type == kSize)
            {
                int size = readNumber(buffer, pos);
                unpackSizes.append(size);
                sum += size;
            }
        }
        if (!folders[i]->unpackSizes.isEmpty()) {
            for (int j = folders[i]->unpackSizes.size() - 1; j >= 0; j--) {
                bool found = false;
                for (int k = 0; j < folders[i]->inIndexes.size(); ++k) {
                    if (folders[i]->inIndexes[k] == (quint64)i) {
                        found = true;
                        break;
                    }
                }
                if (!found) {
                    unpackSizes.append(folders[i]->unpackSizes[j] - sum);
                }
            }
        }
    }

    if (type == kSize)
        type = readByte(buffer, pos);

    int numDigests = 0;
    int numDigestsTotal = 0;
    for (int i = 0; i < folders.size(); i++)
    {
        quint64 numSubstreams = numUnpackStreamsInFolders[i];
        if (numSubstreams != 1 || !folders[i]->unpackCRCDefined) {
            numDigests += numSubstreams;
        }
        numDigestsTotal += numSubstreams;
    }

    for (;;) {
        if (type == kCRC) {
            QVector<bool> digestsDefined2;
            QVector<quint32> digests2;
            readHashDigests(numDigests, digestsDefined2, digests2, buffer, pos);
            int digestIndex = 0;
            for (int i = 0; i < folders.size(); i++)
            {
                quint64 numSubstreams = numUnpackStreamsInFolders[i];
                const Folder* folder = folders[i];
                if (numSubstreams == 1 && folder->unpackCRCDefined) {
                    digestsDefined.append(true);
                    digests.append(folder->unpackCRC);
                } else {
                    for (quint64 j = 0; j < numSubstreams; j++, digestIndex++) {
                        digestsDefined.append(digestsDefined2[digestIndex]);
                        digests.append(digests2[digestIndex]);
                    }
                }
            }
        } else if (type == kEnd) {
            if (digestsDefined.isEmpty()) {
                for (int i = 0; i < numDigestsTotal; i++) {
                    digestsDefined.append(false);
                    digests.append(0);
                }
            }

            break;
        } else {
            skipData(readNumber(buffer, pos), buffer, pos);
        }

        type = readByte(buffer, pos);
    }
    return true;
}

#define TICKSPERSEC        10000000
#define TICKSPERMSEC       10000
#define SECSPERDAY         86400
#define SECSPERHOUR        3600
#define SECSPERMIN         60
#define EPOCHWEEKDAY       1  /* Jan 1, 1601 was Monday */
#define DAYSPERWEEK        7
#define DAYSPERQUADRICENTENNIUM (365 * 400 + 97)
#define DAYSPERNORMALQUADRENNIUM (365 * 4 + 1)

static time_t toTimeT(const long long liTime)
{
    long long time = liTime / TICKSPERSEC;

    /* The native version of RtlTimeToTimeFields does not take leap seconds
     * into account */

    /* Split the time into days and seconds within the day */
    long int days = time / SECSPERDAY;
    int secondsInDay = time % SECSPERDAY;

    /* compute time of day */
    short hour = (short) (secondsInDay / SECSPERHOUR);
    secondsInDay = secondsInDay % SECSPERHOUR;
    short minute = (short) (secondsInDay / SECSPERMIN);
    short second = (short) (secondsInDay % SECSPERMIN);

    /* compute year, month and day of month. */
    long int cleaps=( 3 * ((4 * days + 1227) / DAYSPERQUADRICENTENNIUM) + 3 ) / 4;
    days += 28188 + cleaps;
    long int years = (20 * days - 2442) / (5 * DAYSPERNORMALQUADRENNIUM);
    long int yearday = days - (years * DAYSPERNORMALQUADRENNIUM)/4;
    long int months = (64 * yearday) / 1959;
    /* the result is based on a year starting on March.
     * To convert take 12 from Januari and Februari and
     * increase the year by one. */

    short month, year;
    if( months < 14 ) {
        month = (short)(months - 1);
        year = (short)(years + 1524);
    } else {
        month = (short)(months - 13);
        year = (short)(years + 1525);
    }
    /* calculation of day of month is based on the wonderful
     * sequence of INT( n * 30.6): it reproduces the·
     * 31-30-31-30-31-31 month lengths exactly for small n's */
    short day = (short)(yearday - (1959 * months) / 64 );

    QDateTime t(QDate(year, month, day), QTime(hour, minute, second));
    t.setTimeSpec(Qt::UTC);
    return  t.toTime_t();
}

bool K7Zip::K7ZipPrivate::readMainStreamsInfo(const char* buffer, int& pos)
{
    quint32 type;
    for (;;) {
        type = readByte(buffer, pos);
        if (type > ((quint32)1 << 30)) {
            qDebug() << "type error";
            return false;
        }
        switch(type)
        {
        case kEnd:
            return true;
        case kPackInfo:
        {
            if (!readPackInfo(buffer, pos)) {
                qDebug() << "error during read pack information";
                return false;
            }
            break;
        }
        case kUnpackInfo:
        {
            if (!readUnpackInfo(buffer, pos)) {
                qDebug() << "error during read pack information";
                return false;
            }
            break;
        }
        case kSubStreamsInfo:
        {
            if (!readSubStreamsInfo(buffer, pos)) {
                qDebug() << "error during read substreams information";
                return false;
            }
            break;
        }
        default:
            qDebug() << "Wrong type";
            return false;
        }
    }

    qDebug() << "should not reach";
    return false;
}

QByteArray K7Zip::K7ZipPrivate::readAndDecodePackedStreams(const char* buffer, int& pos, int& headerSize, bool readMainStreamInfo)
{
    if (readMainStreamInfo)
        readMainStreamsInfo(buffer, pos);

    QByteArray inflatedData;

    int packIndex = 0;
    for (int i = 0; i < folders.size(); i++)
    {
        const Folder* folder = folders[i];
        quint64 unpackSize64 = 0;
        // GetUnpackSize
        if (!folder->unpackSizes.isEmpty()) {
            for (int j = folder->unpackSizes.size() - 1; j >= 0; j--) {
                bool found = false;
                for (int k = 0; k < folder->outIndexes.size(); ++k) {
                    if (folder->outIndexes[j] == folder->unpackSizes[j]) {
                        found = true;
                        break;
                    }
                }
                if (!found) {
                    unpackSize64 = folder->unpackSizes[j];
                    break;
                }
            }
        }
        size_t unpackSize = (size_t)unpackSize64;
        if (unpackSize != unpackSize64) {
            qDebug() << "unsupported";
            return inflatedData;
        }

        qDebug() << "unpackSize" << unpackSize;

        char encodedBuffer[packSizes[packIndex]];

        QString method;
        for (int w = 0; w < folders.size(); ++w) {
            for (int g = 0; g < folders[w]->folderInfos.size(); ++g) {
                Folder::FolderInfo* info = folders[w]->folderInfos[g];
                switch (info->methodID) {
                case k_LZMA:
                   method = QLatin1String("LZMA:16");
                   break;
                case k_AES:
                   break;
                }
            }
        }

        qint64 packPosition = packIndex == 0 ? packPos + 32 /*header size*/ : packPos + packSizes[packIndex-1];

        QIODevice* dev = q->device();
        dev->seek(packPosition);
        quint64 n = dev->read(encodedBuffer, packSizes[packIndex]);
        if ( n != packSizes[packIndex] ) {
            qDebug() << "Failed read next header size, should read " << packSizes[packIndex] << ", read " << n;
            return inflatedData;
        }

        // Create Filter
        KFilterBase* filter = KCompressionDevice::filterForCompressionType(KCompressionDevice::Xz);
        if (!filter) {
            qDebug() << "filter not found";
            return inflatedData;
        }


        filter->init(QIODevice::ReadOnly);
        QByteArray deflatedData(encodedBuffer, packSizes[packIndex]);

        const unsigned char lzmaHeader[13] = {0x5d, 0x00, 0x00, 0x80, 0x00, 0xe5, 0x75, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

        for (int i = 12; i >= 0; --i) {
            deflatedData.prepend(lzmaHeader[i]);
        }

        filter->setInBuffer(deflatedData.data(), deflatedData.size());

        QByteArray outBuffer;
        // reserve memory
        outBuffer.resize(unpackSize);

        KFilterBase::Result result = KFilterBase::Ok;
        while (result != KFilterBase::End && result != KFilterBase::Error && !filter->inBufferEmpty()) {
            filter->setOutBuffer(outBuffer.data(), outBuffer.size());
            result = filter->uncompress();
            if (result == KFilterBase::Error) {
                qDebug() << " decode error";
                return QByteArray();
            }
            int uncompressedBytes = outBuffer.size() - filter->outBufferAvailable();

            // append the uncompressed data to inflate buffer
            inflatedData.append(outBuffer.data(), uncompressedBytes);

            if (result == KFilterBase::End) {
                break; // Finished.
            }
        }

        if (result != KFilterBase::End && !filter->inBufferEmpty()) {
            qDebug() << "decode failed result" << result;
            delete filter;
            return QByteArray();
        }

        filter->terminate();
        delete filter;

        if (folder->unpackCRCDefined) {
            quint32 crc = crc32(0, (Bytef*)(inflatedData.data()),unpackSize);
            if (crc != folder->unpackCRC) {
                qDebug() << "wrong crc";
                return QByteArray();
            }
        }

        for (int j = 0; j < folder->packedStreams.size(); j++)
        {
            quint64 packSize = packSizes[packIndex++];
            pos += packSize;
            headerSize += packSize;
        }
    }
    return inflatedData;
}

bool K7Zip::openArchive( QIODevice::OpenMode mode )
{
    if ( !(mode & QIODevice::ReadOnly) )
        return true;

    d->dirList.clear();
    QIODevice* dev = device();

    char header[32];

    if ( !dev )
        return false;

    // check signature
    qint64 n = dev->read( header, 32 );
    if (n != 32) {
        qDebug() << "read header failed";
        return false;
    }

    for ( int i = 0; i < 6; ++i ) {
        if ( (unsigned char)header[i] != k7zip_signature[i] ) {
            qDebug() << "check signature failed";
            return false;
        }
    }

    // get Archive Version
    int major = header[6];
    int minor = header[7];

    if (major > 0 || minor > 2) {
        qDebug() << "wrong archive version";
        return false;
    }

    // get Start Header CRC
    quint32 startHeaderCRC = GetUi32(header, 8);
    quint64 nextHeaderOffset = GetUi64(header, 12);
    quint64 nextHeaderSize = GetUi64(header, 20);
    quint32 nextHeaderCRC = GetUi32(header, 28);

    quint32 crc = crc32(0, (Bytef*)(header + 0xC), 20);

    if (crc != startHeaderCRC) {
        qDebug() << "bad crc";
        return false;
    }

    if (nextHeaderSize == 0) {
        return true;
    }

    if (nextHeaderSize > (quint64)0xFFFFFFFF) {
        return false;
    }

    if ((qint64)nextHeaderOffset < 0) {
        return false;
    }

    dev->seek(nextHeaderOffset + 32);

    QByteArray inBuffer;
    inBuffer.resize(nextHeaderSize);

    n = dev->read(inBuffer.data(), inBuffer.size());
    char* buffer = inBuffer.data();
    if ( n != (qint64)nextHeaderSize ) {
        qDebug() << "Failed read next header size, should read " << nextHeaderSize << ", read " << n;
        return false;
    }
    d->end = nextHeaderSize;

    int headerSize = 32 + nextHeaderSize;
    //int physSize = 32 + nextHeaderSize + nextHeaderOffset;

    crc = crc32(0, (Bytef*)(buffer), (quint32)nextHeaderSize);

    if (crc != nextHeaderCRC) {
        qDebug() << "bad next header crc";
        return false;
    }

    int cur = 0;
    int type = d->readByte(buffer, cur);
    if (type != kHeader) {
        if (type != kEncodedHeader) {
            qDebug() << "error in header";
            return false;
        }
        QByteArray decodedData = d->readAndDecodePackedStreams(buffer, cur, headerSize);

        QByteArray newHeader;
        newHeader.resize(headerSize);

        dev->seek(cur);
        quint64 n = dev->read(newHeader.data(), newHeader.size());
        if ( n != (qint64) headerSize) {
            qDebug() << "Failed read new header size, should read " << newHeader.size() << ", read " << n;
            return false;
        }
        cur = 0;
        d->end = headerSize;

        int external = d->readByte(buffer, cur);
        if (external != 0) {
            int dataIndex = (int)d->readNumber(buffer, cur);
            if (dataIndex < 0 /*|| dataIndex >= dataVector->Size()*/) {
                qDebug() << "dataIndex error";
            }
            buffer = decodedData.data();
            cur = 0;
            d->end = decodedData.size();
        }

        type = d->readByte(buffer, cur);
        if (type != kHeader) {
            qDebug() << "error type should be kHeader";
            return false;
        }
    }
    // read header
    type = d->readByte(buffer, cur);

    if (type == kArchiveProperties) {
        // TODO : implement this part
        qDebug() << "not implemented";
        return false;
    }

    if (type == kAdditionalStreamsInfo) {
        // TODO : implement this part
        qDebug() << "not implemented";
        return false;
    }

    if (type == kMainStreamsInfo) {
        if (!d->readMainStreamsInfo(buffer, cur)) {
            qDebug() << "error during read main streams information";
            return false;
        }
        type = d->readByte(buffer, cur);
    } else {
        for (int i = 0; i < d->folders.size(); ++i)
        {
            Folder* folder = d->folders[i];
            quint64 unpackSize = 0;

            if (!folder->unpackSizes.isEmpty()) {
                for (int j = folder->unpackSizes.size() - 1; j >= 0; j--) {
                    for(int k = 0; k < folder->outIndexes.size(); k++) {
                        if (folder->outIndexes[k] == folder->unpackSizes[j]) {
                            unpackSize = folder->unpackSizes[j];
                        }
                    }
                }
            }
            d->unpackSizes.append(unpackSize);
            d->digestsDefined.append(folder->unpackCRCDefined);
            d->digests.append(folder->unpackCRC);
        }
    }

    if (type == kEnd) {
        return true;
    }
    if (type != kFilesInfo) {
        qDebug() << "read header error";
        return false;
    }

    //read files info
    int numFiles = d->readNumber(buffer, cur);
    for (int i=0; i < numFiles; ++i) {
        d->fileInfos.append(new FileInfo);
    }

    QVector<bool> emptyStreamVector;
    QVector<bool> emptyFileVector;
    QVector<bool> antiFileVector;
    int numEmptyStreams = 0;

    for (;;)
    {
        quint64 type = d->readByte(buffer, cur);
        if (type == kEnd)
            break;

        quint64 size = d->readNumber(buffer, cur);

        size_t ppp = cur;

        bool addPropIdToList = true;
        bool isKnownType = true;

        if (type > ((quint32)1 << 30)) {
            isKnownType = false;
        } else {
            switch(type)
            {
            case kEmptyStream:
            {
                d->readBoolVector(numFiles, emptyStreamVector, buffer, cur);
                for (int i = 0; i < emptyStreamVector.size(); ++i) {
                    if (emptyStreamVector[i]) {
                        numEmptyStreams++;
                    }
                }

                break;
            }
            case kEmptyFile:
                d->readBoolVector(numEmptyStreams, emptyFileVector, buffer, cur);
                break;
            case kAnti:
                d->readBoolVector(numEmptyStreams, antiFileVector, buffer, cur);
                break;
            case kCTime:
                if(!d->readUInt64DefVector(numFiles, d->cTimes, buffer, cur)) {
                    qDebug() << "error read CTime";
                    return false;
                }
                break;
            case kATime:
                if(!d->readUInt64DefVector(numFiles, d->aTimes, buffer, cur)) {
                    qDebug() << "error read ATime";
                    return false;
                }
                break;
            case kMTime:
                if(!d->readUInt64DefVector(numFiles, d->mTimes, buffer, cur)) {
                    qDebug() << "error read MTime";
                    return false;
                }
                break;
            case kName:
            {
                int external = d->readByte(buffer, cur);
                if (external != 0) {
                    int dataIndex = d->readNumber(buffer, cur);
                    if (dataIndex < 0) {
                       qDebug() << "wrong data index";
                    }

                    // TODO : go to the new index
                }

                QString name;
                for (int i = 0; i < numFiles; i++) {
                    name = d->readString(buffer, cur);
                    d->fileInfos[i]->name = name;
                }
                break;
            }
            case kAttributes:
            {
                QVector<bool> attributesAreDefined;
                d->readBoolVector2(numFiles, attributesAreDefined, buffer, cur);
                int external = d->readByte(buffer, cur);
                if (external != 0) {
                    int dataIndex = d->readNumber(buffer, cur);
                    if (dataIndex < 0) {
                        qDebug() << "wrong data index";
                    }

                    // TODO : go to the new index
                }

                for (int i = 0; i < numFiles; i++)
                {
                    FileInfo* fileInfo = d->fileInfos[i];
                    fileInfo->attribDefined = attributesAreDefined[i];
                    if (fileInfo->attribDefined) {
                        fileInfo->attributes = d->readUInt32(buffer, cur);
                    }
                }
                break;
            }
            case kStartPos:
                if(!d->readUInt64DefVector(numFiles, d->startPositions, buffer, cur)) {
                    qDebug() << "error read MTime";
                    return false;
                }
                break;
            case kDummy:
            {
                for (quint64 i = 0; i < size; i++) {
                    if (d->readByte(buffer, cur) != 0) {
                        qDebug() << "invalid";
                        return false;
                    }
                }
                addPropIdToList = false;
                break;
            }
            default:
                addPropIdToList = isKnownType = false;
            }
        }

        if (isKnownType) {
            if(addPropIdToList) {
                d->fileInfoPopIDs.append(type);
            }
        } else {
            d->skipData(d->readNumber(buffer, cur), buffer, cur);
        }

        bool checkRecordsSize = (major > 0 ||
                                 minor > 2);
        if (checkRecordsSize && cur - ppp != size) {
            qDebug() << "error read size failed";
            return false;
        }
    }

    int emptyFileIndex = 0;
    int sizeIndex = 0;

    int numAntiItems = 0;

    if (emptyStreamVector.isEmpty()) {
        for (int i = 0; i < numFiles; ++i) {
            emptyStreamVector.append(false);
        }
    }

    if (antiFileVector.isEmpty()) {
        for (int i = 0; i < numEmptyStreams; i++) {
            antiFileVector.append(false);
        }
    }
    if (emptyFileVector.isEmpty()) {
        for (int i = 0; i < numEmptyStreams; i++) {
            emptyFileVector.append(false);
        }
    }

    for (int i = 0; i < numEmptyStreams; i++) {
        if (antiFileVector[i]) {
            numAntiItems++;
        }
    }

    int oldPos = 0;
    for (int i = 0; i < numFiles; i++)
    {
        FileInfo* fileInfo = d->fileInfos[i];
        bool isAnti;
        fileInfo->hasStream = !emptyStreamVector[i];
        if (fileInfo->hasStream)
        {
            fileInfo->isDir = false;
            isAnti = false;
            fileInfo->size = d->unpackSizes[sizeIndex];
            fileInfo->crc = d->digests[sizeIndex];
            fileInfo->crcDefined = d->digestsDefined[sizeIndex];
            sizeIndex++;
        } else {
            fileInfo->isDir = !emptyFileVector[emptyFileIndex];
            isAnti = antiFileVector[emptyFileIndex];
            emptyFileIndex++;
            fileInfo->size = 0;
            fileInfo->crcDefined = false;
        }
        if (numAntiItems != 0)
            d->isAnti.append(isAnti);

        QString attr;
        if ((fileInfo->attributes & FILE_ATTRIBUTE_DIRECTORY) != 0 || fileInfo->isDir)
            attr += QLatin1String("D");
        if ((fileInfo->attributes & FILE_ATTRIBUTE_READONLY) != 0)
            attr += QLatin1String("R");
        if ((fileInfo->attributes & FILE_ATTRIBUTE_HIDDEN) != 0)
            attr += QLatin1String("H");
        if ((fileInfo->attributes & FILE_ATTRIBUTE_SYSTEM) != 0)
            attr += QLatin1String("S");
        if ((fileInfo->attributes & FILE_ATTRIBUTE_ARCHIVE) != 0)
            attr += QLatin1String("A");

        quint64 packSize = 0;
        for (int j = 0; j < d->packSizes.size(); j++) {
            packSize += d->packSizes[j];
        }

        QString method;
        for (int w = 0; w < d->folders.size(); ++w) {
            for (int g = 0; g < d->folders[w]->folderInfos.size(); ++g) {
                Folder::FolderInfo* info = d->folders[w]->folderInfos[g];
                switch(info->methodID) {
                case k_LZMA:
                    method = QLatin1String("LZMA:16");
                    break;
                case k_AES:
                    break;
                }
            }
        }

        int access = 0100644;
        if (fileInfo->isDir) {
            access = S_IFDIR | 0755;
        }

        qint64 pos = i == 0 ? 0 : pos + oldPos;
        if (!fileInfo->isDir) {
            oldPos = fileInfo->size;
        }

        KArchiveEntry* e;
        if (fileInfo->isDir) {
            QString path = QDir::cleanPath( fileInfo->name );
            const KArchiveEntry* ent = rootDir()->entry( path );
            if ( ent && ent->isDirectory() ) {
                e = 0;
            } else {
                e = new KArchiveDirectory( this, fileInfo->name.mid(fileInfo->name.lastIndexOf(QLatin1String("/")) + 1 ), access, toTimeT(d->mTimes[i]), rootDir()->user(), rootDir()->group(), QString()/*symlink*/ );
            }
        } else {
            e = new KArchiveFile( this, fileInfo->name.mid(fileInfo->name.lastIndexOf(QLatin1String("/")) + 1), access, toTimeT(d->mTimes[i]), rootDir()->user(), rootDir()->group(), QString()/*symlink*/, pos, fileInfo->size );
        }

        if (e) {
            int spos = fileInfo->name.lastIndexOf(QLatin1String("/"));
            if (spos == -1) {
                rootDir()->addEntry( e );
            } else {
                QString path = QDir::cleanPath( fileInfo->name.left( spos ) );
                KArchiveDirectory * d = findOrCreate( path );
                d->addEntry( e );
            }
        }
    }

    // TODO : move this code in method and share this with readAndDecodePackedStreams

    QByteArray inflatedData;
    int packIndex = 0;
    for (int i = 0; i < d->folders.size(); i++)
    {
        const Folder* folder = d->folders[i];
        quint64 unpackSize64 = 0;
        // GetUnpackSize
        if (!folder->unpackSizes.isEmpty()) {
            for (int j = folder->unpackSizes.size() - 1; j >= 0; j--) {
                bool found = false;
                for (int k = 0; k < folder->outIndexes.size(); ++k) {
                    if (folder->outIndexes[j] == folder->unpackSizes[j]) {
                        found = true;
                        break;
                    }
                }
                if (!found) {
                    unpackSize64 = folder->unpackSizes[j];
                    break;
                }
            }
        }
        size_t unpackSize = (size_t)unpackSize64;
        if (unpackSize != unpackSize64) {
            qDebug() << "unsupported";
            return false;
        }

        char encodedBuffer[d->packSizes[packIndex]];

        QString method;
        for (int w = 0; w < d->folders.size(); ++w) {
            for (int g = 0; g < d->folders[w]->folderInfos.size(); ++g) {
                Folder::FolderInfo* info = d->folders[w]->folderInfos[g];
                switch (info->methodID) {
                case k_LZMA:
                   method = QLatin1String("LZMA:16");
                   break;
                case k_AES:
                   break;
                }
            }
        }

        qint64 packPosition = packIndex == 0 ? d->packPos + 32 /*header size*/ : d->packPos + d->packSizes[packIndex-1];

        QIODevice* dev = device();
        dev->seek(packPosition);
        quint64 n = dev->read(encodedBuffer, d->packSizes[packIndex]);
        if ( n != d->packSizes[packIndex] ) {
            qDebug() << "Failed read next header size, should read " << d->packSizes[packIndex] << ", read " << n;
            return false;
        }

        // Create Filter
        KFilterBase* filter = KCompressionDevice::filterForCompressionType(KCompressionDevice::Xz);
        if (!filter) {
            qDebug() << "filter not found";
            return false;
        }

        filter->init(QIODevice::ReadOnly);
        QByteArray deflatedData(encodedBuffer, d->packSizes[packIndex]);

        const unsigned char lzmaHeader[13] = {0x5d, 0x00, 0x00, 0x80, 0x00, 0xe5, 0x75, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

        for (int i = 12; i >= 0; --i) {
            deflatedData.prepend(lzmaHeader[i]);
        }

        filter->setInBuffer(deflatedData.data(), deflatedData.size());

        QByteArray outBuffer;
        // reserve memory
        outBuffer.resize(unpackSize);

        KFilterBase::Result result = KFilterBase::Ok;
        while (result != KFilterBase::End && result != KFilterBase::Error && !filter->inBufferEmpty()) {
            filter->setOutBuffer(outBuffer.data(), outBuffer.size());
            result = filter->uncompress();
            if (result == KFilterBase::Error) {
                qDebug() << " decode error";
                return false;
            }
            int uncompressedBytes = outBuffer.size() - filter->outBufferAvailable();

            // append the uncompressed data to inflate buffer
            inflatedData.append(outBuffer.data(), uncompressedBytes);

            if (result == KFilterBase::End) {
                break; // Finished.
            }
        }

        filter->terminate();
        delete filter;

        QBuffer* out = new QBuffer();
        QByteArray* array = new QByteArray();
        array->append(inflatedData);
        out->setBuffer(array);
        out->open(QIODevice::ReadOnly);
        setDevice(out);
    }

    return true;
}

bool K7Zip::closeArchive()
{
    return false;
}

bool K7Zip::doFinishWriting( qint64 /*size*/ )
{
    return false;
}

bool K7Zip::doPrepareWriting(const QString &/*name*/, const QString &/*user*/,
                          const QString &/*group*/, qint64 /*size*/, mode_t /*perm*/,
                          time_t /*--atime--*/, time_t /*mtime*/, time_t /*--ctime--*/)
{
    return false;
}

bool K7Zip::doWriteDir(const QString &/*name*/, const QString &/*user*/,
                      const QString &/*group*/, mode_t /*perm*/,
                      time_t /*--atime--*/, time_t /*mtime*/, time_t /*--ctime--*/)
{
    return false;
}

bool K7Zip::doWriteSymLink(const QString &/*name*/, const QString &/*target*/,
                        const QString &/*user*/, const QString &/*group*/,
                        mode_t /*perm*/, time_t /*--atime--*/, time_t /*mtime*/, time_t /*--ctime--*/)
{
    return false;
}

void K7Zip::virtual_hook( int id, void* data ) {
    KArchive::virtual_hook( id, data );
}
