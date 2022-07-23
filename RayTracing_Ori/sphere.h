/*
    球体类sphere，派生于基类hittable
    支持移动，time0时球心在center0，time1时球心在enter1，线性移动
    不移动的球体可以将初始位置和结束位置设为相同
*/
#pragma once
#ifndef SPHERE_H
#define SPHERE_H

#include "hittable.h"

// 球体类以hittable抽象类为基类
class sphere : public hittable {
public:
    sphere() {}
    sphere(
        point3 cen0, point3 cen1, double _time0, double _time1, double r, shared_ptr<material> m)
        : center0(cen0), center1(cen1), time0(_time0), time1(_time1), radius(r), mat_ptr(m)
    {};
    virtual bool hit(ray& r, double t_min, double t_max, hit_record& rec) const override;
    virtual bool bounding_box(double _time0, double _time1, aabb& output_box) const override;
    point3 center(double time) const;

public:
    point3 center0, center1;
    double time0, time1;
    double radius;
    shared_ptr<material> mat_ptr;
private:
    // 计算给定球面上的点p的纹理坐标，p是圆心在原点的单位球面上的坐标
    static void get_sphere_uv(const point3& p, double& u, double& v) {
        auto theta = acos(-p.y());
        auto phi = atan2(-p.z(), p.x()) + pi;

        u = phi / (2 * pi);
        v = theta / pi;
    }
};

// 得到物体在某一时刻的球心位置
point3 sphere::center(double time) const {
    if (time1 - time0 == 0) return center0;
    return center0 + ((time - time0) / (time1 - time0)) * (center1 - center0);
}

// 计算物体和光线的交点
bool sphere::hit(ray& r, double t_min, double t_max, hit_record& rec) const
{
    vec3 oc = r.origin() - center(r.time());
    auto a = r.direction().length_squared();
    auto half_b = dot(oc, r.direction());
    auto c = oc.length_squared() - radius * radius;

    auto discriminant = half_b * half_b - a * c;
    if (discriminant < 0) return false;
    auto sqrtd = sqrt(discriminant);

    // 找到满足条件的最近的交点
    auto root = (-half_b - sqrtd) / a;
    if (root < t_min || t_max < root) {
        root = (-half_b + sqrtd) / a;
        if (root < t_min || t_max < root)
            return false;
    }
    // 记录该交点的相关信息
    rec.t = root;
    rec.p = r.at(rec.t);
    // 法线记得归一化
    vec3 outward_normal = (rec.p - center(r.time())) / radius;
    // 判断交点在正面还是背面，并设置正确的法线方向
    rec.set_face_normal(r, outward_normal);
    // 计算纹理坐标
    get_sphere_uv(outward_normal, rec.u, rec.v);
    // 记录材质
    rec.mat_ptr = mat_ptr;

    return true;
};

// 计算物体的包围盒
bool sphere::bounding_box(double _time0, double _time1, aabb& output_box) const
{
    aabb box0(
        center(_time0) - vec3(radius, radius, radius),
        center(_time0) + vec3(radius, radius, radius));
    aabb box1(
        center(_time1) - vec3(radius, radius, radius),
        center(_time1) + vec3(radius, radius, radius));
    output_box = surrounding_box(box0, box1);
    return true;
}

#endif