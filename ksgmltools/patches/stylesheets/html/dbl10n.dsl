;;
;; This file is part of the Modular DocBook Stylesheet distribution.
;; See ../README or http://nwalsh.com/docbook/dsssl.
;; See also ../common/dbl10n.dsl
;;

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

    <![%l10n-da[ (("dk")   (gentext-da-nav-prev prev)) ]]>
    <![%l10n-de[ (("dege") (gentext-de-nav-prev prev)) ]]>
    <![%l10n-en[ (("usen") (gentext-en-nav-prev prev)) ]]>
    <![%l10n-no[ (("bmno") (gentext-no-nav-prev prev)) ]]>
    <![%l10n-sv[ (("svse") (gentext-sv-nav-prev prev)) ]]>
    (else (error "L10N ERROR: gentext-nav-prev"))))

(define (gentext-nav-prevsib prevsib) 
  (case ($lang$)
    <![%l10n-ca[ (("ca") (gentext-ca-nav-prevsib prevsib)) ]]>
    <![%l10n-cs[ (("cs") (gentext-cs-nav-prevsib prevsib)) ]]>
    <![%l10n-da[ (("da") (gentext-da-nav-prevsib prevsib)) ]]>
    <![%l10n-de[ (("de") (gentext-de-nav-prevsib prevsib)) ]]>
    <![%l10n-el[ (("el") (gentext-el-nav-prevsib prevsib)) ]]>
    <![%l10n-en[ (("en") (gentext-en-nav-prevsib prevsib)) ]]>
    <![%l10n-es[ (("es") (gentext-es-nav-prevsib prevsib)) ]]>
    <![%l10n-et[ (("et") (gentext-et-nav-prevsib prevsib)) ]]>
    <![%l10n-fi[ (("fi") (gentext-fi-nav-prevsib prevsib)) ]]>
    <![%l10n-fr[ (("fr") (gentext-fr-nav-prevsib prevsib)) ]]>
    <![%l10n-it[ (("it") (gentext-it-nav-prevsib prevsib)) ]]>
    <![%l10n-ja[ (("ja") (gentext-ja-nav-prevsib prevsib)) ]]>
    <![%l10n-nl[ (("nl") (gentext-nl-nav-prevsib prevsib)) ]]>
    <![%l10n-no[ (("no") (gentext-no-nav-prevsib prevsib)) ]]>
    <![%l10n-pl[ (("pl") (gentext-pl-nav-prevsib prevsib)) ]]>
    <![%l10n-pt[ (("pt") (gentext-pt-nav-prevsib prevsib)) ]]>
    <![%l10n-pt-BR[ (("pt-br") (gentext-pt-BR-nav-prevsib prevsib)) ]]>
    <![%l10n-ro[ (("ro") (gentext-ro-nav-prevsib prevsib)) ]]>
    <![%l10n-ru[ (("ru") (gentext-ru-nav-prevsib prevsib)) ]]>
    <![%l10n-sk[ (("sk") (gentext-sk-nav-prevsib prevsib)) ]]>
    <![%l10n-sl[ (("sl") (gentext-sl-nav-prevsib prevsib)) ]]>
    <![%l10n-sv[ (("sv") (gentext-sv-nav-prevsib prevsib)) ]]>

    <![%l10n-da[ (("dk")   (gentext-da-nav-prevsib prevsib)) ]]>
    <![%l10n-de[ (("dege") (gentext-de-nav-prevsib prevsib)) ]]>
    <![%l10n-en[ (("usen") (gentext-en-nav-prevsib prevsib)) ]]>
    <![%l10n-no[ (("bmno") (gentext-no-nav-prevsib prevsib)) ]]>
    <![%l10n-sv[ (("svse") (gentext-sv-nav-prevsib prevsib)) ]]>
    (else (error "L10N ERROR: gentext-nav-prevsib "))))

(define (gentext-nav-nextsib nextsib)
  (case ($lang$)
    <![%l10n-ca[ (("ca") (gentext-ca-nav-nextsib nextsib)) ]]>
    <![%l10n-cs[ (("cs") (gentext-cs-nav-nextsib nextsib)) ]]>
    <![%l10n-da[ (("da") (gentext-da-nav-nextsib nextsib)) ]]>
    <![%l10n-de[ (("de") (gentext-de-nav-nextsib nextsib)) ]]>
    <![%l10n-el[ (("el") (gentext-el-nav-nextsib nextsib)) ]]>
    <![%l10n-en[ (("en") (gentext-en-nav-nextsib nextsib)) ]]>
    <![%l10n-es[ (("es") (gentext-es-nav-nextsib nextsib)) ]]>
    <![%l10n-et[ (("et") (gentext-et-nav-nextsib nextsib)) ]]>
    <![%l10n-fi[ (("fi") (gentext-fi-nav-nextsib nextsib)) ]]>
    <![%l10n-fr[ (("fr") (gentext-fr-nav-nextsib nextsib)) ]]>
    <![%l10n-it[ (("it") (gentext-it-nav-nextsib nextsib)) ]]>
    <![%l10n-ja[ (("ja") (gentext-ja-nav-nextsib nextsib)) ]]>
    <![%l10n-nl[ (("nl") (gentext-nl-nav-nextsib nextsib)) ]]>
    <![%l10n-no[ (("no") (gentext-no-nav-nextsib nextsib)) ]]>
    <![%l10n-pl[ (("pl") (gentext-pl-nav-nextsib nextsib)) ]]>
    <![%l10n-pt[ (("pt") (gentext-pt-nav-nextsib nextsib)) ]]>
    <![%l10n-pt-BR[ (("pt-br") (gentext-pt-BR-nav-nextsib nextsib)) ]]>
    <![%l10n-ro[ (("ro") (gentext-ro-nav-nextsib nextsib)) ]]>
    <![%l10n-ru[ (("ru") (gentext-ru-nav-nextsib nextsib)) ]]>
    <![%l10n-sk[ (("sk") (gentext-sk-nav-nextsib nextsib)) ]]>
    <![%l10n-sl[ (("sl") (gentext-sl-nav-nextsib nextsib)) ]]>
    <![%l10n-sv[ (("sv") (gentext-sv-nav-nextsib nextsib)) ]]>

    <![%l10n-da[ (("dk")   (gentext-da-nav-nextsib nextsib)) ]]>
    <![%l10n-de[ (("dege") (gentext-de-nav-nextsib nextsib)) ]]>
    <![%l10n-en[ (("usen") (gentext-en-nav-nextsib nextsib)) ]]>
    <![%l10n-no[ (("bmno") (gentext-no-nav-nextsib nextsib)) ]]>
    <![%l10n-sv[ (("svse") (gentext-sv-nav-nextsib nextsib)) ]]>
    (else (error "L10N ERROR: gentext-nav-nextsib"))))

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

    <![%l10n-da[ (("dk")   (gentext-da-nav-home home)) ]]>
    <![%l10n-de[ (("dege") (gentext-de-nav-home home)) ]]>
    <![%l10n-en[ (("usen") (gentext-en-nav-home home)) ]]>
    <![%l10n-no[ (("bmno") (gentext-no-nav-home home)) ]]>
    <![%l10n-sv[ (("svse") (gentext-sv-nav-home home)) ]]>
    (else (error "L10N ERROR: gentext-nav-home"))))

