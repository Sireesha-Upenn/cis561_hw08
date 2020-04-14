#include "projectionlight.h"

Color3f ProjectionLight::L(const Intersection &isect, const Vector3f &w) const
{

}
Color3f ProjectionLight::Sample_Li(const Intersection &ref, const Point2f &u, Vector3f *wi,
                          Float *pdf, const Scene& scene) const
{

}
Color3f ProjectionLight::Power() const
{

}
Float ProjectionLight::Pdf_Li(const Intersection &, const Vector3f &) const
{

}
Color3f ProjectionLight::Sample_Le(const Point2f &u1, const Point2f &u2,
                  Float time, Ray *ray, Normal3f *nLight, Float *pdfPos, Float *pdfDir) const
{

}
void ProjectionLight::Pdf_Le(const Ray &, const Normal3f &, Float *pdfPos, Float *pdfDir) const
{

}
