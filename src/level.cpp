#include "level.h"
#include "creaturas.h"
#include <vector>

std::vector<FloorTile*> FloorTile::tiles;
Goal* Goal::instance = nullptr;
void Goal::update()
{
	if((Player::instance->centre()-centre()).getLengthSquare()<4*4)
		StatusScene::win();
}
void RoundTile::render()
{
	v2i sc = spaceToScr(centre());
	//s_qline({int(sc.x-r*getScalar()), sc.y}, {int(sc.x-r*getScalar()), shitrndr::WindowProps::getHeight()}, '|');
	//s_qline({int(sc.x+r*getScalar()), sc.y}, {int(sc.x+r*getScalar()), shitrndr::WindowProps::getHeight()}, '|');
	s_qfcircle(sc+v2i(0,FONT_SIZE), r*getScalar()/FONT_SIZE-1, L'▒', 0, C_GRAY);
	s_qfcircle(sc, r*getScalar()/FONT_SIZE-1, ' ');
	s_qfcircle(sc, r*getScalar()/FONT_SIZE-1, BCHARS[int(BCHARS_C*(1-std::max(0.f, std::min(1.f,
						5*Player::instance->getFlame()/(Player::instance->centre()-centre()).getLengthSquare()))))], 0, C_GRAY);
}
