#include "pointlight.h"

static float DistanceSquared(const Point3f &p1, const Point3f &p2)
{
    return glm::length(p1 - p2) * glm::length(p1 - p2);
}

Color3f PointLight::L(const Intersection &isect, const Vector3f &w) const
{
    return Color3f(0.f);
}

Color3f PointLight::Sample_Li(const Intersection &ref, const Point2f &u,
                              Vector3f *wi, Float *pdf, const Scene& scene) const
{
    *wi = glm::normalize(transform.position() - ref.point);
    *pdf = 1.f;
    return emittedLight / DistanceSquared(transform.position(), ref.point);
}

Float PointLight::Pdf_Li(const Intersection &, const Vector3f &) const
{
    return 0.f;
}
