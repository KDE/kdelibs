<!DOCTYPE style-sheet PUBLIC "-//James Clark//DTD DSSSL Style Sheet//EN" [
<!ENTITY % lat2 PUBLIC "ISO 8879:1986//ENTITIES Added Latin 2//EN">
%lat2;
<!ENTITY % sl.words SYSTEM "../common/dbl1sl.ent">
%sl.words;
<!ENTITY cmn.sl SYSTEM "../common/dbl1sl.dsl">
]>

<style-sheet>
<style-specification id="docbook-l10n-sl">
<style-specification-body>

;; $Id$
;;
;; This file is part of the Modular DocBook Stylesheet distribution.
;; See ../README or http://www.berkshire.net/~norm/dsssl/
;;

&cmn.sl;

(define (gentext-sl-nav-prev prev) 
  (make sequence (literal "Nazaj")))

(define (gentext-sl-nav-prevsib prevsib) 
  (make sequence (literal "Hitro nazaj")))

(define (gentext-sl-nav-nextsib nextsib)
  (make sequence (literal "Hitro naprej")))

(define (gentext-sl-nav-next next)
  (make sequence (literal "Naprej")))

(define (gentext-sl-nav-up up)
  (make sequence (literal "Gor")))

(define (gentext-sl-nav-home home)
  (make sequence (literal "Domov")))

</style-specification-body>
</style-specification>
</style-sheet>
