#pragma once
class IMove {
public:
	float _speed = 0.03;

    void SetSlow() {
        _speed = 0.01;
    }
    void SetFast() {
        _speed = 0.03;
    }
};