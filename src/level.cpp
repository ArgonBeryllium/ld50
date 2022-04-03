#include "level.h"
#include "creaturas.h"
#include <vector>

std::vector<FloorTile*> FloorTile::tiles;
void Goal::update()
{
	if((Player::instance->centre()-centre()).getLengthSquare()<4*4)
		StatusScene::win();
}
