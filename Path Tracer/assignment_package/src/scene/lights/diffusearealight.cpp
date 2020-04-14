#include "diffusearealight.h"

// Returns the energy emitted along _w_ from a point on our surface _isect_
// If _twoSided_ is false, then we must check if _w_ is in the same direction
// as the surface normal of _isect_. If we are looking at the back side of the
// light, then this function returns zero color
Color3f DiffuseAreaLight::L(const Intersection &isect, const Vector3f &w) const
{
    if(!twoSided)
    {
        return glm::dot(isect.normalGeometric, w) > 0.f ? emittedLight : Color3f(0.f);
    }

   return emittedLight;
}

Color3f DiffuseAreaLight::Sample_Li(const Intersection &ref, const Point2f &xi,
                                    Vector3f *wi, Float *pdf, const Scene& scene) const
{
    Intersection i = shape->Sample(ref, xi, pdf);

    //Check if the resultant PDF is zero or that the reference Intersection and the resultant Intersection are the same point in space,
    //and return black if this is the case.
    if(pdf == 0 || (i.point == ref.point))
    {
        return Color3f(0.f);
    }

    *wi = glm::normalize(i.point - ref.point);

    //Shadow Test
    Ray r(ref.point + (*wi * ShadowEpsilon), *wi);
    Intersection isectray;
    scene.Intersect(r, &isectray);

    float l1 = glm::length(isectray.point - ref.point) + ShadowEpsilon;
    float l2 = glm::length(i.point - ref.point);

    if(l1 < l2)
    {
        return Color3f(0.f);
    }

    return L(i, -*wi);
}

float DiffuseAreaLight::Pdf_Li(const Intersection &ref, const Vector3f &wi) const
{
    return shape->Pdf(ref, wi);
}
