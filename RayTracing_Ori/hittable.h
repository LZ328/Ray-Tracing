/*
    ������hittable���������嶼�̳и���
*/
#pragma once
#ifndef HITTABLE_H
#define HITTABLE_H

#include "utilities.h"
#include "material.h"
#include "aabb.h"

class hittable {
public:
    // ���麯��������������ʵ��
    // �������������Ľ���
    virtual bool hit(ray& r, double t_min, double t_max, hit_record& rec) const = 0;
    // ��������İ�Χ��
    virtual bool bounding_box(double time0, double time1, aabb& output_box) const = 0;
};

/*
* ���α任�࣬ʵ�������ڿռ��еļ��α任
*/

// ƽ�Ʊ任
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
    // �����򷴷���ƽ��
    ray moved_r(r.origin() - offset, r.direction(), r.time());
    
    // ���㽻�㣬���������Ľ�����������꣬���廹��ԭ���ĵط�
    if (!ptr->hit(moved_r, t_min, t_max, rec))
        return false;

    // ������͹��ߵĽ������ƫ�ƣ��õ�ƽ�ƺ������͹��ߵĽ���������ռ�ľ�������
    // ����൱�ڰ������ƶ���
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

// ��y����ת
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

// ���캯����������ת���bounding box�������Ļ�����Ա
rotate_y::rotate_y(shared_ptr<hittable> p, double angle) : ptr(p) {
    auto radians = degrees_to_radians(angle);
    sin_theta = sin(radians);
    cos_theta = cos(radians);
    hasbox = ptr->bounding_box(0, 1, bbox);

    point3 min(infinity, infinity, infinity);
    point3 max(-infinity, -infinity, -infinity);

    // ����bounding box��ÿ�����㣬�����б任
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

    // �����򷴷�����ת
    origin[0] = cos_theta * r.origin()[0] - sin_theta * r.origin()[2];
    origin[2] = sin_theta * r.origin()[0] + cos_theta * r.origin()[2];
    // ��Ϊ���߷���ʵ������������Ĳ����Ҳ����ֱ��Ӧ�ñ任����
    direction[0] = cos_theta * r.direction()[0] - sin_theta * r.direction()[2];
    direction[2] = sin_theta * r.direction()[0] + cos_theta * r.direction()[2];

    ray rotated_r(origin, direction, r.time());

    // �õ��Ľ���ͬ������Ե�����
    if (!ptr->hit(rotated_r, t_min, t_max, rec))
        return false;

    auto p = rec.p;
    auto normal = rec.normal;

    // �����������ת
    p[0] = cos_theta * rec.p[0] + sin_theta * rec.p[2];
    p[2] = -sin_theta * rec.p[0] + cos_theta * rec.p[2];
    // ����ҲҪ��ת�����߱任Ӧ����ԭ�任�������ת�þ�����ת�������������ת�þ������ԭ����
    normal[0] = cos_theta * rec.normal[0] + sin_theta * rec.normal[2];
    normal[2] = -sin_theta * rec.normal[0] + cos_theta * rec.normal[2];

    rec.p = p;
    rec.set_face_normal(rotated_r, normal);

    return true;
}

#endif
