<!DOCTYPE style-sheet PUBLIC "-//James Clark//DTD DSSSL Style Sheet//EN" [
<!ENTITY % zh-CN.words
  PUBLIC "-//Norman Walsh//ENTITIES DocBook Stylesheet Localization//ZHCN"
         "dbl1en.ent">
%zh-CN.words;
]>

<style-sheet>
<style-specification id="docbook-l10n-zh-CN">
<style-specification-body>

;; $Id$
;;
;; This file is part of the Modular DocBook Stylesheet distribution.
;; See ../README or http://nwalsh.com/docbook/dsssl/
;;

;; ----------------------------- Localization -----------------------------

;; If you create a new version of this file, please send it to
;; Norman Walsh, ndw@nwalsh.com

;; The generated text for cross references to elements.  See dblink.dsl
;; for a discussion of how substitution is performed on the %x 
;; keywords.
;;

(define (zh-CN-appendix-xref-string gi-or-name)
  (if %chapter-autolabel%
      "&Appendix; %n"
      "the &appendix; called %t"))

(define (zh-CN-article-xref-string gi-or-name)
  (string-append %gentext-zh-CN-start-quote%
		 "%t"
		 %gentext-zh-CN-end-quote%))

(define (zh-CN-bibliography-xref-string gi-or-name)
  "%t")

(define (zh-CN-book-xref-string gi-or-name)
  "%t")

(define (zh-CN-chapter-xref-string gi-or-name)
  (if %chapter-autolabel%
      "&Chapter; %n"
      "the &chapter; called %t"))

(define (zh-CN-equation-xref-string gi-or-name)
  "&Equation; %n")

(define (zh-CN-example-xref-string gi-or-name)
  "&Example; %n")

(define (zh-CN-figure-xref-string gi-or-name)
  "&Figure; %n")

(define (zh-CN-glossary-xref-string gi-or-name)
  "%t")

(define (zh-CN-index-xref-string gi-or-name)
  "%t")

(define (zh-CN-listitem-xref-string gi-or-name)
  "%n")

(define (zh-CN-part-xref-string gi-or-name)
  "&Part; %n")

(define (zh-CN-preface-xref-string gi-or-name)
  "%t")

(define (zh-CN-procedure-xref-string gi-or-name)
  "&Procedure; %n, %t")

(define (zh-CN-reference-xref-string gi-or-name)
  "&Reference; %n, %t")

(define (zh-CN-sectioning-xref-string gi-or-name)
  (if %section-autolabel% 
      "&Section; %n" 
      "the &section; called %t"))

(define (zh-CN-sect1-xref-string gi-or-name)
  (zh-CN-sectioning-xref-string gi-or-name))

(define (zh-CN-sect2-xref-string gi-or-name)
  (zh-CN-sectioning-xref-string gi-or-name))

(define (zh-CN-sect3-xref-string gi-or-name)
  (zh-CN-sectioning-xref-string gi-or-name))

(define (zh-CN-sect4-xref-string gi-or-name)
  (zh-CN-sectioning-xref-string gi-or-name))

(define (zh-CN-sect5-xref-string gi-or-name)
  (zh-CN-sectioning-xref-string gi-or-name))

(define (zh-CN-section-xref-string gi-or-name)
  (zh-CN-sectioning-xref-string gi-or-name))

(define (zh-CN-sidebar-xref-string gi-or-name)
  "the &sidebar; %t")

(define (zh-CN-step-xref-string gi-or-name)
  "&step; %n")

(define (zh-CN-table-xref-string gi-or-name)
  "&Table; %n")

(define (zh-CN-default-xref-string gi-or-name)
  (let* ((giname (if (string? gi-or-name) gi-or-name (gi gi-or-name)))
	 (msg    (string-append "[&xrefto; "
				(if giname giname "&nonexistantelement;")
				" &unsupported;]"))
	 (err    (node-list-error msg (current-node))))
    msg))

(define (gentext-zh-CN-xref-strings gind)
  (let* ((giname (if (string? gind) gind (gi gind)))
	 (name   (normalize giname)))
    (cond
      ((equal? name (normalize "appendix")) (zh-CN-appendix-xref-string gind))
      ((equal? name (normalize "article"))  (zh-CN-article-xref-string gind))
      ((equal? name (normalize "bibliography")) (zh-CN-bibliography-xref-string gind))
      ((equal? name (normalize "book"))     (zh-CN-book-xref-string gind))
      ((equal? name (normalize "chapter"))  (zh-CN-chapter-xref-string gind))
      ((equal? name (normalize "equation")) (zh-CN-equation-xref-string gind))
      ((equal? name (normalize "example"))  (zh-CN-example-xref-string gind))
      ((equal? name (normalize "figure"))   (zh-CN-figure-xref-string gind))
      ((equal? name (normalize "glossary")) (zh-CN-glossary-xref-string gind))
      ((equal? name (normalize "index"))    (zh-CN-index-xref-string gind))
      ((equal? name (normalize "listitem")) (zh-CN-listitem-xref-string gind))
      ((equal? name (normalize "part"))     (zh-CN-part-xref-string gind))
      ((equal? name (normalize "preface"))  (zh-CN-preface-xref-string gind))
      ((equal? name (normalize "procedure")) (zh-CN-procedure-xref-string gind))
      ((equal? name (normalize "reference")) (zh-CN-reference-xref-string gind))
      ((equal? name (normalize "sect1"))    (zh-CN-sect1-xref-string gind))
      ((equal? name (normalize "sect2"))    (zh-CN-sect2-xref-string gind))
      ((equal? name (normalize "sect3"))    (zh-CN-sect3-xref-string gind))
      ((equal? name (normalize "sect4"))    (zh-CN-sect4-xref-string gind))
      ((equal? name (normalize "sect5"))    (zh-CN-sect5-xref-string gind))
      ((equal? name (normalize "section"))  (zh-CN-section-xref-string gind))
      ((equal? name (normalize "simplesect"))  (zh-CN-section-xref-string gind))
      ((equal? name (normalize "sidebar"))  (zh-CN-sidebar-xref-string gind))
      ((equal? name (normalize "step"))     (zh-CN-step-xref-string gind))
      ((equal? name (normalize "table"))    (zh-CN-table-xref-string gind))
      (else (zh-CN-default-xref-string gind)))))

(define (zh-CN-auto-xref-indirect-connector before) 
  ;; In English, the (cond) is unnecessary since the word is always the
  ;; same, but in other languages, that's not the case.  I've set this
  ;; one up with the (cond) so it stands as an example.
  (cond 
   ((equal? (gi before) (normalize "book"))
    (literal " &in; "))
   ((equal? (gi before) (normalize "chapter"))
    (literal " &in; "))
   ((equal? (gi before) (normalize "sect1"))
    (literal " &in; "))
   (else
    (literal " &in; "))))

;; Should the TOC come first or last?
;;
(define %generate-zh-CN-toc-in-front% #t)

;; gentext-element-name returns the generated text that should be 
;; used to make reference to the selected element.
;;
(define zh-CN-abstract-name	"&Abstract;")
(define zh-CN-answer-name		"&Answer;")
(define zh-CN-appendix-name	"&Appendix;")
(define zh-CN-article-name	"&Article;")
(define zh-CN-bibliography-name	"&Bibliography;")
(define zh-CN-book-name		"&Book;")
(define zh-CN-calloutlist-name	"")
(define zh-CN-caution-name	"&Caution;")
(define zh-CN-chapter-name	"&Chapter;")
(define zh-CN-copyright-name	"&Copyright;")
(define zh-CN-dedication-name	"&Dedication;")
(define zh-CN-edition-name	"&Edition;")
(define zh-CN-equation-name	"&Equation;")
(define zh-CN-example-name	"&Example;")
(define zh-CN-figure-name	"&Figure;")
(define zh-CN-glossary-name	"&Glossary;")
(define zh-CN-glosssee-name	"&GlossSee;")
(define zh-CN-glossseealso-name	"&GlossSeeAlso;")
(define zh-CN-important-name	"&Important;")
(define zh-CN-index-name		"&Index;")
(define zh-CN-colophon-name	"&Colophon;")
(define zh-CN-setindex-name	"&SetIndex;")
(define zh-CN-isbn-name		"&ISBN;")
(define zh-CN-legalnotice-name	"&LegalNotice;")
(define zh-CN-msgaud-name	"&MsgAud;")
(define zh-CN-msglevel-name	"&MsgLevel;")
(define zh-CN-msgorig-name	"&MsgOrig;")
(define zh-CN-note-name		"&Note;")
(define zh-CN-part-name		"&Part;")
(define zh-CN-preface-name	"&Preface;")
(define zh-CN-procedure-name	"&Procedure;")
(define zh-CN-pubdate-name	"&Published;")
(define zh-CN-question-name	"&Question;")
(define zh-CN-refentry-name      "&RefEntry;")
(define zh-CN-reference-name	"&Reference;")
(define zh-CN-refname-name	"&RefName;")
(define zh-CN-revhistory-name	"&RevHistory;")
(define zh-CN-refsect1-name      "&RefSection;")
(define zh-CN-refsect2-name      "&RefSection;")
(define zh-CN-refsect3-name      "&RefSection;")
(define zh-CN-refsynopsisdiv-name      "&RefSynopsisDiv;")
(define zh-CN-revision-name	"&Revision;")
(define zh-CN-sect1-name		"&Section;")
(define zh-CN-sect2-name		"&Section;")
(define zh-CN-sect3-name		"&Section;")
(define zh-CN-sect4-name		"&Section;")
(define zh-CN-sect5-name		"&Section;")
(define zh-CN-section-name		"&Section;")
(define zh-CN-simplesect-name	"&Section;")
(define zh-CN-seeie-name		"&See;")
(define zh-CN-seealsoie-name	"&Seealso;")
(define zh-CN-set-name		"&Set;")
(define zh-CN-sidebar-name	"&Sidebar;")
(define zh-CN-step-name		"&step;")
(define zh-CN-table-name		"&Table;")
(define zh-CN-tip-name		"&Tip;")
(define zh-CN-toc-name		"&TableofContents;")
(define zh-CN-warning-name	"&Warning;")

(define (gentext-zh-CN-element-name gind)
  (let* ((giname (if (string? gind) gind (gi gind)))
	 (name   (normalize giname)))
    (cond
     ((equal? name (normalize "abstract"))	zh-CN-abstract-name)
     ((equal? name (normalize "answer"))	zh-CN-answer-name)
     ((equal? name (normalize "appendix"))	zh-CN-appendix-name)
     ((equal? name (normalize "article"))	zh-CN-article-name)
     ((equal? name (normalize "bibliography"))	zh-CN-bibliography-name)
     ((equal? name (normalize "book"))		zh-CN-book-name)
     ((equal? name (normalize "calloutlist"))	zh-CN-calloutlist-name)
     ((equal? name (normalize "caution"))	zh-CN-caution-name)
     ((equal? name (normalize "chapter"))	zh-CN-chapter-name)
     ((equal? name (normalize "copyright"))	zh-CN-copyright-name)
     ((equal? name (normalize "dedication"))	zh-CN-dedication-name)
     ((equal? name (normalize "edition"))	zh-CN-edition-name)
     ((equal? name (normalize "equation"))	zh-CN-equation-name)
     ((equal? name (normalize "example"))	zh-CN-example-name)
     ((equal? name (normalize "figure"))	zh-CN-figure-name)
     ((equal? name (normalize "glossary"))	zh-CN-glossary-name)
     ((equal? name (normalize "glosssee"))	zh-CN-glosssee-name)
     ((equal? name (normalize "glossseealso"))	zh-CN-glossseealso-name)
     ((equal? name (normalize "important"))	zh-CN-important-name)
     ((equal? name (normalize "index"))		zh-CN-index-name)
     ((equal? name (normalize "colophon"))	zh-CN-colophon-name)
     ((equal? name (normalize "setindex"))	zh-CN-setindex-name)
     ((equal? name (normalize "isbn"))		zh-CN-isbn-name)
     ((equal? name (normalize "legalnotice"))	zh-CN-legalnotice-name)
     ((equal? name (normalize "msgaud"))	zh-CN-msgaud-name)
     ((equal? name (normalize "msglevel"))	zh-CN-msglevel-name)
     ((equal? name (normalize "msgorig"))	zh-CN-msgorig-name)
     ((equal? name (normalize "note"))		zh-CN-note-name)
     ((equal? name (normalize "part"))		zh-CN-part-name)
     ((equal? name (normalize "preface"))	zh-CN-preface-name)
     ((equal? name (normalize "procedure"))	zh-CN-procedure-name)
     ((equal? name (normalize "pubdate"))	zh-CN-pubdate-name)
     ((equal? name (normalize "question"))	zh-CN-question-name)
     ((equal? name (normalize "refentry"))	zh-CN-refentry-name)
     ((equal? name (normalize "reference"))	zh-CN-reference-name)
     ((equal? name (normalize "refname"))	zh-CN-refname-name)
     ((equal? name (normalize "revhistory"))	zh-CN-revhistory-name)
     ((equal? name (normalize "refsect1"))	zh-CN-refsect1-name)
     ((equal? name (normalize "refsect2"))	zh-CN-refsect2-name)
     ((equal? name (normalize "refsect3"))	zh-CN-refsect3-name)
     ((equal? name (normalize "refsynopsisdiv"))	zh-CN-refsynopsisdiv-name)
     ((equal? name (normalize "revision"))	zh-CN-revision-name)
     ((equal? name (normalize "sect1"))		zh-CN-sect1-name)
     ((equal? name (normalize "sect2"))		zh-CN-sect2-name)
     ((equal? name (normalize "sect3"))		zh-CN-sect3-name)
     ((equal? name (normalize "sect4"))		zh-CN-sect4-name)
     ((equal? name (normalize "sect5"))		zh-CN-sect5-name)
     ((equal? name (normalize "section"))	zh-CN-section-name)
     ((equal? name (normalize "simplesect"))    zh-CN-simplesect-name)
     ((equal? name (normalize "seeie"))		zh-CN-seeie-name)
     ((equal? name (normalize "seealsoie"))	zh-CN-seealsoie-name)
     ((equal? name (normalize "set"))		zh-CN-set-name)
     ((equal? name (normalize "sidebar"))	zh-CN-sidebar-name)
     ((equal? name (normalize "step"))		zh-CN-step-name)
     ((equal? name (normalize "table"))		zh-CN-table-name)
     ((equal? name (normalize "tip"))		zh-CN-tip-name)
     ((equal? name (normalize "toc"))		zh-CN-toc-name)
     ((equal? name (normalize "warning"))	zh-CN-warning-name)
     (else (let* ((msg (string-append "gentext-zh-CN-element-name: &unexpectedelementname;: " name))
		  (err (node-list-error msg (current-node))))
	     msg)))))

;; gentext-element-name-space returns gentext-element-name with a 
;; trailing space, if gentext-element-name isn't "".
;;
(define (gentext-zh-CN-element-name-space giname)
  (string-with-space (gentext-element-name giname)))

;; gentext-intra-label-sep returns the seperator to be inserted
;; between multiple occurances of a label (or parts of a label)
;; for the specified element.  Most of these are for enumerated
;; labels like "Figure 2-4", but this function is used elsewhere
;; (e.g. REFNAME) with a little abuse.
;;

(define zh-CN-equation-intra-label-sep "-")
(define zh-CN-informalequation-intra-label-sep "-")
(define zh-CN-example-intra-label-sep "-")
(define zh-CN-figure-intra-label-sep "-")
(define zh-CN-listitem-intra-label-sep ".")
(define zh-CN-procedure-intra-label-sep ".")
(define zh-CN-refentry-intra-label-sep ".")
(define zh-CN-reference-intra-label-sep ".")
(define zh-CN-refname-intra-label-sep ", ")
(define zh-CN-refsect1-intra-label-sep ".")
(define zh-CN-refsect2-intra-label-sep ".")
(define zh-CN-refsect3-intra-label-sep ".")
(define zh-CN-sect1-intra-label-sep ".")
(define zh-CN-sect2-intra-label-sep ".")
(define zh-CN-sect3-intra-label-sep ".")
(define zh-CN-sect4-intra-label-sep ".")
(define zh-CN-sect5-intra-label-sep ".")
(define zh-CN-section-intra-label-sep ".")
(define zh-CN-simplesect-intra-label-sep ".")
(define zh-CN-step-intra-label-sep ".")
(define zh-CN-table-intra-label-sep "-")
(define zh-CN-_pagenumber-intra-label-sep "-")
(define zh-CN-default-intra-label-sep "")

(define (gentext-zh-CN-intra-label-sep gind)
  (let* ((giname (if (string? gind) gind (gi gind)))
	 (name   (normalize giname)))
    (cond
     ((equal? name (normalize "equation"))	zh-CN-equation-intra-label-sep)
     ((equal? name (normalize "informalequation"))	zh-CN-informalequation-intra-label-sep)
     ((equal? name (normalize "example"))	zh-CN-example-intra-label-sep)
     ((equal? name (normalize "figure"))	zh-CN-figure-intra-label-sep)
     ((equal? name (normalize "listitem"))	zh-CN-listitem-intra-label-sep)
     ((equal? name (normalize "procedure"))	zh-CN-procedure-intra-label-sep)
     ((equal? name (normalize "refentry"))	zh-CN-refentry-intra-label-sep)
     ((equal? name (normalize "reference"))	zh-CN-reference-intra-label-sep)
     ((equal? name (normalize "refname"))	zh-CN-refname-intra-label-sep)
     ((equal? name (normalize "refsect1"))	zh-CN-refsect1-intra-label-sep)
     ((equal? name (normalize "refsect2"))	zh-CN-refsect2-intra-label-sep)
     ((equal? name (normalize "refsect3"))	zh-CN-refsect3-intra-label-sep)
     ((equal? name (normalize "sect1"))		zh-CN-sect1-intra-label-sep)
     ((equal? name (normalize "sect2"))		zh-CN-sect2-intra-label-sep)
     ((equal? name (normalize "sect3"))		zh-CN-sect3-intra-label-sep)
     ((equal? name (normalize "sect4"))		zh-CN-sect4-intra-label-sep)
     ((equal? name (normalize "sect5"))		zh-CN-sect5-intra-label-sep)
     ((equal? name (normalize "section"))	zh-CN-section-intra-label-sep)
     ((equal? name (normalize "simplesect"))	zh-CN-simplesect-intra-label-sep)
     ((equal? name (normalize "step"))		zh-CN-step-intra-label-sep)
     ((equal? name (normalize "table"))		zh-CN-table-intra-label-sep)
     ((equal? name (normalize "_pagenumber"))	zh-CN-_pagenumber-intra-label-sep)
     (else zh-CN-default-intra-label-sep))))

;; gentext-label-title-sep returns the seperator to be inserted
;; between a label and the text following the label for the
;; specified element.  Most of these are for use between
;; enumerated labels and titles like "1. Chapter One Title", but
;; this function is used elsewhere (e.g. NOTE) with a little
;; abuse.
;;

(define zh-CN-abstract-label-title-sep ": ")
(define zh-CN-answer-label-title-sep " ")
(define zh-CN-appendix-label-title-sep ". ")
(define zh-CN-caution-label-title-sep "")
(define zh-CN-chapter-label-title-sep ". ")
(define zh-CN-equation-label-title-sep ". ")
(define zh-CN-example-label-title-sep ". ")
(define zh-CN-figure-label-title-sep ". ")
(define zh-CN-footnote-label-title-sep ". ")
(define zh-CN-glosssee-label-title-sep ": ")
(define zh-CN-glossseealso-label-title-sep ": ")
(define zh-CN-important-label-title-sep ": ")
(define zh-CN-note-label-title-sep ": ")
(define zh-CN-orderedlist-label-title-sep ". ")
(define zh-CN-part-label-title-sep ". ")
(define zh-CN-procedure-label-title-sep ". ")
(define zh-CN-prefix-label-title-sep ". ")
(define zh-CN-question-label-title-sep " ")
(define zh-CN-refentry-label-title-sep "")
(define zh-CN-reference-label-title-sep ". ")
(define zh-CN-refsect1-label-title-sep ". ")
(define zh-CN-refsect2-label-title-sep ". ")
(define zh-CN-refsect3-label-title-sep ". ")
(define zh-CN-sect1-label-title-sep ". ")
(define zh-CN-sect2-label-title-sep ". ")
(define zh-CN-sect3-label-title-sep ". ")
(define zh-CN-sect4-label-title-sep ". ")
(define zh-CN-sect5-label-title-sep ". ")
(define zh-CN-section-label-title-sep ". ")
(define zh-CN-simplesect-label-title-sep ". ")
(define zh-CN-seeie-label-title-sep " ")
(define zh-CN-seealsoie-label-title-sep " ")
(define zh-CN-step-label-title-sep ". ")
(define zh-CN-table-label-title-sep ". ")
(define zh-CN-tip-label-title-sep ": ")
(define zh-CN-warning-label-title-sep "")
(define zh-CN-default-label-title-sep "")

(define (gentext-zh-CN-label-title-sep gind)
  (let* ((giname (if (string? gind) gind (gi gind)))
	 (name   (normalize giname)))
    (cond
     ((equal? name (normalize "abstract")) zh-CN-abstract-label-title-sep)
     ((equal? name (normalize "answer")) zh-CN-answer-label-title-sep)
     ((equal? name (normalize "appendix")) zh-CN-appendix-label-title-sep)
     ((equal? name (normalize "caution")) zh-CN-caution-label-title-sep)
     ((equal? name (normalize "chapter")) zh-CN-chapter-label-title-sep)
     ((equal? name (normalize "equation")) zh-CN-equation-label-title-sep)
     ((equal? name (normalize "example")) zh-CN-example-label-title-sep)
     ((equal? name (normalize "figure")) zh-CN-figure-label-title-sep)
     ((equal? name (normalize "footnote")) zh-CN-footnote-label-title-sep)
     ((equal? name (normalize "glosssee")) zh-CN-glosssee-label-title-sep)
     ((equal? name (normalize "glossseealso")) zh-CN-glossseealso-label-title-sep)
     ((equal? name (normalize "important")) zh-CN-important-label-title-sep)
     ((equal? name (normalize "note")) zh-CN-note-label-title-sep)
     ((equal? name (normalize "orderedlist")) zh-CN-orderedlist-label-title-sep)
     ((equal? name (normalize "part")) zh-CN-part-label-title-sep)
     ((equal? name (normalize "procedure")) zh-CN-procedure-label-title-sep)
     ((equal? name (normalize "prefix")) zh-CN-prefix-label-title-sep)
     ((equal? name (normalize "question")) zh-CN-question-label-title-sep)
     ((equal? name (normalize "refentry")) zh-CN-refentry-label-title-sep)
     ((equal? name (normalize "reference")) zh-CN-reference-label-title-sep)
     ((equal? name (normalize "refsect1")) zh-CN-refsect1-label-title-sep)
     ((equal? name (normalize "refsect2")) zh-CN-refsect2-label-title-sep)
     ((equal? name (normalize "refsect3")) zh-CN-refsect3-label-title-sep)
     ((equal? name (normalize "sect1")) zh-CN-sect1-label-title-sep)
     ((equal? name (normalize "sect2")) zh-CN-sect2-label-title-sep)
     ((equal? name (normalize "sect3")) zh-CN-sect3-label-title-sep)
     ((equal? name (normalize "sect4")) zh-CN-sect4-label-title-sep)
     ((equal? name (normalize "sect5")) zh-CN-sect5-label-title-sep)
     ((equal? name (normalize "section")) zh-CN-section-label-title-sep)
     ((equal? name (normalize "simplesect")) zh-CN-simplesect-label-title-sep)
     ((equal? name (normalize "seeie")) zh-CN-seeie-label-title-sep)
     ((equal? name (normalize "seealsoie")) zh-CN-seealsoie-label-title-sep)
     ((equal? name (normalize "step")) zh-CN-step-label-title-sep)
     ((equal? name (normalize "table")) zh-CN-table-label-title-sep)
     ((equal? name (normalize "tip")) zh-CN-tip-label-title-sep)
     ((equal? name (normalize "warning")) zh-CN-warning-label-title-sep)
     (else zh-CN-default-label-title-sep))))

(define (zh-CN-set-label-number-format gind) "1")
(define (zh-CN-book-label-number-format gind) "1")
(define (zh-CN-prefix-label-number-format gind) "1")
(define (zh-CN-part-label-number-format gind) "I")
(define (zh-CN-chapter-label-number-format gind) "1")
(define (zh-CN-appendix-label-number-format gind) "A")
(define (zh-CN-reference-label-number-format gind) "I")
(define (zh-CN-example-label-number-format gind) "1")
(define (zh-CN-figure-label-number-format gind) "1")
(define (zh-CN-table-label-number-format gind) "1")
(define (zh-CN-procedure-label-number-format gind) "1")
(define (zh-CN-step-label-number-format gind) "1")
(define (zh-CN-refsect1-label-number-format gind) "1")
(define (zh-CN-refsect2-label-number-format gind) "1")
(define (zh-CN-refsect3-label-number-format gind) "1")
(define (zh-CN-sect1-label-number-format gind) "1")
(define (zh-CN-sect2-label-number-format gind) "1")
(define (zh-CN-sect3-label-number-format gind) "1")
(define (zh-CN-sect4-label-number-format gind) "1")
(define (zh-CN-sect5-label-number-format gind) "1")
(define (zh-CN-section-label-number-format gind) "1")
(define (zh-CN-default-label-number-format gind) "1")

(define (zh-CN-label-number-format gind)
  (let* ((giname (if (string? gind) gind (gi gind)))
	 (name   (normalize giname)))
    (cond
     ((equal? name (normalize "set")) (zh-CN-set-label-number-format gind))
     ((equal? name (normalize "book")) (zh-CN-book-label-number-format gind))
     ((equal? name (normalize "prefix")) (zh-CN-prefix-label-number-format gind))
     ((equal? name (normalize "part")) (zh-CN-part-label-number-format gind))
     ((equal? name (normalize "chapter")) (zh-CN-chapter-label-number-format gind))
     ((equal? name (normalize "appendix")) (zh-CN-appendix-label-number-format gind))
     ((equal? name (normalize "reference")) (zh-CN-reference-label-number-format gind))
     ((equal? name (normalize "example")) (zh-CN-example-label-number-format gind))
     ((equal? name (normalize "figure")) (zh-CN-figure-label-number-format gind))
     ((equal? name (normalize "table")) (zh-CN-table-label-number-format gind))
     ((equal? name (normalize "procedure")) (zh-CN-procedure-label-number-format gind))
     ((equal? name (normalize "step")) (zh-CN-step-label-number-format gind))
     ((equal? name (normalize "refsect1")) (zh-CN-refsect1-label-number-format gind))
     ((equal? name (normalize "refsect2")) (zh-CN-refsect2-label-number-format gind))
     ((equal? name (normalize "refsect3")) (zh-CN-refsect3-label-number-format gind))
     ((equal? name (normalize "sect1")) (zh-CN-sect1-label-number-format gind))
     ((equal? name (normalize "sect2")) (zh-CN-sect2-label-number-format gind))
     ((equal? name (normalize "sect3")) (zh-CN-sect3-label-number-format gind))
     ((equal? name (normalize "sect4")) (zh-CN-sect4-label-number-format gind))
     ((equal? name (normalize "sect5")) (zh-CN-sect5-label-number-format gind))
     ((equal? name (normalize "section")) (zh-CN-section-label-number-format gind))
     (else (zh-CN-default-label-number-format gind)))))

(define ($lot-title-zh-CN$ gind)
  (let* ((giname (if (string? gind) gind (gi gind)))
	 (name   (normalize giname)))
    (cond ((equal? name (normalize "table"))    "&ListofTables;")
	  ((equal? name (normalize "example"))  "&ListofExamples;")
	  ((equal? name (normalize "figure"))   "&ListofFigures;")
	  ((equal? name (normalize "equation")) "&ListofEquations;")
	  (else (let* ((msg (string-append "&ListofUnknown;: " name))
		       (err (node-list-error msg (current-node))))
		  msg)))))

(define %gentext-zh-CN-start-quote% (dingbat "ldquo"))

(define %gentext-zh-CN-end-quote% (dingbat "rdquo"))

(define %gentext-zh-CN-start-nested-quote% (dingbat "lsquo"))

(define %gentext-zh-CN-end-nested-quote% (dingbat "rsquo"))

(define %gentext-zh-CN-by% "&by;") ;; e.g. Copyright 1997 "by" A. Nonymous
                           ;; Authored "by" Jane Doe

(define %gentext-zh-CN-edited-by% "&Editedby;")
                           ;; "Edited by" Jane Doe

(define %gentext-zh-CN-revised-by% "&Revisedby;")
                           ;; "Revised by" Jane Doe

(define %gentext-zh-CN-page% "")

(define %gentext-zh-CN-and% "&and;")

(define %gentext-zh-CN-bibl-pages% "&Pgs;")

(define %gentext-zh-CN-endnotes% "&Notes;")

(define %gentext-zh-CN-table-endnotes% "&TableNotes;:")

(define %gentext-zh-CN-index-see% "&See;")

(define %gentext-zh-CN-index-seealso% "&SeeAlso;")


(define (gentext-zh-CN-nav-prev prev) 
  (make sequence (literal "&nav-prev;")))

(define (gentext-zh-CN-nav-prev-sibling prevsib) 
  (make sequence (literal "&nav-prev-sibling;")))

(define (gentext-zh-CN-nav-next-sibling nextsib)
  (make sequence (literal "&nav-next-sibling;")))

(define (gentext-zh-CN-nav-next next)
  (make sequence (literal "&nav-next;")))

(define (gentext-zh-CN-nav-up up)
  (make sequence (literal "&nav-up;")))

(define (gentext-zh-CN-nav-home home)
  (make sequence (literal "&nav-home;")))



</style-specification-body>
</style-specification>
</style-sheet>
