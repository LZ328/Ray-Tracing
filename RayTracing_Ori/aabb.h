/*
	轴对齐包围盒类
*/
#pragma once

#ifndef AABB_H
#define AABB_H

#include "utilities.h"

class aabb {
public:
	point3 minslab;
	point3 maxslab;

	aabb() {}
	aabb(const point3& m1, const point3& m2) { minslab = m1; maxslab = m2; }

	point3 min() const { return minslab; }
	point3 max() const { return maxslab; }

	inline bool hit(const ray& r, double tmin, double tmax) const
	{
		for (int i = 0; i < 3; ++i) {
			auto invD = 1.0f / r.direction()[i];
			auto t0 = (min()[i] - r.origin()[i]) * invD;
			auto t1 = (max()[i] - r.origin()[i]) * invD;
			if (invD < 0.0f)
				std::swap(t0, t1);
			tmin = t0 > tmin ? t0 : tmin;
			tmax = t1 < tmax ? t1 : tmax;
			if (tmax <= tmin)
				return false;
		}
		return true;
	}
};

// 合并两个包围盒形成一个大的包围盒
aabb surrounding_box(aabb box0, aabb box1) {
	point3 small(fmin(box0.min().x(), box1.min().x()),
		fmin(box0.min().y(), box1.min().y()),
		fmin(box0.min().z(), box1.min().z()));

	point3 big(fmax(box0.max().x(), box1.max().x()),
		fmax(box0.max().y(), box1.max().y()),
		fmax(box0.max().z(), box1.max().z()));

	return aabb(small, big);
}

#endif
