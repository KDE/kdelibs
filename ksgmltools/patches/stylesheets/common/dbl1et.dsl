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

(define (et-appendix-xref-string gi-or-name)
  (if %chapter-autolabel%
      "&Appendix; %n"
      "the &appendix; called %t"))

(define (et-article-xref-string gi-or-name)
  (string-append %gentext-et-start-quote%
		 "%t"
		 %gentext-et-end-quote%))

(define (et-bibliography-xref-string gi-or-name)
  "%t")

(define (et-book-xref-string gi-or-name)
  "%t")

(define (et-chapter-xref-string gi-or-name)
  (if %chapter-autolabel%
      "&Chapter; %n"
      "the &chapter; called %t"))

(define (et-equation-xref-string gi-or-name)
  "&Equation; %n")

(define (et-example-xref-string gi-or-name)
  "&Example; %n")

(define (et-figure-xref-string gi-or-name)
  "&Figure; %n")

(define (et-glossary-xref-string gi-or-name)
  "%t")

(define (et-index-xref-string gi-or-name)
  "%t")

(define (et-listitem-xref-string gi-or-name)
  "%n")

(define (et-part-xref-string gi-or-name)
  "&Part; %n")

(define (et-preface-xref-string gi-or-name)
  "%t")

(define (et-procedure-xref-string gi-or-name)
  "&Procedure; %n, %t")

(define (et-reference-xref-string gi-or-name)
  "&Reference; %n, %t")

(define (et-sectioning-xref-string gi-or-name)
  (if %section-autolabel% 
      "&Section; %n" 
      "the &section; called %t"))

(define (et-sect1-xref-string gi-or-name)
  (et-sectioning-xref-string gi-or-name))

(define (et-sect2-xref-string gi-or-name)
  (et-sectioning-xref-string gi-or-name))

(define (et-sect3-xref-string gi-or-name)
  (et-sectioning-xref-string gi-or-name))

(define (et-sect4-xref-string gi-or-name)
  (et-sectioning-xref-string gi-or-name))

(define (et-sect5-xref-string gi-or-name)
  (et-sectioning-xref-string gi-or-name))

(define (et-section-xref-string gi-or-name)
  (et-sectioning-xref-string gi-or-name))

(define (et-sidebar-xref-string gi-or-name)
  "the &sidebar; %t")

(define (et-step-xref-string gi-or-name)
  "&step; %n")

(define (et-table-xref-string gi-or-name)
  "&Table; %n")

(define (et-default-xref-string gi-or-name)
  (let* ((giname (if (string? gi-or-name) gi-or-name (gi gi-or-name)))
	 (msg    (string-append "[&xrefto; "
				(if giname giname "&nonexistantelement;")
				" &unsupported;]"))
	 (err    (node-list-error msg (current-node))))
    msg))

(define (gentext-et-xref-strings gind)
  (let* ((giname (if (string? gind) gind (gi gind)))
	 (name   (normalize giname)))
    (cond
      ((equal? name (normalize "appendix")) (et-appendix-xref-string gind))
      ((equal? name (normalize "article"))  (et-article-xref-string gind))
      ((equal? name (normalize "bibliography")) (et-bibliography-xref-string gind))
      ((equal? name (normalize "book"))     (et-book-xref-string gind))
      ((equal? name (normalize "chapter"))  (et-chapter-xref-string gind))
      ((equal? name (normalize "equation")) (et-equation-xref-string gind))
      ((equal? name (normalize "example"))  (et-example-xref-string gind))
      ((equal? name (normalize "figure"))   (et-figure-xref-string gind))
      ((equal? name (normalize "glossary")) (et-glossary-xref-string gind))
      ((equal? name (normalize "index"))    (et-index-xref-string gind))
      ((equal? name (normalize "listitem")) (et-listitem-xref-string gind))
      ((equal? name (normalize "part"))     (et-part-xref-string gind))
      ((equal? name (normalize "preface"))  (et-preface-xref-string gind))
      ((equal? name (normalize "procedure")) (et-procedure-xref-string gind))
      ((equal? name (normalize "reference")) (et-reference-xref-string gind))
      ((equal? name (normalize "sect1"))    (et-sect1-xref-string gind))
      ((equal? name (normalize "sect2"))    (et-sect2-xref-string gind))
      ((equal? name (normalize "sect3"))    (et-sect3-xref-string gind))
      ((equal? name (normalize "sect4"))    (et-sect4-xref-string gind))
      ((equal? name (normalize "sect5"))    (et-sect5-xref-string gind))
      ((equal? name (normalize "section"))  (et-section-xref-string gind))
      ((equal? name (normalize "simplesect"))  (et-section-xref-string gind))
      ((equal? name (normalize "sidebar"))  (et-sidebar-xref-string gind))
      ((equal? name (normalize "step"))     (et-step-xref-string gind))
      ((equal? name (normalize "table"))    (et-table-xref-string gind))
      (else (et-default-xref-string gind)))))

(define (et-auto-xref-indirect-connector before) 
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
(define %generate-et-toc-in-front% #t)

;; gentext-element-name returns the generated text that should be 
;; used to make reference to the selected element.
;;
(define et-abstract-name	"&Abstract;")
(define et-answer-name		"&Answer;")
(define et-appendix-name	"&Appendix;")
(define et-article-name	"&Article;")
(define et-bibliography-name	"&Bibliography;")
(define et-book-name		"&Book;")
(define et-calloutlist-name	"")
(define et-caution-name	"&Caution;")
(define et-chapter-name	"&Chapter;")
(define et-copyright-name	"&Copyright;")
(define et-dedication-name	"&Dedication;")
(define et-edition-name	"&Edition;")
(define et-equation-name	"&Equation;")
(define et-example-name	"&Example;")
(define et-figure-name	"&Figure;")
(define et-glossary-name	"&Glossary;")
(define et-glosssee-name	"&GlossSee;")
(define et-glossseealso-name	"&GlossSeeAlso;")
(define et-important-name	"&Important;")
(define et-index-name		"&Index;")
(define et-colophon-name	"&Colophon;")
(define et-setindex-name	"&SetIndex;")
(define et-isbn-name		"&ISBN;")
(define et-legalnotice-name	"&LegalNotice;")
(define et-msgaud-name	"&MsgAud;")
(define et-msglevel-name	"&MsgLevel;")
(define et-msgorig-name	"&MsgOrig;")
(define et-note-name		"&Note;")
(define et-part-name		"&Part;")
(define et-preface-name	"&Preface;")
(define et-procedure-name	"&Procedure;")
(define et-pubdate-name	"&Published;")
(define et-question-name	"&Question;")
(define et-refentry-name      "&RefEntry;")
(define et-reference-name	"&Reference;")
(define et-refname-name	"&RefName;")
(define et-revhistory-name	"&RevHistory;")
(define et-refsect1-name      "&RefSection;")
(define et-refsect2-name      "&RefSection;")
(define et-refsect3-name      "&RefSection;")
(define et-refsynopsisdiv-name      "&RefSynopsisDiv;")
(define et-revision-name	"&Revision;")
(define et-sect1-name		"&Section;")
(define et-sect2-name		"&Section;")
(define et-sect3-name		"&Section;")
(define et-sect4-name		"&Section;")
(define et-sect5-name		"&Section;")
(define et-section-name		"&Section;")
(define et-simplesect-name	"&Section;")
(define et-seeie-name		"&See;")
(define et-seealsoie-name	"&Seealso;")
(define et-set-name		"&Set;")
(define et-sidebar-name	"&Sidebar;")
(define et-step-name		"&step;")
(define et-table-name		"&Table;")
(define et-tip-name		"&Tip;")
(define et-toc-name		"&TableofContents;")
(define et-warning-name	"&Warning;")

(define (gentext-et-element-name gind)
  (let* ((giname (if (string? gind) gind (gi gind)))
	 (name   (normalize giname)))
    (cond
     ((equal? name (normalize "abstract"))	et-abstract-name)
     ((equal? name (normalize "answer"))	et-answer-name)
     ((equal? name (normalize "appendix"))	et-appendix-name)
     ((equal? name (normalize "article"))	et-article-name)
     ((equal? name (normalize "bibliography"))	et-bibliography-name)
     ((equal? name (normalize "book"))		et-book-name)
     ((equal? name (normalize "calloutlist"))	et-calloutlist-name)
     ((equal? name (normalize "caution"))	et-caution-name)
     ((equal? name (normalize "chapter"))	et-chapter-name)
     ((equal? name (normalize "copyright"))	et-copyright-name)
     ((equal? name (normalize "dedication"))	et-dedication-name)
     ((equal? name (normalize "edition"))	et-edition-name)
     ((equal? name (normalize "equation"))	et-equation-name)
     ((equal? name (normalize "example"))	et-example-name)
     ((equal? name (normalize "figure"))	et-figure-name)
     ((equal? name (normalize "glossary"))	et-glossary-name)
     ((equal? name (normalize "glosssee"))	et-glosssee-name)
     ((equal? name (normalize "glossseealso"))	et-glossseealso-name)
     ((equal? name (normalize "important"))	et-important-name)
     ((equal? name (normalize "index"))		et-index-name)
     ((equal? name (normalize "colophon"))	et-colophon-name)
     ((equal? name (normalize "setindex"))	et-setindex-name)
     ((equal? name (normalize "isbn"))		et-isbn-name)
     ((equal? name (normalize "legalnotice"))	et-legalnotice-name)
     ((equal? name (normalize "msgaud"))	et-msgaud-name)
     ((equal? name (normalize "msglevel"))	et-msglevel-name)
     ((equal? name (normalize "msgorig"))	et-msgorig-name)
     ((equal? name (normalize "note"))		et-note-name)
     ((equal? name (normalize "part"))		et-part-name)
     ((equal? name (normalize "preface"))	et-preface-name)
     ((equal? name (normalize "procedure"))	et-procedure-name)
     ((equal? name (normalize "pubdate"))	et-pubdate-name)
     ((equal? name (normalize "question"))	et-question-name)
     ((equal? name (normalize "refentry"))	et-refentry-name)
     ((equal? name (normalize "reference"))	et-reference-name)
     ((equal? name (normalize "refname"))	et-refname-name)
     ((equal? name (normalize "revhistory"))	et-revhistory-name)
     ((equal? name (normalize "refsect1"))	et-refsect1-name)
     ((equal? name (normalize "refsect2"))	et-refsect2-name)
     ((equal? name (normalize "refsect3"))	et-refsect3-name)
     ((equal? name (normalize "refsynopsisdiv"))	et-refsynopsisdiv-name)
     ((equal? name (normalize "revision"))	et-revision-name)
     ((equal? name (normalize "sect1"))		et-sect1-name)
     ((equal? name (normalize "sect2"))		et-sect2-name)
     ((equal? name (normalize "sect3"))		et-sect3-name)
     ((equal? name (normalize "sect4"))		et-sect4-name)
     ((equal? name (normalize "sect5"))		et-sect5-name)
     ((equal? name (normalize "section"))	et-section-name)
     ((equal? name (normalize "simplesect"))    et-simplesect-name)
     ((equal? name (normalize "seeie"))		et-seeie-name)
     ((equal? name (normalize "seealsoie"))	et-seealsoie-name)
     ((equal? name (normalize "set"))		et-set-name)
     ((equal? name (normalize "sidebar"))	et-sidebar-name)
     ((equal? name (normalize "step"))		et-step-name)
     ((equal? name (normalize "table"))		et-table-name)
     ((equal? name (normalize "tip"))		et-tip-name)
     ((equal? name (normalize "toc"))		et-toc-name)
     ((equal? name (normalize "warning"))	et-warning-name)
     (else (let* ((msg (string-append "gentext-et-element-name: &unexpectedelementname;: " name))
		  (err (node-list-error msg (current-node))))
	     msg)))))

;; gentext-element-name-space returns gentext-element-name with a 
;; trailing space, if gentext-element-name isn't "".
;;
(define (gentext-et-element-name-space giname)
  (string-with-space (gentext-element-name giname)))

;; gentext-intra-label-sep returns the seperator to be inserted
;; between multiple occurances of a label (or parts of a label)
;; for the specified element.  Most of these are for enumerated
;; labels like "Figure 2-4", but this function is used elsewhere
;; (e.g. REFNAME) with a little abuse.
;;

(define et-equation-intra-label-sep "-")
(define et-informalequation-intra-label-sep "-")
(define et-example-intra-label-sep "-")
(define et-figure-intra-label-sep "-")
(define et-listitem-intra-label-sep ".")
(define et-procedure-intra-label-sep ".")
(define et-refentry-intra-label-sep ".")
(define et-reference-intra-label-sep ".")
(define et-refname-intra-label-sep ", ")
(define et-refsect1-intra-label-sep ".")
(define et-refsect2-intra-label-sep ".")
(define et-refsect3-intra-label-sep ".")
(define et-sect1-intra-label-sep ".")
(define et-sect2-intra-label-sep ".")
(define et-sect3-intra-label-sep ".")
(define et-sect4-intra-label-sep ".")
(define et-sect5-intra-label-sep ".")
(define et-section-intra-label-sep ".")
(define et-simplesect-intra-label-sep ".")
(define et-step-intra-label-sep ".")
(define et-table-intra-label-sep "-")
(define et-_pagenumber-intra-label-sep "-")
(define et-default-intra-label-sep "")

(define (gentext-et-intra-label-sep gind)
  (let* ((giname (if (string? gind) gind (gi gind)))
	 (name   (normalize giname)))
    (cond
     ((equal? name (normalize "equation"))	et-equation-intra-label-sep)
     ((equal? name (normalize "informalequation"))	et-informalequation-intra-label-sep)
     ((equal? name (normalize "example"))	et-example-intra-label-sep)
     ((equal? name (normalize "figure"))	et-figure-intra-label-sep)
     ((equal? name (normalize "listitem"))	et-listitem-intra-label-sep)
     ((equal? name (normalize "procedure"))	et-procedure-intra-label-sep)
     ((equal? name (normalize "refentry"))	et-refentry-intra-label-sep)
     ((equal? name (normalize "reference"))	et-reference-intra-label-sep)
     ((equal? name (normalize "refname"))	et-refname-intra-label-sep)
     ((equal? name (normalize "refsect1"))	et-refsect1-intra-label-sep)
     ((equal? name (normalize "refsect2"))	et-refsect2-intra-label-sep)
     ((equal? name (normalize "refsect3"))	et-refsect3-intra-label-sep)
     ((equal? name (normalize "sect1"))		et-sect1-intra-label-sep)
     ((equal? name (normalize "sect2"))		et-sect2-intra-label-sep)
     ((equal? name (normalize "sect3"))		et-sect3-intra-label-sep)
     ((equal? name (normalize "sect4"))		et-sect4-intra-label-sep)
     ((equal? name (normalize "sect5"))		et-sect5-intra-label-sep)
     ((equal? name (normalize "section"))	et-section-intra-label-sep)
     ((equal? name (normalize "simplesect"))	et-simplesect-intra-label-sep)
     ((equal? name (normalize "step"))		et-step-intra-label-sep)
     ((equal? name (normalize "table"))		et-table-intra-label-sep)
     ((equal? name (normalize "_pagenumber"))	et-_pagenumber-intra-label-sep)
     (else et-default-intra-label-sep))))

;; gentext-label-title-sep returns the seperator to be inserted
;; between a label and the text following the label for the
;; specified element.  Most of these are for use between
;; enumerated labels and titles like "1. Chapter One Title", but
;; this function is used elsewhere (e.g. NOTE) with a little
;; abuse.
;;

(define et-abstract-label-title-sep ": ")
(define et-answer-label-title-sep " ")
(define et-appendix-label-title-sep ". ")
(define et-caution-label-title-sep "")
(define et-chapter-label-title-sep ". ")
(define et-equation-label-title-sep ". ")
(define et-example-label-title-sep ". ")
(define et-figure-label-title-sep ". ")
(define et-footnote-label-title-sep ". ")
(define et-glosssee-label-title-sep ": ")
(define et-glossseealso-label-title-sep ": ")
(define et-important-label-title-sep ": ")
(define et-note-label-title-sep ": ")
(define et-orderedlist-label-title-sep ". ")
(define et-part-label-title-sep ". ")
(define et-procedure-label-title-sep ". ")
(define et-prefix-label-title-sep ". ")
(define et-question-label-title-sep " ")
(define et-refentry-label-title-sep "")
(define et-reference-label-title-sep ". ")
(define et-refsect1-label-title-sep ". ")
(define et-refsect2-label-title-sep ". ")
(define et-refsect3-label-title-sep ". ")
(define et-sect1-label-title-sep ". ")
(define et-sect2-label-title-sep ". ")
(define et-sect3-label-title-sep ". ")
(define et-sect4-label-title-sep ". ")
(define et-sect5-label-title-sep ". ")
(define et-section-label-title-sep ". ")
(define et-simplesect-label-title-sep ". ")
(define et-seeie-label-title-sep " ")
(define et-seealsoie-label-title-sep " ")
(define et-step-label-title-sep ". ")
(define et-table-label-title-sep ". ")
(define et-tip-label-title-sep ": ")
(define et-warning-label-title-sep "")
(define et-default-label-title-sep "")

(define (gentext-et-label-title-sep gind)
  (let* ((giname (if (string? gind) gind (gi gind)))
	 (name   (normalize giname)))
    (cond
     ((equal? name (normalize "abstract")) et-abstract-label-title-sep)
     ((equal? name (normalize "answer")) et-answer-label-title-sep)
     ((equal? name (normalize "appendix")) et-appendix-label-title-sep)
     ((equal? name (normalize "caution")) et-caution-label-title-sep)
     ((equal? name (normalize "chapter")) et-chapter-label-title-sep)
     ((equal? name (normalize "equation")) et-equation-label-title-sep)
     ((equal? name (normalize "example")) et-example-label-title-sep)
     ((equal? name (normalize "figure")) et-figure-label-title-sep)
     ((equal? name (normalize "footnote")) et-footnote-label-title-sep)
     ((equal? name (normalize "glosssee")) et-glosssee-label-title-sep)
     ((equal? name (normalize "glossseealso")) et-glossseealso-label-title-sep)
     ((equal? name (normalize "important")) et-important-label-title-sep)
     ((equal? name (normalize "note")) et-note-label-title-sep)
     ((equal? name (normalize "orderedlist")) et-orderedlist-label-title-sep)
     ((equal? name (normalize "part")) et-part-label-title-sep)
     ((equal? name (normalize "procedure")) et-procedure-label-title-sep)
     ((equal? name (normalize "prefix")) et-prefix-label-title-sep)
     ((equal? name (normalize "question")) et-question-label-title-sep)
     ((equal? name (normalize "refentry")) et-refentry-label-title-sep)
     ((equal? name (normalize "reference")) et-reference-label-title-sep)
     ((equal? name (normalize "refsect1")) et-refsect1-label-title-sep)
     ((equal? name (normalize "refsect2")) et-refsect2-label-title-sep)
     ((equal? name (normalize "refsect3")) et-refsect3-label-title-sep)
     ((equal? name (normalize "sect1")) et-sect1-label-title-sep)
     ((equal? name (normalize "sect2")) et-sect2-label-title-sep)
     ((equal? name (normalize "sect3")) et-sect3-label-title-sep)
     ((equal? name (normalize "sect4")) et-sect4-label-title-sep)
     ((equal? name (normalize "sect5")) et-sect5-label-title-sep)
     ((equal? name (normalize "section")) et-section-label-title-sep)
     ((equal? name (normalize "simplesect")) et-simplesect-label-title-sep)
     ((equal? name (normalize "seeie")) et-seeie-label-title-sep)
     ((equal? name (normalize "seealsoie")) et-seealsoie-label-title-sep)
     ((equal? name (normalize "step")) et-step-label-title-sep)
     ((equal? name (normalize "table")) et-table-label-title-sep)
     ((equal? name (normalize "tip")) et-tip-label-title-sep)
     ((equal? name (normalize "warning")) et-warning-label-title-sep)
     (else et-default-label-title-sep))))

(define (et-set-label-number-format gind) "1")
(define (et-book-label-number-format gind) "1")
(define (et-prefix-label-number-format gind) "1")
(define (et-part-label-number-format gind) "I")
(define (et-chapter-label-number-format gind) "1")
(define (et-appendix-label-number-format gind) "A")
(define (et-reference-label-number-format gind) "I")
(define (et-example-label-number-format gind) "1")
(define (et-figure-label-number-format gind) "1")
(define (et-table-label-number-format gind) "1")
(define (et-procedure-label-number-format gind) "1")
(define (et-step-label-number-format gind) "1")
(define (et-refsect1-label-number-format gind) "1")
(define (et-refsect2-label-number-format gind) "1")
(define (et-refsect3-label-number-format gind) "1")
(define (et-sect1-label-number-format gind) "1")
(define (et-sect2-label-number-format gind) "1")
(define (et-sect3-label-number-format gind) "1")
(define (et-sect4-label-number-format gind) "1")
(define (et-sect5-label-number-format gind) "1")
(define (et-section-label-number-format gind) "1")
(define (et-default-label-number-format gind) "1")

(define (et-label-number-format gind)
  (let* ((giname (if (string? gind) gind (gi gind)))
	 (name   (normalize giname)))
    (cond
     ((equal? name (normalize "set")) (et-set-label-number-format gind))
     ((equal? name (normalize "book")) (et-book-label-number-format gind))
     ((equal? name (normalize "prefix")) (et-prefix-label-number-format gind))
     ((equal? name (normalize "part")) (et-part-label-number-format gind))
     ((equal? name (normalize "chapter")) (et-chapter-label-number-format gind))
     ((equal? name (normalize "appendix")) (et-appendix-label-number-format gind))
     ((equal? name (normalize "reference")) (et-reference-label-number-format gind))
     ((equal? name (normalize "example")) (et-example-label-number-format gind))
     ((equal? name (normalize "figure")) (et-figure-label-number-format gind))
     ((equal? name (normalize "table")) (et-table-label-number-format gind))
     ((equal? name (normalize "procedure")) (et-procedure-label-number-format gind))
     ((equal? name (normalize "step")) (et-step-label-number-format gind))
     ((equal? name (normalize "refsect1")) (et-refsect1-label-number-format gind))
     ((equal? name (normalize "refsect2")) (et-refsect2-label-number-format gind))
     ((equal? name (normalize "refsect3")) (et-refsect3-label-number-format gind))
     ((equal? name (normalize "sect1")) (et-sect1-label-number-format gind))
     ((equal? name (normalize "sect2")) (et-sect2-label-number-format gind))
     ((equal? name (normalize "sect3")) (et-sect3-label-number-format gind))
     ((equal? name (normalize "sect4")) (et-sect4-label-number-format gind))
     ((equal? name (normalize "sect5")) (et-sect5-label-number-format gind))
     ((equal? name (normalize "section")) (et-section-label-number-format gind))
     (else (et-default-label-number-format gind)))))

(define ($lot-title-et$ gind)
  (let* ((giname (if (string? gind) gind (gi gind)))
	 (name   (normalize giname)))
    (cond ((equal? name (normalize "table"))    "&ListofTables;")
	  ((equal? name (normalize "example"))  "&ListofExamples;")
	  ((equal? name (normalize "figure"))   "&ListofFigures;")
	  ((equal? name (normalize "equation")) "&ListofEquations;")
	  (else (let* ((msg (string-append "&ListofUnknown;: " name))
		       (err (node-list-error msg (current-node))))
		  msg)))))

(define %gentext-et-start-quote% (dingbat "ldquo"))

(define %gentext-et-end-quote% (dingbat "rdquo"))

(define %gentext-et-start-nested-quote% (dingbat "lsquo"))

(define %gentext-et-end-nested-quote% (dingbat "rsquo"))

(define %gentext-et-by% "&by;") ;; e.g. Copyright 1997 "by" A. Nonymous
                           ;; Authored "by" Jane Doe

(define %gentext-et-edited-by% "&Editedby;")
                           ;; "Edited by" Jane Doe

(define %gentext-et-page% "")

(define %gentext-et-and% "&and;")

(define %gentext-et-bibl-pages% "&Pgs;")

(define %gentext-et-endnotes% "&Notes;")

(define %gentext-et-table-endnotes% "&TableNotes;:")

(define %gentext-et-index-see% "&See;")

(define %gentext-et-index-seealso% "&SeeAlso;")

