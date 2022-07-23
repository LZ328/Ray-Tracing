/*
    材质类，基类和所有派生类
*/
#pragma once
#ifndef MATERIAL_H
#define MATERIAL_H

#include "texture.h"

// 告诉结构体后面会声明这个类，不然会找不到
class material;

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

class material {
public:
    // 纯虚函数，产生散射光线并给定光线衰减系数
    virtual bool scatter(
        ray& r_in, const hit_record& rec, color& attenuation, ray& scattered
    ) const = 0;
    // 虚函数，不必所有派生类都进行实现
    virtual color emitted(double u, double v, const point3& p) const {
        return color(0, 0, 0);
    }
};

// lambertian材质类，在单位球面上采样得到散射方向
class lambertian : public material {
public:
    lambertian(const color& a) : albedo(make_shared<solid_color>(a)) {}
    lambertian(shared_ptr<texture> a) : albedo(a) {}

    virtual bool scatter(
        ray& r_in, const hit_record& rec, color& attenuation, ray& scattered
    ) const override {
        // 这里省略了rec.p + rec.normal + random_unit_vector() - rec.p中的rec.p;
        auto scatter_direction = rec.normal + random_unit_vector();
        // 如果散射方向为0，则取法线方向作为散射方向
        if (scatter_direction.near_zero())
        {
            scatter_direction = rec.normal;
        }
        // 散射光线时刻和入射光线一样
        scattered = ray(rec.p, scatter_direction, r_in.time());
        attenuation = albedo->value(rec.u, rec.v, rec.p);
        return true;
    }

public:
    shared_ptr<texture> albedo;   //反射率
};

// 在单位球体内部采样得到散射方向的材质类
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

// 在半球采样得到散射方向的材质类
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

// 金属材质类
class metal : public material {
public:
    metal(const color& a, const double f) : albedo(a), fuzz(f < 1 ? f : 1) {}

    virtual bool scatter(
        ray& r_in, const hit_record& rec, color& attenuation, ray& scattered
    ) const override {
        // 传入镜面反射函数都是单位向量
        vec3 reflected = reflect(normalize(r_in.direction()), rec.normal);
        scattered = ray(rec.p, reflected + fuzz * random_in_unit_sphere(), r_in.time());
        attenuation = albedo;
        return (dot(scattered.direction(), rec.normal) > 0);
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
        ray& r_in, const hit_record& rec, color& attenuation, ray& scattered
    ) const override {
        // 电介质不吸收任何光，全部被折射或者反射，所以衰减系数恒为1
        attenuation = color(1.0, 1.0, 1.0);
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

        scattered = ray(rec.p, direction, r_in.time());
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

// 各向同性材质
class isotropic : public material {
public:
    isotropic(color c) : albedo(make_shared<solid_color>(c)) {}
    isotropic(shared_ptr<texture> a) : albedo(a) {}

    virtual bool scatter(
        ray& r_in, const hit_record& rec, color& attenuation, ray& scattered
    ) const override {
        // 光线向各个方向等概率均匀散射
        scattered = ray(rec.p, random_in_unit_sphere(), r_in.time());
        attenuation = albedo->value(rec.u, rec.v, rec.p);
        return true;
    }

public:
    shared_ptr<texture> albedo;
};

#endif