#pragma once

#include "light.h"

class SpotLight : public Light
{
public:
    SpotLight(const Transform &t,
              const Color3f &I,
              Float totalWidth, Float falloffStart)
        : Light(t),
          pLight(Point3f(t.T() * glm::vec4(0, 0, 0, 1.f))),
          I(I),
          cosTotalWidth(std::cos(Pi *totalWidth / 180.f)),
          cosFalloffStart(std::cos(Pi * falloffStart / 180.f))
    {}

    virtual Color3f L(const Intersection &isect, const Vector3f &w) const;
    virtual Color3f Sample_Li(const Intersection &ref, const Point2f &u, Vector3f *wi,
                      Float *pdf, const Scene& scene) const;
    Float Falloff(const Vector3f &w) const;
    Color3f Power() const;
    virtual Float Pdf_Li(const Intersection &, const Vector3f &) const;

private:
    const Point3f pLight;
    const Color3f I;
    const Float cosTotalWidth, cosFalloffStart;
};
