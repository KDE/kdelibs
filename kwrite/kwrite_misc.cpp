#include <stdio.h>

#include "kwrite_misc.h"

// KWListNode

void KWListNode::remove() {
  next->prev = prev;
  prev->next = next;
  next = prev = this;
}


// KWList

KWList::KWList() {
  nil.next = &nil;
  nil.prev = &nil;
}

void KWList::insert(Node *newNode, Node *existingNode) {
  newNode->next = existingNode;
  newNode->prev = existingNode->prev;
  existingNode->prev->next = newNode;
  existingNode->prev = newNode;
}


// KWLineAttribute

KWLineAttribute::KWLineAttribute(int line, KWrite *kWrite) {
  next = prev = this;
  m_line = line;
  m_kWrite = kWrite;
}

KWLineAttribute::~KWLineAttribute() {
  remove(); // removes itself from the list
}

void KWLineAttribute::insLine(int line) {
//printf("insLine m_line %d line %d\n", m_line, line);
  if (m_line >= line) {
    m_line++;
    lineChanged(m_line - 1);
  }  
}

bool KWLineAttribute::delLine(int line) {
//printf("delLine m_line %d line %d\n", m_line, line);
  if (m_line > line) {
    m_line--;
    lineChanged(m_line + 1);
  } else if (m_line == line) {
    return deleteNotify();
  }
  return false;
}

KWLineAttribute *KWLineAttributeList::first() {
  return (nil.next == &nil) ? 0L : (KWLineAttribute *) nil.next;
}

KWLineAttribute *KWLineAttributeList::next(KWLineAttribute *a) {
  return (a->next == &nil) ? 0L : (KWLineAttribute *) a->next;
}

void KWLineAttributeList::insert(KWLineAttribute *a) {
  KWList::Node *current = nil.next;
  
  // attributes are ordered by line number, search insert position
  while (current != &nil) {
    if (((KWLineAttribute *) current)->line() > a->line()) break;
    current = current->next;
  }
  KWList::insert(a, current);
}


/*
// KWPixmapLineAttribute

KWPixmapLineAttribute::KWPixmapLineAttribute(int line, const QPixmap &pixmap, 
  KWView *view = 0L) 
  : KWLineAttribute(line, view), m_pixmap(pixmap) {
}

void KWPixmapLineAttribute::paint(QPainter &paint, int x, int y) {
  paint.drawPixmap(x, y, m_pixmap);
}
*/
