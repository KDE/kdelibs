<!DOCTYPE style-sheet PUBLIC "-//James Clark//DTD DSSSL Style Sheet//EN" [
<!ENTITY % cs.words SYSTEM "../common/dbl1cs.ent">
%cs.words;
<!ENTITY cmn.cs SYSTEM "../common/dbl1cs.dsl">
]>

<style-sheet>
<style-specification id="docbook-l10n-cs">
<style-specification-body>

;; $Id$
;;
;; This file is part of the Modular DocBook Stylesheet distribution.
;; See ../README or http://www.berkshire.net/~norm/dsssl/
;;

&cmn.cs;

(define (gentext-cs-nav-prev prev) 
  (make sequence (literal "P\U-0159;edch\U-00E1;zej\U-00ED;c\U-00ED;")))

(define (gentext-cs-nav-prevsib prevsib) 
  (make sequence (literal "Rychle zp\U-011B;t")))

(define (gentext-cs-nav-nextsib nextsib)
  (make sequence (literal "Rychle dop\U-0159;edu")))

(define (gentext-cs-nav-next next)
  (make sequence (literal "Dal\U-0161;\U-00ED;")))

(define (gentext-cs-nav-up up)
  (make sequence (literal "Nahoru")))

(define (gentext-cs-nav-home home)
  (make sequence (literal "Dom\U-016F;")))

</style-specification-body>
</style-specification>
</style-sheet>
