---
title: "Confinement, Train Simulator et STM32: partie 2, le KVB"
date: 2022-08-06T13:41:37+02:00
draft: false

twitter:
  card: summary
  image: "/blog/img/kvb/thumbnail.png"
---

## Le KV-quoi?

Le KVB, ou Contrôle de Vitesse par Balises, est un système qui surveille la vitesse du train pour éviter les accidents. Le système est composé de deux parties: le calculateur, dans le train; et la partie IHM (Interface Homme-Machine), qui contient les boutons et divers affichages, que je vais tenter de recréer.

Le calculateur... calcule, les courbes de vitesses, et selon la vitesse de la section (par radio avec une balise au sol, entre les rails), détecte les problèmes éventuels. Si la vitesse est supérieur de 5 km/h à la vitesse maximale théorique, trois choses se passent: le voyant V s'allume (Vitesse), une alerte sonore s'active pendant 2 secondes, et les deux afficheurs 7 segments clignotent. Si la vitesse dépasse les 10km/h au dessus de la limite, déclenchement du freinage d'urgence et le voyant "FU" (Freinage d'Urgence, le symbole avec une roue et des patins des deux côtés) s'allume. Si vous voulez plus d'explications dans le détail, je vous envoie sur ce [fantastique site](https://lesiteferroviaire.pagesperso-orange.fr/kvb.htm).

![KVB d'une rame RIO](/blog/img/kvb/kvb-rio.jpg)
_Source de l'image: [Clicsouris](https://commons.wikimedia.org/wiki/File:Rame_inox_de_banlieue_-_KVB_sur_le_pupitre_de_conduite.jpg)_

Je n'ait fait, pour l'instant, que la partie supérieur.

### Explication des différents boutons et afficheurs
- #### Boutons (carrés et blancs)
  - VAL (BP-VAL): validation, permet de valider la configuration entrée
  - Violet avec 3 points blancs (BP-MV): bouton manoeuvre
  - Rouge et blanc (BP-FC): franchissement carré (signal fermé)
  - Test (BP-TEST): test du KVB (affichages, boutons, arrêt d'urgence)
  - SF (BP-SF): permet d'éteindre la lampe de répétition des signaux (voir LS-SF)
- #### Afficheurs 7 segments
  - Visualisateur auxiliaire, en jaune
  - visualisateur principal, en vert
- #### Lampes
  - V (LS-V): Vitesse, s'allume en cas d'excès de vitesse de 5 km/h.
  - FU (LS-FU): Freinage d'urgence, s'allume en cas d'arrêt d'urgence par le KVB (+10 km/h)
  - Panne sol: s'allume en cas de balise défaillante (ce qui ne devrait pas arriver en simulation !)
  - Panne engin: idem, mais en cas de panne sur l'engin moteur
  - Grosse lampe en haut à droite: LS-SF, Signal Fermé, pas réellement du KVB (c'est en réalité la [RSO](https://lesiteferroviaire.pagesperso-orange.fr/repetition%20des%20signaux.htm "Répétition des Signaux Optiques")) mais incorporée dans le panneau pour des raisons pratiques

[Schema](https://lesiteferroviaire.pagesperso-orange.fr/Files/Image/panneau_kvb_detaille.gif)


## Maintenant que vous savez ce que c'est, comment?
Au lieu de vouloir bricoler une plaque avec une scie sauteuse et beaucoup de patience, j'ai décidé de faire tout un boîtier (et la facade) en impression 3D.
Le 19 mai 2021, à 02:37 du matin précisément, j'ai donc lancé Fusion 360 et ait commencé mon sketch.

Pour modéliser, j'ai pris exactement la même photo que ci-dessus pour m'en servir de modèle, puis, en partant du principe que les vis étant du M3 ou M4, j'ai tout mis à la bonne échelle. Parce que le boîtier est vissé/dévissé souvent, j'ai choisi d'utiliser des inserts.

## Le plus important: les afficheurs 7 segments
Les information les plus importantes sont données sur les deux visualisateurs à base d'afficheurs 7 segments. Cependant, n'ayant jamais trouvé de bon modules pour l'afficheur auxiliaire (en jaune/ambre), je vais me concentrer sur l'afficheur principal, en vert.

### Les debuts
![Afficheur TM1637 vert](/blog/img/kvb/tm1637-green.png "Exemple de module à base de TM1637")
Pour des raisons très techniques de pure simplicité, j'ai choisi d'utiliser un module à base de TM1637 ([datasheet](/blog/img/kvb/TM1637.pdf)). Si vous lisez la datasheet, vous trouverez que c'est une puce qui fonctionne à anode commune (c'est à dire le côté positif des segments, pour se souvenir: PANIC (Positive is Anode, Negative Is Cathode)). Quelle ne fut pas ma surprise quand j'ai accidentellement commandé des afficheurs à cathode commune.

Après avoir commandé les bons afficheurs, j'en ai testé un manuellement en le connectant directement aux pins sur le module TM1637.
![Un seul afficheur soudé sur une perfboard](/blog/img/kvb/1-7seg-tm1637.jpg)
*C'était 4h du matin !*
![Câblage sur la plaquette](/blog/img/kvb/2-7seg-tm1637.jpg)

### Un c'est bon, plus que le reste !
Vu qu'avec un seul segment ça fonctionnait, autant continuer avec trois ! Je me suis donc empressée de souder 3 afficheurs 7 segments, toujours sur un perfboard, et de relier les différents segments et cathodes. C'est... pas très propre, mais ça marche !
![Perfboard avec 3 afficheurs 7 segments vert branchés avec le module TM1637](/blog/img/kvb/3-7seg-tm1637.jpg)
![Derrière la perfboard](/blog/img/kvb/3bis-7seg-tm1637.jpg)
C'est compliqué de relier 7 segments en communs, plus les cathodes individuelles. J'ai fait un peu de [wire wrapping](https://fr.wikipedia.org/wiki/Wrapper_(%C3%A9lectronique)), mais sans outil ! C'est un peu sportif, ça met beaucoup de temps (comptez une très bonne soirée) et avec un peu de malchance, le fil se casse en plein milieu et il faut tout recommencer. Vous avez vite envie de faire un PCB ! (*foreshadowing quand tu nous tiens*...)

![Afficheur affichant trois tirets, indiquant que ça fonctionne](/blog/img/kvb/4-7seg-tm1637.jpg)
Bonne nouvelle ! Les trois tirets veulent dire que tout fonctionne ! (dans la réalité, ils signifient que tout va bien et le KVB fonctionne)

### Un PCB?
L'étape suivante, c'est de faire un PCB (Printed Circuit Board, un circuit imprimé) !
Toute heureuse, je me lance bêtement pour faire un schéma, puis un PCB avec le TM1637 et de quoi brancher mon module 7 segments. Je suis bien la datasheet, mais moi et ma chance... Je reçoit les circuits (assemblés s'il vous plaît !), je le branche et... Rien !
![Rendu 3D du premier PCB](/blog/img/kvb/pcb1.png "Rendu 3D du premier PCB")
Tant pis, je ferais de manière moche.

Je fais à la place un autre circuit pour tenir les 7 segments et réunir leurs connexions en un seul point, pour éviter de devoir tout câbler à la main à nouveau.
![Rendu 3D du second PCB](/blog/img/kvb/pcb2.png "Rendu 3D du second PCB (pas complet)")
*Vous trouvez le problème? Indice, c'est en bas !*

Cependant, à la réception je me suis aperçue qu'il y avait une trace qui passait trop près de tous les pins sur un des afficheurs. Résultats, je suis obligée de couper cette trace manuellement entre chaque point. Oups !
![Bodge wire et coup de cutter pour corriger cette erreur](/blog/img/kvb/pcb-trace-bodge-wire.jpg "Bodge wire et coup de cutter pour corriger cette erreur")

## Le début de la modélisation
![20 mai, à 14h et quelques](/blog/img/kvb/kvb-20-mai.png "Le lendemain aprem")
Après quelques jours
![Une grosse semaine après](/blog/img/kvb/kvb-29-mai.png "Une grosse semaine après")
Cette version vous intéresse? J'ai un export du [STEP ici](/blog/img/kvb/KVB-mai.stp).

Après pas mal de temps passé avec ma perceuse et une lime pour corriger la taille des trous, trop petits, impressions 3D oblige, j'ai fait beaucoup de légers changements. Quasiment un an plus tard, voici le résultat "final", en prenant en compte les problèmes suivants:
- Je n'ai pas trouvé d'afficheur 7 segment "ambre", encore moins de la bonne taille (et 3 chiffres), donc à la place j'utilise un simple module 4 chiffres rouges
- Normalement on peut tourner le LS-SF afin d'ajuster la luminosité, cependant je ne le supporte pas.

### 1ère version
Première version imprimée, avec d'anciens boutons (qui n'étaient ni assez grands, ni rétro éclairés, ni satisfaisants, ni fonctionnels après avoir été soudé, et il n'est pas non plus possible d'insérer une étiquette) et seulement l'afficheur principal:
![Impression 3D de la plaque frontale avec l'afficheur 7 segment principal qui rentre pile](/blog/img/kvb/kvb-1ere-impression.jpg)

On arrive en juin et en juillet: l'heure de faire une petite pause.

De retour fin juillet, j'ai reçu de nouveaux boutons (excellents, je le recommande clairement, sur [AliExpress](https://fr.aliexpress.com/item/32918157519.html))

À l'époque, j'avais tout sur une breadboard, mais c'était un enfer (les câbles qui se déconnectent, les mauvais contact, et puis la taille !)
![Breadboard avec seulement les câbles, sans composant](/blog/img/kvb/breadboard.jpg)
J'ai donc heureusement décidé de passer sur un perfboard, ce que j'ai fait plutôt rapidement.
![Perfboard vue de dessus](/blog/img/kvb/perfboard1.jpg)
Les câbles à droite vont vers les boutons, les afficheurs 7 segments se branchent en haut à gauche
![Perfboard vue de dessous](/blog/img/kvb/perfboard2.jpg)
On peut voir les gros blobs de soudure, en haut pour le "rail +5V" et en bas pour la masse. On peut apercevoir la demi-tonne de câbles rouges, avec un élastique à droite. Ce sont les câbles qui relient l'afficheur principal (vert) et le module 7 segments derrière.

Pour des raisons de bricolage, ça tient à la patafix blanche.

Avec tout câblé, les LEDs ajoutées et le PCB venu à l'élastique, voilà le résultat !
![Plaque frontale peuplée de tous ses composants](/blog/img/kvb/kvb-led.jpg)
Test automatique des LEDs:

{{< rawhtml >}} 
<video width=100% controls loop alt="">
    <source src="/blog/img/kvb/kvb-led-test.mp4" type="video/mp4">
    La vidéo ne fonctionne pas?
    <a href="/blog/img/kvb/kvb-led-test.mp4">Téléchargez là directement</a>
</video>
{{< /rawhtml >}}
Pour donner une idée, voici l'intérieur du boîtier à l'époque:
![Vue de l'intérieur](/blog/img/kvb/arriere-boitier-1.jpg)
([autre vue](/blog/img/kvb/arriere-boitier-2.jpg))

### Final
Le résultat "final" de l'époque (5 août 2021) !
![KVB complet, en fonctionnement](/blog/img/kvb/kvb-sur-bureau.jpg)

Fin août j'ai rajouté la fameuse lampe LS-SF. Cependant, elle fonctionne en 12V, j'ai donc rajouté un boost converter pour passer à 12 V puis... un optocoupleur, pour contrôler la LED. C'est ce que j'avais sous la main le plus simple, et puis c'est drôle.
Et voici donc une démo en fonctionnement:
{{< rawhtml >}} 
<video width=100% controls loop alt="">
    <source src="/blog/img/kvb/kvb-test.mp4" type="video/mp4">
    La vidéo ne fonctionne pas?
    <a href="/blog/img/kvb/kvb-test.mp4">Téléchargez là directement</a>
</video>
{{< /rawhtml >}}

Récemment je me suis décidée à éditer un peu le modèle 3D pour mieux tenir les boutons, avoir un gap plus faible sur le visualisateur auxiliaire et utiliser des vrais "LED holders" (sur [eBay](https://www.ebay.fr/itm/112408248171)), car la patafix c'est pas vraiment ça. Je remplacerait aussi bientôt LS-SF par la même lampe mais avec un angle plus élevé (30° actuellement, 120 après).
![Rendu actuel](/blog/img/kvb/kvb-final.jpg)

Pour le modèle 3D actuel c'est [ici](/blog/img/kvb/KVB.stp). Notez qu'il faut vous même percer le trou pour LS-SF.

## PCB
Pour me simplifier la vie, j'ai rapidement fait un PCB, quasiment à l'identique de la perfboard.

[![Schéma](/blog/img/kvb/schema-pcb-final.png)](/blog/img/kvb/schema-pcb-final.png)
![PCB final de commande](/blog/img/kvb/pcb3.png)


## Côté communication
Bien que j'utilise une carte à base de STM32F103C8T6, notamment pour son port USB type C, il devrait être possible d'utiliser un autre Arduino sans trop de problème.

Étant donné qu'il y a beaucoup plus d'informations qu'avant (les 5 boutons dans le sens Arduino ↦ PC, les 9 LEDs (4 statut, 4 boutons, LS-SF) et les deux afficheurs 7 segments dans le sens PC ↦ Arduino), j'ai fait le choix d'utiliser du JSON en série pour la communication. C'est extrêmement inefficace et ce n'est pas du tout une bonne solution, mais ça fonctionne et c'est pratique. J'utilise donc l'excellente librairie [ArduinoJson](https://arduinojson.org/). J'utilise aussi la librairie [Bounce2](https://github.com/thomasfredericks/Bounce2) afin d'éviter de devoir implémenter du debouncing manuellement.

Pour éviter de spammer TrainSim, j'ai implémenté une simple vérification qui n'envoie une nouvelle ligne que si elle est différente de la précédente. Côté PC, les deux fonctions (readInput et writeOutput) fonctionnent en parallèle, dans deux threads séparés.
## Code source
[Code source Arduino](/blog/img/kvb/kvb.ino)
[Code source PC](/blog/img/kvb/kvb.py) (le frein/Grand débit est à moitié supporté.)

## Conclusion
Ça m'en aura pris du temps pour brancher une dizaine de LEDs et de boutons ! La tâche qui aura pris le plus de temps est sûrement de remplir les paramètres pour les afficheurs 7 segments. En tout cas j'ai pas mal appris sur tout ça (\*tousse\* DRC).

En terminant cet article, je viens de réaliser la quantité d'afficheurs 7 segments que j'ai acheté pour ce projet. Je vous laisse juger !
![](/blog/img/kvb/7segments.jpg)

Merci beaucoup de m'avoir lue !