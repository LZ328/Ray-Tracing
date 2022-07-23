/*
	���ó��������ߺ���
	���ϰ�������ͷ�ļ�
*/
#pragma once

#ifndef UTILITIES_H
#define UTILITIES_H

#include <cmath>
#include <limits>
#include <memory>
#include <random>
#include <vector>

// ����ͷ�ļ�

#include "ray.h"
#include "vec3.h"

using std::shared_ptr;
using std::make_shared;
using std::sqrt;

// ����
const double infinity = std::numeric_limits<double>::infinity();
const double pi = 3.1415926535897932385;

/*
	һЩ���ߺ���
*/

// �Ƕ�ת����
inline double degrees_to_radians(double degrees) {
	return degrees * pi / 180.0;
}

// ����[0,1]֮������������
inline double random_double() {
	// 0~1֮��ľ��ȷֲ�
	static std::uniform_real_distribution<double> distribution(0.0, 1.0);
	// random_device�������ɾ���������������mt19937���֣�������֮ǰ��srand()
	static std::mt19937 generator(std::random_device{}());
	// ���ɷ��Ϸֲ��������
	return distribution(generator);
}

// ����[min,max]֮������������
inline double random_double(const double min, const double max) {
	return min + (max - min) * random_double();
}

// ����[min,max]֮����������
inline int random_int(int min, int max) {
	return static_cast<int>(random_double(min, max + 1));
}

// ���ɷ�����[0,1]֮����������
inline vec3 random_vec() {
	return vec3(random_double(), random_double(), random_double());
}

// ���ɷ�����[min,max]֮����������
inline vec3 random_vec(double min, double max) {
	return vec3(random_double(min, max), random_double(min, max), random_double(min, max));
}

// ���ɵ�λ���������һ��
inline point3 random_in_unit_sphere() {
	while (true) {
		auto p = random_vec(-1, 1);
		if (p.length_squared() >= 1) continue;
		return p;
	}
}

// ���ɵ�λ���������һ��
inline vec3 random_unit_vector() {
	return normalize(random_in_unit_sphere());
}

// �ڰ��������ȡһ��
inline vec3 random_in_hemisphere(const vec3& normal) {
	vec3 in_unit_sphere = random_in_unit_sphere();
	// �жϸ�ƫ�����Ƿ��������°�����������°�����ƫ����Ӧ��ȡ��
	if (dot(in_unit_sphere, normal) > 0.0)
		return in_unit_sphere;
	else
		return -in_unit_sphere;
}

// ���ɵ�λԲ�������һ��
inline vec3 random_in_unit_disk() {
	while (true) {
		auto p = vec3(random_double(-1, 1), random_double(-1, 1), 0);
		if (p.length_squared() >= 1) continue;
		return p;
	}
}

// �����������ֽ�ȡ��[min,max]��Χ��
inline double clamp(double x, double min, double max) {
	if (x < min) return min;
	if (x > max) return max;
	return x;
}

// ��������д��һ����ɫ���õ���ָ������ô���
// �����color��[0,1]��Χ��
inline void write_color(unsigned char*& p, color pixel_color, int samples_per_pixel)
{
	auto r = pixel_color.x();
	auto g = pixel_color.y();
	auto b = pixel_color.z();

	auto scale = 1.0 / samples_per_pixel;

	// ٤��У����������ʾgamma=2.0
	r = sqrt(scale * r);
	g = sqrt(scale * g);
	b = sqrt(scale * b);

	*p++ = (unsigned char)(256 * clamp(r, 0.0, 0.999));
	*p++ = (unsigned char)(256 * clamp(g, 0.0, 0.999));
	*p++ = (unsigned char)(256 * clamp(b, 0.0, 0.999));
}

#endif UTILITIES_H
