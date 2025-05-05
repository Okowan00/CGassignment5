#include <iostream>
#include <vector>
#include <array>
#include <limits>
#include <cmath>
#include <GL/freeglut.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

const int nx = 512, ny = 512;

struct Vec3 {
    float x, y, z;
};

struct Triangle {
    Vec3 v0, v1, v2;
};

struct Color { unsigned char r, g, b; };

float depthBuffer[ny][nx];
Color framebuffer[ny][nx];
std::vector<Triangle> triangles;

void clearBuffers() {
    for (int y = 0; y < ny; ++y)
        for (int x = 0; x < nx; ++x) {
            framebuffer[y][x] = { 0, 0, 0 };
            depthBuffer[y][x] = std::numeric_limits<float>::infinity();
        }
}

std::array<float, 3> barycentric(Vec3 a, Vec3 b, Vec3 c, Vec3 p) {
    float den = (b.y - c.y) * (a.x - c.x) + (c.x - b.x) * (a.y - c.y);
    float alpha = ((b.y - c.y) * (p.x - c.x) + (c.x - b.x) * (p.y - c.y)) / den;
    float beta = ((c.y - a.y) * (p.x - c.x) + (a.x - c.x) * (p.y - c.y)) / den;
    float gamma = 1.0f - alpha - beta;
    return { alpha, beta, gamma };
}

void rasterize() {
    for (const auto& tri : triangles) {
        float xmin = std::floor(std::min({ tri.v0.x, tri.v1.x, tri.v2.x }));
        float xmax = std::ceil(std::max({ tri.v0.x, tri.v1.x, tri.v2.x }));
        float ymin = std::floor(std::min({ tri.v0.y, tri.v1.y, tri.v2.y }));
        float ymax = std::ceil(std::max({ tri.v0.y, tri.v1.y, tri.v2.y }));

        for (int y = (int)ymin; y <= (int)ymax; ++y) {
            if (y < 0 || y >= ny) continue;
            for (int x = (int)xmin; x <= (int)xmax; ++x) {
                if (x < 0 || x >= nx) continue;
                Vec3 p = { (float)x + 0.5f, (float)y + 0.5f, 0 };
                std::array<float, 3> bary = barycentric(tri.v0, tri.v1, tri.v2, p);
                float alpha = bary[0], beta = bary[1], gamma = bary[2];
                if (alpha >= 0 && beta >= 0 && gamma >= 0) {
                    p.z = alpha * tri.v0.z + beta * tri.v1.z + gamma * tri.v2.z;
                    if (p.z < depthBuffer[y][x]) {
                        depthBuffer[y][x] = p.z;
                        framebuffer[y][x] = { 255, 255, 255 };
                    }
                }
            }
        }
    }
}

void createUnitSphere() {
    int stacks = 20, slices = 40;
    float radius = 2.0f;
    for (int i = 0; i < stacks; ++i) {
        float phi1 = M_PI * i / stacks;
        float phi2 = M_PI * (i + 1) / stacks;
        for (int j = 0; j < slices; ++j) {
            float theta1 = 2 * M_PI * j / slices;
            float theta2 = 2 * M_PI * (j + 1) / slices;

            Vec3 v0 = { radius * sinf(phi1) * cosf(theta1), radius * sinf(phi1) * sinf(theta1), radius * cosf(phi1) - 7 };
            Vec3 v1 = { radius * sinf(phi2) * cosf(theta1), radius * sinf(phi2) * sinf(theta1), radius * cosf(phi2) - 7 };
            Vec3 v2 = { radius * sinf(phi2) * cosf(theta2), radius * sinf(phi2) * sinf(theta2), radius * cosf(phi2) - 7 };
            Vec3 v3 = { radius * sinf(phi1) * cosf(theta2), radius * sinf(phi1) * sinf(theta2), radius * cosf(phi1) - 7 };

            triangles.push_back({ v0, v1, v2 });
            triangles.push_back({ v0, v2, v3 });
        }
    }
}

void applyTransformations() {
    float l = -0.1f, r = 0.1f, b = -0.1f, t = 0.1f, n = -0.1f, f = -1000.0f;
    for (auto& tri : triangles) {
        for (auto& v : { &tri.v0, &tri.v1, &tri.v2 }) {
            v->x = (v->x / -v->z) * n;
            v->y = (v->y / -v->z) * n;

            v->x = ((v->x - l) / (r - l)) * nx;
            v->y = ((v->y - b) / (t - b)) * ny;
        }
    }
}

void display() {
    glClear(GL_COLOR_BUFFER_BIT);
    glDrawPixels(nx, ny, GL_RGB, GL_UNSIGNED_BYTE, framebuffer);
    glFlush();
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);
    glutInitWindowSize(nx, ny);
    glutCreateWindow("Rasterizer");

    gluOrtho2D(0, nx, 0, ny);

    clearBuffers();
    createUnitSphere();
    applyTransformations();
    rasterize();

    glutDisplayFunc(display);
    glutMainLoop();
    return 0;
}
