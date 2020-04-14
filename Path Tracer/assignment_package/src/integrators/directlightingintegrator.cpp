#include "directlightingintegrator.h"
#include "scene/lights/diffusearealight.h"
#include "scene/lights/pointlight.h"
#include "scene/lights/spotlight.h"

Color3f DirectLightingIntegrator::Li(const Ray &ray, const Scene &scene, std::shared_ptr<Sampler> sampler, int depth) const
{
    if(depth < 0)
    {
        return Color3f(0.f);
    }

    //Random Light
    uint idx = getRandLightIdx(scene, sampler);
    std::shared_ptr<Light> randomLight = scene.lights[idx];

    Intersection i_Main = Intersection();      //Will contain the main intersection in the scene
    Vector3f wo = (-ray.direction);            //Because it has to be in the opposite direction
    Vector3f wi_b;                             //ωi sampled by BSDF sampling
    Vector3f wi_l;                             //ωi sampled by light source sampling
    Float pdf_bb = 0.f;                        //pdfs to be set
    Float pdf_bl = 0.f;
    Float pdf_lb = 0.f;
    Float pdf_ll = 0.f;
    BxDFType sampledType = BSDF_ALL;           //to be set by sample_f
    Color3f finalCol;                          //Final color computed by MIS
    Color3f finalCol_b;                        //Color computed by LTE_1
    Color3f finalCol_l;                        //Color computed by LTE_2
    Point2f samples = sampler->Get2D();        //Gives the stratified samples

    if (scene.Intersect(ray, &i_Main))          //i->point will be the main point
    {
        bool hasBSDF = i_Main.ProduceBSDF();   //check if bsdf is false

        if (!hasBSDF)                          //Not material? Light!
        {
            return i_Main.Le(wo);              //If initial intersection is on the light, return light emitted
        }

        ///LTE_1
        //LTE_1 will sample wi based on the BSDF and test for an intersection of this wi_b with any lights in the scene

        //Sample ωi based on BSDF - Call sample_f - this will set ωi_b, pdf_bb and sampledType and will return color of material
        Color3f sampledCol(0.f);
        sampledCol = i_Main.bsdf->Sample_f(wo, &wi_b, samples, &pdf_bb, BSDF_ALL, &sampledType); //Sampling wi based on BSDF

        if (pdf_bb != 0.f && sampledCol!= Color3f(0.f))
        {
            //Create a new ray from the main poi in about the direction of ωi_b and test for intersection of this new ray in the scene
            //This new intersection will be stored in isect
            Intersection isect;
            Ray r_isect(i_Main.point + RayEpsilon * wi_b, wi_b);

            if (scene.Intersect(r_isect, &isect))
            {
                //Check if intersection is a light source and if so, it is the randomly selected light
                bool notLight = isect.ProduceBSDF(); //returns true if the intersection is a material so we check for false
                bool sameLight = false;
                if(randomLight->name == isect.objectHit->name) { sameLight = true; }

                //If intersection is light and its the same randomly selected //this is fine don't mess with it
                if (!notLight && sameLight)
                {
                    //Multiply by absdot
                    finalCol_b = sampledCol * AbsDot(i_Main.normalGeometric, wi_b);

                    //L will return the light emitted along ωi_b
                    finalCol_b *= randomLight->L(isect, -wi_b);

                    //Set pdf_lb by calling Pdf_Li on ωi_b - Divide by num_lights in the scene
                    pdf_lb = randomLight->Pdf_Li(i_Main, wi_b);

                    pdf_lb /= scene.lights.size();
                    //Multiply the final color by the weighting function

                    finalCol_b *= PowerHeuristic(1, pdf_bb, 1 , pdf_lb);

                    //Divide by pdf
                    finalCol_b = (finalCol_b / pdf_bb);
                }
            }
        }

        ///LTE_2

        samples = sampler->Get2D();
        //Sample ωi_l based on some randomly selected light in the scene
        //This will also set pdf_ll and returns the color of that light
        finalCol_l = randomLight->Sample_Li(i_Main, samples, &wi_l, &pdf_ll, scene);

        //If pdf is zero, return black
        if (pdf_ll == 0.f)
        {
            return Color3f(0.f);
        }

        //If light is a point light / spot light
        Light* lightPtr = randomLight.get();
        PointLight* pLightPtr = dynamic_cast<PointLight*>(lightPtr);
        SpotLight* sLightPtr = dynamic_cast<SpotLight*>(lightPtr);

        if(pLightPtr || sLightPtr)
        {
            // Shadow Test from point light to the reference point:
            Ray shadowRay(lightPtr->getTransform()->position(), -wi_l);
            Intersection shadowIsect;

            //Check for intersection with the scene
            if(!scene.Intersect(shadowRay, &shadowIsect))
            {
                finalCol_l += Color3f(0.f); //No intersection - Shadow point
            }
            else if(glm::length(shadowIsect.point - i_Main.point) < 0.01)
            {
                //Clamp values of Li (to get rid of any noise)
                finalCol_l += glm::clamp(finalCol_l, 0.f, 1.f);

                //Divide pdf by num_lights in the scene
                pdf_ll = pdf_ll / scene.lights.size();

                //Get material color at the point of intersection
                finalCol_l *= i_Main.bsdf->f(wo, wi_l, sampledType);

                pdf_bl = i_Main.bsdf->Pdf(wo, wi_l, BSDF_ALL);

                if(pdf_bl == 0.f)
                {
                    finalCol = Color3f(0.f);
                    return finalCol;
                }

                //Compute final color value
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
    }
    else
    {
        return Color3f(0.f);
    }

    ///Adding both LTEs

    finalCol = i_Main.Le(wo) + (finalCol_b + finalCol_l); //* float(scene.lights.size());
    return finalCol;

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

