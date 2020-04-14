#include "thinlenscamera.h"

Ray ThinLensCamera::Raycast(const Point2f &pt) const
{
    return Raycast(pt.x, pt.y);
}

Ray ThinLensCamera::Raycast(float x, float y) const
{
    float ndc_x = (2.f*x/width - 1);
    float ndc_y = (1 - 2.f*y/height);
    Ray r = this->RaycastNDC(ndc_x, ndc_y);
    return r;
}

Ray ThinLensCamera::RaycastNDC(float ndc_x, float ndc_y) const
{
    Point3f pof = eye + (look * focalDistance);
    glm::vec3 pFocal = pof + ndc_x * H + ndc_y * V;

    Point3f point = WarpFunctions::squareToDiskUniform(sampler->Get2D()) * lensRadius;

    point = eye + (up * point.y) + (right * point.x);
    Ray result(point, glm::normalize(pFocal - point));

    return result;
}
