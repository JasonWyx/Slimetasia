#include "AISystem.h"

#include "Application.h"
#include "BoxCollider.h"
#include "Layer.h"
#include "PhysicsSystem.h"
#include "Ray.h"
#include "Renderer.h"
#include "Scene.h"
#define Sz 4
// Converts pos to i and j index
void AISystem::Convert(int& row, int& col, Vector3 pos)
{
    col = int((pos.x - minPos.x) / 2);
    row = int((pos.z - minPos.y) / 2);
}

Vector3 AISystem::RetrieveGridPos(Vector3 pos)
{
    int i, j;
    Convert(i, j, pos);
    // Within array size
    if (i < row_size && i >= 0 && j < col_size && j >= 0)
    {
        if (m_gridMap[i][j].valid == 1)
        {
            return m_gridMap[i][j].pos;
        }
    }
    return Vector3(0, 300, 0);
}

Vector3 AISystem::RetrieveGridPosGeneral(Vector3 pos)
{
    int i, j;
    Convert(i, j, pos);
    // Within array size
    if (i < row_size && i >= 0 && j < col_size && j >= 0)
    {
        return m_gridMap[i][j].pos;
    }
    return Vector3(0, 300, 0);  // out of range
}

bool AISystem::CheckValidGrid(Vector3 pos)
{
    int i, j;
    Convert(i, j, pos);
    if (i < row_size && i >= 0 && j < col_size && j >= 0)
    {
        return (m_gridMap[i][j].valid == 1);
    }
    return false;
}

void AISystem::MarkInvalidNodes(NodePath& n)
{
    if (n.col - 1 >= 0)  // left
    {
        if (n.row - 1 >= 0)  // top
        {
            m_gridMap[n.row - 1][n.col - 1].flags &= 0x11111110;  // top-left node mark input as invalid(hence btm right)
        }
        if (n.row + 1 < row_size)  // btm
        {
            m_gridMap[n.row + 1][n.col - 1].flags &= 0x11011111;  // btm-left node mark input as invalid(hence top right)
        }
        m_gridMap[n.row][n.col - 1].flags &= 0x11110111;  // left node marked the input node as invalid(hence right)
    }
    if (n.col + 1 < col_size)  // right
    {
        if (n.row - 1 >= 0)  // top
        {
            m_gridMap[n.row - 1][n.col + 1].flags &= 0x11111011;  // top-right node mark input as invalid(hence btm left)
        }
        if (n.row + 1 < row_size)  // btm
        {
            m_gridMap[n.row + 1][n.col + 1].flags &= 0x01111111;  // btm-right node mark input as invalid(hence top left)
        }
        m_gridMap[n.row][n.col + 1].flags &= 0x11101111;  // right node marked the input node as invalid(hence left)
    }
    if (n.row + 1 < row_size)  // btm
    {
        m_gridMap[n.row + 1][n.col].flags &= 0x10111111;  // btm node mark input as invalid(hence top)
    }
    if (n.row - 1 >= 0)  // top
    {
        m_gridMap[n.row - 1][n.col].flags &= 0x11111101;  // top node mark input as invalid(hence btm)
    }
}

void AISystem::MarkInvalidByPosition(const Vector3& pos, int validNum)
{
    // Converts position into i and j
    int i, j;
    Convert(i, j, pos);
    // std::cout << "Marking invalid path\n";
    if (m_gridMap[i][j].valid == 1)
    {
        std::cout << "invalid i: " << i << " invalid j: " << j << std::endl;
        std::cout << "Marking invalid path\n";
        // set the valid to input
        m_gridMap[i][j].valid = validNum;
        // mark neighbours
        MarkInvalidNodes(m_gridMap[i][j]);
    }
}

void AISystem::MarkValidByPosition(const Vector3& pos)
{
    // Converts position into i and j
    int i, j;
    Convert(i, j, pos);
    // std::cout << "Marking valid path\n";
    if (m_gridMap[i][j].valid == -1)
    {
        // set the valid to input
        std::cout << "valid i: " << i << " valid j: " << j << std::endl;
        m_gridMap[i][j].valid = 1;
        MarkValidNodes(m_gridMap[i][j]);
    }
    // else
    //{
    //  std::cout << "your setting the wrong grid!\n";
    //}
}

void AISystem::MarkValidNodes(NodePath& n)
{
    if (n.col - 1 >= 0)  // left
    {
        if (n.row - 1 >= 0)  // top
        {
            m_gridMap[n.row - 1][n.col - 1].flags |= 0x00000001;  // top-left node mark input as invalid(hence btm right)
        }
        if (n.row + 1 < row_size)  // btm
        {
            m_gridMap[n.row + 1][n.col - 1].flags |= 0x00100000;  // btm-left node mark input as invalid(hence top right)
        }
        m_gridMap[n.row][n.col - 1].flags |= 0x00001000;  // left node marked the input node as invalid(hence right)
    }
    if (n.col + 1 < col_size)  // right
    {
        if (n.row - 1 >= 0)  // top
        {
            m_gridMap[n.row - 1][n.col + 1].flags |= 0x00000100;  // top-right node mark input as invalid(hence btm left)
        }
        if (n.row + 1 < row_size)  // btm
        {
            m_gridMap[n.row + 1][n.col + 1].flags |= 0x10000000;  // btm-right node mark input as invalid(hence top left)
        }
        m_gridMap[n.row][n.col + 1].flags |= 0x00010000;  // right node marked the input node as invalid(hence left)
    }
    if (n.row + 1 < row_size)  // btm
    {
        m_gridMap[n.row + 1][n.col].flags |= 0x01000000;  // btm node mark input as invalid(hence top)
    }
    if (n.row - 1 >= 0)  // top
    {
        m_gridMap[n.row - 1][n.col].flags |= 0x00000010;  // top node mark input as invalid(hence btm)
    }
}

unsigned AISystem::NearestPathIndex(const Vector3& currpos)
{
    float sqDist = FLT_MAX;
    unsigned index = 0;
    for (unsigned i = 0; i < slimePaths.size(); ++i)
    {
        float sqDistNew = slimePaths[i].SquareDistance(currpos);
        if (sqDistNew < sqDist)
        {
            index = i;
            sqDist = sqDistNew;
        }
    }
    return index;
}

AISystem::AISystem()
    : m_gridMap(nullptr)
    , d()
    , w()
    , xStartPos()
    , zStartPos()
    , row_size()
    , col_size()
    , counter(0)
{
}

void AISystem::Init()
{
    counter++;
    GameObjectList myObject = Application::Instance().GetCurrentScene()->GetLayers().front()->GetObjectListByTag("Ground");
    indexes.clear();
    if (originalPathsArr.size() > 0) originalPathsArr.clear();
    isOriginalPathChanged = false;
    std::string levelName = PlayerPref::GetVariable<std::string>("CurrentLevel");
    std::vector<Vector3> spwnpts = PlayerPref::GetVariable<std::vector<Vector3>>("SpawnPositions", levelName);

    numSpawnpts = static_cast<int>(spwnpts.size());
    if (myObject.size() > 0)
    {
        Vector3 pos1 = myObject.front()->GetComponent<Transform>()->GetWorldPosition();
        Vector3 scale1 = myObject.front()->GetComponent<Transform>()->GetWorldScale();
        maxPos = Vector2(pos1.x + scale1.x / 2.f, pos1.z + scale1.z / 2.f);
        minPos = Vector2(pos1.x - scale1.x / 2.f, pos1.z - scale1.z / 2.f);
        // Find min, max of x and z
        for (auto& elem : myObject)
        {
            Vector3 posTemp = elem->GetComponent<Transform>()->GetWorldPosition();
            Vector3 scaleTemp = elem->GetComponent<Transform>()->GetWorldScale();
            float maxtmpX = posTemp.x + scaleTemp.x / 2.f;
            float maxtmpZ = posTemp.z + scaleTemp.z / 2.f;
            float mintmpX = posTemp.x - scaleTemp.x / 2.f;
            float mintmpZ = posTemp.z - scaleTemp.z / 2.f;
            if (maxPos.x < maxtmpX) maxPos.x = maxtmpX;
            if (maxPos.y < maxtmpZ) maxPos.y = maxtmpZ;
            if (minPos.x > mintmpX) minPos.x = mintmpX;
            if (minPos.y > mintmpZ) minPos.y = mintmpZ;
        }
        d = maxPos.y - minPos.y;
        w = maxPos.x - minPos.x;

        if (m_gridMap == nullptr)
        {
            row_size = static_cast<unsigned>(ceil(d / 2.f));
            col_size = static_cast<unsigned>(ceil(w / 2.f));
            m_gridMap = new NodePath*[row_size];
            m_baseMap = new NodePath*[row_size];
            for (int i = 0; i < row_size; ++i)
            {
                m_gridMap[i] = new NodePath[col_size];
                m_baseMap[i] = new NodePath[col_size];
            }
        }
        else
        {
            for (int i = 0; i < row_size; ++i)
            {
                delete[] m_gridMap[i];
                delete[] m_baseMap[i];
            }
            delete[] m_gridMap;
            delete[] m_baseMap;
            row_size = static_cast<unsigned>(ceil(d / 2.f));
            col_size = static_cast<unsigned>(ceil(w / 2.f));
            m_gridMap = new NodePath*[row_size];
            m_baseMap = new NodePath*[row_size];
            for (int i = 0; i < row_size; ++i)
            {
                m_gridMap[i] = new NodePath[col_size];
                m_baseMap[i] = new NodePath[col_size];
            }
        }

        xStartPos = minPos.x + 1;
        zStartPos = minPos.y + 1;

        Vector3 rayDir(0.f, -1.f, 0.f);
        // Make bigger grids - uses +=2
        unsigned row = 0;
        unsigned col = 0;
        for (unsigned i = 0; i < (unsigned)d; i += 2, row++)
        {
            for (unsigned j = 0, col = 0; j < (unsigned)w; j += 2, col++)
            {
                m_gridMap[row][col].col = col;
                m_gridMap[row][col].row = row;
                if (row == 0)  // remove top-left, top, top-right
                    m_gridMap[row][col].flags &= 0x00011111;
                if (col == 0)  // remove left-top, left,-left-btm
                    m_gridMap[row][col].flags &= 0x01101011;
                if (col == col_size - 1)  // remove right-top, right, right-btm
                    m_gridMap[row][col].flags &= 0x11111000;
                if (row == row_size - 1)  // remove btm-left, btm, btm-right
                    m_gridMap[row][col].flags &= 0x11010110;

                Vector3 pos(xStartPos + j, 40.f, zStartPos + i);
                Ray rc(pos, rayDir);
                std::vector<RaycastData_tmp> datas = PhysicsSystem::Instance().Raycast(rc);
                if (datas.size() == 0)
                {
                    m_gridMap[row][col].valid = -1;
                    continue;
                }
                if (datas.size() > 1)
                {
                    std::sort(datas.begin(), datas.end(), [](const RaycastData_tmp& a, const RaycastData_tmp& b) { return (a.m_HitFrac < b.m_HitFrac); });
                }

                // if data hits ground
                GameObject* firstHitObj = datas.front().m_HitObject;
                if (firstHitObj == nullptr)
                {
                    m_gridMap[row][col].valid = -1;
                }
                // y value of the grid
                pos.y = firstHitObj->GetComponent<Transform>()->GetWorldPosition().y + firstHitObj->GetComponent<Transform>()->GetWorldScale().y / 2.f + 1.f;
                if (firstHitObj->GetTag() == "Ground" || firstHitObj->GetName() == "Core" || firstHitObj->GetName() == "NoEntry")
                {
                    if (firstHitObj->GetName() == "Core") pos.y = firstHitObj->GetComponent<Transform>()->GetWorldPosition().y - firstHitObj->GetComponent<BoxCollider>()->GetHeight() / 2.f + 1.f;
                    m_gridMap[row][col].valid = 1;
                    indexes.push_back(i);
                    indexes.push_back(j);
                    m_gridMap[row][col].pos = pos;
                }
                else
                {
                    MarkInvalidNodes(m_gridMap[row][col]);
                    m_gridMap[row][col].valid = -1;
                    m_gridMap[row][col].pos = pos;
                }
            }
        }
        for (int i = 0; i < row_size; ++i)
        {
            for (int j = 0; j < col_size; ++j)
            {
                m_baseMap[i][j] = m_gridMap[i][j];
            }
        }
        // std::copy(m_gridMap, m_gridMap + row_size * col_size, m_baseMap);
    }
}

AISystem::~AISystem()
{
    for (int i = 0; i < row_size; ++i)
    {
        delete[] m_gridMap[i];
        delete[] m_baseMap[i];
    }
    delete[] m_gridMap;
    delete[] m_baseMap;
}

void AISystem::Update(const float& dt)
{
    //#ifdef EDITOR
    //  if (Editor::s_isPlaying == false)
    //  {
    //    Init();
    //    for (int i = 0; i < row_size; ++i)
    //    {
    //      for (int j = 0; j < col_size; ++j)
    //      {
    //        if (m_gridMap[i][j].valid == 1)
    //          Renderer::Instance().Draw2DBox(1.f, 1.f, m_gridMap[i][j].pos, Color4(0.0, 0.0, 1.0, 1.0));
    //      }
    //    }
    //  }
    //#endif

    // for (auto&elem : originalPathsArr)
    //{
    //  for (auto&path : elem)
    //  {
    //    Renderer::Instance().DrawCube(1.f, path);
    //  }
    //}
}

void AISystem::RevertBase()
{
    for (int i = 0; i < row_size; ++i)
    {
        for (int j = 0; j < col_size; ++j)
        {
            m_gridMap[i][j] = m_baseMap[i][j];
        }
    }
}