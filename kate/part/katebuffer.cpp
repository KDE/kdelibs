/* This file is part of the KDE libraries
   Copyright (c) 2000 Waldo Bastian <bastian@kde.org>
   Copyright (C) 2002, 2003 Christoph Cullmann <cullmann@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#include "katebuffer.h"
#include "katebuffer.moc"

#include "katedocument.h"
#include "katehighlight.h"
#include "kateconfig.h"
#include "katecodefoldinghelpers.h"

#include <kvmallocator.h>
#include <kdebug.h>
#include <kglobal.h>
#include <kcharsets.h>

#include <qfile.h>
#include <qtextstream.h>
#include <qtimer.h>
#include <qtextcodec.h>

#include <assert.h>

// SOME LIMITS, may need testing which values are clever
#define AVG_BLOCK_SIZE              32000

/**
  Some private classes
*/
class KateBufFileLoader
{
  public:
    KateBufFileLoader (const QString &m_file) :
      file (m_file), stream (&file), codec (0), prev (0), lastCharEOL (false)
    {
    }

    ~KateBufFileLoader ()
    {
    }

  public:
    QFile file;
    QTextStream stream;
    QTextCodec *codec;
    KateBufBlock *prev;
    bool lastCharEOL;
};

/**
 * The KateBufBlock class contains an amount of data representing
 * a certain number of lines.
 */
class KateBufBlock
{
  friend class KateBuffer;

  public:
   /*
    * Create an empty block.
    */
   KateBufBlock (KateBuffer *parent, KateBufBlock *prev, KVMAllocator *vm);

   ~KateBufBlock ();

   /**
    * Fill block with lines from @p data1 and @p data2.
    * The first line starts at @p data1[@p dataStart].
    * If @p last is true, all bytes from @p data2 are stored.
    * @return The number of bytes stored form @p data2
    */
   bool fillBlock (QTextStream *stream, bool lastCharEOL);

   /**
    * Create a valid stringList.
    * Post Condition: b_stringListValid is true.
    */
   void buildStringList();

   /**
    * Copy stringlist back to raw data.
    * Post Condition: b_rawDataValid is true.
    */
   void flushStringList();

   /**
    * Dispose of a stringList.
    * Post Condition: b_stringListValid is false.
    */
   void disposeStringList();

   /**
    * Dispose of raw data.
    * Post Condition: b_rawDataValid is false.
    */
   void disposeRawData();

   /**
    * Post Condition: b_vmDataValid is true, b_rawDataValid is false
    */
   bool swapOut ();

   /**
    * Swaps raw data from secondary storage.
    * Post Condition: b_rawDataValid is true.
    */
   bool swapIn ();

   /**
    * Dispose of swap data.
    * Post Condition: b_vmDataValid is false.
    */
   void disposeSwap ();

   /**
    * Return line @p i
    * The first line of this block is line 0.
    */
   TextLine::Ptr line(uint i);

   /**
    * Insert @p line in front of line @p i
    */
   void insertLine(uint i, TextLine::Ptr line);

   /**
    * Remove line @p i
    */
   void removeLine(uint i);

   /**
    * First line in block
    */
   inline uint startLine () { return m_startLine; };

   inline void setStartLine (uint line)
   {
     m_startLine = line;
   }

   /**
    * First line behind this block
    */
   inline uint endLine () { return m_startLine + m_lines; }

   /**
    * Lines in this block
    */
   inline uint lines () { return m_lines; }

   inline uint firstLineIndentation () { return m_firstLineIndentation; }
   inline bool firstLineOnlySpaces () { return m_firstLineOnlySpaces; }
   
   inline TextLine::Ptr lastLine () { return m_lastLine; }

  private:
    // IMPORTANT, start line + lines in block
    uint m_startLine;
    uint m_lines;

    // Used for context & hlContinue flag if this bufblock has no stringlist
    uint m_firstLineIndentation;
    bool m_firstLineOnlySpaces;
    TextLine::Ptr m_lastLine;

    // here we swap our stuff
    KVMAllocator *m_vm;
    KVMAllocator::Block *m_vmblock;
    uint m_vmblockSize;
    bool b_vmDataValid;

    /**
     * raw data, if m_codec != 0 it is still the dump file content,
     * if m_codec == 0 it is the dumped string list
     */
    QByteArray m_rawData;
    bool b_rawDataValid;

    /**
     * list of textlines
     */
    TextLine::List m_stringList;
    bool b_stringListValid;

    // Buffer requires highlighting.
    bool b_needHighlight;

    // Parent buffer.
    KateBuffer* m_parent;
};

/**
 * Create an empty buffer. (with one block with one empty line)
 */
KateBuffer::KateBuffer(KateDocument *doc)
 : QObject (doc),
   m_hlUpdate (true),
   m_lines (0),
   m_highlightedTo (0),
   m_highlightedRequested (0),
   m_lastInSyncBlock (0),
   m_highlight (0),
   m_doc (doc),
   m_loader (0),
   m_vm (0),
   m_regionTree (0),
   m_highlightedTill (0),
   m_highlightedEnd (0),
   m_highlightedSteps (0),
   m_cacheReadError(false),
   m_cacheWriteError(false),
   m_loadingBorked (false),
   m_tabWidth (0)
{
  m_blocks.setAutoDelete(true);

  connect( &m_highlightTimer, SIGNAL(timeout()), this, SLOT(pleaseHighlight()));

  clear();
}

/**
 * Cleanup on destruction
 */
KateBuffer::~KateBuffer()
{
  m_blocks.clear ();

  delete m_vm;
  delete m_loader;
}

void KateBuffer::setTabWidth (uint w)
{
  if (m_tabWidth != w)
  {
    m_tabWidth = w;

    if (m_highlight && m_highlight->foldingIndentationSensitive())
      invalidateHighlighting();
  }
}

/**
 * Check if we already have to much loaded stuff
 */
void KateBuffer::checkLoadedMax ()
{
  if (m_loadedBlocks.count() > 40)
  {
    KateBufBlock *buf2 = m_loadedBlocks.take(2);
    bool ok = buf2->swapOut ();
    if (!ok)
    {
       m_cacheWriteError = true;
       m_loadedBlocks.append(buf2);
    }
  }
}

/**
 * Check if we already have to much clean stuff
 */
void KateBuffer::checkCleanMax ()
{
  if (m_cleanBlocks.count() > 10)
  {
    checkLoadedMax ();

    KateBufBlock *buf2 = m_cleanBlocks.take(2);
    buf2->disposeStringList();
    m_loadedBlocks.append(buf2);
  }
}

/**
 * Check if we already have to much dirty stuff
 */
void KateBuffer::checkDirtyMax ()
{
  if (m_dirtyBlocks.count() > 10)
  {
    checkLoadedMax ();

    KateBufBlock *buf2 = m_dirtyBlocks.take(2);
    buf2->flushStringList(); // Copy stringlist to raw
    buf2->disposeStringList(); // dispose stringlist.
    m_loadedBlocks.append(buf2);
  }
}

uint KateBuffer::countVisible ()
{
  return m_lines - m_regionTree->getHiddenLinesCount(m_lines);
}

uint KateBuffer::lineNumber (uint visibleLine)
{
  return m_regionTree->getRealLine (visibleLine);
}

uint KateBuffer::lineVisibleNumber (uint line)
{
  return m_regionTree->getVirtualLine (line);
}

void KateBuffer::lineInfo (KateLineInfo *info, unsigned int line)
{
  m_regionTree->getLineInfo(info,line);
}

KateCodeFoldingTree *KateBuffer::foldingTree ()
{
  return m_regionTree;
}

/**
 * Load the block out of the swap
 */
void KateBuffer::loadBlock(KateBufBlock *buf)
{
  if (m_loadedBlocks.findRef (buf) > -1)
    return;

  // does we have already to much loaded blocks ?
  checkLoadedMax ();

  // swap the data in
  if (!buf->swapIn ())
  {
    m_cacheReadError = true;
    return; // This is bad!
  }

  m_loadedBlocks.append(buf);
}

/**
 * Here we generate the stringlist out of the raw (or swap) data
 */
void KateBuffer::parseBlock(KateBufBlock *buf)
{
  if (m_cleanBlocks.findRef (buf) > -1)
    return;

  // uh, not even loaded :(
  if (!buf->b_rawDataValid)
    loadBlock(buf);

  // does we have already too much clean blocks ?
  checkCleanMax ();

  // now you are clean my little block
  buf->buildStringList();

  m_loadedBlocks.removeRef(buf);
  m_cleanBlocks.append(buf);
}

/**
 * After this call only the string list will be there
 */
void KateBuffer::dirtyBlock(KateBufBlock *buf)
{
  if (m_dirtyBlocks.findRef (buf) > -1)
    return;

  // does we have already to much dirty blocks ?
  checkDirtyMax ();

  // dispose the dirty raw data
  buf->disposeRawData();

  // dispose the swap stuff
  if (buf->b_vmDataValid)
    buf->disposeSwap();

  m_cleanBlocks.removeRef(buf);
  m_dirtyBlocks.append(buf);
}

/**
 * Find a block for line i + correct start/endlines for blocks
 */
KateBufBlock *KateBuffer::findBlock(uint i)
{
  if ((i >= m_lines))
     return 0;

  KateBufBlock *buf = 0;

  if (m_blocks.current() && (int(m_lastInSyncBlock) >= m_blocks.at()))
  {
    buf = m_blocks.current();
  }
  else
  {
    buf = m_blocks.at (m_lastInSyncBlock);
  }

  int lastLine = 0;
  while (buf != 0)
  {
    lastLine = buf->endLine ();

    if (i < buf->startLine())
    {
      // Search backwards
      buf = m_blocks.prev ();
    }
    else if (i >= buf->endLine())
    {
      // Search forwards
      buf = m_blocks.next();
    }
    else
    {
      // We found the block.
      return buf;
    }

    if (buf && (m_blocks.at () > int(m_lastInSyncBlock)) && (int(buf->startLine()) != lastLine))
    {
      buf->setStartLine (lastLine);
      m_lastInSyncBlock = m_blocks.at ();
    }
  }

  return 0;
}

void KateBuffer::clear()
{
  // reset the folding tree hard !
  //  delete m_regionTree;
  // trying to reset the region tree softly
  if (m_regionTree) m_regionTree->clear();
  else
  {
    m_regionTree=new KateCodeFoldingTree(this);
    connect(m_regionTree,SIGNAL(setLineVisible(unsigned int, bool)), this,SLOT(setLineVisible(unsigned int,bool)));
  }

  // cleanup the blocks
  m_cleanBlocks.clear();
  m_dirtyBlocks.clear();
  m_loadedBlocks.clear();
  m_blocks.clear();
  delete m_vm;
  m_vm = new KVMAllocator;
  m_highlight = 0;

  // create a bufblock with one line, we need that, only in openFile we won't have that
  KateBufBlock *block = new KateBufBlock(this, 0, m_vm);
  block->b_rawDataValid = true;
  block->m_rawData.resize (sizeof(uint) + 1);
  char* buf = block->m_rawData.data ();
  uint length = 0;
  memcpy(buf, (char *) &length, sizeof(uint));
  char attr = TextLine::flagNoOtherData;
  memcpy(buf+sizeof(uint), (char *) &attr, 1);
  block->m_lines++;

  m_blocks.append (block);
  m_loadedBlocks.append (block);

  m_lines = block->m_lines;

  m_highlightedTo = 0;
  m_highlightedRequested = 0;
  m_lastInSyncBlock = 0;

  emit linesChanged(m_lines);
}

void KateBuffer::setHighlight(Highlight *highlight)
{
  m_highlight = highlight;
  invalidateHighlighting();
}

/**
 * Insert a file at line @p line in the buffer.
 */
bool KateBuffer::openFile (const QString &m_file)
{
  clear();

  // here we feed the loader with info
  KateBufFileLoader loader (m_file);

  if ( !loader.file.open( IO_ReadOnly ) ||
       (m_file.startsWith("/dev/") && !loader.file.isDirectAccess()) )
  {
    clear();
    return false; // Error
  }

  if (loader.file.isDirectAccess())
  {
    // detect eol
    while (true)
    {
      int ch = loader.file.getch();

      if (ch == -1)
        break;

      if ((ch == '\r'))
      {
        ch = loader.file.getch ();

        if (ch == '\n')
        {
          m_doc->config()->setEol (KateDocumentConfig::eolDos);
          break;
        }
        else
        {
          m_doc->config()->setEol (KateDocumentConfig::eolMac);
          break;
        }
      }
      else if (ch == '\n')
      {
        m_doc->config()->setEol (KateDocumentConfig::eolUnix);
        break;
      }
    }

    if (loader.file.size () > 0)
    {
      loader.file.at (loader.file.size () - 1);

      int ch = loader.file.getch();

      if ((ch == '\n') || (ch == '\r'))
        loader.lastCharEOL = true;
    }

    loader.file.reset ();
  }
  else
  {
    loader.lastCharEOL = true;
    m_doc->config()->setEol (KateDocumentConfig::eolUnix);
  }

  QTextCodec *codec = m_doc->config()->codec();
  loader.stream.setEncoding(QTextStream::RawUnicode); // disable Unicode headers
  loader.stream.setCodec(codec); // this line sets the mapper to the correct codec
  loader.codec = codec;
  loader.prev = 0;

  // trash away the one unneeded already existing block
  m_loadedBlocks.clear();
  m_blocks.clear();
  m_lines = 0;

  // start with not borked
  m_loadingBorked = false;

  // do the real work

  bool eof = false;
  while (true)
  {
    if (loader.stream.atEnd())
      eof = true;

    if (eof)
      break;

    checkLoadedMax ();
    if (m_cacheWriteError)
      break;

    KateBufBlock *block = new KateBufBlock(this, loader.prev, m_vm);
    eof = block->fillBlock (&loader.stream, loader.lastCharEOL);

    m_blocks.append (block);
    m_loadedBlocks.append (block);

    loader.prev = block;
    m_lines = block->endLine ();
  }

  if (m_cacheWriteError)
  {
    m_loadingBorked = true;
  }

  if (m_cacheWriteError)
    kdDebug(13020)<<"Loading failed, no room for temp-file.\n";
  else
    kdDebug(13020)<<"Loading finished.\n";

  // trigger the creation of a block with one line if there is no data in the buffer now
  // THIS IS IMPORTANT, OR CRASH WITH EMPTY FILE
  if (m_blocks.isEmpty() || (count () == 0))
    clear ();
  else
    m_regionTree->fixRoot (m_lines);

  emit linesChanged(m_lines);
  emit loadingFinished ();

  return !m_loadingBorked;
}

bool KateBuffer::canEncode ()
{
  QTextCodec *codec = m_doc->config()->codec();

  kdDebug(13020) << "ENC NAME: " << codec->name() << endl;

  // hardcode some unicode encodings which can encode all chars
  if ((QString(codec->name()) == "UTF-8") || (QString(codec->name()) == "ISO-10646-UCS-2"))
    return true;

  for (uint i=0; i < m_lines; i++)
  {
    if (!codec->canEncode (plainLine(i)->string()))
    {
      kdDebug(13020) << "STRING LINE: " << plainLine(i)->string() << endl;
      kdDebug(13020) << "ENC WORKING: FALSE" << endl;

      return false;
    }
  }

  return true;
}

bool KateBuffer::saveFile (const QString &m_file)
{
  QFile file (m_file);
  QTextStream stream (&file);

  if ( !file.open( IO_WriteOnly ) )
  {
    return false; // Error
  }

  QTextCodec *codec = m_doc->config()->codec();

  // disable Unicode headers
  stream.setEncoding(QTextStream::RawUnicode);

  // this line sets the mapper to the correct codec
  stream.setCodec(codec);

  QString eol = m_doc->config()->eolString ();

  QString tabs;
  if (m_doc->configFlags() & KateDocument::cfReplaceTabs)
    tabs.fill (QChar(' '), m_doc->config()->tabWidth());

  for (uint i=0; i < m_lines; i++)
  {
    // if enabled strip the trailing spaces !
    if (m_doc->configFlags() & KateDocument::cfReplaceTabs)
      stream << textLine (i, m_doc->configFlags() & KateDocument::cfRemoveSpaces).replace (QChar('\t'), tabs);
    else
      stream << textLine (i, m_doc->configFlags() & KateDocument::cfRemoveSpaces);

    if (i < (m_lines-1))
      stream << eol;
  }

  file.close ();

  m_loadingBorked = false;

  return (file.status() == IO_Ok);
}

/**
 * Return line @p i
 */
TextLine::Ptr KateBuffer::line(uint i)
{
  KateBufBlock *buf = findBlock(i);

  if (!buf)
    return 0;

  if (!buf->b_stringListValid)
    parseBlock(buf);

  if (buf->b_needHighlight)
  {
    buf->b_needHighlight = false;

    if (m_highlightedTo > buf->startLine())
    {
      needHighlight (buf, buf->startLine(), buf->endLine());
    }
  }

  if ((m_highlightedRequested <= i) && (m_highlightedTo <= i))
  {
    m_highlightedRequested = buf->endLine();

    pleaseHighlight (m_highlightedTo, buf->endLine());

    // Check again...
    if (!buf->b_stringListValid)
      parseBlock(buf);
  }

  return buf->line (i - buf->m_startLine);
}

bool KateBuffer::needHighlight(KateBufBlock *buf, uint startLine, uint endLine)
{
  // no hl around, no stuff to do
  if (!m_highlight)
    return false;

  // nothing to update, still up to date ;)
  if (!m_hlUpdate)
    return false;
    
  // we tried to start in a line behind this buf block !
  if (startLine >= (buf->startLine()+buf->lines()))
    return false;
    
  // parse this block if needed, very important !
  if (!buf->b_stringListValid)
    parseBlock(buf);

  // get the previous line, if we start at the beginning of this block
  // take the last line of the previous block
  TextLine::Ptr prevLine = 0;
  
  if (startLine == buf->startLine())
  {
    int pos = m_blocks.findRef (buf);
    if (pos > 0)
    {
      KateBufBlock *blk = m_blocks.at (pos-1);

      if (blk->b_stringListValid && (blk->lines() > 0))
        prevLine = blk->line (blk->lines() - 1);
      else
        prevLine = blk->lastLine();
    }
  }
  else if ((startLine > buf->startLine()) && (startLine <= buf->endLine()))
  {
    prevLine = buf->line(startLine - buf->startLine() - 1);
  }
  
  if (!prevLine)
    prevLine = new TextLine ();
  
  bool line_continue = prevLine->hlLineContinue();
  
  QMemArray<short> ctxNum, endCtx;
  ctxNum.duplicate (prevLine->ctxArray ()); 

  // does we need to emit a signal for the folding changes ?
  bool codeFoldingUpdate = false;
  
  // here we are atm, start at start line in the block
  uint current_line = startLine - buf->startLine();
  
  // does we need to continue
  bool stillcontinue=false;
   
  // loop over the lines of the block, from startline to endline or end of block
  // if stillcontinue forces us to do so
  while ( (current_line < buf->lines())
          && (stillcontinue || ((current_line + buf->startLine()) <= endLine)) )
  {    
    // current line  
    TextLine::Ptr textLine = buf->line(current_line);
  
    endCtx.duplicate (textLine->ctxArray ());

    QMemArray<signed char> foldingList;
    m_highlight->doHighlight(ctxNum, textLine, line_continue, &foldingList);

    //
    // indentation sensitive folding
    //
    bool indentChanged = false;
    if (m_highlight->foldingIndentationSensitive())
    {
      // get the indentation array of the previous line to start with !
      QMemArray<unsigned short> indentDepth;
      indentDepth.duplicate (prevLine->indentationDepthArray());

      // current indentation of this line      
      uint iDepth = textLine->indentDepth(m_tabWidth);

      // this line is empty, beside spaces, use indentation depth of the previous line !
      if (textLine->firstChar() == -1)
      {
        // do this to get skipped empty lines indent right, which was given in the indenation array
        if (!prevLine->indentationDepthArray().isEmpty())
          iDepth = (prevLine->indentationDepthArray())[prevLine->indentationDepthArray().size()-1];
        else
          iDepth = prevLine->indentDepth(m_tabWidth);
        
        // run over empty lines ;)
        indentChanged = true;
      }
       
      // query the next line indentation, if we are at the end of the block
      // use the first line of the next buf block
      uint nextLineIndentation = 0;
      
      if ((current_line+1) < buf->lines())
      {
        if (buf->line(current_line+1)->firstChar() == -1)
          nextLineIndentation = iDepth;
        else
          nextLineIndentation = buf->line(current_line+1)->indentDepth(m_tabWidth);
      }
      else
      {
        int pos = m_blocks.findRef (buf);
        if (uint(pos + 1) < m_blocks.count())
        {
          KateBufBlock *blk = m_blocks.at (pos+1);
    
          if (blk->b_stringListValid && (blk->lines() > 0))
          {
            if (blk->line (0)->firstChar() == -1)
              nextLineIndentation = iDepth;
            else
              nextLineIndentation = blk->line (0)->indentDepth(m_tabWidth);
          }
          else
          {
            if (blk->firstLineOnlySpaces())
              nextLineIndentation = iDepth;
            else
              nextLineIndentation = blk->firstLineIndentation();
          }
        }
      }

      // recalculate the indentation array for this line, query if we have to add
      // a new folding start, this means newIn == true !
      bool newIn = false;
      if ((iDepth > 0) && (indentDepth.isEmpty() || (indentDepth[indentDepth.size()-1] < iDepth)))
      {
        indentDepth.resize (indentDepth.size()+1);
        indentDepth[indentDepth.size()-1] = iDepth;
        newIn = true;
      }
      else
      {
        for (int z=indentDepth.size()-1; z > -1; z--)
        {
          if (indentDepth[z] > iDepth)
            indentDepth.resize (z);
          else if (indentDepth[z] == iDepth)
            break;
          else if (indentDepth[z] < iDepth)
          {
            indentDepth.resize (indentDepth.size()+1);
            indentDepth[indentDepth.size()-1] = iDepth;
            newIn = true;
            break;
          }
        }
      }
      
      // just for debugging always true to start with !
      indentChanged = indentChanged || (indentDepth.size() != textLine->indentationDepthArray().size())
                                    || (indentDepth != textLine->indentationDepthArray());
 
      // set the new array in the textline !
      textLine->setIndentationDepth (indentDepth);
      
      // add folding start to the list !
      if (newIn)
      {
        foldingList.resize (foldingList.size() + 1);
        foldingList[foldingList.size()-1] = 1;
      }
      
      // calculate how much end folding symbols must be added to the list !
      // remIn gives you the count of them
      uint remIn = 0;
      
      for (int z=indentDepth.size()-1; z > -1; z--)
      {
        if (indentDepth[z] > nextLineIndentation)
          remIn++;
        else
          break;
      }

      if (remIn > 0)
      {
        foldingList.resize (foldingList.size() + remIn);

        for (uint z= foldingList.size()-remIn; z < foldingList.size(); z++)
          foldingList[z] = -1;
      }
    }

    bool foldingChanged = (foldingList.size() != textLine->foldingListArray().size())
                          || (foldingList != textLine->foldingListArray());

    if (foldingChanged)
      textLine->setFoldingList(foldingList);

    bool retVal_folding = false;
    m_regionTree->updateLine(current_line + buf->startLine(), &foldingList, &retVal_folding, foldingChanged);

    codeFoldingUpdate = codeFoldingUpdate | retVal_folding;

    line_continue=textLine->hlLineContinue();

    ctxNum.duplicate (textLine->ctxArray());

    if ( indentChanged || (endCtx.size() != ctxNum.size()) )
    {
      stillcontinue = true;
    }
    else
    {
      stillcontinue = false;

      if ((ctxNum != endCtx))
        stillcontinue = true;
    }

    // move around the lines
    prevLine = textLine;
    
    // increment line
    current_line++;
  }

  // tag the changed lines !
  emit tagLines (startLine, current_line + buf->startLine());

  // emit that we have changed the folding
  if (codeFoldingUpdate)
    emit codeFoldingUpdated();

  // if we are at the last line of the block + we still need to continue
  // return the need of that !
  return stillcontinue && ((current_line+1) == buf->lines());
}

void KateBuffer::updateHighlighting(uint from, uint to, bool invalidate)
{
   //kdDebug()<<"KateBuffer::updateHighlight("<<from<<","<<to<<","<<invalidate<<")"<<endl;
   if (!m_hlUpdate)
    return;

   //kdDebug()<<"passed the no update check"<<endl;

   if (from > m_highlightedTo )
     from = m_highlightedTo;

   uint done = 0;
   bool endStateChanged = true;

   while (done < to)
   {
      KateBufBlock *buf = findBlock(from);
      if (!buf)
         return;

      if (!buf->b_stringListValid)
      {
         parseBlock(buf);
      }

      if (buf->b_needHighlight || invalidate || m_highlightedTo < buf->endLine())
      {
         uint fromLine = buf->startLine();
         uint tillLine = buf->endLine();

         if (!buf->b_needHighlight && invalidate)
         {
            if (to < tillLine)
               tillLine = to;

            if (from > fromLine)
            {
               if (m_highlightedTo > from)
                 fromLine = from;
               else if (m_highlightedTo > fromLine)
                 fromLine = m_highlightedTo;
            }
         }

         buf->b_needHighlight = false;

   //kdDebug()<<"Calling need highlight: "<<fromLine<<","<<tillLine<<endl;
         endStateChanged = needHighlight (buf, fromLine, tillLine);

          if (buf->b_rawDataValid)
          {
            dirtyBlock(buf);
          }
      }

      done = buf->endLine();
      from = done;
   }
   if (invalidate)
   {
      if (endStateChanged)
         m_highlightedTo = done;
      m_highlightedRequested = done;
   }
   else
   {
      if (done > m_highlightedTo)
         m_highlightedTo = done;
   }
}

void KateBuffer::invalidateHighlighting()
{
   m_highlightedTo = 0;
   m_highlightedRequested = 0;
}

void KateBuffer::pleaseHighlight(uint from, uint to)
{
  if (to > m_highlightedEnd)
    m_highlightedEnd = to;

  if (m_highlightedEnd < from)
    return;

  //
  // this calc makes much of the responsiveness
  //
  m_highlightedSteps = ((m_highlightedEnd-from) / 5) + 1;
  if (m_highlightedSteps < 100)
    m_highlightedSteps = 100;
  else if (m_highlightedSteps > 2000)
    m_highlightedSteps = 2000;

  uint till = from + m_highlightedSteps;
  if (till > m_highlightedEnd)
    till = m_highlightedEnd;

  updateHighlighting(from, till, false);

  m_highlightedTill = till;
  if (m_highlightedTill >= m_highlightedEnd)
  {
    m_highlightedTill = 0;
    m_highlightedEnd = 0;
    m_highlightTimer.stop();
  }
  else
  {
    m_highlightTimer.start(100, true);
  }
}

void KateBuffer::pleaseHighlight()
{
  uint till = m_highlightedTill + m_highlightedSteps;

  if (m_highlightedSteps == 0)
    till += 100;

  if (m_highlightedEnd > m_lines)
    m_highlightedEnd = m_lines;

  if (till > m_highlightedEnd)
    till = m_highlightedEnd;

  updateHighlighting(m_highlightedTill, till, false);

  m_highlightedTill = till;
  if (m_highlightedTill >= m_highlightedEnd)
  {
    m_highlightedTill = 0;
    m_highlightedEnd = 0;
    m_highlightedSteps = 0;
    m_highlightTimer.stop();
  }
  else
  {
    m_highlightTimer.start(100, true);
  }
}

/**
 * Return line @p i without triggering highlighting
 */
TextLine::Ptr KateBuffer::plainLine(uint i)
{
   KateBufBlock *buf = findBlock(i);
   if (!buf)
      return 0;

   if (!buf->b_stringListValid)
   {
      parseBlock(buf);
   }

   return buf->line(i - buf->m_startLine);
}

/**
 * Return text from line @p i without triggering highlighting
 */
QString KateBuffer::textLine(uint i, bool withoutTrailingSpaces)
{
   KateBufBlock *buf = findBlock(i);
   if (!buf)
      return QString();

   if (!buf->b_stringListValid)
   {
      parseBlock(buf);
   }

   if (withoutTrailingSpaces)
     return buf->line(i - buf->startLine())->withoutTrailingSpaces();

   return buf->line(i - buf->startLine())->string();
}

void KateBuffer::insertLine(uint i, TextLine::Ptr line)
{
  //kdDebug()<<"bit debugging"<<endl;
  //kdDebug()<<"bufferblock count: "<<m_blocks.count()<<endl;

   KateBufBlock *buf;
   if (i == m_lines)
      buf = findBlock(i-1);
   else
      buf = findBlock(i);

  if (!buf)
    return;

   if (!buf->b_stringListValid)
      parseBlock(buf);

   if (buf->b_rawDataValid)
      dirtyBlock(buf);

   buf->insertLine(i -  buf->startLine(), line);

   if (m_highlightedTo > i)
      m_highlightedTo++;
   m_lines++;

   if (int(m_lastInSyncBlock) > m_blocks.findRef (buf))
     m_lastInSyncBlock = m_blocks.findRef (buf);

   m_regionTree->lineHasBeenInserted (i);
}

void
KateBuffer::removeLine(uint i)
{
   KateBufBlock *buf = findBlock(i);
   assert(buf);
   if (!buf->b_stringListValid)
   {
      parseBlock(buf);
   }
   if (buf->b_rawDataValid)
   {
      dirtyBlock(buf);
   }

  buf->removeLine(i -  buf->startLine());

  if (m_highlightedTo > i)
    m_highlightedTo--;

  m_lines--;

  // trash away a empty block
  if (buf->lines() == 0)
  {
    if ((m_lastInSyncBlock > 0) && (int(m_lastInSyncBlock) >= m_blocks.findRef (buf)))
      m_lastInSyncBlock = m_blocks.findRef (buf) -1;

    m_cleanBlocks.removeRef(buf);
    m_dirtyBlocks.removeRef(buf);
    m_loadedBlocks.removeRef(buf);
    m_blocks.removeRef(buf);
  }
  else
  {
    if (int(m_lastInSyncBlock) > m_blocks.findRef (buf))
      m_lastInSyncBlock = m_blocks.findRef (buf);
  }

  m_regionTree->lineHasBeenRemoved (i);
}

void KateBuffer::changeLine(uint i)
{
  ////kdDebug(13020)<<"changeLine "<< i<<endl;
  KateBufBlock *buf = findBlock(i);
  assert(buf);
  assert(buf->b_stringListValid);

  if (buf->b_rawDataValid)
  {
    dirtyBlock(buf);
  }
}

void KateBuffer::setLineVisible(unsigned int lineNr, bool visible)
{
   kdDebug(13000)<<"void KateBuffer::setLineVisible(unsigned int lineNr, bool visible)"<<endl;
   TextLine::Ptr l=plainLine(lineNr);
   if (l)
   {
     l->setVisible(visible);
     changeLine (lineNr);
   }
   else
   kdDebug(13000)<<QString("Invalid line %1").arg(lineNr)<<endl;
}

uint KateBuffer::length ()
{
  uint l = 0;

  for (uint i = 0; i < count(); i++)
  {
    l += plainLine(i)->length();
  }

  return l;
}

int KateBuffer::lineLength ( uint i )
{
  TextLine::Ptr l = plainLine(i);
  Q_ASSERT(l);
  if (!l) return 0;
  return l->length();
}

QString KateBuffer::text()
{
  QString s;

  for (uint i = 0; i < count(); i++)
  {
    s.append (textLine(i));
    if ( (i < (count()-1)) )
      s.append('\n');
  }

  return s;
}

QString KateBuffer::text ( uint startLine, uint startCol, uint endLine, uint endCol, bool blockwise )
{
  if ( blockwise && (startCol > endCol) )
    return QString ();

  QString s;

  if (startLine == endLine)
  {
    if (startCol > endCol)
      return QString ();

    TextLine::Ptr textLine = plainLine(startLine);

    if ( !textLine )
      return QString ();

    return textLine->string(startCol, endCol-startCol);
  }
  else
  {
    for (uint i = startLine; (i <= endLine) && (i < count()); i++)
    {
      TextLine::Ptr textLine = plainLine(i);

      if ( !blockwise )
      {
        if (i == startLine)
          s.append (textLine->string(startCol, textLine->length()-startCol));
        else if (i == endLine)
          s.append (textLine->string(0, endCol));
        else
          s.append (textLine->string());
      }
      else
      {
        s.append (textLine->string (startCol, endCol - startCol));
      }

      if ( i < endLine )
        s.append('\n');
    }
  }

  return s;
}

void KateBuffer::dumpRegionTree()
{
  m_regionTree->debugDump();
}

//-----------------------------------------------------------------

/**
 * The KateBufBlock class contains an amount of data representing
 * a certain number of lines.
 */

/**
 * Create an empty block.
 */
KateBufBlock::KateBufBlock(KateBuffer *parent, KateBufBlock *prev, KVMAllocator *vm)
: m_firstLineIndentation (0),
  m_firstLineOnlySpaces (true),
  m_lastLine (0),
  m_vm (vm),
  m_vmblock (0),
  m_vmblockSize (0),
  b_vmDataValid (false),
  b_rawDataValid (false),
  b_stringListValid (false),
  b_needHighlight (true),
  m_parent (parent)
{
  if (prev)
    m_startLine = prev->endLine ();
  else
    m_startLine = 0;

  m_lines = 0;
}

/**
 * Cleanup ;)
 */
KateBufBlock::~KateBufBlock ()
{
  if (b_vmDataValid)
    disposeSwap ();
}

/**
 * Fill block with unicode data from stream
 */
bool KateBufBlock::fillBlock (QTextStream *stream, bool lastCharEOL)
{
  bool eof = false;
  uint lines = 0;

  m_rawData.resize (AVG_BLOCK_SIZE);
  char *buf = m_rawData.data ();
  uint pos = 0;
  char attr = TextLine::flagNoOtherData;

  uint size = 0;
  while (size < AVG_BLOCK_SIZE)
  {
    QString line = stream->readLine();

    if (!(!lastCharEOL && stream->atEnd() && line.isNull()))
    {
      uint length = line.length ();
      size = pos + sizeof(uint) + (sizeof(QChar)*length) + 1;

      if (size > m_rawData.size ())
      {
        m_rawData.resize (size);
        buf = m_rawData.data ();
      }

      memcpy(buf+pos, (char *) &length, sizeof(uint));
      pos += sizeof(uint);

      if (!line.isNull())
      {
        memcpy(buf+pos, (char *) line.unicode(), sizeof(QChar)*length);
        pos += sizeof(QChar)*length;
      }

      memcpy(buf+pos, (char *) &attr, 1);
      pos += 1;

      lines++;
    }

    if (stream->atEnd() && line.isNull())
    {
      eof = true;
      break;
    }
  }

  if (pos < m_rawData.size())
  {
    m_rawData.resize (size);
  }

  m_lines = lines;
  b_rawDataValid = true;

  return eof;
}

/**
 * Swaps raw data to secondary storage.
 * Uses the filedescriptor @p swap_fd and the file-offset @p swap_offset
 * to store m_rawSize bytes.
 */
bool KateBufBlock::swapOut ()
{
  //kdDebug(13020)<<"KateBufBlock: swapout this ="<< this<<endl;
  assert(b_rawDataValid);

  if (!b_vmDataValid)
  {
    m_vmblock = m_vm->allocate(m_rawData.count());
    m_vmblockSize = m_rawData.count();

    if (!m_rawData.isEmpty())
    {
        bool ok = m_vm->copyBlock(m_vmblock, m_rawData.data(), 0, m_rawData.count());
        if (!ok)
           return false;
    }

    b_vmDataValid = true;
  }
  disposeRawData();
  return true;
}

/**
 * Swaps m_rawSize bytes in from offset m_vmDataOffset in the file
 * with file-descriptor swap_fd.
 */
bool KateBufBlock::swapIn ()
{
  //kdDebug(13020)<<"KateBufBlock: swapin this ="<< this<<endl;
  assert(b_vmDataValid);
  assert(!b_rawDataValid);
  assert(m_vmblock);
  m_rawData.resize(m_vmblockSize);
  bool ok = m_vm->copyBlock(m_rawData.data(), m_vmblock, 0, m_vmblockSize);
  if (!ok)
      return false;
  b_rawDataValid = true;
  return true;
}

/**
 * Create a valid stringList.
 */
void KateBufBlock::buildStringList()
{
  //kdDebug(13020)<<"KateBufBlock: buildStringList this ="<< this<<endl;
  assert(m_stringList.empty());

  char *buf = m_rawData.data();
  char *end = buf + m_rawData.count();

  while(buf < end)
  {
    TextLine::Ptr textLine = new TextLine ();
    buf = textLine->restore (buf);
    m_stringList.push_back (textLine);
  }

  //kdDebug(13020)<<"stringList.count = "<< m_stringList.size()<<" should be "<< (m_endState.lineNr - m_beginState.lineNr) <<endl;

  if (m_lines > 0)
  {
    m_lastLine = m_stringList[m_lines - 1];
  }
  else
  {
    m_lastLine = 0;
  }
  
  m_firstLineIndentation = 0;
  m_firstLineOnlySpaces = true;
  
  assert(m_stringList.size() == m_lines);
  b_stringListValid = true;
  //kdDebug(13020)<<"END: KateBufBlock: buildStringList LINES: "<<m_endState.lineNr - m_beginState.lineNr<<endl;
}

/**
 * Flush string list
 * Copies a string list back to the raw buffer.
 */
void KateBufBlock::flushStringList()
{
  //kdDebug(13020)<<"KateBufBlock: flushStringList this ="<< this<<endl;
  assert(b_stringListValid);
  assert(!b_rawDataValid);
  
  // Calculate size.
  uint size = 0;
  for(TextLine::List::const_iterator it = m_stringList.begin(); it != m_stringList.end(); ++it)
    size += (*it)->dumpSize ();

  m_rawData.resize (size);
  char *buf = m_rawData.data();

  // Dump textlines
  for(TextLine::List::iterator it = m_stringList.begin(); it != m_stringList.end(); ++it)
    buf = (*it)->dump (buf);

  assert(buf-m_rawData.data() == (int)size);
  b_rawDataValid = true;
}

/**
 * Dispose of a stringList.
 */
void KateBufBlock::disposeStringList()
{
  //kdDebug(13020)<<"KateBufBlock: disposeStringList this = "<< this<<endl;
  assert(b_rawDataValid || b_vmDataValid);
  
  if (m_lines > 0)
  {
    m_firstLineIndentation = m_stringList[0]->indentDepth (m_parent->tabWidth());
    m_firstLineOnlySpaces = (m_stringList[0]->firstChar() == -1);
    m_lastLine = m_stringList[m_lines - 1];
  }
  else
  {
    m_firstLineIndentation = 0;
    m_firstLineOnlySpaces = true;
    m_lastLine = 0;
  }

  m_stringList.clear();
  b_stringListValid = false;
}

/**
 * Dispose of raw data.
 */
void KateBufBlock::disposeRawData()
{
  //kdDebug(13020)<< "KateBufBlock: disposeRawData this = "<< this<<endl;
  assert(b_stringListValid || b_vmDataValid);
  b_rawDataValid = false;
  m_rawData.resize (0);
}

/**
 * Dispose of data in vm
 */
void KateBufBlock::disposeSwap()
{
  if (m_vmblock)
    m_vm->free(m_vmblock);

  m_vmblock = 0;
  m_vmblockSize = 0;
  b_vmDataValid = false;
}

/**
 * Return line @p i
 * The first line of this block is line 0.
 */
TextLine::Ptr KateBufBlock::line(uint i)
{
  assert(b_stringListValid);
  assert(i < m_stringList.size());

  return m_stringList[i];
}

void KateBufBlock::insertLine(uint i, TextLine::Ptr line)
{
  assert(b_stringListValid);
  assert(i <= m_stringList.size());

  m_stringList.insert (m_stringList.begin()+i, line);
  m_lines++;
}

void KateBufBlock::removeLine(uint i)
{
  assert(b_stringListValid);
  assert(i < m_stringList.size());

  m_stringList.erase (m_stringList.begin()+i);
  m_lines--;
}

// kate: space-indent on; indent-width 2; replace-tabs on;
