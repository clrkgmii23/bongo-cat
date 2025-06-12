#pragma once
enum CatStates
{
	LEFT_PAW,
	RIGHT_PAW
};

struct CatState {
	bool left_paw;
	bool right_paw;
};

class Cat {
public:
	Cat();
	bool ChangeState(CatStates var, bool val);
	bool isRightPaw() const { return state.right_paw; }
	bool isLeftPaw() const { return state.left_paw; }
private:
	CatState state;
};