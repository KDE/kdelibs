#ifndef KWRITE_MISC_H
#define KWRITE_MISC_H


class QPainter;

class KWriteDoc;
class KWrite;


class KWCursor {
  public:
    KWCursor() : m_x(0), m_y(0) {}
    KWCursor(int x, int y) : m_x(x), m_y(y) {}
    KWCursor(const KWCursor &c) : m_x(c.m_x), m_y(c.m_y) {}

    KWCursor &operator=(const KWCursor &c) {m_x = c.m_x; m_y = c.m_y; return *this;}
    KWCursor &operator+=(const KWCursor &c) {m_x += c.m_x; m_y += c.m_y; return *this;}
    KWCursor &operator-=(const KWCursor &c) {m_x -= c.m_x; m_y -= c.m_y; return *this;}
    bool operator==(const KWCursor &c) {return m_x == c.m_x && m_y == c.m_y;}
    bool operator!=(const KWCursor &c) {return m_x != c.m_x || m_y != c.m_y;}
    bool operator>(const KWCursor &c) {return m_y > c.m_y || (m_y == c.m_y && m_x > c.m_x);}

    void set(int x, int y) {m_x = x; m_y = y;}
    void setX(int x) {m_x = x;}
    void setY(int y) {m_y = y;}
    int x() const {return m_x;}
    int y() const {return m_y;}
    void incX() {m_x++;}
    void incY() {m_y++;}
    void decX() {m_x--;}
    void decY() {m_y--;}
    void move(int dx, int dy) {m_x += dx; m_y += dy;}
    void moveX(int dx) {m_x += dx;}
    void moveY(int dy) {m_y += dy;}

  protected:
    int m_x;
    int m_y;
};



// with this doubly linked list the line attributes can remove themselves
// from the list safely and without overhead

struct KWListNode {
  void remove();
  KWListNode *next;
  KWListNode *prev;
};

class KWList {
  public:
    typedef KWListNode Node;

    KWList();
    
  protected:
    void insert(Node *newNode, Node *existingNode);

    Node nil; // list starts at nil.left
};

class KWLineAttributeList;

class KWLineAttribute : protected KWListNode {
    friend KWLineAttributeList;
    friend KWriteDoc;
  public:
    KWLineAttribute(int line, KWrite *kWrite = 0L);
    virtual ~KWLineAttribute();
    
    int line() {return m_line;}
    void insLine(int line);
    bool delLine(int line);
    virtual void clear() {}

  protected:
    virtual void paint(QPainter &, int y, int height) = 0;

    virtual void lineChanged(int /*oldLine*/) {}
    virtual bool deleteNotify() {return true;}
    
    int m_line;
    KWrite *m_kWrite;
};

class KWLineAttributeList : public KWList {
  public:
    KWLineAttribute *first();
    KWLineAttribute *next(KWLineAttribute *);
    
    void insert(KWLineAttribute *);
};

#endif // KWRITE_MISC_H
