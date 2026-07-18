#ifndef LIGNE_COMMANDE_H
#define LIGNE_COMMANDE_H

#include "Produit.h"

// RG2 : chaque ligne de commande fige le produit, la quantite et le prix
// facture au moment de la commande (independant d'une future evolution du
// prix catalogue).
class LigneCommande {
private:
    Produit produit;
    int quantite;
    double prixFacture;

public:
    LigneCommande(Produit p, int qte)
        : produit(p), quantite(qte),
          prixFacture(p.getPrix() * qte) {}

    double getMontant() const { return prixFacture; }
    int getQuantite() const { return quantite; }
    Produit getProduit() const { return produit; }

    void afficher() const {
        std::cout << "  " << produit.getReference()
                  << " x" << quantite
                  << " = " << prixFacture << " FCFA"
                  << std::endl;
    }
};

#endif
