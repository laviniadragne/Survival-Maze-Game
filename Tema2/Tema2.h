#pragma once

#include "components/simple_scene.h"
#include "components/transform.h"
#include "Maze.cpp"
#include "Projectile.cpp"
#include "Enemy.cpp"
#include "Cube.cpp"
#include "Sphere.cpp"
#include "DeformedEnemy.cpp"
#include "lab_m1/Tema2/lab_camera.h"
#include "lab_m1/Tema2/transformObj2D.h"
#include "lab_m1/Tema2/objectT2D.h"

//#include "lab_m1/Tema2/object2D.h"
//#include "lab_m1/Tema2/object2D.cpp"
#include <cstdlib>
#include <math.h>

#define NUM_WALL 4
#define H_SCENE 13
#define W_SCENE 13
#define MAZE_X 5.5f
#define MAZE_Y 0.5f
#define MAZE_Z 5.5f
#define BOX "box"
#define WHITE glm::vec3(1.0f, 1.0f, 1.0f)
#define BLACK glm::vec3(0.0f, 0.0f, 0.0f)
#define GRAY glm::vec3(0.329412, 0.329412, 0.329412)
#define GREEN glm::vec3( 0.137255, 0.556863, 0.137255)
#define BEIGE glm::vec3(0.5, 1.0, 0.0)
#define BLUE glm::vec3(0.196078, 0.196078, 0.8)
#define YELLOW glm::vec3(1, 1, 0)
#define RED glm::vec3(1, 0, 0)
#define GOLD glm::vec3 (0.8, 0.498039, 0.196078)
#define PI 3.14
#define LIM_PROJECTILE 6
#define DIST_CAMERA_FIRST 0.5f
#define DIST_CAMERA_THIRD 1.5f
#define ORANGE glm::vec3(1.0, 0.5, 0.0)
#define BRONZE glm::vec3(0.82, 0.57, 0.46)
#define MEDIUM_RED glm::vec3(1.0, 0.25, 0)
#define BROWN glm::vec3(0.36, 0.25, 0.20)
#define PLUM glm::vec3(0.917647, 0.678431, 0.917647)
#define INDIAN_RED glm::vec3(0.309804, 0.184314, 0.184314)
#define SIDE_ENEMIE_X 0.5
#define SIDE_ENEMIE_Y 1.2
#define SIDE_ENEMIE_Z 0.5
#define TIME_DISAPPEARANCE 1
#define CORNER glm::vec3(0, 0, 0)
#define SQUARE "square"
#define BORDER "border"
#define SQUARE_TIME "square_time"
#define BORDER_TIME "border_time"
#define STANDARD_LEN 1
#define LOGIC_SPACE_X  0
#define LOGIC_SPACE_Y  0
#define LOGIC_SPACE_WIDTH 4
#define LOGIC_SPACE_HEIGHT 4
#define LIFE_X 6.5
#define LIFE_Y 1
#define TRANS_LIFE_X -1.2
#define TRANS_LIFE_Y 0.2
#define MAX_LIVES 8
#define MAX_TIME 120
#define LIM_LIFE 3
#define LIM_ARMS_ANGLE 0.52


using namespace std;
using namespace implemented;

namespace m1
{
    class Tema2 : public gfxc::SimpleScene
    {
    public:
        struct ViewportSpace
        {
            ViewportSpace() : x(0), y(0), width(1), height(1) {}
            ViewportSpace(int x, int y, int width, int height)
                : x(x), y(y), width(width), height(height) {}
            int x;
            int y;
            int width;
            int height;
        };

        struct LogicSpace
        {
            LogicSpace() : x(0), y(0), width(1), height(1) {}
            LogicSpace(float x, float y, float width, float height)
                : x(x), y(y), width(width), height(height) {}
            float x;
            float y;
            float width;
            float height;
        };

        Tema2();
        ~Tema2();

        void Init() override;

    private:
        void FrameStart() override;
        void Update(float deltaTimeSeconds) override;
        void FrameEnd() override;

        void RenderSimpleMesh(Mesh* mesh, Shader* shader, const glm::mat4& modelMatrix, const glm::vec3& color = glm::vec3(1));

        void OnInputUpdate(float deltaTime, int mods) override;
        void OnKeyPress(int key, int mods) override;
        void OnKeyRelease(int key, int mods) override;
        void OnMouseMove(int mouseX, int mouseY, int deltaX, int deltaY) override;
        void OnMouseBtnPress(int mouseX, int mouseY, int button, int mods) override;
        void OnMouseBtnRelease(int mouseX, int mouseY, int button, int mods) override;
        void OnMouseScroll(int mouseX, int mouseY, int offsetX, int offsetY) override;
        void OnWindowResize(int width, int height) override;

        void SetViewportArea(const ViewportSpace& viewSpace, glm::vec3 colorColor, bool clear);

        std::pair <int, int> InitPosCharacter();
        Maze* GenerateMaze();
        std::vector<std::pair <int, int>> FindNeighbors(int H, int W, int r, int c,
                                                        std::vector<std::vector<int>>, int is_wall);
        void DrawMaze();
        void DrawCharacter(); 
        void DrawProjectiles();
        void DrawEnemies(float deltaTimeSeconds);
        void DrawIncreaseLife();
        void DrawExit();
        void DrawDeformedEnemy(float deltaTimeSeconds);
        bool CollisionBoxBox(Cube a, Cube b);
        bool CollisionSphereBox(Sphere sphere, Cube box);
        bool VerifyCharacterCollision(Cube ch);
        bool VerifyCharacterWalls(Cube ch);
        bool VerifyCharacterEnemies(Cube ch);
        void UpdateEnemies();
        void UpdateIncreaseLife();
        void CreateWallS();
        bool isInteger(double N);
        void RenderMyMesh(Mesh* mesh, Shader* shader, const glm::mat4& modelMatrix,
            const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix, const glm::vec3& color);
        glm::mat3 VisualizationTransf2DUnif(const LogicSpace& logicSpace, const ViewportSpace& viewSpace);

        glm::vec3 lightPosition;
        unsigned int materialShininess;
        float materialKd;
        float materialKs;

        Maze* maze;
        vector<Projectile*> projectiles;
        vector<Enemy*> enemies;
        vector<DeformedEnemy*> deformed_enemies;
        vector<Cube*> walls;
        vector<Cube*> increser_lives;
        float CHAR_POS_X;
        float CHAR_POS_Z;
        float CHAR_POS_Y;
        float translateX;
        float translateY;
        float translateZ;

        // Camera
        ViewportSpace viewSpace;
        LogicSpace logicSpace;
        glm::mat3 visMatrix, modelMatrix;
        MyCamera* camera;
        MyCamera* health_camera;
        MyCamera* time_camera;
        Cube* exit;
        glm::mat4 projectionMatrix;
        glm::mat4 projectionOrtoMatrix;

        bool renderCameraTarget;

        // TODO(student): If you need any other class variables, define them here.
        float left = -5;
        float right = 5;
        float bottom = -3;
        float top = 3;
        float zNear = 0.01;
        float zFar = 200;
        float fov = 60;
        float aspect = window->props.aspectRatio;
        bool perspective_type;
        bool third_person;
        float START_X;
        float START_Y;
        float START_Z;
        float angle = 0;
        float arms_angle = 0;
        int lives = MAX_LIVES;
        bool end = false;
        float rest_time = MAX_TIME;
        // coordonatele iesirii
        float exit_x, exit_y, exit_z;
        int points = 0;
    };
}   // namespace m1
