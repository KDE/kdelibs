<!DOCTYPE style-sheet PUBLIC "-//James Clark//DTD DSSSL Style Sheet//EN" [
<!ENTITY % el.words SYSTEM "../common/dbl1el.ent">
%el.words;
<!ENTITY cmn.el SYSTEM "../common/dbl1el.dsl">
]>

<style-sheet>
<style-specification id="docbook-l10n-el">
<style-specification-body>

;; $Id$
;;
;; This file is part of the Modular DocBook Stylesheet distribution.
;; See ../README or http://www.berkshire.net/~norm/dsssl/
;;

&cmn.el;

(define (gentext-el-nav-prev prev) 
  (make sequence (literal "Prev")))

(define (gentext-el-nav-prevsib prevsib) 
  (make sequence (literal "Fast Backward")))

(define (gentext-el-nav-nextsib nextsib)
  (make sequence (literal "Fast Forward")))

(define (gentext-el-nav-next next)
  (make sequence (literal "Next")))

(define (gentext-el-nav-up up)
  (make sequence (literal "Up")))

(define (gentext-el-nav-home home)
  (make sequence (literal "Home")))

</style-specification-body>
</style-specification>
</style-sheet>
