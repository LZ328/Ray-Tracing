#pragma once

#include "hittable.h"
#include "pdf.h"

// �õ�������ɫ��ʵ����Ⱦ����
color ray_color(
	ray& r, 
	const color& background, 
	const hittable& world, 
	shared_ptr<hittable> lights, 
	int depth, 
	double RR) 
{
	hit_record rec;

	// ���ߵ���ָ��������ʼ��RR�㷨��ֹ�ݹ�
	if (depth < 0 && random_double() >= RR) return color(0, 0, 0);
	
	// �������û�д��κ����壬���ر�����ɫ
	// �����t���½���Ϊ0.001��Ϊ�˷�ֹһЩ���ߵ��䵽�����ϵõ���t�ǳ��ӽ�0��������ܳ���0.000001������ֵ
	if (!world.hit(r, 0.001, infinity, rec))
		return background;
		
	scatter_record srec;
	color emitted = rec.mat_ptr->emitted(r, rec, rec.u, rec.v, rec.p);

	// ���ڹ�Դ�����ᷢ��ɢ�䣬���ع�Դ��ɫ
	if (!rec.mat_ptr->scatter(r, rec, srec))
		return emitted;

	// ����Ǹ߹ⷴ������䣬����֮ǰ����Ⱦ���̣���ʽ��ʹ����pdf��ɢ��pdf����һ��
	if (srec.is_specular) {
		return srec.attenuation
			* ray_color(srec.specular_ray, background, world, lights, depth - 1, RR) / RR;
	}

	// �Թ�Դ������pdf
	auto light_ptr = make_shared<hittable_pdf>(lights, rec.p);
	// ���pdf
	mixture_pdf p(light_ptr, srec.pdf_ptr);

	// ��������
	ray scattered = ray(rec.p, p.generate(), r.time());
	// �������ߵ�pdfֵ
	auto pdf_val = p.value(scattered.direction());

	// ��Ⱦ����
	return emitted
		+ srec.attenuation * rec.mat_ptr->scattering_pdf(r, rec, scattered)
		* ray_color(scattered, background, world, lights, depth - 1, RR) / pdf_val / RR;
}
