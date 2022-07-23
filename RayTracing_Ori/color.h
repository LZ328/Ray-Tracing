#pragma once

#include "hittable.h"

// �õ�������ɫ
color ray_color(ray& r, const color& background, const hittable& world, int depth, double RR) {
	hit_record rec;

	// ���ߵ���ָ��������ʼ��RR�㷨��ֹ�ݹ�
	if (depth < 0 && random_double() >= RR) return color(0, 0, 0);
	
	// �������û�д��κ����壬���ر�����ɫ
	// �����t���½���Ϊ0.001��Ϊ�˷�ֹһЩ���ߵ��䵽�����ϵõ���t�ǳ��ӽ�0��������ܳ���0.000001������ֵ
	if (!world.hit(r, 0.001, infinity, rec))
		return background;
		
	// ����������ʵõ����ߴ������򡢷����ʼ��Է�����ɫ
	ray scattered;
	color attenuation;
	color emitted = rec.mat_ptr->emitted(rec.u, rec.v, rec.p);
	// ���ڹ�Դ�����ᷢ��ɢ�䣬���ع�Դ��ɫ
	if (!rec.mat_ptr->scatter(r, rec, attenuation, scattered))
		return emitted;

	return emitted + attenuation * ray_color(scattered, background, world, depth - 1, RR) / RR;
}
