#pragma once
#include "components/simple_scene.h"
#include "components/transform.h"
#include "Enemy.cpp"

using namespace std;

class DeformedEnemy
{
public:
    float center_x;
    float center_y;
    float center_z;
    int ccol, crow;
    int wall;
    float last_x;
    float last_y;
    float last_z;
    float rest_time;

    DeformedEnemy() {
        crow = 0;
        ccol = 0;
        center_x = 0;
        center_y = 0;
        center_z = 0;
        wall = 0;
        last_x = 0;
        last_y = 0;
        last_z = 0;
        rest_time = 0;
    }
    DeformedEnemy(int crow, int ccol, float center_x, float center_y, float center_z,
            float last_x, float last_y, float last_z,
            int wall, int rest_time) {
        this->crow = crow;
        this->ccol = ccol;
        this->center_x = center_x;
        this->center_y = center_y;
        this->center_z = center_z;
        this->wall = wall;
        this->last_x = last_x;
        this->last_y = last_y;
        this->last_z = last_z;
        this->rest_time = rest_time;
    }
    DeformedEnemy(Enemy* e, float time) {
        this->crow = e->crow;
        this->ccol = e->ccol;
        this->center_x = e->center_x;
        this->center_y = e->center_y;
        this->center_z = e->center_z;
        this->wall = e->wall;
        this->last_x = e->last_x;
        this->last_y = e->last_y;
        this->last_z = e->last_z;
        this->rest_time = time;
    }

};