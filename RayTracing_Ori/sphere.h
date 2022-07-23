/*
    ������sphere�������ڻ���hittable
    ֧���ƶ���time0ʱ������center0��time1ʱ������enter1�������ƶ�
    ���ƶ���������Խ���ʼλ�úͽ���λ����Ϊ��ͬ
*/
#pragma once
#ifndef SPHERE_H
#define SPHERE_H

#include "hittable.h"

// ��������hittable������Ϊ����
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
    // ������������ϵĵ�p���������꣬p��Բ����ԭ��ĵ�λ�����ϵ�����
    static void get_sphere_uv(const point3& p, double& u, double& v) {
        auto theta = acos(-p.y());
        auto phi = atan2(-p.z(), p.x()) + pi;

        u = phi / (2 * pi);
        v = theta / pi;
    }
};

// �õ�������ĳһʱ�̵�����λ��
point3 sphere::center(double time) const {
    if (time1 - time0 == 0) return center0;
    return center0 + ((time - time0) / (time1 - time0)) * (center1 - center0);
}

// ��������͹��ߵĽ���
bool sphere::hit(ray& r, double t_min, double t_max, hit_record& rec) const
{
    vec3 oc = r.origin() - center(r.time());
    auto a = r.direction().length_squared();
    auto half_b = dot(oc, r.direction());
    auto c = oc.length_squared() - radius * radius;

    auto discriminant = half_b * half_b - a * c;
    if (discriminant < 0) return false;
    auto sqrtd = sqrt(discriminant);

    // �ҵ���������������Ľ���
    auto root = (-half_b - sqrtd) / a;
    if (root < t_min || t_max < root) {
        root = (-half_b + sqrtd) / a;
        if (root < t_min || t_max < root)
            return false;
    }
    // ��¼�ý���������Ϣ
    rec.t = root;
    rec.p = r.at(rec.t);
    // ���߼ǵù�һ��
    vec3 outward_normal = (rec.p - center(r.time())) / radius;
    // �жϽ��������滹�Ǳ��棬��������ȷ�ķ��߷���
    rec.set_face_normal(r, outward_normal);
    // ������������
    get_sphere_uv(outward_normal, rec.u, rec.v);
    // ��¼����
    rec.mat_ptr = mat_ptr;

    return true;
};

// ��������İ�Χ��
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