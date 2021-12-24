#pragma once
#include"Object.h"
#include "IMove.h"
#include <glm/common.hpp>
#include <vector>
template <typename T> int sign(T val);

class Car : public Object {
private:
    float initYrot = 0;
    std::vector<IMove*> _move_objects;
    void ClampYRotate(float step = 0.005) {
        if (abs(initYrot - this->transform.rotation.y) > step)
        {
            this->transform.rotation.y += sign(initYrot - this->transform.rotation.y) * step;
        }
    }
public:
    Car() : Object("assets/bus2.png", "assets/bus2.obj") {
    }
    Car(const std::vector<IMove*>& move_objects) : Object("assets/bus2.png", "assets/bus2.obj") {
        for(IMove* var : move_objects)
        {
            _move_objects.push_back(var);
        }
    }
    // Inherited via Object
    virtual void Start() override
    {
        this->transform.scale.x = 0.03;
        this->transform.scale.y = 0.03;
        this->transform.scale.z = 0.03;
        this->transform.position.y = 0.03;
    }
    virtual void Update(sf::Event event) override
    {
        if (event.type == sf::Event::KeyPressed) {
            switch (event.key.code) {
            case (sf::Keyboard::D): {
                this->transform.position.x -= 0.01f;
                this->transform.rotation.y = -0.04f;

            };
                                  break;
            case (sf::Keyboard::A): {
                this->transform.position.x += 0.01f;
                this->transform.rotation.y = 0.04f;
            }
                                  break;

            default: break;
            }
        }

        if (this->transform.position.x>0.5 || this->transform.position.x < -0.5)
        {
            for (IMove* var : _move_objects)
            {
                var->SetSlow();
            }
        }
        else {
            for (IMove* var : _move_objects)
            {
                var->SetFast();
            }
        }
        ClampYRotate();
    }
};
template <typename T> int sign(T val) {
    return (T(0) < val) - (val < T(0));
}