<!DOCTYPE style-sheet PUBLIC "-//James Clark//DTD DSSSL Style Sheet//EN" [
<!ENTITY % pt-BR.words SYSTEM "../common/dbl1pt-BR.ent">
%pt-BR.words;
<!ENTITY cmn.pt-BR SYSTEM "../common/dbl1pt-BR.dsl">
]>

<style-sheet>
<style-specification id="docbook-l10n-pt-BR">
<style-specification-body>

;; $Id$
;;
;; This file is part of the Modular DocBook Stylesheet distribution.
;; See ../README or http://www.berkshire.net/~norm/dsssl/
;;

&cmn.pt-BR;

;; These need to be translated!

(define (gentext-pt-BR-nav-prev prev) 
  (make sequence (literal "Prev")))

(define (gentext-pt-BR-nav-prevsib prevsib) 
  (make sequence (literal "Fast Backward")))

(define (gentext-pt-BR-nav-nextsib nextsib)
  (make sequence (literal "Fast Forward")))

(define (gentext-pt-BR-nav-next next)
  (make sequence (literal "Next")))

(define (gentext-pt-BR-nav-up up)
  (make sequence (literal "Up")))

(define (gentext-pt-BR-nav-home home)
  (make sequence (literal "Home")))

</style-specification-body>
</style-specification>
</style-sheet>
