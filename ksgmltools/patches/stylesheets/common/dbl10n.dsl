;; $Id$
;;
;; This file is part of the Modular DocBook Stylesheet distribution.
;; See ../README or http://nwalsh.com/docbook/dsssl/
;;

;; ----------------------------- Localization -----------------------------

;; If you create a new version of this file, please send it to
;; Norman Walsh, ndw@nwalsh.com.  Please use the ISO 639 language
;; code to identify the language.  Append a subtag as per RFC 1766,
;; if necessary.

;; The generated text for cross references to elements.  See dblink.dsl
;; for a discussion of how substitution is performed on the %x and #x
;; keywords.
;;

;; The following language codes from ISO 639 are recognized:
;; ca - Catalan
;; cs - Czech
;; da - Danish (previously dk)
;; de - German (previously dege)
;; el - Greek
;; en - English (previously usen)
;; es - Spanish
;; et - Estonian
;; fi - Finnish
;; fr - French
;; it - Italian
;; ja - Japanese
;; nl - Dutch
;; no - Norwegian (previously bmno) ???
;; pl - Polish
;; pt - Portuguese
;; pt-BR - Portuguese (Brazil)
;; ro - Romanian
;; ru - Russian
;; sl - Slovenian
;; sk - Slovak
;; sv - Swedish (previously svse)
;; zh-CN - Chinese (Continental)

;; The following language codes are recognized for historical reasons:

;; bmno - Norwegian (Norsk Bokmal) ???
;; dege - German
;; dk   - Danish
;; svse - Swedish
;; usen - English

(define %default-language% "en")
(define %gentext-language% #f)
(define %gentext-use-xref-lang% #f)

(define ($lang$ #!optional (target (current-node)) (xref-context #f))
  (if %gentext-language%
      (lang-fix %gentext-language%)
      (if (or xref-context %gentext-use-xref-lang%)
	  (let loop ((here target))
	    (if (node-list-empty? here)
		(lang-fix %default-language%)
		(if (attribute-string (normalize "lang") here)
		    (lang-fix (attribute-string (normalize "lang") here))
		    (loop (parent here)))))
	  (if (inherited-attribute-string (normalize "lang"))
	      (lang-fix (inherited-attribute-string (normalize "lang")))
	      (lang-fix %default-language%)))))

(define (lang-fix language)
  ;; Lowercase the language
  ;; Translate 'xx_yy' to 'xx-yy'
  (let ((fixed-lang (if (> (string-index language "-") 0)
			(let ((pos (string-index language "-")))
			  (string-append
			   (substring language 0 pos)
			   "-"
			   (substring language (+ pos 1)
				      (string-length language))))
			language)))
    (case-fold-down fixed-lang)))

;; bmno - Norwegian (Norsk Bokmal) ???
;; dege - German
;; dk   - Danish
;; svse - Swedish
;; usen - English

(define (gentext-xref-strings target)
  (let ((giname (if (string? target) (normalize target) (gi target)))
	(lang   (if (string? target) ($lang$) ($lang$ target))))
    (case lang
      ;; ISO 639/ISO 3166/RFC 1766
      <![%l10n-ca[ (("ca") (gentext-ca-xref-strings giname)) ]]>
      <![%l10n-cs[ (("cs") (gentext-cs-xref-strings giname)) ]]>
      <![%l10n-da[ (("da") (gentext-da-xref-strings giname)) ]]>
      <![%l10n-de[ (("de") (gentext-de-xref-strings giname)) ]]>
      <![%l10n-el[ (("el") (gentext-el-xref-strings giname)) ]]>
      <![%l10n-en[ (("en") (gentext-en-xref-strings giname)) ]]>
      <![%l10n-es[ (("es") (gentext-es-xref-strings giname)) ]]>
      <![%l10n-et[ (("et") (gentext-et-xref-strings giname)) ]]>
      <![%l10n-fi[ (("fi") (gentext-fi-xref-strings giname)) ]]>
      <![%l10n-fr[ (("fr") (gentext-fr-xref-strings giname)) ]]>
      <![%l10n-hu[ (("hu") (gentext-hu-xref-strings giname)) ]]>
      <![%l10n-it[ (("it") (gentext-it-xref-strings giname)) ]]>
      <![%l10n-ja[ (("ja") (gentext-ja-xref-strings giname)) ]]>
      <![%l10n-nl[ (("nl") (gentext-nl-xref-strings giname)) ]]>
      <![%l10n-no[ (("no") (gentext-no-xref-strings giname)) ]]>
      <![%l10n-pl[ (("pl") (gentext-pl-xref-strings giname)) ]]>
      <![%l10n-pt[ (("pt") (gentext-pt-xref-strings giname)) ]]>
      <![%l10n-pt-BR[ (("pt-br") (gentext-pt-BR-xref-strings giname)) ]]>
      <![%l10n-ro[ (("ro") (gentext-ro-xref-strings giname)) ]]>
      <![%l10n-ru[ (("ru") (gentext-ru-xref-strings giname)) ]]>
      <![%l10n-sk[ (("sk") (gentext-sk-xref-strings giname)) ]]>
      <![%l10n-sl[ (("sl") (gentext-sl-xref-strings giname)) ]]>
      <![%l10n-sv[ (("sv") (gentext-sv-xref-strings giname)) ]]>
      <![%l10n-zh-CN[ (("zh-cn") (gentext-zh-CN-xref-strings giname)) ]]>

      ;; Historical
      <![%l10n-da[ (("dk")   (gentext-da-xref-strings giname)) ]]>
      <![%l10n-de[ (("dege") (gentext-de-xref-strings giname)) ]]>
      <![%l10n-en[ (("usen") (gentext-en-xref-strings giname)) ]]>
      <![%l10n-no[ (("bmno") (gentext-no-xref-strings giname)) ]]>
      <![%l10n-sv[ (("svse") (gentext-sv-xref-strings giname)) ]]>
      (else (error (string-append "L10N ERROR: gentext-xref-strings: " 
				  lang))))))

(define (auto-xref-indirect-connector before) 
  (case ($lang$)
    <![%l10n-ca[ (("ca") (ca-auto-xref-indirect-connector before)) ]]>
    <![%l10n-cs[ (("cs") (cs-auto-xref-indirect-connector before)) ]]>
    <![%l10n-da[ (("da") (da-auto-xref-indirect-connector before)) ]]>
    <![%l10n-de[ (("de") (de-auto-xref-indirect-connector before)) ]]>
    <![%l10n-el[ (("el") (el-auto-xref-indirect-connector before)) ]]>
    <![%l10n-en[ (("en") (en-auto-xref-indirect-connector before)) ]]>
    <![%l10n-es[ (("es") (es-auto-xref-indirect-connector before)) ]]>
    <![%l10n-et[ (("et") (et-auto-xref-indirect-connector before)) ]]>
    <![%l10n-fi[ (("fi") (fi-auto-xref-indirect-connector before)) ]]>
    <![%l10n-fr[ (("fr") (fr-auto-xref-indirect-connector before)) ]]>
    <![%l10n-it[ (("it") (it-auto-xref-indirect-connector before)) ]]>
    <![%l10n-ja[ (("ja") (ja-auto-xref-indirect-connector before)) ]]>
    <![%l10n-nl[ (("nl") (nl-auto-xref-indirect-connector before)) ]]>
    <![%l10n-no[ (("no") (no-auto-xref-indirect-connector before)) ]]>
    <![%l10n-pl[ (("pl") (pl-auto-xref-indirect-connector before)) ]]>
    <![%l10n-pt[ (("pt") (pt-auto-xref-indirect-connector before)) ]]>
    <![%l10n-pt-BR[ (("pt-br") (pt-BR-auto-xref-indirect-connector before)) ]]>
    <![%l10n-ro[ (("ro") (ro-auto-xref-indirect-connector before)) ]]>
    <![%l10n-ru[ (("ru") (ru-auto-xref-indirect-connector before)) ]]>
    <![%l10n-sl[ (("sl") (sl-auto-xref-indirect-connector before)) ]]>
    <![%l10n-sk[ (("sk") (sk-auto-xref-indirect-connector before)) ]]>
    <![%l10n-sv[ (("sv") (sv-auto-xref-indirect-connector before)) ]]>
    <![%l10n-zh-CN[ (("zh-cn") (zh-CN-auto-xref-indirect-connector before)) ]]>

    <![%l10n-da[ (("dk") (da-auto-xref-indirect-connector before)) ]]>
    <![%l10n-de[ (("dege") (de-auto-xref-indirect-connector before)) ]]>
    <![%l10n-en[ (("usen") (en-auto-xref-indirect-connector before)) ]]>
    <![%l10n-no[ (("bmno") (no-auto-xref-indirect-connector before)) ]]>
    <![%l10n-sv[ (("svse") (sv-auto-xref-indirect-connector before)) ]]>
    (else (error "L10N ERROR: auto-xref-indirect-connector"))))

(define (generate-toc-in-front)
  (case ($lang$)
    <![%l10n-ca[ (("ca") %generate-ca-toc-in-front%) ]]>
    <![%l10n-cs[ (("cs") %generate-cs-toc-in-front%) ]]>
    <![%l10n-da[ (("da") %generate-da-toc-in-front%) ]]>
    <![%l10n-de[ (("de") %generate-de-toc-in-front%) ]]>
    <![%l10n-el[ (("el") %generate-el-toc-in-front%) ]]>
    <![%l10n-en[ (("en") %generate-en-toc-in-front%) ]]>
    <![%l10n-es[ (("es") %generate-es-toc-in-front%) ]]>
    <![%l10n-et[ (("et") %generate-et-toc-in-front%) ]]>
    <![%l10n-fi[ (("fi") %generate-fi-toc-in-front%) ]]>
    <![%l10n-fr[ (("fr") %generate-fr-toc-in-front%) ]]>
    <![%l10n-it[ (("it") %generate-it-toc-in-front%) ]]>
    <![%l10n-ja[ (("ja") %generate-ja-toc-in-front%) ]]>
    <![%l10n-nl[ (("nl") %generate-nl-toc-in-front%) ]]>
    <![%l10n-no[ (("no") %generate-no-toc-in-front%) ]]>
    <![%l10n-pl[ (("pl") %generate-pl-toc-in-front%) ]]>
    <![%l10n-pt[ (("pt") %generate-pt-toc-in-front%) ]]>
    <![%l10n-pt-BR[ (("pt-br") %generate-pt-BR-toc-in-front%) ]]>
    <![%l10n-ro[ (("ro") %generate-ro-toc-in-front%) ]]>
    <![%l10n-ru[ (("ru") %generate-ru-toc-in-front%) ]]>
    <![%l10n-sk[ (("sk") %generate-sk-toc-in-front%) ]]>
    <![%l10n-sl[ (("sl") %generate-sl-toc-in-front%) ]]>
    <![%l10n-sv[ (("sv") %generate-sv-toc-in-front%) ]]>
    <![%l10n-zh-CN[ (("zh-cn") %generate-zh-CN-toc-in-front%) ]]>

    <![%l10n-da[ (("dk") %generate-da-toc-in-front%) ]]>
    <![%l10n-de[ (("dege") %generate-de-toc-in-front%) ]]>
    <![%l10n-en[ (("usen") %generate-en-toc-in-front%) ]]>
    <![%l10n-no[ (("bmno") %generate-no-toc-in-front%) ]]>
    <![%l10n-sv[ (("svse") %generate-sv-toc-in-front%) ]]>
    (else (error "L10N ERROR: generate-toc-in-front"))))

(define (gentext-element-name target)
  (let ((giname (if (string? target) (normalize target) (gi target)))
	(lang   (if (string? target) ($lang$) ($lang$ target #t))))
    (case lang
      <![%l10n-ca[ (("ca") (gentext-ca-element-name giname)) ]]>
      <![%l10n-cs[ (("cs") (gentext-cs-element-name giname)) ]]>
      <![%l10n-da[ (("da") (gentext-da-element-name giname)) ]]>
      <![%l10n-de[ (("de") (gentext-de-element-name giname)) ]]>
      <![%l10n-el[ (("el") (gentext-el-element-name giname)) ]]>
      <![%l10n-en[ (("en") (gentext-en-element-name giname)) ]]>
      <![%l10n-es[ (("es") (gentext-es-element-name giname)) ]]>
      <![%l10n-et[ (("et") (gentext-et-element-name giname)) ]]>
      <![%l10n-fi[ (("fi") (gentext-fi-element-name giname)) ]]>
      <![%l10n-fr[ (("fr") (gentext-fr-element-name giname)) ]]>
      <![%l10n-hu[ (("hu") (gentext-hu-element-name giname)) ]]>
      <![%l10n-it[ (("it") (gentext-it-element-name giname)) ]]>
      <![%l10n-ja[ (("ja") (gentext-ja-element-name giname)) ]]>
      <![%l10n-nl[ (("nl") (gentext-nl-element-name giname)) ]]>
      <![%l10n-no[ (("no") (gentext-no-element-name giname)) ]]>
      <![%l10n-pl[ (("pl") (gentext-pl-element-name giname)) ]]>
      <![%l10n-pt[ (("pt") (gentext-pt-element-name giname)) ]]>
      <![%l10n-pt-BR[ (("pt-br") (gentext-pt-BR-element-name giname)) ]]>
      <![%l10n-ro[ (("ro") (gentext-ro-element-name giname)) ]]>
      <![%l10n-ru[ (("ru") (gentext-ru-element-name giname)) ]]>
      <![%l10n-sk[ (("sk") (gentext-sk-element-name giname)) ]]>
      <![%l10n-sl[ (("sl") (gentext-sl-element-name giname)) ]]>
      <![%l10n-sv[ (("sv") (gentext-sv-element-name giname)) ]]>
      <![%l10n-zh-CN[ (("zh-cn") (gentext-zh-CN-element-name giname)) ]]>

      <![%l10n-da[ (("dk")   (gentext-da-element-name giname)) ]]>
      <![%l10n-de[ (("dege") (gentext-de-element-name giname)) ]]>
      <![%l10n-en[ (("usen") (gentext-en-element-name giname)) ]]>
      <![%l10n-no[ (("bmno") (gentext-no-element-name giname)) ]]>
      <![%l10n-sv[ (("svse") (gentext-sv-element-name giname)) ]]>
      (else (error (string-append "L10N ERROR: gentext-element-name: "
				  lang))))))

(define (gentext-element-name-space target)
  (let ((giname (if (string? target) (normalize target) (gi target)))
	(lang   (if (string? target) ($lang$) ($lang$ target))))
    (case lang
      <![%l10n-ca[ (("ca") (gentext-ca-element-name-space giname)) ]]>
      <![%l10n-cs[ (("cs") (gentext-cs-element-name-space giname)) ]]>
      <![%l10n-da[ (("da") (gentext-da-element-name-space giname)) ]]>
      <![%l10n-de[ (("de") (gentext-de-element-name-space giname)) ]]>
      <![%l10n-el[ (("el") (gentext-el-element-name-space giname)) ]]>
      <![%l10n-en[ (("en") (gentext-en-element-name-space giname)) ]]>
      <![%l10n-es[ (("es") (gentext-es-element-name-space giname)) ]]>
      <![%l10n-et[ (("et") (gentext-et-element-name-space giname)) ]]>
      <![%l10n-fi[ (("fi") (gentext-fi-element-name-space giname)) ]]>
      <![%l10n-fr[ (("fr") (gentext-fr-element-name-space giname)) ]]>
      <![%l10n-hu[ (("hu") (gentext-hu-element-name-space giname)) ]]>
      <![%l10n-it[ (("it") (gentext-it-element-name-space giname)) ]]>
      <![%l10n-ja[ (("ja") (gentext-ja-element-name-space giname)) ]]>
      <![%l10n-nl[ (("nl") (gentext-nl-element-name-space giname)) ]]>
      <![%l10n-no[ (("no") (gentext-no-element-name-space giname)) ]]>
      <![%l10n-pl[ (("pl") (gentext-pl-element-name-space giname)) ]]>
      <![%l10n-pt[ (("pt") (gentext-pt-element-name-space giname)) ]]>
      <![%l10n-pt-BR[ (("pt-br") (gentext-pt-BR-element-name-space giname)) ]]>
      <![%l10n-ro[ (("ro") (gentext-ro-element-name-space giname)) ]]>
      <![%l10n-ru[ (("ru") (gentext-ru-element-name-space giname)) ]]>
      <![%l10n-sk[ (("sk") (gentext-sk-element-name-space giname)) ]]>
      <![%l10n-sl[ (("sl") (gentext-sl-element-name-space giname)) ]]>
      <![%l10n-sv[ (("sv") (gentext-sv-element-name-space giname)) ]]>
      <![%l10n-zh-CN[ (("zh-cn") (gentext-zh-CN-element-name-space giname)) ]]>

      <![%l10n-da[ (("dk")   (gentext-da-element-name-space giname)) ]]>
      <![%l10n-de[ (("dege") (gentext-de-element-name-space giname)) ]]>
      <![%l10n-en[ (("usen") (gentext-en-element-name-space giname)) ]]>
      <![%l10n-no[ (("bmno") (gentext-no-element-name-space giname)) ]]>
      <![%l10n-sv[ (("svse") (gentext-sv-element-name-space giname)) ]]>
      (else (error "L10N ERROR: gentext-element-name-space")))))

(define (gentext-intra-label-sep target)
  (let ((giname (if (string? target) (normalize target) (gi target)))
	(lang   (if (string? target) ($lang$) ($lang$ target))))
    (case lang
      <![%l10n-ca[ (("ca") (gentext-ca-intra-label-sep giname)) ]]>
      <![%l10n-cs[ (("cs") (gentext-cs-intra-label-sep giname)) ]]>
      <![%l10n-da[ (("da") (gentext-da-intra-label-sep giname)) ]]>
      <![%l10n-de[ (("de") (gentext-de-intra-label-sep giname)) ]]>
      <![%l10n-el[ (("el") (gentext-el-intra-label-sep giname)) ]]>
      <![%l10n-en[ (("en") (gentext-en-intra-label-sep giname)) ]]>
      <![%l10n-es[ (("es") (gentext-es-intra-label-sep giname)) ]]>
      <![%l10n-et[ (("et") (gentext-et-intra-label-sep giname)) ]]>
      <![%l10n-fi[ (("fi") (gentext-fi-intra-label-sep giname)) ]]>
      <![%l10n-fr[ (("fr") (gentext-fr-intra-label-sep giname)) ]]>
      <![%l10n-hu[ (("hu") (gentext-hu-intra-label-sep giname)) ]]>
      <![%l10n-it[ (("it") (gentext-it-intra-label-sep giname)) ]]>
      <![%l10n-ja[ (("ja") (gentext-ja-intra-label-sep giname)) ]]>
      <![%l10n-nl[ (("nl") (gentext-nl-intra-label-sep giname)) ]]>
      <![%l10n-no[ (("no") (gentext-no-intra-label-sep giname)) ]]>
      <![%l10n-pl[ (("pl") (gentext-pl-intra-label-sep giname)) ]]>
      <![%l10n-pt[ (("pt") (gentext-pt-intra-label-sep giname)) ]]>
      <![%l10n-pt-BR[ (("pt-br") (gentext-pt-BR-intra-label-sep giname)) ]]>
      <![%l10n-ro[ (("ro") (gentext-ro-intra-label-sep giname)) ]]>
      <![%l10n-ru[ (("ru") (gentext-ru-intra-label-sep giname)) ]]>
      <![%l10n-sk[ (("sk") (gentext-sk-intra-label-sep giname)) ]]>
      <![%l10n-sl[ (("sl") (gentext-sl-intra-label-sep giname)) ]]>
      <![%l10n-sv[ (("sv") (gentext-sv-intra-label-sep giname)) ]]>
      <![%l10n-zh-CN[ (("zh-cn") (gentext-zh-CN-intra-label-sep giname)) ]]>

      <![%l10n-da[ (("dk")   (gentext-da-intra-label-sep giname)) ]]>
      <![%l10n-de[ (("dege") (gentext-de-intra-label-sep giname)) ]]>
      <![%l10n-en[ (("usen") (gentext-en-intra-label-sep giname)) ]]>
      <![%l10n-no[ (("bmno") (gentext-no-intra-label-sep giname)) ]]>
      <![%l10n-sv[ (("svse") (gentext-sv-intra-label-sep giname)) ]]>
      (else (error "L10N ERROR: gentext-intra-label-sep")))))

(define (gentext-label-title-sep target)
  (let ((giname (if (string? target) (normalize target) (gi target)))
	(lang   (if (string? target) ($lang$) ($lang$ target))))
    (case lang
      <![%l10n-ca[ (("ca") (gentext-ca-label-title-sep giname)) ]]>
      <![%l10n-cs[ (("cs") (gentext-cs-label-title-sep giname)) ]]>
      <![%l10n-da[ (("da") (gentext-da-label-title-sep giname)) ]]>
      <![%l10n-de[ (("de") (gentext-de-label-title-sep giname)) ]]>
      <![%l10n-el[ (("el") (gentext-el-label-title-sep giname)) ]]>
      <![%l10n-en[ (("en") (gentext-en-label-title-sep giname)) ]]>
      <![%l10n-es[ (("es") (gentext-es-label-title-sep giname)) ]]>
      <![%l10n-et[ (("et") (gentext-et-label-title-sep giname)) ]]>
      <![%l10n-fi[ (("fi") (gentext-fi-label-title-sep giname)) ]]>
      <![%l10n-fr[ (("fr") (gentext-fr-label-title-sep giname)) ]]>
      <![%l10n-hu[ (("hu") (gentext-hu-label-title-sep giname)) ]]>
      <![%l10n-it[ (("it") (gentext-it-label-title-sep giname)) ]]>
      <![%l10n-ja[ (("ja") (gentext-ja-label-title-sep giname)) ]]>
      <![%l10n-nl[ (("nl") (gentext-nl-label-title-sep giname)) ]]>
      <![%l10n-no[ (("no") (gentext-no-label-title-sep giname)) ]]>
      <![%l10n-pl[ (("pl") (gentext-pl-label-title-sep giname)) ]]>
      <![%l10n-pt[ (("pt") (gentext-pt-label-title-sep giname)) ]]>
      <![%l10n-pt-BR[ (("pt-br") (gentext-pt-BR-label-title-sep giname)) ]]>
      <![%l10n-ro[ (("ro") (gentext-ro-label-title-sep giname)) ]]>
      <![%l10n-ru[ (("ru") (gentext-ru-label-title-sep giname)) ]]>
      <![%l10n-sk[ (("sk") (gentext-sk-label-title-sep giname)) ]]>
      <![%l10n-sl[ (("sl") (gentext-sl-label-title-sep giname)) ]]>
      <![%l10n-sv[ (("sv") (gentext-sv-label-title-sep giname)) ]]>
      <![%l10n-zh-CN[ (("zh-cn") (gentext-zh-CN-label-title-sep giname)) ]]>

      <![%l10n-da[ (("dk")   (gentext-da-label-title-sep giname)) ]]>
      <![%l10n-de[ (("dege") (gentext-de-label-title-sep giname)) ]]>
      <![%l10n-en[ (("usen") (gentext-en-label-title-sep giname)) ]]>
      <![%l10n-no[ (("bmno") (gentext-no-label-title-sep giname)) ]]>
      <![%l10n-sv[ (("svse") (gentext-sv-label-title-sep giname)) ]]>
      (else (error "L10N ERROR: gentext-label-title-sep")))))

(define (label-number-format target)
  (let ((giname (if (string? target) (normalize target) (gi target)))
	(lang   (if (string? target) ($lang$) ($lang$ target))))
    (case lang
      <![%l10n-ca[ (("ca") (ca-label-number-format target)) ]]>
      <![%l10n-cs[ (("cs") (cs-label-number-format target)) ]]>
      <![%l10n-da[ (("da") (da-label-number-format target)) ]]>
      <![%l10n-de[ (("de") (de-label-number-format target)) ]]>
      <![%l10n-el[ (("el") (el-label-number-format target)) ]]>
      <![%l10n-en[ (("en") (en-label-number-format target)) ]]>
      <![%l10n-es[ (("es") (es-label-number-format target)) ]]>
      <![%l10n-et[ (("et") (et-label-number-format target)) ]]>
      <![%l10n-fi[ (("fi") (fi-label-number-format target)) ]]>
      <![%l10n-fr[ (("fr") (fr-label-number-format target)) ]]>
      <![%l10n-it[ (("it") (it-label-number-format target)) ]]>
      <![%l10n-ja[ (("ja") (ja-label-number-format target)) ]]>
      <![%l10n-nl[ (("nl") (nl-label-number-format target)) ]]>
      <![%l10n-no[ (("no") (no-label-number-format target)) ]]>
      <![%l10n-pl[ (("pl") (pl-label-number-format target)) ]]>
      <![%l10n-pt[ (("pt") (pt-label-number-format target)) ]]>
      <![%l10n-pt-BR[ (("pt-br") (pt-BR-label-number-format target)) ]]>
      <![%l10n-ro[ (("ro") (ro-label-number-format target)) ]]>
      <![%l10n-ru[ (("ru") (ru-label-number-format target)) ]]>
      <![%l10n-sl[ (("sl") (sl-label-number-format target)) ]]>
      <![%l10n-sk[ (("sk") (sk-label-number-format target)) ]]>
      <![%l10n-sv[ (("sv") (sv-label-number-format target)) ]]>
      <![%l10n-zh-CN[ (("zh-cn") (zh-CN-label-number-format target)) ]]>

      <![%l10n-da[ (("dk")   (da-label-number-format target)) ]]>
      <![%l10n-de[ (("dege") (de-label-number-format target)) ]]>
      <![%l10n-en[ (("usen") (en-label-number-format target)) ]]>
      <![%l10n-no[ (("bmno") (no-label-number-format target)) ]]>
      <![%l10n-sv[ (("svse") (sv-label-number-format target)) ]]>
      (else (error "L10N ERROR: label-number-format")))))

(define ($lot-title$ lotgi)
  (case ($lang$)
    <![%l10n-ca[ (("ca") ($lot-title-ca$ lotgi)) ]]>
    <![%l10n-cs[ (("cs") ($lot-title-cs$ lotgi)) ]]>
    <![%l10n-da[ (("da") ($lot-title-da$ lotgi)) ]]>
    <![%l10n-de[ (("de") ($lot-title-de$ lotgi)) ]]>
    <![%l10n-el[ (("el") ($lot-title-el$ lotgi)) ]]>
    <![%l10n-en[ (("en") ($lot-title-en$ lotgi)) ]]>
    <![%l10n-es[ (("es") ($lot-title-es$ lotgi)) ]]>
    <![%l10n-et[ (("et") ($lot-title-et$ lotgi)) ]]>
    <![%l10n-fi[ (("fi") ($lot-title-fi$ lotgi)) ]]>
    <![%l10n-fr[ (("fr") ($lot-title-fr$ lotgi)) ]]>
    <![%l10n-hu[ (("hu") ($lot-title-hu$ lotgi)) ]]>
    <![%l10n-it[ (("it") ($lot-title-it$ lotgi)) ]]>
    <![%l10n-ja[ (("ja") ($lot-title-ja$ lotgi)) ]]>
    <![%l10n-nl[ (("nl") ($lot-title-nl$ lotgi)) ]]>
    <![%l10n-no[ (("no") ($lot-title-no$ lotgi)) ]]>
    <![%l10n-pl[ (("pl") ($lot-title-pl$ lotgi)) ]]>
    <![%l10n-pt[ (("pt") ($lot-title-pt$ lotgi)) ]]>
    <![%l10n-pt-BR[ (("pt-br") ($lot-title-pt-BR$ lotgi)) ]]>
    <![%l10n-ro[ (("ro") ($lot-title-ro$ lotgi)) ]]>
    <![%l10n-ru[ (("ru") ($lot-title-ru$ lotgi)) ]]>
    <![%l10n-sl[ (("sl") ($lot-title-sl$ lotgi)) ]]>
    <![%l10n-sk[ (("sk") ($lot-title-sk$ lotgi)) ]]>
    <![%l10n-sv[ (("sv") ($lot-title-sv$ lotgi)) ]]>
    <![%l10n-zh-CN[ (("zh-cn") ($lot-title-zh-CN$ lotgi)) ]]>

    <![%l10n-da[ (("dk")   ($lot-title-da$ lotgi)) ]]>
    <![%l10n-de[ (("dege") ($lot-title-de$ lotgi)) ]]>
    <![%l10n-en[ (("usen") ($lot-title-en$ lotgi)) ]]>
    <![%l10n-no[ (("bmno") ($lot-title-no$ lotgi)) ]]>
    <![%l10n-sv[ (("svse") ($lot-title-sv$ lotgi)) ]]>
    (else (error "L10N ERROR: $lot-title$"))))

(define (gentext-start-quote)
  (case ($lang$)
    <![%l10n-ca[ (("ca") %gentext-ca-start-quote%) ]]>
    <![%l10n-cs[ (("cs") %gentext-cs-start-quote%) ]]>
    <![%l10n-da[ (("da") %gentext-da-start-quote%) ]]>
    <![%l10n-de[ (("de") %gentext-de-start-quote%) ]]>
    <![%l10n-el[ (("el") %gentext-el-start-quote%) ]]>
    <![%l10n-en[ (("en") %gentext-en-start-quote%) ]]>
    <![%l10n-es[ (("es") %gentext-es-start-quote%) ]]>
    <![%l10n-et[ (("et") %gentext-et-start-quote%) ]]>
    <![%l10n-fi[ (("fi") %gentext-fi-start-quote%) ]]>
    <![%l10n-fr[ (("fr") %gentext-fr-start-quote%) ]]>
    <![%l10n-it[ (("it") %gentext-it-start-quote%) ]]>
    <![%l10n-ja[ (("ja") %gentext-ja-start-quote%) ]]>
    <![%l10n-nl[ (("nl") %gentext-nl-start-quote%) ]]>
    <![%l10n-no[ (("no") %gentext-no-start-quote%) ]]>
    <![%l10n-pl[ (("pl") %gentext-pl-start-quote%) ]]>
    <![%l10n-pt[ (("pt") %gentext-pt-start-quote%) ]]>
    <![%l10n-pt-BR[ (("pt-br") %gentext-pt-BR-start-quote%) ]]>
    <![%l10n-ro[ (("ro") %gentext-ro-start-quote%) ]]>
    <![%l10n-ru[ (("ru") %gentext-ru-start-quote%) ]]>
    <![%l10n-sk[ (("sk") %gentext-sk-start-quote%) ]]>
    <![%l10n-sl[ (("sl") %gentext-sl-start-quote%) ]]>
    <![%l10n-sv[ (("sv") %gentext-sv-start-quote%) ]]>
    <![%l10n-zh-CN[ (("zh-cn") %gentext-zh-CN-start-quote%) ]]>

    <![%l10n-da[ (("dk")   %gentext-da-start-quote%) ]]>
    <![%l10n-de[ (("dege") %gentext-de-start-quote%) ]]>
    <![%l10n-en[ (("usen") %gentext-en-start-quote%) ]]>
    <![%l10n-no[ (("bmno") %gentext-no-start-quote%) ]]>
    <![%l10n-sv[ (("svse") %gentext-sv-start-quote%) ]]>
    (else (error "L10N ERROR: gentext-start-quote"))))

(define (gentext-end-quote)
  (case ($lang$)
    <![%l10n-ca[ (("ca") %gentext-ca-end-quote%) ]]>
    <![%l10n-cs[ (("cs") %gentext-cs-end-quote%) ]]>
    <![%l10n-da[ (("da") %gentext-da-end-quote%) ]]>
    <![%l10n-de[ (("de") %gentext-de-end-quote%) ]]>
    <![%l10n-el[ (("el") %gentext-el-end-quote%) ]]>
    <![%l10n-en[ (("en") %gentext-en-end-quote%) ]]>
    <![%l10n-es[ (("es") %gentext-es-end-quote%) ]]>
    <![%l10n-et[ (("et") %gentext-et-end-quote%) ]]>
    <![%l10n-fi[ (("fi") %gentext-fi-end-quote%) ]]>
    <![%l10n-fr[ (("fr") %gentext-fr-end-quote%) ]]>
    <![%l10n-it[ (("it") %gentext-it-end-quote%) ]]>
    <![%l10n-ja[ (("ja") %gentext-ja-end-quote%) ]]>
    <![%l10n-nl[ (("nl") %gentext-nl-end-quote%) ]]>
    <![%l10n-no[ (("no") %gentext-no-end-quote%) ]]>
    <![%l10n-pl[ (("pl") %gentext-pl-end-quote%) ]]>
    <![%l10n-pt[ (("pt") %gentext-pt-end-quote%) ]]>
    <![%l10n-pt-BR[ (("pt-br") %gentext-pt-BR-end-quote%) ]]>
    <![%l10n-ro[ (("ro") %gentext-ro-end-quote%) ]]>
    <![%l10n-ru[ (("ru") %gentext-ru-end-quote%) ]]>
    <![%l10n-sk[ (("sk") %gentext-sk-end-quote%) ]]>
    <![%l10n-sl[ (("sl") %gentext-sl-end-quote%) ]]>
    <![%l10n-sv[ (("sv") %gentext-sv-end-quote%) ]]>
    <![%l10n-zh-CN[ (("zh-cn") %gentext-zh-CN-end-quote%) ]]>

    <![%l10n-da[ (("dk")   %gentext-da-end-quote%) ]]>
    <![%l10n-de[ (("dege") %gentext-de-end-quote%) ]]>
    <![%l10n-en[ (("usen") %gentext-en-end-quote%) ]]>
    <![%l10n-no[ (("bmno") %gentext-no-end-quote%) ]]>
    <![%l10n-sv[ (("svse") %gentext-sv-end-quote%) ]]>
    (else (error "L10N ERROR: gentext-end-quote"))))

(define (gentext-start-nested-quote)
  (case ($lang$)
    <![%l10n-ca[ (("ca") %gentext-ca-start-nested-quote%) ]]>
    <![%l10n-cs[ (("cs") %gentext-cs-start-nested-quote%) ]]>
    <![%l10n-da[ (("da") %gentext-da-start-nested-quote%) ]]>
    <![%l10n-de[ (("de") %gentext-de-start-nested-quote%) ]]>
    <![%l10n-el[ (("el") %gentext-el-start-nested-quote%) ]]>
    <![%l10n-en[ (("en") %gentext-en-start-nested-quote%) ]]>
    <![%l10n-es[ (("es") %gentext-es-start-nested-quote%) ]]>
    <![%l10n-et[ (("et") %gentext-et-start-nested-quote%) ]]>
    <![%l10n-fi[ (("fi") %gentext-fi-start-nested-quote%) ]]>
    <![%l10n-fr[ (("fr") %gentext-fr-start-nested-quote%) ]]>
    <![%l10n-it[ (("it") %gentext-it-start-nested-quote%) ]]>
    <![%l10n-ja[ (("ja") %gentext-ja-start-nested-quote%) ]]>
    <![%l10n-nl[ (("nl") %gentext-nl-start-nested-quote%) ]]>
    <![%l10n-no[ (("no") %gentext-no-start-nested-quote%) ]]>
    <![%l10n-pl[ (("pl") %gentext-pl-start-nested-quote%) ]]>
    <![%l10n-pt[ (("pt") %gentext-pt-start-nested-quote%) ]]>
    <![%l10n-pt-BR[ (("pt-br") %gentext-pt-BR-start-nested-quote%) ]]>
    <![%l10n-ro[ (("ro") %gentext-ro-start-nested-quote%) ]]>
    <![%l10n-ru[ (("ru") %gentext-ru-start-nested-quote%) ]]>
    <![%l10n-sk[ (("sk") %gentext-sk-start-nested-quote%) ]]>
    <![%l10n-sl[ (("sl") %gentext-sl-start-nested-quote%) ]]>
    <![%l10n-sv[ (("sv") %gentext-sv-start-nested-quote%) ]]>
    <![%l10n-zh-CN[ (("zh-cn") %gentext-zh-CN-start-nested-quote%) ]]>
    (else (error "L10N ERROR: gentext-start-nested-quote"))))

(define (gentext-end-nested-quote)
  (case ($lang$)
    <![%l10n-ca[ (("ca") %gentext-ca-end-nested-quote%) ]]>
    <![%l10n-cs[ (("cs") %gentext-cs-end-nested-quote%) ]]>
    <![%l10n-da[ (("da") %gentext-da-end-nested-quote%) ]]>
    <![%l10n-de[ (("de") %gentext-de-end-nested-quote%) ]]>
    <![%l10n-el[ (("el") %gentext-el-end-nested-quote%) ]]>
    <![%l10n-en[ (("en") %gentext-en-end-nested-quote%) ]]>
    <![%l10n-es[ (("es") %gentext-es-end-nested-quote%) ]]>
    <![%l10n-et[ (("et") %gentext-et-end-nested-quote%) ]]>
    <![%l10n-fi[ (("fi") %gentext-fi-end-nested-quote%) ]]>
    <![%l10n-fr[ (("fr") %gentext-fr-end-nested-quote%) ]]>
    <![%l10n-it[ (("it") %gentext-it-end-nested-quote%) ]]>
    <![%l10n-ja[ (("ja") %gentext-ja-end-nested-quote%) ]]>
    <![%l10n-nl[ (("nl") %gentext-nl-end-nested-quote%) ]]>
    <![%l10n-no[ (("no") %gentext-no-end-nested-quote%) ]]>
    <![%l10n-pl[ (("pl") %gentext-pl-end-nested-quote%) ]]>
    <![%l10n-pt[ (("pt") %gentext-pt-end-nested-quote%) ]]>
    <![%l10n-pt-BR[ (("pt-br") %gentext-pt-BR-end-nested-quote%) ]]>
    <![%l10n-ro[ (("ro") %gentext-ro-end-nested-quote%) ]]>
    <![%l10n-ru[ (("ru") %gentext-ru-end-nested-quote%) ]]>
    <![%l10n-sk[ (("sk") %gentext-sk-end-nested-quote%) ]]>
    <![%l10n-sl[ (("sl") %gentext-sl-end-nested-quote%) ]]>
    <![%l10n-sv[ (("sv") %gentext-sv-end-nested-quote%) ]]>
    <![%l10n-zh-CN[ (("zh-cn") %gentext-zh-CN-end-nested-quote%) ]]>
    (else (error "L10N ERROR: gentext-end-nested-quote"))))

(define (gentext-by)
  (case ($lang$)
    <![%l10n-ca[ (("ca") %gentext-ca-by%) ]]>
    <![%l10n-cs[ (("cs") %gentext-cs-by%) ]]>
    <![%l10n-da[ (("da") %gentext-da-by%) ]]>
    <![%l10n-de[ (("de") %gentext-de-by%) ]]>
    <![%l10n-el[ (("el") %gentext-el-by%) ]]>
    <![%l10n-en[ (("en") %gentext-en-by%) ]]>
    <![%l10n-es[ (("es") %gentext-es-by%) ]]>
    <![%l10n-et[ (("et") %gentext-et-by%) ]]>
    <![%l10n-fi[ (("fi") %gentext-fi-by%) ]]>
    <![%l10n-fr[ (("fr") %gentext-fr-by%) ]]>
    <![%l10n-it[ (("it") %gentext-it-by%) ]]>
    <![%l10n-ja[ (("ja") %gentext-ja-by%) ]]>
    <![%l10n-nl[ (("nl") %gentext-nl-by%) ]]>
    <![%l10n-no[ (("no") %gentext-no-by%) ]]>
    <![%l10n-pl[ (("pl") %gentext-pl-by%) ]]>
    <![%l10n-pt[ (("pt") %gentext-pt-by%) ]]>
    <![%l10n-pt-BR[ (("pt-br") %gentext-pt-BR-by%) ]]>
    <![%l10n-ro[ (("ro") %gentext-ro-by%) ]]>
    <![%l10n-ru[ (("ru") %gentext-ru-by%) ]]>
    <![%l10n-sk[ (("sk") %gentext-sk-by%) ]]>
    <![%l10n-sl[ (("sl") %gentext-sl-by%) ]]>
    <![%l10n-sv[ (("sv") %gentext-sv-by%) ]]>
    <![%l10n-zh-CN[ (("zh-cn") %gentext-zh-CN-by%) ]]>

    <![%l10n-da[ (("dk")   %gentext-da-by%) ]]>
    <![%l10n-de[ (("dege") %gentext-de-by%) ]]>
    <![%l10n-en[ (("usen") %gentext-en-by%) ]]>
    <![%l10n-no[ (("bmno") %gentext-no-by%) ]]>
    <![%l10n-sv[ (("svse") %gentext-sv-by%) ]]>
    (else (error "L10N ERROR: gentext-by"))))

(define (gentext-edited-by)
  (case ($lang$)
    <![%l10n-ca[ (("ca") %gentext-ca-edited-by%) ]]>
    <![%l10n-cs[ (("cs") %gentext-cs-edited-by%) ]]>
    <![%l10n-da[ (("da") %gentext-da-edited-by%) ]]>
    <![%l10n-de[ (("de") %gentext-de-edited-by%) ]]>
    <![%l10n-el[ (("el") %gentext-el-edited-by%) ]]>
    <![%l10n-en[ (("en") %gentext-en-edited-by%) ]]>
    <![%l10n-es[ (("es") %gentext-es-edited-by%) ]]>
    <![%l10n-et[ (("et") %gentext-et-edited-by%) ]]>
    <![%l10n-fi[ (("fi") %gentext-fi-edited-by%) ]]>
    <![%l10n-fr[ (("fr") %gentext-fr-edited-by%) ]]>
    <![%l10n-it[ (("it") %gentext-it-edited-by%) ]]>
    <![%l10n-ja[ (("ja") %gentext-ja-edited-by%) ]]>
    <![%l10n-nl[ (("nl") %gentext-nl-edited-by%) ]]>
    <![%l10n-no[ (("no") %gentext-no-edited-by%) ]]>
    <![%l10n-pl[ (("pl") %gentext-pl-edited-by%) ]]>
    <![%l10n-pt[ (("pt") %gentext-pt-edited-by%) ]]>
    <![%l10n-pt-BR[ (("pt-br") %gentext-pt-BR-edited-by%) ]]>
    <![%l10n-ro[ (("ro") %gentext-ro-edited-by%) ]]>
    <![%l10n-ru[ (("ru") %gentext-ru-edited-by%) ]]>
    <![%l10n-sk[ (("sk") %gentext-sk-edited-by%) ]]>
    <![%l10n-sl[ (("sl") %gentext-sl-edited-by%) ]]>
    <![%l10n-sv[ (("sv") %gentext-sv-edited-by%) ]]>
    <![%l10n-zh-CN[ (("zh-cn") %gentext-zh-CN-edited-by%) ]]>

    <![%l10n-da[ (("dk")   %gentext-da-edited-by%) ]]>
    <![%l10n-de[ (("dege") %gentext-de-edited-by%) ]]>
    <![%l10n-en[ (("usen") %gentext-en-edited-by%) ]]>
    <![%l10n-no[ (("bmno") %gentext-no-edited-by%) ]]>
    <![%l10n-sv[ (("svse") %gentext-sv-edited-by%) ]]>
    (else (error "L10N ERROR: gentext-edited-by"))))

(define (gentext-revised-by)
  (case ($lang$)
    <![%l10n-ca[ (("ca") %gentext-ca-revised-by%) ]]>
    <![%l10n-cs[ (("cs") %gentext-cs-revised-by%) ]]>
    <![%l10n-da[ (("da") %gentext-da-revised-by%) ]]>
    <![%l10n-de[ (("de") %gentext-de-revised-by%) ]]>
    <![%l10n-el[ (("el") %gentext-el-revised-by%) ]]>
    <![%l10n-en[ (("en") %gentext-en-revised-by%) ]]>
    <![%l10n-es[ (("es") %gentext-es-revised-by%) ]]>
    <![%l10n-et[ (("et") %gentext-et-revised-by%) ]]>
    <![%l10n-fi[ (("fi") %gentext-fi-revised-by%) ]]>
    <![%l10n-fr[ (("fr") %gentext-fr-revised-by%) ]]>
    <![%l10n-it[ (("it") %gentext-it-revised-by%) ]]>
    <![%l10n-ja[ (("ja") %gentext-ja-revised-by%) ]]>
    <![%l10n-nl[ (("nl") %gentext-nl-revised-by%) ]]>
    <![%l10n-no[ (("no") %gentext-no-revised-by%) ]]>
    <![%l10n-pl[ (("pl") %gentext-pl-revised-by%) ]]>
    <![%l10n-pt[ (("pt") %gentext-pt-revised-by%) ]]>
    <![%l10n-pt-BR[ (("pt-br") %gentext-pt-BR-revised-by%) ]]>
    <![%l10n-ro[ (("ro") %gentext-ro-revised-by%) ]]>
    <![%l10n-ru[ (("ru") %gentext-ru-revised-by%) ]]>
    <![%l10n-sk[ (("sk") %gentext-sk-revised-by%) ]]>
    <![%l10n-sl[ (("sl") %gentext-sl-revised-by%) ]]>
    <![%l10n-sv[ (("sv") %gentext-sv-revised-by%) ]]>
    <![%l10n-zh-CN[ (("zh-cn") %gentext-zh-CN-revised-by%) ]]>

    <![%l10n-da[ (("dk")   %gentext-da-revised-by%) ]]>
    <![%l10n-de[ (("dege") %gentext-de-revised-by%) ]]>
    <![%l10n-en[ (("usen") %gentext-en-revised-by%) ]]>
    <![%l10n-no[ (("bmno") %gentext-no-revised-by%) ]]>
    <![%l10n-sv[ (("svse") %gentext-sv-revised-by%) ]]>
    (else (error "L10N ERROR: gentext-revised-by"))))

(define (gentext-page)
  (case ($lang$)
    <![%l10n-ca[ (("ca") %gentext-ca-page%) ]]>
    <![%l10n-cs[ (("cs") %gentext-cs-page%) ]]>
    <![%l10n-da[ (("da") %gentext-da-page%) ]]>
    <![%l10n-de[ (("de") %gentext-de-page%) ]]>
    <![%l10n-el[ (("el") %gentext-el-page%) ]]>
    <![%l10n-en[ (("en") %gentext-en-page%) ]]>
    <![%l10n-es[ (("es") %gentext-es-page%) ]]>
    <![%l10n-et[ (("et") %gentext-et-page%) ]]>
    <![%l10n-fi[ (("fi") %gentext-fi-page%) ]]>
    <![%l10n-fr[ (("fr") %gentext-fr-page%) ]]>
    <![%l10n-it[ (("it") %gentext-it-page%) ]]>
    <![%l10n-ja[ (("ja") %gentext-ja-page%) ]]>
    <![%l10n-nl[ (("nl") %gentext-nl-page%) ]]>
    <![%l10n-no[ (("no") %gentext-no-page%) ]]>
    <![%l10n-pl[ (("pl") %gentext-pl-page%) ]]>
    <![%l10n-pt[ (("pt") %gentext-pt-page%) ]]>
    <![%l10n-pt-BR[ (("pt-br") %gentext-pt-BR-page%) ]]>
    <![%l10n-ro[ (("ro") %gentext-ro-page%) ]]>
    <![%l10n-ru[ (("ru") %gentext-ru-page%) ]]>
    <![%l10n-sk[ (("sk") %gentext-sk-page%) ]]>
    <![%l10n-sl[ (("sl") %gentext-sl-page%) ]]>
    <![%l10n-sv[ (("sv") %gentext-sv-page%) ]]>
    <![%l10n-zh-CN[ (("zh-cn") %gentext-zh-CN-page%) ]]>

    <![%l10n-da[ (("dk")   %gentext-da-page%) ]]>
    <![%l10n-de[ (("dege") %gentext-de-page%) ]]>
    <![%l10n-en[ (("usen") %gentext-en-page%) ]]>
    <![%l10n-no[ (("bmno") %gentext-no-page%) ]]>
    <![%l10n-sv[ (("svse") %gentext-sv-page%) ]]>
    (else (error "L10N ERROR: gentext-page"))))

(define (gentext-and)
  (case ($lang$)
    <![%l10n-ca[ (("ca") %gentext-ca-and%) ]]>
    <![%l10n-cs[ (("cs") %gentext-cs-and%) ]]>
    <![%l10n-da[ (("da") %gentext-da-and%) ]]>
    <![%l10n-de[ (("de") %gentext-de-and%) ]]>
    <![%l10n-el[ (("el") %gentext-el-and%) ]]>
    <![%l10n-en[ (("en") %gentext-en-and%) ]]>
    <![%l10n-es[ (("es") %gentext-es-and%) ]]>
    <![%l10n-et[ (("et") %gentext-et-and%) ]]>
    <![%l10n-fi[ (("fi") %gentext-fi-and%) ]]>
    <![%l10n-fr[ (("fr") %gentext-fr-and%) ]]>
    <![%l10n-it[ (("it") %gentext-it-and%) ]]>
    <![%l10n-ja[ (("ja") %gentext-ja-and%) ]]>
    <![%l10n-nl[ (("nl") %gentext-nl-and%) ]]>
    <![%l10n-no[ (("no") %gentext-no-and%) ]]>
    <![%l10n-pl[ (("pl") %gentext-pl-and%) ]]>
    <![%l10n-pt[ (("pt") %gentext-pt-and%) ]]>
    <![%l10n-pt-BR[ (("pt-br") %gentext-pt-BR-and%) ]]>
    <![%l10n-ro[ (("ro") %gentext-ro-and%) ]]>
    <![%l10n-ru[ (("ru") %gentext-ru-and%) ]]>
    <![%l10n-sk[ (("sk") %gentext-sk-and%) ]]>
    <![%l10n-sl[ (("sl") %gentext-sl-and%) ]]>
    <![%l10n-sv[ (("sv") %gentext-sv-and%) ]]>
    <![%l10n-zh-CN[ (("zh-cn") %gentext-zh-CN-and%) ]]>

    <![%l10n-da[ (("dk")   %gentext-da-and%) ]]>
    <![%l10n-de[ (("dege") %gentext-de-and%) ]]>
    <![%l10n-en[ (("usen") %gentext-en-and%) ]]>
    <![%l10n-no[ (("bmno") %gentext-no-and%) ]]>
    <![%l10n-sv[ (("svse") %gentext-sv-and%) ]]>
    (else (error "L10N ERROR: gentext-and"))))

(define (gentext-bibl-pages)
  (case ($lang$)
    <![%l10n-ca[ (("ca") %gentext-ca-bibl-pages%) ]]>
    <![%l10n-cs[ (("cs") %gentext-cs-bibl-pages%) ]]>
    <![%l10n-da[ (("da") %gentext-da-bibl-pages%) ]]>
    <![%l10n-de[ (("de") %gentext-de-bibl-pages%) ]]>
    <![%l10n-el[ (("el") %gentext-el-bibl-pages%) ]]>
    <![%l10n-en[ (("en") %gentext-en-bibl-pages%) ]]>
    <![%l10n-es[ (("es") %gentext-es-bibl-pages%) ]]>
    <![%l10n-et[ (("et") %gentext-et-bibl-pages%) ]]>
    <![%l10n-fi[ (("fi") %gentext-fi-bibl-pages%) ]]>
    <![%l10n-fr[ (("fr") %gentext-fr-bibl-pages%) ]]>
    <![%l10n-it[ (("it") %gentext-it-bibl-pages%) ]]>
    <![%l10n-ja[ (("ja") %gentext-ja-bibl-pages%) ]]>
    <![%l10n-nl[ (("nl") %gentext-nl-bibl-pages%) ]]>
    <![%l10n-no[ (("no") %gentext-no-bibl-pages%) ]]>
    <![%l10n-pl[ (("pl") %gentext-pl-bibl-pages%) ]]>
    <![%l10n-pt[ (("pt") %gentext-pt-bibl-pages%) ]]>
    <![%l10n-pt-BR[ (("pt-br") %gentext-pt-BR-bibl-pages%) ]]>
    <![%l10n-ro[ (("ro") %gentext-ro-bibl-pages%) ]]>
    <![%l10n-ru[ (("ru") %gentext-ru-bibl-pages%) ]]>
    <![%l10n-sk[ (("sk") %gentext-sk-bibl-pages%) ]]>
    <![%l10n-sl[ (("sl") %gentext-sl-bibl-pages%) ]]>
    <![%l10n-sv[ (("sv") %gentext-sv-bibl-pages%) ]]>
    <![%l10n-zh-CN[ (("zh-cn") %gentext-zh-CN-bibl-pages%) ]]>

    <![%l10n-da[ (("dk")   %gentext-da-bibl-pages%) ]]>
    <![%l10n-de[ (("dege") %gentext-de-bibl-pages%) ]]>
    <![%l10n-en[ (("usen") %gentext-en-bibl-pages%) ]]>
    <![%l10n-no[ (("bmno") %gentext-no-bibl-pages%) ]]>
    <![%l10n-sv[ (("svse") %gentext-sv-bibl-pages%) ]]>
    (else (error "L10N ERROR: gentext-bibl-pages"))))

(define (gentext-endnotes)
  (case ($lang$)
    <![%l10n-ca[ (("ca") %gentext-ca-endnotes%) ]]>
    <![%l10n-cs[ (("cs") %gentext-cs-endnotes%) ]]>
    <![%l10n-da[ (("da") %gentext-da-endnotes%) ]]>
    <![%l10n-de[ (("de") %gentext-de-endnotes%) ]]>
    <![%l10n-el[ (("el") %gentext-el-endnotes%) ]]>
    <![%l10n-en[ (("en") %gentext-en-endnotes%) ]]>
    <![%l10n-es[ (("es") %gentext-es-endnotes%) ]]>
    <![%l10n-et[ (("et") %gentext-et-endnotes%) ]]>
    <![%l10n-fi[ (("fi") %gentext-fi-endnotes%) ]]>
    <![%l10n-fr[ (("fr") %gentext-fr-endnotes%) ]]>
    <![%l10n-it[ (("it") %gentext-it-endnotes%) ]]>
    <![%l10n-ja[ (("ja") %gentext-ja-endnotes%) ]]>
    <![%l10n-nl[ (("nl") %gentext-nl-endnotes%) ]]>
    <![%l10n-no[ (("no") %gentext-no-endnotes%) ]]>
    <![%l10n-pl[ (("pl") %gentext-pl-endnotes%) ]]>
    <![%l10n-pt[ (("pt") %gentext-pt-endnotes%) ]]>
    <![%l10n-pt-BR[ (("pt-br") %gentext-pt-BR-endnotes%) ]]>
    <![%l10n-ro[ (("ro") %gentext-ro-endnotes%) ]]>
    <![%l10n-ru[ (("ru") %gentext-ru-endnotes%) ]]>
    <![%l10n-sk[ (("sk") %gentext-sk-endnotes%) ]]>
    <![%l10n-sl[ (("sl") %gentext-sl-endnotes%) ]]>
    <![%l10n-sv[ (("sv") %gentext-sv-endnotes%) ]]>
    <![%l10n-zh-CN[ (("zh-cn") %gentext-zh-CN-endnotes%) ]]>

    <![%l10n-da[ (("dk")   %gentext-da-endnotes%) ]]>
    <![%l10n-de[ (("dege") %gentext-de-endnotes%) ]]>
    <![%l10n-en[ (("usen") %gentext-en-endnotes%) ]]>
    <![%l10n-no[ (("bmno") %gentext-no-endnotes%) ]]>
    <![%l10n-sv[ (("svse") %gentext-sv-endnotes%) ]]>
    (else (error "L10N ERROR: gentext-endnotes"))))

(define (gentext-table-endnotes)
  (case ($lang$)
    <![%l10n-ca[ (("ca") %gentext-ca-table-endnotes%) ]]>
    <![%l10n-cs[ (("cs") %gentext-cs-table-endnotes%) ]]>
    <![%l10n-da[ (("da") %gentext-da-table-endnotes%) ]]>
    <![%l10n-de[ (("de") %gentext-de-table-endnotes%) ]]>
    <![%l10n-el[ (("el") %gentext-el-table-endnotes%) ]]>
    <![%l10n-en[ (("en") %gentext-en-table-endnotes%) ]]>
    <![%l10n-es[ (("es") %gentext-es-table-endnotes%) ]]>
    <![%l10n-et[ (("et") %gentext-et-table-endnotes%) ]]>
    <![%l10n-fi[ (("fi") %gentext-fi-table-endnotes%) ]]>
    <![%l10n-fr[ (("fr") %gentext-fr-table-endnotes%) ]]>
    <![%l10n-it[ (("it") %gentext-it-table-endnotes%) ]]>
    <![%l10n-ja[ (("ja") %gentext-ja-table-endnotes%) ]]>
    <![%l10n-nl[ (("nl") %gentext-nl-table-endnotes%) ]]>
    <![%l10n-no[ (("no") %gentext-no-table-endnotes%) ]]>
    <![%l10n-pl[ (("pl") %gentext-pl-table-endnotes%) ]]>
    <![%l10n-pt[ (("pt") %gentext-pt-table-endnotes%) ]]>
    <![%l10n-pt-BR[ (("pt-br") %gentext-pt-BR-table-endnotes%) ]]>
    <![%l10n-ro[ (("ro") %gentext-ro-table-endnotes%) ]]>
    <![%l10n-ru[ (("ru") %gentext-ru-table-endnotes%) ]]>
    <![%l10n-sk[ (("sk") %gentext-sk-table-endnotes%) ]]>
    <![%l10n-sl[ (("sl") %gentext-sl-table-endnotes%) ]]>
    <![%l10n-sv[ (("sv") %gentext-sv-table-endnotes%) ]]>
    <![%l10n-zh-CN[ (("zh-cn") %gentext-zh-CN-table-endnotes%) ]]>

    <![%l10n-da[ (("dk")   %gentext-da-table-endnotes%) ]]>
    <![%l10n-de[ (("dege") %gentext-de-table-endnotes%) ]]>
    <![%l10n-en[ (("usen") %gentext-en-table-endnotes%) ]]>
    <![%l10n-no[ (("bmno") %gentext-no-table-endnotes%) ]]>
    <![%l10n-sv[ (("svse") %gentext-sv-table-endnotes%) ]]>
    (else (error "L10N ERROR: gentext-table-endnotes"))))

(define (gentext-index-see)
  (case ($lang$)
    <![%l10n-ca[ (("ca") %gentext-ca-index-see%) ]]>
    <![%l10n-cs[ (("cs") %gentext-cs-index-see%) ]]>
    <![%l10n-da[ (("da") %gentext-da-index-see%) ]]>
    <![%l10n-de[ (("de") %gentext-de-index-see%) ]]>
    <![%l10n-el[ (("el") %gentext-el-index-see%) ]]>
    <![%l10n-en[ (("en") %gentext-en-index-see%) ]]>
    <![%l10n-es[ (("es") %gentext-es-index-see%) ]]>
    <![%l10n-et[ (("et") %gentext-et-index-see%) ]]>
    <![%l10n-fi[ (("fi") %gentext-fi-index-see%) ]]>
    <![%l10n-fr[ (("fr") %gentext-fr-index-see%) ]]>
    <![%l10n-it[ (("it") %gentext-it-index-see%) ]]>
    <![%l10n-ja[ (("ja") %gentext-ja-index-see%) ]]>
    <![%l10n-nl[ (("nl") %gentext-nl-index-see%) ]]>
    <![%l10n-no[ (("no") %gentext-no-index-see%) ]]>
    <![%l10n-pl[ (("pl") %gentext-pl-index-see%) ]]>
    <![%l10n-pt[ (("pt") %gentext-pt-index-see%) ]]>
    <![%l10n-pt-BR[ (("pt-br") %gentext-pt-BR-index-see%) ]]>
    <![%l10n-ro[ (("ro") %gentext-ro-index-see%) ]]>
    <![%l10n-ru[ (("ru") %gentext-ru-index-see%) ]]>
    <![%l10n-sk[ (("sk") %gentext-sk-index-see%) ]]>
    <![%l10n-sl[ (("sl") %gentext-sl-index-see%) ]]>
    <![%l10n-sv[ (("sv") %gentext-sv-index-see%) ]]>
    <![%l10n-zh-CN[ (("zh-cn") %gentext-zh-CN-index-see%) ]]>

    <![%l10n-da[ (("dk")   %gentext-da-index-see%) ]]>
    <![%l10n-de[ (("dege") %gentext-de-index-see%) ]]>
    <![%l10n-en[ (("usen") %gentext-en-index-see%) ]]>
    <![%l10n-no[ (("bmno") %gentext-no-index-see%) ]]>
    <![%l10n-sv[ (("svse") %gentext-sv-index-see%) ]]>
    (else (error "L10N ERROR: gentext-index-see"))))

(define (gentext-index-seealso)
  (case ($lang$)
    <![%l10n-ca[ (("ca") %gentext-ca-index-seealso%) ]]>
    <![%l10n-cs[ (("cs") %gentext-cs-index-seealso%) ]]>
    <![%l10n-da[ (("da") %gentext-da-index-seealso%) ]]>
    <![%l10n-de[ (("de") %gentext-de-index-seealso%) ]]>
    <![%l10n-el[ (("el") %gentext-el-index-seealso%) ]]>
    <![%l10n-en[ (("en") %gentext-en-index-seealso%) ]]>
    <![%l10n-es[ (("es") %gentext-es-index-seealso%) ]]>
    <![%l10n-et[ (("et") %gentext-et-index-seealso%) ]]>
    <![%l10n-fi[ (("fi") %gentext-fi-index-seealso%) ]]>
    <![%l10n-fr[ (("fr") %gentext-fr-index-seealso%) ]]>
    <![%l10n-it[ (("it") %gentext-it-index-seealso%) ]]>
    <![%l10n-ja[ (("ja") %gentext-ja-index-seealso%) ]]>
    <![%l10n-nl[ (("nl") %gentext-nl-index-seealso%) ]]>
    <![%l10n-no[ (("no") %gentext-no-index-seealso%) ]]>
    <![%l10n-pl[ (("pl") %gentext-pl-index-seealso%) ]]>
    <![%l10n-pt[ (("pt") %gentext-pt-index-seealso%) ]]>
    <![%l10n-pt-BR[ (("pt-br") %gentext-pt-BR-index-seealso%) ]]>
    <![%l10n-ro[ (("ro") %gentext-ro-index-seealso%) ]]>
    <![%l10n-ru[ (("ru") %gentext-ru-index-seealso%) ]]>
    <![%l10n-sk[ (("sk") %gentext-sk-index-seealso%) ]]>
    <![%l10n-sl[ (("sl") %gentext-sl-index-seealso%) ]]>
    <![%l10n-sv[ (("sv") %gentext-sv-index-seealso%) ]]>
    <![%l10n-zh-CN[ (("zh-cn") %gentext-zh-CN-index-seealso%) ]]>

    <![%l10n-da[ (("dk")   %gentext-da-index-seealso%) ]]>
    <![%l10n-de[ (("dege") %gentext-de-index-seealso%) ]]>
    <![%l10n-en[ (("usen") %gentext-en-index-seealso%) ]]>
    <![%l10n-no[ (("bmno") %gentext-no-index-seealso%) ]]>
    <![%l10n-sv[ (("svse") %gentext-sv-index-seealso%) ]]>
    (else (error "L10N ERROR: gentext-index-seealso"))))

(define (gentext-nav-prev prev) 
  (case ($lang$)
    <![%l10n-ca[ (("ca") (gentext-ca-nav-prev prev)) ]]>
    <![%l10n-cs[ (("cs") (gentext-cs-nav-prev prev)) ]]>
    <![%l10n-da[ (("da") (gentext-da-nav-prev prev)) ]]>
    <![%l10n-de[ (("de") (gentext-de-nav-prev prev)) ]]>
    <![%l10n-el[ (("el") (gentext-el-nav-prev prev)) ]]>
    <![%l10n-en[ (("en") (gentext-en-nav-prev prev)) ]]>
    <![%l10n-es[ (("es") (gentext-es-nav-prev prev)) ]]>
    <![%l10n-et[ (("et") (gentext-et-nav-prev prev)) ]]>
    <![%l10n-fi[ (("fi") (gentext-fi-nav-prev prev)) ]]>
    <![%l10n-fr[ (("fr") (gentext-fr-nav-prev prev)) ]]>
    <![%l10n-hu[ (("hu") (gentext-hu-nav-prev prev)) ]]>
    <![%l10n-it[ (("it") (gentext-it-nav-prev prev)) ]]>
    <![%l10n-ja[ (("ja") (gentext-ja-nav-prev prev)) ]]>
    <![%l10n-nl[ (("nl") (gentext-nl-nav-prev prev)) ]]>
    <![%l10n-no[ (("no") (gentext-no-nav-prev prev)) ]]>
    <![%l10n-pl[ (("pl") (gentext-pl-nav-prev prev)) ]]>
    <![%l10n-pt[ (("pt") (gentext-pt-nav-prev prev)) ]]>
    <![%l10n-pt-BR[ (("pt-br") (gentext-pt-BR-nav-prev prev)) ]]>
    <![%l10n-ro[ (("ro") (gentext-ro-nav-prev prev)) ]]>
    <![%l10n-ru[ (("ru") (gentext-ru-nav-prev prev)) ]]>
    <![%l10n-sk[ (("sk") (gentext-sk-nav-prev prev)) ]]>
    <![%l10n-sl[ (("sl") (gentext-sl-nav-prev prev)) ]]>
    <![%l10n-sv[ (("sv") (gentext-sv-nav-prev prev)) ]]>
    <![%l10n-zh-CN[ (("zh-cn") (gentext-zh-CN-nav-prev prev)) ]]>

    <![%l10n-da[ (("dk")   (gentext-da-nav-prev prev)) ]]>
    <![%l10n-de[ (("dege") (gentext-de-nav-prev prev)) ]]>
    <![%l10n-en[ (("usen") (gentext-en-nav-prev prev)) ]]>
    <![%l10n-no[ (("bmno") (gentext-no-nav-prev prev)) ]]>
    <![%l10n-sv[ (("svse") (gentext-sv-nav-prev prev)) ]]>
    (else (error "L10N ERROR: gentext-nav-prev"))))

(define (gentext-nav-prev-sibling prevsib) 
  (case ($lang$)
    <![%l10n-ca[ (("ca") (gentext-ca-nav-prev-sibling prevsib)) ]]>
    <![%l10n-cs[ (("cs") (gentext-cs-nav-prev-sibling prevsib)) ]]>
    <![%l10n-da[ (("da") (gentext-da-nav-prev-sibling prevsib)) ]]>
    <![%l10n-de[ (("de") (gentext-de-nav-prev-sibling prevsib)) ]]>
    <![%l10n-el[ (("el") (gentext-el-nav-prev-sibling prevsib)) ]]>
    <![%l10n-en[ (("en") (gentext-en-nav-prev-sibling prevsib)) ]]>
    <![%l10n-es[ (("es") (gentext-es-nav-prev-sibling prevsib)) ]]>
    <![%l10n-et[ (("et") (gentext-et-nav-prev-sibling prevsib)) ]]>
    <![%l10n-fi[ (("fi") (gentext-fi-nav-prev-sibling prevsib)) ]]>
    <![%l10n-fr[ (("fr") (gentext-fr-nav-prev-sibling prevsib)) ]]>
    <![%l10n-hu[ (("hu") (gentext-hu-nav-prev-sibling prevsib)) ]]>
    <![%l10n-it[ (("it") (gentext-it-nav-prev-sibling prevsib)) ]]>
    <![%l10n-ja[ (("ja") (gentext-ja-nav-prev-sibling prevsib)) ]]>
    <![%l10n-nl[ (("nl") (gentext-nl-nav-prev-sibling prevsib)) ]]>
    <![%l10n-no[ (("no") (gentext-no-nav-prev-sibling prevsib)) ]]>
    <![%l10n-pl[ (("pl") (gentext-pl-nav-prev-sibling prevsib)) ]]>
    <![%l10n-pt[ (("pt") (gentext-pt-nav-prev-sibling prevsib)) ]]>
    <![%l10n-pt-BR[ (("pt-br") (gentext-pt-BR-nav-prev-sibling prevsib)) ]]>
    <![%l10n-ro[ (("ro") (gentext-ro-nav-prev-sibling prevsib)) ]]>
    <![%l10n-ru[ (("ru") (gentext-ru-nav-prev-sibling prevsib)) ]]>
    <![%l10n-sk[ (("sk") (gentext-sk-nav-prev-sibling prevsib)) ]]>
    <![%l10n-sl[ (("sl") (gentext-sl-nav-prev-sibling prevsib)) ]]>
    <![%l10n-sv[ (("sv") (gentext-sv-nav-prev-sibling prevsib)) ]]>
    <![%l10n-zh-CN[ (("zh-cn") (gentext-zh-CN-nav-prev-sibling prevsib)) ]]>

    <![%l10n-da[ (("dk")   (gentext-da-nav-prev-sibling prevsib)) ]]>
    <![%l10n-de[ (("dege") (gentext-de-nav-prev-sibling prevsib)) ]]>
    <![%l10n-en[ (("usen") (gentext-en-nav-prev-sibling prevsib)) ]]>
    <![%l10n-no[ (("bmno") (gentext-no-nav-prev-sibling prevsib)) ]]>
    <![%l10n-sv[ (("svse") (gentext-sv-nav-prev-sibling prevsib)) ]]>
    (else (error "L10N ERROR: gentext-nav-prev-sibling "))))

(define (gentext-nav-next-sibling nextsib)
  (case ($lang$)
    <![%l10n-ca[ (("ca") (gentext-ca-nav-next-sibling nextsib)) ]]>
    <![%l10n-cs[ (("cs") (gentext-cs-nav-next-sibling nextsib)) ]]>
    <![%l10n-da[ (("da") (gentext-da-nav-next-sibling nextsib)) ]]>
    <![%l10n-de[ (("de") (gentext-de-nav-next-sibling nextsib)) ]]>
    <![%l10n-el[ (("el") (gentext-el-nav-next-sibling nextsib)) ]]>
    <![%l10n-en[ (("en") (gentext-en-nav-next-sibling nextsib)) ]]>
    <![%l10n-es[ (("es") (gentext-es-nav-next-sibling nextsib)) ]]>
    <![%l10n-et[ (("et") (gentext-et-nav-next-sibling nextsib)) ]]>
    <![%l10n-fi[ (("fi") (gentext-fi-nav-next-sibling nextsib)) ]]>
    <![%l10n-fr[ (("fr") (gentext-fr-nav-next-sibling nextsib)) ]]>
    <![%l10n-hu[ (("hu") (gentext-hu-nav-next-sibling nextsib)) ]]>
    <![%l10n-it[ (("it") (gentext-it-nav-next-sibling nextsib)) ]]>
    <![%l10n-ja[ (("ja") (gentext-ja-nav-next-sibling nextsib)) ]]>
    <![%l10n-nl[ (("nl") (gentext-nl-nav-next-sibling nextsib)) ]]>
    <![%l10n-no[ (("no") (gentext-no-nav-next-sibling nextsib)) ]]>
    <![%l10n-pl[ (("pl") (gentext-pl-nav-next-sibling nextsib)) ]]>
    <![%l10n-pt[ (("pt") (gentext-pt-nav-next-sibling nextsib)) ]]>
    <![%l10n-pt-BR[ (("pt-br") (gentext-pt-BR-nav-next-sibling nextsib)) ]]>
    <![%l10n-ro[ (("ro") (gentext-ro-nav-next-sibling nextsib)) ]]>
    <![%l10n-ru[ (("ru") (gentext-ru-nav-next-sibling nextsib)) ]]>
    <![%l10n-sk[ (("sk") (gentext-sk-nav-next-sibling nextsib)) ]]>
    <![%l10n-sl[ (("sl") (gentext-sl-nav-next-sibling nextsib)) ]]>
    <![%l10n-sv[ (("sv") (gentext-sv-nav-next-sibling nextsib)) ]]>
    <![%l10n-zh-CN[ (("zh-cn") (gentext-zh-CN-nav-next-sibling nextsib)) ]]>

    <![%l10n-da[ (("dk")   (gentext-da-nav-next-sibling nextsib)) ]]>
    <![%l10n-de[ (("dege") (gentext-de-nav-next-sibling nextsib)) ]]>
    <![%l10n-en[ (("usen") (gentext-en-nav-next-sibling nextsib)) ]]>
    <![%l10n-no[ (("bmno") (gentext-no-nav-next-sibling nextsib)) ]]>
    <![%l10n-sv[ (("svse") (gentext-sv-nav-next-sibling nextsib)) ]]>
    (else (error "L10N ERROR: gentext-nav-next-sibling"))))

(define (gentext-nav-next next)
  (case ($lang$)
    <![%l10n-ca[ (("ca") (gentext-ca-nav-next next)) ]]>
    <![%l10n-cs[ (("cs") (gentext-cs-nav-next next)) ]]>
    <![%l10n-da[ (("da") (gentext-da-nav-next next)) ]]>
    <![%l10n-de[ (("de") (gentext-de-nav-next next)) ]]>
    <![%l10n-el[ (("el") (gentext-el-nav-next next)) ]]>
    <![%l10n-en[ (("en") (gentext-en-nav-next next)) ]]>
    <![%l10n-es[ (("es") (gentext-es-nav-next next)) ]]>
    <![%l10n-et[ (("et") (gentext-et-nav-next next)) ]]>
    <![%l10n-fi[ (("fi") (gentext-fi-nav-next next)) ]]>
    <![%l10n-fr[ (("fr") (gentext-fr-nav-next next)) ]]>
    <![%l10n-hu[ (("hu") (gentext-hu-nav-next next)) ]]>
    <![%l10n-it[ (("it") (gentext-it-nav-next next)) ]]>
    <![%l10n-ja[ (("ja") (gentext-ja-nav-next next)) ]]>
    <![%l10n-nl[ (("nl") (gentext-nl-nav-next next)) ]]>
    <![%l10n-no[ (("no") (gentext-no-nav-next next)) ]]>
    <![%l10n-pl[ (("pl") (gentext-pl-nav-next next)) ]]>
    <![%l10n-pt[ (("pt") (gentext-pt-nav-next next)) ]]>
    <![%l10n-pt-BR[ (("pt-br") (gentext-pt-BR-nav-next next)) ]]>
    <![%l10n-ro[ (("ro") (gentext-ro-nav-next next)) ]]>
    <![%l10n-ru[ (("ru") (gentext-ru-nav-next next)) ]]>
    <![%l10n-sk[ (("sk") (gentext-sk-nav-next next)) ]]>
    <![%l10n-sl[ (("sl") (gentext-sl-nav-next next)) ]]>
    <![%l10n-sv[ (("sv") (gentext-sv-nav-next next)) ]]>
    <![%l10n-zh-CN[ (("zh-cn") (gentext-zh-CN-nav-next next)) ]]>

    <![%l10n-da[ (("dk")   (gentext-da-nav-next next)) ]]>
    <![%l10n-de[ (("dege") (gentext-de-nav-next next)) ]]>
    <![%l10n-en[ (("usen") (gentext-en-nav-next next)) ]]>
    <![%l10n-no[ (("bmno") (gentext-no-nav-next next)) ]]>
    <![%l10n-sv[ (("svse") (gentext-sv-nav-next next)) ]]>
    (else (error "L10N ERROR: gentext-nav-next"))))

(define (gentext-nav-up up)
  (case ($lang$)
    <![%l10n-ca[ (("ca") (gentext-ca-nav-up up)) ]]>
    <![%l10n-cs[ (("cs") (gentext-cs-nav-up up)) ]]>
    <![%l10n-da[ (("da") (gentext-da-nav-up up)) ]]>
    <![%l10n-de[ (("de") (gentext-de-nav-up up)) ]]>
    <![%l10n-el[ (("el") (gentext-el-nav-up up)) ]]>
    <![%l10n-en[ (("en") (gentext-en-nav-up up)) ]]>
    <![%l10n-es[ (("es") (gentext-es-nav-up up)) ]]>
    <![%l10n-et[ (("et") (gentext-et-nav-up up)) ]]>
    <![%l10n-fi[ (("fi") (gentext-fi-nav-up up)) ]]>
    <![%l10n-fr[ (("fr") (gentext-fr-nav-up up)) ]]>
    <![%l10n-hu[ (("hu") (gentext-hu-nav-up up)) ]]>
    <![%l10n-it[ (("it") (gentext-it-nav-up up)) ]]>
    <![%l10n-ja[ (("ja") (gentext-ja-nav-up up)) ]]>
    <![%l10n-nl[ (("nl") (gentext-nl-nav-up up)) ]]>
    <![%l10n-no[ (("no") (gentext-no-nav-up up)) ]]>
    <![%l10n-pl[ (("pl") (gentext-pl-nav-up up)) ]]>
    <![%l10n-pt[ (("pt") (gentext-pt-nav-up up)) ]]>
    <![%l10n-pt-BR[ (("pt-br") (gentext-pt-BR-nav-up up)) ]]>
    <![%l10n-ro[ (("ro") (gentext-ro-nav-up up)) ]]>
    <![%l10n-ru[ (("ru") (gentext-ru-nav-up up)) ]]>
    <![%l10n-sk[ (("sk") (gentext-sk-nav-up up)) ]]>
    <![%l10n-sl[ (("sl") (gentext-sl-nav-up up)) ]]>
    <![%l10n-sv[ (("sv") (gentext-sv-nav-up up)) ]]>
    <![%l10n-zh-CN[ (("zh-cn") (gentext-zh-CN-nav-up up)) ]]>

    <![%l10n-da[ (("dk")   (gentext-da-nav-up up)) ]]>
    <![%l10n-de[ (("dege") (gentext-de-nav-up up)) ]]>
    <![%l10n-en[ (("usen") (gentext-en-nav-up up)) ]]>
    <![%l10n-no[ (("bmno") (gentext-no-nav-up up)) ]]>
    <![%l10n-sv[ (("svse") (gentext-sv-nav-up up)) ]]>
    (else (error "L10N ERROR: gentext-nav-up"))))

(define (gentext-nav-home home)
  (case ($lang$)
    <![%l10n-ca[ (("ca") (gentext-ca-nav-home home)) ]]>
    <![%l10n-cs[ (("cs") (gentext-cs-nav-home home)) ]]>
    <![%l10n-da[ (("da") (gentext-da-nav-home home)) ]]>
    <![%l10n-de[ (("de") (gentext-de-nav-home home)) ]]>
    <![%l10n-el[ (("el") (gentext-el-nav-home home)) ]]>
    <![%l10n-en[ (("en") (gentext-en-nav-home home)) ]]>
    <![%l10n-es[ (("es") (gentext-es-nav-home home)) ]]>
    <![%l10n-et[ (("et") (gentext-et-nav-home home)) ]]>
    <![%l10n-fi[ (("fi") (gentext-fi-nav-home home)) ]]>
    <![%l10n-fr[ (("fr") (gentext-fr-nav-home home)) ]]>
    <![%l10n-hu[ (("hu") (gentext-hu-nav-home home)) ]]>
    <![%l10n-it[ (("it") (gentext-it-nav-home home)) ]]>
    <![%l10n-ja[ (("ja") (gentext-ja-nav-home home)) ]]>
    <![%l10n-nl[ (("nl") (gentext-nl-nav-home home)) ]]>
    <![%l10n-no[ (("no") (gentext-no-nav-home home)) ]]>
    <![%l10n-pl[ (("pl") (gentext-pl-nav-home home)) ]]>
    <![%l10n-pt[ (("pt") (gentext-pt-nav-home home)) ]]>
    <![%l10n-pt-BR[ (("pt-br") (gentext-pt-BR-nav-home home)) ]]>
    <![%l10n-ro[ (("ro") (gentext-ro-nav-home home)) ]]>
    <![%l10n-ru[ (("ru") (gentext-ru-nav-home home)) ]]>
    <![%l10n-sk[ (("sk") (gentext-sk-nav-home home)) ]]>
    <![%l10n-sl[ (("sl") (gentext-sl-nav-home home)) ]]>
    <![%l10n-sv[ (("sv") (gentext-sv-nav-home home)) ]]>
    <![%l10n-zh-CN[ (("zh-cn") (gentext-zh-CN-nav-home home)) ]]>

    <![%l10n-da[ (("dk")   (gentext-da-nav-home home)) ]]>
    <![%l10n-de[ (("dege") (gentext-de-nav-home home)) ]]>
    <![%l10n-en[ (("usen") (gentext-en-nav-home home)) ]]>
    <![%l10n-no[ (("bmno") (gentext-no-nav-home home)) ]]>
    <![%l10n-sv[ (("svse") (gentext-sv-nav-home home)) ]]>
    (else (error "L10N ERROR: gentext-nav-home"))))
