<!DOCTYPE style-sheet PUBLIC "-//James Clark//DTD DSSSL Style Sheet//EN" [
<!ENTITY % et.words SYSTEM "../common/dbl1et.ent">
%et.words;
<!ENTITY cmn.et SYSTEM "../common/dbl1et.dsl">
]>

<style-sheet>
<style-specification id="docbook-l10n-et">
<style-specification-body>

;; $Id$
;;
;; This file is part of the Modular DocBook Stylesheet distribution.
;; See ../README or http://www.berkshire.net/~norm/dsssl/
;;

&cmn.et;

(define (gentext-et-nav-prev prev) 
  (make sequence (literal "Eelmine")))

(define (gentext-et-nav-prevsib prevsib) 
  (make sequence (literal "Kiiresti tagasi")))

(define (gentext-et-nav-nextsib nextsib)
  (make sequence (literal "Kiiresti edasi")))

(define (gentext-et-nav-next next)
  (make sequence (literal "Järgmine")))

(define (gentext-et-nav-up up)
  (make sequence (literal "Üles")))

(define (gentext-et-nav-home home)
  (make sequence (literal "Koju")))

</style-specification-body>
</style-specification>
</style-sheet>
