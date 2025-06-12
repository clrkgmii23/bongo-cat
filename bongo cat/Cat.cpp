#include "Cat.h"

Cat::Cat() :state({ false, false }) {};

bool Cat::ChangeState(CatStates var, bool val) {
	switch (var)
	{
		case LEFT_PAW:
			state.left_paw = val;
			return state.left_paw;
		case RIGHT_PAW:
			state.right_paw = val;
			return state.right_paw;
	}
	return false;
}