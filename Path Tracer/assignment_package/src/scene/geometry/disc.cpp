#include "disc.h"

float Disc::Area() const
{
    //Use scale of shape about x and y to get the radius //radius is assumed to be 1
    Point3f transformedRadius = Point3f(1.f) * transform.getScale().x * transform.getScale().y;
    return Pi * glm::length(transformedRadius) * glm::length(transformedRadius);
}

bool Disc::Intersect(const Ray &ray, Intersection *isect) const
{
    //Transform the ray
    Ray r_loc = ray.GetTransformedCopy(transform.invT());

    //Ray-plane intersection
    float t = glm::dot(glm::vec3(0,0,1), (glm::vec3(0.5f, 0.5f, 0) - r_loc.origin)) / glm::dot(glm::vec3(0,0,1), r_loc.direction);
    Point3f P = Point3f(t * r_loc.direction + r_loc.origin);
    //Check that P is within the bounds of the disc (not bothering to take the sqrt of the dist b/c we know the radius)
    float dist2 = (P.x * P.x + P.y * P.y);
    if(t > 0 && dist2 <= 1.f)
    {
        InitializeIntersection(isect, t, P);
        return true;
    }
    return false;
}

void Disc::ComputeTBN(const Point3f &P, Normal3f *nor, Vector3f *tan, Vector3f *bit) const
{
    *nor = glm::normalize(transform.invTransT() * Normal3f(0,0,1));
    CoordinateSystem(*nor, tan, bit);
}


Point2f Disc::GetUVCoordinates(const Point3f &point) const
{
    return glm::vec2((point.x + 1)/2.f, (point.y + 1)/2.f);
}

Intersection Disc::Sample(const Point2f &xi, Float *pdf) const
{
    //Generate a world-space point on the surface of the shape.
    Point3f pd = WarpFunctions::squareToDiskConcentric(xi) / 2.f;

    //Create an Intersection to return.
    Intersection res = Intersection();

    //Set the point and normal of this Intersection to the correct values.
    //use shape->transform to get the transformed radius // mainly scale
    Point3f pObj(pd.x , pd.y , pd.z);
    pObj = glm::vec3(transform.T() * glm::vec4(pObj, 1.f));

    //Setting normal and poi in world space
    res.point = pObj;
    res.normalGeometric = glm::normalize(transform.invTransT() * Vector3f(0.f, 0.f, 1.f));


    //Set the output PDF to the correct value, which would be a uniform PDF with respect to surface area.
    *pdf = 1.f / Area();
    return res;
}

bool Disc::bothIntersects(const Ray &ray, CSGIntersection& csg_isect) const
{
    Intersection* isect = csg_isect.isect;
    //Transform the ray
    Ray r_loc = ray.GetTransformedCopy(transform.invT());

    //Ray-plane intersection
    float t = glm::dot(glm::vec3(0,0,1), (glm::vec3(0.5f, 0.5f, 0) - r_loc.origin)) / glm::dot(glm::vec3(0,0,1), r_loc.direction);
    Point3f P = Point3f(t * r_loc.direction + r_loc.origin);
    //Check that P is within the bounds of the disc (not bothering to take the sqrt of the dist b/c we know the radius)
    float dist2 = (P.x * P.x + P.y * P.y);
    if(t > 0 && dist2 <= 1.f)
    {
        InitializeIntersection(isect, t, P);
        return true;
    }
    csg_isect.t1 = t;
    csg_isect.t2 = t; //since it is 2D
    return false;
}
