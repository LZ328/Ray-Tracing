/*
    �����࣬���������������
*/
#pragma once
#ifndef MATERIAL_H
#define MATERIAL_H

#include "texture.h"

// ���߽ṹ��������������࣬��Ȼ���Ҳ���
class material;

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

class material {
public:
    // ���麯��������ɢ����߲���������˥��ϵ��
    virtual bool scatter(
        ray& r_in, const hit_record& rec, color& attenuation, ray& scattered
    ) const = 0;
    // �麯�����������������඼����ʵ��
    virtual color emitted(double u, double v, const point3& p) const {
        return color(0, 0, 0);
    }
};

// lambertian�����࣬�ڵ�λ�����ϲ����õ�ɢ�䷽��
class lambertian : public material {
public:
    lambertian(const color& a) : albedo(make_shared<solid_color>(a)) {}
    lambertian(shared_ptr<texture> a) : albedo(a) {}

    virtual bool scatter(
        ray& r_in, const hit_record& rec, color& attenuation, ray& scattered
    ) const override {
        // ����ʡ����rec.p + rec.normal + random_unit_vector() - rec.p�е�rec.p;
        auto scatter_direction = rec.normal + random_unit_vector();
        // ���ɢ�䷽��Ϊ0����ȡ���߷�����Ϊɢ�䷽��
        if (scatter_direction.near_zero())
        {
            scatter_direction = rec.normal;
        }
        // ɢ�����ʱ�̺��������һ��
        scattered = ray(rec.p, scatter_direction, r_in.time());
        attenuation = albedo->value(rec.u, rec.v, rec.p);
        return true;
    }

public:
    shared_ptr<texture> albedo;   //������
};

// �ڵ�λ�����ڲ������õ�ɢ�䷽��Ĳ�����
class lambertian_insphere : public material {
public:
    lambertian_insphere(const color& a) : albedo(make_shared<solid_color>(a)) {}
    lambertian_insphere(shared_ptr<texture> a) : albedo(a) {}

    virtual bool scatter(
        ray& r_in, const hit_record& rec, color& attenuation, ray& scattered
    ) const override {
        auto scatter_direction = rec.normal + random_in_unit_sphere();
        if (scatter_direction.near_zero())
        {
            scatter_direction = rec.normal;
        }
        scattered = ray(rec.p, scatter_direction, r_in.time());
        attenuation = albedo->value(rec.u, rec.v, rec.p);
        return true;
    }

public:
    shared_ptr<texture> albedo;
};

// �ڰ�������õ�ɢ�䷽��Ĳ�����
class lambertian_hemisphere : public material {
public:
    lambertian_hemisphere(const color& a) : albedo(make_shared<solid_color>(a)) {}
    lambertian_hemisphere(shared_ptr<texture> a) : albedo(a) {}

    virtual bool scatter(
        ray& r_in, const hit_record& rec, color& attenuation, ray& scattered
    ) const override {
        auto scatter_direction = random_in_hemisphere(rec.normal);
        scattered = ray(rec.p, scatter_direction, r_in.time());
        attenuation = albedo->value(rec.u, rec.v, rec.p);
        return true;
    }

public:
    shared_ptr<texture> albedo;
};

// ����������
class metal : public material {
public:
    metal(const color& a, const double f) : albedo(a), fuzz(f < 1 ? f : 1) {}

    virtual bool scatter(
        ray& r_in, const hit_record& rec, color& attenuation, ray& scattered
    ) const override {
        // ���뾵�淴�亯�����ǵ�λ����
        vec3 reflected = reflect(normalize(r_in.direction()), rec.normal);
        scattered = ray(rec.p, reflected + fuzz * random_in_unit_sphere(), r_in.time());
        attenuation = albedo;
        return (dot(scattered.direction(), rec.normal) > 0);
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
        ray& r_in, const hit_record& rec, color& attenuation, ray& scattered
    ) const override {
        // ����ʲ������κι⣬ȫ����������߷��䣬����˥��ϵ����Ϊ1
        attenuation = color(1.0, 1.0, 1.0);
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

        scattered = ray(rec.p, direction, r_in.time());
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
        ray& r_in, const hit_record& rec, color& attenuation, ray& scattered
    ) const override {
        return false;
    }

    virtual color emitted(double u, double v, const point3& p) const override {
        return emit->value(u, v, p);
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
        ray& r_in, const hit_record& rec, color& attenuation, ray& scattered
    ) const override {
        // �������������ȸ��ʾ���ɢ��
        scattered = ray(rec.p, random_in_unit_sphere(), r_in.time());
        attenuation = albedo->value(rec.u, rec.v, rec.p);
        return true;
    }

public:
    shared_ptr<texture> albedo;
};

#endif