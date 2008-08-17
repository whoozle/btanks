Version bêta de Battle Tanks
(Les bêta testeurs peuvent considérer que ce fichier est leur manuel.)

Installation :
Décompactez les fichiers dans un répertoire quelconque (par exemple, C:\Jeux\Btanks\).

Ce paquet est fourni sans installeur, vous devrez donc utiliser l'invite de commande pour changer de mode de fonctionnement (si nécessaire), voir la liste ci-dessous.

Le jeu peut fonctionner de deux façons différentes :
1. En utilisant le rendu OpenGL.  Celui-ci tire parti des capacités de votre carte graphique.  Les pilotes d'origine de votre carte graphique sont probablement installés et sont suffisant en général.  Si ce n'est pas le cas, les performances risquent d'être extrêmement faibles.
2. En utilisant le rendu logiciel.  Cela fonctionne indépendamment de la carte graphique utilisée, au prix d'une baisse modérée de performances.

Paramètres optionnels de la ligne de commande :
--no-gl     désactive l'utilisation d'OpenGL (accélération matérielle)
--force-gl  force l'utilisation d'OpenGL, même si aucune accélération matérielle n'est détectée.
--fs        active le mode plein écran (il sera activé par défaut à l'avenir)
--lang=XX   force le choix de la langue (XX - un code ISO à deux lettres : en, ru, de, fr)
--vsync     active la synchronisation verticale -- à utiliser en cas de problèmes d'affichage
--connect=host  se connecte à l'hôte donné en argument
--no-sound      désactive complètement le son.
--sound         active le son, même s'il est désactivé dans les préférences (bt.xml).

La résolution d'écran peut également être précisée :
-0 640x480
-1 800x600 (valeur par défaut)
-2 1024x768
-3 1152x864
-4 1280x1024

--connect=nomdhote/IP  rejoindre la partie à l'adresse IP ou au nom d'hôte spécifié
--no-sound             désactive le son (bruitages et musique)

Les erreurs sont reportées dans le fichier de log "stderr.txt".

