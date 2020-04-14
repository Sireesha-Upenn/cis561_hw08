#pragma once
#include<scene/geometry/primitive.h>
#include <scene/geometry/shape.h>

class Shape;

//CSGIntersection is declared in Intersection class

//Represents what type of a node the current node is
enum NodeType {
    U, N, D, GEOM
};

//A structure that holds both the intersections and t values
//of the current node
struct Segment {
    Segment() {}
    Segment(Intersection* f, Intersection* s, float t1, float t2)
        :first(f), second(s), first_t(t1), second_t(t2) {}
    //Each segment has 2 intersections and 2 ts
    Intersection* first;
    Intersection* second;
    float first_t, second_t;
};

class CSGNode : public Shape
{
public:
    CSGNode() :  geom(nullptr), left(nullptr), right(nullptr) {}

    ///Member Variables

    NodeType type;
    Shape* geom;
    CSGNode* left;
    CSGNode* right;

    ///Member Functions

    virtual bool Intersect(const Ray &ray, Intersection *isect) const;

    virtual Point2f GetUVCoordinates(const Point3f &point) const;

    // A helper function for computing the world-space normal, tangent, and bitangent at local-space point P
    virtual void ComputeTBN(const Point3f& P, Normal3f* nor, Vector3f* tan, Vector3f* bit) const;

    // Compute and return the surface area of the shape in world space
    // i.e. you must account for the Transform applied to the Shape
    virtual float Area() const;

    // Sample a point on the surface of the shape and return the PDF with
    // respect to area on the surface.
    virtual Intersection Sample(const Point2f &xi, Float *pdf) const;

    //Helper functions
    CSGNode& addLeftChild(std::unique_ptr<CSGNode> n);
    CSGNode& addRightChild(std::unique_ptr<CSGNode> n);

    //Pre-order CSG-tree traversal to check for ray intersection
    void rayCSGIntersect(const Ray &ray, std::vector<Segment>& segments);
    void CombineSegments(std::vector<Segment>& LL, std::vector<Segment>& RL,
                         std::vector<Segment>& segments, NodeType nodetype) const;
    void InputSegment(std::vector<Segment>& resSegList, Segment& s, NodeType Opt) const;

    void create();

    //Operators
    void applyUnion(std::vector<Segment>&, std::vector<Segment>& LL, std::vector<Segment>& RL) const;
    void applyIntersection(std::vector<Segment>&, std::vector<Segment>& LL, std::vector<Segment>& RL) const;
    void applyDifference(std::vector<Segment>&, std::vector<Segment>& LL, std::vector<Segment>& RL) const;

    Segment clubUnion(Segment& s1, Segment& s2) const;
    Segment clubIntersection(Segment& s1, Segment& s2) const;
    Segment clubDifference(Segment& s1, Segment& s2) const;
};

//Unused??
class CSGTree
{
public:

    CSGTree();
    CSGTree(CSGNode* r) : root(r) {}
    ~CSGTree();

    ///Member variables
    CSGNode* root;
    ///Member functions
};
