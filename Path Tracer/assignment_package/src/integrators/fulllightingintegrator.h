#pragma once
#include "integrator.h"
#include "pcg32.h"
#include <glm/gtx/component_wise.hpp>
#include "globals.h"

class FullLightingIntegrator : public Integrator
{
public:
    FullLightingIntegrator(Bounds2i bounds, Scene* s, std::shared_ptr<Sampler> sampler, int recursionLimit)
        : Integrator(bounds, s, sampler, recursionLimit), randInt(rand.nextUInt()), randFloat(rand.nextFloat())
    {}

    pcg32 rand;
    int randInt;
    float randFloat;
    // Evaluate the energy transmitted along the ray back to
    // its origin using multiple importance sampling
    virtual Color3f Li(const Ray &ray, const Scene &scene, std::shared_ptr<Sampler> sampler, int depth) const;
    Color3f MIS(const Scene &scene, Intersection &isect, Vector3f wo,  std::shared_ptr<Sampler> sampler) const;
};

static inline float BalanceHeuristic(int nf, Float fPdf, int ng, Float gPdf);
static float PowerHeuristic(int nf, Float fPdf, int ng, Float gPdf);
static int getRandLightIdx(const Scene& scene, std::shared_ptr<Sampler> sampler);
