#pragma once
#include <string>
//#include "Message.hpp"


class Controleur {
  public:
      Controleur(std::string nom) : nom_(std::move(nom)) {}
      virtual ~Controleur() = default;

      const std::string& nom() const { return nom_; }
      virtual void update(float dt) = 0;

      //virtual void recevoirMessage(const Message& msg) = 0;

  protected:
      std::string nom_;
};

