/*
* 包含全部测试场景
*/

#pragma once

#include "hittable_list.h"
#include "sphere.h"
#include "aarect.h"
#include "box.h"
#include "constant_medium.h"
#include "bvh.h"

// 随机构建场景
hittable_list random_scene() {
    hittable_list world;

    auto checker = make_shared<checker_texture>(color(0.2, 0.3, 0.1), color(0.9, 0.9, 0.9));
    auto ground_material = make_shared<lambertian>(checker);
    world.add(make_shared<sphere>(
        point3(0, -1000, 0), point3(0, -1000, 0), 0.0, 1.0, 1000, ground_material));

    for (int a = -11; a < 11; a++) {
        for (int b = -11; b < 11; b++) {
            auto choose_mat = random_double();
            point3 center(a + 0.9 * random_double(), 0.2, b + 0.9 * random_double());

            if ((center - point3(4, 0.2, 0)).length() > 0.9) {
                shared_ptr<material> sphere_material;

                if (choose_mat < 0.6) {
                    // diffuse
                    auto albedo = random_vec() * random_vec();
                    sphere_material = make_shared<lambertian>(albedo);
                    // 移动的小球
                    auto center2 = center + vec3(0, random_double(0, 0.5), 0);
                    world.add(make_shared<sphere>(
                        center, center2, 0.0, 1.0, 0.2, sphere_material));
                }
                else if (choose_mat < 0.8) {
                    // 大理石小球
                    auto pertext = make_shared<noise_texture>(4);
                    sphere_material = make_shared<lambertian>(pertext);
                    world.add(make_shared<sphere>(
                        center, center, 0.0, 1.0, 0.2, sphere_material));
                }
                else if (choose_mat < 0.9) {
                    // metal
                    auto albedo = random_vec(0.5, 1);
                    auto fuzz = random_double(0, 0.5);
                    sphere_material = make_shared<metal>(albedo, fuzz);
                    world.add(make_shared<sphere>(
                        center, center, 0.0, 1.0, 0.2, sphere_material));
                }
                else {
                    // glass
                    sphere_material = make_shared<dielectric>(1.5);
                    world.add(make_shared<sphere>(
                        center, center, 0.0, 1.0, 0.2, sphere_material));
                }
            }
        }
    }

    auto material1 = make_shared<dielectric>(1.5);
    world.add(make_shared<sphere>(
        point3(0, 1, 0), point3(0, 1, 0), 0.0, 1.0, 1.0, material1));

    auto material2 = make_shared<lambertian>(make_shared<noise_texture>(4));
    world.add(make_shared<sphere>(
        point3(-4, 1, 0), point3(-4, 1, 0), 0.0, 1.0, 1.0, material2));

    auto material3 = make_shared<metal>(color(0.7, 0.6, 0.5), 0.0);
    world.add(make_shared<sphere>(
        point3(4, 1, 0), point3(4, 1, 0), 0.0, 1.0, 1.0, material3));

    return world;
}

// 两个球体场景
hittable_list two_spheres() {
    hittable_list objects;

    auto checker = make_shared<checker_texture>(color(0.2, 0.3, 0.1), color(0.9, 0.9, 0.9));

    objects.add(make_shared<sphere>(
        point3(0, -10, 0), point3(0, -10, 0), 0.0, 1.0, 10, make_shared<lambertian>(checker)));
    objects.add(make_shared<sphere>(
        point3(0, 10, 0), point3(0, 10, 0), 0.0, 1.0, 10, make_shared<lambertian>(checker)));

    return objects;
}

// Perlin噪声测试场景
hittable_list two_perlin_spheres() {
    hittable_list objects;

    auto pertext = make_shared<noise_texture>(4);
    objects.add(make_shared<sphere>(
        point3(0, -1000, 0), point3(0, -1000, 0), 0.0, 1.0, 1000, make_shared<lambertian>(pertext)));
    objects.add(make_shared<sphere>(
        point3(0, 2, 0), point3(0, 2, 0), 0.0, 1.0, 2, make_shared<lambertian>(pertext)));

    return objects;
}

// 地球
hittable_list earth() {
    std::string TexturePath = 
        "D:\\TechStack\\ComputerGraphics\\Ray Tracing in One Weekend Series\\Project\\Textures\\earthmap.jpg";
    auto earth_texture = make_shared<image_texture>(TexturePath.c_str());
    auto earth_surface = make_shared<lambertian>(earth_texture);
    auto globe = make_shared<sphere>(
        point3(0, 0, 0), point3(0, 0, 0), 0.0, 1.0, 2, earth_surface);

    return hittable_list(globe);
}

// 简单光照场景
hittable_list simple_light() {
    hittable_list objects;

    auto pertext = make_shared<noise_texture>(4);
    objects.add(make_shared<sphere>(
        point3(0, -1000, 0), point3(0, -1000, 0), 0.0, 1.0, 1000, make_shared<lambertian>(pertext)));
    objects.add(make_shared<sphere>(
        point3(0, 2, 0), point3(0, 2, 0), 0.0, 1.0, 2, make_shared<lambertian>(pertext)));

    auto difflight = make_shared<diffuse_light>(color(4, 4, 4));
    objects.add(make_shared<xy_rect>(3, 5, 1, 3, -2, difflight));
    objects.add(make_shared<sphere>(
        point3(0, 6, 0), point3(0, 6, 0), 0.0, 1.0, 1.5, difflight));

    return objects;
}

// Cornell Box场景
hittable_list cornell_box() {
    hittable_list objects;

    auto red = make_shared<lambertian>(color(.65, .05, .05));
    auto white = make_shared<lambertian>(color(.73, .73, .73));
    auto green = make_shared<lambertian>(color(.12, .45, .15));
    auto light = make_shared<diffuse_light>(color(15, 15, 15));

    objects.add(make_shared<yz_rect>(0, 555, 0, 555, 555, green));
    objects.add(make_shared<yz_rect>(0, 555, 0, 555, 0, red));
    objects.add(make_shared<xz_rect>(213, 343, 227, 332, 554, light));
    objects.add(make_shared<xz_rect>(0, 555, 0, 555, 0, white));
    objects.add(make_shared<xz_rect>(0, 555, 0, 555, 555, white));
    objects.add(make_shared<xy_rect>(0, 555, 0, 555, 555, white));

    shared_ptr<hittable> box1 = make_shared<box>(point3(0, 0, 0), point3(165, 330, 165), white);
    box1 = make_shared<rotate_y>(box1, 15);
    box1 = make_shared<translate>(box1, vec3(265, 0, 295));
    objects.add(box1);

    shared_ptr<hittable> box2 = make_shared<box>(point3(0, 0, 0), point3(165, 165, 165), white);
    box2 = make_shared<rotate_y>(box2, -18);
    box2 = make_shared<translate>(box2, vec3(130, 0, 65));
    objects.add(box2);

    return objects;
}

// 烟雾Cornell Box场景
hittable_list cornell_smoke() {
    hittable_list objects;

    auto red = make_shared<lambertian>(color(.65, .05, .05));
    auto white = make_shared<lambertian>(color(.73, .73, .73));
    auto green = make_shared<lambertian>(color(.12, .45, .15));
    auto light = make_shared<diffuse_light>(color(7, 7, 7));

    objects.add(make_shared<yz_rect>(0, 555, 0, 555, 555, green));
    objects.add(make_shared<yz_rect>(0, 555, 0, 555, 0, red));
    objects.add(make_shared<xz_rect>(113, 443, 127, 432, 554, light));
    objects.add(make_shared<xz_rect>(0, 555, 0, 555, 555, white));
    objects.add(make_shared<xz_rect>(0, 555, 0, 555, 0, white));
    objects.add(make_shared<xy_rect>(0, 555, 0, 555, 555, white));

    shared_ptr<hittable> box1 = make_shared<box>(point3(0, 0, 0), point3(165, 330, 165), white);
    box1 = make_shared<rotate_y>(box1, 15);
    box1 = make_shared<translate>(box1, vec3(265, 0, 295));

    shared_ptr<hittable> box2 = make_shared<box>(point3(0, 0, 0), point3(165, 165, 165), white);
    box2 = make_shared<rotate_y>(box2, -18);
    box2 = make_shared<translate>(box2, vec3(130, 0, 65));

    objects.add(make_shared<constant_medium>(box1, 0.1, color(0, 0, 0)));
    objects.add(make_shared<constant_medium>(box2, 0.01, color(1, 1, 1)));

    return objects;
}

// 新特性最终场景
hittable_list final_scene() {
    // 高低起伏的盒子组成地面
    hittable_list boxes1;
    auto ground = make_shared<lambertian>(color(0.48, 0.83, 0.53));

    const int boxes_per_side = 20;
    for (int i = 0; i < boxes_per_side; i++) {
        for (int j = 0; j < boxes_per_side; j++) {
            auto w = 100.0;
            auto x0 = -1000.0 + i * w;
            auto z0 = -1000.0 + j * w;
            auto y0 = 0.0;
            auto x1 = x0 + w;
            auto y1 = random_double(1, 101);
            auto z1 = z0 + w;

            boxes1.add(make_shared<box>(point3(x0, y0, z0), point3(x1, y1, z1), ground));
        }
    }

    hittable_list objects;
    // 构建地面的BVH树
    objects.add(make_shared<bvh_node>(boxes1, 0, 1));
    // 光源
    auto light = make_shared<diffuse_light>(color(7, 7, 7));
    objects.add(make_shared<xz_rect>(123, 423, 147, 412, 554, light));
    // 移动的球体
    auto center1 = point3(400, 400, 200);
    auto center2 = center1 + vec3(30, 0, 0);
    auto moving_sphere_material = make_shared<lambertian>(color(0.7, 0.3, 0.1));
    objects.add(make_shared<sphere>(center1, center2, 0, 1, 50, moving_sphere_material));
    // 透明球体
    objects.add(make_shared<sphere>(
        point3(260, 150, 45), point3(260, 150, 45), 0, 1, 50, make_shared<dielectric>(1.5)));
    // 金属球体
    objects.add(make_shared<sphere>(
        point3(0, 150, 145), point3(0, 150, 145), 0, 1, 50, make_shared<metal>(color(0.8, 0.8, 0.9), 1.0)
        ));

    // 参与介质
    auto boundary = make_shared<sphere>(
        point3(360, 150, 145), point3(360, 150, 145), 0, 1, 70, make_shared<dielectric>(1.5));
    objects.add(boundary);
    objects.add(make_shared<constant_medium>(boundary, 0.01, color(0.2, 0.4, 0.9)));
    boundary = make_shared<sphere>(
        point3(0, 0, 0), point3(0, 0, 0), 0, 1, 5000, make_shared<dielectric>(1.5));
    objects.add(make_shared<constant_medium>(boundary, .0001, color(1, 1, 1)));

    // 地球
    auto emat = make_shared<lambertian>(make_shared<image_texture>(
        "D:\\TechStack\\ComputerGraphics\\Ray Tracing in One Weekend Series\\Project\\Textures\\earthmap.jpg"));
    objects.add(make_shared<sphere>(
        point3(400, 200, 400), point3(400, 200, 400), 0, 1, 100, emat));
    // 噪声纹理
    auto pertext = make_shared<noise_texture>(4);
    objects.add(make_shared<sphere>(
        point3(220, 280, 300), point3(220, 280, 300), 0, 1, 80, make_shared<lambertian>(pertext)));

    // 一堆小球
    hittable_list boxes2;
    auto white = make_shared<lambertian>(color(.73, .73, .73));
    int ns = 1000;
    for (int j = 0; j < ns; j++) {
        auto center = random_vec(0, 165);
        boxes2.add(make_shared<sphere>(
            center, center, 0, 1, 10, white));
    }
    // 旋转平移一堆小球
    objects.add(make_shared<translate>(
        make_shared<rotate_y>(
            make_shared<bvh_node>(boxes2, 0.0, 1.0), 15),
        vec3(-100, 270, 395)
        )
    );

    return objects;
}