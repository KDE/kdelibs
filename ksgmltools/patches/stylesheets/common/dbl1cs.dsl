;; $Id$
;;
;; This file is part of the Modular DocBook Stylesheet distribution.
;; See ../README or http://nwalsh.com/docbook/dsssl/
;;

;; ----------------------------- Localization -----------------------------

;; If you create a new version of this file, please send it to
;; Norman Walsh, ndw@nwalsh.com

;; Contributors:
;; Ralf Schleitzer, ralf.schleitzer@ixos.de

(define (cs-appendix-xref-string gi-or-name)
  (if %chapter-autolabel%
      "&Appendix; %n"
      "&appendix; nazvan\U-00E1; %t"))

(define (cs-article-xref-string gi-or-name)
  (string-append %gentext-cs-start-quote%
		 "%t"
		 %gentext-cs-end-quote%))

(define (cs-bibliography-xref-string gi-or-name)
  "%t")

(define (cs-book-xref-string gi-or-name)
  "%t")

(define (cs-chapter-xref-string gi-or-name)
  (if %chapter-autolabel%
      "&Chapter; %n"
      "&chapter; nazvan\U-00E1; %t"))

(define (cs-equation-xref-string gi-or-name)
  "&Equation; %n")

(define (cs-example-xref-string gi-or-name)
  "&Example; %n")

(define (cs-figure-xref-string gi-or-name)
  "&Figure; %n")

(define (cs-glossary-xref-string gi-or-name)
  "%t")

(define (cs-index-xref-string gi-or-name)
  "%t")

(define (cs-listitem-xref-string gi-or-name)
  "%n")

(define (cs-part-xref-string gi-or-name)
  "&Part; %n")

(define (cs-preface-xref-string gi-or-name)
  "%t")

(define (cs-procedure-xref-string gi-or-name)
  "&Procedure; %n, %t")

(define (cs-reference-xref-string gi-or-name)
  "&Reference; %n, %t")

(define (cs-sectioning-xref-string gi-or-name)
  (if %section-autolabel% 
      "&Section; %n" 
      "&section; nazvan\U-00E1; %t"))

(define (cs-sect1-xref-string gi-or-name)
  (cs-sectioning-xref-string gi-or-name))

(define (cs-sect2-xref-string gi-or-name)
  (cs-sectioning-xref-string gi-or-name))

(define (cs-sect3-xref-string gi-or-name)
  (cs-sectioning-xref-string gi-or-name))

(define (cs-sect4-xref-string gi-or-name)
  (cs-sectioning-xref-string gi-or-name))

(define (cs-sect5-xref-string gi-or-name)
  (cs-sectioning-xref-string gi-or-name))

(define (cs-section-xref-string gi-or-name)
  (cs-sectioning-xref-string gi-or-name))

(define (cs-sidebar-xref-string gi-or-name)
  "&sidebar; %t")

(define (cs-step-xref-string gi-or-name)
  "&step; %n")

(define (cs-table-xref-string gi-or-name)
  "&Table; %n")

(define (cs-default-xref-string gi-or-name)
  (let* ((giname (if (string? gi-or-name) gi-or-name (gi gi-or-name)))
	 (msg    (string-append "[&xrefto; "
				(if giname giname "&nonexistantelement;")
				" &unsupported;]"))
	 (err    (node-list-error msg (current-node))))
    msg))

(define (gentext-cs-xref-strings gind)
  (let* ((giname (if (string? gind) gind (gi gind)))
	 (name   (normalize giname)))
    (cond
      ((equal? name (normalize "appendix")) (cs-appendix-xref-string gind))
      ((equal? name (normalize "article"))  (cs-article-xref-string gind))
      ((equal? name (normalize "bibliography")) (cs-bibliography-xref-string gind))
      ((equal? name (normalize "book"))     (cs-book-xref-string gind))
      ((equal? name (normalize "chapter"))  (cs-chapter-xref-string gind))
      ((equal? name (normalize "equation")) (cs-equation-xref-string gind))
      ((equal? name (normalize "example"))  (cs-example-xref-string gind))
      ((equal? name (normalize "figure"))   (cs-figure-xref-string gind))
      ((equal? name (normalize "glossary")) (cs-glossary-xref-string gind))
      ((equal? name (normalize "index"))    (cs-index-xref-string gind))
      ((equal? name (normalize "listitem")) (cs-listitem-xref-string gind))
      ((equal? name (normalize "part"))     (cs-part-xref-string gind))
      ((equal? name (normalize "preface"))  (cs-preface-xref-string gind))
      ((equal? name (normalize "procedure")) (cs-procedure-xref-string gind))
      ((equal? name (normalize "reference")) (cs-reference-xref-string gind))
      ((equal? name (normalize "sect1"))    (cs-sect1-xref-string gind))
      ((equal? name (normalize "sect2"))    (cs-sect2-xref-string gind))
      ((equal? name (normalize "sect3"))    (cs-sect3-xref-string gind))
      ((equal? name (normalize "sect4"))    (cs-sect4-xref-string gind))
      ((equal? name (normalize "sect5"))    (cs-sect5-xref-string gind))
      ((equal? name (normalize "section"))  (cs-section-xref-string gind))
      ((equal? name (normalize "simplesect"))  (cs-section-xref-string gind))
      ((equal? name (normalize "sidebar"))  (cs-sidebar-xref-string gind))
      ((equal? name (normalize "step"))     (cs-step-xref-string gind))
      ((equal? name (normalize "table"))    (cs-table-xref-string gind))
      (else (cs-default-xref-string gind)))))

(define (cs-auto-xref-indirect-connector before) 
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
(define %generate-cs-toc-in-front% #t)

;; gentext-element-name returns the generated text that should be 
;; used to make reference to the selected element.
;;
(define cs-abstract-name	"&Abstract;")
(define cs-answer-name		"&Answer;")
(define cs-appendix-name	"&Appendix;")
(define cs-article-name	"&Article;")
(define cs-bibliography-name	"&Bibliography;")
(define cs-book-name		"&Book;")
(define cs-calloutlist-name	"")
(define cs-caution-name	"&Caution;")
(define cs-chapter-name	"&Chapter;")
(define cs-copyright-name	"&Copyright;")
(define cs-dedication-name	"&Dedication;")
(define cs-edition-name	"&Edition;")
(define cs-equation-name	"&Equation;")
(define cs-example-name	"&Example;")
(define cs-figure-name	"&Figure;")
(define cs-glossary-name	"&Glossary;")
(define cs-glosssee-name	"&GlossSee;")
(define cs-glossseealso-name	"&GlossSeeAlso;")
(define cs-important-name	"&Important;")
(define cs-index-name		"&Index;")
(define cs-colophon-name	"&Colophon;")
(define cs-setindex-name	"&SetIndex;")
(define cs-isbn-name		"&ISBN;")
(define cs-legalnotice-name	"&LegalNotice;")
(define cs-msgaud-name	"&MsgAud;")
(define cs-msglevel-name	"&MsgLevel;")
(define cs-msgorig-name	"&MsgOrig;")
(define cs-note-name		"&Note;")
(define cs-part-name		"&Part;")
(define cs-preface-name	"&Preface;")
(define cs-procedure-name	"&Procedure;")
(define cs-pubdate-name	"&Published;")
(define cs-question-name	"&Question;")
(define cs-refentry-name      "&RefEntry;")
(define cs-reference-name	"&Reference;")
(define cs-refname-name	"&RefName;")
(define cs-revhistory-name	"&RevHistory;")
(define cs-refsect1-name      "&RefSection;")
(define cs-refsect2-name      "&RefSection;")
(define cs-refsect3-name      "&RefSection;")
(define cs-refsynopsisdiv-name      "&RefSynopsisDiv;")
(define cs-revision-name	"&Revision;")
(define cs-sect1-name		"&Section;")
(define cs-sect2-name		"&Section;")
(define cs-sect3-name		"&Section;")
(define cs-sect4-name		"&Section;")
(define cs-sect5-name		"&Section;")
(define cs-section-name		"&Section;")
(define cs-simplesect-name	"&Section;")
(define cs-seeie-name		"&See;")
(define cs-seealsoie-name	"&Seealso;")
(define cs-set-name		"&Set;")
(define cs-sidebar-name	"&Sidebar;")
(define cs-step-name		"&step;")
(define cs-table-name		"&Table;")
(define cs-tip-name		"&Tip;")
(define cs-toc-name		"&TableofContents;")
(define cs-warning-name	"&Warning;")

(define (gentext-cs-element-name gind)
  (let* ((giname (if (string? gind) gind (gi gind)))
	 (name   (normalize giname)))
    (cond
     ((equal? name (normalize "abstract"))	cs-abstract-name)
     ((equal? name (normalize "answer"))	cs-answer-name)
     ((equal? name (normalize "appendix"))	cs-appendix-name)
     ((equal? name (normalize "article"))	cs-article-name)
     ((equal? name (normalize "bibliography"))	cs-bibliography-name)
     ((equal? name (normalize "book"))		cs-book-name)
     ((equal? name (normalize "calloutlist"))	cs-calloutlist-name)
     ((equal? name (normalize "caution"))	cs-caution-name)
     ((equal? name (normalize "chapter"))	cs-chapter-name)
     ((equal? name (normalize "copyright"))	cs-copyright-name)
     ((equal? name (normalize "dedication"))	cs-dedication-name)
     ((equal? name (normalize "edition"))	cs-edition-name)
     ((equal? name (normalize "equation"))	cs-equation-name)
     ((equal? name (normalize "example"))	cs-example-name)
     ((equal? name (normalize "figure"))	cs-figure-name)
     ((equal? name (normalize "glossary"))	cs-glossary-name)
     ((equal? name (normalize "glosssee"))	cs-glosssee-name)
     ((equal? name (normalize "glossseealso"))	cs-glossseealso-name)
     ((equal? name (normalize "important"))	cs-important-name)
     ((equal? name (normalize "index"))		cs-index-name)
     ((equal? name (normalize "colophon"))	cs-colophon-name)
     ((equal? name (normalize "setindex"))	cs-setindex-name)
     ((equal? name (normalize "isbn"))		cs-isbn-name)
     ((equal? name (normalize "legalnotice"))	cs-legalnotice-name)
     ((equal? name (normalize "msgaud"))	cs-msgaud-name)
     ((equal? name (normalize "msglevel"))	cs-msglevel-name)
     ((equal? name (normalize "msgorig"))	cs-msgorig-name)
     ((equal? name (normalize "note"))		cs-note-name)
     ((equal? name (normalize "part"))		cs-part-name)
     ((equal? name (normalize "preface"))	cs-preface-name)
     ((equal? name (normalize "procedure"))	cs-procedure-name)
     ((equal? name (normalize "pubdate"))	cs-pubdate-name)
     ((equal? name (normalize "question"))	cs-question-name)
     ((equal? name (normalize "refentry"))	cs-refentry-name)
     ((equal? name (normalize "reference"))	cs-reference-name)
     ((equal? name (normalize "refname"))	cs-refname-name)
     ((equal? name (normalize "revhistory"))	cs-revhistory-name)
     ((equal? name (normalize "refsect1"))	cs-refsect1-name)
     ((equal? name (normalize "refsect2"))	cs-refsect2-name)
     ((equal? name (normalize "refsect3"))	cs-refsect3-name)
     ((equal? name (normalize "refsynopsisdiv"))	cs-refsynopsisdiv-name)
     ((equal? name (normalize "revision"))	cs-revision-name)
     ((equal? name (normalize "sect1"))		cs-sect1-name)
     ((equal? name (normalize "sect2"))		cs-sect2-name)
     ((equal? name (normalize "sect3"))		cs-sect3-name)
     ((equal? name (normalize "sect4"))		cs-sect4-name)
     ((equal? name (normalize "sect5"))		cs-sect5-name)
     ((equal? name (normalize "section"))	cs-section-name)
     ((equal? name (normalize "simplesect"))    cs-simplesect-name)
     ((equal? name (normalize "seeie"))		cs-seeie-name)
     ((equal? name (normalize "seealsoie"))	cs-seealsoie-name)
     ((equal? name (normalize "set"))		cs-set-name)
     ((equal? name (normalize "sidebar"))	cs-sidebar-name)
     ((equal? name (normalize "step"))		cs-step-name)
     ((equal? name (normalize "table"))		cs-table-name)
     ((equal? name (normalize "tip"))		cs-tip-name)
     ((equal? name (normalize "toc"))		cs-toc-name)
     ((equal? name (normalize "warning"))	cs-warning-name)
     (else (let* ((msg (string-append "gentext-cs-element-name: &unexpectedelementname;: " name))
		  (err (node-list-error msg (current-node))))
	     msg)))))

;; gentext-element-name-space returns gentext-element-name with a 
;; trailing space, if gentext-element-name isn't "".
;;
(define (gentext-cs-element-name-space giname)
  (string-with-space (gentext-element-name giname)))

;; gentext-intra-label-sep returns the seperator to be inserted
;; between multiple occurances of a label (or parts of a label)
;; for the specified element.  Most of these are for enumerated
;; labels like "Figure 2-4", but this function is used elsewhere
;; (e.g. REFNAME) with a little abuse.
;;

(define cs-equation-intra-label-sep "-")
(define cs-informalequation-intra-label-sep "-")
(define cs-example-intra-label-sep "-")
(define cs-figure-intra-label-sep "-")
(define cs-listitem-intra-label-sep ".")
(define cs-procedure-intra-label-sep ".")
(define cs-refentry-intra-label-sep ".")
(define cs-reference-intra-label-sep ".")
(define cs-refname-intra-label-sep ", ")
(define cs-refsect1-intra-label-sep ".")
(define cs-refsect2-intra-label-sep ".")
(define cs-refsect3-intra-label-sep ".")
(define cs-sect1-intra-label-sep ".")
(define cs-sect2-intra-label-sep ".")
(define cs-sect3-intra-label-sep ".")
(define cs-sect4-intra-label-sep ".")
(define cs-sect5-intra-label-sep ".")
(define cs-section-intra-label-sep ".")
(define cs-simplesect-intra-label-sep ".")
(define cs-step-intra-label-sep ".")
(define cs-table-intra-label-sep "-")
(define cs-_pagenumber-intra-label-sep "-")
(define cs-default-intra-label-sep "")

(define (gentext-cs-intra-label-sep gind)
  (let* ((giname (if (string? gind) gind (gi gind)))
	 (name   (normalize giname)))
    (cond
     ((equal? name (normalize "equation"))	cs-equation-intra-label-sep)
     ((equal? name (normalize "informalequation"))	cs-informalequation-intra-label-sep)
     ((equal? name (normalize "example"))	cs-example-intra-label-sep)
     ((equal? name (normalize "figure"))	cs-figure-intra-label-sep)
     ((equal? name (normalize "listitem"))	cs-listitem-intra-label-sep)
     ((equal? name (normalize "procedure"))	cs-procedure-intra-label-sep)
     ((equal? name (normalize "refentry"))	cs-refentry-intra-label-sep)
     ((equal? name (normalize "reference"))	cs-reference-intra-label-sep)
     ((equal? name (normalize "refname"))	cs-refname-intra-label-sep)
     ((equal? name (normalize "refsect1"))	cs-refsect1-intra-label-sep)
     ((equal? name (normalize "refsect2"))	cs-refsect2-intra-label-sep)
     ((equal? name (normalize "refsect3"))	cs-refsect3-intra-label-sep)
     ((equal? name (normalize "sect1"))		cs-sect1-intra-label-sep)
     ((equal? name (normalize "sect2"))		cs-sect2-intra-label-sep)
     ((equal? name (normalize "sect3"))		cs-sect3-intra-label-sep)
     ((equal? name (normalize "sect4"))		cs-sect4-intra-label-sep)
     ((equal? name (normalize "sect5"))		cs-sect5-intra-label-sep)
     ((equal? name (normalize "section"))	cs-section-intra-label-sep)
     ((equal? name (normalize "simplesect"))	cs-simplesect-intra-label-sep)
     ((equal? name (normalize "step"))		cs-step-intra-label-sep)
     ((equal? name (normalize "table"))		cs-table-intra-label-sep)
     ((equal? name (normalize "_pagenumber"))	cs-_pagenumber-intra-label-sep)
     (else cs-default-intra-label-sep))))

;; gentext-label-title-sep returns the seperator to be inserted
;; between a label and the text following the label for the
;; specified element.  Most of these are for use between
;; enumerated labels and titles like "1. Chapter One Title", but
;; this function is used elsewhere (e.g. NOTE) with a little
;; abuse.
;;

(define cs-abstract-label-title-sep ": ")
(define cs-answer-label-title-sep " ")
(define cs-appendix-label-title-sep ". ")
(define cs-caution-label-title-sep "")
(define cs-chapter-label-title-sep ". ")
(define cs-equation-label-title-sep ". ")
(define cs-example-label-title-sep ". ")
(define cs-figure-label-title-sep ". ")
(define cs-footnote-label-title-sep ". ")
(define cs-glosssee-label-title-sep ": ")
(define cs-glossseealso-label-title-sep ": ")
(define cs-important-label-title-sep ": ")
(define cs-note-label-title-sep ": ")
(define cs-orderedlist-label-title-sep ". ")
(define cs-part-label-title-sep ". ")
(define cs-procedure-label-title-sep ". ")
(define cs-prefix-label-title-sep ". ")
(define cs-question-label-title-sep " ")
(define cs-refentry-label-title-sep "")
(define cs-reference-label-title-sep ". ")
(define cs-refsect1-label-title-sep ". ")
(define cs-refsect2-label-title-sep ". ")
(define cs-refsect3-label-title-sep ". ")
(define cs-sect1-label-title-sep ". ")
(define cs-sect2-label-title-sep ". ")
(define cs-sect3-label-title-sep ". ")
(define cs-sect4-label-title-sep ". ")
(define cs-sect5-label-title-sep ". ")
(define cs-section-label-title-sep ". ")
(define cs-simplesect-label-title-sep ". ")
(define cs-seeie-label-title-sep " ")
(define cs-seealsoie-label-title-sep " ")
(define cs-step-label-title-sep ". ")
(define cs-table-label-title-sep ". ")
(define cs-tip-label-title-sep ": ")
(define cs-warning-label-title-sep "")
(define cs-default-label-title-sep "")

(define (gentext-cs-label-title-sep gind)
  (let* ((giname (if (string? gind) gind (gi gind)))
	 (name   (normalize giname)))
    (cond
     ((equal? name (normalize "abstract")) cs-abstract-label-title-sep)
     ((equal? name (normalize "answer")) cs-answer-label-title-sep)
     ((equal? name (normalize "appendix")) cs-appendix-label-title-sep)
     ((equal? name (normalize "caution")) cs-caution-label-title-sep)
     ((equal? name (normalize "chapter")) cs-chapter-label-title-sep)
     ((equal? name (normalize "equation")) cs-equation-label-title-sep)
     ((equal? name (normalize "example")) cs-example-label-title-sep)
     ((equal? name (normalize "figure")) cs-figure-label-title-sep)
     ((equal? name (normalize "footnote")) cs-footnote-label-title-sep)
     ((equal? name (normalize "glosssee")) cs-glosssee-label-title-sep)
     ((equal? name (normalize "glossseealso")) cs-glossseealso-label-title-sep)
     ((equal? name (normalize "important")) cs-important-label-title-sep)
     ((equal? name (normalize "note")) cs-note-label-title-sep)
     ((equal? name (normalize "orderedlist")) cs-orderedlist-label-title-sep)
     ((equal? name (normalize "part")) cs-part-label-title-sep)
     ((equal? name (normalize "procedure")) cs-procedure-label-title-sep)
     ((equal? name (normalize "prefix")) cs-prefix-label-title-sep)
     ((equal? name (normalize "question")) cs-question-label-title-sep)
     ((equal? name (normalize "refentry")) cs-refentry-label-title-sep)
     ((equal? name (normalize "reference")) cs-reference-label-title-sep)
     ((equal? name (normalize "refsect1")) cs-refsect1-label-title-sep)
     ((equal? name (normalize "refsect2")) cs-refsect2-label-title-sep)
     ((equal? name (normalize "refsect3")) cs-refsect3-label-title-sep)
     ((equal? name (normalize "sect1")) cs-sect1-label-title-sep)
     ((equal? name (normalize "sect2")) cs-sect2-label-title-sep)
     ((equal? name (normalize "sect3")) cs-sect3-label-title-sep)
     ((equal? name (normalize "sect4")) cs-sect4-label-title-sep)
     ((equal? name (normalize "sect5")) cs-sect5-label-title-sep)
     ((equal? name (normalize "section")) cs-section-label-title-sep)
     ((equal? name (normalize "simplesect")) cs-simplesect-label-title-sep)
     ((equal? name (normalize "seeie")) cs-seeie-label-title-sep)
     ((equal? name (normalize "seealsoie")) cs-seealsoie-label-title-sep)
     ((equal? name (normalize "step")) cs-step-label-title-sep)
     ((equal? name (normalize "table")) cs-table-label-title-sep)
     ((equal? name (normalize "tip")) cs-tip-label-title-sep)
     ((equal? name (normalize "warning")) cs-warning-label-title-sep)
     (else cs-default-label-title-sep))))

(define (cs-set-label-number-format gind) "1")
(define (cs-book-label-number-format gind) "1")
(define (cs-prefix-label-number-format gind) "1")
(define (cs-part-label-number-format gind) "I")
(define (cs-chapter-label-number-format gind) "1")
(define (cs-appendix-label-number-format gind) "A")
(define (cs-reference-label-number-format gind) "I")
(define (cs-example-label-number-format gind) "1")
(define (cs-figure-label-number-format gind) "1")
(define (cs-table-label-number-format gind) "1")
(define (cs-procedure-label-number-format gind) "1")
(define (cs-step-label-number-format gind) "1")
(define (cs-refsect1-label-number-format gind) "1")
(define (cs-refsect2-label-number-format gind) "1")
(define (cs-refsect3-label-number-format gind) "1")
(define (cs-sect1-label-number-format gind) "1")
(define (cs-sect2-label-number-format gind) "1")
(define (cs-sect3-label-number-format gind) "1")
(define (cs-sect4-label-number-format gind) "1")
(define (cs-sect5-label-number-format gind) "1")
(define (cs-section-label-number-format gind) "1")
(define (cs-default-label-number-format gind) "1")

(define (cs-label-number-format gind)
  (let* ((giname (if (string? gind) gind (gi gind)))
	 (name   (normalize giname)))
    (cond
     ((equal? name (normalize "set")) (cs-set-label-number-format gind))
     ((equal? name (normalize "book")) (cs-book-label-number-format gind))
     ((equal? name (normalize "prefix")) (cs-prefix-label-number-format gind))
     ((equal? name (normalize "part")) (cs-part-label-number-format gind))
     ((equal? name (normalize "chapter")) (cs-chapter-label-number-format gind))
     ((equal? name (normalize "appendix")) (cs-appendix-label-number-format gind))
     ((equal? name (normalize "reference")) (cs-reference-label-number-format gind))
     ((equal? name (normalize "example")) (cs-example-label-number-format gind))
     ((equal? name (normalize "figure")) (cs-figure-label-number-format gind))
     ((equal? name (normalize "table")) (cs-table-label-number-format gind))
     ((equal? name (normalize "procedure")) (cs-procedure-label-number-format gind))
     ((equal? name (normalize "step")) (cs-step-label-number-format gind))
     ((equal? name (normalize "refsect1")) (cs-refsect1-label-number-format gind))
     ((equal? name (normalize "refsect2")) (cs-refsect2-label-number-format gind))
     ((equal? name (normalize "refsect3")) (cs-refsect3-label-number-format gind))
     ((equal? name (normalize "sect1")) (cs-sect1-label-number-format gind))
     ((equal? name (normalize "sect2")) (cs-sect2-label-number-format gind))
     ((equal? name (normalize "sect3")) (cs-sect3-label-number-format gind))
     ((equal? name (normalize "sect4")) (cs-sect4-label-number-format gind))
     ((equal? name (normalize "sect5")) (cs-sect5-label-number-format gind))
     ((equal? name (normalize "section")) (cs-section-label-number-format gind))
     (else (cs-default-label-number-format gind)))))

(define ($lot-title-cs$ gind)
  (let* ((giname (if (string? gind) gind (gi gind)))
	 (name   (normalize giname)))
    (cond ((equal? name (normalize "table"))    "&ListofTables;")
	  ((equal? name (normalize "example"))  "&ListofExamples;")
	  ((equal? name (normalize "figure"))   "&ListofFigures;")
	  ((equal? name (normalize "equation")) "&ListofEquations;")
	  (else (let* ((msg (string-append "&ListofUnknown;: " name))
		       (err (node-list-error msg (current-node))))
		  msg)))))

(define %gentext-cs-start-quote% "\U-201E;")

(define %gentext-cs-end-quote% "\U-201C;")

(define %gentext-cs-start-nested-quote% "\U-201A;")

(define %gentext-cs-end-nested-quote% "\U-2018;")

(define %gentext-cs-by% "&by;") ;; e.g. Copyright 1997 "by" A. Nonymous
                           ;; Authored "by" Jane Doe

(define %gentext-cs-edited-by% "&Editedby;")
                           ;; "Edited by" Jane Doe

(define %gentext-cs-page% "")

(define %gentext-cs-and% "&and;")

(define %gentext-cs-bibl-pages% "&Pgs;")

(define %gentext-cs-endnotes% "&Notes;")

(define %gentext-cs-table-endnotes% "&TableNotes;:")

(define %gentext-cs-index-see% "&See;")

(define %gentext-cs-index-seealso% "&SeeAlso;")

