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

(define (el-appendix-xref-string gi-or-name)
  (if %chapter-autolabel%
      "&Appendix; %n"
      "the &appendix; called %t"))

(define (el-article-xref-string gi-or-name)
  (string-append %gentext-el-start-quote%
		 "%t"
		 %gentext-el-end-quote%))

(define (el-bibliography-xref-string gi-or-name)
  "%t")

(define (el-book-xref-string gi-or-name)
  "%t")

(define (el-chapter-xref-string gi-or-name)
  (if %chapter-autolabel%
      "&Chapter; %n"
      "the &chapter; called %t"))

(define (el-equation-xref-string gi-or-name)
  "&Equation; %n")

(define (el-example-xref-string gi-or-name)
  "&Example; %n")

(define (el-figure-xref-string gi-or-name)
  "&Figure; %n")

(define (el-glossary-xref-string gi-or-name)
  "%t")

(define (el-index-xref-string gi-or-name)
  "%t")

(define (el-listitem-xref-string gi-or-name)
  "%n")

(define (el-part-xref-string gi-or-name)
  "&Part; %n")

(define (el-preface-xref-string gi-or-name)
  "%t")

(define (el-procedure-xref-string gi-or-name)
  "&Procedure; %n, %t")

(define (el-reference-xref-string gi-or-name)
  "&Reference; %n, %t")

(define (el-sectioning-xref-string gi-or-name)
  (if %section-autolabel% 
      "&Section; %n" 
      "the &section; called %t"))

(define (el-sect1-xref-string gi-or-name)
  (el-sectioning-xref-string gi-or-name))

(define (el-sect2-xref-string gi-or-name)
  (el-sectioning-xref-string gi-or-name))

(define (el-sect3-xref-string gi-or-name)
  (el-sectioning-xref-string gi-or-name))

(define (el-sect4-xref-string gi-or-name)
  (el-sectioning-xref-string gi-or-name))

(define (el-sect5-xref-string gi-or-name)
  (el-sectioning-xref-string gi-or-name))

(define (el-section-xref-string gi-or-name)
  (el-sectioning-xref-string gi-or-name))

(define (el-sidebar-xref-string gi-or-name)
  "the &sidebar; %t")

(define (el-step-xref-string gi-or-name)
  "&step; %n")

(define (el-table-xref-string gi-or-name)
  "&Table; %n")

(define (el-default-xref-string gi-or-name)
  (let* ((giname (if (string? gi-or-name) gi-or-name (gi gi-or-name)))
	 (msg    (string-append "[&xrefto; "
				(if giname giname "&nonexistantelement;")
				" &unsupported;]"))
	 (err    (node-list-error msg (current-node))))
    msg))

(define (gentext-el-xref-strings gind)
  (let* ((giname (if (string? gind) gind (gi gind)))
	 (name   (normalize giname)))
    (cond
      ((equal? name (normalize "appendix")) (el-appendix-xref-string gind))
      ((equal? name (normalize "article"))  (el-article-xref-string gind))
      ((equal? name (normalize "bibliography")) (el-bibliography-xref-string gind))
      ((equal? name (normalize "book"))     (el-book-xref-string gind))
      ((equal? name (normalize "chapter"))  (el-chapter-xref-string gind))
      ((equal? name (normalize "equation")) (el-equation-xref-string gind))
      ((equal? name (normalize "example"))  (el-example-xref-string gind))
      ((equal? name (normalize "figure"))   (el-figure-xref-string gind))
      ((equal? name (normalize "glossary")) (el-glossary-xref-string gind))
      ((equal? name (normalize "index"))    (el-index-xref-string gind))
      ((equal? name (normalize "listitem")) (el-listitem-xref-string gind))
      ((equal? name (normalize "part"))     (el-part-xref-string gind))
      ((equal? name (normalize "preface"))  (el-preface-xref-string gind))
      ((equal? name (normalize "procedure")) (el-procedure-xref-string gind))
      ((equal? name (normalize "reference")) (el-reference-xref-string gind))
      ((equal? name (normalize "sect1"))    (el-sect1-xref-string gind))
      ((equal? name (normalize "sect2"))    (el-sect2-xref-string gind))
      ((equal? name (normalize "sect3"))    (el-sect3-xref-string gind))
      ((equal? name (normalize "sect4"))    (el-sect4-xref-string gind))
      ((equal? name (normalize "sect5"))    (el-sect5-xref-string gind))
      ((equal? name (normalize "section"))  (el-section-xref-string gind))
      ((equal? name (normalize "simplesect"))  (el-section-xref-string gind))
      ((equal? name (normalize "sidebar"))  (el-sidebar-xref-string gind))
      ((equal? name (normalize "step"))     (el-step-xref-string gind))
      ((equal? name (normalize "table"))    (el-table-xref-string gind))
      (else (el-default-xref-string gind)))))

(define (el-auto-xref-indirect-connector before) 
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
(define %generate-el-toc-in-front% #t)

;; gentext-element-name returns the generated text that should be 
;; used to make reference to the selected element.
;;
(define el-abstract-name	"&Abstract;")
(define el-answer-name		"&Answer;")
(define el-appendix-name	"&Appendix;")
(define el-article-name	"&Article;")
(define el-bibliography-name	"&Bibliography;")
(define el-book-name		"&Book;")
(define el-calloutlist-name	"")
(define el-caution-name	"&Caution;")
(define el-chapter-name	"&Chapter;")
(define el-copyright-name	"&Copyright;")
(define el-dedication-name	"&Dedication;")
(define el-edition-name	"&Edition;")
(define el-equation-name	"&Equation;")
(define el-example-name	"&Example;")
(define el-figure-name	"&Figure;")
(define el-glossary-name	"&Glossary;")
(define el-glosssee-name	"&GlossSee;")
(define el-glossseealso-name	"&GlossSeeAlso;")
(define el-important-name	"&Important;")
(define el-index-name		"&Index;")
(define el-colophon-name	"&Colophon;")
(define el-setindex-name	"&SetIndex;")
(define el-isbn-name		"&ISBN;")
(define el-legalnotice-name	"&LegalNotice;")
(define el-msgaud-name	"&MsgAud;")
(define el-msglevel-name	"&MsgLevel;")
(define el-msgorig-name	"&MsgOrig;")
(define el-note-name		"&Note;")
(define el-part-name		"&Part;")
(define el-preface-name	"&Preface;")
(define el-procedure-name	"&Procedure;")
(define el-pubdate-name	"&Published;")
(define el-question-name	"&Question;")
(define el-refentry-name      "&RefEntry;")
(define el-reference-name	"&Reference;")
(define el-refname-name	"&RefName;")
(define el-revhistory-name	"&RevHistory;")
(define el-refsect1-name      "&RefSection;")
(define el-refsect2-name      "&RefSection;")
(define el-refsect3-name      "&RefSection;")
(define el-refsynopsisdiv-name      "&RefSynopsisDiv;")
(define el-revision-name	"&Revision;")
(define el-sect1-name		"&Section;")
(define el-sect2-name		"&Section;")
(define el-sect3-name		"&Section;")
(define el-sect4-name		"&Section;")
(define el-sect5-name		"&Section;")
(define el-section-name		"&Section;")
(define el-simplesect-name	"&Section;")
(define el-seeie-name		"&See;")
(define el-seealsoie-name	"&Seealso;")
(define el-set-name		"&Set;")
(define el-sidebar-name	"&Sidebar;")
(define el-step-name		"&step;")
(define el-table-name		"&Table;")
(define el-tip-name		"&Tip;")
(define el-toc-name		"&TableofContents;")
(define el-warning-name	"&Warning;")

(define (gentext-el-element-name gind)
  (let* ((giname (if (string? gind) gind (gi gind)))
	 (name   (normalize giname)))
    (cond
     ((equal? name (normalize "abstract"))	el-abstract-name)
     ((equal? name (normalize "answer"))	el-answer-name)
     ((equal? name (normalize "appendix"))	el-appendix-name)
     ((equal? name (normalize "article"))	el-article-name)
     ((equal? name (normalize "bibliography"))	el-bibliography-name)
     ((equal? name (normalize "book"))		el-book-name)
     ((equal? name (normalize "calloutlist"))	el-calloutlist-name)
     ((equal? name (normalize "caution"))	el-caution-name)
     ((equal? name (normalize "chapter"))	el-chapter-name)
     ((equal? name (normalize "copyright"))	el-copyright-name)
     ((equal? name (normalize "dedication"))	el-dedication-name)
     ((equal? name (normalize "edition"))	el-edition-name)
     ((equal? name (normalize "equation"))	el-equation-name)
     ((equal? name (normalize "example"))	el-example-name)
     ((equal? name (normalize "figure"))	el-figure-name)
     ((equal? name (normalize "glossary"))	el-glossary-name)
     ((equal? name (normalize "glosssee"))	el-glosssee-name)
     ((equal? name (normalize "glossseealso"))	el-glossseealso-name)
     ((equal? name (normalize "important"))	el-important-name)
     ((equal? name (normalize "index"))		el-index-name)
     ((equal? name (normalize "colophon"))	el-colophon-name)
     ((equal? name (normalize "setindex"))	el-setindex-name)
     ((equal? name (normalize "isbn"))		el-isbn-name)
     ((equal? name (normalize "legalnotice"))	el-legalnotice-name)
     ((equal? name (normalize "msgaud"))	el-msgaud-name)
     ((equal? name (normalize "msglevel"))	el-msglevel-name)
     ((equal? name (normalize "msgorig"))	el-msgorig-name)
     ((equal? name (normalize "note"))		el-note-name)
     ((equal? name (normalize "part"))		el-part-name)
     ((equal? name (normalize "preface"))	el-preface-name)
     ((equal? name (normalize "procedure"))	el-procedure-name)
     ((equal? name (normalize "pubdate"))	el-pubdate-name)
     ((equal? name (normalize "question"))	el-question-name)
     ((equal? name (normalize "refentry"))	el-refentry-name)
     ((equal? name (normalize "reference"))	el-reference-name)
     ((equal? name (normalize "refname"))	el-refname-name)
     ((equal? name (normalize "revhistory"))	el-revhistory-name)
     ((equal? name (normalize "refsect1"))	el-refsect1-name)
     ((equal? name (normalize "refsect2"))	el-refsect2-name)
     ((equal? name (normalize "refsect3"))	el-refsect3-name)
     ((equal? name (normalize "refsynopsisdiv"))	el-refsynopsisdiv-name)
     ((equal? name (normalize "revision"))	el-revision-name)
     ((equal? name (normalize "sect1"))		el-sect1-name)
     ((equal? name (normalize "sect2"))		el-sect2-name)
     ((equal? name (normalize "sect3"))		el-sect3-name)
     ((equal? name (normalize "sect4"))		el-sect4-name)
     ((equal? name (normalize "sect5"))		el-sect5-name)
     ((equal? name (normalize "section"))	el-section-name)
     ((equal? name (normalize "simplesect"))    el-simplesect-name)
     ((equal? name (normalize "seeie"))		el-seeie-name)
     ((equal? name (normalize "seealsoie"))	el-seealsoie-name)
     ((equal? name (normalize "set"))		el-set-name)
     ((equal? name (normalize "sidebar"))	el-sidebar-name)
     ((equal? name (normalize "step"))		el-step-name)
     ((equal? name (normalize "table"))		el-table-name)
     ((equal? name (normalize "tip"))		el-tip-name)
     ((equal? name (normalize "toc"))		el-toc-name)
     ((equal? name (normalize "warning"))	el-warning-name)
     (else (let* ((msg (string-append "gentext-el-element-name: &unexpectedelementname;: " name))
		  (err (node-list-error msg (current-node))))
	     msg)))))

;; gentext-element-name-space returns gentext-element-name with a 
;; trailing space, if gentext-element-name isn't "".
;;
(define (gentext-el-element-name-space giname)
  (string-with-space (gentext-element-name giname)))

;; gentext-intra-label-sep returns the seperator to be inserted
;; between multiple occurances of a label (or parts of a label)
;; for the specified element.  Most of these are for enumerated
;; labels like "Figure 2-4", but this function is used elsewhere
;; (e.g. REFNAME) with a little abuse.
;;

(define el-equation-intra-label-sep "-")
(define el-informalequation-intra-label-sep "-")
(define el-example-intra-label-sep "-")
(define el-figure-intra-label-sep "-")
(define el-listitem-intra-label-sep ".")
(define el-procedure-intra-label-sep ".")
(define el-refentry-intra-label-sep ".")
(define el-reference-intra-label-sep ".")
(define el-refname-intra-label-sep ", ")
(define el-refsect1-intra-label-sep ".")
(define el-refsect2-intra-label-sep ".")
(define el-refsect3-intra-label-sep ".")
(define el-sect1-intra-label-sep ".")
(define el-sect2-intra-label-sep ".")
(define el-sect3-intra-label-sep ".")
(define el-sect4-intra-label-sep ".")
(define el-sect5-intra-label-sep ".")
(define el-section-intra-label-sep ".")
(define el-simplesect-intra-label-sep ".")
(define el-step-intra-label-sep ".")
(define el-table-intra-label-sep "-")
(define el-_pagenumber-intra-label-sep "-")
(define el-default-intra-label-sep "")

(define (gentext-el-intra-label-sep gind)
  (let* ((giname (if (string? gind) gind (gi gind)))
	 (name   (normalize giname)))
    (cond
     ((equal? name (normalize "equation"))	el-equation-intra-label-sep)
     ((equal? name (normalize "informalequation"))	el-informalequation-intra-label-sep)
     ((equal? name (normalize "example"))	el-example-intra-label-sep)
     ((equal? name (normalize "figure"))	el-figure-intra-label-sep)
     ((equal? name (normalize "listitem"))	el-listitem-intra-label-sep)
     ((equal? name (normalize "procedure"))	el-procedure-intra-label-sep)
     ((equal? name (normalize "refentry"))	el-refentry-intra-label-sep)
     ((equal? name (normalize "reference"))	el-reference-intra-label-sep)
     ((equal? name (normalize "refname"))	el-refname-intra-label-sep)
     ((equal? name (normalize "refsect1"))	el-refsect1-intra-label-sep)
     ((equal? name (normalize "refsect2"))	el-refsect2-intra-label-sep)
     ((equal? name (normalize "refsect3"))	el-refsect3-intra-label-sep)
     ((equal? name (normalize "sect1"))		el-sect1-intra-label-sep)
     ((equal? name (normalize "sect2"))		el-sect2-intra-label-sep)
     ((equal? name (normalize "sect3"))		el-sect3-intra-label-sep)
     ((equal? name (normalize "sect4"))		el-sect4-intra-label-sep)
     ((equal? name (normalize "sect5"))		el-sect5-intra-label-sep)
     ((equal? name (normalize "section"))	el-section-intra-label-sep)
     ((equal? name (normalize "simplesect"))	el-simplesect-intra-label-sep)
     ((equal? name (normalize "step"))		el-step-intra-label-sep)
     ((equal? name (normalize "table"))		el-table-intra-label-sep)
     ((equal? name (normalize "_pagenumber"))	el-_pagenumber-intra-label-sep)
     (else el-default-intra-label-sep))))

;; gentext-label-title-sep returns the seperator to be inserted
;; between a label and the text following the label for the
;; specified element.  Most of these are for use between
;; enumerated labels and titles like "1. Chapter One Title", but
;; this function is used elsewhere (e.g. NOTE) with a little
;; abuse.
;;

(define el-abstract-label-title-sep ": ")
(define el-answer-label-title-sep " ")
(define el-appendix-label-title-sep ". ")
(define el-caution-label-title-sep "")
(define el-chapter-label-title-sep ". ")
(define el-equation-label-title-sep ". ")
(define el-example-label-title-sep ". ")
(define el-figure-label-title-sep ". ")
(define el-footnote-label-title-sep ". ")
(define el-glosssee-label-title-sep ": ")
(define el-glossseealso-label-title-sep ": ")
(define el-important-label-title-sep ": ")
(define el-note-label-title-sep ": ")
(define el-orderedlist-label-title-sep ". ")
(define el-part-label-title-sep ". ")
(define el-procedure-label-title-sep ". ")
(define el-prefix-label-title-sep ". ")
(define el-question-label-title-sep " ")
(define el-refentry-label-title-sep "")
(define el-reference-label-title-sep ". ")
(define el-refsect1-label-title-sep ". ")
(define el-refsect2-label-title-sep ". ")
(define el-refsect3-label-title-sep ". ")
(define el-sect1-label-title-sep ". ")
(define el-sect2-label-title-sep ". ")
(define el-sect3-label-title-sep ". ")
(define el-sect4-label-title-sep ". ")
(define el-sect5-label-title-sep ". ")
(define el-section-label-title-sep ". ")
(define el-simplesect-label-title-sep ". ")
(define el-seeie-label-title-sep " ")
(define el-seealsoie-label-title-sep " ")
(define el-step-label-title-sep ". ")
(define el-table-label-title-sep ". ")
(define el-tip-label-title-sep ": ")
(define el-warning-label-title-sep "")
(define el-default-label-title-sep "")

(define (gentext-el-label-title-sep gind)
  (let* ((giname (if (string? gind) gind (gi gind)))
	 (name   (normalize giname)))
    (cond
     ((equal? name (normalize "abstract")) el-abstract-label-title-sep)
     ((equal? name (normalize "answer")) el-answer-label-title-sep)
     ((equal? name (normalize "appendix")) el-appendix-label-title-sep)
     ((equal? name (normalize "caution")) el-caution-label-title-sep)
     ((equal? name (normalize "chapter")) el-chapter-label-title-sep)
     ((equal? name (normalize "equation")) el-equation-label-title-sep)
     ((equal? name (normalize "example")) el-example-label-title-sep)
     ((equal? name (normalize "figure")) el-figure-label-title-sep)
     ((equal? name (normalize "footnote")) el-footnote-label-title-sep)
     ((equal? name (normalize "glosssee")) el-glosssee-label-title-sep)
     ((equal? name (normalize "glossseealso")) el-glossseealso-label-title-sep)
     ((equal? name (normalize "important")) el-important-label-title-sep)
     ((equal? name (normalize "note")) el-note-label-title-sep)
     ((equal? name (normalize "orderedlist")) el-orderedlist-label-title-sep)
     ((equal? name (normalize "part")) el-part-label-title-sep)
     ((equal? name (normalize "procedure")) el-procedure-label-title-sep)
     ((equal? name (normalize "prefix")) el-prefix-label-title-sep)
     ((equal? name (normalize "question")) el-question-label-title-sep)
     ((equal? name (normalize "refentry")) el-refentry-label-title-sep)
     ((equal? name (normalize "reference")) el-reference-label-title-sep)
     ((equal? name (normalize "refsect1")) el-refsect1-label-title-sep)
     ((equal? name (normalize "refsect2")) el-refsect2-label-title-sep)
     ((equal? name (normalize "refsect3")) el-refsect3-label-title-sep)
     ((equal? name (normalize "sect1")) el-sect1-label-title-sep)
     ((equal? name (normalize "sect2")) el-sect2-label-title-sep)
     ((equal? name (normalize "sect3")) el-sect3-label-title-sep)
     ((equal? name (normalize "sect4")) el-sect4-label-title-sep)
     ((equal? name (normalize "sect5")) el-sect5-label-title-sep)
     ((equal? name (normalize "section")) el-section-label-title-sep)
     ((equal? name (normalize "simplesect")) el-simplesect-label-title-sep)
     ((equal? name (normalize "seeie")) el-seeie-label-title-sep)
     ((equal? name (normalize "seealsoie")) el-seealsoie-label-title-sep)
     ((equal? name (normalize "step")) el-step-label-title-sep)
     ((equal? name (normalize "table")) el-table-label-title-sep)
     ((equal? name (normalize "tip")) el-tip-label-title-sep)
     ((equal? name (normalize "warning")) el-warning-label-title-sep)
     (else el-default-label-title-sep))))

(define (el-set-label-number-format gind) "1")
(define (el-book-label-number-format gind) "1")
(define (el-prefix-label-number-format gind) "1")
(define (el-part-label-number-format gind) "I")
(define (el-chapter-label-number-format gind) "1")
(define (el-appendix-label-number-format gind) "A")
(define (el-reference-label-number-format gind) "I")
(define (el-example-label-number-format gind) "1")
(define (el-figure-label-number-format gind) "1")
(define (el-table-label-number-format gind) "1")
(define (el-procedure-label-number-format gind) "1")
(define (el-step-label-number-format gind) "1")
(define (el-refsect1-label-number-format gind) "1")
(define (el-refsect2-label-number-format gind) "1")
(define (el-refsect3-label-number-format gind) "1")
(define (el-sect1-label-number-format gind) "1")
(define (el-sect2-label-number-format gind) "1")
(define (el-sect3-label-number-format gind) "1")
(define (el-sect4-label-number-format gind) "1")
(define (el-sect5-label-number-format gind) "1")
(define (el-section-label-number-format gind) "1")
(define (el-default-label-number-format gind) "1")

(define (el-label-number-format gind)
  (let* ((giname (if (string? gind) gind (gi gind)))
	 (name   (normalize giname)))
    (cond
     ((equal? name (normalize "set")) (el-set-label-number-format gind))
     ((equal? name (normalize "book")) (el-book-label-number-format gind))
     ((equal? name (normalize "prefix")) (el-prefix-label-number-format gind))
     ((equal? name (normalize "part")) (el-part-label-number-format gind))
     ((equal? name (normalize "chapter")) (el-chapter-label-number-format gind))
     ((equal? name (normalize "appendix")) (el-appendix-label-number-format gind))
     ((equal? name (normalize "reference")) (el-reference-label-number-format gind))
     ((equal? name (normalize "example")) (el-example-label-number-format gind))
     ((equal? name (normalize "figure")) (el-figure-label-number-format gind))
     ((equal? name (normalize "table")) (el-table-label-number-format gind))
     ((equal? name (normalize "procedure")) (el-procedure-label-number-format gind))
     ((equal? name (normalize "step")) (el-step-label-number-format gind))
     ((equal? name (normalize "refsect1")) (el-refsect1-label-number-format gind))
     ((equal? name (normalize "refsect2")) (el-refsect2-label-number-format gind))
     ((equal? name (normalize "refsect3")) (el-refsect3-label-number-format gind))
     ((equal? name (normalize "sect1")) (el-sect1-label-number-format gind))
     ((equal? name (normalize "sect2")) (el-sect2-label-number-format gind))
     ((equal? name (normalize "sect3")) (el-sect3-label-number-format gind))
     ((equal? name (normalize "sect4")) (el-sect4-label-number-format gind))
     ((equal? name (normalize "sect5")) (el-sect5-label-number-format gind))
     ((equal? name (normalize "section")) (el-section-label-number-format gind))
     (else (el-default-label-number-format gind)))))

(define ($lot-title-el$ gind)
  (let* ((giname (if (string? gind) gind (gi gind)))
	 (name   (normalize giname)))
    (cond ((equal? name (normalize "table"))    "&ListofTables;")
	  ((equal? name (normalize "example"))  "&ListofExamples;")
	  ((equal? name (normalize "figure"))   "&ListofFigures;")
	  ((equal? name (normalize "equation")) "&ListofEquations;")
	  (else (let* ((msg (string-append "&ListofUnknown;: " name))
		       (err (node-list-error msg (current-node))))
		  msg)))))

(define %gentext-el-start-quote% (dingbat "ldquo"))

(define %gentext-el-end-quote% (dingbat "rdquo"))

(define %gentext-el-start-nested-quote% (dingbat "lsquo"))

(define %gentext-el-end-nested-quote% (dingbat "rsquo"))

(define %gentext-el-by% "&by;") ;; e.g. Copyright 1997 "by" A. Nonymous
                           ;; Authored "by" Jane Doe

(define %gentext-el-edited-by% "&Editedby;")
                           ;; "Edited by" Jane Doe

(define %gentext-el-page% "")

(define %gentext-el-and% "&and;")

(define %gentext-el-bibl-pages% "&Pgs;")

(define %gentext-el-endnotes% "&Notes;")

(define %gentext-el-table-endnotes% "&TableNotes;:")

(define %gentext-el-index-see% "&See;")

(define %gentext-el-index-seealso% "&SeeAlso;")

