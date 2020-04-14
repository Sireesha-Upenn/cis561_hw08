#include "bsdf.h"
#include <warpfunctions.h>

BSDF::BSDF(const Intersection& isect, float eta /*= 1*/)
    : worldToTangent(Matrix3x3(glm::vec3(isect.tangent.x ,isect.bitangent.x, isect.normalGeometric.x),
                               glm::vec3(isect.tangent.y ,isect.bitangent.y, isect.normalGeometric.y),
                               glm::vec3(isect.tangent.z ,isect.bitangent.z, isect.normalGeometric.z))),
      tangentToWorld(Matrix3x3(isect.tangent, isect.bitangent, isect.normalGeometric)),
      normal(isect.normalGeometric),
      eta(eta),
      numBxDFs(0),
      bxdfs{nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr}
{}


void BSDF::UpdateTangentSpaceMatrices(const Normal3f& n, const Vector3f& t, const Vector3f b)
{
    tangentToWorld = Matrix3x3(n, t, b);
    worldToTangent = Matrix3x3(glm::vec3(t.x ,b.x, n.x),
                               glm::vec3(t.y ,b.y, n.y),
                               glm::vec3(t.z ,b.z, n.z));

}

// Compute the f() result of all of our BxDFs which match the BxDFType indicated
// Returns the sum of these f()s
// IMPORTANT: Convert woW and wiW from world space to tangent space
// before passing them to BxDF::f()!
Color3f BSDF::f(const Vector3f &woW, const Vector3f &wiW, BxDFType flags /*= BSDF_ALL*/) const
{
    //Converting to tangent space
    Vector3f wi = worldToTangent * wiW;
    Vector3f wo = worldToTangent * woW;
    bool reflect = glm::dot(wiW, normal) * glm::dot(woW, normal) > 0;
    Color3f f(0.f);

    if (wo.z == 0)
    {
        return f;
    }

    for (int i = 0; i < numBxDFs; ++i)
    {
        if (bxdfs[i]->MatchesFlags(flags) &&
                ((reflect && (bxdfs[i]->type & BSDF_REFLECTION)) ||
                 (!reflect && (bxdfs[i]->type & BSDF_TRANSMISSION))))
        {
            f += bxdfs[i]->f(wo, wi);
        }
    }
    return f;
}

// Invokes the Sample_f() of one of our BxDFs, chosen at random from the set
// of BxDFs that match the BxDFType indicated.
// Additionally invokes the f() of all BxDFs after using Sample_f to generate
// a ray direction.
// Refer to the .cpp file for a more detailed function description.

Color3f BSDF::Sample_f(const Vector3f &woW, Vector3f *wiW, const Point2f &xi, // point on square - pair of ksis
                       float *pdf, BxDFType type, BxDFType *sampledType) const
{
    Vector3f wo = worldToTangent * woW;
    Vector3f wi = worldToTangent * (*wiW);
    Color3f f(0.f);
    Color3f sumFs;
    Point2f ksis = xi;
    pcg32 rng;


    int matchingComps = NumComponents(type);
    if (matchingComps == 0)
    {
        *pdf = 0;
        return Color3f(0);
    }

    int comp = std::min((int)std::floor(xi[0] * matchingComps),
            matchingComps - 1);

    BxDF* randBxDF = nullptr;
    int count = comp;
    for (int i = 0; i < numBxDFs; ++i)
        if (bxdfs[i]->MatchesFlags(type) && count-- == 0) {
            randBxDF = bxdfs[i];
            break;
        }

    Point2f uRemapped(xi[0] * matchingComps - comp, xi[1]);

    if (sampledType)
    {
        *sampledType = randBxDF->type;
    }

    f = randBxDF->Sample_f(wo, &wi, uRemapped, pdf, sampledType);

    if (*pdf == 0)
    {
        return Color3f(0.f);
    }
    *wiW = tangentToWorld * (wi); //Setting wiW
    wi = tangentToWorld * (wi);
    wo = tangentToWorld * wo;

    if (!(randBxDF->type & BSDF_SPECULAR) && matchingComps > 1)
    {
        for (int i = 0; i < numBxDFs; ++i){
            if (bxdfs[i] != randBxDF && bxdfs[i]->MatchesFlags(type)){
                *pdf += bxdfs[i]->Pdf(wo, wi);
            }
        }
    }
    if (matchingComps > 1)
    {
        *pdf /= matchingComps; //Setting pdf
    }


    if (!(randBxDF->type & BSDF_SPECULAR) && matchingComps > 1)
    {
        bool reflect = glm::dot(*wiW, normal) * glm::dot(woW, normal) > 0;

        for (int i = 0; i < numBxDFs; ++i)
        {
            if (bxdfs[i]->MatchesFlags(type) &&
                    ((reflect && (bxdfs[i]->type & BSDF_REFLECTION)) ||
                     (!reflect && (bxdfs[i]->type & BSDF_TRANSMISSION))))
            {
                f += bxdfs[i]->f(wo, wi);
            }
        }
    }

    return f;
}


float BSDF::Pdf(const Vector3f &woW, const Vector3f &wiW, BxDFType flags) const
{
    float sum = 0.f;
    int matchCount = 0;
    for (int i = 0; i < numBxDFs; ++i)
    {
        if (bxdfs[i]->MatchesFlags(flags))
        {
            sum += bxdfs[i]->Pdf(woW, wiW);
            matchCount++;
        }
    }
    return sum/matchCount;
}

// Generates a uniformly random sample on the hemisphere
// and evaluates f() on it.
// Specific BxDF subclasses can (and should) override this.
Color3f BxDF::Sample_f(const Vector3f &wo, Vector3f *wi, const Point2f &xi,
                       Float *pdf, BxDFType *sampledType) const
{
    *wi = WarpFunctions::squareToHemisphereCosine(xi);
    if (wo.z < 0)
    {
        wi->z *= -1.f;
    }
    *pdf = Pdf(wo, *wi);
    return f(wo, *wi);
}

// The PDF for uniform hemisphere sampling
float BxDF::Pdf(const Vector3f &wo, const Vector3f &wi) const
{
    return SameHemisphere(wo, wi) ? Inv2Pi : 0;
}

BSDF::~BSDF()
{
    for(int i = 0; i < numBxDFs; i++)
    {
        delete bxdfs[i];
    }
}
int BSDF::NumComponents(BxDFType flags) const
{
    int count = 0;
    for (int i = 0; i < numBxDFs; ++i)
    {
        if (bxdfs[i]->MatchesFlags(flags))
        {
            count++;
        }
    }
}
