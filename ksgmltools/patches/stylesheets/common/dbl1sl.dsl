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

(define (sl-appendix-xref-string gi-or-name)
  (if %chapter-autolabel%
      "&Appendix; %n"
      "&appendix; imenovan %t"))
;; ^^^^^^^^^^^^^^^^^
;; Possible troubles!


(define (sl-article-xref-string gi-or-name)
  (string-append %gentext-sl-start-quote%
		 "%t"
		 %gentext-sl-end-quote%))

(define (sl-bibliography-xref-string gi-or-name)
  "%t")

(define (sl-book-xref-string gi-or-name)
  "%t")

(define (sl-chapter-xref-string gi-or-name)
  (if %chapter-autolabel%
      "&Chapter; %n"
      "&chapter;, imenovan %t"))

(define (sl-equation-xref-string gi-or-name)
  "&Equation; %n")

(define (sl-example-xref-string gi-or-name)
  "&Example; %n")

(define (sl-figure-xref-string gi-or-name)
  "&Figure; %n")

(define (sl-glossary-xref-string gi-or-name)
  "%t")

(define (sl-index-xref-string gi-or-name)
  "%t")

(define (sl-listitem-xref-string gi-or-name)
  "%n")

(define (sl-part-xref-string gi-or-name)
  "&Part; %n")

(define (sl-preface-xref-string gi-or-name)
  "%t")

(define (sl-procedure-xref-string gi-or-name)
  "&Procedure; %n, %t")

(define (sl-reference-xref-string gi-or-name)
  "&Reference; %n, %t")

(define (sl-sectioning-xref-string gi-or-name)
  (if %section-autolabel% 
      "&Section; %n" 
      "&section;, imenovan %t"))

(define (sl-sect1-xref-string gi-or-name)
  (sl-sectioning-xref-string gi-or-name))

(define (sl-sect2-xref-string gi-or-name)
  (sl-sectioning-xref-string gi-or-name))

(define (sl-sect3-xref-string gi-or-name)
  (sl-sectioning-xref-string gi-or-name))

(define (sl-sect4-xref-string gi-or-name)
  (sl-sectioning-xref-string gi-or-name))

(define (sl-sect5-xref-string gi-or-name)
  (sl-sectioning-xref-string gi-or-name))

(define (sl-section-xref-string gi-or-name)
  (sl-sectioning-xref-string gi-or-name))

(define (sl-sidebar-xref-string gi-or-name)
  "&sidebar; %t")

(define (sl-step-xref-string gi-or-name)
  "&step; %n")

(define (sl-table-xref-string gi-or-name)
  "&Table; %n")

(define (sl-default-xref-string gi-or-name)
  (let* ((giname (if (string? gi-or-name) gi-or-name (gi gi-or-name)))
	 (msg    (string-append "[&xrefto; "
				(if giname giname "&nonexistantelement;")
				" &unsupported;]"))
	 (err    (node-list-error msg (current-node))))
    msg))

(define (gentext-sl-xref-strings gind)
  (let* ((giname (if (string? gind) gind (gi gind)))
	 (name   (normalize giname)))
    (cond
      ((equal? name (normalize "appendix")) (sl-appendix-xref-string gind))
      ((equal? name (normalize "article"))  (sl-article-xref-string gind))
      ((equal? name (normalize "bibliography")) (sl-bibliography-xref-string gind))
      ((equal? name (normalize "book"))     (sl-book-xref-string gind))
      ((equal? name (normalize "chapter"))  (sl-chapter-xref-string gind))
      ((equal? name (normalize "equation")) (sl-equation-xref-string gind))
      ((equal? name (normalize "example"))  (sl-example-xref-string gind))
      ((equal? name (normalize "figure"))   (sl-figure-xref-string gind))
      ((equal? name (normalize "glossary")) (sl-glossary-xref-string gind))
      ((equal? name (normalize "index"))    (sl-index-xref-string gind))
      ((equal? name (normalize "listitem")) (sl-listitem-xref-string gind))
      ((equal? name (normalize "part"))     (sl-part-xref-string gind))
      ((equal? name (normalize "preface"))  (sl-preface-xref-string gind))
      ((equal? name (normalize "procedure")) (sl-procedure-xref-string gind))
      ((equal? name (normalize "reference")) (sl-reference-xref-string gind))
      ((equal? name (normalize "sect1"))    (sl-sect1-xref-string gind))
      ((equal? name (normalize "sect2"))    (sl-sect2-xref-string gind))
      ((equal? name (normalize "sect3"))    (sl-sect3-xref-string gind))
      ((equal? name (normalize "sect4"))    (sl-sect4-xref-string gind))
      ((equal? name (normalize "sect5"))    (sl-sect5-xref-string gind))
      ((equal? name (normalize "section"))  (sl-section-xref-string gind))
      ((equal? name (normalize "simplesect"))  (sl-section-xref-string gind))
      ((equal? name (normalize "sidebar"))  (sl-sidebar-xref-string gind))
      ((equal? name (normalize "step"))     (sl-step-xref-string gind))
      ((equal? name (normalize "table"))    (sl-table-xref-string gind))
      (else (sl-default-xref-string gind)))))

(define (sl-auto-xref-indirect-connector before) 
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
(define %generate-sl-toc-in-front% #t)

;; gentext-element-name returns the generated text that should be 
;; used to make reference to the selected element.
;;
(define sl-abstract-name	"&Abstract;")
(define sl-answer-name		"&Answer;")
(define sl-appendix-name	"&Appendix;")
(define sl-article-name	"&Article;")
(define sl-bibliography-name	"&Bibliography;")
(define sl-book-name		"&Book;")
(define sl-calloutlist-name	"")
(define sl-caution-name	"&Caution;")
(define sl-chapter-name	"&Chapter;")
(define sl-copyright-name	"&Copyright;")
(define sl-dedication-name	"&Dedication;")
(define sl-edition-name	"&Edition;")
(define sl-equation-name	"&Equation;")
(define sl-example-name	"&Example;")
(define sl-figure-name	"&Figure;")
(define sl-glossary-name	"&Glossary;")
(define sl-glosssee-name	"&GlossSee;")
(define sl-glossseealso-name	"&GlossSeeAlso;")
(define sl-important-name	"&Important;")
(define sl-index-name		"&Index;")
(define sl-colophon-name	"&Colophon;")
(define sl-setindex-name	"&SetIndex;")
(define sl-isbn-name		"&ISBN;")
(define sl-legalnotice-name	"&LegalNotice;")
(define sl-msgaud-name	"&MsgAud;")
(define sl-msglevel-name	"&MsgLevel;")
(define sl-msgorig-name	"&MsgOrig;")
(define sl-note-name		"&Note;")
(define sl-part-name		"&Part;")
(define sl-preface-name	"&Preface;")
(define sl-procedure-name	"&Procedure;")
(define sl-pubdate-name	"&Published;")
(define sl-question-name	"&Question;")
(define sl-refentry-name      "&RefEntry;")
(define sl-reference-name	"&Reference;")
(define sl-refname-name	"&RefName;")
(define sl-revhistory-name	"&RevHistory;")
(define sl-refsect1-name      "&RefSection;")
(define sl-refsect2-name      "&RefSection;")
(define sl-refsect3-name      "&RefSection;")
(define sl-refsynopsisdiv-name      "&RefSynopsisDiv;")
(define sl-revision-name	"&Revision;")
(define sl-sect1-name		"&Section;")
(define sl-sect2-name		"&Section;")
(define sl-sect3-name		"&Section;")
(define sl-sect4-name		"&Section;")
(define sl-sect5-name		"&Section;")
(define sl-section-name		"&Section;")
(define sl-simplesect-name	"&Section;")
(define sl-seeie-name		"&See;")
(define sl-seealsoie-name	"&Seealso;")
(define sl-set-name		"&Set;")
(define sl-sidebar-name	"&Sidebar;")
(define sl-step-name		"&step;")
(define sl-table-name		"&Table;")
(define sl-tip-name		"&Tip;")
(define sl-toc-name		"&TableofContents;")
(define sl-warning-name	"&Warning;")

(define (gentext-sl-element-name gind)
  (let* ((giname (if (string? gind) gind (gi gind)))
	 (name   (normalize giname)))
    (cond
     ((equal? name (normalize "abstract"))	sl-abstract-name)
     ((equal? name (normalize "answer"))	sl-answer-name)
     ((equal? name (normalize "appendix"))	sl-appendix-name)
     ((equal? name (normalize "article"))	sl-article-name)
     ((equal? name (normalize "bibliography"))	sl-bibliography-name)
     ((equal? name (normalize "book"))		sl-book-name)
     ((equal? name (normalize "calloutlist"))	sl-calloutlist-name)
     ((equal? name (normalize "caution"))	sl-caution-name)
     ((equal? name (normalize "chapter"))	sl-chapter-name)
     ((equal? name (normalize "copyright"))	sl-copyright-name)
     ((equal? name (normalize "dedication"))	sl-dedication-name)
     ((equal? name (normalize "edition"))	sl-edition-name)
     ((equal? name (normalize "equation"))	sl-equation-name)
     ((equal? name (normalize "example"))	sl-example-name)
     ((equal? name (normalize "figure"))	sl-figure-name)
     ((equal? name (normalize "glossary"))	sl-glossary-name)
     ((equal? name (normalize "glosssee"))	sl-glosssee-name)
     ((equal? name (normalize "glossseealso"))	sl-glossseealso-name)
     ((equal? name (normalize "important"))	sl-important-name)
     ((equal? name (normalize "index"))		sl-index-name)
     ((equal? name (normalize "colophon"))	sl-colophon-name)
     ((equal? name (normalize "setindex"))	sl-setindex-name)
     ((equal? name (normalize "isbn"))		sl-isbn-name)
     ((equal? name (normalize "legalnotice"))	sl-legalnotice-name)
     ((equal? name (normalize "msgaud"))	sl-msgaud-name)
     ((equal? name (normalize "msglevel"))	sl-msglevel-name)
     ((equal? name (normalize "msgorig"))	sl-msgorig-name)
     ((equal? name (normalize "note"))		sl-note-name)
     ((equal? name (normalize "part"))		sl-part-name)
     ((equal? name (normalize "preface"))	sl-preface-name)
     ((equal? name (normalize "procedure"))	sl-procedure-name)
     ((equal? name (normalize "pubdate"))	sl-pubdate-name)
     ((equal? name (normalize "question"))	sl-question-name)
     ((equal? name (normalize "refentry"))	sl-refentry-name)
     ((equal? name (normalize "reference"))	sl-reference-name)
     ((equal? name (normalize "refname"))	sl-refname-name)
     ((equal? name (normalize "revhistory"))	sl-revhistory-name)
     ((equal? name (normalize "refsect1"))	sl-refsect1-name)
     ((equal? name (normalize "refsect2"))	sl-refsect2-name)
     ((equal? name (normalize "refsect3"))	sl-refsect3-name)
     ((equal? name (normalize "refsynopsisdiv"))	sl-refsynopsisdiv-name)
     ((equal? name (normalize "revision"))	sl-revision-name)
     ((equal? name (normalize "sect1"))		sl-sect1-name)
     ((equal? name (normalize "sect2"))		sl-sect2-name)
     ((equal? name (normalize "sect3"))		sl-sect3-name)
     ((equal? name (normalize "sect4"))		sl-sect4-name)
     ((equal? name (normalize "sect5"))		sl-sect5-name)
     ((equal? name (normalize "section"))	sl-section-name)
     ((equal? name (normalize "simplesect"))    sl-simplesect-name)
     ((equal? name (normalize "seeie"))		sl-seeie-name)
     ((equal? name (normalize "seealsoie"))	sl-seealsoie-name)
     ((equal? name (normalize "set"))		sl-set-name)
     ((equal? name (normalize "sidebar"))	sl-sidebar-name)
     ((equal? name (normalize "step"))		sl-step-name)
     ((equal? name (normalize "table"))		sl-table-name)
     ((equal? name (normalize "tip"))		sl-tip-name)
     ((equal? name (normalize "toc"))		sl-toc-name)
     ((equal? name (normalize "warning"))	sl-warning-name)
     (else (let* ((msg (string-append "gentext-sl-element-name: &unexpectedelementname;: " name))
		  (err (node-list-error msg (current-node))))
	     msg)))))

;; gentext-element-name-space returns gentext-element-name with a 
;; trailing space, if gentext-element-name isn't "".
;;
(define (gentext-sl-element-name-space giname)
  (string-with-space (gentext-element-name giname)))

;; gentext-intra-label-sep returns the seperator to be inserted
;; between multiple occurances of a label (or parts of a label)
;; for the specified element.  Most of these are for enumerated
;; labels like "Figure 2-4", but this function is used elsewhere
;; (e.g. REFNAME) with a little abuse.
;;

(define sl-equation-intra-label-sep ".")
(define sl-informalequation-intra-label-sep ".")
(define sl-example-intra-label-sep ".")
(define sl-figure-intra-label-sep ".")
(define sl-listitem-intra-label-sep ".")
(define sl-procedure-intra-label-sep ".")
(define sl-refentry-intra-label-sep ".")
(define sl-reference-intra-label-sep ".")
(define sl-refname-intra-label-sep ", ")
(define sl-refsect1-intra-label-sep ".")
(define sl-refsect2-intra-label-sep ".")
(define sl-refsect3-intra-label-sep ".")
(define sl-sect1-intra-label-sep ".")
(define sl-sect2-intra-label-sep ".")
(define sl-sect3-intra-label-sep ".")
(define sl-sect4-intra-label-sep ".")
(define sl-sect5-intra-label-sep ".")
(define sl-section-intra-label-sep ".")
(define sl-simplesect-intra-label-sep ".")
(define sl-step-intra-label-sep ".")
(define sl-table-intra-label-sep ".")
(define sl-_pagenumber-intra-label-sep ".")
(define sl-default-intra-label-sep "")

(define (gentext-sl-intra-label-sep gind)
  (let* ((giname (if (string? gind) gind (gi gind)))
	 (name   (normalize giname)))
    (cond
     ((equal? name (normalize "equation"))	sl-equation-intra-label-sep)
     ((equal? name (normalize "informalequation"))	sl-informalequation-intra-label-sep)
     ((equal? name (normalize "example"))	sl-example-intra-label-sep)
     ((equal? name (normalize "figure"))	sl-figure-intra-label-sep)
     ((equal? name (normalize "listitem"))	sl-listitem-intra-label-sep)
     ((equal? name (normalize "procedure"))	sl-procedure-intra-label-sep)
     ((equal? name (normalize "refentry"))	sl-refentry-intra-label-sep)
     ((equal? name (normalize "reference"))	sl-reference-intra-label-sep)
     ((equal? name (normalize "refname"))	sl-refname-intra-label-sep)
     ((equal? name (normalize "refsect1"))	sl-refsect1-intra-label-sep)
     ((equal? name (normalize "refsect2"))	sl-refsect2-intra-label-sep)
     ((equal? name (normalize "refsect3"))	sl-refsect3-intra-label-sep)
     ((equal? name (normalize "sect1"))		sl-sect1-intra-label-sep)
     ((equal? name (normalize "sect2"))		sl-sect2-intra-label-sep)
     ((equal? name (normalize "sect3"))		sl-sect3-intra-label-sep)
     ((equal? name (normalize "sect4"))		sl-sect4-intra-label-sep)
     ((equal? name (normalize "sect5"))		sl-sect5-intra-label-sep)
     ((equal? name (normalize "section"))	sl-section-intra-label-sep)
     ((equal? name (normalize "simplesect"))	sl-simplesect-intra-label-sep)
     ((equal? name (normalize "step"))		sl-step-intra-label-sep)
     ((equal? name (normalize "table"))		sl-table-intra-label-sep)
     ((equal? name (normalize "_pagenumber"))	sl-_pagenumber-intra-label-sep)
     (else sl-default-intra-label-sep))))

;; gentext-label-title-sep returns the seperator to be inserted
;; between a label and the text following the label for the
;; specified element.  Most of these are for use between
;; enumerated labels and titles like "1. Chapter One Title", but
;; this function is used elsewhere (e.g. NOTE) with a little
;; abuse.
;;

(define sl-abstract-label-title-sep ": ")
(define sl-answer-label-title-sep " ")
(define sl-appendix-label-title-sep ". ")
(define sl-caution-label-title-sep "")
(define sl-chapter-label-title-sep ". ")
(define sl-equation-label-title-sep ". ")
(define sl-example-label-title-sep ". ")
(define sl-figure-label-title-sep ". ")
(define sl-footnote-label-title-sep ". ")
(define sl-glosssee-label-title-sep ": ")
(define sl-glossseealso-label-title-sep ": ")
(define sl-important-label-title-sep ": ")
(define sl-note-label-title-sep ": ")
(define sl-orderedlist-label-title-sep ". ")
(define sl-part-label-title-sep ". ")
(define sl-procedure-label-title-sep ". ")
(define sl-prefix-label-title-sep ". ")
(define sl-question-label-title-sep " ")
(define sl-refentry-label-title-sep "")
(define sl-reference-label-title-sep ". ")
(define sl-refsect1-label-title-sep ". ")
(define sl-refsect2-label-title-sep ". ")
(define sl-refsect3-label-title-sep ". ")
(define sl-sect1-label-title-sep ". ")
(define sl-sect2-label-title-sep ". ")
(define sl-sect3-label-title-sep ". ")
(define sl-sect4-label-title-sep ". ")
(define sl-sect5-label-title-sep ". ")
(define sl-section-label-title-sep ". ")
(define sl-simplesect-label-title-sep ". ")
(define sl-seeie-label-title-sep " ")
(define sl-seealsoie-label-title-sep " ")
(define sl-step-label-title-sep ". ")
(define sl-table-label-title-sep ". ")
(define sl-tip-label-title-sep ": ")
(define sl-warning-label-title-sep "")
(define sl-default-label-title-sep "")

(define (gentext-sl-label-title-sep gind)
  (let* ((giname (if (string? gind) gind (gi gind)))
	 (name   (normalize giname)))
    (cond
     ((equal? name (normalize "abstract")) sl-abstract-label-title-sep)
     ((equal? name (normalize "answer")) sl-answer-label-title-sep)
     ((equal? name (normalize "appendix")) sl-appendix-label-title-sep)
     ((equal? name (normalize "caution")) sl-caution-label-title-sep)
     ((equal? name (normalize "chapter")) sl-chapter-label-title-sep)
     ((equal? name (normalize "equation")) sl-equation-label-title-sep)
     ((equal? name (normalize "example")) sl-example-label-title-sep)
     ((equal? name (normalize "figure")) sl-figure-label-title-sep)
     ((equal? name (normalize "footnote")) sl-footnote-label-title-sep)
     ((equal? name (normalize "glosssee")) sl-glosssee-label-title-sep)
     ((equal? name (normalize "glossseealso")) sl-glossseealso-label-title-sep)
     ((equal? name (normalize "important")) sl-important-label-title-sep)
     ((equal? name (normalize "note")) sl-note-label-title-sep)
     ((equal? name (normalize "orderedlist")) sl-orderedlist-label-title-sep)
     ((equal? name (normalize "part")) sl-part-label-title-sep)
     ((equal? name (normalize "procedure")) sl-procedure-label-title-sep)
     ((equal? name (normalize "prefix")) sl-prefix-label-title-sep)
     ((equal? name (normalize "question")) sl-question-label-title-sep)
     ((equal? name (normalize "refentry")) sl-refentry-label-title-sep)
     ((equal? name (normalize "reference")) sl-reference-label-title-sep)
     ((equal? name (normalize "refsect1")) sl-refsect1-label-title-sep)
     ((equal? name (normalize "refsect2")) sl-refsect2-label-title-sep)
     ((equal? name (normalize "refsect3")) sl-refsect3-label-title-sep)
     ((equal? name (normalize "sect1")) sl-sect1-label-title-sep)
     ((equal? name (normalize "sect2")) sl-sect2-label-title-sep)
     ((equal? name (normalize "sect3")) sl-sect3-label-title-sep)
     ((equal? name (normalize "sect4")) sl-sect4-label-title-sep)
     ((equal? name (normalize "sect5")) sl-sect5-label-title-sep)
     ((equal? name (normalize "section")) sl-section-label-title-sep)
     ((equal? name (normalize "simplesect")) sl-simplesect-label-title-sep)
     ((equal? name (normalize "seeie")) sl-seeie-label-title-sep)
     ((equal? name (normalize "seealsoie")) sl-seealsoie-label-title-sep)
     ((equal? name (normalize "step")) sl-step-label-title-sep)
     ((equal? name (normalize "table")) sl-table-label-title-sep)
     ((equal? name (normalize "tip")) sl-tip-label-title-sep)
     ((equal? name (normalize "warning")) sl-warning-label-title-sep)
     (else sl-default-label-title-sep))))

(define (sl-set-label-number-format gind) "1")
(define (sl-book-label-number-format gind) "1")
(define (sl-prefix-label-number-format gind) "1")
(define (sl-part-label-number-format gind) "I")
(define (sl-chapter-label-number-format gind) "1")
(define (sl-appendix-label-number-format gind) "A")
(define (sl-reference-label-number-format gind) "I")
(define (sl-example-label-number-format gind) "1")
(define (sl-figure-label-number-format gind) "1")
(define (sl-table-label-number-format gind) "1")
(define (sl-procedure-label-number-format gind) "1")
(define (sl-step-label-number-format gind) "1")
(define (sl-refsect1-label-number-format gind) "1")
(define (sl-refsect2-label-number-format gind) "1")
(define (sl-refsect3-label-number-format gind) "1")
(define (sl-sect1-label-number-format gind) "1")
(define (sl-sect2-label-number-format gind) "1")
(define (sl-sect3-label-number-format gind) "1")
(define (sl-sect4-label-number-format gind) "1")
(define (sl-sect5-label-number-format gind) "1")
(define (sl-section-label-number-format gind) "1")
(define (sl-default-label-number-format gind) "1")

(define (sl-label-number-format gind)
  (let* ((giname (if (string? gind) gind (gi gind)))
	 (name   (normalize giname)))
    (cond
     ((equal? name (normalize "set")) (sl-set-label-number-format gind))
     ((equal? name (normalize "book")) (sl-book-label-number-format gind))
     ((equal? name (normalize "prefix")) (sl-prefix-label-number-format gind))
     ((equal? name (normalize "part")) (sl-part-label-number-format gind))
     ((equal? name (normalize "chapter")) (sl-chapter-label-number-format gind))
     ((equal? name (normalize "appendix")) (sl-appendix-label-number-format gind))
     ((equal? name (normalize "reference")) (sl-reference-label-number-format gind))
     ((equal? name (normalize "example")) (sl-example-label-number-format gind))
     ((equal? name (normalize "figure")) (sl-figure-label-number-format gind))
     ((equal? name (normalize "table")) (sl-table-label-number-format gind))
     ((equal? name (normalize "procedure")) (sl-procedure-label-number-format gind))
     ((equal? name (normalize "step")) (sl-step-label-number-format gind))
     ((equal? name (normalize "refsect1")) (sl-refsect1-label-number-format gind))
     ((equal? name (normalize "refsect2")) (sl-refsect2-label-number-format gind))
     ((equal? name (normalize "refsect3")) (sl-refsect3-label-number-format gind))
     ((equal? name (normalize "sect1")) (sl-sect1-label-number-format gind))
     ((equal? name (normalize "sect2")) (sl-sect2-label-number-format gind))
     ((equal? name (normalize "sect3")) (sl-sect3-label-number-format gind))
     ((equal? name (normalize "sect4")) (sl-sect4-label-number-format gind))
     ((equal? name (normalize "sect5")) (sl-sect5-label-number-format gind))
     ((equal? name (normalize "section")) (sl-section-label-number-format gind))
     (else (sl-default-label-number-format gind)))))

(define ($lot-title-sl$ gind)
  (let* ((giname (if (string? gind) gind (gi gind)))
	 (name   (normalize giname)))
    (cond ((equal? name (normalize "table"))    "&ListofTables;")
	  ((equal? name (normalize "example"))  "&ListofExamples;")
	  ((equal? name (normalize "figure"))   "&ListofFigures;")
	  ((equal? name (normalize "equation")) "&ListofEquations;")
	  (else (let* ((msg (string-append "&ListofUnknown;: " name))
		       (err (node-list-error msg (current-node))))
		  msg)))))

(define %gentext-sl-start-quote% (dingbat "ldquo"))

(define %gentext-sl-end-quote% (dingbat "rdquo"))

(define %gentext-sl-start-nested-quote% (dingbat "lsquo"))

(define %gentext-sl-end-nested-quote% (dingbat "rsquo"))

(define %gentext-sl-by% "&by;") ;; e.g. Copyright 1997 "by" A. Nonymous
                           ;; Authored "by" Jane Doe

(define %gentext-sl-edited-by% "&Editedby;")
                           ;; "Edited by" Jane Doe

(define %gentext-sl-page% "")

(define %gentext-sl-and% "&and;")

(define %gentext-sl-bibl-pages% "&Pgs;")

(define %gentext-sl-endnotes% "&Notes;")

(define %gentext-sl-table-endnotes% "&TableNotes;:")

(define %gentext-sl-index-see% "&See;")

(define %gentext-sl-index-seealso% "&SeeAlso;")

