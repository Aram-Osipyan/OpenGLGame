#pragma once
#include"Object.h"
#include "IMove.h"
class Grass : public Object, public IMove {
public:
    Grass(float x, float y, float z) :Object("assets/grass.png", "assets/grass.obj") {
        this->transform.position.x = x;
        this->transform.position.y = y;
        this->transform.position.z = z;
    }
    // Inherited via Object
    virtual void Start() override
    {
        this->transform.scale.x = 0.05;
        this->transform.scale.y = 0.01;
        this->transform.scale.z = 0.1;
        this->transform.rotation.z = 0.1;
        this->transform.position.y = -0.240;
    }
    virtual void Update(sf::Event event) override
    {
        float init_pos = -20;
        if (this->transform.position.z >= -init_pos)
        {
            this->transform.position.z = init_pos;
        }

        this->transform.position.z += _speed;
    }
};
