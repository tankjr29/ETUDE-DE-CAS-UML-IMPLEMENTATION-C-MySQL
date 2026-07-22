# NutriCI — Étude de cas UML + Implémentation C++/MySQL

Projet de modélisation UML et d'implémentation C++/MySQL pour la plateforme e-commerce
NutriCI (compléments alimentaires, Abidjan). Réalisé dans le cadre de l'examen de 2ème
session — Modélisation UML.

---

## 📁 Contenu du projet

```
NutriCI_Projet/
├── README.md                              ← ce fichier
├── 1_Dossier_Word/
│   └── Dossier_Conception_NutriCI.docx    ← dossier de conception complet (23 pages)
├── 2_Diagrammes_drawio/
│   ├── 01_cas_utilisation.drawio
│   ├── 02_classes.drawio
│   ├── 03_sequence_passer_commande.drawio
│   ├── 04_sequence_confirmer_livraison.drawio
│   ├── 05_etats_transitions.drawio
│   ├── 06_activite.drawio
│   └── 07_deploiement.drawio
├── 3_Script_SQL/
│   └── nutrici.sql                        ← création de la base + données de référence
└── 4_Code_Source_Cpp/
    ├── Produit.h
    ├── Client.h
    ├── Paiement.h
    ├── LigneCommande.h
    ├── Commande.h
    ├── main.cpp                           ← démo automatique 
    └── .vscode/
        ├── tasks.json                     ← tâche de compilation VS Code
        ├── c_cpp_properties.json          ← configuration IntelliSense
        └── launch.json                    ← configuration de débogage (F5)
```

---

## 🧰 Prérequis

| Outil | Rôle | Où l'obtenir |
|---|---|---|
| **MySQL Server** (8.0+) | Base de données | [dev.mysql.com/downloads/installer](https://dev.mysql.com/downloads/installer/) (Web Installer, inclut Server + Workbench) |
| **Compilateur C++** (MinGW/GCC ou équivalent) | Compilation du code | Fourni par **Dev-C++** ([sourceforge.net/projects/orwelldevcpp](https://sourceforge.net/projects/orwelldevcpp/)), ou via **MSYS2** |
| **VS Code** (optionnel mais recommandé) | Éditeur + débogueur | [code.visualstudio.com](https://code.visualstudio.com/) + extension **C/C++** (Microsoft) |
| **draw.io** | Ouvrir/éditer les diagrammes `.drawio` | [app.diagrams.net](https://app.diagrams.net/) (aucune installation nécessaire, web) |

---

## 1️⃣ Créer la base de données

### Windows (PowerShell)

```powershell
Get-Content "3_Script_SQL\nutrici.sql" | & "C:\Program Files\MySQL\MySQL Server 8.0\bin\mysql.exe" -u root -p
```

### Windows (cmd) / Linux / macOS

```bash
mysql -u root -p < 3_Script_SQL/nutrici.sql
```

### Vérifier que ça a fonctionné

```bash
mysql -u root -p nutrici -e "SHOW TABLES; SELECT reference, nom, qte_stock FROM produit;"
```
→ tu dois voir **11 tables** et **8 produits** (Vitamine C, Whey Protein, Oméga-3, etc.).

---

## 2️⃣ Localiser les fichiers MySQL nécessaires à la compilation

Le code C++ utilise l'API C de MySQL (`mysql.h`, `libmysql.dll`). Si tu as installé
MySQL Server via le Web Installer (Server + Workbench), tout est déjà sur ta machine —
il suffit de le localiser :

```powershell
dir "C:\Program Files\MySQL\MySQL Server 8.0\include\mysql.h"
dir "C:\Program Files\MySQL\MySQL Server 8.0\lib\libmysql.lib"
dir "C:\Program Files\MySQL\MySQL Server 8.0\lib\libmysql.dll"
```

Note le nom exact du dossier (`MySQL Server 8.0`, `8.4`, etc. selon ta version) — il
faudra l'utiliser dans les chemins ci-dessous s'il diffère.

### ⚠️ Copier les DLL nécessaires au runtime (étape que tout le monde oublie)

`libmysql.dll` a besoin d'OpenSSL pour fonctionner. Copie ces 3 DLL **dans le dossier
`4_Code_Source_Cpp/`**, à côté de `main.cpp` :

```powershell
copy "C:\Program Files\MySQL\MySQL Server 8.0\lib\libmysql.dll" "4_Code_Source_Cpp\"
copy "C:\Program Files\MySQL\MySQL Server 8.0\bin\libssl-3-x64.dll" "4_Code_Source_Cpp\"
copy "C:\Program Files\MySQL\MySQL Server 8.0\bin\libcrypto-3-x64.dll" "4_Code_Source_Cpp\"
```

Sans ces DLL, `main.exe` se lance mais plante immédiatement avec une erreur du type
*"libssl-3-x64.dll est introuvable"*.

---

## 3️⃣ Adapter le mot de passe MySQL dans le code

Ouvre `main.cpp` (et `main_interactif.cpp` si tu l'utilises), cherche cette ligne :

```cpp
if (!mysql_real_connect(conn, "localhost", "root",
                         "root", "nutrici", 3306, NULL, 0)) {
```

Remplace le 4ᵉ argument (`"root"`) par **ton propre mot de passe root MySQL**, puis
sauvegarde.

---

## 4️⃣ Compiler et exécuter

### Option A — VS Code (recommandé)

1. Ouvre **uniquement** le dossier `4_Code_Source_Cpp` dans VS Code (**Fichier > Ouvrir un
   dossier**) — pas le dossier parent, sinon `.vscode/` n'est pas détecté à la racine.
2. Installe l'extension **C/C++** (Microsoft) si ce n'est pas déjà fait.
3. Ouvre les 3 fichiers dans `.vscode/` et **adapte les chemins** à ta machine :
   - `tasks.json` → chemin vers ton `g++.exe`, et vers `include`/`lib` de MySQL
   - `c_cpp_properties.json` → même chemin `include` (pour l'auto-complétion)
   - `launch.json` → chemin vers `gdb.exe` (optionnel, pour le débogage F5)

   Exemple de `tasks.json` fonctionnel (à adapter) :
   ```jsonc
   {
       "version": "2.0.0",
       "tasks": [
           {
               "label": "Compiler NutriCI (main.cpp)",
               "type": "shell",
               "command": "C:\\Program Files (x86)\\Dev-Cpp\\MinGW64\\bin\\g++.exe",
               "args": [
                   "-g",
                   "-std=c++11",
                   "-I", "C:\\Program Files\\MySQL\\MySQL Server 8.0\\include",
                   "-L", "C:\\Program Files\\MySQL\\MySQL Server 8.0\\lib",
                   "-lmysql",
                   "${file}",
                   "-o",
                   "${fileDirname}\\${fileBasenameNoExtension}.exe"
               ],
               "options": { "cwd": "${fileDirname}" },
               "problemMatcher": ["$gcc"],
               "group": { "kind": "build", "isDefault": true },
               "detail": "Compile le fichier C++ actif avec la librairie MySQL"
           }
       ]
   }
   ```

4. Ouvre `main.cpp` (ou `main_interactif.cpp`) pour qu'il soit l'onglet **actif**.
5. Compile : **Ctrl+Shift+B**
6. Exécute, dans le terminal intégré (**Ctrl+`**) :
   ```powershell
   .\main.exe
   ```
   ou pour la version interactive :
   ```powershell
   .\main_interactif.exe
   ```

> **Piège classique** : si Ctrl+Shift+B lance une tâche différente de la nôtre
> (par ex. générée automatiquement par l'extension C/C++), fais
> **Ctrl+Shift+P → "Tasks: Configure Default Build Task"** et choisis explicitement
> **"Compiler NutriCI (main.cpp)"**.

### Option B — Dev-C++

1. **Outils > Options du compilateur** :
   - Onglet **Général** : dans *"Ajouter les commandes suivantes lors de l'appel du
     linker"*, écris `-lmysql`
   - Onglet **Répertoires** :
     - *Includes* : `C:\Program Files\MySQL\MySQL Server 8.0\include`
     - *Bibliothèques* : `C:\Program Files\MySQL\MySQL Server 8.0\lib`
2. **Fichier > Ouvrir** → sélectionne `main.cpp` (les `.h` doivent être dans le même dossier)
3. **F9** pour compiler, **F10**/**F11** pour exécuter

### Option C — Linux / macOS (terminal)

```bash
sudo apt install libmysqlclient-dev g++      # Debian/Ubuntu
# ou : brew install mysql-connector-c         # macOS
```

Dans `main.cpp`, remplacer la ligne d'include par la version Linux :
```cpp
#include <mysql/mysql.h>
```
(sur Windows, avec MySQL Connector/C ou MySQL Server, c'est `#include <mysql.h>` sans
sous-dossier — les deux formes sont indiquées en commentaire dans le fichier)

Puis compiler et exécuter :
```bash
g++ -std=c++11 main.cpp -o nutrici -lmysqlclient
./nutrici
```

---

## 🧪 Sortie attendue

```
Connexion MySQL OK.

=== Catalogue NutriCI (8 produits) ===
BRU005  Bruleur L-Carnitine            12000 FCFA (Stock: 2) [ALERTE STOCK BAS]
COL007  Collagene Marin                14500 FCFA (Stock: 15)
...

Client #42 - Awa Kouassi (Abidjan)

=== Commande #0 (EN_ATTENTE) ===
  BRU005 x2 = 24000 FCFA
  COL007 x1 = 14500 FCFA
  Total : 38500 FCFA
  [ALERTE ADMIN] Stock de BRU005 sous le seuil critique (0 restants)
Commande #0 enregistree avec le statut : PAYEE
```

La version interactive (`main_interactif.cpp`) affiche à la place un menu :

```
=========================================
   NUTRICI - Plateforme e-commerce
   Connecte en tant que : Awa Kouassi
=========================================
 1. Consulter le catalogue
 2. Rechercher un produit
 3. Passer une commande
 4. Consulter mon historique de commandes
 5. Annuler une commande (en attente)
 6. [Administrateur] Gerer le stock / alertes
 0. Quitter
Choix :
```

---

## 🩺 Dépannage — erreurs fréquentes

| Erreur | Cause | Solution |
|---|---|---|
| `mysql/mysql.h: No such file or directory` | Include Linux utilisé sous Windows | Remplacer par `#include <mysql.h>` |
| `unrecognized command line option '-std=c++17'` | Compilateur trop ancien (GCC < 8) | Utiliser `-std=c++11` à la place |
| `cannot find -lmysql` / `undefined reference to mysql_init` | Mauvais chemin `-L`, ou lib incompatible avec le linker | Vérifier le chemin `lib`, s'assurer que `libmysql.lib`/`.dll` existent bien à cet endroit |
| Le programme se lance et se ferme sans rien afficher | DLL manquante (souvent OpenSSL) | Copier `libssl-3-x64.dll` et `libcrypto-3-x64.dll` à côté de l'exécutable |
| `Access denied for user 'root'@'localhost'` | Mauvais mot de passe dans le code | Corriger le 4ᵉ argument de `mysql_real_connect()` dans `main.cpp` |
| Ctrl+Shift+B lance la mauvaise tâche | Une autre tâche s'est mise par défaut | `Ctrl+Shift+P → Tasks: Configure Default Build Task` → choisir la nôtre |
| `main.exe` ne reflète pas mes changements | Fichier modifié mais pas recompilé | Vérifier les dates avec `Get-Item main.cpp, main.exe \| Select Name, LastWriteTime`, puis Ctrl+Shift+B |

---

## 📊 Diagrammes UML

Les 7 fichiers `.drawio` s'ouvrent sur **[app.diagrams.net](https://app.diagrams.net/)** via
**Fichier > Ouvrir depuis > Appareil**. Chacun est entièrement éditable (pas des images).
Pour les captures d'écran de l'Annexe B du dossier Word : **Fichier > Exporter sous > PNG**.

---

## ✅ Règles de gestion couvertes (traçabilité)

| Règle | Description | Implémentation |
|---|---|---|
| RG1 | Compte client obligatoire | `Client.h` |
| RG2 | Prix figé à la commande | `LigneCommande::prixFacture` |
| RG3 | Vérification/décrément du stock | `Produit::verifierStock()`, `decrementerStock()` |
| RG4 | Paiement Mobile Money, timeout 24h | `Paiement.h` (le timeout réel nécessiterait une tâche planifiée, non implémentée dans ce squelette) |
| RG5 | 5 statuts, ordre strict | `Commande::transitionAutorisee()` |
| RG6 | Produit jamais supprimé | `Produit::desactiver()` |
| RG7 | Notification livreur + signature | séquence "Confirmer une livraison" |
| RG8 | Alerte stock < 5 | `Produit::isStockBas()` |
| RG9 | Frais de livraison / gratuité | `Client::getFraisLivraisonBase()`, `Commande::calculerTotal()` |
| RG10 | Annulation EN_ATTENTE uniquement | `Commande::annuler()` |

---

## 👤 Auteur

**AGBENONZAN Kossivi Jacques Junior**
Licence 3 — Réseaux Informatique Sécurité et Télécommunications
Université Félix Houphouët-Boigny — UFR Mathématiques et Informatique
Enseignant : Dr. BROU — Examen de 2ème session, Modélisation UML — Juillet 2026