#pragma once
#include "components/simple_scene.h"
#include "components/transform.h"

using namespace std;

class Cube
{
public:
    float center_x;
    float center_y;
    float center_z;
    float side_x;
    float side_y;
    float side_z;

    Cube() {
        center_x = 0;
        center_y = 0;
        center_z = 0;
        side_x = 0;
        side_y = 0;
        side_z = 0;
    }
    Cube(float center_x, float center_y, float center_z, float side_x,
         float side_y, float side_z) {
        this->center_x = center_x;
        this->center_y = center_y;
        this->center_z = center_z;
        this->side_x = side_x;
        this->side_y = side_y;
        this->side_z = side_z;
       
    }

};