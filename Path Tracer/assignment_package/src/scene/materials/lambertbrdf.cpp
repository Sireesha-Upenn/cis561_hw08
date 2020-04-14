#include "lambertbrdf.h"
#include <warpfunctions.h>

Color3f LambertBRDF::f(const Vector3f &wo, const Vector3f &wi) const
{
    return R * InvPi;
}

Color3f LambertBRDF::Sample_f(const Vector3f &wo, Vector3f *wi, const Point2f &u,
                        Float *pdf, BxDFType *sampledType) const
{
    *wi = WarpFunctions::squareToHemisphereCosine(u);
    *pdf = Pdf(wo, *wi);
    if (wo.z < 0)
    {
        wi->z *= -1.f;
    }
    return f(wo, *wi);
}

float LambertBRDF::Pdf(const Vector3f &wo, const Vector3f &wi) const
{
    return CosTheta(wi) * InvPi;
}

 
