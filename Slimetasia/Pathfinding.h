#pragma once
#include <queue>

#include "AISystem.h"
#include "IComponent.h"
class Pathfinding : public IComponent
{
    bool CheckValidity(int row, int col);
    bool DestinationReach(int row, int col, const Vector3& dest);
    void ConvertToIndex(int& row, int& col, Vector3 pos);
    float ComputeHValue(int row, int col, Vector3 pos);
    void UpdateNeighbour(int row, int col, float gval, const Vector3& end, bool& found, int i, int j, std::priority_queue<NodePath, std::vector<NodePath>, CustomCompare>& m_openlist);
    void StoreMovement(NodePath* n);
    Vector3 startPos;
    // When initialize the first position
    Vector3 initialPos;
    NodePath* destNode;
    NodePath** localgrid;
    Vector3 m_dir;

    bool isPathSplit = true;
    bool isBaseGrid = false;

public:

    Pathfinding(GameObject* parentObject, const std::string& name = "Pathfinding");
    ~Pathfinding() = default;
    bool AStarFindPath(const Vector3& end, bool splitpath);
    void OnUpdate(float dt);
    void ChangeLocalToBaseGrid();
    std::vector<Vector3> m_paths;
    REFLECT();
};