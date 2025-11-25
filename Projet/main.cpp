#include <iostream>
#include "etat.hpp"*
#include <thread>
#include <chrono>
#include "world.hpp"
#include "avion.hpp"
#include "etat.hpp"

using namespace std::chrono_literals;

int main() {
	ls::World world;

	world.ajouterAvion(ls::Avion("A123", ls::Vec2(0.f, 0.f), 1000.f, ls::Vec2(100.f, 0.f), 500.f, 1.f));
	world.ajouterAvion(ls::Avion("B456", ls::Vec2(0.f, 1000.f), 1200.f, ls::Vec2(80.f, -20.f), 600.f, 1.2f));

	const float dt = 0.2f; // 200 ms

	for (int step = 0; step < 40; step++) {
		world.avancer(dt);

		std::vector<const ls::Avion*> avions;
		world.snapshot(avions);

		std::cout << "t = " << step * dt << "s\n";
		for (auto a : avions) {
			std::cout << " " << a->id()
				<< " pos=(" << a->pos().x << "," << a->pos().y << ")"
				<< " fuel=" << a->fuel()
				<< " etat=" << ls::toString(a->etat())
				<< "\n";
		}

		std::cout << "-----------------------\n";

		std::this_thread::sleep_for(200ms);

	}
}