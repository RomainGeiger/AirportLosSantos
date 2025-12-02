#include "ATC.hpp"

// --- Logger Static ---
std::mutex Logger::logMutex;
void Logger::log(const std::string& actor, const std::string& message) {
    std::lock_guard<std::mutex> lock(logMutex);
    // Ici, vous pourriez écrire dans un fichier JSON
    // std::cout << "[JSON LOG] Actor: " << actor << " | Action: " << message << std::endl;
}

// --- Implementation Aircraft ---
Aircraft::Aircraft(std::string id, sf::Vector2f startPos, sf::Vector2f target, float speed)
    : m_id(id), m_position(startPos), m_targetPosition(target), m_speed(speed),
    m_state(FlightState::CRUISE), m_running(false) {
}

Aircraft::~Aircraft() {
    stopEngine();
}

void Aircraft::startEngine() {
    m_running = true;
    m_thread = std::thread(&Aircraft::run, this);
    Logger::log("Aircraft " + m_id, "Engine Started");
}

void Aircraft::stopEngine() {
    m_running = false;
    if (m_thread.joinable()) m_thread.join();
}

void Aircraft::setTarget(sf::Vector2f newTarget) {
    std::lock_guard<std::mutex> lock(m_dataMutex);
    m_targetPosition = newTarget;
}

void Aircraft::setState(FlightState newState) {
    std::lock_guard<std::mutex> lock(m_dataMutex);
    m_state = newState;
}

FlightState Aircraft::getState() const {
    return m_state;
}

std::string Aircraft::getId() const {
    return m_id;
}

sf::Vector2f Aircraft::getPosition() {
    std::lock_guard<std::mutex> lock(m_dataMutex);
    return m_position;
}
float Aircraft::getRotation() {
    std::lock_guard<std::mutex> lock(m_dataMutex);
    sf::Vector2f dir = m_targetPosition - m_position;
    return std::atan2(dir.y, dir.x) * 180.0f / 3.14159f;
}

void Aircraft::run() {
    // Boucle de simulation de l'agent (ex: 60Hz logic)
    while (m_running) {
        {
            std::lock_guard<std::mutex> lock(m_dataMutex);

            // Déplacement simple vers la cible
            sf::Vector2f direction = m_targetPosition - m_position;
            float length = std::sqrt(direction.x * direction.x + direction.y * direction.y);

            if (length > 2.0f) { // Si pas encore arrivé
                sf::Vector2f normalized = direction / length;
                m_position += normalized * m_speed; // Vitesse arbitraire par tick
            }
            else {
                // Arrivé à destination (logique simple pour l'exemple)
                if (m_state == FlightState::LANDING) {
                    m_state = FlightState::TAXI;
                }
            }
        }
        // Simulation d'un délai de traitement (simuler le temps réel)
        std::this_thread::sleep_for(std::chrono::milliseconds(16));
    }
}

// --- Implementation TWR ---
TWR::TWR(std::string airportName) : m_name(airportName), m_runwayBusy(false) {}

bool TWR::requestTakeoff(std::shared_ptr<Aircraft> plane) {
    std::lock_guard<std::mutex> lock(m_twrMutex);
    // Logique simplifiée : si la piste n'est pas occupée, on autorise
    if (!m_runwayBusy) {
        m_runwayBusy = true;
        Logger::log("TWR " + m_name, "Takeoff clearance for " + plane->getId());

        // Timer pour libérer la piste
        std::thread([this]() {
            std::this_thread::sleep_for(std::chrono::seconds(5));
            this->m_runwayBusy = false;
            }).detach();
        return true;
    }
    return false;
}

void TWR::update() {
    // Logique de gestion de la file d'attente au sol (à implémenter plus tard)
    // Pour l'instant vide pour que ça compile
}

bool TWR::requestLanding(std::shared_ptr<Aircraft> plane) {
    std::lock_guard<std::mutex> lock(m_twrMutex);
    if (!m_runwayBusy) {
        m_runwayBusy = true;
        Logger::log("TWR " + m_name, "Landing clearance granted for " + plane->getId());

        // Timer fictif pour libérer la piste après 5 secondes
        std::thread([this]() {
            std::this_thread::sleep_for(std::chrono::seconds(5));
            this->m_runwayBusy = false;
            Logger::log("TWR " + m_name, "Runway is now free");
            }).detach();

        return true;
    }
    return false;
}

// --- Implementation APP ---
APP::APP(std::string airportName, sf::Vector2f location, TWR* twrRef)
    : m_name(airportName), m_location(location), m_linkedTWR(twrRef) {
}

void APP::registerAircraft(std::shared_ptr<Aircraft> plane) {
    std::lock_guard<std::mutex> lock(m_appMutex);
    m_planesInZone.push_back(plane);
    plane->setState(FlightState::APPROACH);
    plane->setTarget(m_location); // L'APP guide l'avion vers l'aéroport
    Logger::log("APP " + m_name, "Aircraft " + plane->getId() + " under control");
}

void APP::update() {
    // Thread ou appel cyclique gérant l'approche
    std::lock_guard<std::mutex> lock(m_appMutex);

    for (auto& plane : m_planesInZone) {
        // Distance simple pour déclencher la demande d'atterrissage
        sf::Vector2f dist = plane->getPosition() - m_location;
        float d = std::sqrt(dist.x * dist.x + dist.y * dist.y);

        if (d < 50.0f && plane->getState() == FlightState::APPROACH) {
            // Demande à la tour
            if (m_linkedTWR->requestLanding(plane)) {
                plane->setState(FlightState::LANDING);
                Logger::log("APP " + m_name, "Handoff to TWR for " + plane->getId());
            }
        }
    }
}

// --- Implementation Airport ---
Airport::Airport(std::string name, sf::Vector2f position)
    : m_name(name), m_position(position) {
    m_twr = std::make_unique<TWR>(name);
    m_app = std::make_unique<APP>(name, position, m_twr.get());
}

// --- Implementation CCR ---
CCR::CCR() : m_running(false) {}
CCR::~CCR() { stopControl(); }

void CCR::addFlight(std::shared_ptr<Aircraft> plane) {
    std::lock_guard<std::mutex> lock(m_ccrMutex);
    m_flights.push_back(plane);
    plane->startEngine(); // Lance le thread de l'avion
}

void CCR::addAirport(std::shared_ptr<Airport> airport) {
    m_airports.push_back(airport);
}

void CCR::startControl() {
    m_running = true;
    m_thread = std::thread(&CCR::run, this);
}

void CCR::stopControl() {
    m_running = false;
    if (m_thread.joinable()) m_thread.join();
}

std::vector<std::shared_ptr<Aircraft>> CCR::getFlights() {
    std::lock_guard<std::mutex> lock(m_ccrMutex);
    return m_flights;
}

void CCR::run() {
    while (m_running) {
        {
            std::lock_guard<std::mutex> lock(m_ccrMutex);

            // Gestion En-Route : Vérifier si un avion est proche d'un aéroport
            for (auto& plane : m_flights) {
                if (plane->getState() == FlightState::CRUISE) {
                    sf::Vector2f pPos = plane->getPosition();

                    for (auto& airport : m_airports) {
                        sf::Vector2f aPos = airport->getPosition();
                        float dist = std::hypot(pPos.x - aPos.x, pPos.y - aPos.y);

                        // Si avion à moins de 200px de l'aéroport -> Transfert à l'APP
                        if (dist < 200.0f) {
                            Logger::log("CCR", "Handoff " + plane->getId() + " to APP " + airport->getName());
                            airport->getAPP().registerAircraft(plane);
                        }
                    }
                }
            }

            // Update des APPs (pourraient avoir leurs propres threads aussi)
            for (auto& airport : m_airports) {
                airport->getAPP().update();
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
}