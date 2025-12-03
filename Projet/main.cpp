#include "ATC.hpp"
#include <iostream>

const int LARGEUR_FENETRE = 1200;
const int HAUTEUR_FENETRE = 800;

int main() {
    // Création de la fenêtre SFML
    sf::RenderWindow fenetre(sf::VideoMode({ LARGEUR_FENETRE, HAUTEUR_FENETRE }), "Controle Aerien - Projet C++", sf::Style::Close);
    fenetre.setFramerateLimit(60);

    // Chargement de la police pour le texte
    sf::Font police;
    if (!police.openFromFile("arial.ttf")) {
        std::cerr << "Erreur : Impossible de charger arial.ttf" << std::endl;
    }

    // 1. Initialisation du contrôleur national (CCR)
    CCR ccr;

    // 2. Création des aéroports
    auto cdg = std::make_shared<Aeroport>("CDG", sf::Vector2f(200, 200));
    auto ory = std::make_shared<Aeroport>("ORY", sf::Vector2f(1000, 200));
    auto lys = std::make_shared<Aeroport>("LYS", sf::Vector2f(600, 700));

    ccr.ajouterAeroport(cdg);
    ccr.ajouterAeroport(ory);
    ccr.ajouterAeroport(lys);

    // 3. Création des vols
    // Vol vers LYS
    auto vol1 = std::make_shared<Avion>("AFR001", sf::Vector2f(100, 100), lys->getPosition(), 1.5f);
    // Vol vers ORY
    auto vol2 = std::make_shared<Avion>("EZY456", sf::Vector2f(600, 650), ory->getPosition(), 2.0f);
    // Vol vers CDG
    auto vol3 = std::make_shared<Avion>("DLH999", sf::Vector2f(950, 250), cdg->getPosition(), 1.2f);

    ccr.ajouterVol(vol1);
    ccr.ajouterVol(vol2);
    ccr.ajouterVol(vol3);

    // Lancement de la logique (threads)
    ccr.lancerSimulation();

    // 4. Boucle d'affichage
    while (fenetre.isOpen()) {

        // Gestion des événements (Fermeture fenêtre)
        while (const auto evenement = fenetre.pollEvent()) {
            if (evenement->is<sf::Event::Closed>()) {
                fenetre.close();
            }
        }

        fenetre.clear(sf::Color(30, 30, 30)); // Fond gris foncé

        // A. Affichage des Aéroports
        std::vector<std::shared_ptr<Aeroport>> listeAeroports = { cdg, ory, lys };

        for (auto& aero : listeAeroports) {
            // Cercle pour la zone d'approche
            sf::CircleShape zoneApproche(200);
            zoneApproche.setOrigin({ 200, 200 });
            zoneApproche.setPosition(aero->getPosition());
            zoneApproche.setFillColor(sf::Color::Transparent);
            zoneApproche.setOutlineColor(sf::Color(100, 100, 100));
            zoneApproche.setOutlineThickness(1);
            fenetre.draw(zoneApproche);

            // Carré bleu pour la piste
            sf::RectangleShape piste(sf::Vector2f(20, 20));
            piste.setOrigin({ 10, 10 });
            piste.setPosition(aero->getPosition());
            piste.setFillColor(sf::Color::Blue);
            fenetre.draw(piste);

            // Nom de l'aéroport
            sf::Text texteNom(police, aero->getNom(), 15);
            texteNom.setPosition({ aero->getPosition().x + 15, aero->getPosition().y - 10 });
            fenetre.draw(texteNom);
        }

        // B. Affichage des Avions
        auto vols = ccr.recupererVols();

        for (auto& avion : vols) {
            sf::Vector2f pos = avion->getPosition();

            // Triangle pour représenter l'avion
            sf::ConvexShape formeAvion;
            formeAvion.setPointCount(3);
            formeAvion.setPoint(0, sf::Vector2f(10, 0));   // Nez
            formeAvion.setPoint(1, sf::Vector2f(-10, -7)); // Aile gauche
            formeAvion.setPoint(2, sf::Vector2f(-10, 7));  // Aile droite

            formeAvion.setPosition(pos);
            formeAvion.setRotation(sf::degrees(avion->getRotation()));

            // Couleur selon l'état du vol
            EtatVol etat = avion->getEtat();
            if (etat == EtatVol::CROISIERE) formeAvion.setFillColor(sf::Color::Green);
            else if (etat == EtatVol::APPROCHE) formeAvion.setFillColor(sf::Color::Yellow);
            else if (etat == EtatVol::ATTERRISSAGE) formeAvion.setFillColor(sf::Color::Red);
            else formeAvion.setFillColor(sf::Color::White); // Au sol

            fenetre.draw(formeAvion);

            // Identifiant de l'avion
            sf::Text texteId(police, avion->getId(), 12);
            texteId.setPosition({ pos.x + 15, pos.y });
            fenetre.draw(texteId);
        }

        fenetre.display();
    }

    // Nettoyage avant de quitter
    ccr.arreterSimulation();
    return 0;
}