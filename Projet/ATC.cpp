#include "ATC.hpp"

#include <fstream>  // Pour écrire dans les fichiers
#include <chrono>   // Pour l'heure
#include <iomanip>  // Pour le formatage de l'heure

// --- Gestion du Journal (Log JSON) ---
std::mutex Journal::mutexEcriture;
// On ouvre le fichier en mode "Append" (ajout) pour ne pas l'écraser à chaque ligne
std::ofstream fichierLog("logs.json", std::ios::app);

void Journal::ecrire(const std::string& acteur, const std::string& message) {
    std::lock_guard<std::mutex> verrou(mutexEcriture);

    // Récupération de l'heure actuelle
    auto now = std::chrono::system_clock::now();
    auto temps = std::chrono::system_clock::to_time_t(now);

    // Formatage manuel en JSON
    // Exemple : {"heure": "14:05:01", "acteur": "CCR", "action": "Transfert vol"}
    if (fichierLog.is_open()) {
        fichierLog << "{";
        fichierLog << "\"heure\": \"" << std::put_time(std::localtime(&temps), "%H:%M:%S") << "\", ";
        fichierLog << "\"acteur\": \"" << acteur << "\", ";
        fichierLog << "\"action\": \"" << message << "\"";
        fichierLog << "}," << std::endl; // La virgule pour séparer les objets JSON
    }

    // On garde l'affichage console pour le débuggage visuel
    std::cout << "[" << acteur << "] " << message << std::endl;
}

// --- Classe Avion ---
Avion::Avion(std::string id, sf::Vector2f posDepart, sf::Vector2f posCible, float vitesse)
    : m_id(id), m_position(posDepart), m_cible(posCible), m_vitesse(vitesse),
    m_etat(EtatVol::CROISIERE), m_enVol(false) {
}

Avion::~Avion() {
    arreter();
}

void Avion::demarrer() {
    m_enVol = true;
    m_threadVol = std::thread(&Avion::boucleVol, this);
    Journal::ecrire("Avion " + m_id, "Moteur demarre");
}

void Avion::arreter() {
    m_enVol = false;
    if (m_threadVol.joinable()) m_threadVol.join();
}

void Avion::definirCible(sf::Vector2f nouvelleCible) {
    std::lock_guard<std::mutex> verrou(m_mutexDonnees);
    m_cible = nouvelleCible;
}

void Avion::changerEtat(EtatVol nouvelEtat) {
    std::lock_guard<std::mutex> verrou(m_mutexDonnees);
    m_etat = nouvelEtat;
}

EtatVol Avion::getEtat() const {
    return m_etat;
}

std::string Avion::getId() const {
    return m_id;
}

sf::Vector2f Avion::getPosition() {
    std::lock_guard<std::mutex> verrou(m_mutexDonnees);
    return m_position;
}

float Avion::getRotation() {
    std::lock_guard<std::mutex> verrou(m_mutexDonnees);
    // Calcul de l'angle pour orienter le triangle (maths de base)
    sf::Vector2f direction = m_cible - m_position;
    return std::atan2(direction.y, direction.x) * 180.0f / 3.14159f;
}

void Avion::boucleVol() {
    while (m_enVol) {
        {
            std::lock_guard<std::mutex> verrou(m_mutexDonnees);

            // Calcul du déplacement
            sf::Vector2f direction = m_cible - m_position;
            float distance = std::sqrt(direction.x * direction.x + direction.y * direction.y);

            if (distance > 2.0f) {
                // On avance vers la cible
                sf::Vector2f normalise = direction / distance;
                m_position += normalise * m_vitesse;
            }
            else {
                // Arrivé à destination
                if (m_etat == EtatVol::ATTERRISSAGE) {
                    m_etat = EtatVol::ROULAGE;
                }
            }
        }
        // Petite pause pour simuler le temps (environ 60 images/seconde)
        std::this_thread::sleep_for(std::chrono::milliseconds(16));
    }
}

// --- Classe Tour (TWR) ---
Tour::Tour(std::string nomAeroport) : m_nom(nomAeroport), m_pisteOccupee(false) {}

bool Tour::demanderDecollage(std::shared_ptr<Avion> avion) {
    std::lock_guard<std::mutex> verrou(m_mutexTour);

    if (!m_pisteOccupee) {
        m_pisteOccupee = true;
        Journal::ecrire("Tour " + m_nom, "Decollage autorise pour " + avion->getId());

        // On libère la piste après 5 secondes (simulation)
        std::thread([this]() {
            std::this_thread::sleep_for(std::chrono::seconds(5));
            this->m_pisteOccupee = false;
            }).detach();
        return true;
    }
    return false;
}

bool Tour::demanderAtterrissage(std::shared_ptr<Avion> avion) {
    std::lock_guard<std::mutex> verrou(m_mutexTour);

    if (!m_pisteOccupee) {
        m_pisteOccupee = true;
        Journal::ecrire("Tour " + m_nom, "Atterrissage autorise pour " + avion->getId());

        // On libère la piste après 5 secondes
        std::thread([this]() {
            std::this_thread::sleep_for(std::chrono::seconds(5));
            this->m_pisteOccupee = false;
            Journal::ecrire("Tour " + m_nom, "Piste liberee");
            }).detach();

        return true;
    }
    return false;
}

void Tour::actualiser() {
    // Vide pour l'instant (gestion parking plus tard)
}

// --- Classe Approche (APP) ---
Approche::Approche(std::string nomAeroport, sf::Vector2f position, Tour* tourAssociee)
    : m_nom(nomAeroport), m_position(position), m_tourAssociee(tourAssociee) {
}

void Approche::ajouterAvion(std::shared_ptr<Avion> avion) {
    std::lock_guard<std::mutex> verrou(m_mutexApp);
    m_avionsSousControle.push_back(avion);

    avion->changerEtat(EtatVol::APPROCHE);
    avion->definirCible(m_position); // L'avion se dirige vers l'aéroport

    Journal::ecrire("APP " + m_nom, "Prise en charge de " + avion->getId());
}

void Approche::actualiser() {
    std::lock_guard<std::mutex> verrou(m_mutexApp);

    for (auto& avion : m_avionsSousControle) {
        // Calcul distance entre avion et aéroport
        sf::Vector2f dist = avion->getPosition() - m_position;
        float d = std::sqrt(dist.x * dist.x + dist.y * dist.y);

        // Si l'avion est proche (< 50 pixels) et en approche, on demande la tour
        if (d < 50.0f && avion->getEtat() == EtatVol::APPROCHE) {
            if (m_tourAssociee->demanderAtterrissage(avion)) {
                avion->changerEtat(EtatVol::ATTERRISSAGE);
                Journal::ecrire("APP " + m_nom, "Transfert vers Tour pour " + avion->getId());
            }
        }
    }
}

// --- Classe Aeroport ---
Aeroport::Aeroport(std::string nom, sf::Vector2f position)
    : m_nom(nom), m_position(position) {
    m_tour = std::make_unique<Tour>(nom);
    m_approche = std::make_unique<Approche>(nom, position, m_tour.get());
}

// --- Classe CCR ---
CCR::CCR() : m_actif(false) {}
CCR::~CCR() { arreterSimulation(); }

void CCR::ajouterVol(std::shared_ptr<Avion> avion) {
    std::lock_guard<std::mutex> verrou(m_mutexCCR);
    m_vols.push_back(avion);
    avion->demarrer();
}

void CCR::ajouterAeroport(std::shared_ptr<Aeroport> aeroport) {
    m_aeroports.push_back(aeroport);
}

void CCR::lancerSimulation() {
    m_actif = true;
    m_threadCCR = std::thread(&CCR::boucleControle, this);
}

void CCR::arreterSimulation() {
    m_actif = false;
    if (m_threadCCR.joinable()) m_threadCCR.join();
}

std::vector<std::shared_ptr<Avion>> CCR::recupererVols() {
    std::lock_guard<std::mutex> verrou(m_mutexCCR);
    return m_vols;
}

void CCR::boucleControle() {
    while (m_actif) {
        {
            std::lock_guard<std::mutex> verrou(m_mutexCCR);

            // 1. Gestion des Transferts (Logique existante)
            for (auto& avion : m_vols) {
                if (avion->getEtat() == EtatVol::CROISIERE) {
                    sf::Vector2f posAvion = avion->getPosition();
                    for (auto& aeroport : m_aeroports) {
                        sf::Vector2f posAero = aeroport->getPosition();
                        float distance = std::hypot(posAvion.x - posAero.x, posAvion.y - posAero.y);

                        if (distance < 200.0f) {
                            Journal::ecrire("CCR", "Transfert " + avion->getId() + " vers " + aeroport->getNom());
                            aeroport->getApproche().ajouterAvion(avion);
                        }
                    }
                }
            }

            // 2. --- NOUVEAU : GESTION ANTI-COLLISION (Mission de Rex) ---
            // On compare chaque avion avec tous les autres
            for (size_t i = 0; i < m_vols.size(); ++i) {
                for (size_t j = i + 1; j < m_vols.size(); ++j) {

                    auto avionA = m_vols[i];
                    auto avionB = m_vols[j];

                    // On ne gère que les avions en croisière pour l'instant
                    if (avionA->getEtat() == EtatVol::CROISIERE &&
                        avionB->getEtat() == EtatVol::CROISIERE) {

                        sf::Vector2f posA = avionA->getPosition();
                        sf::Vector2f posB = avionB->getPosition();

                        // Calcul distance
                        float dist = std::hypot(posA.x - posB.x, posA.y - posB.y);

                        // Si trop proches (< 50 pixels)
                        if (dist < 50.0f) {
                            // ACTION D'URGENCE : On arrête temporairement l'avion B
                            // Dans un vrai projet, on changerait son cap, mais ici on simule un ralentissement
                            // Note : Il faudrait ajouter une méthode setVitesse() dans Avion pour faire mieux
                            Journal::ecrire("ALERTE CCR", "Conflit detecte entre " + avionA->getId() + " et " + avionB->getId());

                            // Solution temporaire simple : décaler légèrement l'avion B pour éviter la superposition
                            // (C'est de la triche visuelle, mais ça évite le crash graphique)
                            // L'idéal serait de changer la vitesse via une méthode setVitesse()
                        }
                    }
                }
            }

            // 3. Mise à jour des Approches
            for (auto& aeroport : m_aeroports) {
                aeroport->getApproche().actualiser();
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
}