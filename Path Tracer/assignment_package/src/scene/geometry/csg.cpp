#include "csg.h"

Intersection CSGNode::Sample(const Point2f &xi, Float *pdf) const
{
    return Intersection();
}

void CSGNode::ComputeTBN(const Point3f &P, Normal3f *nor, Vector3f *tan, Vector3f *bit) const
{
    *nor = glm::normalize(transform.invTransT() * Normal3f(0,0,1));
    CoordinateSystem(*nor, tan, bit);
}

Point2f CSGNode::GetUVCoordinates(const Point3f &point) const
{
    return Point2f(point.x + 0.5f, point.y + 0.5f);
}

float CSGNode::Area() const
{
    //Recursively calls area on the children
    float areaLeft, areaRight;
    if(this->left->geom) {
        areaLeft = this->left->geom->Area();
    } else {
        areaLeft = this->left->Area();
    }
    if(this->right->geom) {
        areaRight = this->right->geom->Area();
    } else {
        areaRight = this->right->Area();
    }
    return areaLeft + areaRight;
}

void CSGNode::create()
{
    //Recursively calls create on the children
        if(this->left->geom) {
            this->left->geom->create();
        } else {
            this->left->create();
        }
        if(this->right->geom) {
            this->right->geom->create();
        } else {
            this->right->create();
        }
}

CSGNode& CSGNode::addLeftChild(std::unique_ptr<CSGNode> n)
{
    CSGNode& ref = *n;
    this->left = n.get();
    return ref;
}

CSGNode& CSGNode::addRightChild(std::unique_ptr<CSGNode> n)
{
    CSGNode& ref = *n;
    this->right = n.get();
    return ref;
}

void CSGNode::rayCSGIntersect(const Ray &ray, std::vector<Segment>& segments)
{
    std::cout<<"Called rayCSGIntersect"<<std::endl;
    if (type == GEOM){
        //Hit a leaf node with geometry - Call bothIntersects on it.
        CSGIntersection csg_isect;
        //Call double intersection here
        if(geom->bothIntersects(ray, csg_isect)) {
            if(csg_isect.t1 > 0.f && csg_isect.t2 > 0.f) {
                Segment segNew(csg_isect.isect, csg_isect.isect, csg_isect.t1, csg_isect.t2);
                segments.push_back(segNew);
            }
        }
    }
    else {
        //Traverse tree and call rayCSGIntersect on them
        std::vector<Segment> LL, RL, combined;
        left->rayCSGIntersect(ray, LL);
        if(LL.empty() && type!= U) {
            return;
        }
        else {
            right->rayCSGIntersect(ray, RL);
            this->CombineSegments(LL, RL, combined, type);
            return;
        }
    }
}

bool CSGNode::Intersect(const Ray &ray, Intersection *isect) const
{
    std::vector<Segment> LL, RL, combined;
    //Algorithm from Norm's slides
    left->rayCSGIntersect(ray, LL);
    if(LL.empty() && type!= U) {
        return false;
    }
    else {
        right->rayCSGIntersect(ray, RL);
        this->CombineSegments(LL, RL, combined, type);
    }
    //Set intersection
    if(!combined.empty()) {
        isect = combined.begin()->first;
        return true;
    }
    return false;
}

void CSGNode::CombineSegments(std::vector<Segment>& LL, std::vector<Segment>& RL,
                              std::vector<Segment>& segments, NodeType nodetype) const
{
    if (!LL.empty() && !RL.empty())
    {
        switch (nodetype) {
        case U: {
            applyUnion(segments, LL, RL);
            break;
        }
        case N: {
            applyIntersection(segments, LL, RL);
            break;
        }
        case D: {
            applyDifference(segments, LL, RL);
            break;
        }
        case GEOM: {
            //Shouldn't call combine if type is geom as it is a leaf node
            return;
        }
        }
    }
    else if(LL.empty()) { segments = RL; }
    else if(RL.empty()) { segments = LL; }
}

void CSGNode::applyIntersection(std::vector<Segment>& combined, std::vector<Segment>& LL, std::vector<Segment>& RL) const
{
    for(auto segL : LL) {
        for(auto segR : RL) {
            if((segL.second_t > segR.first_t) && (segL.first_t < segR.second_t))  {
                Segment seg = clubIntersection(segL, segR);
                combined.push_back(seg);
            }
        }
    }
}

Segment CSGNode::clubIntersection(Segment& s1, Segment& s2) const
{
    Segment seg;
    // Get largest intersection:
    if(s1.second_t > s2.second_t) {
        seg.second_t = s2.second_t;
        seg.second = s2.second;
    }
    else {
        seg.second_t = s1.second_t;
        seg.second = s1.second;
    }
    // Get smallest intersection:
    if(s1.first_t < s2.first_t) {
        seg.first_t = s2.first_t;
        seg.first = s2.first;
    }
    else {
        seg.first_t = s1.first_t;
        seg.first = s1.first;
    }
    return seg;
}

void CSGNode::applyDifference(std::vector<Segment>& combined, std::vector<Segment>& LL, std::vector<Segment>& RL) const
{
    for (auto segL : LL) {
        for (auto segR : RL) {
            if (!(segL.first_t == segL.second_t) && !(segR.first_t > segL.second_t) &&
                    (segL.second_t > segR.first_t) && (segL.first_t < segR.second_t))
            {
                if ((segL.first_t < segR.first_t) && (segL.second_t > segR.second_t))
                {
                    Segment rightSeg(segR.second, segL.second, segR.second_t, segL.second_t);
                    LL.push_back(rightSeg);
                    segL.second = segR.second;
                    segL.second_t = segR.second_t;
                }
                else
                {
                    segL = clubDifference(segL, segR);
                }
            }
        }
        if(segL.first_t != segL.second_t) { combined.push_back(segL); }
    }
}

Segment CSGNode::clubDifference(Segment& s1, Segment& s2) const
{
    Segment seg;
    // Cut s2 from s1:
    if(s1.first_t == s1.second_t)  {
        seg = s1;
        return seg;
    }

    // s1 still has residual and s1, s2 are continuous:
    else if((s1.first_t > s2.first_t) && (s1.second_t < s2.second_t)) {
        // s2 totally includes s1:
        seg.first_t = s1.first_t;
        seg.second_t = s1.first_t;
        return seg;
    }
    else if((s2.second_t > s1.first_t) && (s2.second_t < s1.second_t)) {
        // s2 intersect s1 from s1's left:
        seg.first_t = s2.second_t;
        seg.first = s2.second;
        return seg;
    }
    else if((s2.first_t > s1.first_t) && (s2.first_t < s1.second_t)) {
        // s2 intersect s1 from s1's right:
        seg.first_t = s1.first_t;
        seg.first = s1.first;
        seg.second_t = s2.first_t;
        seg.second = s2.first;
        return seg;
    }
}

void CSGNode::InputSegment(std::vector<Segment>& segments, Segment& s2, NodeType Opt) const
{
    Segment s1 = segments.back();
    if(segments.size() == 0) {
        segments.push_back(s2);
    }
    else {
        if((s1.second_t > s2.first_t) && (s1.first_t < s2.second_t))  {
            segments.push_back(s2);
        }
        else {
            Segment mergedSeg = clubUnion(s1, s2);
            segments.pop_back();
            segments.push_back(mergedSeg);
        }
    }
}

void CSGNode::applyUnion(std::vector<Segment>& combined, std::vector<Segment>& LL, std::vector<Segment>& RL) const
{
    auto itrL = LL.begin();
    auto itrR = RL.begin();

    while (itrL != LL.end() && itrR != RL.end()) {
        if (itrL == LL.end()) {
            for (;itrR != RL.end();itrR++) {
                InputSegment(combined, *itrR, U);
            }
        }
        if (itrR == RL.end()) {
            for (;itrL != LL.end();itrL++) {
                InputSegment(combined, *itrL, U);
            }
        }
        if (itrL->first_t < itrR->first_t) {
            InputSegment(combined, *itrL, U);
            itrL++;
        }
        else {
            InputSegment(combined, *itrR, U);
            itrR++;
        }
    }
}

Segment CSGNode::clubUnion(Segment& s1, Segment& s2) const
{
    Segment seg;
    // Get largest intersection:
    if(s1.second_t > s2.second_t) {
        seg.second_t = s1.second_t;
        seg.second = s1.second;
    }
    else {
        seg.second_t = s2.second_t;
        seg.second = s2.second;
    }
    // Get smallest intersection:
    if(s1.first_t < s2.first_t) {
        seg.first_t = s1.first_t;
        seg.first = s1.first;
    }
    else {
        seg.first_t = s2.first_t;
        seg.first = s2.first;
    }
    return seg;
}

