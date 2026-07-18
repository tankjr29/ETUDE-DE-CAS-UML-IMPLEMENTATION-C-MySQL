NUTRICI - DOSSIER DE CONCEPTION UML - PROJET COMPLET
======================================================
Genere le 18/07/2026 - a relire et personnaliser avant remise.

CONTENU DU ZIP
---------------
1_Dossier_Word/
    Dossier_Conception_NutriCI.docx
    -> Le dossier complet a imprimer (couverture, 4 chapitres, dictionnaire
       de donnees, script SQL, code C++, annexes A et B).
    -> A l'ouverture dans Word : Ctrl+A puis F9 pour actualiser le sommaire.
    -> Complete tes Nom / Prenom / N. etudiant en page de garde.

2_Diagrammes_drawio/
    7 fichiers .drawio, un par diagramme, a ouvrir sur app.diagrams.net
    (Fichier > Ouvrir depuis > Appareil) ou dans l'app de bureau draw.io.
    Tous verifies automatiquement (0 chevauchement, 0 reference cassee) :
      01_cas_utilisation.drawio
      02_classes.drawio
      03_sequence_passer_commande.drawio
      04_sequence_confirmer_livraison.drawio
      05_etats_transitions.drawio
      06_activite.drawio
      07_deploiement.drawio
    -> Exporte chacun en PNG (Fichier > Exporter sous > PNG) pour les
       coller en Annexe B du dossier Word (captures d'ecran de l'AGL,
       exige par le sujet).

3_Script_SQL/
    nutrici.sql -> a executer avec : mysql -u root -p < nutrici.sql

4_Code_Source_Cpp/
    Produit.h, Client.h, Paiement.h, LigneCommande.h, Commande.h, main.cpp
    Compilation : g++ -o nutrici main.cpp -lmysqlclient -std=c++17
    (necessite libmysqlclient-dev installe sur ta machine)

POINTS DE VIGILANCE AVANT LA REMISE DU 25 JUILLET
---------------------------------------------------
1. Ouvre les 7 .drawio et verifie visuellement le rendu (la structure a
   ete validee automatiquement mais je n'ai pas pu les ouvrir moi-meme
   dans draw.io).
2. RG4 (timeout 24h) : le code montre la structure (statuts, methode
   changerStatut) mais N'IMPLEMENTE PAS de vrai minuteur/tache planifiee.
   A mentionner si on te le demande, ou a ajouter en bonus (ex. evenement
   MySQL ou tache cron qui annule les commandes EN_ATTENTE > 24h).
3. Teste la compilation et l'execution avec un vrai serveur MySQL chez toi.
4. Optionnel pour maximiser la note :
   - separer "Gerer le panier" en 2 UC distincts (ajouter / modifier qte)
     si le correcteur est strict sur la granularite des user stories ;
   - separer la transition "annulation client" et "timeout 24h" en deux
     fleches distinctes dans le diagramme d'etats.

VERIFICATION DES REGLES DE GESTION (RG1-RG10)
------------------------------------------------
Toutes verifiees et tracees dans le dossier Word, chapitre par chapitre.
Voir section "Dictionnaire de donnees" (2.2) et les commentaires RGx dans
le code C++ et le script SQL pour la tracabilite complete.
