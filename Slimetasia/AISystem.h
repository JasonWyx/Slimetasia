#pragma once
#include "CorePrerequisites.h"
#include "ISystem.h"
#include "PlayerPref.h"
enum InList
{
    NONE = 0,
    OPENLIST,
    CLOSEDLIST
};

struct NodePath
{
    NodePath()
        : valid(-100)
        , row()
        , col()
        , f(FLT_MAX)
        , g(0.f)
        , h(0.f)
        , pos()
        , flags(0x11111111)
        , parent(nullptr)
        , closedlist(false)
    {
    }
    NodePath(const NodePath& rhs)
        : valid(rhs.valid)
        , row(rhs.row)
        , col(rhs.col)
        , f(rhs.f)
        , g(rhs.g)
        , h(rhs.h)
        , pos(rhs.pos)
        , flags(rhs.flags)
        , parent(rhs.parent)
        , closedlist(rhs.closedlist)
    {
    }
    NodePath& operator=(const NodePath& rhs)
    {
        valid = rhs.valid;
        row = rhs.row;
        col = rhs.col;
        f = rhs.f;
        g = rhs.g;
        h = rhs.h;
        pos = rhs.pos;
        flags = rhs.flags;
        parent = rhs.parent;
        closedlist = rhs.closedlist;
        return *this;
    }

    // Initialized as invalid for base map
    int valid = -100;
    // index of Node
    int row, col;
    float f;
    float g;
    float h;
    Vector3 pos;
    // top-left, top, top-right, left, right, bot-left, bot, bot-right
    int flags;
    NodePath* parent;
    bool closedlist;
    friend bool operator<(const NodePath& lhs, const NodePath& rhs) { return lhs.g < rhs.g; }

    friend bool operator>(const NodePath& lhs, const NodePath& rhs) { return lhs.g > rhs.g; }
};

struct CustomCompare
{
    bool operator()(const NodePath& lhs, const NodePath& rhs) { return lhs.f > rhs.f; }
};

class AISystem : public ISystem<AISystem>
{
    // No copy or move semantics allowed.
    AISystem(const AISystem& rhs) = delete;
    AISystem(AISystem&& rhs) = delete;
    AISystem& operator=(const AISystem& rhs) = delete;
    AISystem& operator=(AISystem&& rhs) = delete;

public:
    AISystem();
    void Init();
    ~AISystem();
    // funcs
    void Update(const float& dt);
    float d;
    float w;
    float xStartPos;
    float zStartPos;
    int numSpawnpts;
    int path_index = 1;
    std::vector<unsigned> indexes;
    Vector2 minPos;
    Vector2 maxPos;
    int counter;
    NodePath** m_gridMap;
    NodePath** m_baseMap;
    // Original route
    std::vector<Vector3> slimePaths;
    // Use this to get all the paths from n amt of spawn points from spawn pt to core
    std::vector<std::vector<Vector3>> originalPathsArr;
    bool isOriginalPathChanged = false;
    int row_size;
    int col_size;
    void Convert(int& row, int& col, Vector3 pos);
    void MarkInvalidNodes(NodePath& n);
    void MarkInvalidByPosition(const Vector3& pos, int validNum);
    void MarkValidByPosition(const Vector3& pos);
    void MarkValidNodes(NodePath& n);
    unsigned NearestPathIndex(const Vector3&);
    void RevertBase();
    Vector3 RetrieveGridPos(Vector3 pos);
    Vector3 RetrieveGridPosGeneral(Vector3 pos);
    bool CheckValidGrid(Vector3 pos);
};
