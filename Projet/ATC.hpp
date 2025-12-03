#include <iostream>
#include <vector>
#include <string>
#include <thread>
#include <mutex>
#include <atomic>
#include <cmath>
#include <memory>
#include <SFML/Graphics.hpp>

// États possibles d'un avion
enum class EtatVol {
    CROISIERE,      // Géré par le CCR
    APPROCHE,       // Géré par l'Approche (APP)
    ATTERRISSAGE,   // Géré par la Tour (TWR)
    ROULAGE,        // Sur le tarmac
    PARKING,        // Garé
    DECOLLAGE       // Sur la piste
};

// Petite classe utilitaire pour écrire dans la console sans bug d'affichage
struct Journal {
    static std::mutex mutexEcriture;
    static void ecrire(const std::string& acteur, const std::string& message);
};

// --- Classe Avion ---
class Avion {
public:
    Avion(std::string id, sf::Vector2f posDepart, sf::Vector2f posCible, float vitesse);
    ~Avion();

    // Gestion du moteur (Thread)
    void demarrer();
    void arreter();

    // Interaction
    void definirCible(sf::Vector2f nouvelleCible);
    void changerEtat(EtatVol nouvelEtat);
    EtatVol getEtat() const;
    std::string getId() const;

    // Pour l'affichage SFML
    sf::Vector2f getPosition();
    float getRotation();

private:
    void boucleVol(); // La fonction qui tourne dans le thread

    std::string m_id;
    sf::Vector2f m_position;
    sf::Vector2f m_cible;
    float m_vitesse;
    EtatVol m_etat;

    // Gestion technique (Thread et protection des données)
    std::atomic<bool> m_enVol;
    std::thread m_threadVol;
    mutable std::mutex m_mutexDonnees;
};

// --- Tour de Contrôle (TWR) ---
class Tour {
public:
    Tour(std::string nomAeroport);

    // Retourne vrai si la demande est acceptée
    bool demanderAtterrissage(std::shared_ptr<Avion> avion);
    bool demanderDecollage(std::shared_ptr<Avion> avion);

    void actualiser();

private:
    std::string m_nom;
    bool m_pisteOccupee; // Une seule piste pour faire simple
    std::mutex m_mutexTour;
};

// --- Contrôle d'Approche (APP) ---
class Approche {
public:
    Approche(std::string nomAeroport, sf::Vector2f position, Tour* tourAssociee);

    void ajouterAvion(std::shared_ptr<Avion> avion);
    void actualiser(); // Gère les avions proches
    sf::Vector2f getPosition() const { return m_position; }

private:
    std::string m_nom;
    sf::Vector2f m_position;
    Tour* m_tourAssociee;
    std::vector<std::shared_ptr<Avion>> m_avionsSousControle;
    std::mutex m_mutexApp;
};

// --- Aéroport (Contient Tour + Approche) ---
class Aeroport {
public:
    Aeroport(std::string nom, sf::Vector2f position);

    Approche& getApproche() { return *m_approche; }
    Tour& getTour() { return *m_tour; }

    sf::Vector2f getPosition() const { return m_position; }
    std::string getNom() const { return m_nom; }

private:
    std::string m_nom;
    sf::Vector2f m_position;
    std::unique_ptr<Tour> m_tour;
    std::unique_ptr<Approche> m_approche;
};

// --- Centre de Contrôle Régional (CCR) ---
class CCR {
public:
    CCR();
    ~CCR();

    void ajouterVol(std::shared_ptr<Avion> avion);
    void ajouterAeroport(std::shared_ptr<Aeroport> aeroport);

    void lancerSimulation();
    void arreterSimulation();

    // Pour dessiner dans le main
    std::vector<std::shared_ptr<Avion>> recupererVols();

private:
    void boucleControle(); // Thread principal du CCR

    std::vector<std::shared_ptr<Avion>> m_vols;
    std::vector<std::shared_ptr<Aeroport>> m_aeroports;

    std::atomic<bool> m_actif;
    std::thread m_threadCCR;
    std::mutex m_mutexCCR;
};
