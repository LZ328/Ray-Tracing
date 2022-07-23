/*
* Perlin������
*/

#pragma once
#ifndef PERLIN_H
#define PERLIN_H

#include "utilities.h"

class perlin {
public:
    perlin() {
        // ��������ݶ�����
        ranvec = new vec3[point_count];
        for (int i = 0; i < point_count; ++i) {
            ranvec[i] = normalize(random_vec(-1, 1));
        }
        // ��������ά�ȵ�P[n]����
        perm_x = perlin_generate_perm();
        perm_y = perlin_generate_perm();
        perm_z = perlin_generate_perm();
    }

    ~perlin() {
        delete[] ranvec;
        delete[] perm_x;
        delete[] perm_y;
        delete[] perm_z;
    }

    // Perlin ����������������ά�ռ�һ�㣬��������ֵ
    double noise(const point3& p) const {
        // ����ά�������С�������൱�ڵ�һ����λ������(����)����ά�ȵľ���
        auto u = p.x() - floor(p.x());
        auto v = p.y() - floor(p.y());
        auto w = p.z() - floor(p.z());
        // ����ά��������������֣����ڵ�P[n]�������ҵ����ұ��±�
        auto i = static_cast<int>(floor(p.x()));
        auto j = static_cast<int>(floor(p.y()));
        auto k = static_cast<int>(floor(p.z()));
        // �洢��õ�����İ˸����񶥵������ݶ�����
        vec3 c[2][2][2];
        // �����ұ��в��Ұ˸�����ݶ�����
        for (int di = 0; di < 2; di++)
            for (int dj = 0; dj < 2; dj++)
                for (int dk = 0; dk < 2; dk++)
                    // ����ǲ���λ�ӷ����൱�ڰ�����P[n]�����е��±��������������0~255֮�䣬�����±�Խ��
                    c[di][dj][dk] = ranvec[
                        perm_x[(i + di) & 255] ^
                            perm_y[(j + dj) & 255] ^
                            perm_z[(k + dk) & 255]
                    ];
        // �ð˸��ݶ������;���������˲����Բ�ֵ
        return perlin_interp(c, u, v, w);
    }

    // ���noise����γ�turbulence
    double turb(const point3& p, int depth = 7) const {
        auto accum = 0.0;
        auto temp_p = p;
        auto weight = 1.0;

        for (int i = 0; i < depth; i++) {
            accum += weight * noise(temp_p);
            weight *= 0.5;
            temp_p *= 2;
        }

        return fabs(accum);
    }

private:
    static const int point_count = 256;
    vec3* ranvec;       // �洢����ݶ��������ұ�
    int* perm_x;        // xά�ȵ�P[n]����
    int* perm_y;        // yά�ȵ�P[n]����
    int* perm_z;        // zά�ȵ�P[n]����

    // ���ɴ� 0 �� point_count - 1 ����������
    // ˼·��������˳������飬Ȼ��ʹ��ϴ���㷨����
    static int* perlin_generate_perm() {
        auto p = new int[point_count];

        for (int i = 0; i < perlin::point_count; i++)
            p[i] = i;

        permute(p, point_count);

        return p;
    }

    // ϴ���㷨�������飬�Ӻ���ǰ�������飬ÿ�������һ���±�͵�ǰ�±꽻��
    static void permute(int* p, int n) {
        for (int i = n - 1; i > 0; i--) {
            int target = random_int(0, i);
            int tmp = p[i];
            p[i] = p[target];
            p[target] = tmp;
        }
    }

    // ���˸�������ݶ������͵�p���˸�����ľ���������ˣ����������Բ�ֵ
    static double perlin_interp(vec3 c[2][2][2], double u, double v, double w) {
        // �����Ľ������Բ�ֵϵ��
        auto uu = fade(u);
        auto vv = fade(v);
        auto ww = fade(w);

        auto accum = 0.0;

        for (int i = 0; i < 2; i++)
            for (int j = 0; j < 2; j++)
                for (int k = 0; k < 2; k++) {
                    vec3 weight_v(u - i, v - j, w - k);
                    accum += (i * uu + (1 - i) * (1 - uu))
                        * (j * vv + (1 - j) * (1 - vv))
                        * (k * ww + (1 - k) * (1 - ww))
                        * dot(c[i][j][k], weight_v);
                }

        return accum;
    }

    // �������ߣ����ڻ������Բ�ֵϵ��
    static double fade(double x)
    {
        return x * x * x * (x * (x * 6 - 15) + 10);
    }
};

#endif