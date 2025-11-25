#pragma once 
#include <cmath>

namespace ls {

	struct Vec2 {
		float x = 0.f;
		float y = 0.f;

		Vec2() = default;
		Vec2(float x_, float y_) : x(x_), y(y_) {}

		//op�rations basiques

		Vec2 operator+(const Vec2& o) const {
			return Vec2(x + o.x, y + o.y);
		}

		Vec2 operator-(const Vec2& o) const {
			return Vec2(x - o.x, y - o.y);
		}

		Vec2 operator*(float s) const {
			return Vec2(x * s, y * s);
		}

		//op�rations compos�es

		Vec2 operator+=(const Vec2& o) {
			x += o.x;
			y += o.y;
			return *this;
		}
		Vec2 operator-=(const Vec2& o) {
			x -= o.x;
			y -= o.y;
			return *this;
		}
		Vec2 operator*=(float s) {
			x *= s;
			y *= s;
			return *this;
		}
		
		float length() const {
			return std::sqrt(x * x + y * y);
		}
	};

	inline Vec2 operator*(float s, const Vec2& v) {
		return Vec2(v.x * s, v.y * s);
	}
}