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
	(make sequence
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
)))

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

; Customisation hook
(define ($html-body-content-end$)
  (let ((navbar-height "57"))
    (make element gi: "DIV"
	  attributes: (list '("CLASS" "navfooterlogo"))
	  (make element gi: "TABLE"
		(make element gi: "TBODY"
		      (make element gi: "TR"
			    (make element gi: "TD"
				  (make element gi: "IMG"
					attributes: (list
						     (list "SRC" (string-append %kde-doc-common-path% "lines2.png"))
						     (list "ALT" "Lines")
						     (list "HEIGHT" navbar-height))))
			    (make element gi: "TD"
				  (make element gi: "A"
					attributes: (list 
						     (list "HREF" (kde-gentext-home-url)))
					(make element gi: "IMG"
					      attributes: (list
							   (list "SRC" (string-append %kde-doc-common-path% "kdelogo2.png"))
							   (list "ALT" (kde-gentext-logo-alt))
							   (list "WIDTH" "204")
							   (list "HEIGHT" navbar-height))))))))))

; Redefined from html/dbnavig.dsl
; What: more things in header table (only table+ and ff-)
; Why: extra table would? not work, since we don't want whitespace in between them
; (they are one unit - is more convincing ;-)
; Moreover, changing this function, rather than using the hook ensures that
; all elements get the change
; Attention: when the definition changes
(define (default-header-nav-tbl-noff elemnode prev next prevsib nextsib)
  (let* ((navbar-height "83")
	 (navbar1-color "#DDDDDD")
	 (navbar2-color "#505050")
	 (r1? (nav-banner? elemnode))
	 (r1-sosofo (make element gi: "TR"
			  attributes: (list (list "BGCOLOR" navbar1-color))
			  (make element gi: "TH"
				attributes: (list
					     (list "BGCOLOR" navbar1-color)
					     (list "ALIGN" "center")
					     (list "VALIGN" "top"))
					; alternative is background, but this is not as good
				(make element gi: "IMG"
				      attributes: (list
						   (list "SRC" (string-append %kde-doc-common-path% "lightbulb.png"))
						   (list "ALT" "Light bulb")
						   (list "WIDTH" "142")
						   (list "HEIGHT" navbar-height))))
			  (make element gi: "TH"
				attributes: (list
					     (list "BGCOLOR" navbar1-color)
					     (list "STYLE" (string-append "background: url(" %kde-doc-common-path% "lines.png" ");"))
					     (list "COLSPAN" "2")
					     (list "HEIGHT" navbar-height)
					     (list "ALIGN" "center")
					     (list "VALIGN" "top"))
				(nav-banner elemnode))))
	 (r2? (or (not (node-list-empty? prev))
		  (not (node-list-empty? next))
		  (nav-context? elemnode)))
	 (r2-sosofo (make element gi: "TR"
			  attributes:  (list (list "BGCOLOR" "navbar2-color"))
			  (make element gi: "TD"
				attributes: (list
					     (list "BGCOLOR" "navbar2-color")
					     (list "WIDTH" "10%")
					     (list "ALIGN" "left")
					     (list "VALIGN" "bottom"))
				(if (node-list-empty? prev)
				    (make entity-ref name: "nbsp")
				    (make element gi: "A"
					  attributes: (list
						       (list "HREF" 
							     (href-to 
							      prev)))
					  (gentext-nav-prev prev))))
			  (make element gi: "TD"
				attributes: (list
					     (list "BGCOLOR" "navbar2-color")
					     (list "WIDTH" "80%")
					     (list "ALIGN" "center")
					     (list "VALIGN" "bottom"))
				(nav-context elemnode))
			  (make element gi: "TD"
				attributes: (list
					     (list "BGCOLOR" "navbar2-color")
					     (list "WIDTH" "10%")
					     (list "ALIGN" "right")
					     (list "VALIGN" "bottom"))
				(if (node-list-empty? next)
				    (make entity-ref name: "nbsp")
				    (make element gi: "A"
					  attributes: (list
						       (list "HREF" 
							     (href-to
							      next)))
					  (gentext-nav-next next)))))))
    (if (or r1? r2?)
	(make element gi: "DIV"
	      attributes: '(("CLASS" "NAVHEADER"))
	  (make element gi: "TABLE"
		attributes: (list
			     (list "WIDTH" %gentext-nav-tblwidth%)
			     (list "BORDER" "0")
			     (list "CELLPADDING" "0")
			     (list "CELLSPACING" "0"))
		(if r1? r1-sosofo (empty-sosofo))
		(if r2? r2-sosofo (empty-sosofo))))
	(empty-sosofo))))
