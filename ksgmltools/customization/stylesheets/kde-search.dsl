;; -*- dsssl -*-
;; This file is part of the KDE application ksgmltools
;; For licensing, documentation etc, refer to that application
;; This file should not be distributed outside ksgmltools

;; KDE new
(define %search-index% #f)
;; KDE new
(define %search-index-filename% ".keywords") ; .keyword/.keyword-index/keyword.index/HTML.keywords/...

;; KDE new
(mode searchindex
  ;; this mode is really just a hack to get at the root element
  (root (process-children))

  (default
    (if (node-list=? (current-node) (sgml-root-element))
	(make entity
	  system-id: (html-entity-file %search-index-filename%)
	  (process-node-list (select-elements
			      (descendants (current-node))
			      (normalize "keywordset"))))
	(empty-sosofo)))

  ; This generates an keyword entry - is this a good format?
  ; Keywords should NOT be legislated: use subjectterms for that
  ; As they are not used yet, they are not processed here.
  (element keywordset
    (make sequence
	(make formatting-instruction data: "keywordset ")
        (make formatting-instruction data: (href-to (current-node)))
	(htmlnewline)
	(htmlindexattr "arch")
	(htmlindexattr "conformance")
	(htmlindexattr "id")
	(htmlindexattr "lang")
	(htmlindexattr "os")
	(htmlindexattr "remap")
	(htmlindexattr "revision")
	(htmlindexattr "revisionflag")
	(htmlindexattr "role")
	(htmlindexattr "userlevel")
	(htmlindexattr "vendor")
	(htmlindexattr "xreflabel")
	(process-children)
	(make formatting-instruction data: "/keywordset")
	(htmlnewline)))

  (element keyword
    (make sequence
	(make formatting-instruction data: "keyword ")
	(htmlnewline)
	(htmlindexattr "arch")
	(htmlindexattr "conformance")
	(htmlindexattr "id")
	(htmlindexattr "lang")
	(htmlindexattr "os")
	(htmlindexattr "remap")
	(htmlindexattr "revision")
	(htmlindexattr "revisionflag")
	(htmlindexattr "role")
	(htmlindexattr "userlevel")
	(htmlindexattr "vendor")
	(htmlindexattr "xreflabel")
	(process-children)
	(make formatting-instruction data: "/keyword")
	(htmlnewline)))
  )
