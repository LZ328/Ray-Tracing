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
    std::string SavePath = "D:\\TechStack\\ComputerGraphics\\Ray Tracing in One Weekend Series\\Results\\The Rest of Life\\";
    std::string filename = "CornellBoxConv.png";
    std::string filepath = SavePath + filename;
    
    /*******ͼƬ����*******/ 
    // ��߱�
    auto aspect_ratio = 1.0;
    int image_width = 600;
    const int channel = 3;
    // ÿ�����صĲ�������
    int samples_per_pixel = 10000;
    // �������ٵ������
    int min_bounce = 95;
    // ����˹���̶��㷨�������
    const double RR = 0.9;
 
    /*******��������*******/
    hittable_list world = cornell_box();
    auto lights = make_shared<hittable_list>();
    lights->add(make_shared<xz_rect>(213, 343, 227, 332, 554, shared_ptr<material>()));
    // ȫ�����䳡������Ҫ����Բ������������Ҫע�͵�
    lights->add(make_shared<sphere>(
        point3(190, 90, 190), point3(190, 90, 190), 0, 1, 90, shared_ptr<material>()));

    point3 lookfrom = point3(278, 278, -800);
    point3 lookat = point3(278, 278, 0);
    auto vfov = 40.0;
    auto aperture = 0.0;
    color background(0, 0, 0);
       
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
                pixel_color += ray_color(r, background, world, lights, min_bounce, RR);
            }
            write_color(p, pixel_color, samples_per_pixel);
        }
    }
    stbi_write_png(filepath.c_str(), image_width, image_height, channel, odata, 0);
    std::cerr << "\nDone.\n";
}