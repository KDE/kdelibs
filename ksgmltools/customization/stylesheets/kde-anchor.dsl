;; -*- dsssl -*-
;; This file is part of the KDE application ksgmltools
;; For licensing, documentation etc, refer to that application
;; This file should not be distributed outside ksgmltools

;; KDE new
(define %anchor-index% #t)
;; KDE new
(define %anchor-index-filename% ".anchors") ; .anchor/.anchor-index/anchor.index/HTML.anchors/...

;; KDE new
(mode anchorindex
  ;; this mode is really just a hack to get at the root element
  (root (process-children))

  (default 
    (if (node-list=? (current-node) (sgml-root-element))
	(make entity
	  system-id: (html-entity-file %anchor-index-filename%)
	  (process-node-list (select-elements 
			      (descendants (current-node))
			      (normalize "anchor"))))
	(empty-sosofo)))

  ; This generates an anchor entry - is this a good format?
  (element anchor
    (make sequence
	(make formatting-instruction data: "anchor ")
        ; the first field is not really necessary: it's repeated in the second
        (make formatting-instruction data: (attribute-string (normalize "id")))
	(make formatting-instruction data: " ")
        (make formatting-instruction data: (href-to (current-node)))
	(htmlnewline)
	(htmlindexattr "role")
	(htmlindexattr "os")
	(htmlindexattr "xreflabel")
	(htmlindexattr "remap")
	(htmlindexattr "userlevel")
	(htmlindexattr "revisionflag")
	(htmlindexattr "conformance")
	(htmlindexattr "pagenum")
	(htmlindexattr "vendor")
	(htmlindexattr "arch")
	(htmlindexattr "revision")
	(process-children) ; is empty anyway
	(make formatting-instruction data: "/anchor")
	(htmlnewline)))
; one problem left: it seems a bit pointless to refer to sections with anchors when they already have IDs.  Is that true?
  )

;; Origin: html/docbook.dsl
;; How:    removed comments;
;;         Added as last sexp in (make sequence ...)
;;         (if %anchor-index% (with-mode anchorindex (process-children)) (empty-sosofo))
;; Related: added (define %anchor-index% ...);
;;          added (mode anchorindex ...);
;;          added (define %anchor-index-filename% ...)
;; Why:    creates file %anchor-index-filename% with anchors for referencing 
;;         from applications (not necessarily HTML only: could be PDF/PostScript as well)
;;         TO MAKE MORE GENERAL FOR THIS PURPOSE (eg no href)
;; Watch out: - if (root ...) is modified in html/docbook.dsl
(root
 (make sequence
   (process-children)
   (with-mode manifest
     (process-children))
   (if html-index
       (with-mode htmlindex
	 (process-children))
       (empty-sosofo))
   (if %anchor-index% ; this was added
       (with-mode anchorindex
	 (process-children))
       (empty-sosofo))))
