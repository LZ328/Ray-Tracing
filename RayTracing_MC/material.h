/*
    材质类，基类和所有派生类
*/
#pragma once
#ifndef MATERIAL_H
#define MATERIAL_H

#include "texture.h"
#include "onb.h"
#include "pdf.h"

// 告诉结构体后面会声明这个类，不然会找不到
class material;

// 光线和物体的交点
struct hit_record {
    point3 p;           //交点
    vec3 normal;        //交点法线
    double t;           //交点t值
    double u;           //纹理坐标
    double v;           //纹理坐标
    bool front_face;    //交点是否在物体的正面
    shared_ptr<material> mat_ptr;   //物体材质
    // 如果交点在物体的背面，则法线应该取反方向，以用于计算光照
    inline void set_face_normal(ray& r, vec3& outward_normal) {
        front_face = dot(r.direction(), outward_normal) < 0;
        normal = front_face ? outward_normal : -outward_normal;
    }
};

// 统一管理散射光线
struct scatter_record {
    ray specular_ray;           // 散射光线
    bool is_specular;           // 是否是镜面反射，金属或者电介质为true
    color attenuation;          // 反射率
    shared_ptr<pdf> pdf_ptr;    // 散射光线pdf，如果是金属或者电介质就是空指针
};

class material {
public:
    // 散射函数
    virtual bool scatter(
        ray& r_in, const hit_record& rec, scatter_record& srec
    ) const {
        return false;
    };
    // 计算材质散射光线的pdf
    virtual double scattering_pdf(
        const ray& r_in, const hit_record& rec, const ray& scattered
    ) const {
        return 0;
    }
    // 自发光，可选
    virtual color emitted(
        const ray& r_in, const hit_record& rec, double u, double v, const point3& p) const {
        return color(0, 0, 0);
    }
};

// lambertian材质类，在单位球面上采样得到散射方向
class lambertian : public material {
public:
    lambertian(const color& a) : albedo(make_shared<solid_color>(a)) {}
    lambertian(shared_ptr<texture> a) : albedo(a) {}

    // 采样散射光线
    virtual bool scatter(
        ray& r_in, const hit_record& rec, scatter_record& srec
    ) const override {
        srec.is_specular = false;
        srec.attenuation = albedo->value(rec.u, rec.v, rec.p);
        // 采样光线的概率密度
        srec.pdf_ptr = make_shared<cosine_pdf>(rec.normal);
        return true;
    }
    
    // 材质本身散射光线的概率密度
    double scattering_pdf(
        const ray& r_in, const hit_record& rec, const ray& scattered
    ) const {
        auto cosine = dot(rec.normal, normalize(scattered.direction()));
        return cosine < 0 ? 0 : cosine / pi;
    }

public:
    shared_ptr<texture> albedo;   //反射率
};

// 金属材质类
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
    color albedo;   //反射率
    double fuzz;    //Glossy反射扰动系数
};

// 电介质材质类
class dielectric : public material {
public:
    dielectric(double index_of_refraction) : ir(index_of_refraction) {}

    virtual bool scatter(
        ray& r_in, const hit_record& rec, scatter_record& srec
    ) const override {
        srec.is_specular = true;
        srec.pdf_ptr = nullptr;
        // 电介质不吸收任何光，全部被折射或者反射，所以衰减系数恒为1
        srec.attenuation = color(1.0, 1.0, 1.0);
        // 如果是正面，则是从空气进入介质，反之从介质折射出去
        double refraction_ratio = rec.front_face ? (1.0 / ir) : ir;
        // 入射光线记得单位化，折射函数传入的参数都是单位向量
        vec3 unit_direction = normalize(r_in.direction());
        // 计算是否发生折射
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
    double ir;  //介质折射率

private:
    static double reflectance(double cosine, double ref_idx) {
        // 使用Schlick's近似计算菲涅尔项
        auto r0 = (1 - ref_idx) / (1 + ref_idx);
        r0 = r0 * r0;
        return r0 + (1 - r0) * pow((1 - cosine), 5);
    }
};

// 自发光材质，用作光源
class diffuse_light : public material {
public:
    diffuse_light(shared_ptr<texture> a) : emit(a) {}
    diffuse_light(color c) : emit(make_shared<solid_color>(c)) {}

    virtual bool scatter(
        ray& r_in, const hit_record& rec, scatter_record& srec
    ) const override {
        return false;
    }

    // 保证只有正面发光
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

// 各向同性材质
class isotropic : public material {
public:
    isotropic(color c) : albedo(make_shared<solid_color>(c)) {}
    isotropic(shared_ptr<texture> a) : albedo(a) {}

    virtual bool scatter(
        ray& r_in, const hit_record& rec, scatter_record& srec
    ) const override {
        // 光线向各个方向等概率均匀散射
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