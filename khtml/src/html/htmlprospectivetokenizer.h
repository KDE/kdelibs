/*
 * Copyright (C) 2008 Apple Inc. All Rights Reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef HTMLPROSPECTIVETOKENIZER_H
#define HTMLPROSPECTIVETOKENIZER_H

#include "misc/stringit.h"
#include <wtf/Vector.h>

namespace DOM {
    class DocumentImpl;
}

namespace khtml {

    class CachedObject;
    class CachedObjectClient;

    class ProspectiveTokenizer {
    public:
        ProspectiveTokenizer(DOM::DocumentImpl*);
        ~ProspectiveTokenizer();
        void begin();
        void write(const khtml::TokenizerString&);
        void end();
        bool inProgress() const { return m_inProgress; }

        static unsigned consumeEntity(khtml::TokenizerString&, bool& notEnoughCharacters);

    private:
        void tokenize(const khtml::TokenizerString&);
        void reset();

        void emitTag();
        void emitCharacter(QChar);

        void tokenizeCSS(QChar);
        void emitCSSRule();

        void processAttribute();


        void clearLastCharacters();
        void rememberCharacter(QChar);
        bool lastCharactersMatch(const char*, unsigned count) const;

        bool m_inProgress;
        khtml::TokenizerString m_source;

        enum State {
            Data,
            EntityData,
            TagOpen,
            CloseTagOpen,
            TagName,
            BeforeAttributeName,
            AttributeName,
            AfterAttributeName,
            BeforeAttributeValue,
            AttributeValueDoubleQuoted,
            AttributeValueSingleQuoted,
            AttributeValueUnquoted,
            EntityInAttributeValue,
            BogusComment,
            MarkupDeclarationOpen,
            CommentStart,
            CommentStartDash,
            Comment,
            CommentEndDash,
            CommentEnd
        };
        State m_state;
        bool m_escape;
        enum ContentModel {
            PCDATA,
            RCDATA,
            CDATA,
            PLAINTEXT
        };
        ContentModel m_contentModel;
        unsigned m_commentPos;
        State m_stateBeforeEntityInAttributeValue;

        static const unsigned lastCharactersBufferSize = 8;
        QChar m_lastCharacters[lastCharactersBufferSize];
        unsigned m_lastCharacterIndex;

        bool m_closeTag;
        WTF::Vector<QChar, 8> m_tagName;
        WTF::Vector<QChar, 8> m_attributeName;
        WTF::Vector<QChar, 32> m_attributeValue;
        WTF::Vector<QChar, 8> m_lastStartTag;
        uint m_lastStartTagId;

        DOM::DOMString m_urlToLoad;
        bool m_linkIsStyleSheet;

        enum CSSState {
            CSSInitial,
            CSSMaybeComment,
            CSSComment,
            CSSMaybeCommentEnd,
            CSSRuleStart,
            CSSRule,
            CSSAfterRule,
            CSSRuleValue,
            CSSAferRuleValue
        };
        CSSState m_cssState;
        WTF::Vector<QChar> m_cssRule;
        WTF::Vector<QChar> m_cssRuleValue;

        int m_timeUsed;

        DOM::DocumentImpl * m_document;
    };

}

#endif
