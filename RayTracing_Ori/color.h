#pragma once

#include "hittable.h"

// 得到光线颜色
color ray_color(ray& r, const color& background, const hittable& world, int depth, double RR) {
	hit_record rec;

	// 光线弹射指定次数后开始用RR算法终止递归
	if (depth < 0 && random_double() >= RR) return color(0, 0, 0);
	
	// 如果光线没有打到任何物体，返回背景颜色
	// 这里的t的下界设为0.001是为了防止一些光线弹射到物体上得到的t非常接近0，比如可能出现0.000001这样的值
	if (!world.hit(r, 0.001, infinity, rec))
		return background;
		
	// 根据物体材质得到光线传播方向、反射率及自发光颜色
	ray scattered;
	color attenuation;
	color emitted = rec.mat_ptr->emitted(rec.u, rec.v, rec.p);
	// 对于光源，不会发生散射，返回光源颜色
	if (!rec.mat_ptr->scatter(r, rec, attenuation, scattered))
		return emitted;

	return emitted + attenuation * ray_color(scattered, background, world, depth - 1, RR) / RR;
}
