#pragma once
#include "camera.h"
#include "samplers/sampler.h"
#include "warpfunctions.h"
class ThinLensCamera : public Camera
{
public:
    ThinLensCamera()
        : Camera(), lensRadius(2.f), focalDistance(10.f), sampler(new Sampler(100, 0)) {}
    ThinLensCamera(Float radius, Float fd)
        : Camera(), lensRadius(radius), focalDistance(fd), sampler(new Sampler(100, 0)) {}
    ThinLensCamera(unsigned int w, unsigned int h, Float radius, Float fd)
        : Camera(w, h), lensRadius(radius), focalDistance(fd), sampler(new Sampler(100, 0)) {}
    ThinLensCamera(unsigned int w, unsigned int h,
                   const Vector3f &e, const Vector3f &r, const Vector3f &worldUp, Float radius, Float fd)
        : Camera(w, h, e, r, worldUp), lensRadius(radius), focalDistance(fd), sampler(new Sampler(100, 0)) {}
    ThinLensCamera(const Camera &c, Float radius, Float fd)
        : Camera(c), lensRadius(radius), focalDistance(fd), sampler(new Sampler(100, 0)) {
         this->RecomputeAttributes();
    }

    virtual ~ThinLensCamera()
    {}

    //Overloaded Member Functions
    virtual Ray Raycast(const Point2f &pt) const;         //Creates a ray in 3D space given a 2D point on the screen, in screen coordinates.
    virtual Ray Raycast(float x, float y) const;            //Same as above, but takes two floats rather than a vec2.
    virtual Ray RaycastNDC(float ndc_x, float ndc_y) const; //Creates a ray in 3D space given a 2D point in normalized device coordinates.

    //Extra Member Variables
    Float lensRadius, focalDistance;
    Sampler* sampler;

};


