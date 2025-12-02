#include "ATC.hpp"
#include <iostream>

const int WIN_WIDTH = 1200;
const int WIN_HEIGHT = 800;

int main() {
    // SFML 3 : Taille fenêtre avec des accolades {}
    sf::RenderWindow window(sf::VideoMode({ WIN_WIDTH, WIN_HEIGHT }), "Simulation ATC - SFML", sf::Style::Close);
    window.setFramerateLimit(60);

    sf::Font font;
    // SFML 3 : openFromFile est préféré (et retourne bool)
    // Note : Assure-toi d'avoir arial.ttf à côté de l'exe ou dans le dossier source
    if (!font.openFromFile("arial.ttf")) {
        std::cerr << "Erreur: arial.ttf introuvable" << std::endl;
        // On continue quand même pour la démo, mais le texte ne s'affichera pas
    }

    CCR ccr;

    auto airport1 = std::make_shared<Airport>("CDG", sf::Vector2f(200, 200));
    auto airport2 = std::make_shared<Airport>("ORY", sf::Vector2f(1000, 200));
    auto airport3 = std::make_shared<Airport>("LYS", sf::Vector2f(600, 700));

    ccr.addAirport(airport1);
    ccr.addAirport(airport2);
    ccr.addAirport(airport3);

    auto flight1 = std::make_shared<Aircraft>("AFR001", sf::Vector2f(100, 100), airport3->getPosition(), 1.5f);
    auto flight2 = std::make_shared<Aircraft>("EZY456", sf::Vector2f(600, 650), airport2->getPosition(), 2.0f);
    auto flight3 = std::make_shared<Aircraft>("DLH999", sf::Vector2f(950, 250), airport1->getPosition(), 1.2f);

    ccr.addFlight(flight1);
    ccr.addFlight(flight2);
    ccr.addFlight(flight3);

    ccr.startControl();

    while (window.isOpen()) {
        // SFML 3 : Nouveau système d'événements (pollEvent retourne un std::optional)
        while (const auto event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) {
                window.close();
            }
        }

        window.clear(sf::Color(30, 30, 30));

        std::vector<std::shared_ptr<Airport>> airports = { airport1, airport2, airport3 };
        for (auto& apt : airports) {
            sf::CircleShape appZone(200);
            appZone.setOrigin({ 200, 200 });
            appZone.setPosition(apt->getPosition());
            appZone.setFillColor(sf::Color::Transparent);
            appZone.setOutlineColor(sf::Color(100, 100, 100));
            appZone.setOutlineThickness(1);
            window.draw(appZone);

            sf::RectangleShape rwy(sf::Vector2f(20, 20));
            rwy.setOrigin({ 10, 10 });
            rwy.setPosition(apt->getPosition());
            rwy.setFillColor(sf::Color::Blue);
            window.draw(rwy);

            // SFML 3 : Constructeur Text (Police, String, Taille) -> L'ordre a changé !
            sf::Text text(font, apt->getName(), 15);
            text.setPosition({ apt->getPosition().x + 15, apt->getPosition().y - 10 });
            window.draw(text);
        }

        auto flights = ccr.getFlights();
        for (auto& plane : flights) {
            sf::Vector2f pos = plane->getPosition();

            sf::ConvexShape shape;
            shape.setPointCount(3);
            shape.setPoint(0, sf::Vector2f(10, 0));
            shape.setPoint(1, sf::Vector2f(-10, -7));
            shape.setPoint(2, sf::Vector2f(-10, 7));

            shape.setPosition(pos);
            // SFML 3 : Rotation explicite en degrés
            shape.setRotation(sf::degrees(plane->getRotation()));

            FlightState st = plane->getState();
            if (st == FlightState::CRUISE) shape.setFillColor(sf::Color::Green);
            else if (st == FlightState::APPROACH) shape.setFillColor(sf::Color::Yellow);
            else if (st == FlightState::LANDING) shape.setFillColor(sf::Color::Red);
            else shape.setFillColor(sf::Color::White);

            window.draw(shape);

            // SFML 3 : Constructeur Text
            sf::Text lbl(font, plane->getId(), 12);
            lbl.setPosition({ pos.x + 15, pos.y });
            window.draw(lbl);
        }

        window.display();
    }

    ccr.stopControl();
    return 0;
}