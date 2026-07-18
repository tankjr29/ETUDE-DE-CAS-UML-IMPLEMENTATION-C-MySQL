-- =====================================================================
-- NutriCI - Modele physique de donnees (MySQL 8)
-- Coherent avec le diagramme de classes (Client, Administrateur, Livreur
-- heritent d'Utilisateur ; Adresse et Livraison sont des classes a part)
-- =====================================================================

CREATE DATABASE IF NOT EXISTS nutrici;
USE nutrici;

-- ---------------------------------------------------------------------
-- Categorie
-- ---------------------------------------------------------------------
CREATE TABLE categorie (
    id          INT AUTO_INCREMENT PRIMARY KEY,
    nom         VARCHAR(50) NOT NULL,
    description TEXT
) ENGINE=InnoDB;

-- ---------------------------------------------------------------------
-- Produit
-- ---------------------------------------------------------------------
CREATE TABLE produit (
    reference       VARCHAR(6) PRIMARY KEY,
    nom             VARCHAR(100) NOT NULL,
    description     TEXT,
    prix_unitaire   DECIMAL(10,2) NOT NULL,
    qte_stock       INT NOT NULL DEFAULT 0,
    actif           BOOLEAN DEFAULT TRUE,
    categorie_id    INT,
    FOREIGN KEY (categorie_id) REFERENCES categorie(id)
) ENGINE=InnoDB;

-- ---------------------------------------------------------------------
-- Utilisateur (table racine) + specialisations (Client / Administrateur /
-- Livreur) : strategie "table par sous-classe" (RG1, heritage UML)
-- ---------------------------------------------------------------------
CREATE TABLE utilisateur (
    id                INT AUTO_INCREMENT PRIMARY KEY,
    nom               VARCHAR(50) NOT NULL,
    prenom            VARCHAR(50) NOT NULL,
    email             VARCHAR(100) UNIQUE NOT NULL,
    telephone         VARCHAR(20) NOT NULL,
    mot_de_passe      VARCHAR(255) NOT NULL,
    type_utilisateur  ENUM('CLIENT','ADMIN','LIVREUR') NOT NULL,
    date_inscription  DATETIME DEFAULT CURRENT_TIMESTAMP
) ENGINE=InnoDB;

CREATE TABLE client (
    utilisateur_id INT PRIMARY KEY,
    ville          VARCHAR(50) DEFAULT 'Abidjan',
    FOREIGN KEY (utilisateur_id) REFERENCES utilisateur(id)
) ENGINE=InnoDB;

CREATE TABLE administrateur (
    utilisateur_id INT PRIMARY KEY,
    niveau_acces   VARCHAR(30) DEFAULT 'STANDARD',
    FOREIGN KEY (utilisateur_id) REFERENCES utilisateur(id)
) ENGINE=InnoDB;

CREATE TABLE livreur (
    utilisateur_id  INT PRIMARY KEY,
    vehicule        VARCHAR(50),
    zone_livraison  VARCHAR(100),
    FOREIGN KEY (utilisateur_id) REFERENCES utilisateur(id)
) ENGINE=InnoDB;

-- ---------------------------------------------------------------------
-- Adresse (1 client -> 1..* adresses ; adresse de livraison par defaut)
-- ---------------------------------------------------------------------
CREATE TABLE adresse (
    id            INT AUTO_INCREMENT PRIMARY KEY,
    client_id     INT NOT NULL,
    rue           VARCHAR(150) NOT NULL,
    quartier      VARCHAR(80),
    ville         VARCHAR(50) DEFAULT 'Abidjan',
    indications   TEXT,
    par_defaut    BOOLEAN DEFAULT TRUE,
    FOREIGN KEY (client_id) REFERENCES client(utilisateur_id)
) ENGINE=InnoDB;

-- ---------------------------------------------------------------------
-- Commande
-- ---------------------------------------------------------------------
CREATE TABLE commande (
    id                INT AUTO_INCREMENT PRIMARY KEY,
    client_id         INT NOT NULL,
    adresse_id        INT NOT NULL,
    date_commande     DATETIME DEFAULT CURRENT_TIMESTAMP,
    statut            ENUM('EN_ATTENTE','PAYEE','PREPARATION',
                            'EXPEDIEE','LIVREE','ANNULEE') DEFAULT 'EN_ATTENTE',
    frais_livraison   DECIMAL(10,2) DEFAULT 1500,
    montant_total     DECIMAL(10,2),
    FOREIGN KEY (client_id)  REFERENCES client(utilisateur_id),
    FOREIGN KEY (adresse_id) REFERENCES adresse(id)
) ENGINE=InnoDB;

-- ---------------------------------------------------------------------
-- LigneCommande
-- ---------------------------------------------------------------------
CREATE TABLE ligne_commande (
    id            INT AUTO_INCREMENT PRIMARY KEY,
    commande_id   INT NOT NULL,
    produit_ref   VARCHAR(6) NOT NULL,
    quantite      INT NOT NULL,
    prix_facture  DECIMAL(10,2) NOT NULL,
    FOREIGN KEY (commande_id) REFERENCES commande(id),
    FOREIGN KEY (produit_ref) REFERENCES produit(reference)
) ENGINE=InnoDB;

-- ---------------------------------------------------------------------
-- Paiement (1 commande -> 0..1 paiement)
-- ---------------------------------------------------------------------
CREATE TABLE paiement (
    id                      INT AUTO_INCREMENT PRIMARY KEY,
    commande_id             INT NOT NULL UNIQUE,
    montant                 DECIMAL(10,2) NOT NULL,
    mode                    ENUM('ORANGE_MONEY','WAVE','MTN_MONEY') NOT NULL,
    reference_transaction   VARCHAR(50),
    date_paiement           DATETIME DEFAULT CURRENT_TIMESTAMP,
    statut                  ENUM('EN_COURS','VALIDE','ECHOUE') DEFAULT 'EN_COURS',
    FOREIGN KEY (commande_id) REFERENCES commande(id)
) ENGINE=InnoDB;

-- ---------------------------------------------------------------------
-- Livraison (1 commande -> 0..1 livraison ; * livraisons -> 1 livreur)
-- ---------------------------------------------------------------------
CREATE TABLE livraison (
    id                 INT AUTO_INCREMENT PRIMARY KEY,
    commande_id        INT NOT NULL UNIQUE,
    livreur_id         INT NOT NULL,
    date_livraison     DATETIME,
    signature_client   VARCHAR(255),
    statut             ENUM('EN_COURS','LIVREE','PROBLEME') DEFAULT 'EN_COURS',
    FOREIGN KEY (commande_id) REFERENCES commande(id),
    FOREIGN KEY (livreur_id)  REFERENCES livreur(utilisateur_id)
) ENGINE=InnoDB;

-- =====================================================================
-- Donnees de reference
-- =====================================================================
INSERT INTO categorie VALUES (1,'Vitamines','Complements vitaminiques');
INSERT INTO categorie VALUES (2,'Proteines','Whey, caseine, vegetale');
INSERT INTO categorie VALUES (3,'Acides gras','Omega-3, huiles');
INSERT INTO categorie VALUES (4,'Anti-inflammatoires','Curcuma, boswellia');
INSERT INTO categorie VALUES (5,'Minceur','Bruleurs, draineurs');
INSERT INTO categorie VALUES (6,'Mineraux','Magnesium, zinc, fer');
INSERT INTO categorie VALUES (7,'Beaute','Collagene, biotine');

INSERT INTO produit VALUES
('VIT001','Vitamine C 1000mg','60 capsules',4500,25,TRUE,1),
('PRO002','Whey Protein 900g Chocolat','Proteine de lactoserum',18500,12,TRUE,2),
('OME003','Omega-3 Fish Oil','90 capsules',7800,3,TRUE,3),
('CUR004','Curcuma Bio 500mg','60 capsules',5200,45,TRUE,4),
('BRU005','Bruleur L-Carnitine','90 capsules',12000,2,TRUE,5),
('MAG006','Magnesium Bisglycinate','120 capsules',6300,30,TRUE,6),
('COL007','Collagene Marin','90 capsules',14500,15,TRUE,7),
('ZIN008','Zinc + Vitamine B6','60 capsules',3800,50,TRUE,6);
