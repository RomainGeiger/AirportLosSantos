#include "etat.hpp"

class Plane {

	private:
		float posx_;
		float posy_;
		//int id;
		Etat state_;

	public:
		Plane(float pos_x, float pos_y, Etat state) {
			posx_ = pos_x;
			posy_ = pos_y;
			state_ = state;
		}

		//Getter
		float get_x() { return posx_; }
		float get_y() { return posy_; }
		Etat get_state() { return state_; }

		//Setter
		void set_state(Etat e) { state_ = e; }
};

