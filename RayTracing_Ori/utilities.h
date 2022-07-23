/*
	常用常量及工具函数
	整合包含其他头文件
*/
#pragma once

#ifndef UTILITIES_H
#define UTILITIES_H

#include <cmath>
#include <limits>
#include <memory>
#include <random>
#include <vector>

// 常用头文件

#include "ray.h"
#include "vec3.h"

using std::shared_ptr;
using std::make_shared;
using std::sqrt;

// 常量
const double infinity = std::numeric_limits<double>::infinity();
const double pi = 3.1415926535897932385;

/*
	一些工具函数
*/

// 角度转弧度
inline double degrees_to_radians(double degrees) {
	return degrees * pi / 180.0;
}

// 生成[0,1]之间的随机浮点数
inline double random_double() {
	// 0~1之间的均匀分布
	static std::uniform_real_distribution<double> distribution(0.0, 1.0);
	// random_device用于生成均匀整数，用来给mt19937播种，类似于之前的srand()
	static std::mt19937 generator(std::random_device{}());
	// 生成符合分布的随机数
	return distribution(generator);
}

// 生成[min,max]之间的随机浮点数
inline double random_double(const double min, const double max) {
	return min + (max - min) * random_double();
}

// 生成[min,max]之间的随机整数
inline int random_int(int min, int max) {
	return static_cast<int>(random_double(min, max + 1));
}

// 生成分量在[0,1]之间的随机向量
inline vec3 random_vec() {
	return vec3(random_double(), random_double(), random_double());
}

// 生成分量在[min,max]之间的随机向量
inline vec3 random_vec(double min, double max) {
	return vec3(random_double(min, max), random_double(min, max), random_double(min, max));
}

// 生成单位球体内随机一点
inline point3 random_in_unit_sphere() {
	while (true) {
		auto p = random_vec(-1, 1);
		if (p.length_squared() >= 1) continue;
		return p;
	}
}

// 生成单位球面上随机一点
inline vec3 random_unit_vector() {
	return normalize(random_in_unit_sphere());
}

// 在半球内随机取一点
inline vec3 random_in_hemisphere(const vec3& normal) {
	vec3 in_unit_sphere = random_in_unit_sphere();
	// 判断该偏移量是否落入了下半球，如果落入下半球则偏移量应该取反
	if (dot(in_unit_sphere, normal) > 0.0)
		return in_unit_sphere;
	else
		return -in_unit_sphere;
}

// 生成单位圆盘内随机一点
inline vec3 random_in_unit_disk() {
	while (true) {
		auto p = vec3(random_double(-1, 1), random_double(-1, 1), 0);
		if (p.length_squared() >= 1) continue;
		return p;
	}
}

// 将给定的数字截取到[min,max]范围内
inline double clamp(double x, double min, double max) {
	if (x < min) return min;
	if (x > max) return max;
	return x;
}

// 向数组中写入一个颜色，用到了指针的引用传递
// 输入的color是[0,1]范围的
inline void write_color(unsigned char*& p, color pixel_color, int samples_per_pixel)
{
	auto r = pixel_color.x();
	auto g = pixel_color.y();
	auto b = pixel_color.z();

	auto scale = 1.0 / samples_per_pixel;

	// 伽马校正，假设显示gamma=2.0
	r = sqrt(scale * r);
	g = sqrt(scale * g);
	b = sqrt(scale * b);

	*p++ = (unsigned char)(256 * clamp(r, 0.0, 0.999));
	*p++ = (unsigned char)(256 * clamp(g, 0.0, 0.999));
	*p++ = (unsigned char)(256 * clamp(b, 0.0, 0.999));
}

#endif UTILITIES_H
