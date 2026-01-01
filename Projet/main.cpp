#include "ATC.hpp"
#include <iostream>

const int LARGEUR = 1024;
const int HAUTEUR = 1024;

int main() {
    sf::RenderWindow fenetre(sf::VideoMode({ LARGEUR, HAUTEUR }), "Controle Aerien - Los Santos", sf::Style::Close);
    fenetre.setFramerateLimit(60);

    sf::Font police;
    if (!police.openFromFile("lib/arial.ttf")) {
        std::cerr << "Erreur Police (arial.ttf manquant dans lib/)" << std::endl;
    }

    sf::Texture textureMap;
    if (!textureMap.loadFromFile("lib/gta_map.jpg")) {
        std::cerr << "Erreur Texture (gta_map.jpg manquant dans lib/)" << std::endl;
        return -1;
    }

    sf::Sprite spriteMap(textureMap);
    sf::Vector2u tailleImg = textureMap.getSize();
    spriteMap.setScale({ (float)LARGEUR / tailleImg.x, (float)HAUTEUR / tailleImg.y });

    CCR ccr;

    // --- COORDONNEES GTA V (TES VALEURS) ---
    // LSIA
    sf::Vector2f posLSIA_Piste(350.0f, 950.0f);
    sf::Vector2f posLSIA_Park(400.0f, 950.0f);
    auto lsia = std::make_shared<Aeroport>("LSIA", posLSIA_Piste, posLSIA_Park);

    // Sandy Shore
    sf::Vector2f posSandy_Piste(570.0f, 425.0f);
    sf::Vector2f posSandy_Park(620.0f, 425.0f);
    auto sandy = std::make_shared<Aeroport>("Sandy Shores", posSandy_Piste, posSandy_Park);

    // Lago
    sf::Vector2f posGrapeseed_Piste(260.0f, 425.0f);
    sf::Vector2f posGrapeseed_Park(310.0f, 425.0f);
    auto grapeseed = std::make_shared<Aeroport>("Lago", posGrapeseed_Piste, posGrapeseed_Park);

    ccr.ajouterAeroport(lsia);
    ccr.ajouterAeroport(sandy);
    ccr.ajouterAeroport(grapeseed);

    // --- GENERATION DE FLOTTE ---

    // On crée un petit générateur de nombres aléatoires
    srand((unsigned int)time(0));

    for (int i = 0; i < 5; ++i) {
        // Position aléatoire sur la carte (entre 50 et 900)
        float randX = (float)(rand() % 900 + 50);
        float randY = (float)(rand() % 900 + 50);

        // Vitesse aléatoire entre 1.2 et 2.2
        float randSpeed = 1.2f + static_cast <float>(rand()) / (static_cast <float>(RAND_MAX / 1.0f));

        // Nom unique : VOL + numero
        std::string id = "VOL" + std::to_string(100 + i);

        auto avion = std::make_shared<Avion>(id, sf::Vector2f(randX, randY), randSpeed);

        // On leur donne une première cible au hasard parmi les 3 aéroports
        int r = rand() % 3;
        if (r == 0) avion->definirCible(lsia->getPosition());
        else if (r == 1) avion->definirCible(sandy->getPosition());
        else avion->definirCible(grapeseed->getPosition());

        ccr.ajouterVol(avion);
    }

    ccr.lancerSimulation();

    while (fenetre.isOpen()) {
        while (const auto ev = fenetre.pollEvent()) {
            if (ev->is<sf::Event::Closed>()) fenetre.close();
        }

        fenetre.clear();
        fenetre.draw(spriteMap);

        std::vector<std::shared_ptr<Aeroport>> aeroports = { lsia, sandy, grapeseed };
        for (auto& a : aeroports) {
            // Zone APP
            sf::CircleShape c(80);
            c.setOrigin({ 80, 80 });
            c.setPosition(a->getPosition());
            c.setFillColor(sf::Color(0, 0, 255, 30));
            c.setOutlineColor(sf::Color::Blue);
            c.setOutlineThickness(1);
            fenetre.draw(c);

            // Piste (Rouge)
            sf::RectangleShape r({ 30, 10 });
            r.setOrigin({ 15, 5 });
            r.setPosition(a->getPosition());
            r.setFillColor(sf::Color::Red);
            fenetre.draw(r);

            // Parking (Vert)
            sf::RectangleShape p({ 10, 10 });
            p.setOrigin({ 5, 5 });
            p.setPosition(a->getParking());
            p.setFillColor(sf::Color::Green);
            fenetre.draw(p);

            sf::Text t(police, a->getNom(), 14);
            t.setPosition(a->getPosition() + sf::Vector2f(0, -30));
            t.setOutlineColor(sf::Color::Black);
            t.setOutlineThickness(1);
            fenetre.draw(t);
        }

        auto vols = ccr.recupererVols();
        for (auto& av : vols) {
            sf::Vector2f pos = av->getPosition();

            sf::ConvexShape s;
            s.setPointCount(3);
            s.setPoint(0, { 10, 0 });
            s.setPoint(1, { -10, -7 });
            s.setPoint(2, { -10, 7 });
            s.setPosition(pos);
            s.setRotation(sf::degrees(av->getRotation()));

            // Couleur Fuel
            float fuelPct = av->getCarburant() / av->getCarburantMax();
            if (fuelPct < 0.2f) s.setFillColor(sf::Color(255, 140, 0)); // Orange
            else {
                switch (av->getEtat()) {
                case EtatVol::CROISIERE: s.setFillColor(sf::Color::Cyan); break;
                case EtatVol::ATTERRISSAGE: s.setFillColor(sf::Color::Magenta); break;
                case EtatVol::PARKING: s.setFillColor(sf::Color::Black); break;
                case EtatVol::DECOLLAGE: s.setFillColor(sf::Color::White); break;
                default: s.setFillColor(sf::Color::Yellow); break;
                }
            }
            fenetre.draw(s);

            std::string label = av->getId() + "\n" + std::to_string((int)(fuelPct * 100)) + "%";
            sf::Text lbl(police, label, 10);
            lbl.setPosition(pos + sf::Vector2f(12, -10));
            lbl.setOutlineColor(sf::Color::Black);
            lbl.setOutlineThickness(1);
            fenetre.draw(lbl);
        }

        fenetre.display();
    }

    ccr.arreterSimulation();
    return 0;
}