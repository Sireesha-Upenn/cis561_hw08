#include "naiveintegrator.h"

Color3f NaiveIntegrator::Li(const Ray &ray, const Scene &scene, std::shared_ptr<Sampler> sampler, int depth) const
{
    if(depth < 0)
    {
        return Color3f(0.f);
    }

    Intersection i = Intersection();
    Vector3f wo = (-ray.direction);     //Because it has to be in the opposite direction
    Vector3f wi;                        //to be set by sample_f
    float pdfToBeSet;                   //to be set by sample_f
    BxDFType sampledType;               //to be set by sample_f
    Color3f finalCol;                   //Computed color by Li

    Point2f samples = sampler->Get2D(); //Gives the stratified samples

    if(scene.Intersect(ray, &i))        //i will be the point
    {
        bool hasBSDF = i.ProduceBSDF(); //check if bsdf is false

        if (!hasBSDF)
        {
            return i.Le(wo);
        }
        Color3f sampledCol = i.bsdf->Sample_f(wo, &wi, samples, &pdfToBeSet, BSDF_ALL, &sampledType);
        Ray spawnedRay = i.SpawnRay(wi);

        //if(pdf is 0)
        if(pdfToBeSet == 0.f)
        {
            return Color3f(0.f);
        }

        //LTE
        finalCol = i.Le(wo) + ((sampledCol * Li(spawnedRay, scene, sampler, --depth) * AbsDot(i.normalGeometric, wi)) / pdfToBeSet);
    }
    else
    {
        return Color3f(0.f);
    }
    return finalCol;

}
