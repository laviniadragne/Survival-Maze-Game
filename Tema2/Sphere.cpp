#pragma once
#include "components/simple_scene.h"
#include "components/transform.h"

using namespace std;

class Sphere
{
public:
    float center_x;
    float center_y;
    float center_z;
    float radius;

    Sphere() {
        center_x = 0;
        center_y = 0;
        center_z = 0;
        radius = 0;
    }
    Sphere(float center_x, float center_y, float center_z,
           float radius) {
        this->center_x = center_x;
        this->center_y = center_y;
        this->center_z = center_z;
        this->radius = radius;
    }

};