;; This file is part of the KDE application ksgmltools
;; For licensing, documentation etc, refer to that application
;; This file should not be distributed outside ksgmltools

; Redefined from html/dbnavig.dsl
; What: more things in header table (only table+ and ff-)
; Why: extra table would? not work, since we don't want whitespace in between them
; (they are one unit - is more convincing ;-)
; Moreover, changing this function, rather than using the hook ensures that
; all elements get the change
; Attention: when the definition changes
(define (default-header-nav-tbl-noff elemnode prev next prevsib nextsib)
  (let* ((navbar-height "85")
	 (navbar2-height "25")
	 (navbar1-color "#DDDDDD")
	 (navbar2-color "#515151")
	 (r1? (nav-banner? elemnode))
	 (r1-sosofo (make element gi: "TR"
			  (make element gi: "TD"
				attributes: '(("CLASS" "bulb"))
				(make empty-element gi: "IMG"
				      attributes: (list
						   (list "SRC" (string-append %kde-doc-common-path% "doctop1.png"))
						   (list "ALT" "Light bulb")
						   (list "WIDTH" "150")
						   (list "HEIGHT" navbar-height))))
			  (make element gi: "TD"
				attributes: (list
					     (list "COLSPAN" "2")
					     (list "HEIGHT" navbar-height)
					     (list "CLASS" "headline"))
				; H1 is a hack!
				(make element gi: "H1"
				      (nav-banner elemnode)))))
	 (r2? (or (not (node-list-empty? prev))
		  (not (node-list-empty? next))
		  (nav-context? elemnode)))
	 (r2-sosofo (make sequence 
		      (make element gi: "TR"
			  attributes:  (list
					(list "STYLE" "background-color: #515151;")
					(list "CLASS" "navrow"))
			  (make element gi: "TD"
				attributes: (list
					     (list "ALIGN" "left")
					     (list "CLASS" "left")
					     (list "HEIGHT" navbar2-height))
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
					     (list "ALIGN" "center")
					     (list "CLASS" "center")
					     (list "HEIGHT" navbar2-height))
				(nav-context elemnode)
				(make entity-ref name: "nbsp")) ; hack!
			  (make element gi: "TD"
				attributes: (list
					     (list "ALIGN" "right")
					     (list "CLASS" "right")
					     (list "HEIGHT" navbar2-height))
				(if (node-list-empty? next)
				    (make entity-ref name: "nbsp")
				    (make element gi: "A"
					  attributes: (list
						       (list "HREF" 
							     (href-to
							      next)))
					  (gentext-nav-next next)))))
		     (make element gi: "TR"
			   (make element gi: "TD"
				 attributes:  (list (list "COLSPAN" "3")
						    (list "CLASS" "shadow"))
				(make empty-element gi: "IMG"
				      attributes: (list
						   (list "WIDTH" "100%")
						   (list "SRC" (string-append %kde-doc-common-path% "shadow.png"))
						   (list "ALT" "Shadow")
						   (list "HEIGHT" "14"))))))))
    (if (or r1? r2?)
	(make element gi: "TABLE"
	      attributes: (list
			   (list "CLASS" "navheader")
			   ;(list "WIDTH" %gentext-nav-tblwidth%)
			   (list "BORDER" "0")
			   (list "CELLPADDING" "0")
			   (list "CELLSPACING" "0"))
	      (if r1? r1-sosofo (empty-sosofo))
	      (if r2? r2-sosofo (empty-sosofo)))
	(empty-sosofo))))

; copied from html/dbnavig.dsl
; Reason: we don't want a HR element
; and a second table was added (for logos)
(define (default-footer-nav-tbl elemnode prev next prevsib nextsib)
  (let ((r1? (or (not (node-list-empty? prev))
		 (not (node-list-empty? next))
		 (nav-home? elemnode)))
	(r2? (or (not (node-list-empty? prev))
		 (not (node-list-empty? next))
		 (nav-up? elemnode)))
	(r1-sosofo (make element gi: "TR"
			 (make element gi: "TD"
			       attributes: (list
					    (list "WIDTH" "33%")
					    (list "ALIGN" "left")
					    (list "VALIGN" "top"))
			       (if (node-list-empty? prev)
				   (make entity-ref name: "nbsp")
				   (make element gi: "A"
					 attributes: (list
						      (list "HREF" (href-to
								    prev)))
					 (gentext-nav-prev prev))))
			 (make element gi: "TD"
			       attributes: (list
					    (list "WIDTH" "34%")
					    (list "ALIGN" "center")
					    (list "VALIGN" "top"))
			       (nav-home-link elemnode))
			 (make element gi: "TD"
			       attributes: (list
					    (list "WIDTH" "33%")
					    (list "ALIGN" "right")
					    (list "VALIGN" "top"))
			       (if (node-list-empty? next)
				   (make entity-ref name: "nbsp")
				   (make element gi: "A"
					 attributes: (list
						      (list "HREF" (href-to
								    next)))
					 (gentext-nav-next next))))))
	(r2-sosofo (make element gi: "TR"
			 (make element gi: "TD"
			       attributes: (list
					    (list "WIDTH" "33%")
					    (list "ALIGN" "left")
					    (list "VALIGN" "top"))
			       (if (node-list-empty? prev)
				   (make entity-ref name: "nbsp")
				   (element-title-sosofo prev)))
			 (make element gi: "TD"
			       attributes: (list
					    (list "WIDTH" "34%")
					    (list "ALIGN" "center")
					    (list "VALIGN" "top"))
			       (if (nav-up? elemnode)
				   (nav-up elemnode)
				   (make entity-ref name: "nbsp")))
			 (make element gi: "TD"
			       attributes: (list
					    (list "WIDTH" "33%")
					    (list "ALIGN" "right")
					    (list "VALIGN" "top"))
			       (if (node-list-empty? next)
				   (make entity-ref name: "nbsp")
				   (element-title-sosofo next))))))
    (if (or r1? r2?)
	(make element gi: "DIV"
	      attributes: '(("CLASS" "NAVFOOTER"))
	      (let ((navbar-height "59"))
		(make element gi: "TABLE"
		      attributes: (list (list "WIDTH" "100%")
					(list "BORDER" "0")      ; these three to be removed?
					(list "CELLPADDING" "0")
					(list "CELLSPACING" "0"))
		      (make element gi: "COLGROUP"
			    (make empty-element gi: "COL"
				  attributes: '(("WIDTH" "*")))
			    (make empty-element gi: "COL"
				  attributes: '(("WIDTH" "227"))))
		      (make element gi: "TBODY"
			    (make element gi: "TR"
				  (make element gi: "TD"
					attributes: (list (list "CLASS" "end")
							  (list "WIDTH" "100%"))
					(make empty-element gi: "IMG"
					      attributes: (list 
							   (list "SRC" (string-append %kde-doc-common-path% "bottom1.png"))
							   (list "ALT" "Lines")
							   (list "HEIGHT" navbar-height))))
				  (make element gi: "TD"
					(make element gi: "A"
					      attributes: (list 
							   (list "HREF" (kde-gentext-home-url)))
					      (make empty-element gi: "IMG"
						    attributes: (list
								 (list "SRC" (string-append %kde-doc-common-path% "bottom2.png"))
								 (list "ALT" "Bottom decoration")
								 (list "WIDTH" "227")
								 (list "BORDER" "0")
								 (list "HEIGHT" navbar-height))))))
			    (make element gi: "TR"
				  (make element gi: "TD"
					attributes: (list
						     (list "HEIGHT" "50")
						     (list "COLSPAN" "2"))
					(make entity-ref name: "nbsp"))))))
	      (make element gi: "TABLE"
		    attributes: (list
				 (list "WIDTH" %gentext-nav-tblwidth%)
				 (list "BORDER" "0")
				 (list "CELLPADDING" "0")
				 (list "CELLSPACING" "0"))
		    (if r1? r1-sosofo (empty-sosofo))
		    (if r2? r2-sosofo (empty-sosofo))))
	(empty-sosofo))))
