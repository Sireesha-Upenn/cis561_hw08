#include "spotlight.h"

Color3f SpotLight::L(const Intersection &isect, const Vector3f &w) const
{
    return Color3f(0.f);
}
static float DistanceSquared(const Point3f &p1, const Point3f &p2)
{
    return glm::length(p1 - p2) * glm::length(p1 - p2);
}

Color3f SpotLight::Sample_Li(const Intersection &ref, const Point2f &u, Vector3f *wi, Float *pdf, const Scene& scene) const
{
    *wi = glm::normalize(transform.position() - ref.point);
    *pdf = 1.f;
    return I * Falloff(-*wi) / DistanceSquared(transform.position(), ref.point);
}

Float SpotLight::Falloff(const Vector3f &w) const
{
    Vector3f wl = glm::normalize(transform.rotateT() * w);
    Float cosTheta = -wl.y;
    if (cosTheta < cosTotalWidth) return 0;
    if (cosTheta >= cosFalloffStart) return 1;
    Float delta = (cosTheta - cosTotalWidth) / (cosFalloffStart - cosTotalWidth);
    return (delta * delta) * (delta * delta);
}

Color3f SpotLight::Power() const
{
    return I * 2.f * Pi * (1.f - 0.5f * (cosFalloffStart + cosTotalWidth));
}

Float SpotLight::Pdf_Li(const Intersection &, const Vector3f &) const
{
    return 0.f;
}
