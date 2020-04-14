#define _USE_MATH_DEFINES
#include "warpfunctions.h"
#include <math.h>

Point3f WarpFunctions::squareToDiskUniform(const Point2f &sample)
{
    float r = std::sqrt(sample.x);
    float theta = 2 * M_PI * sample.y;
    return glm::vec3(r * std::cos(theta), r * std::sin(theta), 0.f);
}

Point3f WarpFunctions::squareToDiskConcentric(const Point2f &sample)
{
    glm::vec2 uOffset = 2.f * sample - glm::vec2(1.f, 1.f);
    if (uOffset.x == 0.f && uOffset.y == 0.f)
    {
        return glm::vec3(0.f, 0.f, 0.f);
    }
    float theta, r;
    if (std::abs(uOffset.x) > std::abs(uOffset.y))
    {
        r = uOffset.x;
        theta = M_PI_4 * (uOffset.y / uOffset.x);
    }
    else
    {
        r = uOffset.y;
        theta = M_PI_2 - M_PI_4 * (uOffset.x / uOffset.y);
    }
    return r * glm::vec3(std::cos(theta), std::sin(theta), 0.f);
}

float WarpFunctions::squareToDiskPDF(const Point3f &sample)
{
    return InvPi;
}

Point3f WarpFunctions::squareToSphereUniform(const Point2f &sample)
{
    float z = 1 - 2 * sample.x;
    float r = std::sqrt(std::max(float(0.f), float(1.f) - z * z));
    float phi = 2 * M_PI * sample.y;
    return glm::vec3(r * std::cos(phi), r * std::sin(phi), z);
}

float WarpFunctions::squareToSphereUniformPDF(const Point3f &sample)
{
    return Inv4Pi;
}

Point3f WarpFunctions::squareToSphereCapUniform(const Point2f &sample, float thetaMin)
{
    float h = 1.f - sin(glm::radians(90.f - (180.f - thetaMin)));
    float z = 1 - 2 * sample.x *h;
    float r = std::sqrt(std::max(float(0.f), float(1.f) - z * z));
    float phi = 2 * M_PI * sample.y;
    return glm::vec3(r * std::cos(phi), r * std::sin(phi), z);
}

float WarpFunctions::squareToSphereCapUniformPDF(const Point3f &sample, float thetaMin)
{
    float h = 1.f - sin(glm::radians(90.f - (180.f - thetaMin)));
    return (1.f / (2.f * M_PI * h));
}

Point3f WarpFunctions::squareToHemisphereUniform(const Point2f &sample)
{
    float theta = acos(sample.x);
    float phi = 2.f * M_PI * sample.y;
    float z = cos(theta);
    return glm::vec3( sin(theta) * cos(phi), sin(theta) * sin(phi), z);
}

float WarpFunctions::squareToHemisphereUniformPDF(const Point3f &sample)
{
    return Inv2Pi;
}

Point3f WarpFunctions::squareToHemisphereCosine(const Point2f &sample)
{
    glm::vec3 d = squareToDiskConcentric(sample);
    float z = std::sqrt(std::max(float(0), 1 - d.x * d.x - d.y * d.y));
    return glm::vec3(d.x, d.y, z);
}

float WarpFunctions::squareToHemisphereCosinePDF(const Point3f &sample)
{
   return sample.z * InvPi;
}
