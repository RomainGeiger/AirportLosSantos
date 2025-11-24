#include <iostream>
#include "etat.hpp"

int main() {
    ls::Etat e = ls::Etat::Roulage;
    std::cout << "Etat avion = " << ls::toString(e) << "\n";
}