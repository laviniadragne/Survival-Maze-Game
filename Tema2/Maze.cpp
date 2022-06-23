#include "Maze.h"

using namespace std;

class Maze
{
public:
    vector<vector<int>> grid;
    int crow;
    int ccol;
    int wall;

    Maze() {
        crow = 0;
        ccol = 0;
        wall = 0;
    }
    Maze(vector<vector<int>>grid, int crow, int ccol, int wall) {
        this->grid = grid;
        this->crow = crow;
        this->ccol = ccol;
        this->wall = wall;
    }

};