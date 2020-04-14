#include "shape.h"
#include <QDateTime>

pcg32 Shape::colorRNG = pcg32(QDateTime::currentMSecsSinceEpoch());

static float DistanceSquared(const Point3f &p1, const Point3f &p2)
{
    return  glm::length(p1 - p2) *  glm::length(p1 - p2);
}

float Shape::Pdf(const Intersection &ref, const Vector3f &wi)
{
    Ray ray = ref.SpawnRay(wi);
    Intersection isectLight;
    if (!Intersect(ray, &isectLight)) //If new ray doesn't have an intersection with the shape - return 0
    { return 0; }

    //Solid angle pdf
    Float pdf = DistanceSquared(ref.point, isectLight.point) / (AbsDot(isectLight.normalGeometric, -wi) * Area());
    return pdf;
}

void Shape::InitializeIntersection(Intersection *isect, float t, Point3f pLocal) const
{
    isect->point = Point3f(transform.T() * glm::vec4(pLocal, 1));
    ComputeTBN(pLocal, &(isect->normalGeometric), &(isect->tangent), &(isect->bitangent));
    isect->uv = GetUVCoordinates(pLocal);
    isect->t = t;
}

//This function must invoke the two-input Sample, to be implemented by subclasses,
//and generate a ωi from the resulting Intersection.
//From there, it must convert the PDF obtained from the other Sample function from a PDF with respect to surface area to a PDF
//with respect to solid angle at the reference point of intersection.
//As always, be careful of dividing by zero! Should this occur, you should set your PDF to zero.
Intersection Shape::Sample(const Intersection &ref, const Point2f &xi, float *pdf) const
{
    Intersection i = Sample(xi, pdf);

    //Convert pdf wrt solid angle
    const Vector3f wi = -(glm::normalize(glm::vec3(i.point - ref.point)));

    if ((AbsDot(i.normalGeometric, wi) * Area()) == 0.f)
    {
        *pdf = 0.f;
    }
    else
    {
        *pdf *= (glm::length(ref.point - i.point) * glm::length(ref.point - i.point)) / (AbsDot(i.normalGeometric, wi));
    }

    return i;
}
bool Shape::bothIntersects(const Ray &ray, CSGIntersection& csg_isect) const
{
    //bothIntersections should not be called on shape directly
   return false;
}
