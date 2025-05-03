#include "program.h"
#include "scene.h"
#include "vec.h"
#include "acc_bvh.h"
#include "acc_bbox.h"
#include "acc_none.h"
// Main code
int main(int, char **) {
	Program prog;
	Scene scene;
	scene.load_obj("../sponza.obj");
	auto bvh = AccelBvh(scene);
	auto t = timer();
	bvh.build();
	println("Polygons:", scene.m_poly.size(), "Builld toook:", timer(t));
	t = timer();
	Uint res = 0;
	Uint idx = 0;
	for(Uint i = 0; i < (1 << 20); i++){
		HitInfo  hi;
		res += bvh.intersect(Ray(Vec3f(0), 2.f * Vec3f(randfl(), randfl(), randfl())), hi);
		if(hi.idx != Uint(-1))
		idx += hi.idx;
	}
	println(res, idx, timer(t));
	//prog.run();
}