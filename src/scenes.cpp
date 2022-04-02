#include <cumt/cumt.h>
#include "scenes.h"

using namespace cumt;

std::vector<Scene*> Scene::scenes = {};
size_t Scene::active = 0;
size_t Scene::nactive = 0;

static float tt = 0;
static bool t = 1;

Scene* Scene::getActive() { return scenes[active]; }
void Scene::setActive(size_t i)
{
	nactive = i;
	t = 1;
	tt = 0;
}
void Scene::allStart()
{
	size_t i = 0;
	for(Scene* s : scenes)
	{
		s->index = i++;
		s->start();
	}
}
void Scene::update()
{
	if(active==nactive)
	{
		if(t)
		{
			tt += FD::delta/getActive()->ti_dur;
			getActive()->transIn(tt);
			if(tt>=1)t = 0;
		}
	}
	else
	{
		tt += FD::delta/getActive()->to_dur;
		getActive()->transOut(tt);
		if(tt>=1)
		{
			getActive()->unload();
			active = nactive;
			getActive()->load();
			tt = 0;
		}
	}
}
