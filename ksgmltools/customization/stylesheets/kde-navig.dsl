;; This file is part of the KDE application ksgmltools
;; For licensing, documentation etc, refer to that application
;; This file should not be distributed outside ksgmltools

(define (kde-navigation-links #!optional 
			      (prev (empty-node-list))
			      (next (empty-node-list))
			      (prevm (empty-node-list))
			      (nextm (empty-node-list)))
  (make element gi: "DIV"
	attributes: '(("ALIGN" "RIGHT") ("CLASS" "NAVBAR"))
	(if (and (node-list-empty? prev)
		 (node-list-empty? prevm)
		 (node-list-empty? nextm)
		 (node-list-empty? next))
	    (empty-sosofo)
	    (make element gi: "P"
		  (make sequence 
		    (if (node-list-empty? prev)
			(gentext-nav-prev prev)
			(make sequence
			  (make element gi: "A"
				attributes: (list
					     (list "HREF" (href-to prev)))
				(gentext-nav-prev prev))))
		    (literal " ")
		    (kde-nav-toc)
		    (literal " ")
		    (if (node-list-empty? next)
			(gentext-nav-next next)
			(make sequence 
			  (make element gi: "A"
				attributes: (list
					     (list "HREF" (href-to next)))
				(gentext-nav-next next)))))))))

(define %kde-logo% #f)

(define ($html-body-start$ #!optional
			   (prev (empty-node-list))
			   (next (empty-node-list))
			   (prevm (empty-node-list))
			   (nextm (empty-node-list)))
  (make element gi: "DIV"
	attributes: (list (list "CLASS" "logoheader"))
	(make element gi: "A"
	      attributes: (list (list "HREF" (kde-gentext-home-url)))
	      (make empty-element gi: "IMG"
		    attributes: (list (list "SRC" (string-append %kde-doc-common-path% "logotp3.png"))
				      (list "BORDER" "0")
				      (list "ALT" (kde-gentext-logo-alt))
				      (list "HEIGHT" "62")
				      (list "WIDTH" "229"))))
	(if %kde-logo%
	    (make element gi: "A"
		  attributes: (list (list "HREF" (kde-gentext-home-url)))
		  (make empty-element gi: "IMG"
			attributes: (list (list "SRC" (string-append %kde-doc-common-path% %kde-logo%))
					  (list "BORDER" "0")
					  (list "ALT" (kde-gentext-logo-alt))
					  (list "HEIGHT" "62")
					  (list "WIDTH" "229"))))
	    (empty-sosofo))
;     (make empty-element gi: "HR"
; 	  attributes: '(("WIDTH" "100%")
; 			("SIZE" "2")
; 			("ALIGN" "CENTER")
; 			("NOSHADE" "NOSHADE")))
;    (kde-navigation-links prev next prevm nextm)
))

; (define ($user-footer-navigation$ #!optional
; 				  (prev (empty-node-list))
; 				  (next (empty-node-list))
; 				  (prevm (empty-node-list))
; 				  (nextm (empty-node-list)))
;   (make sequence
;     (kde-navigation-links prev next prevm nextm)
;     (make empty-element gi: "HR"
; 	  attributes: '(("WIDTH" "100%")
; 			("SIZE" "2")
; 			("ALIGN" "CENTER")
; 			("NOSHADE" "NOSHADE")))))

; KDE-new
; derived from nav-up - can be improved (eg root =/= toc)
(define (kde-nav-toc)
  (let* ((up   (sgml-root-element))
	 (href (href-to up)))
    (if (kde-toc?)
	(if (node-list=? (current-node) up) ; is the node a root element?
	    (literal (gentext-element-name "toc"))
	    (make element gi: "A"
		  attributes: (list
			       (list "HREF" href))
		  (make sequence (literal (gentext-element-name "toc")))))
	(empty-sosofo))))

; KDE-new
; the list is fairly complete
(define (kde-toc?)
  (let ((doc (gi (sgml-root-element))))
    (cond ((equal? doc (normalize "article")) %generate-article-toc%)
	  ((equal? doc (normalize "book")) %generate-book-toc%)
	  ((equal? doc (normalize "set")) %generate-set-toc%)
	  ((equal? doc (normalize "part")) %generate-part-toc%)
	  ((equal? doc (normalize "reference")) %generate-reference-toc%)
	  ((equal? doc (normalize "qandaset")) ($generate-qandaset-toc$))
	  ((equal? doc (normalize "chapter")) ($generate-chapter-toc$))
	  (else #f))))
