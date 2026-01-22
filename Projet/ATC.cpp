#include "ATC.hpp"
#include <iomanip>

std::mutex Journal::mutexEcriture;
std::ofstream Journal::fichierLog;

void Journal::initialiser() {
    fichierLog.open("logs.json", std::ios::out | std::ios::trunc);
}

void Journal::ecrire(const std::string& acteur, const std::string& action) {
    std::lock_guard<std::mutex> verrou(mutexEcriture);

    // Ecriture fichier
    if (fichierLog.is_open()) {
        auto now = std::chrono::system_clock::now();
        std::time_t t = std::chrono::system_clock::to_time_t(now);
        fichierLog << "{\"t\": " << t << ", \"qui\": \"" << acteur << "\", \"quoi\": \"" << action << "\"}" << std::endl;
    }

    // Ecriture Console
    std::cout << "[" << std::setw(10) << acteur << "] " << action << std::endl;
}

// --- Avion ---
Avion::Avion(std::string id, sf::Vector2f posDepart, float vitesse)
    : m_id(id), m_position(posDepart), m_cible(posDepart), m_vitesse(vitesse), m_vitesseStandard(vitesse),
    m_carburant(1000.0f), m_carburantMax(1000.0f), m_consommation(0.5f), m_compteurParking(0),
    m_etat(EtatVol::CROISIERE), m_enVol(false) {
}

Avion::~Avion() { arreter(); }

void Avion::demarrer() {
    m_enVol = true;
    m_threadVol = std::thread(&Avion::boucleVol, this);
}
void Avion::arreter() {
    m_enVol = false;
    if (m_threadVol.joinable()) m_threadVol.join();
}

void Avion::definirCible(sf::Vector2f c) { std::lock_guard<std::mutex> l(m_mutexDonnees); m_cible = c; }
void Avion::changerEtat(EtatVol e) { std::lock_guard<std::mutex> l(m_mutexDonnees); m_etat = e; }
void Avion::definirVitesse(float v) { std::lock_guard<std::mutex> l(m_mutexDonnees); m_vitesse = v; }
void Avion::setCompteurParking(int ticks) { std::lock_guard<std::mutex> l(m_mutexDonnees); m_compteurParking = ticks; }

EtatVol Avion::getEtat() const { return m_etat; }
std::string Avion::getId() const { return m_id; }
sf::Vector2f Avion::getPosition() { std::lock_guard<std::mutex> l(m_mutexDonnees); return m_position; }
sf::Vector2f Avion::getCible() { std::lock_guard<std::mutex> l(m_mutexDonnees); return m_cible; }
float Avion::getCarburant() const { return m_carburant; }
float Avion::getCarburantMax() const { return m_carburantMax; }
int Avion::getCompteurParking() const { return m_compteurParking; }

float Avion::getRotation() {
    std::lock_guard<std::mutex> l(m_mutexDonnees);
    sf::Vector2f d = m_cible - m_position;
    return std::atan2(d.y, d.x) * 180.0f / 3.14159f;
}

void Avion::boucleVol() {
    while (m_enVol) {
        {
            std::lock_guard<std::mutex> verrou(m_mutexDonnees);

            // 1. Gestion Carburant & Timer
            if (m_etat == EtatVol::PARKING) {
                // Remplissage lent
                if (m_carburant < m_carburantMax) m_carburant += 3.0f;
                // Décompte du temps d'attente
                if (m_compteurParking > 0) m_compteurParking--;
            }
            else {
                // Consommation en vol/roulage
                m_carburant -= m_consommation;
                if (m_carburant < 0) m_carburant = 0;
            }

            // 2. Physique
            sf::Vector2f dir = m_cible - m_position;
            float dist = std::hypot(dir.x, dir.y);
            if (dist > 2.0f) {
                sf::Vector2f norm = dir / dist;
                m_position += norm * m_vitesse;
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(16));
    }
}

// --- Tour ---
Tour::Tour(std::string nom) : m_nom(nom), m_pisteOccupee(false) {}

bool Tour::demanderAccesPiste(std::shared_ptr<Avion> avion) {
    std::lock_guard<std::mutex> l(m_mutexTour);
    if (!m_pisteOccupee) {
        m_pisteOccupee = true;
        Journal::ecrire("TWR " + m_nom, "Piste accordee a " + avion->getId());
        return true;
    }
    return false;
}
void Tour::libererPiste() { m_pisteOccupee = false; }
bool Tour::estOccupee() { return m_pisteOccupee; }

// --- Approche ---
Approche::Approche(std::string nom, sf::Vector2f pos, Tour* t)
    : m_nom(nom), m_position(pos), m_tourAssociee(t) {
}

void Approche::ajouterAvion(std::shared_ptr<Avion> av) {
    std::lock_guard<std::mutex> l(m_mutexApp);
    m_avions.push_back(av);
    av->changerEtat(EtatVol::APP_ENTREE);
    av->definirCible(m_position);
    Journal::ecrire("APP " + m_nom, "Radar contact " + av->getId());
}

void Approche::actualiser() {
    std::lock_guard<std::mutex> l(m_mutexApp);
    m_avions.erase(std::remove_if(m_avions.begin(), m_avions.end(),
        [](std::shared_ptr<Avion> a) { return a->getEtat() == EtatVol::CROISIERE; }), m_avions.end());

    for (auto& av : m_avions) {
        sf::Vector2f pos = av->getPosition();
        float d = std::hypot(pos.x - m_position.x, pos.y - m_position.y);

        if (av->getEtat() == EtatVol::APP_ENTREE && d < 100.0f) {
            if (m_tourAssociee->demanderAccesPiste(av)) {
                av->changerEtat(EtatVol::ATTERRISSAGE);
                Journal::ecrire("APP " + m_nom, "Autorise atterrissage " + av->getId());
            }
            else {
                av->definirVitesse(0.5f);
            }
        }
    }
}

// --- Aeroport ---
Aeroport::Aeroport(std::string nom, sf::Vector2f p, sf::Vector2f pk)
    : m_nom(nom), m_position(p), m_posParking(pk) {
    m_tour = std::make_unique<Tour>(nom);
    m_approche = std::make_unique<Approche>(nom, p, m_tour.get());
}

// --- CCR ---
CCR::CCR() : m_actif(false) { Journal::initialiser(); }
CCR::~CCR() { arreterSimulation(); }

void CCR::ajouterVol(std::shared_ptr<Avion> a) { std::lock_guard<std::mutex> l(m_mutexCCR); m_vols.push_back(a); a->demarrer(); }
void CCR::ajouterAeroport(std::shared_ptr<Aeroport> a) { m_aeroports.push_back(a); }
std::vector<std::shared_ptr<Avion>> CCR::recupererVols() { std::lock_guard<std::mutex> l(m_mutexCCR); return m_vols; }
void CCR::lancerSimulation() { m_actif = true; m_threadCCR = std::thread(&CCR::boucleControle, this); }
void CCR::arreterSimulation() { m_actif = false; if (m_threadCCR.joinable()) m_threadCCR.join(); }

void CCR::boucleControle() {
    while (m_actif) {
        {
            std::lock_guard<std::mutex> verrou(m_mutexCCR);

            // 1. ANTI-COLLISION
            for (size_t i = 0; i < m_vols.size(); ++i) {

                // On ne gère les collisions que si l'avion est EN L'AIR (Croisière)
                // Si on est au sol ou en approche finale, on fait confiance à la Tour (TWR)
                if (m_vols[i]->getEtat() != EtatVol::CROISIERE) continue;

                bool risqueCollision = false;

                for (size_t j = 0; j < m_vols.size(); ++j) {
                    if (i == j) continue; // Pas de collision avec soi-même

                    // On ne teste la collision qu'avec d'autres avions en CROISIERE
                    if (m_vols[j]->getEtat() != EtatVol::CROISIERE) continue;

                    sf::Vector2f d = m_vols[i]->getPosition() - m_vols[j]->getPosition();
                    float dist = std::hypot(d.x, d.y);

                    // Zone de danger (60 pixels)
                    if (dist < 60.0f) {
                        m_vols[i]->definirVitesse(0.0f); // Freinage d'urgence
                        risqueCollision = true;
                    }
                }

                // Si le danger est écarté, on relance la machine
                if (!risqueCollision && m_vols[i]->getEtat() == EtatVol::CROISIERE) {
                    // On remet une vitesse normale
                    m_vols[i]->definirVitesse(1.5f);
                }
            }
            }


            // 2. GESTION DES FLUX
            for (auto& av : m_vols) {
                EtatVol etat = av->getEtat();
                sf::Vector2f pos = av->getPosition();
                sf::Vector2f target = av->getCible();

                // 1. CROISIERE -> APPROCHE
                if (etat == EtatVol::CROISIERE) {
                    for (auto& aero : m_aeroports) {
                        float dist = std::hypot(pos.x - aero->getPosition().x, pos.y - aero->getPosition().y);
                        float distCible = std::hypot(target.x - aero->getPosition().x, target.y - aero->getPosition().y);

                        if (dist < 300.0f && distCible < 10.0f) {
                            aero->getApproche().ajouterAvion(av);
                            break;
                        }
                    }
                }

                // 2. ATTERRISSAGE -> PARKING
                if (etat == EtatVol::ATTERRISSAGE) {
                    for (auto& aero : m_aeroports) {
                        if (std::hypot(pos.x - aero->getPosition().x, pos.y - aero->getPosition().y) < 10.0f) {
                            av->changerEtat(EtatVol::ROULAGE_VERS_PARKING);
                            av->definirCible(aero->getParking());
                        }
                    }
                }

                // 3. ARRIVÉE PARKING (Lancement Timer)
                if (etat == EtatVol::ROULAGE_VERS_PARKING) {
                    for (auto& aero : m_aeroports) {
                        if (std::hypot(pos.x - aero->getParking().x, pos.y - aero->getParking().y) < 5.0f) {
                            av->changerEtat(EtatVol::PARKING);
                            // On définit un temps d'attente (ex: 500 ticks ~ 8-10 secondes)
                            av->setCompteurParking(500);
                            aero->getTour().libererPiste();
                            Journal::ecrire("SOL", av->getId() + " au parking. Arret moteurs.");
                        }
                    }
                }

                // 4. DÉPART PARKING (Condition : Plein ET Timer fini)
                if (etat == EtatVol::PARKING) {
                    if (av->getCarburant() >= 990.0f && av->getCompteurParking() <= 0) {
                        for (auto& aero : m_aeroports) {
                            if (std::hypot(pos.x - aero->getParking().x, pos.y - aero->getParking().y) < 10.0f) {
                                av->changerEtat(EtatVol::ROULAGE_VERS_PISTE);
                                av->definirCible(aero->getPosition());
                                Journal::ecrire("SOL", av->getId() + " demarrage et roulage piste.");
                            }
                        }
                    }
                }

                // 5. DEMANDE DÉCOLLAGE
                if (etat == EtatVol::ROULAGE_VERS_PISTE) {
                    for (auto& aero : m_aeroports) {
                        if (std::hypot(pos.x - aero->getPosition().x, pos.y - aero->getPosition().y) < 10.0f) {
                            if (aero->getTour().demanderAccesPiste(av)) {
                                av->changerEtat(EtatVol::DECOLLAGE);
                                av->definirCible(aero->getPosition() + sf::Vector2f(0, -200));
                            }
                        }
                    }
                }

                // 6. FIN DÉCOLLAGE -> NOUVELLE DESTINATION
                if (etat == EtatVol::DECOLLAGE) {
                    for (auto& aero : m_aeroports) {
                        float dist = std::hypot(pos.x - aero->getPosition().x, pos.y - aero->getPosition().y);

                        if (dist > 100.0f && dist < 150.0f) {
                            std::shared_ptr<Aeroport> dest = nullptr;
                            int attempts = 0;
                            do {
                                int r = rand() % m_aeroports.size();
                                dest = m_aeroports[r];
                                attempts++;
                            } while (dest->getNom() == aero->getNom() && attempts < 10);

                            av->changerEtat(EtatVol::CROISIERE);
                            av->definirCible(dest->getPosition());
                            aero->getTour().libererPiste();

                            Journal::ecrire("CCR", av->getId() + " cap sur " + dest->getNom());
                        }
                    }
                }
            }
            for (auto& a : m_aeroports) a->getApproche().actualiser();
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
