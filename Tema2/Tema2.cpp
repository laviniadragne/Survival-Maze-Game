#include "lab_m1/Tema2/Tema2.h"

#include <vector>
#include <string>
#include <iostream>
#include <algorithm>
#include <chrono>
#include <random>

using namespace std;
using namespace m1;
using namespace implemented;

/*
 *  To find out more about `FrameStart`, `Update`, `FrameEnd`
 *  and the order in which they are called, see `world.cpp`.
 */


Tema2::Tema2()
{
}


Tema2::~Tema2()
{
}


void Tema2::Init()
{
    translateX = 0;
    translateY = 0;
    translateZ = 0;

    logicSpace.x = LOGIC_SPACE_X;       // logic x
    logicSpace.y = LOGIC_SPACE_Y;       // logic y
    logicSpace.width = LOGIC_SPACE_WIDTH;   // logic width
    logicSpace.height = LOGIC_SPACE_HEIGHT;  // logic height

    // Generez labirintul
    maze = GenerateMaze();

    // Generez pozitiile initiale ale personajului
    pair <int, int> init_pos = InitPosCharacter();
    int C_ROW = init_pos.first;
    int C_COL = init_pos.second;

    cout << "ALESE INITIAL RANDOM: "<<C_ROW << " " << C_COL << "\n";
    // Pozitia initiala a personajului
    CHAR_POS_X = 3 * (MAZE_X - C_COL);
    CHAR_POS_Z = 3 * (MAZE_Z - C_ROW);
    CHAR_POS_Y = MAZE_Y;

    // coordonatele iesirii
    exit_x = 3 * (MAZE_X - maze->ccol);
    exit_y = MAZE_Y;
    exit_z = 3 * (MAZE_Z - maze->crow);
    exit = new Cube(exit_x, exit_y, exit_z, 3, 3, 3);

    renderCameraTarget = false;

    // centrul patratului in care va fi randat personajul
    START_X = 3 * (MAZE_X - C_COL);
    START_Z = 3 * (MAZE_Z - C_ROW);
    START_Y = MAZE_Y;

    // Setez camera in third person
    third_person = true;
    camera = new MyCamera();
    camera->Set(glm::vec3(START_X, START_Y + 0.375f, START_Z - 1.5),
        glm::vec3(START_X, START_Y + 0.375f, START_Z + 1), glm::vec3(0, 1, 0));

    glm::ivec2 resolution = window->GetResolution();
    // camera pt health bar
    health_camera = new MyCamera();
    health_camera->Set(glm::vec3 (resolution.x - 50, resolution.y - 50, 900),
        glm::vec3(resolution.x - 50, resolution.y - 50, 1000), glm::vec3(0, 1, 0));

    {
        Mesh* mesh = new Mesh("box");
        mesh->LoadMesh(PATH_JOIN(window->props.selfDir, RESOURCE_PATH::MODELS, "primitives"), "box.obj");
        meshes[mesh->GetMeshID()] = mesh;
    }

    {
        Mesh* mesh = new Mesh("sphere");
        mesh->LoadMesh(PATH_JOIN(window->props.selfDir, RESOURCE_PATH::MODELS, "primitives"), "sphere.obj");
        meshes[mesh->GetMeshID()] = mesh;
    }

    {
        Mesh* square = objectT2D::CreateSquare(SQUARE, CORNER, STANDARD_LEN, RED, true);
        AddMeshToList(square);
    }

    {
        Mesh* square = objectT2D::CreateSquare(BORDER, CORNER, STANDARD_LEN, RED, false);
        AddMeshToList(square);
    }


    {
        Mesh* mesh = new Mesh("plane");
        mesh->LoadMesh(PATH_JOIN(window->props.selfDir, RESOURCE_PATH::MODELS, "primitives"), "plane50.obj");
        meshes[mesh->GetMeshID()] = mesh;
    }

    // Create a shader program for drawing face polygon with the color of the normal
    {
        Shader* shader = new Shader("LabShader");
        shader->AddShader(PATH_JOIN(window->props.selfDir, SOURCE_PATH::M1, "Tema2", "shaders", "VertexShader.glsl"), GL_VERTEX_SHADER);
        shader->AddShader(PATH_JOIN(window->props.selfDir, SOURCE_PATH::M1, "Tema2", "shaders", "FragmentShader.glsl"), GL_FRAGMENT_SHADER);
        shader->CreateAndLink();
        shaders[shader->GetName()] = shader;
    }

    // Shader pentru inamici
    {
        Shader* shader = new Shader("EnemieShader");
        shader->AddShader(PATH_JOIN(window->props.selfDir, SOURCE_PATH::M1, "Tema2", "shaders", "VertexShaderEnemie.glsl"), GL_VERTEX_SHADER);
        shader->AddShader(PATH_JOIN(window->props.selfDir, SOURCE_PATH::M1, "Tema2", "shaders", "FragmentShaderEnemie.glsl"), GL_FRAGMENT_SHADER);
        shader->CreateAndLink();
        shaders[shader->GetName()] = shader;
    }

    // Light & material properties
    {
        lightPosition = glm::vec3(0, 1, 1);
        materialShininess = 30;
        materialKd = 0.5;
        materialKs = 0.5;
    }


     // Randez personajul
     {
         glm::mat4 modelMatrix = glm::mat4(1);
         modelMatrix = glm::translate(modelMatrix, glm::vec3(CHAR_POS_X, CHAR_POS_Y, CHAR_POS_Z));
         modelMatrix = glm::scale(modelMatrix, glm::vec3(0.25f));
         RenderSimpleMesh(meshes["box"], shaders["LabShader"], modelMatrix, BEIGE);
     }

     projectionMatrix = glm::perspective(RADIANS(60), window->props.aspectRatio, 0.01f, 200.0f);

     // creez vectorul de pereti
     CreateWallS();

     // projectionOrtoMatrix = glm::ortho(left, right, bottom, top, zNear, zFar);
}


void Tema2::FrameStart()
{
    // Clears the color buffer (using the previously set color) and depth buffer
    glClearColor(0, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glm::ivec2 resolution = window->GetResolution();
    // Sets the screen area where to draw
    glViewport(0, 0, resolution.x, resolution.y);
}

glm::mat3 Tema2::VisualizationTransf2DUnif(const LogicSpace& logicSpace, const ViewportSpace& viewSpace)
{
    float sx, sy, tx, ty, smin;
    sx = viewSpace.width / logicSpace.width;
    sy = viewSpace.height / logicSpace.height;
    if (sx < sy)
        smin = sx;
    else
        smin = sy;
    tx = viewSpace.x - smin * logicSpace.x + (viewSpace.width - smin * logicSpace.width) / 2;
    ty = viewSpace.y - smin * logicSpace.y + (viewSpace.height - smin * logicSpace.height) / 2;

    return glm::transpose(glm::mat3(
        smin, 0.0f, tx,
        0.0f, smin, ty,
        0.0f, 0.0f, 1.0f));
}

void Tema2::SetViewportArea(const ViewportSpace& viewSpace, glm::vec3 colorColor, bool clear)
{
    auto camera2D = GetSceneCamera();
    camera2D->SetPosition(glm::vec3(0, 0, 50));
    camera2D->SetRotation(glm::vec3(0, 0, 0));
    GetCameraInput()->SetActive(false);

    glViewport(viewSpace.x, viewSpace.y, viewSpace.width, viewSpace.height);

    glEnable(GL_SCISSOR_TEST);
    glScissor(viewSpace.x, viewSpace.y, viewSpace.width, viewSpace.height);

    // Clears the color buffer (using the previously set color) and depth buffer
    glClearColor(colorColor.r, colorColor.g, colorColor.b, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glDisable(GL_SCISSOR_TEST);

    camera2D->SetOrthographic((float)viewSpace.x, (float)(viewSpace.x + viewSpace.width), (float)viewSpace.y, (float)(viewSpace.y + viewSpace.height), 0.1f, 400);
    camera2D->Update();
}


// Gaseste vecinii
std::vector <std::pair <int, int>> Tema2::FindNeighbors(int H, int W, int r, int c,
                                                        vector<vector<int>> grid, int is_wall)
{
    vector <std::pair <int, int>> ns;
    if (r > 1 && grid[r - 2][c] == is_wall) {
        ns.push_back(make_pair(r - 2, c));
    }
    if (r < H - 2 && grid[r + 2][c] == is_wall) {
        ns.push_back(make_pair(r + 2, c));
    }
    if (c > 1 && grid[r][c - 2] == is_wall) {
        ns.push_back(make_pair(r, c - 2));
    }
    if (c < W - 2 && grid[r][c + 2] == is_wall) {
        ns.push_back(make_pair(r, c + 2));
    }

    // randomizez vecinii
    unsigned num = chrono::system_clock::now().time_since_epoch().count();
    shuffle(ns.begin(), ns.end(), default_random_engine(num));

    return ns;
}

// Functie de generat matricea pentru labirint
Maze* Tema2::GenerateMaze()
{
    int H, W;
    H = H_SCENE;
    W = W_SCENE;

    vector<vector<int>> grid(H, vector<int>(W, 1));

    int step = 2;
    int init_crow, init_ccol;
    // aleg un perete random pentru iesire
    int rand_wall = rand() % (NUM_WALL - 1 + 1) + 1;
    int crow = (rand() % ((H - 1 - 1) / step)) * step + 1;
    int ccol = (rand() % ((W - 1 - 1) / step)) * step + 1;

    if (rand_wall == 1) {
        crow = 1;
    }
    if (rand_wall == 2) {
        ccol = 1;
    }

    if (rand_wall == 3) {
        crow = H - 2;
    }

    if (rand_wall == 4) {
        ccol = W - 2;
    }

    init_ccol = ccol;
    init_crow = crow;

    grid[crow][ccol] = 0;
  /*  cout << crow << " " << ccol << "\n";*/
    int num_visited = 1;
    int h = (H - 1) / 2;
    int w = (W - 1) / 2;

    vector <std::pair <int, int>> ns;
    // gaseste vecinii
    while (num_visited < h * w) {
        ns = FindNeighbors(H, W, crow, ccol, grid, 1);

        if (ns.size() == 0) {
            ns = FindNeighbors(H, W, crow, ccol, grid, 0);
            int index = rand() % (ns.size() - 1);
            crow = get<0>(ns[index]);
            ccol = get<1>(ns[index]);
            continue;
        }


        int nrow, ncol;
        for (vector <pair<int, int>> ::const_iterator it = ns.begin(); it != ns.end(); it++) {
            nrow = it->first;
            ncol = it->second;
            if (grid[nrow][ncol] > 0) {
                grid[(nrow + crow) / 2][(ncol + ccol) / 2] = 0;
                grid[nrow][ncol] = 0;
                num_visited++;
                crow = nrow;
                ccol = ncol;
                break;
            }
        }
    }

    float exit_row = init_crow;
    float exit_col = init_ccol;

    // sparg peretele spre iesire
    if (rand_wall == 1) {
        grid[init_crow - 1][init_ccol] = 0;
        exit_row = init_crow - 2;
    }

    if (rand_wall == 2) {
        grid[init_crow][init_ccol - 1] = 0;
        exit_col = init_ccol - 2;

    }

    if (rand_wall == 3) {
        grid[init_crow + 1][init_ccol] = 0;
        exit_row = init_crow + 2;
    }

    if (rand_wall == 4) {
        grid[init_crow][init_ccol + 1] = 0;
        exit_col = init_ccol + 2;
    }
    
    for (int i = 0; i < H; i++) {
        for (int j = 0; j < W; j++) {
            if (grid[i][j] == 0)
                cout << ".";
            else
                cout << "#";
        }
        cout << "\n";
    }

    Maze* my_maze = new Maze(grid, exit_row, exit_col, rand_wall);
    return my_maze;
}

// Gaseste pozitia de start random a jucatorului din cele libere
pair <int, int> Tema2::InitPosCharacter() 
{
    // salvez toate spatiile goale
    vector <std::pair <int, int>> white;
    for (int i = 0; i < H_SCENE; i++) {
        for (int j = 0; j < W_SCENE; j++) {
            if (maze->grid[i][j] == 0) {
                white.push_back(make_pair(i, j));
            }
        }
    }

    // generez random indicele pentru jucator
    int num = rand() % (white.size() - 1);

    // intr-unul din 4 patrate goale va fi un inamic
    for (int i = 0; i < white.size(); i += 4) {
        // nu randez un inamic fix in jucator
        if (i != num) {
            // il pozitionez in coltul stanga jos al casutei
            Enemy* new_enemy = new Enemy(white[i].first, white[i].second,
                                        3 * (MAZE_X - white[i].second),
                                        MAZE_Y, 3 * (MAZE_Z - white[i].first),
                                        3 * (MAZE_X - white[i].second) + 1.25,
                                        MAZE_Y, 3 * (MAZE_Z - white[i].first) - 1.25,
                                        1);
            enemies.push_back(new_enemy);
        }
    }

    // din 7 in 7 casute va fi un element de crescut viata
    for (int i = 1; i < white.size(); i += 7) {

        // nu randez o viata fix in jucator sau intr-un inamic
        if (i != num && (i % 4 != 0)) {
            Cube* new_increaser_life = new Cube(3 * (MAZE_X - white[i].second),
                MAZE_Y, 3 * (MAZE_Z - white[i].first), 0.25, 0.25, 0.25);
            increser_lives.push_back(new_increaser_life);
        }
    }

    return white[num];
}

void Tema2::CreateWallS()
{
    for (int i = 0; i < H_SCENE; i++) {
        for (int j = 0; j < W_SCENE; j++) {
            // e perete
            if (maze->grid[i][j] == 1)
            {
                Cube* new_wall = new Cube(3 * MAZE_X - 3 * j, MAZE_Y, 3 * MAZE_Z - 3 * i, 3, 3, 3);
                walls.push_back(new_wall);
            }
        }
    }
}

// Functia deseneaza labirintul pe baza gridului
void Tema2::DrawMaze()
{
    for (int i = 0; i < H_SCENE; i++) {
        for (int j = 0; j < W_SCENE; j++) {
            // e perete
            if (maze->grid[i][j] == 1) 
            {
                glm::mat4 modelMatrix = glm::mat4(1);
                modelMatrix = glm::translate(modelMatrix, glm::vec3(3 * MAZE_X - 3 * j, MAZE_Y, 3 * MAZE_Z - 3 * i));
                modelMatrix = glm::scale(modelMatrix, glm::vec3(3, 3, 3));
                RenderSimpleMesh(meshes[BOX], shaders["LabShader"], modelMatrix, BROWN);
            }
        }
    }
}

void Tema2::DrawCharacter()
{
    // Randez personajul
   

    float my_angle = angle - PI / 2;
    // Randez capul
    {
        glm::mat4 modelMatrix = glm::mat4(1);
        modelMatrix = glm::translate(modelMatrix, glm::vec3(translateX, translateY, translateZ));
        modelMatrix = glm::translate(modelMatrix, glm::vec3(CHAR_POS_X, CHAR_POS_Y, CHAR_POS_Z));
        // il aduc cu centrul pe oy si ox = 0
        modelMatrix = glm::translate(modelMatrix, glm::vec3(0, 0.375f, 0));
        modelMatrix = glm::rotate(modelMatrix, -angle, glm::vec3(0, 1, 0));
        modelMatrix = glm::scale(modelMatrix, glm::vec3(0.25f, 0.25f, 0.25f));
        RenderSimpleMesh(meshes["box"], shaders["LabShader"], modelMatrix, BRONZE);
    }

    // Randez corpul
    {
        glm::mat4 modelMatrix = glm::mat4(1);
        modelMatrix = glm::translate(modelMatrix, glm::vec3(translateX, translateY, translateZ));
        modelMatrix = glm::translate(modelMatrix, glm::vec3(CHAR_POS_X, CHAR_POS_Y, CHAR_POS_Z));
        modelMatrix = glm::translate(modelMatrix, glm::vec3(0, 0.0625f, 0));
        modelMatrix = glm::rotate(modelMatrix, -angle, glm::vec3(0, 1, 0));
        modelMatrix = glm::scale(modelMatrix, glm::vec3(0.25f, 0.375f, 0.25f));
        RenderSimpleMesh(meshes["box"], shaders["LabShader"], modelMatrix, GREEN);
    }

    // Randez bratele
    {
        glm::mat4 modelMatrix = glm::mat4(1);
        modelMatrix = glm::translate(modelMatrix, glm::vec3(translateX, translateY, translateZ));
        modelMatrix = glm::translate(modelMatrix, glm::vec3(CHAR_POS_X, CHAR_POS_Y, CHAR_POS_Z));
        modelMatrix = glm::rotate(modelMatrix, -angle, glm::vec3(0, 1, 0));
        modelMatrix = glm::rotate(modelMatrix, arms_angle, glm::vec3(1, 0, 0));
        modelMatrix = glm::translate(modelMatrix, glm::vec3(-0.3125f - 0.01f, 0.0625f, 0));
        modelMatrix = glm::scale(modelMatrix, glm::vec3(0.375f - 0.01f, 0.375f, 0.25f));
        RenderSimpleMesh(meshes["box"], shaders["LabShader"], modelMatrix, GREEN);
    }

    {
        glm::mat4 modelMatrix = glm::mat4(1);
        modelMatrix = glm::translate(modelMatrix, glm::vec3(translateX, translateY, translateZ));
        modelMatrix = glm::translate(modelMatrix, glm::vec3(CHAR_POS_X, CHAR_POS_Y, CHAR_POS_Z));
        modelMatrix = glm::rotate(modelMatrix, -angle, glm::vec3(0, 1, 0));
        modelMatrix = glm::rotate(modelMatrix, arms_angle, glm::vec3(1, 0, 0));
        modelMatrix = glm::translate(modelMatrix, glm::vec3(0.3125f + 0.01f, 0.0625f, 0));
        modelMatrix = glm::scale(modelMatrix, glm::vec3(0.375f - 0.01f, 0.375f, 0.25f));
        RenderSimpleMesh(meshes["box"], shaders["LabShader"], modelMatrix, GREEN);
    }

    // Randez mainile
    {
        glm::mat4 modelMatrix = glm::mat4(1);
        modelMatrix = glm::translate(modelMatrix, glm::vec3(translateX, translateY, translateZ));
        modelMatrix = glm::translate(modelMatrix, glm::vec3(CHAR_POS_X, CHAR_POS_Y, CHAR_POS_Z));
        modelMatrix = glm::rotate(modelMatrix, -angle, glm::vec3(0, 1, 0));
        modelMatrix = glm::rotate(modelMatrix, arms_angle, glm::vec3(1, 0, 0));
        modelMatrix = glm::translate(modelMatrix, glm::vec3(-0.3125f - 0.01f, -0.1875f, 0));
        modelMatrix = glm::scale(modelMatrix, glm::vec3(0.375f - 0.01f, 0.125f, 0.25f));
        RenderSimpleMesh(meshes["box"], shaders["LabShader"], modelMatrix, BRONZE);
    }

    {
        glm::mat4 modelMatrix = glm::mat4(1);
        modelMatrix = glm::translate(modelMatrix, glm::vec3(translateX, translateY, translateZ));
        modelMatrix = glm::translate(modelMatrix, glm::vec3(CHAR_POS_X, CHAR_POS_Y, CHAR_POS_Z));
        modelMatrix = glm::rotate(modelMatrix, -angle, glm::vec3(0, 1, 0));
        modelMatrix = glm::rotate(modelMatrix, arms_angle, glm::vec3(1, 0, 0));
        modelMatrix = glm::translate(modelMatrix, glm::vec3(0.3125f + 0.01f, -0.1875f, 0));
        modelMatrix = glm::scale(modelMatrix, glm::vec3(0.375f - 0.01f, 0.125f, 0.25f));
        RenderSimpleMesh(meshes["box"], shaders["LabShader"], modelMatrix, BRONZE);
    }

    // Randez picioarele
    {
        glm::mat4 modelMatrix = glm::mat4(1);
        modelMatrix = glm::translate(modelMatrix, glm::vec3(translateX, translateY, translateZ));
        modelMatrix = glm::translate(modelMatrix, glm::vec3(CHAR_POS_X, CHAR_POS_Y, CHAR_POS_Z));
        modelMatrix = glm::rotate(modelMatrix, -angle, glm::vec3(0, 1, 0));
        modelMatrix = glm::translate(modelMatrix, glm::vec3(-0.0625f - 0.01f, -0.3125f, 0));
        modelMatrix = glm::scale(modelMatrix, glm::vec3(0.125f - 0.01f, 0.375f, 0.25f));
        RenderSimpleMesh(meshes["box"], shaders["LabShader"], modelMatrix, BLUE);
    }

    {
        glm::mat4 modelMatrix = glm::mat4(1);
        modelMatrix = glm::translate(modelMatrix, glm::vec3(translateX, translateY, translateZ));
        modelMatrix = glm::translate(modelMatrix, glm::vec3(CHAR_POS_X, CHAR_POS_Y, CHAR_POS_Z));
        modelMatrix = glm::rotate(modelMatrix, -angle, glm::vec3(0, 1, 0));
        modelMatrix = glm::translate(modelMatrix, glm::vec3(0.0625f + 0.01f, -0.3125f, 0));
        modelMatrix = glm::scale(modelMatrix, glm::vec3(0.125f - 0.01f, 0.375f, 0.25f));
        RenderSimpleMesh(meshes["box"], shaders["LabShader"], modelMatrix, BLUE);
    }
}

void Tema2::DrawProjectiles() {
    // le updatez pozitia in timp ce le parcurg si le randez
    // cu noua pozitie, daca nu au coliziuni si daca mai au viata
    float new_center_x, new_center_y, new_center_z;
    for (int i = 0; i < projectiles.size(); i++) {
        new_center_x = projectiles[i]->last_x + projectiles[i]->translate_x;
        new_center_y = projectiles[i]->last_y + projectiles[i]->translate_y;
        new_center_z = projectiles[i]->last_z + projectiles[i]->translate_z;

        // daca se afla inca in perimetru si (daca nu s-a lovit de nimic)!!!
        if ((new_center_x - projectiles[i]->center_x) * 
            (new_center_x - projectiles[i]->center_x) + 
            (new_center_z - projectiles[i]->center_z) *
            (new_center_z - projectiles[i]->center_z) < (LIM_PROJECTILE * LIM_PROJECTILE)) 
        {
            // il updatez si il desenez
            projectiles[i]->last_x = new_center_x;
            projectiles[i]->last_y = new_center_y;
            projectiles[i]->last_z = new_center_z;

            // daca este intre personaj si camera nu il desenez
            // camera este la 0.5 de personaj
            if ((new_center_x - projectiles[i]->center_x)*
                (new_center_x - projectiles[i]->center_x) +
                (new_center_z - projectiles[i]->center_z) *
                (new_center_z - projectiles[i]->center_z) > ((DIST_CAMERA_FIRST + 0.3f) * (DIST_CAMERA_FIRST + 0.3f))) {
                glm::mat4 modelMatrix = glm::mat4(1);
                modelMatrix = glm::translate(modelMatrix, glm::vec3(new_center_x, new_center_y, new_center_z));
                modelMatrix = glm::scale(modelMatrix, glm::vec3(0.25f, 0.25f, 0.25f));
                RenderSimpleMesh(meshes["sphere"], shaders["LabShader"], modelMatrix, BLACK);
            }

        }

        // trebuie scos din vectorul de proiectile
        else {
            projectiles.erase(projectiles.begin() + i);
            i--;
        }
    }
    
}

void Tema2::DrawEnemies(float deltaTimeSeconds) 
{
    float speed_enemy = 15;
    for (int i = 0; i < enemies.size(); i++) {
        // daca nu are coliziune, il desenez
        // altfel, il sterg din vector

        float new_center_x, new_center_y, new_center_z;
        bool noupdate = true;

        // e peretele 1
        if (enemies[i]->wall == 1) {
            // pe x scade pozitia
            new_center_x = enemies[i]->last_x - 1 * speed_enemy * deltaTimeSeconds;
            new_center_y = enemies[i]->last_y;
            // pe ramane constanta
            new_center_z = enemies[i]->last_z;
            // daca am iesit din casuta, nu desenez si trec la peretele urmator
            if (abs(enemies[i]->center_x - new_center_x) >= 1.25) {
                noupdate = false;
                enemies[i]->wall = 2;
            }
            else {
                enemies[i]->last_x = new_center_x;
                enemies[i]->last_z = new_center_z;
            }
        }

        // e peretele 2
        if (enemies[i]->wall == 2 && noupdate == true) {
            // pe x constant
            new_center_x = enemies[i]->last_x;
            new_center_y = enemies[i]->last_y;
            // pe z creste
            new_center_z = enemies[i]->last_z + 1 * speed_enemy * deltaTimeSeconds;
            // daca am iesit din casuta, nu desenez si trec la peretele urmator
            if (abs(enemies[i]->center_z - new_center_z) >= 1.25) {
                noupdate = false;
                enemies[i]->wall = 3;
            }
            else {
                enemies[i]->last_x = new_center_x;
                enemies[i]->last_z = new_center_z;
            }
        }

        // e peretele 4
        if (enemies[i]->wall == 3 && noupdate == true) {
            // pe x scade pozitia
            new_center_x = enemies[i]->last_x + 1 * speed_enemy * deltaTimeSeconds;
            new_center_y = enemies[i]->last_y;
            // pe ramane constanta
            new_center_z = enemies[i]->last_z;
            // daca am iesit din casuta, nu desenez si trec la peretele urmator
            if (abs(enemies[i]->center_x - new_center_x) >= 1.25) {
                noupdate = false;
                enemies[i]->wall = 4;
            }
            else {
                enemies[i]->last_x = new_center_x;
                enemies[i]->last_z = new_center_z;
            }
        }

        // e peretele 4
        if (enemies[i]->wall == 4 && noupdate == true) {
            // pe x scade pozitia
            new_center_x = enemies[i]->last_x;
            new_center_y = enemies[i]->last_y;
            // pe ramane constanta
            new_center_z = enemies[i]->last_z - 1 * speed_enemy * deltaTimeSeconds;
            // daca am iesit din casuta, nu desenez si trec la peretele urmator
            if (abs(enemies[i]->center_z - new_center_z) >= 1.25) {
                noupdate = false;
                enemies[i]->wall = 1;
            }
            else {
                enemies[i]->last_x = new_center_x;
                enemies[i]->last_z = new_center_z;
            }
        }
        glm::mat4 modelMatrix = glm::mat4(1);
        modelMatrix = glm::translate(modelMatrix, glm::vec3(new_center_x, new_center_y, new_center_z));
        modelMatrix = glm::scale(modelMatrix, glm::vec3(SIDE_ENEMIE_X, SIDE_ENEMIE_Y, SIDE_ENEMIE_Z));
        
        // desenez inamicii in functie de cata viata mi-a mai ramas
        if (lives > 3) {
            RenderSimpleMesh(meshes[BOX], shaders["LabShader"], modelMatrix, MEDIUM_RED);
        }
        else {
            RenderSimpleMesh(meshes[BOX], shaders["LabShader"], modelMatrix, INDIAN_RED);
        }
    }
}

// functia verifica coliziunile proiectil-inamic
void Tema2::UpdateEnemies() 
{
    for (int i = 0; i < enemies.size(); i++) {
        Cube* simulate_enemie = new Cube(enemies[i]->last_x, enemies[i]->last_y,
            enemies[i]->last_z, SIDE_ENEMIE_X, SIDE_ENEMIE_Y,
            SIDE_ENEMIE_Z);
        float enemie_out = false;
        for (int j = 0; j < projectiles.size(); j++) {
            Sphere* simulate_proj = new Sphere(projectiles[j]->last_x, projectiles[j]->last_y,
                projectiles[j]->last_z, projectiles[j]->radius);

            // daca e coliziune, deformez inamicul...
            // scot din vectorul de inamici si din vectorul de proiectile
           if (CollisionSphereBox((*simulate_proj), (*simulate_enemie))) {
               projectiles.erase(projectiles.begin() + j);
                j--;
                enemie_out = true;
            }
        }

        // fac deformarea
        if (enemie_out) {
            DeformedEnemy* new_enemy = new DeformedEnemy(enemies[i], TIME_DISAPPEARANCE);
            deformed_enemies.push_back(new_enemy);
            enemies.erase(enemies.begin() + i);
            i--;
            points++;
        }
    }
}

void Tema2::DrawIncreaseLife() {
    for (int i = 0; i < increser_lives.size(); i++) {
        float new_center_x, new_center_y, new_center_z;
        new_center_x = increser_lives[i]->center_x;
        new_center_y = increser_lives[i]->center_y;
        new_center_z = increser_lives[i]->center_z;

        glm::mat4 modelMatrix = glm::mat4(1);
        modelMatrix = glm::translate(modelMatrix, glm::vec3(new_center_x, new_center_y, new_center_z));
        modelMatrix = glm::scale(modelMatrix, glm::vec3(increser_lives[i]->side_x, increser_lives[i]->side_y,
            increser_lives[i]->side_z));

        RenderSimpleMesh(meshes[BOX], shaders["LabShader"], modelMatrix, GREEN);
    }
}

// verifica coliziunea personaj increaser-life
void Tema2::UpdateIncreaseLife() {
    for (int i = 0; i < increser_lives.size(); i++) {
        // daca e coliziune cu personajul sterg increaser-ul
        // life din vector si adaug 1 la lives
        Cube* character = new Cube(translateX + CHAR_POS_X, translateY + CHAR_POS_Y,
            translateZ + CHAR_POS_Z, 1, 1, 1);
        if (CollisionBoxBox(*increser_lives[i], (*character))) {
            increser_lives.erase(increser_lives.begin() + i);
            i--;
            // daca pierduse vreo viata pana acum
            // face recovery, altfel pick-up-ul
            // nu are niciun rol
            if (lives < MAX_LIVES) {
                lives++;
            }
        }
    }
}

// Functia returneaza daca exista coliziune box-box
bool Tema2::CollisionBoxBox(Cube a, Cube b) {
    float a_minX = a.center_x - (float) a.side_x / (float) 2;
    float a_maxX = a.center_x + (float) a.side_x / (float) 2;
    float a_minY = a.center_y - (float) a.side_y / (float) 2;
    float a_maxY = a.center_y + (float) a.side_y / (float) 2;
    float a_minZ = a.center_z - (float) a.side_z / (float) 2;
    float a_maxZ = a.center_z + (float) a.side_z / (float) 2;

    float b_minX = b.center_x - (float)b.side_x / (float) 2;
    float b_maxX = b.center_x + (float)b.side_x / (float) 2;
    float b_minY = b.center_y - (float)b.side_y / (float) 2;
    float b_maxY = b.center_y + (float)b.side_y / (float) 2;
    float b_minZ = b.center_z - (float)b.side_z / (float) 2;
    float b_maxZ = b.center_z + (float)b.side_z / (float) 2;

    return (a_minX <= b_maxX && a_maxX >= b_minX) &&
        (a_minY <= b_maxY && a_maxY >= b_minY) &&
        (a_minZ <= b_maxZ && a_maxZ >= b_minZ);
}

// returneaza true daca e coliziune
bool Tema2::CollisionSphereBox(Sphere sphere, Cube box) 
{
    float box_minX = box.center_x - (float)box.side_x / (float)2;
    float box_maxX = box.center_x + (float)box.side_x / (float)2;
    float box_minY = box.center_y - (float)box.side_y / (float)2;
    float box_maxY = box.center_y + (float)box.side_y / (float)2;
    float box_minZ = box.center_z - (float)box.side_z / (float)2;
    float box_maxZ = box.center_z + (float)box.side_z / (float)2;

    // get box closest point to sphere center by clamping
    float x = max(box_minX, min(sphere.center_x, box_maxX));
    float y = max(box_minY, min(sphere.center_y, box_maxY));
    float z = max(box_minZ, min(sphere.center_z, box_maxZ));

    // this is the same as isPointInsideSphere
    float distance = sqrt((x - sphere.center_x) * (x - sphere.center_x) +
        (y - sphere.center_y) * (y - sphere.center_y) +
        (z - sphere.center_z) * (z - sphere.center_z));

    return distance < sphere.radius;
}

// returneaza true daca exista coliziune perete-personaj
bool Tema2::VerifyCharacterWalls(Cube ch)
{
    for (int i = 0; i < walls.size(); i++) {
        if (CollisionBoxBox(ch, (*walls[i])) == true)
            return true;
    }

    return false;
}

// returneaza true daca exista coliziune perete-personaj
bool Tema2::VerifyCharacterEnemies(Cube ch)
{
    for (int i = 0; i < enemies.size(); i++) {
        Cube* simulate_enemie = new Cube(enemies[i]->last_x, enemies[i]->last_y,
            enemies[i]->last_z, SIDE_ENEMIE_X, SIDE_ENEMIE_Y, SIDE_ENEMIE_Z);
        // scot inamicul din vector
        if (CollisionBoxBox(ch, (*simulate_enemie)) == true)
        {
            enemies.erase(enemies.begin() + i);
            return true;
        }
            
    }
    return false;
}

// returneaza true daca se loveste de ceva
bool Tema2::VerifyCharacterCollision(Cube ch)
{
    if (VerifyCharacterWalls(ch) == true) {
        return true;
    }
    if (VerifyCharacterEnemies(ch) == true) {
        lives--;
        return true;
    }

    return false;
}

void Tema2::DrawExit() 
{
    glm::mat4 modelMatrix = glm::mat4(1);
    modelMatrix = glm::translate(modelMatrix, glm::vec3(exit_x, exit_y, exit_z));
    modelMatrix = glm::scale(modelMatrix, glm::vec3(3, 3, 3));
    RenderSimpleMesh(meshes[BOX], shaders["LabShader"], modelMatrix, GOLD);
}

void Tema2::DrawDeformedEnemy(float deltaTimeSeconds)
{
    for (int i = 0; i < deformed_enemies.size(); i++) {
        deformed_enemies[i]->rest_time -= deltaTimeSeconds;
        // deseneaza-l
        if (deformed_enemies[i]->rest_time > 0) {
            glm::mat4 modelMatrix = glm::mat4(1);
            modelMatrix = glm::translate(modelMatrix, glm::vec3(deformed_enemies[i]->last_x, 
                deformed_enemies[i]->last_y, deformed_enemies[i]->last_z));
            // sfera circumscrisa paralelipipedului
            modelMatrix = glm::scale(modelMatrix, glm::vec3((2 * SIDE_ENEMIE_X) / sqrt(3), 
                            (2 * SIDE_ENEMIE_Y) / sqrt (3), (2 * SIDE_ENEMIE_Z) / sqrt (3)));

            // Colorare inamici in functie de viata
            if (lives > 3) {
                RenderSimpleMesh(meshes["sphere"], shaders["EnemieShader"], modelMatrix, MEDIUM_RED);
            }
            else {
                RenderSimpleMesh(meshes["sphere"], shaders["EnemieShader"], modelMatrix, INDIAN_RED);
            }
        }
        else {
            deformed_enemies.erase(deformed_enemies.begin() + i);
            i--;
        }
    }
}

void Tema2::Update(float deltaTimeSeconds)
{
    Cube* ch = new Cube(translateX + CHAR_POS_X, translateY + CHAR_POS_Y,
            translateZ + CHAR_POS_Z, 1, 1, 1);
    bool outside = CollisionBoxBox((*ch), (*exit));

    if (lives > 0 && rest_time > 0 && !end && !outside)
    {
        rest_time -= deltaTimeSeconds;
        // Render ground
        {
            glm::mat4 modelMatrix = glm::mat4(1);
            modelMatrix = glm::translate(modelMatrix, glm::vec3(0, 0.01f, 0));
            modelMatrix = glm::scale(modelMatrix, glm::vec3(1.2f));
            RenderSimpleMesh(meshes["plane"], shaders["LabShader"], modelMatrix, GRAY);
        }

        // Desenez labirintul
        DrawMaze();

        // Desenez iesirea
        DrawExit();

        // Desenez caracterul
        DrawCharacter();

        // Deseneaza proiectilele
        DrawProjectiles();

        // Deseneaza inamicii
        DrawEnemies(deltaTimeSeconds);

        // Updatez inamicii
        UpdateEnemies();

        // Deseneaza inamicii deformati
        DrawDeformedEnemy(deltaTimeSeconds);

        // Deseneaza viata
        DrawIncreaseLife();

        // Updatez pick-up de increaser life
        UpdateIncreaseLife();

        {
            
            // LIFE BAR
            glm::ivec2 resolution = window->GetResolution();
            viewSpace = ViewportSpace(7 * resolution.x / 8, 7 * resolution.y / 8, resolution.x / 8, resolution.y / 8);
            SetViewportArea(viewSpace, BLACK, true);

            // Compute the 2D visualization matrix
            visMatrix = glm::mat3(1);
            visMatrix *= VisualizationTransf2DUnif(logicSpace, viewSpace);

            double len_x;
            len_x = ((double)(LIFE_X / MAX_LIVES)) * lives;
            if (lives > LIM_LIFE) {
                modelMatrix = visMatrix * transformObj2D::Translate(TRANS_LIFE_X, TRANS_LIFE_Y + 2);
                modelMatrix *= transformObj2D::Scale(len_x, LIFE_Y);
                RenderMesh2D(meshes[SQUARE], modelMatrix, RED);

                modelMatrix = visMatrix * transformObj2D::Translate(TRANS_LIFE_X, TRANS_LIFE_Y + 2);
                modelMatrix *= transformObj2D::Scale(LIFE_X, LIFE_Y);
                RenderMesh2D(meshes[BORDER], modelMatrix, RED);
            }
            else {
                modelMatrix = visMatrix * transformObj2D::Translate(TRANS_LIFE_X, TRANS_LIFE_Y + 2);
                modelMatrix *= transformObj2D::Scale(len_x, LIFE_Y);
                RenderMesh2D(meshes[SQUARE], modelMatrix, PLUM);

                modelMatrix = visMatrix * transformObj2D::Translate(TRANS_LIFE_X, TRANS_LIFE_Y + 2);
                modelMatrix *= transformObj2D::Scale(LIFE_X, LIFE_Y);
                RenderMesh2D(meshes[BORDER], modelMatrix, PLUM);
            }

            // TIMER
            len_x = ((double)(LIFE_X / MAX_TIME)) * rest_time;
            modelMatrix = visMatrix * transformObj2D::Translate(TRANS_LIFE_X, TRANS_LIFE_Y);
            modelMatrix *= transformObj2D::Scale(len_x, LIFE_Y);
            RenderMesh2D(meshes[SQUARE], modelMatrix, BLUE);

            modelMatrix = visMatrix * transformObj2D::Translate(TRANS_LIFE_X, TRANS_LIFE_Y);
            modelMatrix *= transformObj2D::Scale(LIFE_X, LIFE_Y);
            RenderMesh2D(meshes[BORDER], modelMatrix, BLUE);
        }

    }
    else {
       
        if (outside && !end) {
            cout << "\nPOINTS: " << points << "\n";
            cout << "#####VICTORY!\n######";
        }

        else {
            if (!end && rest_time <= 0) {
                cout << "\nPOINTS: " << points << "\n";
                cout << "\nTIME IS OVER\n";
            }

            if (lives <= 0 && !end) {
                cout << "\nPOINTS: " << points << "\n";
                cout << "\nNO MORE LIVES\n";
            }

            if (!end) {
                cout << "########GAME OVER########";
            }
        }

        end = true;
    }

}


void Tema2::FrameEnd()
{

}


// Render pentru healthbar 
void Tema2::RenderMyMesh(Mesh* mesh, Shader* shader, const glm::mat4& modelMatrix,
                        const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix, const glm::vec3& color)
{
    if (!mesh || !shader || !shader->GetProgramID())
        return;

    glUseProgram(shader->program);

    GLint loc_model_matrix = glGetUniformLocation(shader->program, "Model");
    glUniformMatrix4fv(loc_model_matrix, 1, GL_FALSE, glm::value_ptr(modelMatrix));

    // Bind view matrix
    int loc_view_matrix = glGetUniformLocation(shader->program, "View");
    glUniformMatrix4fv(loc_view_matrix, 1, GL_FALSE, glm::value_ptr(viewMatrix));

    // Bind projection matrix
    int loc_projection_matrix = glGetUniformLocation(shader->program, "Projection");
    glUniformMatrix4fv(loc_projection_matrix, 1, GL_FALSE, glm::value_ptr(projectionMatrix));

    // Draw the object
    glBindVertexArray(mesh->GetBuffers()->m_VAO);
    glDrawElements(mesh->GetDrawMode(), static_cast<int>(mesh->indices.size()), GL_UNSIGNED_INT, 0);
}


void Tema2::RenderSimpleMesh(Mesh* mesh, Shader* shader, const glm::mat4& modelMatrix, const glm::vec3& color)
{
    if (!mesh || !shader || !shader->GetProgramID())
        return;

    // Render an object using the specified shader and the specified position
    glUseProgram(shader->program);

    glUniform3fv(glGetUniformLocation(shader->program, "object_color"), 1, glm::value_ptr(color));


    // Bind model matrix
    GLint loc_model_matrix = glGetUniformLocation(shader->program, "Model");
    glUniformMatrix4fv(loc_model_matrix, 1, GL_FALSE, glm::value_ptr(modelMatrix));

    // Bind view matrix
    glm::mat4 viewMatrix = camera->GetViewMatrix();
    int loc_view_matrix = glGetUniformLocation(shader->program, "View");
    glUniformMatrix4fv(loc_view_matrix, 1, GL_FALSE, glm::value_ptr(viewMatrix));

    // Bind projection matrix
    int loc_projection_matrix = glGetUniformLocation(shader->program, "Projection");
    glUniformMatrix4fv(loc_projection_matrix, 1, GL_FALSE, glm::value_ptr(projectionMatrix));

    int locationT = glGetUniformLocation(shader->program, "Time");
    // Set shader uniform "Time"
    glUniform1f(locationT, (float)Engine::GetElapsedTime());

    // Draw the object
    glBindVertexArray(mesh->GetBuffers()->m_VAO);
    glDrawElements(mesh->GetDrawMode(), static_cast<int>(mesh->indices.size()), GL_UNSIGNED_INT, 0);
}


/*
 *  These are callback functions. To find more about callbacks and
 *  how they behave, see `input_controller.h`.
 */


void Tema2::OnInputUpdate(float deltaTime, int mods)
{
        float cameraSpeed = 2.0f;

        if (window->KeyHold(GLFW_KEY_W)) {
            // TODO(student): Translate the camera forward
            // 
            float newTranslateX = translateX + cameraSpeed * deltaTime * cos(angle + PI / 2);
            float newTranslateZ = translateZ + cameraSpeed * deltaTime * sin(angle + PI / 2);
            float newTranslateY = 0;
            Cube* character = new Cube(newTranslateX + CHAR_POS_X, newTranslateY + CHAR_POS_Y,
                newTranslateZ + CHAR_POS_Z, 1, 1, 1);
            // verificare personaj cu toti peretii
            bool collision = false;
            collision = VerifyCharacterCollision(*character);

            // unghiul pentru rotatie bratelor
            arms_angle += deltaTime;
            if (arms_angle > LIM_ARMS_ANGLE)
                arms_angle = 0;

            // daca nu exista coliziune mut personajul (prin translateX) si camera
            if (collision == false)
            {
                camera->TranslateForward(deltaTime * cameraSpeed);

                translateX += cameraSpeed * deltaTime * cos(angle + PI / 2);
                translateZ += cameraSpeed * deltaTime * sin(angle + PI / 2);
            }
        }

        if (window->KeyHold(GLFW_KEY_A)) {

            // TODO(student): Translate the camera to the left

            float newTranslateX = translateX + cameraSpeed * deltaTime * cos(angle);
            float newTranslateZ = translateZ + cameraSpeed * deltaTime * sin(angle);
            float newTranslateY = 0;
            Cube* character = new Cube(newTranslateX + CHAR_POS_X, newTranslateY + CHAR_POS_Y,
                newTranslateZ + CHAR_POS_Z, 1, 1, 1);
            // verificare personaj cu toti peretii
            bool collision = false;
            collision = VerifyCharacterCollision(*character);

            // daca nu exista coliziune mut personajul (prin translateX) si camera
            if (collision == false)
            {
                camera->TranslateRight(-deltaTime * cameraSpeed);
                translateX += cameraSpeed * deltaTime * cos(angle);
                translateZ += cameraSpeed * deltaTime * sin(angle);
            }
        }

        if (window->KeyHold(GLFW_KEY_S)) {
            // TODO(student): Translate the camera backward

            float newTranslateX = translateX - cameraSpeed * deltaTime * cos(angle + PI / 2);
            float newTranslateZ = translateZ - cameraSpeed * deltaTime * sin(angle + PI / 2);
            float newTranslateY = 0;
            Cube* character = new Cube(newTranslateX + CHAR_POS_X, newTranslateY + CHAR_POS_Y,
                newTranslateZ + CHAR_POS_Z, 1, 1, 1);
            // verificare personaj cu toti peretii
            bool collision = false;
            collision = VerifyCharacterCollision(*character);

            // unghiul pentru rotatie bratelor
            arms_angle += deltaTime;
            if (arms_angle > LIM_ARMS_ANGLE)
                arms_angle = 0;

            // daca nu exista coliziune mut personajul (prin translateX) si camera
            if (collision == false)
            {
                camera->TranslateForward(-cameraSpeed * deltaTime);
                translateX -= cameraSpeed * deltaTime * cos(angle + PI / 2);
                translateZ -= cameraSpeed * deltaTime * sin(angle + PI / 2);
            }
        }

        if (window->KeyHold(GLFW_KEY_D)) {
         
            // TODO(student): Translate the camera to the right
            float newTranslateX = translateX - cameraSpeed * deltaTime * cos(angle);
            float newTranslateZ = translateZ - cameraSpeed * deltaTime * sin(angle);
            float newTranslateY = 0;
            Cube* character = new Cube(newTranslateX + CHAR_POS_X, newTranslateY + CHAR_POS_Y,
                newTranslateZ + CHAR_POS_Z, 1, 1, 1);
            // verificare personaj cu toti peretii
            bool collision = false;
            collision = VerifyCharacterCollision(*character);

            // daca nu exista coliziune mut personajul (prin translateX) si camera
            if (collision == false)
            {
                camera->TranslateRight(deltaTime * cameraSpeed);
                translateX -= cameraSpeed * deltaTime * cos(angle);
                translateZ -= cameraSpeed * deltaTime * sin(angle);
            }
        }
}


void Tema2::OnKeyPress(int key, int mods)
{
    // Add key press event
     // Schimb in third person/ first person
    float speed = 2;
    float viewCameraX = (speed / 10) * cos(angle + PI / 2);
    float viewCameraZ = (speed / 10) * sin(angle + PI / 2);

    // daca sunt in first person si s-a apasat space trebuie sa lansez un proiectil
    if (!third_person && window->KeyHold(GLFW_KEY_SPACE)) {
        // centrul lui e unde e capul personajului
        Projectile* new_projectile = new Projectile(1, CHAR_POS_X + translateX,
            CHAR_POS_Y + translateY + 0.375f, CHAR_POS_Z + translateZ,
            CHAR_POS_X + translateX, CHAR_POS_Y + translateY + 0.375f,
            CHAR_POS_Z + translateZ, viewCameraX, 0, viewCameraZ);
        projectiles.push_back(new_projectile);
    }
}


void Tema2::OnKeyRelease(int key, int mods)
{
    // Add key release event
}


void Tema2::OnMouseMove(int mouseX, int mouseY, int deltaX, int deltaY)
{
    // Add mouse move event
    float sensivityOX = 0.008f;
    float sensivityOY = 0.008f;
    angle += deltaX * sensivityOY;
    camera->RotateThirdPerson_OY(-deltaX * sensivityOY);

}


void Tema2::OnMouseBtnPress(int mouseX, int mouseY, int button, int mods)
{
    // Add mouse button press event
    if (IS_BIT_SET(button, GLFW_MOUSE_BUTTON_RIGHT))
    {
        if (third_person == true) {
            // se va trece in first person
            camera->MoveForward(DIST_CAMERA_FIRST + DIST_CAMERA_THIRD);
            camera->distanceToTarget = -DIST_CAMERA_FIRST;
        }
        // se va trece in third person
        else {
            camera->MoveForward(-(DIST_CAMERA_FIRST + DIST_CAMERA_THIRD));
            camera->distanceToTarget = DIST_CAMERA_THIRD;
        }
        third_person = !third_person;

    }
}


void Tema2::OnMouseBtnRelease(int mouseX, int mouseY, int button, int mods)
{
    // Add mouse button release event
}


void Tema2::OnMouseScroll(int mouseX, int mouseY, int offsetX, int offsetY)
{
}


void Tema2::OnWindowResize(int width, int height)
{
}
