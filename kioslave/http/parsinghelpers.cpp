/* This file is part of the KDE libraries
    Copyright (C) 2008 Andreas Hartmetz <ahartmetz@gmail.com>

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
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/


// Advance *pos beyond spaces / tabs
static void skipSpace(const char input[], int *pos, int end)
{
    int idx = *pos;
    while (idx < end && (input[idx] == ' ' || input[idx] == '\t')) {
        idx++;
    }
    *pos = idx;
    return;
}

// Advance *pos beyond anything not space/tab/CR/LF
static void skipNonSpace(const char input[], int *pos, int end)
{
    int idx = *pos;
    while (idx < end && input[idx] != ' ' && input[idx] != '\t' &&
                        input[idx] != '\r' && input[idx] != '\n') {
        idx++;
    }
    *pos = idx;
    return;
}

// Advance *pos to start of next line while being forgiving about line endings.
// Return false if the end of the header has been reached, true otherwise.
static bool nextLine(const char input[], int *pos, int end)
{
    int idx = *pos;
    while (idx < end && input[idx] != '\r' && input[idx] != '\n') {
        idx++;
    }
    int rCount = 0;
    int nCount = 0;
    while (idx < end && qMax(rCount, nCount) < 2 && (input[idx] == '\r' || input[idx] == '\n')) {
        input[idx] == '\r' ? rCount++ : nCount++;
        idx++;
    }
    if (idx < end && qMax(rCount, nCount) == 2 && qMin(rCount, nCount) == 1) {
        // if just one of the others is missing eat it too.
        // this ensures that conforming headers using the proper
        // \r\n sequence (and also \n\r) will be parsed correctly.
        if ((rCount == 1 && input[idx] == '\r') || (nCount == 1 && input[idx] == '\n')) {
            idx++;
        }
    }

    *pos = idx;
    return idx < end && rCount < 2 && nCount < 2;
}

//Return true if the term was found, false otherwise. Advance *pos.
//If (*pos + strlen(term) >= end) just advance *pos to end and return false.
//This means that users should always search for the shortest terms first.
static bool consume(const char input[], int *pos, int end, const char *term)
{
    // note: gcc/g++ is quite good at optimizing away redundant strlen()s
    int idx = *pos;
    if (idx + (int)strlen(term) >= end) {
        *pos = end;
        return false;
    }
    if (strncasecmp(&input[idx], term, strlen(term)) == 0) {
        *pos = idx + strlen(term);
        return true;
    }
    return false;
}


QByteArray TokenIterator::next()
{
    QPair<int, int> token = m_tokens[m_currentToken++];
    //fromRawData brings some speed advantage but also the requirement to keep the text buffer
    //around. this together with implicit sharing (you don't know where copies end up)
    //is dangerous!
    //return QByteArray::fromRawData(&m_buffer[token.first], token.second - token.first);
    return QByteArray(&m_buffer[token.first], token.second - token.first);
}

QByteArray TokenIterator::current() const
{
    QPair<int, int> token = m_tokens[m_currentToken - 1];
    //return QByteArray::fromRawData(&m_buffer[token.first], token.second - token.first);
    return QByteArray(&m_buffer[token.first], token.second - token.first);
}

QList<QByteArray> TokenIterator::all() const
{
    QList<QByteArray> ret;
    for (int i = 0; i < m_tokens.count(); i++) {
        QPair<int, int> token = m_tokens[i];
        ret.append(QByteArray(&m_buffer[token.first], token.second - token.first));
    }
    return ret;
}


HeaderTokenizer::HeaderTokenizer(char *buffer)
 : m_buffer(buffer)
{
    // add information about available headers and whether they have one or multiple,
    // comma-separated values.
    
    //The following response header fields are from RFC 2616 unless otherwise specified.
    //Hint: search the web for e.g. 'http "accept-ranges header"' to find information about
    //a header field.
    static const HeaderFieldTemplate headerFieldTemplates[] = {
        {"accept-ranges", false},
        {"cache-control", true},
        {"connection", true},
        {"content-disposition", false}, //is multi-valued in a way, but with ";" separator!
        {"content-encoding", true},
        {"content-language", true},
        {"content-length", false},
        {"content-location", false},
        {"content-md5", false},
        {"content-type", false},
        {"date", false},
        {"dav", true}, //RFC 2518
        {"etag", false},
        {"expires", false},
        {"keep-alive", false}, //RFC 2068
        {"last-modified", false},
        {"link", false}, //RFC 2068, multi-valued with ";" separator
        {"location", false},
        {"p3p", true}, // http://www.w3.org/TR/P3P/
        {"pragma", true},
        {"proxy-authenticate", false}, //complicated multi-valuedness: quoted commas don't separate
                                       //multiple values. we handle this at a higher level.
        {"proxy-connection", true}, //inofficial but well-known; to avoid misunderstandings
                                    //when using "connection" when talking to a proxy.
        {"refresh", false}, //not sure, only found some mailing list posts mentioning it
        {"set-cookie", false}, //RFC 2109; the multi-valuedness seems to be usually achieved
                               //by sending several instances of this field as opposed to
                               //usually comma-separated lists with maybe multiple instances.
        {"transfer-encoding", true},
        {"upgrade", true},
        {"warning", true},
        {"www-authenticate", false} //see proxy-authenticate
    };

    for (uint i = 0; i < sizeof(headerFieldTemplates) / sizeof(HeaderFieldTemplate); i++) {
        const HeaderFieldTemplate &ft = headerFieldTemplates[i];
        insert(QByteArray(ft.name), HeaderField(ft.isMultiValued));
    }
}

int HeaderTokenizer::tokenize(int begin, int end)
{
    char *buf = m_buffer;  //keep line length in check :/
    int idx = begin;
    int startIdx = begin; //multi-purpose start of current token
    bool multiValuedEndedWithComma = false; //did the last multi-valued line end with a comma?
    QByteArray headerKey;
    do {
        
        if (buf[idx] == ' ' || buf [idx] == '\t') {
            // line continuation; preserve startIdx except (see below)
            if (headerKey.isEmpty()) {
                continue;
            }
            // turn CR/LF into spaces for later parsing convenience
            int backIdx = idx - 1;
            while (backIdx >= begin && (buf[backIdx] == '\r' || buf[backIdx] == '\n')) {
                buf[backIdx--] = ' ';
            }

            // multiple values, comma-separated: add new value or continue previous?
            if (operator[](headerKey).isMultiValued) {
                if (multiValuedEndedWithComma) {
                    // start new value; this is almost like no line continuation
                    skipSpace(buf, &idx, end);
                    startIdx = idx;
                } else {
                    // continue previous value; this is tricky. unit tests to the rescue!
                    if (operator[](headerKey).beginEnd.last().first == startIdx) {
                        // remove entry, it will be re-added because already idx != startIdx
                        operator[](headerKey).beginEnd.removeLast();
                    } else {
                        // no comma, no entry: the prev line was whitespace only - start new value
                        skipSpace(buf, &idx, end);
                        startIdx = idx;
                    }
                }
            }

        } else {
            // new field
            startIdx = idx;
            // also make sure that there is at least one char after the colon
            while (idx < (end - 1) && buf[idx] != ':' && buf[idx] != '\r' && buf[idx] != '\n') {
                buf[idx] = tolower(buf[idx]);
                idx++;
            }
            if (buf[idx] != ':') {
                //malformed line: no colon
                headerKey.clear();
                continue;
            }
            headerKey = QByteArray(&buf[startIdx], idx - startIdx);
            if (!contains(headerKey)) {
                //we don't recognize this header line
                headerKey.clear();
                continue;
            }
            // skip colon & leading whitespace
            idx++;
            skipSpace(buf, &idx, end);
            startIdx = idx;
        }

        // we have the name/key of the field, now parse the value
        if (!operator[](headerKey).isMultiValued) {
        
            // scan to end of line
            while (idx < end && buf[idx] != '\r' && buf[idx] != '\n') {
                idx++;
            }
            if (!operator[](headerKey).beginEnd.isEmpty()) {
                // there already is an entry; are we just in a line continuation?
                if (operator[](headerKey).beginEnd.last().first == startIdx) {
                    // line continuation: delete previous entry and later insert a new, longer one.
                    operator[](headerKey).beginEnd.removeLast();
                }
            }
            operator[](headerKey).beginEnd.append(QPair<int, int>(startIdx, idx));
            
        } else {
        
            // comma-separated list
            while (true) {
                //skip one value
                while (idx < end && buf[idx] != '\r' && buf[idx] != '\n' && buf[idx] != ',') {
                    idx++;
                }
                if (idx != startIdx) {
                    operator[](headerKey).beginEnd.append(QPair<int, int>(startIdx, idx));                    
                }
                multiValuedEndedWithComma = buf[idx] == ',';
                //skip comma(s) and leading whitespace, if any respectively
                while (idx < end && buf[idx] == ',') {
                    idx++;
                }
                skipSpace(buf, &idx, end);
                //next value or end-of-line / end of header?
                if (buf[idx] >= end || buf[idx] == '\r' || buf[idx] == '\n') {
                    break;
                }
                //next value
                startIdx = idx;
            }
        }
    } while (nextLine(buf, &idx, end));
    return idx;
}


TokenIterator HeaderTokenizer::iterator(const char *key)
{
    QByteArray keyBa = QByteArray::fromRawData(key, strlen(key));
    if (contains(keyBa)) {
        return TokenIterator(value(keyBa).beginEnd, m_buffer);
    } else {
        return TokenIterator(m_nullTokens, m_buffer);
    }
}
