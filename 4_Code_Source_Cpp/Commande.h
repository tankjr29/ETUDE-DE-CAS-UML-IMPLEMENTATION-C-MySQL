#ifndef COMMANDE_H
#define COMMANDE_H

#include "LigneCommande.h"
#include <vector>
#include <string>
#include <stdexcept>

// RG5 : EN_ATTENTE -> PAYEE -> PREPARATION -> EXPEDIEE -> LIVREE
// Le passage inverse est interdit ; ANNULEE n'est atteignable que
// depuis EN_ATTENTE (RG10, ou timeout 24h RG4).
enum class StatutCommande {
    EN_ATTENTE, PAYEE, PREPARATION, EXPEDIEE, LIVREE, ANNULEE
};

inline std::string statutToString(StatutCommande s) {
    switch (s) {
        case StatutCommande::EN_ATTENTE:  return "EN_ATTENTE";
        case StatutCommande::PAYEE:       return "PAYEE";
        case StatutCommande::PREPARATION: return "PREPARATION";
        case StatutCommande::EXPEDIEE:    return "EXPEDIEE";
        case StatutCommande::LIVREE:      return "LIVREE";
        case StatutCommande::ANNULEE:     return "ANNULEE";
    }
    return "";
}

class Commande {
private:
    int id;
    int clientId;
    StatutCommande statut;
    std::vector<LigneCommande> lignes;
    double fraisLivraison;

    // RG5 : table de transition autorisee
    bool transitionAutorisee(StatutCommande de, StatutCommande vers) const {
        switch (de) {
            case StatutCommande::EN_ATTENTE:
                return vers == StatutCommande::PAYEE || vers == StatutCommande::ANNULEE;
            case StatutCommande::PAYEE:
                return vers == StatutCommande::PREPARATION;
            case StatutCommande::PREPARATION:
                return vers == StatutCommande::EXPEDIEE;
            case StatutCommande::EXPEDIEE:
                return vers == StatutCommande::LIVREE;
            default:
                return false; // LIVREE et ANNULEE sont des etats terminaux
        }
    }

public:
    Commande(int id, int clientId, double fraisLivraisonBase = 1500)
        : id(id), clientId(clientId),
          statut(StatutCommande::EN_ATTENTE), fraisLivraison(fraisLivraisonBase) {}

    int getId() const { return id; }
    StatutCommande getStatut() const { return statut; }

    void ajouterLigne(Produit p, int qte) {
        // RG3 : refus si stock insuffisant
        if (!p.verifierStock(qte)) {
            throw std::runtime_error("Commande refusee : stock insuffisant pour " + p.getReference());
        }
        lignes.push_back(LigneCommande(p, qte));
    }

    double calculerTotal() const {
        double total = 0;
        for (const auto& l : lignes)
            total += l.getMontant();
        // RG9 : livraison gratuite au-dela de 25 000 FCFA
        if (total >= 25000) return total;
        return total + fraisLivraison;
    }

    // RG5 : changement de statut controle
    void changerStatut(StatutCommande nouveau) {
        if (!transitionAutorisee(statut, nouveau)) {
            throw std::runtime_error("Transition interdite : " +
                statutToString(statut) + " -> " + statutToString(nouveau));
        }
        statut = nouveau;
    }

    // RG10 : annulation possible uniquement depuis EN_ATTENTE
    bool annuler() {
        if (statut != StatutCommande::EN_ATTENTE) return false;
        statut = StatutCommande::ANNULEE;
        return true;
    }

    const std::vector<LigneCommande>& getLignes() const { return lignes; }

    void afficher() const {
        std::cout << "=== Commande #" << id
                  << " (" << statutToString(statut) << ") ===" << std::endl;
        for (const auto& l : lignes) l.afficher();
        std::cout << "  Total : " << calculerTotal()
                  << " FCFA" << std::endl;
    }
};

#endif
