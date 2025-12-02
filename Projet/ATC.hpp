#ifndef ATC_H
#define ATC_H

#include <iostream>
#include <vector>
#include <string>
#include <thread>
#include <mutex>
#include <atomic>
#include <cmath>
#include <memory>
#include <queue>
#include <SFML/Graphics.hpp>

// États possibles d'un avion (selon le PDF)
enum class FlightState {
    CRUISE,     // Géré par CCRS
    APPROACH,   // Géré par APP
    LANDING,    // Géré par TWR
    TAXI,       // Géré par TWR (Roulage)
    PARKED,     // Géré par TWR
    TAKEOFF     // Géré par TWR
};

// Logger simple pour simuler la traçabilité JSON
struct Logger {
    static std::mutex logMutex;
    static void log(const std::string& actor, const std::string& message);
};

// --- Classe Avion (Agent Autonome) ---
class Aircraft {
public:
    Aircraft(std::string id, sf::Vector2f startPos, sf::Vector2f target, float speed);
    ~Aircraft();

    // Méthodes pour le thread principal (Physique)
    void startEngine();
    void stopEngine();

    // Méthodes pour l'interaction avec les contrôleurs
    void setTarget(sf::Vector2f newTarget);
    void setState(FlightState newState);
    FlightState getState() const;
    std::string getId() const;

    // Méthodes pour l'affichage SFML (Thread-safe)
    sf::Vector2f getPosition();
    float getRotation();

private:
    void run(); // Boucle principale du thread de l'avion

    std::string m_id;
    sf::Vector2f m_position;
    sf::Vector2f m_targetPosition;
    float m_speed;
    FlightState m_state;

    // Gestion du Thread
    std::atomic<bool> m_running;
    std::thread m_thread;
    mutable std::mutex m_dataMutex; // Protège position et état pour l'affichage
};

// --- Tour de Contrôle (TWR) ---
class TWR {
public:
    TWR(std::string airportName);
    bool requestLanding(std::shared_ptr<Aircraft> plane);
    bool requestTakeoff(std::shared_ptr<Aircraft> plane);
    void update(); // Logique de gestion de la piste

private:
    std::string m_name;
    bool m_runwayBusy;
    std::mutex m_twrMutex;
};

// --- Contrôle d'Approche (APP) ---
class APP {
public:
    APP(std::string airportName, sf::Vector2f location, TWR* twrRef);
    void registerAircraft(std::shared_ptr<Aircraft> plane);
    void update(); // Gère les trajectoires d'approche
    sf::Vector2f getLocation() const { return m_location; }

private:
    std::string m_name;
    sf::Vector2f m_location;
    TWR* m_linkedTWR; // Lien vers la tour associée
    std::vector<std::shared_ptr<Aircraft>> m_planesInZone;
    std::mutex m_appMutex;
};

// --- Structure Aéroport (Conteneur Logique & Graphique) ---
class Airport {
public:
    Airport(std::string name, sf::Vector2f position);

    APP& getAPP() { return *m_app; }
    TWR& getTWR() { return *m_twr; }
    sf::Vector2f getPosition() const { return m_position; }
    std::string getName() const { return m_name; }

private:
    std::string m_name;
    sf::Vector2f m_position;
    std::unique_ptr<TWR> m_twr;
    std::unique_ptr<APP> m_app;
};

// --- Centre de Contrôle Régional (CCR) ---
class CCR {
public:
    CCR();
    ~CCR();

    void addFlight(std::shared_ptr<Aircraft> plane);
    void addAirport(std::shared_ptr<Airport> airport);

    void startControl();
    void stopControl();

    // Pour l'affichage dans le main
    std::vector<std::shared_ptr<Aircraft>> getFlights();

private:
    void run(); // Boucle de gestion globale (collisions, hand-off)

    std::vector<std::shared_ptr<Aircraft>> m_flights;
    std::vector<std::shared_ptr<Airport>> m_airports;

    std::atomic<bool> m_running;
    std::thread m_thread;
    std::mutex m_ccrMutex;
};

#endif // ATC_H