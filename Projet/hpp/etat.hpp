#pragma once

namespace ls {

	enum class Etat {

		EnApproche,
		Attente,
		AlignePiste,
		Atterrissage,
		AuSol,
		Roulage,
		Decollage,
		Sortie
	};



	inline const char* toString(Etat e) {
		switch (e) {
		case Etat::EnApproche:   return "EnApproche";
		case Etat::Attente:      return "Attente";
		case Etat::AlignePiste:  return "AlignePiste";
		case Etat::Atterrissage: return "Atterrissage";
		case Etat::AuSol:        return "AuSol";
		case Etat::Roulage:      return "Roulage";
		case Etat::Decollage:    return "Decollage";
		case Etat::Sortie:       return "Sortie";
		default:                 return "Inconnu";

		}
}
