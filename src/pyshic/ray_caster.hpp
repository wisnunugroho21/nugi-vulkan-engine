#include <math.h>

#include "arr3.h"
#include "ray.h"

#include "helper/helper.h"
#include "struct/hit_record.h"
#include "hittable/hittable.h"
#include "hittable/hittable_list.h"
#include "hittable/bvh.h"
#include "hittable/shape/xy_rect.h"
#include "hittable/shape/yz_rect.h"
#include "hittable/shape/xz_rect.h"
#include "hittable/shape/sphere.h"
#include "hittable/shape/box.h"

#include "../render/game_object/game_object.hpp"

#include <iostream>
#include <fstream>
#include <memory>

hittable_list makeHittableList(std::vector<std::shared_ptr<nugiEngine::EngineGameObject>> gameObjects) {
	hittable_list objects;

	for (auto &obj : gameObjects) {
		glm::vec4 minVec = obj->transform.mat4() * glm::vec4{obj->model->getMinimumPoint(), 1.0f};
		point3 minPoint = point3{static_cast<double>(minVec.x), static_cast<double>(minVec.y), static_cast<double>(minVec.z)};

		glm::vec4 maxVec = obj->transform.mat4() * glm::vec4{obj->model->getMaximumPoint(), 1.0f};
		point3 maxPoint = point3{static_cast<double>(maxVec.x), static_cast<double>(maxVec.y), static_cast<double>(maxVec.z)};

		std::shared_ptr<hittable> box1 = make_shared<box>(minPoint, maxPoint);
		box1->objectId = obj->getId();
		objects.add(make_shared<gameobject>(box1));
	}

	return objects;
}

uint32_t processHit(ray r, hittable& world) {
	hit_record res = world.hit(r, 0.001, infinity);
	return (res.is_hit) ? res.objectId : -1;
}

uint32_t rayCasting(glm::vec3 origin, glm::vec3 direction, std::vector<std::shared_ptr<nugiEngine::EngineGameObject>> gameObjects) {
	point3 originPoint{origin.x, origin.y, origin.z};
	vector3 directionVector{direction.x, direction.y, direction.z};

	ray rayCaster{originPoint, directionVector};

	auto objectList = makeHittableList(gameObjects);
	bvh_node bvhObjectList(objectList);

	return processHit(rayCaster, bvhObjectList);
}