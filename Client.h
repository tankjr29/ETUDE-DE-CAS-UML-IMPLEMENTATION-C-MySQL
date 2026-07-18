#ifndef CLIENT_H
#define CLIENT_H

#include <string>
#include <iostream>

// RG1 : un client doit fournir nom, prenom, email, telephone et adresse
// avant de pouvoir passer commande.
class Client {
private:
    int id;
    std::string nom;
    std::string prenom;
    std::string email;
    std::string telephone;
    std::string adresseLivraison;
    std::string ville;

public:
    Client(int id, std::string nom, std::string prenom, std::string email,
           std::string telephone, std::string adresseLivraison,
           std::string ville = "Abidjan")
        : id(id), nom(nom), prenom(prenom), email(email),
          telephone(telephone), adresseLivraison(adresseLivraison),
          ville(ville) {}

    int getId() const { return id; }
    std::string getNomComplet() const { return prenom + " " + nom; }
    std::string getAdresse() const { return adresseLivraison; }
    std::string getVille() const { return ville; }

    // RG9 : la ville determine les frais de livraison
    double getFraisLivraisonBase() const {
        return (ville == "Abidjan") ? 1500.0 : 3000.0;
    }

    void afficher() const {
        std::cout << "Client #" << id << " - " << getNomComplet()
                  << " (" << ville << ")" << std::endl;
    }
};

#endif
