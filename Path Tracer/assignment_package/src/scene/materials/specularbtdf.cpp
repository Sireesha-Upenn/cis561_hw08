 #include "scene/materials/specularbtdf.h"

Color3f SpecularBTDF::f(const Vector3f &wo, const Vector3f &wi) const
{
    return Color3f(0.f);
}


float SpecularBTDF::Pdf(const Vector3f &wo, const Vector3f &wi) const
{
    return 0.f;
}

Color3f SpecularBTDF::Sample_f(const Vector3f &wo, Vector3f *wi, const Point2f &sample, Float *pdf, BxDFType *sampledType) const
{
    bool entering = CosTheta(wo) > 0; // check whether your ray is entering or leaving the object with which it has intersected;
       //you can do this by comparing the direction of ωo to the direction of your normal

       Float etaI = entering ? etaA : etaB;
       Float etaT = entering ? etaB : etaA;

       //generate ωi by refracting ωo, but you must also check for total internal reflection and return black if it would occur
       if (!Refract(wo, Faceforward(Normal3f(0, 0, 1), wo), etaI / etaT, wi))
       {
           *pdf = 1;
           return Color3f(0);
       }

       const Vector3f * wi_temp = wi;
       *pdf = 1;
       Color3f ft = T * (Color3f(1.f) - fresnel->Evaluate(CosTheta(*wi_temp))); //albedo * (1-fresnel) / absdot

       return ft / AbsCosTheta(*wi);
}
