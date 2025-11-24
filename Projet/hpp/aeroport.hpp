#pragma once
#include <string>
#include "avion.hpp"

namespace ls {

	class Aeroport(){

		protected:
			std::string nom_;
			

		public:
			const std::string& nom() const { return nom_; }
}