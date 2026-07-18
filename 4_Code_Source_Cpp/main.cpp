#include <iostream>
#include <mysql.h> // Windows (MySQL Connector/C ou MySQL Server) : mysql.h direct.
                    // Sous Linux : remplacer par #include <mysql/mysql.h>
#include <vector>
#include <sstream>
#include <limits>
#include "Produit.h"
#include "Client.h"
#include "Commande.h"
#include "Paiement.h"

// ============================================================
// Connexion MySQL
// ============================================================
MYSQL* connecter() {
    MYSQL* conn = mysql_init(NULL);
    if (!mysql_real_connect(conn, "localhost", "root",
                             "root", "nutrici", 3306, NULL, 0)) {
        std::cerr << "Erreur MySQL : " << mysql_error(conn) << std::endl;
        return nullptr;
    }
    std::cout << "Connexion MySQL OK.\n";
    return conn;
}

// ============================================================
// Acces aux donnees
// ============================================================
std::vector<Produit> listerProduits(MYSQL* conn, bool seulementActifs = true) {
    std::vector<Produit> produits;
    std::string sql = "SELECT reference, nom, prix_unitaire, qte_stock, actif FROM produit";
    if (seulementActifs) sql += " WHERE actif = 1";
    sql += " ORDER BY nom";
    mysql_query(conn, sql.c_str());
    MYSQL_RES* res = mysql_store_result(conn);
    MYSQL_ROW row;
    while ((row = mysql_fetch_row(res))) {
        produits.push_back(Produit(row[0], row[1], std::stod(row[2]),
                                    std::stoi(row[3]), std::string(row[4]) == "1"));
    }
    mysql_free_result(res);
    return produits;
}

bool trouverProduit(MYSQL* conn, const std::string& refOuNom, Produit& resultat) {
    std::ostringstream q;
    q << "SELECT reference, nom, prix_unitaire, qte_stock, actif FROM produit "
      << "WHERE reference='" << refOuNom << "' OR nom LIKE '%" << refOuNom << "%' LIMIT 1";
    mysql_query(conn, q.str().c_str());
    MYSQL_RES* res = mysql_store_result(conn);
    MYSQL_ROW row = mysql_fetch_row(res);
    bool trouve = false;
    if (row) {
        resultat = Produit(row[0], row[1], std::stod(row[2]), std::stoi(row[3]),
                            std::string(row[4]) == "1");
        trouve = true;
    }
    mysql_free_result(res);
    return trouve;
}

bool decrementerStockEnBase(MYSQL* conn, const std::string& ref, int qte) {
    std::ostringstream check;
    check << "SELECT qte_stock FROM produit WHERE reference='" << ref << "'";
    mysql_query(conn, check.str().c_str());
    MYSQL_RES* res = mysql_store_result(conn);
    MYSQL_ROW row = mysql_fetch_row(res);
    int stockActuel = row ? std::stoi(row[0]) : 0;
    mysql_free_result(res);

    if (stockActuel < qte) return false; // RG3

    std::ostringstream upd;
    upd << "UPDATE produit SET qte_stock = qte_stock - " << qte
        << " WHERE reference='" << ref << "'";
    mysql_query(conn, upd.str().c_str());

    if (stockActuel - qte < 5) { // RG8
        std::cout << "  [ALERTE ADMIN] Stock de " << ref
                  << " sous le seuil critique (" << (stockActuel - qte)
                  << " restants)\n";
    }
    return true;
}

int enregistrerCommande(MYSQL* conn, const Client& client, Commande& commande,
                         ModePaiement mode) {
    std::ostringstream insCmd;
    insCmd << "INSERT INTO commande (client_id, adresse_id, statut, "
              "frais_livraison, montant_total) VALUES ("
           << client.getId() << ", 1, 'EN_ATTENTE', "
           << client.getFraisLivraisonBase() << ", "
           << commande.calculerTotal() << ")";
    mysql_query(conn, insCmd.str().c_str());
    int commandeId = static_cast<int>(mysql_insert_id(conn));

    for (const auto& ligne : commande.getLignes()) {
        std::ostringstream insLigne;
        insLigne << "INSERT INTO ligne_commande (commande_id, produit_ref, "
                     "quantite, prix_facture) VALUES ("
                 << commandeId << ", '" << ligne.getProduit().getReference()
                 << "', " << ligne.getQuantite() << ", " << ligne.getMontant() << ")";
        mysql_query(conn, insLigne.str().c_str());
        decrementerStockEnBase(conn, ligne.getProduit().getReference(), ligne.getQuantite());
    }

    Paiement paiement(0, commandeId, commande.calculerTotal(), mode);
    paiement.valider("TXN-" + std::to_string(commandeId));

    std::ostringstream insPay;
    insPay << "INSERT INTO paiement (commande_id, montant, mode, "
              "reference_transaction, statut) VALUES ("
           << commandeId << ", " << paiement.getMontant() << ", '"
           << modeToString(mode) << "', 'TXN-" << commandeId << "', 'VALIDE')";
    mysql_query(conn, insPay.str().c_str());

    commande.changerStatut(StatutCommande::PAYEE);
    mysql_query(conn, ("UPDATE commande SET statut='PAYEE' WHERE id=" +
                        std::to_string(commandeId)).c_str());

    return commandeId;
}

// RG10 : annulation uniquement si EN_ATTENTE
bool annulerCommandeEnBase(MYSQL* conn, int commandeId) {
    std::ostringstream check;
    check << "SELECT statut FROM commande WHERE id=" << commandeId;
    mysql_query(conn, check.str().c_str());
    MYSQL_RES* res = mysql_store_result(conn);
    MYSQL_ROW row = mysql_fetch_row(res);
    std::string statut = row ? row[0] : "";
    mysql_free_result(res);

    if (statut != "EN_ATTENTE") return false;

    mysql_query(conn, ("UPDATE commande SET statut='ANNULEE' WHERE id=" +
                        std::to_string(commandeId)).c_str());
    return true;
}

void afficherHistorique(MYSQL* conn, int clientId) {
    std::ostringstream q;
    q << "SELECT id, date_commande, statut, montant_total FROM commande "
      << "WHERE client_id=" << clientId << " ORDER BY date_commande DESC";
    mysql_query(conn, q.str().c_str());
    MYSQL_RES* res = mysql_store_result(conn);
    MYSQL_ROW row;
    std::cout << "\n=== Historique des commandes (client #" << clientId << ") ===\n";
    bool vide = true;
    while ((row = mysql_fetch_row(res))) {
        vide = false;
        std::cout << "  Commande #" << row[0] << " | " << row[1]
                  << " | " << row[2] << " | " << row[3] << " FCFA\n";
    }
    if (vide) std::cout << "  Aucune commande pour l'instant.\n";
    mysql_free_result(res);
}

void afficherAlertesStock(MYSQL* conn) {
    mysql_query(conn, "SELECT reference, nom, qte_stock FROM produit WHERE qte_stock < 5 AND actif=1");
    MYSQL_RES* res = mysql_store_result(conn);
    MYSQL_ROW row;
    std::cout << "\n=== Alertes stock bas (RG8) ===\n";
    bool vide = true;
    while ((row = mysql_fetch_row(res))) {
        vide = false;
        std::cout << "  [ALERTE] " << row[0] << " - " << row[1]
                   << " : " << row[2] << " unites restantes\n";
    }
    if (vide) std::cout << "  Aucune alerte, tous les stocks sont corrects.\n";
    mysql_free_result(res);
}

// ============================================================
// Utilitaires de saisie
// ============================================================
int lireEntier(const std::string& invite) {
    int v;
    while (true) {
        std::cout << invite;
        if (std::cin >> v) { std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); return v; }
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::cout << "  Entree invalide, reessaie.\n";
    }
}

std::string lireLigne(const std::string& invite) {
    std::string v;
    std::cout << invite;
    std::getline(std::cin, v);
    return v;
}

// ============================================================
// Actions du menu
// ============================================================
void actionCatalogue(MYSQL* conn) {
    auto produits = listerProduits(conn);
    std::cout << "\n=== Catalogue NutriCI (" << produits.size() << " produits) ===\n";
    for (const auto& p : produits) p.afficher();
}

void actionRecherche(MYSQL* conn) {
    std::string terme = lireLigne("\nReference ou nom (ou partie du nom) a rechercher : ");
    Produit p("", "", 0, 0);
    if (trouverProduit(conn, terme, p)) {
        std::cout << "Trouve :\n  ";
        p.afficher();
    } else {
        std::cout << "Aucun produit ne correspond a \"" << terme << "\".\n";
    }
}

void actionPasserCommande(MYSQL* conn, Client& client) {
    actionCatalogue(conn);
    Commande commande(0, client.getId(), client.getFraisLivraisonBase());
    std::cout << "\n--- Constitution du panier ---\n"
              << "(tape la reference du produit, ou \"fin\" pour terminer)\n";

    while (true) {
        std::string ref = lireLigne("\nReference produit (ou fin) : ");
        if (ref == "fin" || ref == "FIN") break;

        Produit p("", "", 0, 0);
        if (!trouverProduit(conn, ref, p)) {
            std::cout << "  Produit introuvable, reessaie.\n";
            continue;
        }
        int qte = lireEntier("Quantite souhaitee : ");
        try {
            commande.ajouterLigne(p, qte); // RG3 : verifie le stock cote application
            std::cout << "  Ajoute : " << p.getNom() << " x" << qte << "\n";
        } catch (const std::exception& e) {
            std::cout << "  Refuse : " << e.what() << " (RG3)\n";
        }
    }

    if (commande.getLignes().empty()) {
        std::cout << "\nPanier vide, commande annulee.\n";
        return;
    }

    std::cout << "\n--- Recapitulatif ---\n";
    commande.afficher();

    std::cout << "\nMode de paiement Mobile Money :\n"
              << "  1. Orange Money\n  2. Wave\n  3. MTN Money\n";
    int choixMode = lireEntier("Choix (1-3) : ");
    ModePaiement mode = choixMode == 2 ? ModePaiement::WAVE
                      : choixMode == 3 ? ModePaiement::MTN_MONEY
                      : ModePaiement::ORANGE_MONEY;

    std::string confirmation = lireLigne("Confirmer et payer ? (o/n) : ");
    if (confirmation != "o" && confirmation != "O") {
        std::cout << "Commande annulee par le client.\n";
        return;
    }

    try {
        int commandeId = enregistrerCommande(conn, client, commande, mode);
        std::cout << "\nCommande #" << commandeId << " enregistree, statut : "
                  << statutToString(commande.getStatut()) << "\n";
    } catch (const std::exception& e) {
        std::cout << "Erreur lors de l'enregistrement : " << e.what() << "\n";
    }
}

void actionAnnuler(MYSQL* conn) {
    int id = lireEntier("\nNumero de la commande a annuler : ");
    if (annulerCommandeEnBase(conn, id)) {
        std::cout << "Commande #" << id << " annulee (RG10).\n";
    } else {
        std::cout << "Impossible d'annuler : commande introuvable ou deja "
                  << "payee/en cours de traitement (RG10).\n";
    }
}

void menuAdmin(MYSQL* conn) {
    bool retour = false;
    while (!retour) {
        std::cout << "\n--- Menu Administrateur ---\n"
                  << " 1. Voir les alertes de stock bas\n"
                  << " 2. Mettre a jour le stock d'un produit\n"
                  << " 0. Retour au menu principal\n";
        int choix = lireEntier("Choix : ");
        switch (choix) {
            case 1: afficherAlertesStock(conn); break;
            case 2: {
                std::string ref = lireLigne("Reference du produit : ");
                int qte = lireEntier("Quantite a AJOUTER au stock (reapprovisionnement) : ");
                mysql_query(conn, ("UPDATE produit SET qte_stock = qte_stock + " +
                                    std::to_string(qte) + " WHERE reference='" + ref + "'").c_str());
                std::cout << "Stock mis a jour.\n";
                break;
            }
            case 0: retour = true; break;
            default: std::cout << "Choix invalide.\n";
        }
    }
}

// ============================================================
// Programme principal
// ============================================================
int main() {
    MYSQL* conn = connecter();
    if (!conn) return 1;

    // Client de demonstration (RG1 : compte deja cree)
    Client client(42, "Kouassi", "Awa", "awa.kouassi@mail.ci",
                  "+225 07 00 00 00 00", "Cocody, Rue des Jardins", "Abidjan");

    bool quitter = false;
    while (!quitter) {
        std::cout << "\n=========================================\n"
                  << "   NUTRICI - Plateforme e-commerce\n"
                  << "   Connecte en tant que : " << client.getNomComplet() << "\n"
                  << "=========================================\n"
                  << " 1. Consulter le catalogue\n"
                  << " 2. Rechercher un produit\n"
                  << " 3. Passer une commande\n"
                  << " 4. Consulter mon historique de commandes\n"
                  << " 5. Annuler une commande (en attente)\n"
                  << " 6. [Administrateur] Gerer le stock / alertes\n"
                  << " 0. Quitter\n";
        int choix = lireEntier("Choix : ");

        switch (choix) {
            case 1: actionCatalogue(conn); break;
            case 2: actionRecherche(conn); break;
            case 3: actionPasserCommande(conn, client); break;
            case 4: afficherHistorique(conn, client.getId()); break;
            case 5: actionAnnuler(conn); break;
            case 6: menuAdmin(conn); break;
            case 0: quitter = true; break;
            default: std::cout << "Choix invalide, reessaie.\n";
        }
    }

    std::cout << "\nA bientot sur NutriCI !\n";
    mysql_close(conn);
    return 0;
}