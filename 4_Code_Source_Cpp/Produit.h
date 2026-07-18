#ifndef PRODUIT_H
#define PRODUIT_H

#include <string>
#include <iostream>
#include <iomanip>
#include <stdexcept>

class Produit {
private:
    std::string reference;
    std::string nom;
    double prixUnitaire;
    int qteStock;
    bool actif;

public:
    // Constructeur
    Produit(std::string ref, std::string nom,
            double prix, int stock, bool actif = true)
        : reference(ref), nom(nom), prixUnitaire(prix),
          qteStock(stock), actif(actif) {}

    // Getters
    std::string getReference() const { return reference; }
    std::string getNom() const { return nom; }
    double getPrix() const { return prixUnitaire; }
    int getStock() const { return qteStock; }
    bool isActif() const { return actif; }

    // RG8 : alerte si stock < 5
    bool isStockBas() const { return qteStock < 5; }

    // RG3 : verification de la disponibilite avant validation de commande
    bool verifierStock(int qteDemandee) const {
        return actif && qteStock >= qteDemandee;
    }

    // RG3 : decrement automatique du stock a la validation de la commande
    void decrementerStock(int qte) {
        if (!verifierStock(qte)) {
            throw std::runtime_error("Stock insuffisant pour " + reference);
        }
        qteStock -= qte;
    }

    // RG6 : un produit ne se supprime jamais, on le desactive
    void desactiver() { actif = false; }

    // Affichage
    void afficher() const {
        std::cout << std::left << std::setw(8) << reference
                  << std::setw(35) << nom
                  << std::right << std::setw(8) << prixUnitaire
                  << " FCFA (Stock: " << qteStock << ")"
                  << (isStockBas() ? " [ALERTE STOCK BAS]" : "")
                  << (!actif ? " [DESACTIVE]" : "")
                  << std::endl;
    }
};

#endif
