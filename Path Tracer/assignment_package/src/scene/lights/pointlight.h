#pragma once
#include "globals.h"
#include "la.h"
#include "light.h"

class PointLight  : public Light
{
public:
    PointLight(const Transform &t, const Color3f& Le)
        :Light(t), emittedLight(Le)
    {}

    const Color3f emittedLight;

    // Returns the energy emitted along _w_ from a point on our surface _isect_
    // If _twoSided_ is false, then we must check if _w_ is in the same direction
    // as the surface normal of _isect_. If we are looking at the back side of the
    // light, then this function returns zero color.
    virtual Color3f L(const Intersection &isect, const Vector3f &w) const;
    // Given a point of intersection on some surface in the scene and a pair
    // of uniform random variables, generate a sample point on the surface
    // of our shape and evaluate the light emitted along the ray from
    // our Shape's surface to the reference point.
    virtual Color3f Sample_Li(const Intersection &ref, const Point2f &u,
                              Vector3f *wi, Float *pdf, const Scene& scene) const;
    virtual Float Pdf_Li(const Intersection &, const Vector3f &) const;
};


