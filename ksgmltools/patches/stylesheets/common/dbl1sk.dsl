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

(define (sk-appendix-xref-string gi-or-name)
  (if %chapter-autolabel%
      "&Appendix; %n"
      "&appendix; nazvan\U-00E1; %t"))

(define (sk-article-xref-string gi-or-name)
  (string-append %gentext-sk-start-quote%
		 "%t"
		 %gentext-sk-end-quote%))

(define (sk-bibliography-xref-string gi-or-name)
  "%t")

(define (sk-book-xref-string gi-or-name)
  "%t")

(define (sk-chapter-xref-string gi-or-name)
  (if %chapter-autolabel%
      "&Chapter; %n"
      "&chapter; nazvan\U-00E1; %t"))

(define (sk-equation-xref-string gi-or-name)
  "&Equation; %n")

(define (sk-example-xref-string gi-or-name)
  "&Example; %n")

(define (sk-figure-xref-string gi-or-name)
  "&Figure; %n")

(define (sk-glossary-xref-string gi-or-name)
  "%t")

(define (sk-index-xref-string gi-or-name)
  "%t")

(define (sk-listitem-xref-string gi-or-name)
  "%n")

(define (sk-part-xref-string gi-or-name)
  "&Part; %n")

(define (sk-preface-xref-string gi-or-name)
  "%t")

(define (sk-procedure-xref-string gi-or-name)
  "&Procedure; %n, %t")

(define (sk-reference-xref-string gi-or-name)
  "&Reference; %n, %t")

(define (sk-sectioning-xref-string gi-or-name)
  (if %section-autolabel% 
      "&Section; %n" 
      "&section; nazvan\U-00E1; %t"))

(define (sk-sect1-xref-string gi-or-name)
  (sk-sectioning-xref-string gi-or-name))

(define (sk-sect2-xref-string gi-or-name)
  (sk-sectioning-xref-string gi-or-name))

(define (sk-sect3-xref-string gi-or-name)
  (sk-sectioning-xref-string gi-or-name))

(define (sk-sect4-xref-string gi-or-name)
  (sk-sectioning-xref-string gi-or-name))

(define (sk-sect5-xref-string gi-or-name)
  (sk-sectioning-xref-string gi-or-name))

(define (sk-section-xref-string gi-or-name)
  (sk-sectioning-xref-string gi-or-name))

(define (sk-sidebar-xref-string gi-or-name)
  "&sidebar; %t")

(define (sk-step-xref-string gi-or-name)
  "&step; %n")

(define (sk-table-xref-string gi-or-name)
  "&Table; %n")

(define (sk-default-xref-string gi-or-name)
  (let* ((giname (if (string? gi-or-name) gi-or-name (gi gi-or-name)))
	 (msg    (string-append "[&xrefto; "
				(if giname giname "&nonexistantelement;")
				" &unsupported;]"))
	 (err    (node-list-error msg (current-node))))
    msg))

(define (gentext-sk-xref-strings gind)
  (let* ((giname (if (string? gind) gind (gi gind)))
	 (name   (normalize giname)))
    (cond
      ((equal? name (normalize "appendix")) (sk-appendix-xref-string gind))
      ((equal? name (normalize "article"))  (sk-article-xref-string gind))
      ((equal? name (normalize "bibliography")) (sk-bibliography-xref-string gind))
      ((equal? name (normalize "book"))     (sk-book-xref-string gind))
      ((equal? name (normalize "chapter"))  (sk-chapter-xref-string gind))
      ((equal? name (normalize "equation")) (sk-equation-xref-string gind))
      ((equal? name (normalize "example"))  (sk-example-xref-string gind))
      ((equal? name (normalize "figure"))   (sk-figure-xref-string gind))
      ((equal? name (normalize "glossary")) (sk-glossary-xref-string gind))
      ((equal? name (normalize "index"))    (sk-index-xref-string gind))
      ((equal? name (normalize "listitem")) (sk-listitem-xref-string gind))
      ((equal? name (normalize "part"))     (sk-part-xref-string gind))
      ((equal? name (normalize "preface"))  (sk-preface-xref-string gind))
      ((equal? name (normalize "procedure")) (sk-procedure-xref-string gind))
      ((equal? name (normalize "reference")) (sk-reference-xref-string gind))
      ((equal? name (normalize "sect1"))    (sk-sect1-xref-string gind))
      ((equal? name (normalize "sect2"))    (sk-sect2-xref-string gind))
      ((equal? name (normalize "sect3"))    (sk-sect3-xref-string gind))
      ((equal? name (normalize "sect4"))    (sk-sect4-xref-string gind))
      ((equal? name (normalize "sect5"))    (sk-sect5-xref-string gind))
      ((equal? name (normalize "section"))  (sk-section-xref-string gind))
      ((equal? name (normalize "simplesect"))  (sk-section-xref-string gind))
      ((equal? name (normalize "sidebar"))  (sk-sidebar-xref-string gind))
      ((equal? name (normalize "step"))     (sk-step-xref-string gind))
      ((equal? name (normalize "table"))    (sk-table-xref-string gind))
      (else (sk-default-xref-string gind)))))

(define (sk-auto-xref-indirect-connector before) 
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
(define %generate-sk-toc-in-front% #t)

;; gentext-element-name returns the generated text that should be 
;; used to make reference to the selected element.
;;
(define sk-abstract-name	"&Abstract;")
(define sk-answer-name		"&Answer;")
(define sk-appendix-name	"&Appendix;")
(define sk-article-name	"&Article;")
(define sk-bibliography-name	"&Bibliography;")
(define sk-book-name		"&Book;")
(define sk-calloutlist-name	"")
(define sk-caution-name	"&Caution;")
(define sk-chapter-name	"&Chapter;")
(define sk-copyright-name	"&Copyright;")
(define sk-dedication-name	"&Dedication;")
(define sk-edition-name	"&Edition;")
(define sk-equation-name	"&Equation;")
(define sk-example-name	"&Example;")
(define sk-figure-name	"&Figure;")
(define sk-glossary-name	"&Glossary;")
(define sk-glosssee-name	"&GlossSee;")
(define sk-glossseealso-name	"&GlossSeeAlso;")
(define sk-important-name	"&Important;")
(define sk-index-name		"&Index;")
(define sk-colophon-name	"&Colophon;")
(define sk-setindex-name	"&SetIndex;")
(define sk-isbn-name		"&ISBN;")
(define sk-legalnotice-name	"&LegalNotice;")
(define sk-msgaud-name	"&MsgAud;")
(define sk-msglevel-name	"&MsgLevel;")
(define sk-msgorig-name	"&MsgOrig;")
(define sk-note-name		"&Note;")
(define sk-part-name		"&Part;")
(define sk-preface-name	"&Preface;")
(define sk-procedure-name	"&Procedure;")
(define sk-pubdate-name	"&Published;")
(define sk-question-name	"&Question;")
(define sk-refentry-name      "&RefEntry;")
(define sk-reference-name	"&Reference;")
(define sk-refname-name	"&RefName;")
(define sk-revhistory-name	"&RevHistory;")
(define sk-refsect1-name      "&RefSection;")
(define sk-refsect2-name      "&RefSection;")
(define sk-refsect3-name      "&RefSection;")
(define sk-refsynopsisdiv-name      "&RefSynopsisDiv;")
(define sk-revision-name	"&Revision;")
(define sk-sect1-name		"&Section;")
(define sk-sect2-name		"&Section;")
(define sk-sect3-name		"&Section;")
(define sk-sect4-name		"&Section;")
(define sk-sect5-name		"&Section;")
(define sk-section-name		"&Section;")
(define sk-simplesect-name	"&Section;")
(define sk-seeie-name		"&See;")
(define sk-seealsoie-name	"&Seealso;")
(define sk-set-name		"&Set;")
(define sk-sidebar-name	"&Sidebar;")
(define sk-step-name		"&step;")
(define sk-table-name		"&Table;")
(define sk-tip-name		"&Tip;")
(define sk-toc-name		"&TableofContents;")
(define sk-warning-name	"&Warning;")

(define (gentext-sk-element-name gind)
  (let* ((giname (if (string? gind) gind (gi gind)))
	 (name   (normalize giname)))
    (cond
     ((equal? name (normalize "abstract"))	sk-abstract-name)
     ((equal? name (normalize "answer"))	sk-answer-name)
     ((equal? name (normalize "appendix"))	sk-appendix-name)
     ((equal? name (normalize "article"))	sk-article-name)
     ((equal? name (normalize "bibliography"))	sk-bibliography-name)
     ((equal? name (normalize "book"))		sk-book-name)
     ((equal? name (normalize "calloutlist"))	sk-calloutlist-name)
     ((equal? name (normalize "caution"))	sk-caution-name)
     ((equal? name (normalize "chapter"))	sk-chapter-name)
     ((equal? name (normalize "copyright"))	sk-copyright-name)
     ((equal? name (normalize "dedication"))	sk-dedication-name)
     ((equal? name (normalize "edition"))	sk-edition-name)
     ((equal? name (normalize "equation"))	sk-equation-name)
     ((equal? name (normalize "example"))	sk-example-name)
     ((equal? name (normalize "figure"))	sk-figure-name)
     ((equal? name (normalize "glossary"))	sk-glossary-name)
     ((equal? name (normalize "glosssee"))	sk-glosssee-name)
     ((equal? name (normalize "glossseealso"))	sk-glossseealso-name)
     ((equal? name (normalize "important"))	sk-important-name)
     ((equal? name (normalize "index"))		sk-index-name)
     ((equal? name (normalize "colophon"))	sk-colophon-name)
     ((equal? name (normalize "setindex"))	sk-setindex-name)
     ((equal? name (normalize "isbn"))		sk-isbn-name)
     ((equal? name (normalize "legalnotice"))	sk-legalnotice-name)
     ((equal? name (normalize "msgaud"))	sk-msgaud-name)
     ((equal? name (normalize "msglevel"))	sk-msglevel-name)
     ((equal? name (normalize "msgorig"))	sk-msgorig-name)
     ((equal? name (normalize "note"))		sk-note-name)
     ((equal? name (normalize "part"))		sk-part-name)
     ((equal? name (normalize "preface"))	sk-preface-name)
     ((equal? name (normalize "procedure"))	sk-procedure-name)
     ((equal? name (normalize "pubdate"))	sk-pubdate-name)
     ((equal? name (normalize "question"))	sk-question-name)
     ((equal? name (normalize "refentry"))	sk-refentry-name)
     ((equal? name (normalize "reference"))	sk-reference-name)
     ((equal? name (normalize "refname"))	sk-refname-name)
     ((equal? name (normalize "revhistory"))	sk-revhistory-name)
     ((equal? name (normalize "refsect1"))	sk-refsect1-name)
     ((equal? name (normalize "refsect2"))	sk-refsect2-name)
     ((equal? name (normalize "refsect3"))	sk-refsect3-name)
     ((equal? name (normalize "refsynopsisdiv"))	sk-refsynopsisdiv-name)
     ((equal? name (normalize "revision"))	sk-revision-name)
     ((equal? name (normalize "sect1"))		sk-sect1-name)
     ((equal? name (normalize "sect2"))		sk-sect2-name)
     ((equal? name (normalize "sect3"))		sk-sect3-name)
     ((equal? name (normalize "sect4"))		sk-sect4-name)
     ((equal? name (normalize "sect5"))		sk-sect5-name)
     ((equal? name (normalize "section"))	sk-section-name)
     ((equal? name (normalize "simplesect"))    sk-simplesect-name)
     ((equal? name (normalize "seeie"))		sk-seeie-name)
     ((equal? name (normalize "seealsoie"))	sk-seealsoie-name)
     ((equal? name (normalize "set"))		sk-set-name)
     ((equal? name (normalize "sidebar"))	sk-sidebar-name)
     ((equal? name (normalize "step"))		sk-step-name)
     ((equal? name (normalize "table"))		sk-table-name)
     ((equal? name (normalize "tip"))		sk-tip-name)
     ((equal? name (normalize "toc"))		sk-toc-name)
     ((equal? name (normalize "warning"))	sk-warning-name)
     (else (let* ((msg (string-append "gentext-sk-element-name: &unexpectedelementname;: " name))
		  (err (node-list-error msg (current-node))))
	     msg)))))

;; gentext-element-name-space returns gentext-element-name with a 
;; trailing space, if gentext-element-name isn't "".
;;
(define (gentext-sk-element-name-space giname)
  (string-with-space (gentext-element-name giname)))

;; gentext-intra-label-sep returns the seperator to be inserted
;; between multiple occurances of a label (or parts of a label)
;; for the specified element.  Most of these are for enumerated
;; labels like "Figure 2-4", but this function is used elsewhere
;; (e.g. REFNAME) with a little abuse.
;;

(define sk-equation-intra-label-sep "-")
(define sk-informalequation-intra-label-sep "-")
(define sk-example-intra-label-sep "-")
(define sk-figure-intra-label-sep "-")
(define sk-listitem-intra-label-sep ".")
(define sk-procedure-intra-label-sep ".")
(define sk-refentry-intra-label-sep ".")
(define sk-reference-intra-label-sep ".")
(define sk-refname-intra-label-sep ", ")
(define sk-refsect1-intra-label-sep ".")
(define sk-refsect2-intra-label-sep ".")
(define sk-refsect3-intra-label-sep ".")
(define sk-sect1-intra-label-sep ".")
(define sk-sect2-intra-label-sep ".")
(define sk-sect3-intra-label-sep ".")
(define sk-sect4-intra-label-sep ".")
(define sk-sect5-intra-label-sep ".")
(define sk-section-intra-label-sep ".")
(define sk-simplesect-intra-label-sep ".")
(define sk-step-intra-label-sep ".")
(define sk-table-intra-label-sep "-")
(define sk-_pagenumber-intra-label-sep "-")
(define sk-default-intra-label-sep "")

(define (gentext-sk-intra-label-sep gind)
  (let* ((giname (if (string? gind) gind (gi gind)))
	 (name   (normalize giname)))
    (cond
     ((equal? name (normalize "equation"))	sk-equation-intra-label-sep)
     ((equal? name (normalize "informalequation"))	sk-informalequation-intra-label-sep)
     ((equal? name (normalize "example"))	sk-example-intra-label-sep)
     ((equal? name (normalize "figure"))	sk-figure-intra-label-sep)
     ((equal? name (normalize "listitem"))	sk-listitem-intra-label-sep)
     ((equal? name (normalize "procedure"))	sk-procedure-intra-label-sep)
     ((equal? name (normalize "refentry"))	sk-refentry-intra-label-sep)
     ((equal? name (normalize "reference"))	sk-reference-intra-label-sep)
     ((equal? name (normalize "refname"))	sk-refname-intra-label-sep)
     ((equal? name (normalize "refsect1"))	sk-refsect1-intra-label-sep)
     ((equal? name (normalize "refsect2"))	sk-refsect2-intra-label-sep)
     ((equal? name (normalize "refsect3"))	sk-refsect3-intra-label-sep)
     ((equal? name (normalize "sect1"))		sk-sect1-intra-label-sep)
     ((equal? name (normalize "sect2"))		sk-sect2-intra-label-sep)
     ((equal? name (normalize "sect3"))		sk-sect3-intra-label-sep)
     ((equal? name (normalize "sect4"))		sk-sect4-intra-label-sep)
     ((equal? name (normalize "sect5"))		sk-sect5-intra-label-sep)
     ((equal? name (normalize "section"))	sk-section-intra-label-sep)
     ((equal? name (normalize "simplesect"))	sk-simplesect-intra-label-sep)
     ((equal? name (normalize "step"))		sk-step-intra-label-sep)
     ((equal? name (normalize "table"))		sk-table-intra-label-sep)
     ((equal? name (normalize "_pagenumber"))	sk-_pagenumber-intra-label-sep)
     (else sk-default-intra-label-sep))))

;; gentext-label-title-sep returns the seperator to be inserted
;; between a label and the text following the label for the
;; specified element.  Most of these are for use between
;; enumerated labels and titles like "1. Chapter One Title", but
;; this function is used elsewhere (e.g. NOTE) with a little
;; abuse.
;;

(define sk-abstract-label-title-sep ": ")
(define sk-answer-label-title-sep " ")
(define sk-appendix-label-title-sep ". ")
(define sk-caution-label-title-sep "")
(define sk-chapter-label-title-sep ". ")
(define sk-equation-label-title-sep ". ")
(define sk-example-label-title-sep ". ")
(define sk-figure-label-title-sep ". ")
(define sk-footnote-label-title-sep ". ")
(define sk-glosssee-label-title-sep ": ")
(define sk-glossseealso-label-title-sep ": ")
(define sk-important-label-title-sep ": ")
(define sk-note-label-title-sep ": ")
(define sk-orderedlist-label-title-sep ". ")
(define sk-part-label-title-sep ". ")
(define sk-procedure-label-title-sep ". ")
(define sk-prefix-label-title-sep ". ")
(define sk-question-label-title-sep " ")
(define sk-refentry-label-title-sep "")
(define sk-reference-label-title-sep ". ")
(define sk-refsect1-label-title-sep ". ")
(define sk-refsect2-label-title-sep ". ")
(define sk-refsect3-label-title-sep ". ")
(define sk-sect1-label-title-sep ". ")
(define sk-sect2-label-title-sep ". ")
(define sk-sect3-label-title-sep ". ")
(define sk-sect4-label-title-sep ". ")
(define sk-sect5-label-title-sep ". ")
(define sk-section-label-title-sep ". ")
(define sk-simplesect-label-title-sep ". ")
(define sk-seeie-label-title-sep " ")
(define sk-seealsoie-label-title-sep " ")
(define sk-step-label-title-sep ". ")
(define sk-table-label-title-sep ". ")
(define sk-tip-label-title-sep ": ")
(define sk-warning-label-title-sep "")
(define sk-default-label-title-sep "")

(define (gentext-sk-label-title-sep gind)
  (let* ((giname (if (string? gind) gind (gi gind)))
	 (name   (normalize giname)))
    (cond
     ((equal? name (normalize "abstract")) sk-abstract-label-title-sep)
     ((equal? name (normalize "answer")) sk-answer-label-title-sep)
     ((equal? name (normalize "appendix")) sk-appendix-label-title-sep)
     ((equal? name (normalize "caution")) sk-caution-label-title-sep)
     ((equal? name (normalize "chapter")) sk-chapter-label-title-sep)
     ((equal? name (normalize "equation")) sk-equation-label-title-sep)
     ((equal? name (normalize "example")) sk-example-label-title-sep)
     ((equal? name (normalize "figure")) sk-figure-label-title-sep)
     ((equal? name (normalize "footnote")) sk-footnote-label-title-sep)
     ((equal? name (normalize "glosssee")) sk-glosssee-label-title-sep)
     ((equal? name (normalize "glossseealso")) sk-glossseealso-label-title-sep)
     ((equal? name (normalize "important")) sk-important-label-title-sep)
     ((equal? name (normalize "note")) sk-note-label-title-sep)
     ((equal? name (normalize "orderedlist")) sk-orderedlist-label-title-sep)
     ((equal? name (normalize "part")) sk-part-label-title-sep)
     ((equal? name (normalize "procedure")) sk-procedure-label-title-sep)
     ((equal? name (normalize "prefix")) sk-prefix-label-title-sep)
     ((equal? name (normalize "question")) sk-question-label-title-sep)
     ((equal? name (normalize "refentry")) sk-refentry-label-title-sep)
     ((equal? name (normalize "reference")) sk-reference-label-title-sep)
     ((equal? name (normalize "refsect1")) sk-refsect1-label-title-sep)
     ((equal? name (normalize "refsect2")) sk-refsect2-label-title-sep)
     ((equal? name (normalize "refsect3")) sk-refsect3-label-title-sep)
     ((equal? name (normalize "sect1")) sk-sect1-label-title-sep)
     ((equal? name (normalize "sect2")) sk-sect2-label-title-sep)
     ((equal? name (normalize "sect3")) sk-sect3-label-title-sep)
     ((equal? name (normalize "sect4")) sk-sect4-label-title-sep)
     ((equal? name (normalize "sect5")) sk-sect5-label-title-sep)
     ((equal? name (normalize "section")) sk-section-label-title-sep)
     ((equal? name (normalize "simplesect")) sk-simplesect-label-title-sep)
     ((equal? name (normalize "seeie")) sk-seeie-label-title-sep)
     ((equal? name (normalize "seealsoie")) sk-seealsoie-label-title-sep)
     ((equal? name (normalize "step")) sk-step-label-title-sep)
     ((equal? name (normalize "table")) sk-table-label-title-sep)
     ((equal? name (normalize "tip")) sk-tip-label-title-sep)
     ((equal? name (normalize "warning")) sk-warning-label-title-sep)
     (else sk-default-label-title-sep))))

(define (sk-set-label-number-format gind) "1")
(define (sk-book-label-number-format gind) "1")
(define (sk-prefix-label-number-format gind) "1")
(define (sk-part-label-number-format gind) "I")
(define (sk-chapter-label-number-format gind) "1")
(define (sk-appendix-label-number-format gind) "A")
(define (sk-reference-label-number-format gind) "I")
(define (sk-example-label-number-format gind) "1")
(define (sk-figure-label-number-format gind) "1")
(define (sk-table-label-number-format gind) "1")
(define (sk-procedure-label-number-format gind) "1")
(define (sk-step-label-number-format gind) "1")
(define (sk-refsect1-label-number-format gind) "1")
(define (sk-refsect2-label-number-format gind) "1")
(define (sk-refsect3-label-number-format gind) "1")
(define (sk-sect1-label-number-format gind) "1")
(define (sk-sect2-label-number-format gind) "1")
(define (sk-sect3-label-number-format gind) "1")
(define (sk-sect4-label-number-format gind) "1")
(define (sk-sect5-label-number-format gind) "1")
(define (sk-section-label-number-format gind) "1")
(define (sk-default-label-number-format gind) "1")

(define (sk-label-number-format gind)
  (let* ((giname (if (string? gind) gind (gi gind)))
	 (name   (normalize giname)))
    (cond
     ((equal? name (normalize "set")) (sk-set-label-number-format gind))
     ((equal? name (normalize "book")) (sk-book-label-number-format gind))
     ((equal? name (normalize "prefix")) (sk-prefix-label-number-format gind))
     ((equal? name (normalize "part")) (sk-part-label-number-format gind))
     ((equal? name (normalize "chapter")) (sk-chapter-label-number-format gind))
     ((equal? name (normalize "appendix")) (sk-appendix-label-number-format gind))
     ((equal? name (normalize "reference")) (sk-reference-label-number-format gind))
     ((equal? name (normalize "example")) (sk-example-label-number-format gind))
     ((equal? name (normalize "figure")) (sk-figure-label-number-format gind))
     ((equal? name (normalize "table")) (sk-table-label-number-format gind))
     ((equal? name (normalize "procedure")) (sk-procedure-label-number-format gind))
     ((equal? name (normalize "step")) (sk-step-label-number-format gind))
     ((equal? name (normalize "refsect1")) (sk-refsect1-label-number-format gind))
     ((equal? name (normalize "refsect2")) (sk-refsect2-label-number-format gind))
     ((equal? name (normalize "refsect3")) (sk-refsect3-label-number-format gind))
     ((equal? name (normalize "sect1")) (sk-sect1-label-number-format gind))
     ((equal? name (normalize "sect2")) (sk-sect2-label-number-format gind))
     ((equal? name (normalize "sect3")) (sk-sect3-label-number-format gind))
     ((equal? name (normalize "sect4")) (sk-sect4-label-number-format gind))
     ((equal? name (normalize "sect5")) (sk-sect5-label-number-format gind))
     ((equal? name (normalize "section")) (sk-section-label-number-format gind))
     (else (sk-default-label-number-format gind)))))

(define ($lot-title-sk$ gind)
  (let* ((giname (if (string? gind) gind (gi gind)))
	 (name   (normalize giname)))
    (cond ((equal? name (normalize "table"))    "&ListofTables;")
	  ((equal? name (normalize "example"))  "&ListofExamples;")
	  ((equal? name (normalize "figure"))   "&ListofFigures;")
	  ((equal? name (normalize "equation")) "&ListofEquations;")
	  (else (let* ((msg (string-append "&ListofUnknown;: " name))
		       (err (node-list-error msg (current-node))))
		  msg)))))

(define %gentext-sk-start-quote% "\U-201E;")

(define %gentext-sk-end-quote% "\U-201C;")

(define %gentext-sk-start-nested-quote% "\U-201A;")

(define %gentext-sk-end-nested-quote% "\U-2018;")

(define %gentext-sk-by% "&by;") ;; e.g. Copyright 1997 "by" A. Nonymous
                           ;; Authored "by" Jane Doe

(define %gentext-sk-edited-by% "&Editedby;")
                           ;; "Edited by" Jane Doe

(define %gentext-sk-page% "")

(define %gentext-sk-and% "&and;")

(define %gentext-sk-bibl-pages% "&Pgs;")

(define %gentext-sk-endnotes% "&Notes;")

(define %gentext-sk-table-endnotes% "&TableNotes;:")

(define %gentext-sk-index-see% "&See;")

(define %gentext-sk-index-seealso% "&SeeAlso;")

