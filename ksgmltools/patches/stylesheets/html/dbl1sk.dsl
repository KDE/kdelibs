<!DOCTYPE style-sheet PUBLIC "-//James Clark//DTD DSSSL Style Sheet//EN" [
<!ENTITY % sk.words SYSTEM "../common/dbl1sk.ent">
%sk.words;
<!ENTITY cmn.sk SYSTEM "../common/dbl1sk.dsl">
]>

<style-sheet>
<style-specification id="docbook-l10n-sk">
<style-specification-body>

;; $Id$
;;
;; This file is part of the Modular DocBook Stylesheet distribution.
;; See ../README or http://www.berkshire.net/~norm/dsssl/
;;

&cmn.sk;

(define (gentext-sk-nav-prev prev) 
  (make sequence (literal "Predch\U-00E1;dzaj\U-00FA;ci")))

(define (gentext-sk-nav-prevsib prevsib) 
  (make sequence (literal "R\U-00FD;chlo nazp\U-00E4;t")))

(define (gentext-sk-nav-nextsib nextsib)
  (make sequence (literal "R\U-00FD;chlo dopredu")))

(define (gentext-sk-nav-next next)
  (make sequence (literal "\U-010E;al\U-0161;\U-00ED;")))

(define (gentext-sk-nav-up up)
  (make sequence (literal "Hore")))

(define (gentext-sk-nav-home home)
  (make sequence (literal "Domov")))

</style-specification-body>
</style-specification>
</style-sheet>
