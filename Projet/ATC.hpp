#ifndef ATC_HPP
#define ATC_HPP

#include <iostream>
#include <vector>
#include <string>
#include <thread>
#include <mutex>
#include <atomic>
#include <cmath>
#include <memory>
#include <fstream>
#include <chrono>
#include <SFML/Graphics.hpp>

enum class EtatVol {
    CROISIERE, APP_ENTREE, APP_FINALE, ATTERRISSAGE,
    ROULAGE_VERS_PARKING, PARKING, ROULAGE_VERS_PISTE, DECOLLAGE
};

struct Journal {
    static std::mutex mutexEcriture;
    static std::ofstream fichierLog;
    static void initialiser();
    static void ecrire(const std::string& acteur, const std::string& action);
};

class Avion {
public:
    Avion(std::string id, sf::Vector2f posDepart, float vitesse);
    ~Avion();

    void demarrer();
    void arreter();

    void definirCible(sf::Vector2f nouvelleCible);
    void changerEtat(EtatVol nouvelEtat);
    void definirVitesse(float v);
    void setCompteurParking(int ticks); // <--- NOUVEAU

    EtatVol getEtat() const;
    std::string getId() const;
    sf::Vector2f getPosition();
    sf::Vector2f getCible();
    float getRotation();

    float getCarburant() const;
    float getCarburantMax() const;
    int getCompteurParking() const;     // <--- NOUVEAU

private:
    void boucleVol();

    std::string m_id;
    sf::Vector2f m_position;
    sf::Vector2f m_cible;
    float m_vitesse;
    float m_vitesseStandard; // Pour re-accélérer après freinage

    float m_carburant;
    float m_carburantMax;
    float m_consommation;

    int m_compteurParking; // Temps d'attente au sol

    EtatVol m_etat;
    std::atomic<bool> m_enVol;
    std::thread m_threadVol;
    mutable std::mutex m_mutexDonnees;
};

class Tour {
public:
    Tour(std::string nom);
    bool demanderAccesPiste(std::shared_ptr<Avion> avion);
    void libererPiste();
    bool estOccupee();
private:
    std::string m_nom;
    std::atomic<bool> m_pisteOccupee;
    std::mutex m_mutexTour;
};

class Approche {
public:
    Approche(std::string nom, sf::Vector2f pos, Tour* tour);
    void ajouterAvion(std::shared_ptr<Avion> avion);
    void actualiser();
    sf::Vector2f getPosition() const { return m_position; }
    Tour* getTour() { return m_tourAssociee; }
private:
    std::string m_nom;
    sf::Vector2f m_position;
    Tour* m_tourAssociee;
    std::vector<std::shared_ptr<Avion>> m_avions;
    std::mutex m_mutexApp;
};

class Aeroport {
public:
    Aeroport(std::string nom, sf::Vector2f posPiste, sf::Vector2f posParking);
    Approche& getApproche() { return *m_approche; }
    Tour& getTour() { return *m_tour; }
    sf::Vector2f getPosition() const { return m_position; }
    sf::Vector2f getParking() const { return m_posParking; }
    std::string getNom() const { return m_nom; }
private:
    std::string m_nom;
    sf::Vector2f m_position;
    sf::Vector2f m_posParking;
    std::unique_ptr<Tour> m_tour;
    std::unique_ptr<Approche> m_approche;
};

class CCR {
public:
    CCR();
    ~CCR();
    void ajouterVol(std::shared_ptr<Avion> avion);
    void ajouterAeroport(std::shared_ptr<Aeroport> aeroport);
    void lancerSimulation();
    void arreterSimulation();
    std::vector<std::shared_ptr<Avion>> recupererVols();
private:
    void boucleControle();
    std::vector<std::shared_ptr<Avion>> m_vols;
    std::vector<std::shared_ptr<Aeroport>> m_aeroports;
    std::atomic<bool> m_actif;
    std::thread m_threadCCR;
    std::mutex m_mutexCCR;
};

#endif