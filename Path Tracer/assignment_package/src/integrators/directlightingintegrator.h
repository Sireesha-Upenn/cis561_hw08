#pragma once
#include "integrator.h"

class DirectLightingIntegrator : public Integrator
{
public:
    DirectLightingIntegrator(Bounds2i bounds, Scene* s, std::shared_ptr<Sampler> sampler, int recursionLimit)
        : Integrator(bounds, s, sampler, recursionLimit) , randLight(rand.nextUInt())
    {}

    ///Member Variables
    pcg32 rand;
    int randLight;
    // Evaluate the energy transmitted along the ray back to
    // its origin, which can only be the camera in a direct lighting
    // integrator (only rays from light sources are considered)
    virtual Color3f Li(const Ray &ray, const Scene &scene, std::shared_ptr<Sampler> sampler, int depth) const;
};

static inline float BalanceHeuristic(int nf, Float fPdf, int ng, Float gPdf);
static float PowerHeuristic(int nf, Float fPdf, int ng, Float gPdf);
static int getRandLightIdx(const Scene& scene, std::shared_ptr<Sampler> sampler);
