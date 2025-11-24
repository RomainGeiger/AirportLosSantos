#include "hpp/avion.hpp"

namespace ls {

    Avion::Avion(std::string id,
        Vec2 position2D,
        float altitude,
        Vec2 vitesse2D,
        float carburantInitial,
        float consommationParSeconde,
        Etat etatInitial)
        : id_(std::move(id))
        , pos_(position2D)
        , alt_(altitude)
        , vit_(vitesse2D)
        , carburant_(carburantInitial)
        , conso_(consommationParSeconde)
        , etat_(etatInitial)
    {
    }

    void Avion::update(float dt) {
        pos_ += vit_ * dt;
        carburant_ -= conso_ * dt;
        if (carburant_ < 0.f) carburant_ = 0.f;
    }

}
