#ifndef PAIEMENT_H
#define PAIEMENT_H

#include <string>
#include <iostream>

enum class ModePaiement { ORANGE_MONEY, WAVE, MTN_MONEY };
enum class StatutPaiement { EN_COURS, VALIDE, ECHOUE };

inline std::string modeToString(ModePaiement m) {
    switch (m) {
        case ModePaiement::ORANGE_MONEY: return "ORANGE_MONEY";
        case ModePaiement::WAVE:         return "WAVE";
        case ModePaiement::MTN_MONEY:    return "MTN_MONEY";
    }
    return "";
}

// RG4 : le paiement se fait via Mobile Money ; une commande non payee
// dans les 24h est automatiquement annulee (verifie cote systeme/CRON).
class Paiement {
private:
    int id;
    int commandeId;
    double montant;
    ModePaiement mode;
    std::string referenceTransaction;
    StatutPaiement statut;

public:
    Paiement(int id, int commandeId, double montant, ModePaiement mode)
        : id(id), commandeId(commandeId), montant(montant), mode(mode),
          referenceTransaction(""), statut(StatutPaiement::EN_COURS) {}

    void valider(const std::string& refTransaction) {
        referenceTransaction = refTransaction;
        statut = StatutPaiement::VALIDE;
    }

    void echouer() { statut = StatutPaiement::ECHOUE; }

    bool estValide() const { return statut == StatutPaiement::VALIDE; }
    double getMontant() const { return montant; }

    void afficher() const {
        std::cout << "Paiement #" << id << " - " << montant << " FCFA via "
                  << modeToString(mode) << " ["
                  << (statut == StatutPaiement::VALIDE ? "VALIDE" :
                      statut == StatutPaiement::ECHOUE ? "ECHOUE" : "EN_COURS")
                  << "]" << std::endl;
    }
};

#endif
