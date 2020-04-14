#include "specularbrdf.h"

Color3f SpecularBRDF::f(const Vector3f &wo, const Vector3f &wi) const
{
    return Color3f(0.f);
}


float SpecularBRDF::Pdf(const Vector3f &wo, const Vector3f &wi) const
{
    return 0.f;
}

Color3f SpecularBRDF::Sample_f(const Vector3f &wo, Vector3f *wi, const Point2f &sample, Float *pdf, BxDFType *sampledType) const
{
    //Since this BRDF is perfectly specular, Sample_f should generate ωi by reflecting ωo about the surface normal.
    //the surface normal is always (0, 0, 1) so we invert x and y
    *wi = Vector3f(-wo.x, -wo.y, wo.z);
    //Pdf of wi is 1 as it is the only unique wi that is possible for specular reflection
    *pdf = 1;

    return fresnel->Evaluate(CosTheta(*wi)) * R / AbsCosTheta(*wi);
}
