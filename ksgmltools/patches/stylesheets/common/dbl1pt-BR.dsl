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

(define (pt-BR-appendix-xref-string gi-or-name)
  (if %chapter-autolabel%
      "&Appendix; %n"
      "o &appendix; %t"))

(define (pt-BR-article-xref-string gi-or-name)
  (string-append %gentext-pt-BR-start-quote%
		 "%t"
		 %gentext-pt-BR-end-quote%))

(define (pt-BR-bibliography-xref-string gi-or-name)
  "%t")

(define (pt-BR-book-xref-string gi-or-name)
  "%t")

(define (pt-BR-chapter-xref-string gi-or-name)
  (if %chapter-autolabel%
      "&Chapter; %n"
      "o &chapter; %t"))

(define (pt-BR-equation-xref-string gi-or-name)
  "&Equation; %n")

(define (pt-BR-example-xref-string gi-or-name)
  "&Example; %n")

(define (pt-BR-figure-xref-string gi-or-name)
  "&Figure; %n")

(define (pt-BR-listitem-xref-string gi-or-name)
  "%n")

(define (pt-BR-part-xref-string gi-or-name)
  "&Part; %n")

(define (pt-BR-preface-xref-string gi-or-name)
  "%t")

(define (pt-BR-procedure-xref-string gi-or-name)
  "&Procedure; %n, %t")

(define (pt-BR-section-xref-string gi-or-name)
  (if %section-autolabel% 
      "&Section; %n" 
      "o &section; %t"))

(define (pt-BR-sect1-xref-string gi-or-name)
  (pt-BR-section-xref-string gi-or-name))

(define (pt-BR-sect2-xref-string gi-or-name)
  (pt-BR-section-xref-string gi-or-name))

(define (pt-BR-sect3-xref-string gi-or-name)
  (pt-BR-section-xref-string gi-or-name))

(define (pt-BR-sect4-xref-string gi-or-name)
  (pt-BR-section-xref-string gi-or-name))

(define (pt-BR-sect5-xref-string gi-or-name)
  (pt-BR-section-xref-string gi-or-name))

(define (pt-BR-step-xref-string gi-or-name)
  "&step; %n")

(define (pt-BR-table-xref-string gi-or-name)
  "&Table; %n")

(define (pt-BR-default-xref-string gi-or-name)
  (let* ((giname (if (string? gi-or-name) gi-or-name (gi gi-or-name)))
	 (msg    (string-append "[&xrefto; "
				(if giname giname "&nonexistantelement;")
				" &unsupported;]"))
	 (err    (node-list-error msg (current-node))))
    msg))

(define (gentext-pt-BR-xref-strings gind)
  (let* ((giname (if (string? gind) gind (gi gind)))
	 (name   (normalize giname)))
    (cond
      ((equal? name (normalize "appendix")) (pt-BR-appendix-xref-string gind))
      ((equal? name (normalize "article"))  (pt-BR-article-xref-string gind))
      ((equal? name (normalize "bibliography")) (pt-BR-bibliography-xref-string gind))
      ((equal? name (normalize "book"))     (pt-BR-book-xref-string gind))
      ((equal? name (normalize "chapter"))  (pt-BR-chapter-xref-string gind))
      ((equal? name (normalize "equation")) (pt-BR-equation-xref-string gind))
      ((equal? name (normalize "example"))  (pt-BR-example-xref-string gind))
      ((equal? name (normalize "figure"))   (pt-BR-figure-xref-string gind))
      ((equal? name (normalize "listitem")) (pt-BR-listitem-xref-string gind))
      ((equal? name (normalize "part"))     (pt-BR-part-xref-string gind))
      ((equal? name (normalize "preface"))  (pt-BR-preface-xref-string gind))
      ((equal? name (normalize "procedure")) (pt-BR-procedure-xref-string gind))
      ((equal? name (normalize "sect1"))    (pt-BR-sect1-xref-string gind))
      ((equal? name (normalize "sect2"))    (pt-BR-sect2-xref-string gind))
      ((equal? name (normalize "sect3"))    (pt-BR-sect3-xref-string gind))
      ((equal? name (normalize "sect4"))    (pt-BR-sect4-xref-string gind))
      ((equal? name (normalize "sect5"))    (pt-BR-sect5-xref-string gind))
      ((equal? name (normalize "step"))     (pt-BR-step-xref-string gind))
      ((equal? name (normalize "table"))    (pt-BR-table-xref-string gind))
      (else (pt-BR-default-xref-string gind)))))

(define (pt-BR-auto-xref-indirect-connector before) 
  (literal " &in; "))

;; Should the TOC come first or last?
;;
(define %generate-pt-BR-toc-in-front% #t)

;; gentext-element-name returns the generated text that should be 
;; used to make reference to the selected element.
;;
(define pt-BR-abstract-name	"&Abstract;")
(define pt-BR-appendix-name	"&Appendix;")
(define pt-BR-article-name	"&Article;")
(define pt-BR-bibliography-name	"&Bibliography;")
(define pt-BR-book-name		"&Book;")
(define pt-BR-calloutlist-name	"")
(define pt-BR-caution-name	"&Caution;")
(define pt-BR-chapter-name	"&Chapter;")
(define pt-BR-copyright-name	"&Copyright;")
(define pt-BR-dedication-name	"&Dedication;")
(define pt-BR-edition-name	"&Edition;")
(define pt-BR-equation-name	"&Equation;")
(define pt-BR-example-name	"&Example;")
(define pt-BR-figure-name	"&Figure;")
(define pt-BR-glossary-name	"&Glossary;")
(define pt-BR-glosssee-name	"&GlossSee;")
(define pt-BR-glossseealso-name	"&GlossSeeAlso;")
(define pt-BR-important-name	"&Important;")
(define pt-BR-index-name		"&Index;")
(define pt-BR-setindex-name	"&SetIndex;")
(define pt-BR-isbn-name		"&ISBN;")
(define pt-BR-legalnotice-name	"&LegalNotice;")
(define pt-BR-msgaud-name	"&MsgAud;")
(define pt-BR-msglevel-name	"&MsgLevel;")
(define pt-BR-msgorig-name	"&MsgOrig;")
(define pt-BR-note-name		"&Note;")
(define pt-BR-part-name		"&Part;")
(define pt-BR-preface-name	"&Preface;")
(define pt-BR-procedure-name	"&Procedure;")
(define pt-BR-pubdate-name	"&Published;")
(define pt-BR-reference-name	"&Reference;")
(define pt-BR-refname-name	"&RefName;")
(define pt-BR-revhistory-name	"&RevHistory;")
(define pt-BR-revision-name	"&Revision;")
(define pt-BR-sect1-name		"&Section;")
(define pt-BR-sect2-name		"&Section;")
(define pt-BR-sect3-name		"&Section;")
(define pt-BR-sect4-name		"&Section;")
(define pt-BR-sect5-name		"&Section;")
(define pt-BR-simplesect-name	"&Section;")
(define pt-BR-seeie-name		"&See;")
(define pt-BR-seealsoie-name	"&Seealso;")
(define pt-BR-set-name		"&Set;")
(define pt-BR-sidebar-name	"&Sidebar;")
(define pt-BR-step-name		"&step;")
(define pt-BR-table-name		"&Table;")
(define pt-BR-tip-name		"&Tip;")
(define pt-BR-toc-name		"&TableofContents;")
(define pt-BR-warning-name	"&Warning;")

(define (gentext-pt-BR-element-name gind)
  (let* ((giname (if (string? gind) gind (gi gind)))
	 (name   (normalize giname)))
    (cond
     ((equal? name (normalize "abstract"))	pt-BR-abstract-name)
     ((equal? name (normalize "appendix"))	pt-BR-appendix-name)
     ((equal? name (normalize "article"))	pt-BR-article-name)
     ((equal? name (normalize "bibliography"))	pt-BR-bibliography-name)
     ((equal? name (normalize "book"))		pt-BR-book-name)
     ((equal? name (normalize "calloutlist"))	pt-BR-calloutlist-name)
     ((equal? name (normalize "caution"))	pt-BR-caution-name)
     ((equal? name (normalize "chapter"))	pt-BR-chapter-name)
     ((equal? name (normalize "copyright"))	pt-BR-copyright-name)
     ((equal? name (normalize "dedication"))	pt-BR-dedication-name)
     ((equal? name (normalize "edition"))	pt-BR-edition-name)
     ((equal? name (normalize "equation"))	pt-BR-equation-name)
     ((equal? name (normalize "example"))	pt-BR-example-name)
     ((equal? name (normalize "figure"))	pt-BR-figure-name)
     ((equal? name (normalize "glossary"))	pt-BR-glossary-name)
     ((equal? name (normalize "glosssee"))	pt-BR-glosssee-name)
     ((equal? name (normalize "glossseealso"))	pt-BR-glossseealso-name)
     ((equal? name (normalize "important"))	pt-BR-important-name)
     ((equal? name (normalize "index"))		pt-BR-index-name)
     ((equal? name (normalize "setindex"))	pt-BR-setindex-name)
     ((equal? name (normalize "isbn"))		pt-BR-isbn-name)
     ((equal? name (normalize "legalnotice"))	pt-BR-legalnotice-name)
     ((equal? name (normalize "msgaud"))	pt-BR-msgaud-name)
     ((equal? name (normalize "msglevel"))	pt-BR-msglevel-name)
     ((equal? name (normalize "msgorig"))	pt-BR-msgorig-name)
     ((equal? name (normalize "note"))		pt-BR-note-name)
     ((equal? name (normalize "part"))		pt-BR-part-name)
     ((equal? name (normalize "preface"))	pt-BR-preface-name)
     ((equal? name (normalize "procedure"))	pt-BR-procedure-name)
     ((equal? name (normalize "pubdate"))	pt-BR-pubdate-name)
     ((equal? name (normalize "reference"))	pt-BR-reference-name)
     ((equal? name (normalize "refname"))	pt-BR-refname-name)
     ((equal? name (normalize "revhistory"))	pt-BR-revhistory-name)
     ((equal? name (normalize "revision"))	pt-BR-revision-name)
     ((equal? name (normalize "sect1"))		pt-BR-sect1-name)
     ((equal? name (normalize "sect2"))		pt-BR-sect2-name)
     ((equal? name (normalize "sect3"))		pt-BR-sect3-name)
     ((equal? name (normalize "sect4"))		pt-BR-sect4-name)
     ((equal? name (normalize "sect5"))		pt-BR-sect5-name)
     ((equal? name (normalize "simplesect"))    pt-BR-simplesect-name)
     ((equal? name (normalize "seeie"))		pt-BR-seeie-name)
     ((equal? name (normalize "seealsoie"))	pt-BR-seealsoie-name)
     ((equal? name (normalize "set"))		pt-BR-set-name)
     ((equal? name (normalize "sidebar"))	pt-BR-sidebar-name)
     ((equal? name (normalize "step"))		pt-BR-step-name)
     ((equal? name (normalize "table"))		pt-BR-table-name)
     ((equal? name (normalize "tip"))		pt-BR-tip-name)
     ((equal? name (normalize "toc"))		pt-BR-toc-name)
     ((equal? name (normalize "warning"))	pt-BR-warning-name)
     (else (let* ((msg (string-append "&unexpectedelementname;: " name))
		  (err (node-list-error msg (current-node))))
	     msg)))))

;; gentext-element-name-space returns gentext-element-name with a 
;; trailing space, if gentext-element-name isn't "".
;;
(define (gentext-pt-BR-element-name-space giname)
  (string-with-space (gentext-element-name giname)))

;; gentext-intra-label-sep returns the seperator to be inserted
;; between multiple occurances of a label (or parts of a label)
;; for the specified element.  Most of these are for enumerated
;; labels like "Figure 2-4", but this function is used elsewhere
;; (e.g. REFNAME) with a little abuse.
;;

(define pt-BR-equation-intra-label-sep "-")
(define pt-BR-example-intra-label-sep "-")
(define pt-BR-figure-intra-label-sep "-")
(define pt-BR-procedure-intra-label-sep ".")
(define pt-BR-refentry-intra-label-sep ".")
(define pt-BR-reference-intra-label-sep ".")
(define pt-BR-refname-intra-label-sep ", ")
(define pt-BR-refsect1-intra-label-sep ".")
(define pt-BR-refsect2-intra-label-sep ".")
(define pt-BR-refsect3-intra-label-sep ".")
(define pt-BR-sect1-intra-label-sep ".")
(define pt-BR-sect2-intra-label-sep ".")
(define pt-BR-sect3-intra-label-sep ".")
(define pt-BR-sect4-intra-label-sep ".")
(define pt-BR-sect5-intra-label-sep ".")
(define pt-BR-step-intra-label-sep ".")
(define pt-BR-table-intra-label-sep "-")
(define pt-BR-_pagenumber-intra-label-sep "-")
(define pt-BR-default-intra-label-sep "")

(define (gentext-pt-BR-intra-label-sep gind)
  (let* ((giname (if (string? gind) gind (gi gind)))
	 (name   (normalize giname)))
    (cond
     ((equal? name (normalize "equation"))	pt-BR-equation-intra-label-sep)
     ((equal? name (normalize "example"))	pt-BR-example-intra-label-sep)
     ((equal? name (normalize "figure"))	pt-BR-figure-intra-label-sep)
     ((equal? name (normalize "procedure"))	pt-BR-procedure-intra-label-sep)
     ((equal? name (normalize "refentry"))	pt-BR-refentry-intra-label-sep)
     ((equal? name (normalize "reference"))	pt-BR-reference-intra-label-sep)
     ((equal? name (normalize "refname"))	pt-BR-refname-intra-label-sep)
     ((equal? name (normalize "refsect1"))	pt-BR-refsect1-intra-label-sep)
     ((equal? name (normalize "refsect2"))	pt-BR-refsect2-intra-label-sep)
     ((equal? name (normalize "refsect3"))	pt-BR-refsect3-intra-label-sep)
     ((equal? name (normalize "sect1"))		pt-BR-sect1-intra-label-sep)
     ((equal? name (normalize "sect2"))		pt-BR-sect2-intra-label-sep)
     ((equal? name (normalize "sect3"))		pt-BR-sect3-intra-label-sep)
     ((equal? name (normalize "sect4"))		pt-BR-sect4-intra-label-sep)
     ((equal? name (normalize "sect5"))		pt-BR-sect5-intra-label-sep)
     ((equal? name (normalize "step"))		pt-BR-step-intra-label-sep)
     ((equal? name (normalize "table"))		pt-BR-table-intra-label-sep)
     ((equal? name (normalize "_pagenumber"))	pt-BR-_pagenumber-intra-label-sep)
     (else pt-BR-default-intra-label-sep))))

;; gentext-label-title-sep returns the seperator to be inserted
;; between a label and the text following the label for the
;; specified element.  Most of these are for use between
;; enumerated labels and titles like "1. Chapter One Title", but
;; this function is used elsewhere (e.g. NOTE) with a little
;; abuse.
;;

(define pt-BR-abstract-label-title-sep ": ")
(define pt-BR-appendix-label-title-sep ". ")
(define pt-BR-caution-label-title-sep "")
(define pt-BR-chapter-label-title-sep ". ")
(define pt-BR-equation-label-title-sep ". ")
(define pt-BR-example-label-title-sep ". ")
(define pt-BR-figure-label-title-sep ". ")
(define pt-BR-footnote-label-title-sep ". ")
(define pt-BR-glosssee-label-title-sep ": ")
(define pt-BR-glossseealso-label-title-sep ": ")
(define pt-BR-important-label-title-sep ": ")
(define pt-BR-note-label-title-sep ": ")
(define pt-BR-orderedlist-label-title-sep ". ")
(define pt-BR-part-label-title-sep ". ")
(define pt-BR-procedure-label-title-sep ". ")
(define pt-BR-prefix-label-title-sep ". ")
(define pt-BR-refentry-label-title-sep "")
(define pt-BR-reference-label-title-sep ". ")
(define pt-BR-refsect1-label-title-sep ". ")
(define pt-BR-refsect2-label-title-sep ". ")
(define pt-BR-refsect3-label-title-sep ". ")
(define pt-BR-sect1-label-title-sep ". ")
(define pt-BR-sect2-label-title-sep ". ")
(define pt-BR-sect3-label-title-sep ". ")
(define pt-BR-sect4-label-title-sep ". ")
(define pt-BR-sect5-label-title-sep ". ")
(define pt-BR-seeie-label-title-sep " ")
(define pt-BR-seealsoie-label-title-sep " ")
(define pt-BR-step-label-title-sep ". ")
(define pt-BR-table-label-title-sep ". ")
(define pt-BR-tip-label-title-sep ": ")
(define pt-BR-warning-label-title-sep "")
(define pt-BR-default-label-title-sep "")

(define (gentext-pt-BR-label-title-sep gind)
  (let* ((giname (if (string? gind) gind (gi gind)))
	 (name   (normalize giname)))
    (cond
     ((equal? name (normalize "abstract")) pt-BR-abstract-label-title-sep)
     ((equal? name (normalize "appendix")) pt-BR-appendix-label-title-sep)
     ((equal? name (normalize "caution")) pt-BR-caution-label-title-sep)
     ((equal? name (normalize "chapter")) pt-BR-chapter-label-title-sep)
     ((equal? name (normalize "equation")) pt-BR-equation-label-title-sep)
     ((equal? name (normalize "example")) pt-BR-example-label-title-sep)
     ((equal? name (normalize "figure")) pt-BR-figure-label-title-sep)
     ((equal? name (normalize "footnote")) pt-BR-footnote-label-title-sep)
     ((equal? name (normalize "glosssee")) pt-BR-glosssee-label-title-sep)
     ((equal? name (normalize "glossseealso")) pt-BR-glossseealso-label-title-sep)
     ((equal? name (normalize "important")) pt-BR-important-label-title-sep)
     ((equal? name (normalize "note")) pt-BR-note-label-title-sep)
     ((equal? name (normalize "orderedlist")) pt-BR-orderedlist-label-title-sep)
     ((equal? name (normalize "part")) pt-BR-part-label-title-sep)
     ((equal? name (normalize "procedure")) pt-BR-procedure-label-title-sep)
     ((equal? name (normalize "prefix")) pt-BR-prefix-label-title-sep)
     ((equal? name (normalize "refentry")) pt-BR-refentry-label-title-sep)
     ((equal? name (normalize "reference")) pt-BR-reference-label-title-sep)
     ((equal? name (normalize "refsect1")) pt-BR-refsect1-label-title-sep)
     ((equal? name (normalize "refsect2")) pt-BR-refsect2-label-title-sep)
     ((equal? name (normalize "refsect3")) pt-BR-refsect3-label-title-sep)
     ((equal? name (normalize "sect1")) pt-BR-sect1-label-title-sep)
     ((equal? name (normalize "sect2")) pt-BR-sect2-label-title-sep)
     ((equal? name (normalize "sect3")) pt-BR-sect3-label-title-sep)
     ((equal? name (normalize "sect4")) pt-BR-sect4-label-title-sep)
     ((equal? name (normalize "sect5")) pt-BR-sect5-label-title-sep)
     ((equal? name (normalize "seeie")) pt-BR-seeie-label-title-sep)
     ((equal? name (normalize "seealsoie")) pt-BR-seealsoie-label-title-sep)
     ((equal? name (normalize "step")) pt-BR-step-label-title-sep)
     ((equal? name (normalize "table")) pt-BR-table-label-title-sep)
     ((equal? name (normalize "tip")) pt-BR-tip-label-title-sep)
     ((equal? name (normalize "warning")) pt-BR-warning-label-title-sep)
     (else pt-BR-default-label-title-sep))))

(define (pt-BR-set-label-number-format gind) "1")
(define (pt-BR-book-label-number-format gind) "1")
(define (pt-BR-prefix-label-number-format gind) "1")
(define (pt-BR-part-label-number-format gind) "I")
(define (pt-BR-chapter-label-number-format gind) "1")
(define (pt-BR-appendix-label-number-format gind) "A")
(define (pt-BR-reference-label-number-format gind) "I")
(define (pt-BR-example-label-number-format gind) "1")
(define (pt-BR-figure-label-number-format gind) "1")
(define (pt-BR-table-label-number-format gind) "1")
(define (pt-BR-procedure-label-number-format gind) "1")
(define (pt-BR-step-label-number-format gind) "1")
(define (pt-BR-refsect1-label-number-format gind) "1")
(define (pt-BR-refsect2-label-number-format gind) "1")
(define (pt-BR-refsect3-label-number-format gind) "1")
(define (pt-BR-sect1-label-number-format gind) "1")
(define (pt-BR-sect2-label-number-format gind) "1")
(define (pt-BR-sect3-label-number-format gind) "1")
(define (pt-BR-sect4-label-number-format gind) "1")
(define (pt-BR-sect5-label-number-format gind) "1")
(define (pt-BR-default-label-number-format gind) "1")

(define (pt-BR-label-number-format gind)
  (let* ((giname (if (string? gind) gind (gi gind)))
	 (name   (normalize giname)))
    (cond
     ((equal? name (normalize "set")) (pt-BR-set-label-number-format gind))
     ((equal? name (normalize "book")) (pt-BR-book-label-number-format gind))
     ((equal? name (normalize "prefix")) (pt-BR-prefix-label-number-format gind))
     ((equal? name (normalize "part")) (pt-BR-part-label-number-format gind))
     ((equal? name (normalize "chapter")) (pt-BR-chapter-label-number-format gind))
     ((equal? name (normalize "appendix")) (pt-BR-appendix-label-number-format gind))
     ((equal? name (normalize "reference")) (pt-BR-reference-label-number-format gind))
     ((equal? name (normalize "example")) (pt-BR-example-label-number-format gind))
     ((equal? name (normalize "figure")) (pt-BR-figure-label-number-format gind))
     ((equal? name (normalize "table")) (pt-BR-table-label-number-format gind))
     ((equal? name (normalize "procedure")) (pt-BR-procedure-label-number-format gind))
     ((equal? name (normalize "step")) (pt-BR-step-label-number-format gind))
     ((equal? name (normalize "refsect1")) (pt-BR-refsect1-label-number-format gind))
     ((equal? name (normalize "refsect2")) (pt-BR-refsect2-label-number-format gind))
     ((equal? name (normalize "refsect3")) (pt-BR-refsect3-label-number-format gind))
     ((equal? name (normalize "sect1")) (pt-BR-sect1-label-number-format gind))
     ((equal? name (normalize "sect2")) (pt-BR-sect2-label-number-format gind))
     ((equal? name (normalize "sect3")) (pt-BR-sect3-label-number-format gind))
     ((equal? name (normalize "sect4")) (pt-BR-sect4-label-number-format gind))
     ((equal? name (normalize "sect5")) (pt-BR-sect5-label-number-format gind))
     (else (pt-BR-default-label-number-format gind)))))

(define ($lot-title-pt-BR$ gind)
  (let* ((giname (if (string? gind) gind (gi gind)))
	 (name   (normalize giname)))
    (cond ((equal? name (normalize "table"))    "&ListofTables;")
	  ((equal? name (normalize "example"))  "&ListofExamples;")
	  ((equal? name (normalize "figure"))   "&ListofFigures;")
	  ((equal? name (normalize "equation")) "&ListofEquations;")
	  (else (let* ((msg (string-append "&ListofUnknown;: " name))
		       (err (node-list-error msg (current-node))))
		  msg)))))

(define %gentext-pt-BR-start-quote% (dingbat "ldquo"))

(define %gentext-pt-BR-end-quote% (dingbat "rdquo"))

(define %gentext-pt-BR-start-nested-quote% (dingbat "lsquo"))

(define %gentext-pt-BR-end-nested-quote% (dingbat "rsquo"))

(define %gentext-pt-BR-by% "&by;") ;; e.g. Copyright 1997 "by" A. Nonymous
                           ;; Authored "by" Jane Doe

(define %gentext-pt-BR-edited-by% "&Editedby;")
                           ;; "Edited by" Jane Doe

(define %gentext-pt-BR-page% "")

(define %gentext-pt-BR-and% "&and;")

(define %gentext-pt-BR-bibl-pages% "&Pgs;")

(define %gentext-pt-BR-endnotes% "&Notes;")

(define %gentext-pt-BR-table-endnotes% "&TableNotes;:")

(define %gentext-pt-BR-index-see% "&See;")

(define %gentext-pt-BR-index-seealso% "&SeeAlso;")

