#include "ATC.hpp"
#include <iostream>

// Constantes graphiques
const int WIN_WIDTH = 1200;
const int WIN_HEIGHT = 800;

int main() {
    // 1. Initialisation SFML
    sf::RenderWindow window(sf::VideoMode(WIN_WIDTH, WIN_HEIGHT), "Simulation ATC - SFML", sf::Style::Close);
    window.setFramerateLimit(60);

    // Chargement d'une police (optionnel, sinon texte moche ou crash si manquant)
    sf::Font font;
    // Note: Assurez-vous d'avoir un fichier arial.ttf ou commentez les parties texte si besoin
    if (!font.loadFromFile("arial.ttf")) {
        std::cerr << "Erreur: arial.ttf introuvable (copiez une police dans le dossier)" << std::endl;
        // Création d'une police par défaut système si possible ou gestion d'erreur
    }

    // 2. Création du monde (Architecture)
    CCR ccr; // Centre de contrôle national

    // Création de 3 aéroports (Positions arbitraires sur la carte)
    auto airport1 = std::make_shared<Airport>("CDG", sf::Vector2f(200, 200));
    auto airport2 = std::make_shared<Airport>("ORY", sf::Vector2f(1000, 200));
    auto airport3 = std::make_shared<Airport>("LYS", sf::Vector2f(600, 700));

    ccr.addAirport(airport1);
    ccr.addAirport(airport2);
    ccr.addAirport(airport3);

    // 3. Création des vols (Scénario)
    // Vol allant de près de CDG vers LYS
    auto flight1 = std::make_shared<Aircraft>("AFR001", sf::Vector2f(100, 100), airport3->getPosition(), 1.5f);
    // Vol allant de LYS vers ORY
    auto flight2 = std::make_shared<Aircraft>("EZY456", sf::Vector2f(600, 650), airport2->getPosition(), 2.0f);
    // Vol allant de ORY vers CDG
    auto flight3 = std::make_shared<Aircraft>("DLH999", sf::Vector2f(950, 250), airport1->getPosition(), 1.2f);

    ccr.addFlight(flight1);
    ccr.addFlight(flight2);
    ccr.addFlight(flight3);

    // Lancement de la simulation (Threads CCR et Avions)
    ccr.startControl();

    // 4. Boucle Principale (Rendu SFML)
    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();
        }

        window.clear(sf::Color(30, 30, 30)); // Fond gris foncé (Style Radar)

        // A. Dessiner les Aéroports
        std::vector<std::shared_ptr<Airport>> airports = { airport1, airport2, airport3 };
        for (auto& apt : airports) {
            // Zone APP (Cercle)
            sf::CircleShape appZone(200);
            appZone.setOrigin(200, 200);
            appZone.setPosition(apt->getPosition());
            appZone.setFillColor(sf::Color::Transparent);
            appZone.setOutlineColor(sf::Color(100, 100, 100));
            appZone.setOutlineThickness(1);
            window.draw(appZone);

            // Piste/Aéroport (Carré)
            sf::RectangleShape rwy(sf::Vector2f(20, 20));
            rwy.setOrigin(10, 10);
            rwy.setPosition(apt->getPosition());
            rwy.setFillColor(sf::Color::Blue);
            window.draw(rwy);

            // Nom
            sf::Text text(apt->getName(), font, 15);
            text.setPosition(apt->getPosition().x + 15, apt->getPosition().y - 10);
            window.draw(text);
        }

        // B. Dessiner les Avions
        auto flights = ccr.getFlights(); // Thread-safe copy du vecteur (pas des objets, pointeurs partagés)
        for (auto& plane : flights) {
            sf::Vector2f pos = plane->getPosition(); // Thread-safe
            float rot = plane->getRotation();

            // Forme de l'avion (Triangle)
            sf::ConvexShape shape;
            shape.setPointCount(3);
            shape.setPoint(0, sf::Vector2f(10, 0));
            shape.setPoint(1, sf::Vector2f(-10, -7));
            shape.setPoint(2, sf::Vector2f(-10, 7));

            shape.setPosition(pos);
            shape.setRotation(rot);

            // Couleur selon l'état
            FlightState st = plane->getState();
            if (st == FlightState::CRUISE) shape.setFillColor(sf::Color::Green);
            else if (st == FlightState::APPROACH) shape.setFillColor(sf::Color::Yellow);
            else if (st == FlightState::LANDING) shape.setFillColor(sf::Color::Red);
            else shape.setFillColor(sf::Color::White);

            window.draw(shape);

            // Label ID
            sf::Text lbl(plane->getId(), font, 12);
            lbl.setPosition(pos.x + 15, pos.y);
            window.draw(lbl);
        }

        window.display();
    }

    // Arrêt propre
    ccr.stopControl();
    return 0;
}