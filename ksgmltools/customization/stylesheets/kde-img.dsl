<!-- this is derived from imagemap.dsl  from conrib/imagemap in DBMSS distribution -->
; This file might not be necessary?  To try ...
; (it isn't when DBMSS can find it on its own, and does the right thing)
(element (graphicco areaspec)
  (make element gi: "MAP"
	attributes: (list (list "NAME" (element-id)))
	(let loop ((nl (select-elements (descendants (current-node)) 
					(normalize "area"))))
	  (if (node-list-empty? nl)
	      (empty-sosofo)
	      (make sequence
		(make-html-area (node-list-first nl))
		(loop (node-list-rest nl)))))))

(define ($graphic$ fileref 
		   #!optional (format #f) (alt #f) (align #f))
  (let* ((csim?  (equal? (gi (parent (current-node)))
			 (normalize "graphicco")))
	 (ismap  (if csim?
		     '(("ISMAP" "ISMAP"))
		     '()))
	 (usemap (if csim?
		     (let* ((graphicco (parent (current-node)))
			    (areaspec  (select-elements 
					(children graphicco)
					(normalize "areaspec"))))
		       (list (list "USEMAP"
				   (string-append "#"
						  (attribute-string 
						   (normalize "id")
						   areaspec)))))
		     '()))
	 (border (if csim? '(("BORDER" "0")) '()))
	 (img-attr  (append
		     (list     (list "SRC" (graphic-file fileref)))
		     (if align (list (list "ALIGN" align)) '())
		     (if image-library (graphic-attrs fileref alt) '())
		     ismap
		     usemap
		     border)))
    (make empty-element gi: "IMG"
	  attributes: img-attr)))

; fromprint/dbgraph.dsl
(define ($graphic$ fileref 
		   #!optional (display #f) (format #f) (scale #f) (align #f))
  (let ((graphic-format (if format format ""))
	(graphic-scale  (if scale (/  (string->number scale) 100) 1))
	(graphic-align  (cond ((equal? align (normalize "center"))
			       'center)
			      ((equal? align (normalize "right"))
			       'end)
			      (else
			       'start))))
    (make external-graphic
      entity-system-id: (graphic-file fileref)
      notation-system-id: graphic-format
      scale: graphic-scale
      display?: display
      display-alignment: graphic-align)))

(define (make-html-area area)
  (let* ((shape  (attribute-string (normalize "otherunits") area))
	 (coords (attribute-string (normalize "coords") area))
	 (idrefs (attribute-string (normalize "linkends") area))
	 (idlist (if idrefs 
		     (split idrefs)
		     '()))
	 (target (if idrefs
		     (element-with-id (car idlist))
		     (empty-node-list)))
	 (href   (attribute-string (normalize "label") area)))
    (if idrefs
	(make empty-element gi: "AREA"
	      attributes: (list (list "SHAPE" shape)
				(list "COORDS" coords)
				(list "HREF" (href-to target))))
	(make empty-element gi: "AREA"
	      attributes: (list (list "SHAPE" shape)
				(list "COORDS" coords)
				(list "HREF" href))))))

(define ($kde-graphic$ fileref 
		       #!optional (display #f) (format #f) (scale #f) (align #f))
  (let ((graphic-format (if format format ""))
	(graphic-scale  (if scale (/  (string->number scale) 100) 1))
	(graphic-align  (cond ((equal? align (normalize "center"))
			       'center)
			      ((equal? align (normalize "right"))
			       'end)
			      (else
			       'start))))
    (make external-graphic
      entity-system-id: (graphic-file fileref)
      notation-system-id: graphic-format
      scale: graphic-scale
      display?: display
      display-alignment: graphic-align)))
