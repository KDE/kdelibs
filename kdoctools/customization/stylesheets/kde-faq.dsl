;; -*- dsssl -*-
;; This file is part of the KDE application ksgmltools
;; For licensing, documentation etc, refer to that application
;; This file should not be distributed outside ksgmltools

;; Origin: html/db31.dsl (and taken analogies from html/dbsect.dsl)
;; How:    wrapped with (html-document (with-mode ...) ...) 
;;         (taken from dbsect.dsl from section element);
;;         core has remained unchanged
;; Related: chunk-element-list;
;;          chunk-skip-first-element-list 
;; Why:    gets chunks from qandadiv
;; Watch out: - if (element qandadiv ...) is modified
;;            - if (element section(?) ...) is modified
(element qandadiv
; if parent is qandadiv and first, no new document - here ??
;  (if (equal? (gi (parent (current-node))) (normalize "qandadiv"))
; or only for articles class=faq?
  (html-document
   (with-mode head-title-mode
     (literal (element-title-string (current-node))))
   (make element gi: "DIV"
	 attributes: (list (list "CLASS" (gi)))
	 (process-children))))

;; Origin: html/dbchunk.dsl
;; How:    Prepended qandaset and qandadiv to list of default values
;; Why:    to create references to qandasets/divs in chunk files
;; Watch out: - if (define (chunk-element-list) ...) is modified 
(define (chunk-element-list)
  (append
   (list (normalize "qandaset")
	 (normalize "qandadiv"))
   ;; default below this
   (list (normalize "preface")
	 (normalize "chapter")
	 (normalize "appendix") 
	 (normalize "article")
	 (normalize "glossary")
	 (normalize "bibliography")
	 (normalize "index")
	 (normalize "colophon")
	 (normalize "setindex")
	 (normalize "reference")
	 (normalize "refentry")
	 (normalize "part")
	 (normalize "sect1") 
	 (normalize "section") 
	 (normalize "book")
	 (normalize "set")
	 )))

;; Origin: html/dbchunk.dsl
;; How:    Add qandadiv to list of default values
;;         if the qandaset has no ToC, otherwise, don't skip the first
;;         element (= put the first qandadiv in the same file as the set)
;;         qandaset is added as well, because it is in chunk-element-list
;;         (and we don't want to lose count of the elements; without it
;;         jade generated qandadiv14 for the link and qandadiv15 for the 
;;         file name, only for the first page).
;; Why:    to create references to qandasets/divs in chunk files
;; Watch out: - if (define (chunk-skip-first-element-list) ...) is modified
;;            - if (define ($generate-qandaset-toc$) is modified
;; Note: it should also test if the parent is a qandaset, but that cannot
;;       be determined at this stage yet (no current node).  It may be
;;       done elsewhere already (qandadivs can be nested, that's the problem).
(define (chunk-skip-first-element-list)
  (append
   (list (normalize "qandaset"))
   ; and if its parent is a qandaset
   (if ($generate-qandaset-toc$) '() (list (normalize "qandadiv")))
   ;; default below this
   (list (normalize "sect1")
	 (normalize "section"))))
