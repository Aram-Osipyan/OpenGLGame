#pragma once
#include "Object.h"
#include "IMove.h"
class Road : public Object, public IMove {

public:
    Road(float z) :Object("assets/road.png", "assets/road.obj") {
        this->transform.position.z = z;
    }
    // Inherited via Object
    virtual void Start() override
    {
        this->transform.scale.x = 0.03;
        this->transform.scale.y = 0.02;
        this->transform.scale.z = 0.1;
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
