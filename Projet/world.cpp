#include "hpp/world.hpp"

namespace ls {
	Avion& World::ajouterAvion(const Avion& a) {
		avions_.push_back(a);
		return avions_.back();
	}
	void World::avancer(float dt) {
		for (auto& avion : avions_) {
			avion.update(dt);
		}
	}
	void World::snapshot(std::vector<const Avion*>& out) const {
		out.clear();
		for (const auto& avion : avions_) {
			out.push_back(&avion);
		}
	}
}