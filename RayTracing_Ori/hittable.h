/*
    抽象类hittable，所有物体都继承该类
*/
#pragma once
#ifndef HITTABLE_H
#define HITTABLE_H

#include "utilities.h"
#include "material.h"
#include "aabb.h"

class hittable {
public:
    // 纯虚函数，在派生类中实现
    // 计算光线与物体的交点
    virtual bool hit(ray& r, double t_min, double t_max, hit_record& rec) const = 0;
    // 计算物体的包围盒
    virtual bool bounding_box(double time0, double time1, aabb& output_box) const = 0;
};

/*
* 几何变换类，实现物体在空间中的几何变换
*/

// 平移变换
class translate : public hittable {
public:
    translate(shared_ptr<hittable> p, const vec3& displacement)
        : ptr(p), offset(displacement) {}

    virtual bool hit(
        ray& r, double t_min, double t_max, hit_record& rec) const override;

    virtual bool bounding_box(double time0, double time1, aabb& output_box) const override;

public:
    shared_ptr<hittable> ptr;
    vec3 offset;
};

bool translate::hit(ray& r, double t_min, double t_max, hit_record& rec) const {
    // 光线向反方向平移
    ray moved_r(r.origin() - offset, r.direction(), r.time());
    
    // 计算交点，这里计算出的交点是相对坐标，物体还在原本的地方
    if (!ptr->hit(moved_r, t_min, t_max, rec))
        return false;

    // 把物体和光线的交点加上偏移，得到平移后的物体和光线的交点在世界空间的绝对坐标
    // 这才相当于把物体移动了
    rec.p += offset;
    rec.set_face_normal(moved_r, rec.normal);

    return true;
}

bool translate::bounding_box(double time0, double time1, aabb& output_box) const {
    if (!ptr->bounding_box(time0, time1, output_box))
        return false;

    output_box = aabb(
        output_box.min() + offset,
        output_box.max() + offset);

    return true;
}

// 绕y轴旋转
class rotate_y : public hittable {
public:
    rotate_y(shared_ptr<hittable> p, double angle);

    virtual bool hit(
        ray& r, double t_min, double t_max, hit_record& rec) const override;

    virtual bool bounding_box(double time0, double time1, aabb& output_box) const override {
        output_box = bbox;
        return hasbox;
    }

public:
    shared_ptr<hittable> ptr;
    double sin_theta;
    double cos_theta;
    bool hasbox;
    aabb bbox;
};

// 构造函数，计算旋转后的bounding box及其他的基本成员
rotate_y::rotate_y(shared_ptr<hittable> p, double angle) : ptr(p) {
    auto radians = degrees_to_radians(angle);
    sin_theta = sin(radians);
    cos_theta = cos(radians);
    hasbox = ptr->bounding_box(0, 1, bbox);

    point3 min(infinity, infinity, infinity);
    point3 max(-infinity, -infinity, -infinity);

    // 遍历bounding box的每个顶点，并进行变换
    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < 2; j++) {
            for (int k = 0; k < 2; k++) {
                auto x = i * bbox.max().x() + (1 - i) * bbox.min().x();
                auto y = j * bbox.max().y() + (1 - j) * bbox.min().y();
                auto z = k * bbox.max().z() + (1 - k) * bbox.min().z();

                auto newx = cos_theta * x + sin_theta * z;
                auto newz = -sin_theta * x + cos_theta * z;

                vec3 tester(newx, y, newz);

                for (int c = 0; c < 3; c++) {
                    min[c] = fmin(min[c], tester[c]);
                    max[c] = fmax(max[c], tester[c]);
                }
            }
        }
    }

    bbox = aabb(min, max);
}

bool rotate_y::hit(ray& r, double t_min, double t_max, hit_record& rec) const {
    auto origin = r.origin();
    auto direction = r.direction();

    // 光线向反方向旋转
    origin[0] = cos_theta * r.origin()[0] - sin_theta * r.origin()[2];
    origin[2] = sin_theta * r.origin()[0] + cos_theta * r.origin()[2];
    // 因为光线方向实际上是两个点的差，所以也可以直接应用变换矩阵
    direction[0] = cos_theta * r.direction()[0] - sin_theta * r.direction()[2];
    direction[2] = sin_theta * r.direction()[0] + cos_theta * r.direction()[2];

    ray rotated_r(origin, direction, r.time());

    // 得到的交点同样是相对的坐标
    if (!ptr->hit(rotated_r, t_min, t_max, rec))
        return false;

    auto p = rec.p;
    auto normal = rec.normal;

    // 将交点进行旋转
    p[0] = cos_theta * rec.p[0] + sin_theta * rec.p[2];
    p[2] = -sin_theta * rec.p[0] + cos_theta * rec.p[2];
    // 法线也要旋转，法线变换应该是原变换矩阵的逆转置矩阵，旋转矩阵正交因此逆转置矩阵就是原矩阵
    normal[0] = cos_theta * rec.normal[0] + sin_theta * rec.normal[2];
    normal[2] = -sin_theta * rec.normal[0] + cos_theta * rec.normal[2];

    rec.p = p;
    rec.set_face_normal(rotated_r, normal);

    return true;
}

#endif
