---
title: "Confinement, Train Simulator et STM32: partie 1, la VA"
date: 2022-08-04T14:06:07+02:00
draft: false
---

# Le contexte
C'était le confinement, vers mi-2020. À l'époque où tout sur AliExpress ne coûtait pas encore un prix ridicule, à l'époque où on pouvait acheter un module STM32F103(C8T6) avec USB type C pour... 1€60 !
![Capture d'écran d'Aliexpress montrant la commande d'un STM32 pour 1€60](/blog/img/kvb/aliexpress-1.png)
Je suis fan de trains (et donc de [Train simulator](https://store.steampowered.com/app/24010/Train_Simulator_Classic/?l=french)) et j'avais beaucoup de temps libre, école à distance/hybride oblige. Comme beaucoup de passioné·e·s, je rêve d'avoir, un jour, un vrai pupitre (de commande) chez moi, et de pouvoir le connecter à un simulateur ferroviaire (TS, ou surtout OpenRails, comme souvent).
Par exemple:
[![Pupitre ce CC 72000, converti par Ferrovisim](/blog/img/kvb/pupitre-72000-ferrovisim.jpg "Pupitre de CC 72000 converti sous OpenRails")](https://www.youtube.com/watch?v=fl0SIUiUrc0)

N'ayant pas de pupitre (ça se trouve pas sur leboncoin (quoi que...)), il existe une autre option: le RailDriver.
![Contrôleur RailDriver pour TrainSimulator](/blog/img/kvb/raildriver.png)
Le seul problème, c'est que ça coûte cher, et je ne croule pas sous l'argent. Doooonc, la bricole à la rescousse (et puis c'est plus intéressant !)
# En pratique?
Donc pour commencer, j'ai voulu tenter par un élément simple: la pédale de VA ([Veille Automatique](https://fr.wikipedia.org/wiki/Veille_automatique)).
Explication rapide: la VA (complet: VACMA, veille automatique à contrôle du maintien d'appui) sert à éviter qu'en cas de (notamment) malaise conducteur, le train s'arrête automatique.

La VA est présente sous différentes formes: une pédale à maintenir appuyée, mais pas plus de 55 secondes. Il faut en effet la relâcher et rappuyer pour prouver qu'on est bien conscient. Si le conducteur garde sa pédale enfoncée 55s, une sonnerie se met à sonner. Si au bout de 2.5s il n'a pas relâché l'appui, le freinage d'urgence s'engage.

L'avantage de commencer par cette partie est qu'elle est très simple à implémenter (un seul élément booléen), et qu'une pédale est bien plus pratique que la barre espace !

J'avais acheté par curiosité une pédale à connecter avant: parfait !
![Pédale](/blog/img/kvb/pédale.png)
![Pédale connecteurisée](/blog/img/kvb/pédale_avec_connecteur.jpg)

Donc côté matériel, c'est presque fini. Plus que la connexion avec le logiciel.

# La communication et l'intégration
Et c'est là qu'on a de la chance: grâce au RailDriver précédemment évoqué, il existe une DLL pour s'interface "facilement" avec TS: "RailDriver.dll" et "RailDriver64.dll".

Je saute toute la partie "bricoler avec un outils en Java" ([cet outil](https://github.com/reallyinsane/trainsimulator-controller)), j'ai pas réussi à en faire grand chose.

Cependant, il existe un autre language plus pratique que Java: Python !
Malgré que je n'aime pas spécialement Python (manque d'expérience, j'aime pas trop la syntaxe basée sur l'indentation, etc), il existe une superbe librairie: [py-raildriver](https://github.com/piotrkilczuk/py-raildriver).

Ainsi, côté Arduino (le STM32F103 précédent, quand il était encore vivant) en série, j'envoie simplement un 0 ou un 1, suivi d'un LF (\n, retour à la ligne), et côté client (sur le PC):

{{< highlight python >}}
from serial import *
import raildriver
import os
import configparser
import json


rd = raildriver.RailDriver()
rd.set_rail_driver_connected(True)  # start data exchange
loco_name = rd.get_loco_name()      # checks if it actually works
print(loco_name)

config = configparser.ConfigParser()
config.read('config.ini')
try:
    keys = config[loco_name[1]]
except TypeError as e:
    print(e)
    print("Is train Simulator running correctly?")
    input('Press Enter to exit')
    os._exit(1)


port_serie = Serial(port=COM46, baudrate=115200, timeout=1, writeTimeout=1)

if port_serie.isOpen():
    while True:
        ligne = port_serie.readline().rstrip()
        rd.set_controller_value(keys["VACMA"], float(ligne.decode("utf-8")))
{{< / highlight >}}
Le nom des contrôles dépend du matériel utilisé. J'utilise donc un fichier .ini, qui contient un *mappage* entre les termes génériques (ici VACMA) et le terme spécifique au matériel (La VACMA sur AGC, c'est `CommandeVacma` mais sur matériel *SimExpress* c'est `vacma_control`).

Et avec un peu de chance ça marche. Bon, il faut parfois relancer quelques fois le jeu ou le script, mais à part ça, ça marche !

# La suite
Le plus évident pour contrôler un train est le, en language SNCF, ✨ MPT ✨: le ManiPulateur de Traction (en terme TS, *Regulator*).

Si on ne peut pas avancer, on ne peut pas faire grand chose !

![Potentiomètre de côté](/blog/img/kvb/slide_pot.png)

Côté Arduino:
J'envoie une virgule entre chaque valeur, *CSV-style*, et après je découpe côté PC.
{{< highlight C >}}
#include <Arduino.h>

void setup()
{
  pinMode(VA, INPUT_PULLUP); // remplacez VA par votre pin

  Serial.begin(115200); // on initialise la connexion série

  while (Serial.available())
  {
    Serial.println("Arduino démarré");
  }
}


void separate()
{
  Serial.print(",");
}

void loop()
{
    Serial.print(readButton(PB14)); separate();
    Serial.println(float(analogRead(PA7)) / 1023);

    delay(300); // temporaire, pas très propre, mais bon
}
{{< / highlight >}}
Ensuite, côté PC, c'est pareil:
{{< highlight python >}}
if port_serie.isOpen():
    while True:
        ligne = port_serie.readline().rstrip()
        args = ligne.split(",")
        rd.set_controller_value(keys["VACMA"], float(args[0].decode("utf-8")))
        rd.set_controller_value(keys["Regulator"], float(args[1]))
{{< / highlight >}}

Et normalement, ça devrait marcher. Cependant, j'écris ce code presque deux ans après l'avoir réellement expérimenté donc ça peut ne pas être parfait.

# Final
![Plaque de test "terminée"](/blog/img/kvb/plaque.jpg)
Après m'être ratée à gauche sur la découpe, j'ai rentré, plus proprement à droite. De gauche à droite:
- Interrupteur 3 position - inverseur: avancer, neutre, reculer
- Bouton vert - Grand Débit: permet de retirer rapidement les freins
- Bouton rouge réarmable - Arrêt d'urgence
- Interrupteur avec protection rouge - Aucun usage, juste pour tester
- Potentiomètre vertical qui sert pour le manipulateur de traction

C'était sympa, mais niveau immersion, on a vu mieux. Notamment car, dans la liste des éléments vraiment obligatoires, il manque... de quoi freiner !

Donc j'ai décidé de m'attaquer à un composant de tous les pupitres modernes: le [KVB](https://fr.wikipedia.org/wiki/Contr%C3%B4le_de_vitesse_par_balises "Contrôle de Vitesse par Balises"). Mais ça, c'est pour le prochain article.