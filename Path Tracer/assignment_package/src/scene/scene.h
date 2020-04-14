#pragma once
#include <QList>
#include <raytracing/film.h>
#include <scene/camera.h>
#include <scene/lights/light.h>
#include <scene/thinlenscamera.h>
//#include <scene/geometry/csg.h>

class Primitive;
class Material;
class Light;

class Scene
{
public:
    Scene();
    QList<std::shared_ptr<Primitive>> primitives;
    QList<std::shared_ptr<Material>> materials;
    QList<std::shared_ptr<Light>> lights;
    Camera camera;
    ThinLensCamera thinLensCamera;
    Film film;

    void SetCamera(const Camera &c);
    void SetThinLensCamera(const ThinLensCamera &c);
    void CreateTestScene();
   // void CreateCSGTestScene();
    void Clear();

    bool Intersect(const Ray& ray, Intersection* isect) const;
};
