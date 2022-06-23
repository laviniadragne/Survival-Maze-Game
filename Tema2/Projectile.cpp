#pragma once
#include "components/simple_scene.h"
#include "components/transform.h"

using namespace std;

class Projectile
{
public:
    float radius;
    float center_x;
    float center_y;
    float center_z;
    float last_x;
    float last_y;
    float last_z;
    float translate_x;
    float translate_y;
    float translate_z;

    Projectile() {
        radius = 0;
        center_x = 0;
        center_y = 0;
        center_z = 0;
        last_x = 0;
        last_y = 0;
        last_z = 0;
        translate_x = 0;
        translate_y = 0;
        translate_z = 0;
    }

    Projectile(float radius, float center_x, float center_y, float center_z, 
                float last_x, float last_y, float last_z, float translate_x, float translate_y,
                float translate_z) {
        this->radius = radius;
        this->center_x = center_x;
        this->center_y = center_y;
        this->center_z = center_z;
        this->last_x = last_x;
        this->last_y = last_y;
        this->last_z = last_z;
        this->translate_x = translate_x;
        this->translate_y = translate_y;
        this->translate_z = translate_z;

    }

};