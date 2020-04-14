#include "fulllightingintegrator.h"
#include "scene/lights/diffusearealight.h"
#include "scene/lights/pointlight.h"
#include "scene/lights/spotlight.h"

Color3f FullLightingIntegrator::Li(const Ray &r, const Scene &scene, std::shared_ptr<Sampler> sampler, int maxDepth) const
{
    Ray ray(r);
    Color3f accumulatedColor(0.f), throughput(1.f);
    int depth = 0;
    Vector3f wo = (-ray.direction), wi_g(0.f);
    BxDFType sampledType = BSDF_ALL;
    Float pdf_g(0.f);
    bool specularBounce = false;

    while (depth < maxDepth)
    {
        //1. Get intersection with scene, call it isect. Check all intersection cases.
        Intersection isect;
        bool foundIntersection = scene.Intersect(ray, &isect);
        wo = -ray.direction;
        //Terminate path if ray escaped (has no intersection) or maxDepth was reached
        if (!foundIntersection || depth >= maxDepth) { break; }
        //If depth is 0 or if prev bounce was specular
        if (depth == 0 || specularBounce) {
            //Add emitted light at path vertex or from the environment
            if (foundIntersection) {
                //Light (as this is the first bounce)
                if(!isect.ProduceBSDF()) {
                    accumulatedColor += isect.Le(wo);
                    return accumulatedColor;
                }
                accumulatedColor += throughput * isect.Le(-ray.direction);
            }
        }
        //If curr interesction doesn't have a bsdf - i.e, not an object, Break.
        if(!isect.ProduceBSDF())
        {
            break;
        }
        //All ok - Proceed to spawn ray and get MIS and stuff
        if (foundIntersection)
        {
            if (!isect.bsdf) {
                ray = isect.SpawnRay(ray.direction);
                ++depth;
                continue;
            }
            //Get Sampled color for wi_g
            Color3f sampledCol = isect.bsdf->Sample_f(wo, &wi_g, sampler->Get2D(), &pdf_g, BSDF_ALL, &sampledType);
            if (sampledCol == Color3f(0.f) || pdf_g == 0.f)
            { break; }
            //Check if that bounce was a specular material
            specularBounce = (sampledType & BSDF_SPECULAR) != 0;

            //2. Compute MIS for isect if bounce wasn't specular
            if(!specularBounce) {
                Color3f dirLight = MIS(scene, isect, wo, sampler);
                accumulatedColor += (dirLight * throughput); //Get GI Term from next iter
            }

            //3. Update throughput
            throughput *= (sampledCol * AbsDot(isect.normalGeometric, wi_g) / pdf_g);

            //4. Spawn a new ray in wi_g direction
            ray = isect.SpawnRay(wi_g);
            ++depth;

            //5. Apply Russian Roulette Ray Termination
            if (depth > 3) {
                float q = std::max(0.05f, 1.f - glm::compMax(throughput));
                if(sampler->Get1D() < q) { throughput /= (1.f - q); }
                else { break;}
            }
        }
        else {return Color3f(0.f);}
    }
    return accumulatedColor;
}


inline float BalanceHeuristic(int nf, Float fPdf, int ng, Float gPdf) { return (nf * fPdf) / (nf * fPdf + ng * gPdf); }

float PowerHeuristic(int nf, Float fPdf, int ng, Float gPdf)
{
    Float f = nf * fPdf, g = ng * gPdf;
    return (f * f) / (f * f + g * g);
}

int getRandLightIdx(const Scene& scene, std::shared_ptr<Sampler> sampler)
{
    int nLights = int(scene.lights.size());
    if (nLights == 0) return 0;
    int lightNum = std::min((int)(sampler->Get1D() * nLights), nLights - 1);
    return lightNum;
}

Color3f FullLightingIntegrator::MIS(const Scene& scene, Intersection& i_Main,
                                    Vector3f wo, std::shared_ptr<Sampler> sampler) const
{
    uint idx = getRandLightIdx(scene, sampler);
    std::shared_ptr<Light> randomLight = scene.lights[idx];
    Point2f samples = sampler->Get2D();        //Gives the stratified samples
    Float pdf_bb = 0.f, pdf_bl = 0.f, pdf_lb = 0.f, pdf_ll = 0.f; //to be set
    Vector3f wi_b,                              //ωi sampled by BSDF sampling
            wi_l;                               //ωi sampled by light source sampling
    BxDFType sampledType = BSDF_ALL;            //to be set by sample_f
    Color3f finalCol, finalCol_b, finalCol_l;   //Final color computed by MIS, Color computed by LTE_1 and LTE_2
    //If intersection is at a light
    Color3f light = i_Main.Le(wo);
    if(!IsBlack(light)) {
        finalCol += light;
        return finalCol;
    }
    ///LTE_1
    //LTE_1 will sample wi based on the BSDF and test for an intersection of this wi_b with any lights in the scene
    Color3f sampledCol(0.f);
    sampledCol = i_Main.bsdf->Sample_f(wo, &wi_b, samples, &pdf_bb, BSDF_ALL, &sampledType); //Sampling wi based on BSDF

    if (pdf_bb != 0.f && sampledCol!= Color3f(0.f)) {
        Intersection isect;
        Ray r_isect(i_Main.point + RayEpsilon * wi_b, wi_b);
        if (scene.Intersect(r_isect, &isect)) {
            bool notLight = isect.ProduceBSDF();
            bool sameLight = false;
            if(randomLight->name == isect.objectHit->name) { sameLight = true; }
            if (!notLight && sameLight) {
                finalCol_b = sampledCol * AbsDot(i_Main.normalGeometric, wi_b);
                finalCol_b *= randomLight->L(isect, -wi_b);
                pdf_lb = randomLight->Pdf_Li(i_Main, wi_b);
                pdf_lb /= scene.lights.size();
                finalCol_b *= PowerHeuristic(1, pdf_bb, 1 , pdf_lb);
                finalCol_b = (finalCol_b / pdf_bb);
            }
        }
    }

    ///LTE_2
    //LTE_2 will sample wi based on the Light source
    samples = sampler->Get2D();
    finalCol_l = randomLight->Sample_Li(i_Main, samples, &wi_l, &pdf_ll, scene);
    if (pdf_ll == 0.f) { return Color3f(0.f);}
    pdf_ll = pdf_ll / scene.lights.size();

    //If light is a point light / spot light
    Light* lightPtr = randomLight.get();
    PointLight* pLightPtr = dynamic_cast<PointLight*>(lightPtr);
    SpotLight* sLightPtr = dynamic_cast<SpotLight*>(lightPtr);

    if(pLightPtr || sLightPtr) {
        // Shadow Test from point light to the reference point:
        Ray shadowRay(lightPtr->getTransform()->position(), -wi_l);
        Intersection tempIsect;

        //Check for intersection with the scene
        if(!scene.Intersect(shadowRay, &tempIsect)) {
            finalCol_l += Color3f(0.f); //No intersection - Shadow point
        }
        else if(glm::length(tempIsect.point - i_Main.point) < ShadowEpsilon) {
            finalCol_l += glm::clamp(finalCol_l, 0.f, 1.f);
            finalCol_l *= i_Main.bsdf->f(wo, wi_l, sampledType);
            pdf_ll = pdf_ll / scene.lights.size();
            pdf_bl = i_Main.bsdf->Pdf(wo, wi_l, BSDF_ALL);
            if(pdf_bl == 0.f) {
                return Color3f(0.f);
            }
            finalCol += AbsDot(wi_l, i_Main.normalGeometric) * finalCol_l / pdf_ll;
        }
        return finalCol;
    }
    //Divide pdf by num_lights in the scene
    pdf_ll = pdf_ll / scene.lights.size();
    //Call bsdf-f to get the color of that poi about ωi_l
    finalCol_l *= i_Main.bsdf->f(wo, wi_l, sampledType);
    //Multiply by Absdot
    finalCol_l *= AbsDot(i_Main.normalGeometric, wi_l);
    //Set pdf_bl by calling BSDFpdf on ωi_l
    pdf_bl = i_Main.bsdf->Pdf(wo, wi_l, BSDF_ALL);
    //Multiply by weighting function
    finalCol_l *= PowerHeuristic(1, pdf_ll, 1, pdf_bl);
    //Finally, divide by pdf
    finalCol_l = (finalCol_l / pdf_ll);
    ///Adding both LTEs
    finalCol = i_Main.Le(wo) + (finalCol_b + finalCol_l);
    return finalCol;
}

