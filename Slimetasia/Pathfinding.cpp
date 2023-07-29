#include "Pathfinding.h"

#include "AISystem.h"
#include "Application.h"
#include "GameObject.h"
#include "Layer.h"
#include "Renderer.h"
#include "RigidbodyComponent.h"
#include "Scene.h"

const float maxSpeed = 1.f;
const float slimeSpeed = 1.5f;
bool Pathfinding::CheckValidity(int row, int col)
{
    return (localgrid[row][col].valid == 1);
}
bool Pathfinding::DestinationReach(int row, int col, const Vector3& dest)
{
    int destCol, destRow;
    ConvertToIndex(destRow, destCol, dest);
    return (row == destRow && col == destCol);
}
void Pathfinding::ConvertToIndex(int& row, int& col, Vector3 pos)
{
    col = int((pos[0] - AISystem::Instance().minPos[0]) / 2);
    row = int((pos[2] - AISystem::Instance().minPos[1]) / 2);
}
float Pathfinding::ComputeHValue(int row, int col, Vector3 pos)
{
    return localgrid[row][col].pos.Distance(pos);
}

void Pathfinding::StoreMovement(NodePath* n)
{
    if (isPathSplit)
    {
        if (m_paths.size() > 0) m_paths.clear();
        // condition n->parent instead of n because i don't require the start path!
        while (n->parent != nullptr)
        {
            m_paths.emplace_back(n->pos);
            n = n->parent;
        }
        std::reverse(m_paths.begin(), m_paths.end());
    }
    else
    {
        if (AISystem::Instance().slimePaths.size() > 0) AISystem::Instance().slimePaths.clear();
        // condition n->parent instead of n because i don't require the start path!
        while (n->parent != nullptr)
        {
            AISystem::Instance().slimePaths.emplace_back(n->pos);
            n = n->parent;
        }
        std::reverse(AISystem::Instance().slimePaths.begin(), AISystem::Instance().slimePaths.end());
        AISystem::Instance().originalPathsArr.clear();
        AISystem::Instance().originalPathsArr.emplace_back(AISystem::Instance().slimePaths);
    }
}
void Pathfinding::UpdateNeighbour(int row, int col, float gval, const Vector3& end, bool& found, int i, int j, std::priority_queue<NodePath, std::vector<NodePath>, CustomCompare>& m_openlist)
{
    float newG, newH, newF;
    if (row >= AISystem::Instance().row_size || row < 0 || col >= AISystem::Instance().col_size || col < 0)
    {
        return;
    }
    if (localgrid[row][col].valid == 1)
    {
        if (DestinationReach(row, col, end))
        {
            localgrid[row][col].parent = &(localgrid[i][j]);
            StoreMovement(&(localgrid[row][col]));
            // std::cout << "Destination found\n";
            found = true;
            return;
        }
        if (!localgrid[row][col].closedlist)
        {
            newG = localgrid[row][col].g + gval;
            newH = ComputeHValue(row, col, end);
            newF = newG + newH;
            float gridF = localgrid[row][col].f;
            if (gridF == FLT_MAX || newF < gridF)
            {
                localgrid[row][col].f = newF;
                localgrid[row][col].g = newG;
                localgrid[row][col].h = newH;
                localgrid[row][col].parent = &(localgrid[i][j]);
                m_openlist.push(localgrid[row][col]);
            }
        }
    }
}

Pathfinding::Pathfinding(GameObject* parentObject, const std::string& name)
    : IComponent(parentObject, name)
{
    if (m_OwnerObject->GetParentLayer() == nullptr) return;
    localgrid = std::move(AISystem::Instance().m_gridMap);
    initialPos = m_OwnerObject->GetComponent<Transform>()->GetWorldPosition();
    // GameObject * coreObj = Application::Instance().GetCurrentScene()->GetLayers().front()->GetObjectByName("Core");
}

bool Pathfinding::AStarFindPath(const Vector3& end, bool splitPath)
{
    if (m_OwnerObject->GetParentLayer() == nullptr) return false;
    int rowtmp, coltmp;
    // 1 is main path, splitpath==true is indv path
    if (splitPath)
    {
        startPos = m_OwnerObject->GetComponent<Transform>()->GetWorldPosition();
    }
    else
    {
        // localgrid = AISystem::Instance().m_gridMap;
        startPos = initialPos;
    }

    // Reset for recomputation
    for (int i = 0; i < AISystem::Instance().row_size; ++i)
    {
        for (int j = 0; j < AISystem::Instance().col_size; ++j)
        {
            localgrid[i][j].closedlist = false;
            localgrid[i][j].f = FLT_MAX;
            localgrid[i][j].g = 0.f;
        }
    }

    // std::cout << "Starting pt: " << "Startpos: " << startPos[0] << " " << startPos[2] << "\n";
    ConvertToIndex(rowtmp, coltmp, startPos);
    NodePath& startNode = localgrid[rowtmp][coltmp];

    startNode.parent = nullptr;
    if (!CheckValidity(rowtmp, coltmp))
    {
        std::cout << "Starting pt is invalid!"
                  << "Startpos: " << startPos[0] << " " << startPos[2] << "\n";
        return false;
    }
    if (DestinationReach(rowtmp, coltmp, end))
    {
        std::cout << "Destination already reached!\n";
        return true;
    }
    ConvertToIndex(rowtmp, coltmp, end);
    NodePath& endNode = localgrid[rowtmp][coltmp];
    if (!CheckValidity(rowtmp, coltmp))
    {
        std::cout << "End pt is invalid!\n";
        return false;
    }
    isPathSplit = splitPath;
    startNode.f = 0;
    startNode.g = 0;
    startNode.h = 0;
    std::priority_queue<NodePath, std::vector<NodePath>, CustomCompare> m_openlist;
    m_openlist.push(startNode);
    bool destFound = false;
    while (!m_openlist.empty())
    {
        NodePath currNode = m_openlist.top();
        m_openlist.pop();
        int i = currNode.row;
        int j = currNode.col;
        // Add node to closed list
        localgrid[i][j].closedlist = true;
        if ((localgrid[i][j].flags & 0x10000000) == 0x10000000)
        {
            // top
            UpdateNeighbour(i - 1, j, 1.0f, end, destFound, i, j, m_openlist);
            if (destFound) return true;
        }
        if ((localgrid[i][j].flags & 0x00000010) == 0x00000010)
        {
            // btm
            UpdateNeighbour(i + 1, j, 1.0f, end, destFound, i, j, m_openlist);
            if (destFound) return true;
        }
        if ((localgrid[i][j].flags & 0x00010000) == 0x00010000)
        {
            // left
            UpdateNeighbour(i, j - 1, 1.0f, end, destFound, i, j, m_openlist);
            if (destFound) return true;
        }
        if ((localgrid[i][j].flags & 0x00001000) == 0x00001000)
        {
            // right
            UpdateNeighbour(i, j + 1, 1.0f, end, destFound, i, j, m_openlist);
            if (destFound) return true;
        }
        // DIAGONALS
        // if ((localgrid[i][j].flags & 0x10000000) == 0x10000000)
        //{
        //  //topleft
        //  UpdateNeighbour(i - 1, j - 1, 10000000000.414f, end, destFound, i, j, m_openlist);
        //  if (destFound)
        //    return true;
        //}
        // if ((localgrid[i][j].flags & 0x00100000) == 0x00100000)
        //{
        //  //topright
        //  UpdateNeighbour(i - 1, j + 1, 1.414f, end, destFound, i, j, m_openlist);
        //  if (destFound)
        //    return true;
        //}
        // if ((localgrid[i][j].flags & 0x00000100) == 0x00000100)
        //{
        //  //btmleft
        //  UpdateNeighbour(i + 1, j - 1, 1.414f, end, destFound, i, j, m_openlist);
        //  if (destFound)
        //    return true;
        //}
        // if ((localgrid[i][j].flags & 0x00000001) == 0x00000001)
        //{
        //  //btmright
        //  UpdateNeighbour(i + 1, j + 1, 1.414f, end, destFound, i, j, m_openlist);
        //  if (destFound)
        //    return true;
        //}
    }

    std::cout << "No path found\n";
    return false;
}

void Pathfinding::OnUpdate(float dt)
{
    ////Non Shared
    // for (auto & elem : m_paths)
    //{
    //  Renderer::Instance().DrawCube(1.f, elem);
    //}
    ////Original path- SHared

    // for (auto & elem : AISystem::Instance().slimePaths)
    //{
    //  Renderer::Instance().DrawCube(1.f, elem);
    //}
}

void Pathfinding::ChangeLocalToBaseGrid()
{
    if (isBaseGrid)
    {
        localgrid = AISystem::Instance().m_gridMap;
        isBaseGrid = false;
    }
    else
    {
        localgrid = AISystem::Instance().m_baseMap;
        isBaseGrid = true;
    }
}

REFLECT_INIT(Pathfinding)
REFLECT_END()