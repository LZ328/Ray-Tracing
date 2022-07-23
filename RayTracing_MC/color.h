#pragma once

#include "hittable.h"
#include "pdf.h"

// 得到光线颜色，实现渲染方程
color ray_color(
	ray& r, 
	const color& background, 
	const hittable& world, 
	shared_ptr<hittable> lights, 
	int depth, 
	double RR) 
{
	hit_record rec;

	// 光线弹射指定次数后开始用RR算法终止递归
	if (depth < 0 && random_double() >= RR) return color(0, 0, 0);
	
	// 如果光线没有打到任何物体，返回背景颜色
	// 这里的t的下界设为0.001是为了防止一些光线弹射到物体上得到的t非常接近0，比如可能出现0.000001这样的值
	if (!world.hit(r, 0.001, infinity, rec))
		return background;
		
	scatter_record srec;
	color emitted = rec.mat_ptr->emitted(r, rec, rec.u, rec.v, rec.p);

	// 对于光源，不会发生散射，返回光源颜色
	if (!rec.mat_ptr->scatter(r, rec, srec))
		return emitted;

	// 如果是高光反射或折射，采用之前的渲染方程，隐式的使采样pdf和散射pdf保持一致
	if (srec.is_specular) {
		return srec.attenuation
			* ray_color(srec.specular_ray, background, world, lights, depth - 1, RR) / RR;
	}

	// 对光源采样的pdf
	auto light_ptr = make_shared<hittable_pdf>(lights, rec.p);
	// 混合pdf
	mixture_pdf p(light_ptr, srec.pdf_ptr);

	// 采样光线
	ray scattered = ray(rec.p, p.generate(), r.time());
	// 采样光线的pdf值
	auto pdf_val = p.value(scattered.direction());

	// 渲染方程
	return emitted
		+ srec.attenuation * rec.mat_ptr->scattering_pdf(r, rec, scattered)
		* ray_color(scattered, background, world, lights, depth - 1, RR) / pdf_val / RR;
}
