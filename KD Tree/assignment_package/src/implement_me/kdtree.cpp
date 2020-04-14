#include "kdtree.h"

KDNode::KDNode()
    : leftChild(nullptr), rightChild(nullptr), axis(0), minCorner(), maxCorner(), particles()
{}

KDNode::~KDNode()
{
    delete leftChild;
    delete rightChild;
}

KDTree::KDTree()
    : root(nullptr)
{}

KDTree::~KDTree()
{
    delete root;
}

///Helper Functions
// Comparator functions you can use with std::sort to sort vec3s along the cardinal axes
bool xSort(glm::vec3* a, glm::vec3* b) { return a->x < b->x; }
bool ySort(glm::vec3* a, glm::vec3* b) { return a->y < b->y; }
bool zSort(glm::vec3* a, glm::vec3* b) { return a->z < b->z; }

bool sortAxis(int axis, glm::vec3* a, glm::vec3* b)
{
    switch(axis){
    case 0:
        return a->x < b->x;
    case 1:
        return a->y < b->y;
    case 2:
        return a->z < b->z;
    }
}

void setMax(float& point, float& maxCorner)
{
    if(point > maxCorner) { maxCorner = point; }
}

void setMin(float& point, float& minCorner)
{
    if(point < minCorner) { minCorner = point; }
}

void KDTree::build(const std::vector<glm::vec3*> &points)
{
    int depth = 1;
    if(points.size() > 0)
    {
        getMinMaxCorners(points, minCorner, maxCorner);
        this->root = buildRecursive(0, points.size() - 1, points, depth);
    }
}

void KDTree::getMinMaxCorners(const std::vector<glm::vec3*> &points, glm::vec3& minCorner, glm::vec3& maxCorner)
{
    //initialize
    minCorner = maxCorner = *points[0];

    //Get Min and max corners from points
    for(uint i = 0; i < points.size(); ++i) {
        //Set Max
        setMax(points[i]->x, maxCorner.x);
        setMax(points[i]->y, maxCorner.y);
        setMax(points[i]->z, maxCorner.z);
        //Set Min
        setMin(points[i]->x, minCorner.x);
        setMin(points[i]->y, minCorner.y);
        setMin(points[i]->z, minCorner.z);
    }
}

//Unused
void sortAboutAxis(uint axis, std::vector<glm::vec3 *>& sorted,
                   std::vector<glm::vec3 *>& left, std::vector<glm::vec3 *>& right)
{
    //Sort about axis
    if (axis == 0)      { std::sort(sorted.begin(), sorted.end(), xSort); }
    else if (axis == 1) { std::sort(sorted.begin(), sorted.end(), ySort); }
    else if (axis == 2) { std::sort(sorted.begin(), sorted.end(), zSort); }
    else { return; }

    //Find the median about the selected axis
    float median = 0.f;
    int size = sorted.size();
    if (size % 2 == 1) { median = (*sorted[size / 2])[axis]; }
    else {  median = ((*sorted[(size / 2) - 1])[axis] + (*sorted[size / 2])[axis]) / 2.f; }

    // Split
    for (auto it = sorted.begin(); it != sorted.end(); it++) {
        float temp = (*(*it))[axis];
        if(temp < median) {
            left.insert(it, *it);
        }
        else {
            right.insert(it, *it);
        }
    }
}

KDNode* KDTree::buildRecursive(int min, int max, const std::vector<glm::vec3 *> &points, int depth)
{
    std::vector<glm::vec3 *> sorted = points, left, right;
    float median = 0.f;
    int size = sorted.size();
    uint axis = depth % 3;
    KDNode* node = new KDNode();

    //Leaf node
    if(points.size() == 1) {
        node->leftChild = nullptr;
        node->rightChild = nullptr;
        node->minCorner = *points[0];
        node->maxCorner = *points[0];
        node->particles.push_back(points[0]);
        node->axis = axis;
        return node;
    }

    //Sort about axis
    if (axis == 0) {
        std::sort(sorted.begin(), sorted.end(), xSort);
        //Odd median
        if (size % 2 == 1) { median = sorted[size / 2]->x; }
        //Even median
        else {  median = ((sorted[(size / 2) - 1])->x + sorted[size / 2]->x) / 2.f; }
        //Divide into left and right children
        for(uint i = 0; i < sorted.size(); i++) {
            float val = sorted[i]->x;
            if(val < median) {
                left.push_back(sorted[i]);
            }
            if(val >= median) {
                right.push_back(sorted[i]);
            }
        }
    }
    else if (axis == 1) {
        std::sort(sorted.begin(), sorted.end(), ySort);
        //Odd median
        if (size % 2 == 1) { median = sorted[size / 2]->y; }
        //Even median
        else {  median = ((sorted[(size / 2) - 1])->y + sorted[size / 2]->y) / 2.f; }
        //Divide into left and right children
        for(uint i = 0; i < sorted.size(); i++) {
            float val = sorted[i]->y;
            if(val < median) {
                left.push_back(sorted[i]);
            }
            if(val >= median) {
                right.push_back(sorted[i]);
            }
        }
    }
    else if (axis == 2) {
        std::sort(sorted.begin(), sorted.end(), zSort);
        //Odd median
        if (size % 2 == 1) { median = sorted[size / 2]->z; }
        //Even median
        else {  median = ((sorted[(size / 2) - 1])->z + sorted[size / 2]->z) / 2.f; }
        //Divide into left and right children
        for(uint i = 0; i < sorted.size(); i++) {
            float val = sorted[i]->z;
            if(val < median) {
                left.push_back(sorted[i]);
            }
            if(val >= median) {
                right.push_back(sorted[i]);
            }
        }
    }

    //Assign values to the minCorner and maxCorner members
    getMinMaxCorners(points, node->minCorner, node->maxCorner);
    //tell each node which axis it was split on
    node->axis = axis;
    //Update left and right child
    if(!left.empty()) {
        node->leftChild = buildRecursive(min, max, left, depth++);
    }
    if(!right.empty()) {
        node->rightChild = buildRecursive(min, max, right, depth++);
    }

    return node;
}


std::vector<glm::vec3> KDTree::particlesInSphere(glm::vec3 c, float r)
{
    std::vector<glm::vec3> selected;
    KDNode* node;
    node = root;

    if (node->particles.size() > 0) {

        glm::vec3 pos = *(node->particles.at(0));
        float length = glm::dot(pos - c, pos - c);
        if (length < r * r) {
            selected.push_back(pos);
        }
        if (node->leftChild != nullptr) {
            particlesInSphereRecursive(selected, node->leftChild, c, r);
        }
        if (node->rightChild != nullptr) {
            particlesInSphereRecursive(selected, node->rightChild, c, r);
        }
    }
    return selected;
}
void KDTree::particlesInSphereRecursive(std::vector<glm::vec3> &selected, KDNode* node, glm::vec3 c, float r)
{
    if (node->particles.size() > 0) {

        glm::vec3 pos = *(node->particles.at(0));
        float length = glm::dot(pos - c, pos - c);
        if (length < r * r) {
            selected.push_back(pos);
        }
        if (node->leftChild != nullptr) {
            particlesInSphereRecursive(selected, node->leftChild, c, r);
        }
        if (node->rightChild != nullptr) {
            particlesInSphereRecursive(selected, node->rightChild, c, r);
        }
    }
}


void KDTree::clear()
{
    delete root;
    root = nullptr;
}
