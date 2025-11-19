#include "etat.hpp"

class Plane {

	private:
		float posx_;
		float posy_;
		float posz_; //Altitude
		//int id;
		Etat state_;

	public:
		Plane(float pos_x, float pos_y, float pos_z, Etat state) {
			posx_ = pos_x;
			posy_ = pos_y;
			posz_ = pos_z;
			state_ = state;
		}

		//Getter
		float get_x() { return posx_; }
		float get_y() { return posy_; }
		float get_z() { return posz_; }
		Etat get_state() { return state_; }

		//Setter
		void set_state(Etat e) { state_ = e; }
};

