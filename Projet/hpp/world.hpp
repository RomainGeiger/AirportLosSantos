#pragma once 
#include <vector>
#include "avion.hpp"
#include "controleur.hpp"

namespace ls {
	class World {
	public:
		Avion& ajouterAvion(const Avion& a);
		void avancer(float dt);
		void snapshot(std::vector<const Avion*>& out) const;

	private:
		std::vector<Avion> avions_;
	};

}