#include <iostream>
#include <mysql/mysql.h>
#include <vector>
#include <sstream>
#include "Produit.h"
#include "Client.h"
#include "Commande.h"
#include "Paiement.h"

// Connexion MySQL
MYSQL* connecter() {
    MYSQL* conn = mysql_init(NULL);
    if (!mysql_real_connect(conn, "localhost", "root",
                             "password", "nutrici", 3306, NULL, 0)) {
        std::cerr << "Erreur MySQL : " << mysql_error(conn) << std::endl;
        return nullptr;
    }
    std::cout << "Connexion MySQL OK." << std::endl;
    return conn;
}

// Charger les produits actifs depuis la BDD
std::vector<Produit> listerProduits(MYSQL* conn) {
    std::vector<Produit> produits;
    mysql_query(conn,
        "SELECT reference, nom, prix_unitaire, qte_stock, actif "
        "FROM produit ORDER BY nom");
    MYSQL_RES* res = mysql_store_result(conn);
    MYSQL_ROW row;
    while ((row = mysql_fetch_row(res))) {
        produits.push_back(Produit(
            row[0], row[1],
            std::stod(row[2]), std::stoi(row[3]),
            std::string(row[4]) == "1"
        ));
    }
    mysql_free_result(res);
    return produits;
}

// RG3 + RG8 : verifie le stock, decremente en base, declenche l'alerte
bool decrementerStockEnBase(MYSQL* conn, const std::string& ref, int qte) {
    std::ostringstream check;
    check << "SELECT qte_stock FROM produit WHERE reference='" << ref << "'";
    mysql_query(conn, check.str().c_str());
    MYSQL_RES* res = mysql_store_result(conn);
    MYSQL_ROW row = mysql_fetch_row(res);
    int stockActuel = row ? std::stoi(row[0]) : 0;
    mysql_free_result(res);

    if (stockActuel < qte) return false; // RG3 : commande refusee

    std::ostringstream upd;
    upd << "UPDATE produit SET qte_stock = qte_stock - " << qte
        << " WHERE reference='" << ref << "'";
    mysql_query(conn, upd.str().c_str());

    if (stockActuel - qte < 5) { // RG8
        std::cout << "  [ALERTE ADMIN] Stock de " << ref
                  << " sous le seuil critique (" << (stockActuel - qte)
                  << " restants)" << std::endl;
    }
    return true;
}

// Persiste la commande, ses lignes et son paiement (RG2, RG4, RG5)
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
                 << "', " << ligne.getQuantite() << ", "
                 << ligne.getMontant() << ")";
        mysql_query(conn, insLigne.str().c_str());

        // RG3/RG8 : decrement du stock + alerte
        decrementerStockEnBase(conn, ligne.getProduit().getReference(),
                                ligne.getQuantite());
    }

    // Simulation de l'appel a l'API Mobile Money (RG4)
    Paiement paiement(0, commandeId, commande.calculerTotal(), mode);
    paiement.valider("TXN-DEMO-0001"); // reponse simulee de l'API

    std::ostringstream insPay;
    insPay << "INSERT INTO paiement (commande_id, montant, mode, "
              "reference_transaction, statut) VALUES ("
           << commandeId << ", " << paiement.getMontant() << ", '"
           << modeToString(mode) << "', 'TXN-DEMO-0001', 'VALIDE')";
    mysql_query(conn, insPay.str().c_str());

    // RG5 : la commande passe a PAYEE une fois le paiement confirme
    commande.changerStatut(StatutCommande::PAYEE);
    mysql_query(conn, ("UPDATE commande SET statut='PAYEE' WHERE id=" +
                        std::to_string(commandeId)).c_str());

    return commandeId;
}

int main() {
    MYSQL* conn = connecter();
    if (!conn) return 1;

    // 1. Lister le catalogue (USC02)
    auto produits = listerProduits(conn);
    std::cout << "\n=== Catalogue NutriCI (" << produits.size()
              << " produits) ===\n";
    for (const auto& p : produits) p.afficher();

    // 2. Client (USC01 : compte deja cree, RG1)
    Client client(42, "Kouassi", "Awa", "awa.kouassi@mail.ci",
                  "+225 07 00 00 00 00", "Cocody, Rue des Jardins", "Abidjan");
    std::cout << std::endl;
    client.afficher();

    // 3. Passer une commande (USC04-06)
    try {
        Commande cmd(0, client.getId(), client.getFraisLivraisonBase());
        cmd.ajouterLigne(produits[0], 2); // 2x Vitamine C (RG3 verifie le stock)
        cmd.ajouterLigne(produits[1], 1); // 1x Whey Protein
        std::cout << std::endl;
        cmd.afficher();

        // 4. Paiement + persistance (RG2, RG4, RG5, RG8, RG9)
        int commandeId = enregistrerCommande(conn, client, cmd,
                                              ModePaiement::ORANGE_MONEY);
        std::cout << "\nCommande #" << commandeId
                  << " enregistree avec le statut : "
                  << statutToString(cmd.getStatut()) << std::endl;

    } catch (const std::exception& e) {
        std::cerr << "Erreur : " << e.what() << std::endl;
    }

    mysql_close(conn);
    return 0;
}
