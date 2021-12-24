#include <iostream>
#include <gl/glew.h>
#include <SFML/OpenGL.hpp>
#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include  <random>
#include  <iterator>
#include "Object.h"
#include "Road.h"
#include "Car.h"
#include "Grass.h"
#include "IMove.h"

#define PI 3.1415926f
#define WINDOW_WIDTH  1000
#define WINDOW_HEIGHT 1000
#define ToRadian(x) (float)(((x) * PI / 180.0f))

template <typename valType>
std::vector<valType> perspective
(
    valType const& fovy,
    valType const& aspect,
    valType const& zNear,
    valType const& zFar
)
{
    assert(aspect != valType(0));
    assert(zFar != zNear);

    valType const rad = ToRadian(fovy);

    valType tanHalfFovy = tan(rad / valType(2));

    std::vector<valType> Result = std::vector<valType>(16, valType(0));
    Result[0] = valType(1) / (aspect * tanHalfFovy);
    Result[5] = valType(1) / (tanHalfFovy);
    Result[10] = -(zFar + zNear) / (zFar - zNear);
    Result[14] = -valType(1);
    Result[11] = -(valType(2) * zFar * zNear) / (zFar - zNear);
    return Result;
}
std::vector<float> perspective(float r) {
    std::vector<float> result = {
        1,0,0,0,
        0,1,0,0,
        0,0,1,-1.0f/r,
        0,0,0,1
    };
    return result;
}

template<typename Iter, typename RandomGenerator>
Iter select_randomly(Iter start, Iter end, RandomGenerator& g) {
    std::uniform_int_distribution<> dis(0, std::distance(start, end) - 1);
    std::advance(start, dis(g));
    return start;
}

template<typename Iter>
Iter select_randomly(Iter start, Iter end) {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    return select_randomly(start, end, gen);
}




class Obstacle : public Object, public IMove {
    std::vector<float> x_positions = {0,0.3,-0.3};
public:
    Obstacle(std::string texture_path,std::string obj_path) : Object(texture_path, obj_path) {

    }
    virtual void Update(sf::Event event) override
    {
        
        float init_pos = -6;
        if (this->transform.position.z >= -init_pos)
        {
            this->transform.position.z = init_pos;
            this->transform.position.x = *select_randomly(x_positions.begin(), x_positions.end());
        }

        this->transform.position.z += _speed;
    }
};

class Cone : public Obstacle {
public:
    Cone(float initZPos = -13) : Obstacle("assets/cone.png", "assets/cone.obj") {
        this->transform.position.z = initZPos;
    }
    // Inherited via Obstacle
    virtual void Start() override
    {
        this->transform.scale.x = 0.03;
        this->transform.scale.y = 0.03;
        this->transform.scale.z = 0.03;
        this->transform.position.y = 0.03;
    }
};

class Elk : public Obstacle {
public:
    Elk(float initZPos = -20) : Obstacle("assets/los.png", "assets/los.obj") {
        this->transform.position.z = initZPos;
    }
    // Inherited via Obstacle
    virtual void Start() override
    {
        this->transform.scale.x = 0.03;
        this->transform.scale.y = 0.03;
        this->transform.scale.z = 0.03;
        this->transform.position.y = 0.03;
        this->transform.rotation.y = PI;
    }
};

class Picture : public Object {
private:

public:
    Picture(std::string file_name) : Object(file_name, "assets/rectangle.obj") {
    }
    // Inherited via Object
    virtual void Start() override
    {
        this->transform.scale.x = 2;
        this->transform.scale.y = 2;
        this->transform.scale.z = 2;
        this->transform.rotation.x = PI;
        this->transform.position.x = -1;
        this->transform.position.y = 1;
        this->transform.position.z = 0;

    }
    virtual void Update(sf::Event event) override
    {

    }
};
inline void wait_on_enter()
{
    std::string dummy;
    std::cout << "Enter to continue..." << std::endl;
    std::getline(std::cin, dummy);
}
int main()
{
GAME_START:

    ////////////////////////////////////////////////
    Object::Vector3 cameraPos = Object::Vector3(-0.1f, 0.7f, 0.1f);
    Object::Vector3 cameraFront = Object::Vector3(33.5f, -56.2f, 274.0f);
    Object::Vector3 cameraUp = Object::Vector3(0.0f, 1.0f, 0.0f);
    float fov = 45.0f;
    //////////////////////////////////////////////////


    sf::Window window(sf::VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), "My OpenGL window", sf::Style::Default, sf::ContextSettings(24));
    window.setVerticalSyncEnabled(true);
    window.setActive(true);

    // Инициализация glew
    glewInit();
    std::vector<Object*> objects_on_scene;
    std::vector<Obstacle*> obstacles;
    std::vector<IMove*> move_objects;
    float init_pos = -20;
    Road road = Road(0);
    Road road2 = Road(init_pos);
    
    Grass grass = Grass(0,0, 0);
    Grass grass2 = Grass(0,0, init_pos);

    Cone cone1 = Cone();
    Elk elk = Elk();

    Picture pict = Picture("assets/gameover.png");

    Picture morning_sky = Picture("assets/morning.png");
    Picture night_sky = Picture("assets/night.png");

    Picture *sky = &morning_sky;

    move_objects.push_back(&grass);
    move_objects.push_back(&grass2);
    move_objects.push_back(&road);
    move_objects.push_back(&road2);
    move_objects.push_back(&cone1);
    move_objects.push_back(&elk);
    Car car = Car(move_objects);
    objects_on_scene.push_back(&road);
    objects_on_scene.push_back(&road2);
    objects_on_scene.push_back(&car);
    objects_on_scene.push_back(&grass);
    objects_on_scene.push_back(&grass2);

    objects_on_scene.push_back(&cone1);
    objects_on_scene.push_back(&elk);


    obstacles.push_back(&cone1);
    obstacles.push_back(&elk);
    pict.Start();

    for (Object* obj : objects_on_scene) {
        obj->Start();
    }

    Object::Vector3 lightPos(1.2f, 1.0f, 0.0f);
    Object::Vector3 lightOn(-1.0f, 0.3f, 0.0f);
    Object::Vector3 lightOff(0.0f, 0.0f, 0.0f);
    Object::Vector3* view = &lightOn;

    float a = -36.1f;
    float b = 40.0f;
    float aspect = WINDOW_WIDTH/ WINDOW_HEIGHT;
    float r = -1.089;
    std::vector<float> projection = perspective(r);
    bool GAMEOVER_FLAG = false;
    while (window.isOpen()) {
        sf::Event event;
        window.pollEvent(event);

        if (event.type == sf::Event::Closed) {
            window.close();
        }
        else if (event.type == sf::Event::Resized) {
            glViewport(0, 0, event.size.width, event.size.height);
        }
        // обработка нажатий клавиш
        else if (event.type == sf::Event::KeyPressed) {
            switch (event.key.code) {
            case (sf::Keyboard::Q): view = &lightOn; break;
            case (sf::Keyboard::E): view = &lightOff; break;

            /*case (sf::Keyboard::Z): cameraPos.x += 0.1f; break;
            case (sf::Keyboard::X): cameraPos.x -= 0.1f; break;
            case (sf::Keyboard::C): cameraPos.y += 0.1f; break;
            case (sf::Keyboard::V): cameraPos.y -= 0.1f; break; 
            case (sf::Keyboard::B): cameraPos.z += 0.1f; break;
            case (sf::Keyboard::N): cameraPos.z -= 0.1f; break;*/

            /*case (sf::Keyboard::E): cameraFront.x += 1.0f; break;
            case (sf::Keyboard::R): cameraFront.x -= 1.1f; break;
            case (sf::Keyboard::T): cameraFront.y += 1.1f; break;
            case (sf::Keyboard::Y): cameraFront.y -= 1.1f; break;
            case (sf::Keyboard::U): cameraFront.z += 1.1f; break;
            case (sf::Keyboard::I): cameraFront.z -= 1.1f; break;*/
            //case (sf::Keyboard::E): view->x += 0.1f; break;
            case (sf::Keyboard::R): view->x -= 0.1f; break;
            case (sf::Keyboard::T): view->y += 0.1f; break;
            case (sf::Keyboard::Y): view->y -= 0.1f; break;
            case (sf::Keyboard::U): view->z += 0.1f; break;
            case (sf::Keyboard::I): view->z -= 0.1f; break;

            case (sf::Keyboard::Z): lightPos.x += 0.1f; break;
            case (sf::Keyboard::X): lightPos.x -= 0.1f; break;
            case (sf::Keyboard::C): lightPos.y += 0.1f; break;
            case (sf::Keyboard::V): lightPos.y -= 0.1f; break;
            case (sf::Keyboard::B): lightPos.z += 0.1f; break;
            case (sf::Keyboard::N): lightPos.z -= 0.1f; break;

            default: break;
            }
            if (GAMEOVER_FLAG)
            {
                goto GAME_START;
            }
        }
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        if (!GAMEOVER_FLAG)
        {
            morning_sky.Draw(lightPos, *view);
            for (Object* obj : objects_on_scene) {
                
                obj->Draw(lightPos, *view, projection, cameraPos, cameraFront, cameraUp);
                obj->Update(event);
            }
        }
        else {
            pict.Draw(lightPos, *view);            
        }
        
        for (Obstacle * obstacle : obstacles )
        {
            if (Object::Vector3::distance(obstacle->transform.position,car.transform.position)<0.1)
            {                
                GAMEOVER_FLAG = true;
            }
        }
        
        window.display();
    }

    return 0;
}

