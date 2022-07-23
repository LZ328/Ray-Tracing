/*
    �����࣬���������������
*/
#pragma once
#ifndef MATERIAL_H
#define MATERIAL_H

#include "texture.h"
#include "onb.h"
#include "pdf.h"

// ���߽ṹ��������������࣬��Ȼ���Ҳ���
class material;

// ���ߺ�����Ľ���
struct hit_record {
    point3 p;           //����
    vec3 normal;        //���㷨��
    double t;           //����tֵ
    double u;           //��������
    double v;           //��������
    bool front_face;    //�����Ƿ������������
    shared_ptr<material> mat_ptr;   //�������
    // �������������ı��棬����Ӧ��ȡ�����������ڼ������
    inline void set_face_normal(ray& r, vec3& outward_normal) {
        front_face = dot(r.direction(), outward_normal) < 0;
        normal = front_face ? outward_normal : -outward_normal;
    }
};

// ͳһ����ɢ�����
struct scatter_record {
    ray specular_ray;           // ɢ�����
    bool is_specular;           // �Ƿ��Ǿ��淴�䣬�������ߵ����Ϊtrue
    color attenuation;          // ������
    shared_ptr<pdf> pdf_ptr;    // ɢ�����pdf������ǽ������ߵ���ʾ��ǿ�ָ��
};

class material {
public:
    // ɢ�亯��
    virtual bool scatter(
        ray& r_in, const hit_record& rec, scatter_record& srec
    ) const {
        return false;
    };
    // �������ɢ����ߵ�pdf
    virtual double scattering_pdf(
        const ray& r_in, const hit_record& rec, const ray& scattered
    ) const {
        return 0;
    }
    // �Է��⣬��ѡ
    virtual color emitted(
        const ray& r_in, const hit_record& rec, double u, double v, const point3& p) const {
        return color(0, 0, 0);
    }
};

// lambertian�����࣬�ڵ�λ�����ϲ����õ�ɢ�䷽��
class lambertian : public material {
public:
    lambertian(const color& a) : albedo(make_shared<solid_color>(a)) {}
    lambertian(shared_ptr<texture> a) : albedo(a) {}

    // ����ɢ�����
    virtual bool scatter(
        ray& r_in, const hit_record& rec, scatter_record& srec
    ) const override {
        srec.is_specular = false;
        srec.attenuation = albedo->value(rec.u, rec.v, rec.p);
        // �������ߵĸ����ܶ�
        srec.pdf_ptr = make_shared<cosine_pdf>(rec.normal);
        return true;
    }
    
    // ���ʱ���ɢ����ߵĸ����ܶ�
    double scattering_pdf(
        const ray& r_in, const hit_record& rec, const ray& scattered
    ) const {
        auto cosine = dot(rec.normal, normalize(scattered.direction()));
        return cosine < 0 ? 0 : cosine / pi;
    }

public:
    shared_ptr<texture> albedo;   //������
};

// ����������
class metal : public material {
public:
    metal(const color& a, const double f) : albedo(a), fuzz(f < 1 ? f : 1) {}

    virtual bool scatter(
        ray& r_in, const hit_record& rec, scatter_record& srec
    ) const override {
        vec3 reflected = reflect(normalize(r_in.direction()), rec.normal);
        srec.specular_ray = ray(rec.p, reflected + fuzz * random_in_unit_sphere());
        srec.attenuation = albedo;
        srec.is_specular = true;
        srec.pdf_ptr = nullptr;
        return true;
    }

public:
    color albedo;   //������
    double fuzz;    //Glossy�����Ŷ�ϵ��
};

// ����ʲ�����
class dielectric : public material {
public:
    dielectric(double index_of_refraction) : ir(index_of_refraction) {}

    virtual bool scatter(
        ray& r_in, const hit_record& rec, scatter_record& srec
    ) const override {
        srec.is_specular = true;
        srec.pdf_ptr = nullptr;
        // ����ʲ������κι⣬ȫ����������߷��䣬����˥��ϵ����Ϊ1
        srec.attenuation = color(1.0, 1.0, 1.0);
        // ��������棬���Ǵӿ���������ʣ���֮�ӽ��������ȥ
        double refraction_ratio = rec.front_face ? (1.0 / ir) : ir;
        // ������߼ǵõ�λ�������亯������Ĳ������ǵ�λ����
        vec3 unit_direction = normalize(r_in.direction());
        // �����Ƿ�������
        double cos_theta = fmin(dot(-unit_direction, rec.normal), 1.0);
        double sin_theta = sqrt(1.0 - cos_theta * cos_theta);

        bool cannot_refract = refraction_ratio * sin_theta > 1.0;
        vec3 direction;

        if (cannot_refract || reflectance(cos_theta, refraction_ratio) > random_double())
            direction = reflect(unit_direction, rec.normal);
        else
            direction = refract(unit_direction, rec.normal, refraction_ratio);
        
        srec.specular_ray = ray(rec.p, direction, r_in.time());
        return true;
    }

public:
    double ir;  //����������

private:
    static double reflectance(double cosine, double ref_idx) {
        // ʹ��Schlick's���Ƽ����������
        auto r0 = (1 - ref_idx) / (1 + ref_idx);
        r0 = r0 * r0;
        return r0 + (1 - r0) * pow((1 - cosine), 5);
    }
};

// �Է�����ʣ�������Դ
class diffuse_light : public material {
public:
    diffuse_light(shared_ptr<texture> a) : emit(a) {}
    diffuse_light(color c) : emit(make_shared<solid_color>(c)) {}

    virtual bool scatter(
        ray& r_in, const hit_record& rec, scatter_record& srec
    ) const override {
        return false;
    }

    // ��ֻ֤�����淢��
    virtual color emitted(
        const ray& r_in, const hit_record& rec, double u, double v, const point3& p) const override {
        if (rec.front_face)
            return emit->value(u, v, p);
        else
            return color(0, 0, 0);
    }

public:
    shared_ptr<texture> emit;
};

// ����ͬ�Բ���
class isotropic : public material {
public:
    isotropic(color c) : albedo(make_shared<solid_color>(c)) {}
    isotropic(shared_ptr<texture> a) : albedo(a) {}

    virtual bool scatter(
        ray& r_in, const hit_record& rec, scatter_record& srec
    ) const override {
        // �������������ȸ��ʾ���ɢ��
        srec.is_specular = true;
        srec.pdf_ptr = nullptr;
        srec.specular_ray = ray(rec.p, random_in_unit_sphere(), r_in.time());
        srec.attenuation = albedo->value(rec.u, rec.v, rec.p);
        return true;
    }

public:
    shared_ptr<texture> albedo;
};

#endif