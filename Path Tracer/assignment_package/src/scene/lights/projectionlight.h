#pragma once

#include "light.h"


class ProjectionLight : public Light
{
public:
    ProjectionLight(const Transform &t, const Color3f& Le, std::shared_ptr<QImage>pMap, const std::string &texname)
        : Light(t), projectionMap(pMap), I(Le)
    {
//        const float fov = 45.f;
//        Point2f resolution;
//        std::unique_ptr<Color3f> texels =
//        float aspect =projectionMap ?
//                            (Float(resolution.x) / Float(resolution.y)) : 1; ;
//        n = 1e-3f;
//        f = 1e30f;
//        lightProjection = glm::perspective(fov, aspect, n, f);
    }

    virtual Color3f L(const Intersection &isect, const Vector3f &w) const;
    virtual Color3f Sample_Li(const Intersection &ref, const Point2f &u, Vector3f *wi,
                              Float *pdf, const Scene& scene) const;
    Color3f Power() const;
    virtual Float Pdf_Li(const Intersection &, const Vector3f &) const;
    Color3f Sample_Le(const Point2f &u1, const Point2f &u2,
                      Float time, Ray *ray, Normal3f *nLight, Float *pdfPos, Float *pdfDir) const;
    void Pdf_Le(const Ray &, const Normal3f &, Float *pdfPos, Float *pdfDir) const;

    //Member variables
    std::shared_ptr<QImage> projectionMap;
    const Point3f pLight;
    const Color3f I;
    Transform lightProjection;
    Float n,f;
    Point2f screenBounds;
    Float cosTotalWidth;
};


