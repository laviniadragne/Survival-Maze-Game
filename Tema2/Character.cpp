#pragma once
#include "components/simple_scene.h"
#include "components/transform.h"

using namespace std;

class Character
{
public:
    float center_x;
    float center_y;
    float center_z;
    int ccol, crow;

    Character() {
        crow = 0;
        ccol = 0;
        center_x = 0;
        center_y = 0;
        center_z = 0;
    }
    Character(int crow, int ccol, float center_x, float center_y, float center_z) {
        this->crow = crow;
        this->ccol = ccol;
        this->center_x = center_x;
        this->center_y = center_y;
        this->center_z = center_z;
    }

};