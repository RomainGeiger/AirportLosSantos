#pragma once

#include <string>
#include "types.h"

namespace ls {
public : 
	
	Avion(std::string id,
		Vec2 positionInitiale,
		Vec2 vitesseInitiale,
		float carburantInitial,
		float consomationParSeconde);

	void update(float dt);

	const std::string& id() const { return id_; }
	const Vec2& pos() const { return pos_; }
	const Vec2& vit() const { return vit_; }
	float              fuel() const { return carburant_; }
	float              conso() const { return conso_; }
	Etat               etat() const { return etat_; }

	//Les setters
	void setPosition(Vec2 p) { pos_ = p; }
	void setVitesse(Vec2 v) { vit_ = v; }
	void setEtat(Etat e) { etat_ = e; }

	bool estALaReserve() const;

private:
	std::string id_;      
	Vec2        pos_;      
	Vec2        vit_;      
	float       carburant_; 
	float       conso_;   
	Etat        etat_;    
};

}

}