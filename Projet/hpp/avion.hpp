#pragma once
#include <string>
#include "etat.hpp"

namespace ls {

    class Avion {
    public:
        Avion(std::string id,
            Vec2 position2D,
            float altitude,
            Vec2 vitesse2D,
            float carburantInitial,
            float consommationParSeconde,
            Etat etatInitial = Etat::EnApproche);

        void update(float dt);

        const std::string& id() const { return id_; }
        const Vec2& pos() const { return pos_; }
        float alt() const { return alt_; }
        const Vec2& vit() const { return vit_; }
        float fuel() const { return carburant_; }
        float conso() const { return conso_; }
        Etat etat() const { return etat_; }

        void setEtat(Etat e) { etat_ = e; }
        void setPosition(Vec2 p) { pos_ = p; }
        void setAltitude(float z) { alt_ = z; }
        void setVitesse(Vec2 v) { vit_ = v; }

        bool estALaReserve() const { return carburant_ < 10.f; }

    private:
        std::string id_;
        Vec2 pos_;
        float alt_;
        Vec2 vit_;
        float carburant_;
        float conso_;
        Etat etat_;
    };

