#include <iostream>
#include <string>
#include <omp.h>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"
#include "hittable_list.h"
#include "sphere.h"
#include "aarect.h"
#include "color.h"
#include "camera.h"
#include "sence.h"

int main()
{
    /****ͼƬ���棬����Ϊpng��ʽ****/
    std::string SavePath = "D:\\TechStack\\ComputerGraphics\\Ray Tracing in One Weekend Series\\Results\\The Next Week\\";
    std::string filename = "FinalSencetest2.png";
    std::string filepath = SavePath + filename;
    
    /*******ͼƬ����*******/ 
    // ��߱�
    auto aspect_ratio = 16.0 / 9.0;
    int image_width = 400;
    const int channel = 3;
    // ÿ�����صĲ�������
    int samples_per_pixel = 100;
    // �������ٵ������
    int min_bounce = 45;
    // ����˹���̶��㷨�������
    const double RR = 0.9;
 
    /*******��������*******/
    hittable_list world;

    point3 lookfrom;
    point3 lookat;
    auto vfov = 40.0;
    auto aperture = 0.0;
    color background(0, 0, 0);

    int sence = 8;
    switch (sence) {
    case 1: // ������ɳ���
        world = random_scene();
        background = color(0.70, 0.80, 1.00);
        lookfrom = point3(13, 2, 3);
        lookat = point3(0, 0, 0);
        vfov = 20.0;
        aperture = 0.1;
        break;
    case 2: // �������壬���Ը�������
        world = two_spheres();
        background = color(0.70, 0.80, 1.00);
        lookfrom = point3(13, 2, 3);
        lookat = point3(0, 0, 0);
        vfov = 20.0;
        break;
    case 3: // ����perlin����
        world = two_perlin_spheres();
        background = color(0.70, 0.80, 1.00);
        lookfrom = point3(13, 2, 3);
        lookat = point3(0, 0, 0);
        vfov = 20.0;
        break;
    case 4: // ���򣬲���ͼƬ����
        world = earth();
        background = color(0.70, 0.80, 1.00);
        lookfrom = point3(13, 2, 3);
        lookat = point3(0, 0, 0);
        vfov = 20.0;
        break;
    case 5: // �򵥹��ճ���
        world = simple_light();
        samples_per_pixel = 400;
        background = color(0, 0, 0);
        lookfrom = point3(26, 3, 6);
        lookat = point3(0, 2, 0);
        vfov = 20.0;
        break;
    case 6: // Cornell Box ����
        world = cornell_box();
        aspect_ratio = 1.0;
        image_width = 600;
        samples_per_pixel = 200;
        background = color(0, 0, 0);
        lookfrom = point3(278, 278, -800);
        lookat = point3(278, 278, 0);
        vfov = 40.0;
        break;
    case 7: // ���� Cornell Box ����
        world = cornell_smoke();
        aspect_ratio = 1.0;
        image_width = 600;
        samples_per_pixel = 200;
        lookfrom = point3(278, 278, -800);
        lookat = point3(278, 278, 0);
        vfov = 40.0;
        break;
    default:
    case 8: // ���������ճ���
        world = final_scene();
        aspect_ratio = 1.0;
        image_width = 800;
        samples_per_pixel = 10000;
        min_bounce = 95;
        background = color(0, 0, 0);
        lookfrom = point3(478, 278, -600);
        lookat = point3(278, 278, 0);
        vfov = 40.0;
        break;
    }
    
    // ʹ��static_cast������ȷ���߱�������������ʧ���ȵ�ת������֪�������½��е�
    // Ҳ�������Ķ��������������Ա��ȷ��ת����Ŀ�Ķ������������
    const int image_height = static_cast<int>(image_width / aspect_ratio);

    /*******�������*******/
    vec3 vup(0, 1, 0);
    auto dist_to_focus = 10.0;

    camera cam(lookfrom, lookat, vup, vfov, aspect_ratio, aperture, dist_to_focus, 0.0, 1.0);

    /******��Ⱦ����*****/
    // 3ͨ��ͼ�����һά������
    unsigned char* odata = (unsigned char*)malloc(image_width * image_height * channel);
    unsigned char* p = odata;
    for (int j = image_height - 1; j >= 0; --j) {
        // ��׼��������ʾ������Ϣ������ˢ����ʾ
        std::cerr << "\rScanlines remaining: " << j << ' ' << std::flush;
        for (int i = 0; i < image_width; ++i) {
            color pixel_color(0, 0, 0);
            for (int s = 0; s < samples_per_pixel; ++s) {
                // �������ڲ��������
                auto u = (i + random_double()) / (image_width - 1);
                auto v = (j + random_double()) / (image_height - 1);
                ray r = cam.get_ray(u, v);
                pixel_color += ray_color(r, background, world, min_bounce, RR);
            }
            write_color(p, pixel_color, samples_per_pixel);
        }
    }
    stbi_write_png(filepath.c_str(), image_width, image_height, channel, odata, 0);
    std::cerr << "\nDone.\n";
}