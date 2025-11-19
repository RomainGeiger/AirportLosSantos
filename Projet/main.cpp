#include <iostream>
#include "etat.hpp"

int main() {
    Etat e = Etat::Roulage;
    std::cout << "Etat avion = " << ls::toString(e) << "\n";
}