#include "luascript.h"

#include <cmath>
#include <sstream>

#include "Application.h"
#include "AudioEmitter.h"
#include "AudioListener.h"
#include "BoxCollider.h"
#include "Editor.h"
#include "GameObject.h"
#include "Input.h"
#include "Math.h"
#include "MeshAnimator.h"
#include "MeshRenderer.h"
#include "ParticleSystem.h"
#include "Pathfinding.h"
#include "PhysicsSystem.h"
#include "PlayerPref.h"
#include "Ray.h"
#include "Renderer.h"
#include "ResourceManager.h"
#include "RigidbodyComponent.h"
#include "Scene.h"
#include "SphereCollider.h"
#include "TextRenderer.h"
#include "Transform.h"

// ============================================================================
// HELPER FUNCTIONS
// ============================================================================
template <typename T> static bool GetComp(lua_State* L, T** result)
{
    GameObject** go = LuaScript::checkGameObject(L);
    *result = (*go)->GetComponent<T>();
    return *result != nullptr;
}

template <typename T> static T check(std::string name, lua_State* L, int index, bool assert)
{
    void* ud = nullptr;

    if (assert)
    {
        ud = luaL_testudata(L, index, name.c_str());

        if (ud == NULL)
        {
            std::cout << "LUA ASSERT : Parameter (" << index << " ) is not of type " << name << std::endl;
            throw(1);
        }
        luaL_argcheck(L, ud != NULL, index, (name + std::string{" expected"}).c_str());
    }
    else
        ud = luaL_testudata(L, index, name.c_str());

    return (T)ud;
}

template <typename T> static int pushCustomType(std::string name, lua_State* L, T input)
{
    T* t = (T*)lua_newuserdata(L, sizeof(T));
    *t = input;
    luaL_getmetatable(L, name.c_str());
    lua_setmetatable(L, -2);
    return 1;
}

template <typename T> static int pushCustomTypeArray(std::string name, lua_State* L, std::vector<T> input)
{
    lua_createtable(L, (int)input.size(), 0);
    int newTable = lua_gettop(L);
    for (unsigned i = 1; i <= input.size(); ++i)
    {
        T* t = (T*)lua_newuserdata(L, sizeof(T));
        *t = input[i - 1];
        luaL_getmetatable(L, name.c_str());
        lua_setmetatable(L, -2);
        lua_rawseti(L, newTable, i);
    }
    return 1;
}

// ============================================================================
// LIBARIES
// ============================================================================
/// LAYER
const luaL_Reg LuaScript::layerlib[] = {{"Name", layer_name},
                                        {"Create", layer_createObject},
                                        {"GetObject", layer_find},
                                        {"GetObjectsList", layer_getObjects},
                                        {"GetObjectsListByName", layer_getObjectsByName},
                                        {"GetObjectByID", layer_getObjectByID},
                                        {"GetObjectsListBySubName", layer_getObjectsBySubName},
                                        {"GetObjectsListByTag", layer_getObjectsByTag},
                                        {"DestroyObjectsWithTag", layer_DestroyAllObjectsWithTag},
                                        {NULL, NULL}};
/// TYPES
const luaL_Reg LuaScript::vector3lib[] = {{"__index", vector3_index},
                                          {"__newindex", vector3_newindex},
                                          {"__add", vector3_add},
                                          {"__sub", vector3_sub},
                                          {"__mul", vector3_mul},
                                          {"__div", vector3_div},
                                          {"__unm", vector3_unm},
                                          {"__eq", vector3_eq},
                                          {"Set", vector3_set},
                                          {"Normalize", vector3_Normalize},
                                          {"Normalized", vector3_Normalized},
                                          {"Project", vector3_Projection},
                                          {"Cross", vector3_Cross},
                                          {"Dot", vector3_Dot},
                                          {"Length", vector3_Length},
                                          {"SquareLength", vector3_SquareLength},
                                          {"DistanceTo", vector3_DistanceTo},
                                          {"SquareDistanceTo", vector3_SquaredDistanceTo},
                                          {"Rotate", vector3_Rotate},
                                          {"x", vector3_X},
                                          {"y", vector3_Y},
                                          {"z", vector3_Z},
                                          {"PolarAngle", vector3_polarAngle},
                                          {NULL, NULL}};
const luaL_Reg LuaScript::colorlib[] = {{"__index", color_index},
                                        {"__newindex", color_newindex},
                                        {"__add", color_add},
                                        {"__sub", color_sub},
                                        {"__mul", color_mul},
                                        {"__div", color_div},
                                        {"__eq", color_eq},
                                        {"Set", color_set},
                                        {"r", color_R},
                                        {"g", color_G},
                                        {"b", color_B},
                                        {"a", color_A},
                                        {NULL, NULL}};
/// Physics Ray cast
const luaL_Reg LuaScript::rayCastDatalib[] = {{"Point", rayCastData_point}, {"Normal", rayCastData_normal}, {"HitFrac", rayCastData_hitFrac}, {"GameObject", rayCastData_gameObject}, {NULL, NULL}};
/// Game object
const luaL_Reg LuaScript::gameObjectlib[] = {{"Name", gameObject_name},
                                             {"SetName", gameObject_Setname},
                                             {"Destroy", gameObject_destroy},
                                             {"SetActive", setGameObjectActive},
                                             {"GetActive", getGameObjectActive},
                                             {"GetComponent", getGameObjectComponent},
                                             {"GetComponents", getGameObjectComponents},
                                             {"AddComponent", addGameObjectComponent},
                                             {"GetLayer", getGameObjectLayer},
                                             {"GetLuaScript", gameObject_getLuaScript},
                                             {"AddComponent", gameObject_addComponent},
                                             {"GetID", gameObject_GetID},
                                             {"RemoveComponent", gameObject_removeComponent},
                                             {"Tag", gameObject_GetTag},
                                             {"SetParent", gameObject_SetParent},
                                             {"GetParent", gameObject_GetParent},
                                             {"UnParent", gameObject_UnParent},
                                             {"GetChild", gameObject_GetChild},
                                             {NULL, NULL}};
/// Components
const luaL_Reg LuaScript::transformlib[] = {{"Owner", component_Owner},
                                            {"SetActive", component_SetActive},
                                            //{ "GetlocalPosition"         , transform_GetLocalPosition          },
                                            //{ "GetlocalRotation"         , transform_GetLocalRotation          },
                                            //{ "GetlocalScale"            , transform_GetLocalScale             },
                                            {"GetWorldPosition", transform_GetWorldPosition},
                                            {"GetWorldRotation", transform_GetWorldRotation},
                                            {"GetWorldScale", transform_GetWorldScale},
                                            //{ "SetLocalPosition"         , transform_SetLocalPosition          },
                                            //{ "SetLocalRotation"         , transform_SetLocalRotation          },
                                            //{ "SetLocalScale"            , transform_SetLocalScale             },
                                            {"SetWorldPosition", transform_SetWorldPosition},
                                            {"SetWorldRotation", transform_SetWorldRotation},
                                            {"SetWorldScale", transform_SetWorldScale},
                                            {"Translate", transform_Translate},
                                            {"Rotate", transform_Rotate},
                                            {"Scale", transform_Scale},
                                            {"ForwardVector", transform_GetForwardVector},
                                            {"UpwardVector", transform_GetUpwardVector},
                                            {"RightVector", transform_GetRightVector},
                                            {"LookAt", transform_LookAt},
                                            {"LookAtV", transform_LookAtV},
                                            {NULL, NULL}};
const luaL_Reg LuaScript::rigidbodylib[] = {{"Owner", component_Owner},
                                            {"SetActive", component_SetActive},
                                            {"GetGhost", rigidbody_GetGhost},
                                            //{ "GetCollisionWithStatic"   , rigidbody_GetCollideWithStatic      },
                                            {"GetMass", rigidbody_GetMass},
                                            //{ "GetDrag"                  , rigidbody_GetDrag                   },
                                            {"GetBodyType", rigidbody_GetBodytype},
                                            //{ "GetFreezeRotation"        , rigidbody_GetFreezeRotation         },
                                            {"GetRestitution", rigidbody_GetRestitution},
                                            {"GetVelocity", rigidbody_GetVelocity},
                                            {"GetAcceleration", rigidbody_GetAcceleration},
                                            //{ "GetOffset"                , rigidbody_GetOffset                 },
                                            //{ "GetAngularVelocity"       , rigidbody_GetAngularVelocity        },
                                            {"GetGravityEnabled", rigidbody_GetGravityEnabled},
                                            //{ "GetAffectedByResistance"  , rigidbody_GetIsAffectedByResistance },
                                            {"GetForce", rigidbody_GetForce},
                                            {"SetGhost", rigidbody_SetGhost},
                                            //{ "SetCollisionWithStatic"   , rigidbody_SetCollideWithStatic      },
                                            {"SetMass", rigidbody_SetMass},
                                            //{ "SetDrag"                  , rigidbody_SetDrag                   },
                                            {"SetBodyType", rigidbody_SetBodytype},
                                            //{ "SetFreezeRotation"        , rigidbody_SetFreezeRotation         },
                                            {"SetRestitution", rigidbody_SetRestitution},
                                            {"SetVelocity", rigidbody_SetVelocity},
                                            {"SetAcceleration", rigidbody_SetAcceleration},
                                            //{ "SetOffset"                , rigidbody_SetOffset                 },
                                            //{ "SetAngularVelocity"       , rigidbody_SetAngularVelocity        },
                                            {"SetGravityEnabled", rigidbody_SetGravityEnabled},
                                            //{ "SetAffectedByResistance"  , rigidbody_SetIsAffectedByResistance },
                                            {"AddVelocity", rigidbody_AddVelocity},
                                            {"AddForce", rigidbody_AddForce},
                                            {"SetYVelocity", rigidbody_SetYVelocity},
                                            {NULL, NULL}};
const luaL_Reg LuaScript::audioListenerlib[] = {{"Owner", component_Owner}, {"SetActive", component_SetActive}, {"IsMain", audioListener_IsMain}, {"SetMain", audioListener_SetMain}, {NULL, NULL}};
const luaL_Reg LuaScript::audioEmitterlib[] = {{"Owner", component_Owner},
                                               {"SetActive", component_SetActive},
                                               {"Play", audioEmitter_Play},
                                               {"Stop", audioEmitter_Stop},
                                               {"Pause", audioEmitter_Pause},
                                               {"IsPlaying", audioEmitter_IsPlaying},
                                               {"IsPaused", audioEmitter_IsPaused},
                                               {"SetVolume", audioEmitter_SetVolume},
                                               {"GetVolume", audioEmitter_GetVolume},
                                               {"SetPitch", audioEmitter_SetPitch},
                                               {"GetPitch", audioEmitter_GetPitch},
                                               {"SetLoop", audioEmitter_SetLoop},
                                               {"SetLoopCount", audioEmitter_SetLoopCount},
                                               {"GetLoopCount", audioEmitter_GetLoopCount},
                                               {"SetAudioClip", audioEmitter_SetAudioClip},
                                               {"SetAndPlayAudioClip", audioEmitter_SetAndPlayAudioClip},
                                               {"SetMinDistance3D", audioEmitter_SetMinDistance3D},
                                               {"GetMinDistance3D", audioEmitter_GetMinDistance3D},
                                               {"SetMaxDistance3D", audioEmitter_SetMaxDistance3D},
                                               {"GetMaxDistance3D", audioEmitter_GetMaxDistance3D},
                                               {"GetAudioClip", audioEmitter_GetSoundName},
                                               {"FadeOut", audioEmitter_FadeOut},
                                               {"SetChannelGroup", audioEmitter_SetChannelGroup},
                                               {NULL, NULL}};
const luaL_Reg LuaScript::meshRendererlib[] = {{"Owner", component_Owner},
                                               {"SetActive", component_SetActive},
                                               {"SetColor", meshRenderer_SetColor},
                                               {"GetColor", meshRenderer_GetColor},
                                               {"SetDiffuseTexture", meshRenderer_SetDiffuse},
                                               {"GetMesh", meshRenderer_GetMesh},
                                               {"SetMesh", meshRenderer_SetMesh},
                                               {"SetEmissive", meshRenderer_SetEmissive},
                                               {"SetEmissiveTexture", meshRenderer_SetEmissiveTexture},
                                               {"GetEnableEmissive", meshRenderer_GetEnableEmissive},
                                               {"SetEnableEmissive", meshRenderer_SetEnableEmissive},
                                               {NULL, NULL}};
const luaL_Reg LuaScript::meshAnimatorlib[] = {{"Owner", component_Owner},
                                               {"SetActive", component_SetActive},
                                               {"Play", meshAnimator_PlayAnimation},
                                               {"PlayOnce", meshAnimator_PlayAnimationOnce},
                                               {"Stop", meshAnimator_StopAnimation},
                                               {"Pause", meshAnimator_PauseAnimation},
                                               {"GetAnimation", meshAnimator_GetAnimationSet},
                                               {"SetAnimation", meshAnimator_SetAnimationSet},
                                               {"IsPlaying", meshAnimator_IsPlaying},
                                               {"SetCrossFade", meshAnimator_SetCrossFadeBoolean},
                                               {"SetTimeScale", meshAnimator_SetTimeScale},
                                               {NULL, NULL}};
const luaL_Reg LuaScript::luaScriptlib[] = {{"Owner", component_Owner},       {"SetActive", component_SetActive}, {"GetVariable", lua_GetVariable},   {"GetVariableArray", lua_GetVariableArray},
                                            {"SetVariable", lua_SetVariable}, {"CallFunction", lua_callFunction}, {"EnableScript", lua_EnableScript}, {NULL, NULL}};
const luaL_Reg LuaScript::cameralib[] = {{"Owner", component_Owner},
                                         {"SetActive", component_SetActive},
                                         {"GetLookAt", camera_GetLookAt},
                                         {"SetLookAt", camera_SetLookAt},
                                         {"SetUp", camera_SetUp},
                                         {"GetViewporSize", camera_GetViewportSize},
                                         {"SetViewportSize", camera_SetViewportSize},
                                         {"SetUICam", camera_SetUICamera},
                                         {"SetLightIntensity", camera_SetDirectionalLightIntensity},
                                         {"SetLightDirection", camera_SetDirectionalLightDirection},
                                         {"SetShadowCasted", camera_SetShadowCasted},
                                         {"SetColor", camera_SetColor},
                                         {NULL, NULL}};
const luaL_Reg LuaScript::boxParticlelib[] = {{"Owner", component_Owner},
                                              {"SetActive", component_SetActive},

                                              {"GetStartMinColor", particle_GetStartMinColor},
                                              {"GetStartMaxColor", particle_GetStartMaxColor},
                                              {"GetEndMinColor", particle_GetEndMinColor},
                                              {"GetEndMaxColor", particle_GetEndMaxColor},
                                              {"GetMinSize", particle_GetMinSize},
                                              {"GetMaxSize", particle_GetMaxSize},
                                              {"GetEndTexture", particle_GetEndTexture},
                                              {"GetTextureFade", particle_GetTextureFade},
                                              {"SetStartMinColor", particle_SetStartMinColor},
                                              {"SetStartMaxColor", particle_SetStartMaxColor},
                                              {"SetEndMinColor", particle_SetEndMinColor},
                                              {"SetEndMaxColor", particle_SetEndMaxColor},
                                              {"SetMinSize", particle_SetMinSize},
                                              {"SetMaxSize", particle_SetMaxSize},
                                              {"SetEndTexture", particle_SetEndTexture},
                                              {"SetTextureFade", particle_SetTextureFade},
                                              {"SetFloorHeight", particle_SetFloorHeight},

                                              {"GetPos", boxParticle_GetPos},
                                              {"SetPos", boxParticle_SetPos},
                                              {"GetOffset", boxParticle_GetStartPosOff},
                                              {"SetOffset", boxParticle_SetStartPosOff},
                                              {"GetMinVel", boxParticle_GetMinVel},
                                              {"SetMinVel", boxParticle_SetMinVel},
                                              {"GetMaxVel", boxParticle_GetMaxVel},
                                              {"SetMaxVel", boxParticle_SetMaxVel},
                                              {"GetMinTime", boxParticle_GetMinTime},
                                              {"SetMinTime", boxParticle_SetMinTime},
                                              {"GetMaxTime", boxParticle_GetMaxTime},
                                              {"SetMaxTime", boxParticle_SetMaxTime},
                                              {"GetEmitRate", boxParticle_GetEmitRate},
                                              {"SetEmitRate", boxParticle_SetEmitRate},
                                              {"AddAttractor", boxParticle_AddAttractor},
                                              {"RemoveAttractor", boxParticle_RemoveAttractor},
                                              {NULL, NULL}};
const luaL_Reg LuaScript::circleParticlelib[] = {{"Owner", component_Owner},
                                                 {"SetActive", component_SetActive},
                                                 {"GetStartMinColor", particle_GetStartMinColor},
                                                 {"GetStartMaxColor", particle_GetStartMaxColor},
                                                 {"GetEndMinColor", particle_GetEndMinColor},
                                                 {"GetEndMaxColor", particle_GetEndMaxColor},
                                                 {"GetMinSize", particle_GetMinSize},
                                                 {"GetMaxSize", particle_GetMaxSize},
                                                 {"GetEndTexture", particle_GetEndTexture},
                                                 {"GetTextureFade", particle_GetTextureFade},
                                                 {"SetStartMinColor", particle_SetStartMinColor},
                                                 {"SetStartMaxColor", particle_SetStartMaxColor},
                                                 {"SetEndMinColor", particle_SetEndMinColor},
                                                 {"SetEndMaxColor", particle_SetEndMaxColor},
                                                 {"SetMinSize", particle_SetMinSize},
                                                 {"SetMaxSize", particle_SetMaxSize},
                                                 {"SetEndTexture", particle_SetEndTexture},
                                                 {"SetTextureFade", particle_SetTextureFade},
                                                 {"SetFloorHeight", particle_SetFloorHeight},

                                                 {"GetEmitRate", circleParticle_GetEmitRate},
                                                 {"SetEmitRate", circleParticle_SetEmitRate},
                                                 {"AddAttractor", circleParticle_AddAttractor},
                                                 {"RemoveAttractor", circleParticle_RemoveAttractor},
                                                 {NULL, NULL}};
const luaL_Reg LuaScript::pathFindinglib[] = {{"FindPath", pathFinding_AStarFindPath},        {"GetPath", pathFinding_GetPath},         {"ChangeLocalToBaseMap", pathFinding_ChangeLocalToBaseMap},
                                              {"GetPathByIndex", pathFinding_GetPathByIndex}, {"GetPathSize", pathFinding_GetPathSize}, {NULL, NULL}};
const luaL_Reg LuaScript::boxColliderlib[] = {{"GetHalfExtent", boxCollider_GetHalfExtent}, {NULL, NULL}};
const luaL_Reg LuaScript::sphereColliderlib[] = {{"SetRadius", sphereCollider_SetRadius}, {"GetRadius", sphereCollider_GetRadius}, {NULL, NULL}};
const luaL_Reg LuaScript::textRendererlib[] = {{"SetText", TextRenderer_SetText},
                                               {"GetText", TextRenderer_GetText},
                                               {"SetFontSize", TextRenderer_SetSize},
                                               {"GetFontSize", TextRenderer_GetSize},
                                               {"SetColor", TextRenderer_SetColor},
                                               {"GetColor", TextRenderer_GetColor},
                                               {"SetFaceCamera", TextRenderer_SetFaceCamera},
                                               {"GetFaceCamera", TextRenderer_GetFaceCamera},
                                               {"SetAnchorPoint", TextRenderer_SetAnchorPoint},
                                               {"GetAnchorPoint", TextRenderer_GetAnchorPoint},
                                               {"SetFont", TextRenderer_SetFont},
                                               {"GetFont", TextRenderer_GetFont},
                                               {"SetOutline", TextRenderer_SetOutline},
                                               {"SetOutlineSize", TextRenderer_SetOutlineSize},
                                               {NULL, NULL}};
/// Resource
const luaL_Reg LuaScript::resAnimationlib[] = {{"IsValid", resAnimation_Validate}, {NULL, NULL}};

const luaL_Reg LuaScript::attractorlib[] = {{"SetForce", attractor_SetForce}, {"GetForce", attractor_GetForce}, {NULL, NULL}};

const luaL_Reg LuaScript::directionalLightlib[] = {{"GetDirection", directionalLight_GetDirection},
                                                   {"SetDirection", directionalLight_SetDirection},
                                                   {"GetIntensity", Light_GetIntensity},
                                                   {"SetIntensity", Light_SetIntensity},
                                                   {"GetColor", Light_GetLightColor},
                                                   {"SetColor", Light_SetLightColor},
                                                   {"GetCastShadows", Light_IsCastShadows},
                                                   {"SetCastShadows", Light_SetCastShadows},
                                                   {"GetShadowDistance", Light_GetShadowDistance},
                                                   {"GetShadowBias", Light_GetShadowBias},
                                                   {"SetShadowBias", Light_SetShadowBias},
                                                   {NULL, NULL}};

const luaL_Reg LuaScript::pointLightlib[] = {{"GetIntensity", Light_GetIntensity},
                                             {"SetIntensity", Light_SetIntensity},
                                             {"GetColor", Light_GetLightColor},
                                             {"SetColor", Light_SetLightColor},
                                             {"GetCastShadows", Light_IsCastShadows},
                                             {"SetCastShadows", Light_SetCastShadows},
                                             {"GetShadowDistance", Light_GetShadowDistance},
                                             {"GetShadowBias", Light_GetShadowBias},
                                             {"SetShadowBias", Light_SetShadowBias},
                                             {NULL, NULL}};

const luaL_Reg LuaScript::spotLightlib[] = {{"GetDirection", spotLight_GetDirection}, {"SetDirection", spotLight_SetDirection}, {"GetIntensity", Light_GetIntensity},
                                            {"SetIntensity", Light_SetIntensity},     {"GetColor", Light_GetLightColor},        {"SetColor", Light_SetLightColor},
                                            {"GetCastShadows", Light_IsCastShadows},  {"SetCastShadows", Light_SetCastShadows}, {"GetShadowDistance", Light_GetShadowDistance},
                                            {"GetShadowBias", Light_GetShadowBias},   {"SetShadowBias", Light_SetShadowBias},   {NULL, NULL}};

// ============================================================================
// Delta time
// ============================================================================
int LuaScript::UnscaledDT(lua_State* L)
{
    lua_pushnumber(L, Application::Instance().GetGameTimer().GetActualFrameTime());
    return 1;
}

int LuaScript::SetTimeScale(lua_State* L)
{
    Application::Instance().GetGameTimer().SetTimeScale((float)lua_tonumber(L, 1));
    return 0;
}

int LuaScript::GetTimeScale(lua_State* L)
{
    lua_pushnumber(L, Application::Instance().GetGameTimer().GetTimeScale());
    return 1;
}

// ============================================================================
// LUA
// ============================================================================
int LuaScript::lua_write(lua_State* L)
{
    int params = lua_gettop(L);

    /// no parameters
    if (params == 0) return 0;
    /// Iterate through the parameters
    std::stringstream buffer;
    for (int i = 1; i <= params; ++i)
    {
        /// Default types
        if (lua_isstring(L, i))
        {
            const char* tmp = luaL_checkstring(L, i);
            buffer << tmp << " ";
        }
        else if (lua_isnumber(L, i))
        {
            double tmp = lua_tonumber(L, i);
            buffer << tmp << " ";
        }
        else if (lua_isinteger(L, i))
        {
            long long tmp = lua_tointeger(L, i);
            buffer << tmp << " ";
        }
        else if (lua_isboolean(L, i))
        {
            bool tmp = lua_toboolean(L, i);
            buffer << std::boolalpha << tmp << " ";
        }
        /// None default types
        else if (lua_isuserdata(L, i))
        {
            Vector3* vec3 = checkVector3(L, i, false);
            if (vec3) buffer << "(x:" << vec3->x << " y:" << vec3->y << " z:" << vec3->z << ") ";

            Vector4* color = checkColor(L, i, false);
            if (color) buffer << "(r:" << color->r << " g:" << color->g << " b:" << color->b << " a:" << color->a << ") ";

            /// Wring user type
            if (!vec3 && !color)
            {
                std::cout << "LUASCRIPT : WRITE - FAILED | Invalid USER type" << std::endl;
                return 0;
            }
        }
        /// Some wrong type
        else
        {
            std::cout << "LUASCRIPT : WRITE - FAILED | Invalid type" << std::endl;
            return 0;
        }
    }

    std::cout << buffer.str() << "\n";
    return 0;
}

int LuaScript::create_prefab(lua_State* L)
{
    std::string prefabName = luaL_checkstring(L, 1);
    GameObject** parent = checkGameObject(L, 2, false);

    GameObject* newPrefab = Editor::Instance().LuaCreateArchetypeObject(prefabName, Application::Instance().GetCurrentScene()->GetLayers().front());
    if (newPrefab && parent) newPrefab->SetParentObject((*parent)->GetID());

    return pushGameObject(L, newPrefab);
}

int LuaScript::Scene_Load(lua_State* L)
{
    std::string levelName = luaL_checkstring(L, 1);
    std::string nextLevel = "Resources/" + levelName + ".xml";
    ParticleSystem::Instance().Reset();
    Application::Instance().LoadScene(nextLevel.c_str());
    return 0;
}

int LuaScript::Scene_Reload(lua_State* L)
{
    Application::Instance().ReloadScene();
    return 0;
}

int LuaScript::Scene_Quit(lua_State* L)
{
    Application::Instance().QuitProgram();
    return 0;
}

int LuaScript::ToString(lua_State* L)
{
    int params = lua_gettop(L);
    std::string result = "";
    for (int i = 1; i <= params; ++i)
    {
        if (lua_isstring(L, i))
            result += std::string{luaL_checkstring(L, i)};
        else if (lua_isnumber(L, i))
            result += std::to_string((int)lua_tonumber(L, i));
        else if (lua_isinteger(L, i))
            result += std::to_string((float)lua_tointeger(L, i));
        else
            return 0;
    }

    lua_pushstring(L, result.c_str());
    return 1;
}

int LuaScript::ToInt(lua_State* L)
{
    lua_pushinteger(L, (int)lua_tonumber(L, 1));
    return 1;
}

void LuaScript::SetActive(bool state)
{
    isActive = state;
}

// ============================================================================
// PLAYER PREF
// ============================================================================
int LuaScript::Playerpref_CheckExist(lua_State* L)
{
    lua_gettop(L) == 1 ? lua_pushboolean(L, PlayerPref::CheckExist(luaL_checkstring(L, 1))) : lua_pushboolean(L, PlayerPref::CheckExist(luaL_checkstring(L, 1), luaL_checkstring(L, 2)));
    return 1;
}

int LuaScript::Playerpref_RemoveAll(lua_State* L)
{
    lua_gettop(L) == 0 ? PlayerPref::DeleteAllVariables() : PlayerPref::DeleteAllVariablesInTable(luaL_checkstring(L, 1));
    return 0;
}

int LuaScript::Playerpref_RemoveVariable(lua_State* L)
{
    lua_gettop(L) == 1 ? PlayerPref::DeleteVariable(luaL_checkstring(L, 1)) : PlayerPref::DeleteVariable(luaL_checkstring(L, 1), luaL_checkstring(L, 2));
    return 0;
}

int LuaScript::Playerpref_GetInteger(lua_State* L)
{
    lua_gettop(L) == 1 ? lua_pushinteger(L, PlayerPref::GetVariable<int>(luaL_checkstring(L, 1))) : lua_pushinteger(L, PlayerPref::GetVariable<int>(luaL_checkstring(L, 1), luaL_checkstring(L, 2)));
    return 1;
}

int LuaScript::Playerpref_SetInteger(lua_State* L)
{
    lua_gettop(L) <= 2 ? lua_pushboolean(L, PlayerPref::SaveVariable<int>(luaL_checkstring(L, 1), (int)lua_tointeger(L, 2)))
                       : lua_pushboolean(L, PlayerPref::SaveVariable<int>(luaL_checkstring(L, 1), (int)lua_tointeger(L, 2), luaL_checkstring(L, 3)));
    return 1;
}

int LuaScript::Playerpref_GetFloat(lua_State* L)
{
    lua_gettop(L) == 1 ? lua_pushnumber(L, PlayerPref::GetVariable<float>(luaL_checkstring(L, 1))) : lua_pushnumber(L, PlayerPref::GetVariable<float>(luaL_checkstring(L, 1), luaL_checkstring(L, 2)));
    return 1;
}

int LuaScript::Playerpref_SetFloat(lua_State* L)
{
    lua_gettop(L) <= 2 ? lua_pushboolean(L, PlayerPref::SaveVariable<float>(luaL_checkstring(L, 1), (float)lua_tonumber(L, 2)))
                       : lua_pushboolean(L, PlayerPref::SaveVariable<float>(luaL_checkstring(L, 1), (float)lua_tonumber(L, 2), luaL_checkstring(L, 3)));
    return 1;
}

int LuaScript::Playerpref_GetBool(lua_State* L)
{
    lua_gettop(L) == 1 ? lua_pushboolean(L, PlayerPref::GetVariable<bool>(luaL_checkstring(L, 1))) : lua_pushboolean(L, PlayerPref::GetVariable<bool>(luaL_checkstring(L, 1), luaL_checkstring(L, 2)));
    return 1;
}

int LuaScript::Playerpref_SetBool(lua_State* L)
{
    lua_gettop(L) <= 2 ? lua_pushboolean(L, PlayerPref::SaveVariable<bool>(luaL_checkstring(L, 1), lua_toboolean(L, 2)))
                       : lua_pushboolean(L, PlayerPref::SaveVariable<bool>(luaL_checkstring(L, 1), lua_toboolean(L, 2), luaL_checkstring(L, 3)));
    return 1;
}

int LuaScript::Playerpref_GetString(lua_State* L)
{
    lua_gettop(L) == 1 ? lua_pushstring(L, PlayerPref::GetVariable<std::string>(luaL_checkstring(L, 1)).c_str())
                       : lua_pushstring(L, PlayerPref::GetVariable<std::string>(luaL_checkstring(L, 1), luaL_checkstring(L, 2)).c_str());
    return 1;
}

int LuaScript::Playerpref_SetString(lua_State* L)
{
    lua_gettop(L) <= 2 ? lua_pushboolean(L, PlayerPref::SaveVariable<std::string>(luaL_checkstring(L, 1), luaL_checkstring(L, 2)))
                       : lua_pushboolean(L, PlayerPref::SaveVariable<std::string>(luaL_checkstring(L, 1), luaL_checkstring(L, 2), luaL_checkstring(L, 3)));
    return 1;
}

int LuaScript::Playerpref_GetVector3(lua_State* L)
{
    lua_gettop(L) == 1 ? pushVector3(L, PlayerPref::GetVariable<Vector3>(luaL_checkstring(L, 1))) : pushVector3(L, PlayerPref::GetVariable<Vector3>(luaL_checkstring(L, 1), luaL_checkstring(L, 2)));
    return 1;
}

int LuaScript::Playerpref_SetVector3(lua_State* L)
{
    lua_gettop(L) <= 2 ? lua_pushboolean(L, PlayerPref::SaveVariable<Vector3>(luaL_checkstring(L, 1), *checkVector3(L, 2)))
                       : lua_pushboolean(L, PlayerPref::SaveVariable<Vector3>(luaL_checkstring(L, 1), *checkVector3(L, 2), luaL_checkstring(L, 3)));
    return 1;
}

// https://stackoverflow.com/questions/30887829/reading-lua-table-from-c
// https://www.gamedev.net/forums/topic/462367-passing-arrays-between-lua-cc/
int LuaScript::Playerpref_GetIntegerArray(lua_State* L)
{
    std::vector<int> variables =
        lua_gettop(L) == 1 ? PlayerPref::GetVariable<std::vector<int>>(luaL_checkstring(L, 1)) : PlayerPref::GetVariable<std::vector<int>>(luaL_checkstring(L, 1), luaL_checkstring(L, 2));
    lua_createtable(L, (int)variables.size(), 0);
    int newTable = lua_gettop(L);
    for (unsigned i = 1; i <= variables.size(); ++i)
    {
        lua_pushinteger(L, variables[i - 1]);
        lua_rawseti(L, newTable, i);
    }
    return 1;
}

int LuaScript::Playerpref_SetIntegerArray(lua_State* L)
{
    std::vector<int> elems;
    int args = lua_gettop(L);
    std::string name{luaL_checkstring(L, 1)};

    int type = lua_type(L, 2);
    if (type != LUA_TTABLE)
    {
        std::cout << "Playerpref_SetIntegerArray : ERROR - PARAMETER MUST BE AN ARRAY" << std::endl;
        return 0;
    }

    int tableSize = (int)lua_rawlen(L, 2);  /// get size of table
    for (int i = 1; i <= tableSize; i++)
    {
        lua_rawgeti(L, 2, i);
        int Top = lua_gettop(L);
        elems.push_back((int)lua_tointeger(L, Top));
    }

    args <= 2 ? lua_pushboolean(L, PlayerPref::SaveVariable<std::vector<int>>(name.c_str(), elems))
              : lua_pushboolean(L, PlayerPref::SaveVariable<std::vector<int>>(name.c_str(), elems, luaL_checkstring(L, 3)));
    return 1;
}

int LuaScript::Playerpref_GetFloatArray(lua_State* L)
{
    std::vector<float> variables =
        lua_gettop(L) == 1 ? PlayerPref::GetVariable<std::vector<float>>(luaL_checkstring(L, 1)) : PlayerPref::GetVariable<std::vector<float>>(luaL_checkstring(L, 1), luaL_checkstring(L, 2));
    lua_createtable(L, (int)variables.size(), 0);
    int newTable = lua_gettop(L);
    for (unsigned i = 1; i <= variables.size(); ++i)
    {
        lua_pushnumber(L, variables[i - 1]);
        lua_rawseti(L, newTable, i);
    }
    return 1;
}

int LuaScript::Playerpref_SetFloatArray(lua_State* L)
{
    std::vector<float> elems;
    int args = lua_gettop(L);
    std::string name{luaL_checkstring(L, 1)};

    int type = lua_type(L, 2);
    if (type != LUA_TTABLE)
    {
        std::cout << "Playerpref_SetFloatArray : ERROR - PARAMETER MUST BE AN ARRAY" << std::endl;
        return 0;
    }

    int tableSize = (int)lua_rawlen(L, 2);  /// get size of table
    for (int i = 1; i <= tableSize; i++)
    {
        lua_rawgeti(L, 2, i);
        int Top = lua_gettop(L);
        elems.push_back((float)lua_tonumber(L, Top));
    }

    args <= 2 ? lua_pushboolean(L, PlayerPref::SaveVariable<std::vector<float>>(name.c_str(), elems))
              : lua_pushboolean(L, PlayerPref::SaveVariable<std::vector<float>>(name.c_str(), elems, luaL_checkstring(L, 3)));
    return 1;
}

int LuaScript::Playerpref_GetBoolArray(lua_State* L)
{
    std::vector<bool> variables =
        lua_gettop(L) == 1 ? PlayerPref::GetVariable<std::vector<bool>>(luaL_checkstring(L, 1)) : PlayerPref::GetVariable<std::vector<bool>>(luaL_checkstring(L, 1), luaL_checkstring(L, 2));
    lua_createtable(L, (int)variables.size(), 0);
    int newTable = lua_gettop(L);
    for (unsigned i = 1; i <= variables.size(); ++i)
    {
        lua_pushboolean(L, variables[i - 1]);
        lua_rawseti(L, newTable, i);
    }
    return 1;
}

int LuaScript::Playerpref_SetBoolArray(lua_State* L)
{
    std::vector<bool> elems;
    int args = lua_gettop(L);
    std::string name{luaL_checkstring(L, 1)};

    int type = lua_type(L, 2);
    if (type != LUA_TTABLE)
    {
        std::cout << "Playerpref_SetBoolArray : ERROR - PARAMETER MUST BE AN ARRAY" << std::endl;
        return 0;
    }

    int tableSize = (int)lua_rawlen(L, 2);  /// get size of table
    for (int i = 1; i <= tableSize; i++)
    {
        lua_rawgeti(L, 2, i);
        int Top = lua_gettop(L);
        elems.push_back(lua_toboolean(L, Top));
    }

    args <= 2 ? lua_pushboolean(L, PlayerPref::SaveVariable<std::vector<bool>>(name.c_str(), elems))
              : lua_pushboolean(L, PlayerPref::SaveVariable<std::vector<bool>>(name.c_str(), elems, luaL_checkstring(L, 3)));
    return 1;
}

int LuaScript::Playerpref_GetStringArray(lua_State* L)
{
    std::vector<std::string> variables = lua_gettop(L) == 1 ? PlayerPref::GetVariable<std::vector<std::string>>(luaL_checkstring(L, 1))
                                                            : PlayerPref::GetVariable<std::vector<std::string>>(luaL_checkstring(L, 1), luaL_checkstring(L, 2));
    lua_createtable(L, (int)variables.size(), 0);
    int newTable = lua_gettop(L);
    for (unsigned i = 1; i <= variables.size(); ++i)
    {
        lua_pushstring(L, variables[i - 1].c_str());
        lua_rawseti(L, newTable, i);
    }
    return 1;
}

int LuaScript::Playerpref_SetStringArray(lua_State* L)
{
    std::vector<std::string> elems;
    int args = lua_gettop(L);
    std::string name{luaL_checkstring(L, 1)};

    int type = lua_type(L, 2);
    if (type != LUA_TTABLE)
    {
        std::cout << "Playerpref_SetStringArray : ERROR - PARAMETER MUST BE AN ARRAY" << std::endl;
        return 0;
    }

    int tableSize = (int)lua_rawlen(L, 2);  /// get size of table
    for (int i = 1; i <= tableSize; i++)
    {
        lua_rawgeti(L, 2, i);
        int Top = lua_gettop(L);
        elems.push_back(std::string{luaL_checkstring(L, Top)});
    }

    args <= 2 ? lua_pushboolean(L, PlayerPref::SaveVariable<std::vector<std::string>>(name.c_str(), elems))
              : lua_pushboolean(L, PlayerPref::SaveVariable<std::vector<std::string>>(name.c_str(), elems, luaL_checkstring(L, 3)));
    return 1;
}

int LuaScript::Playerpref_GetVector3Array(lua_State* L)
{
    std::vector<Vector3> variables =
        lua_gettop(L) == 1 ? PlayerPref::GetVariable<std::vector<Vector3>>(luaL_checkstring(L, 1)) : PlayerPref::GetVariable<std::vector<Vector3>>(luaL_checkstring(L, 1), luaL_checkstring(L, 2));
    lua_createtable(L, (int)variables.size(), 0);
    int newTable = lua_gettop(L);
    for (unsigned i = 1; i <= variables.size(); ++i)
    {
        pushVector3(L, variables[i - 1]);
        lua_rawseti(L, newTable, i);
    }
    return 1;
}

int LuaScript::Playerpref_SetVector3Array(lua_State* L)
{
    std::vector<Vector3> elems;
    int args = lua_gettop(L);
    std::string name{luaL_checkstring(L, 1)};

    int type = lua_type(L, 2);
    if (type != LUA_TTABLE)
    {
        std::cout << "Playerpref_SetVector3Array : ERROR - PARAMETER MUST BE AN ARRAY" << std::endl;
        return 0;
    }

    int tableSize = (int)lua_rawlen(L, 2);  /// get size of table
    for (int i = 1; i <= tableSize; i++)
    {
        lua_rawgeti(L, 2, i);
        int Top = lua_gettop(L);
        elems.push_back(*checkVector3(L, Top));
    }

    args <= 2 ? lua_pushboolean(L, PlayerPref::SaveVariable<std::vector<Vector3>>(name.c_str(), elems))
              : lua_pushboolean(L, PlayerPref::SaveVariable<std::vector<Vector3>>(name.c_str(), elems, luaL_checkstring(L, 3)));
    return 1;
}

// ============================================================================
// DEBUG
// ============================================================================
int LuaScript::debug_DrawLine(lua_State* L)
{
    Vector3* v1 = checkVector3(L, 1);
    Vector3* v2 = checkVector3(L, 2);
    Color4* c = checkColor(L, 3, false);

    if (c)
        Renderer::Instance().DrawLine(*v1, *v2, *c);
    else
        Renderer::Instance().DrawLine(*v1, *v2);
    return 0;
}

// ============================================================================
// INPUT
// ============================================================================

int LuaScript::lua_down(lua_State* L)
{
    short key = static_cast<short>(lua_tonumber(L, 1));
    lua_pushboolean(L, Input::Instance().GetKeyDown(key));
    return 1;
}

int LuaScript::lua_pressed(lua_State* L)
{
    short key = static_cast<short>(lua_tonumber(L, 1));
    lua_pushboolean(L, Input::Instance().GetKeyPressed(key));
    return 1;
}

int LuaScript::lua_up(lua_State* L)
{
    short key = static_cast<short>(lua_tonumber(L, 1));
    lua_pushboolean(L, Input::Instance().GetKeyUp(key));
    return 1;
}

int LuaScript::lua_gamepad_pressed(lua_State* L)
{
    WORD key = static_cast<WORD>(lua_tonumber(L, 1));
    lua_pushboolean(L, Input::Instance().GetButtonPressed(key));
    return 1;
}

int LuaScript::lua_gamepad_down(lua_State* L)
{
    WORD key = static_cast<WORD>(lua_tonumber(L, 1));
    lua_pushboolean(L, Input::Instance().GetButtonDown(key));
    return 1;
}

int LuaScript::lua_gamepad_up(lua_State* L)
{
    WORD key = static_cast<WORD>(lua_tonumber(L, 1));
    lua_pushboolean(L, Input::Instance().GetButtonUp(key));
    return 1;
}

int LuaScript::lua_controllerMap_pressed(lua_State* L)
{
    std::string action{luaL_checkstring(L, 1)};
    lua_pushboolean(L, Input::Instance().GetControllerInputPressed(action));
    return 1;
}

int LuaScript::lua_controllerMap_down(lua_State* L)
{
    std::string action{luaL_checkstring(L, 1)};
    lua_pushboolean(L, Input::Instance().GetControllerInputDown(action));
    return 1;
}

int LuaScript::lua_controllerMap_up(lua_State* L)
{
    std::string action{luaL_checkstring(L, 1)};
    lua_pushboolean(L, Input::Instance().GetControllerInputUp(action));
    return 1;
}

int LuaScript::lua_controllerMap_axis(lua_State* L)
{
    std::string action{luaL_checkstring(L, 1)};
    lua_pushnumber(L, Input::Instance().GetControllerInputAxis(action));
    return 1;
}

int LuaScript::lua_mousePosition(lua_State* L)
{
    Vector3 mousePos{Input::Instance().GetMousePosition(), 0};
    return pushVector3(L, mousePos);
}

int LuaScript::lua_mouseDeltaPosition(lua_State* L)
{
    Vector3 mousDeltaPos{Input::Instance().GetMouseDelta(), 0};
    return pushVector3(L, mousDeltaPos);
}

int LuaScript::lua_displayMouse(lua_State* L)
{
    bool tmp = lua_toboolean(L, 1);

    auto& timer = Application::Instance().GetGameTimer();
    if (!timer.IsEditorPaused() && !timer.IsPlayModePaused())
    {
        if (tmp)
            while (ShowCursor(tmp) < 0)
                ;
        else
            while (ShowCursor(tmp) > 0)
                ;
    }

    return 0;
}

int LuaScript::lua_setMouseIcon(lua_State* L)
{
    std::string cursorPath{luaL_checkstring(L, 1)};
    std::string fullPath{RESOURCEMANAGER.s_ResourcePathTexture.string() + cursorPath};

    HCURSOR cursor = LoadCursorFromFile(fullPath.c_str());  //.cur or .ani
    SetCursor(cursor);
    SetClassLongPtr(Application::Instance().GetWindowHandle(), GCLP_HCURSOR, reinterpret_cast<LONG_PTR>(cursor));

    return 0;
}

int LuaScript::lua_lockMouseToCenter(lua_State* L)
{
    bool lockMouse = lua_toboolean(L, 1);
    Editor::Instance().s_lockMousePosition = lockMouse;
    return 0;
}

int LuaScript::lua_changeControllerInput(lua_State* L)
{
    int mode = int(lua_tointeger(L, 1));
    Input::Instance().SetControllerMode(mode == 0 ? ControllerMapping::CONTROLLERTYPE::KEYBOARD : ControllerMapping::CONTROLLERTYPE::CONTROLLER);
    return 0;
}

int LuaScript::lua_getControllerInput(lua_State* L)
{
    lua_pushinteger(L, Input::Instance().GetControllerMode());
    return 1;
}

int LuaScript::lua_ControllerVibrateLeft(lua_State* L)
{
    float rumbleStrength = static_cast<float>(lua_tonumber(L, 1));
    Input::Instance().Rumble(0.0f, rumbleStrength);
    return 0;
}

int LuaScript::lua_ControllerVibrateRight(lua_State* L)
{
    float rumbleStrength = static_cast<float>(lua_tonumber(L, 1));
    Input::Instance().Rumble(rumbleStrength, 0.0f);
    return 0;
}

int LuaScript::lua_ControllerVibrateBoth(lua_State* L)
{
    float rumbleStrength = static_cast<float>(lua_tonumber(L, 1));
    Input::Instance().Rumble(rumbleStrength, rumbleStrength);
    return 0;
}

int LuaScript::lua_SetMouseWrap(lua_State* L)
{
    Input::Instance().ToggleMouseWrap(lua_toboolean(L, 1));
    return 0;
}

int LuaScript::lua_callFunction(lua_State* L)
{
    LuaScript** script = checkluaScript(L, 1);
    std::string functionName{luaL_checkstring(L, 2)};

    lua_getglobal((*script)->L, functionName.c_str());
    if (lua_type((*script)->L, -1) == LUA_TFUNCTION)
        lua_pcall((*script)->L, 0, 0, 0);
    else
        std::cout << "Error! Function not found!" << std::endl;

    return 1;
}

int LuaScript::lua_EnableScript(lua_State* L)
{
    LuaScript** lua = checkluaScript(L, 1);
    (*lua)->SetActive(lua_toboolean(L, 2));
    return 0;
}

// ============================================================================
// LAYER
// ============================================================================
int LuaScript::layer_metatable(lua_State* L)
{
    // New version
    luaL_newmetatable(L, "Layer");
    luaL_setfuncs(L, layerlib, 0);
    lua_pushvalue(L, -1);
    lua_setfield(L, -2, "__index");
    return 1;

    // Old version
    // luaL_newmetatable(L, "Layer");
    // lua_pushstring(L, "__index");
    // lua_pushvalue(L, -2);
    // lua_settable(L, -3);
    // luaL_setfuncs(L, layerlib, 0);
    // return 1;
}

int LuaScript::layer_currentLayer(lua_State* L)
{
    if (Application::InstancePtr())
    {
        Layer* ly = Application::Instance().GetCurrentScene()->GetLayers().front();
        setlayer(L, (void*)ly);
        return 1;
    }
    return 0;
}

int LuaScript::layer_getLayer(lua_State* L)
{
    std::string layerName{luaL_checkstring(L, 1)};

    if (Application::InstancePtr())
    {
        for (auto& c : Application::Instance().GetCurrentScene()->GetLayers())
            if (c->GetName() == layerName)
            {
                setlayer(L, (void*)c);
                return 1;
            }
    }
    return 0;
}

int LuaScript::layer_name(lua_State* L)
{
    Layer** ly = checklayer(L);
    lua_pushstring(L, (*ly)->GetName().c_str());
    return 1;
}

int LuaScript::layer_createObject(lua_State* L)
{
    Layer** ly = checklayer(L);
    std::string name = luaL_checkstring(L, 2);
    GameObject* go = (*ly)->CreateObject(name);
    pushGameObject(L, (void*)go);
    return 1;
}

Layer** LuaScript::checklayer(lua_State* L, int index, bool assert)
{
    return check<Layer**>("Layer", L, index, assert);
}

void LuaScript::setlayer(lua_State* L, void* ly)
{
    Layer** tmp = (Layer**)lua_newuserdata(L, sizeof(Layer*));
    *tmp = (Layer*)ly;
    luaL_getmetatable(L, "Layer");
    lua_setmetatable(L, -2);
}

int LuaScript::layer_find(lua_State* L)
{
    Layer** ly = checklayer(L);
    std::string name = luaL_checkstring(L, 2);
    GameObject* go = (*ly)->GetObjectByName(name);
    if (!go)
        lua_pushnil(L);
    else
        pushGameObject(L, go);
    return 1;
}

int LuaScript::layer_getObjects(lua_State* L)
{
    Layer** ly = checklayer(L);
    auto Gos = (*ly)->GetObjectsList();
    auto GoIT = Gos.begin();
    lua_createtable(L, (int)Gos.size(), 0);
    int newTable = lua_gettop(L);
    for (unsigned i = 1; i <= Gos.size(); ++i)
    {
        GameObject** tmp = (GameObject**)lua_newuserdata(L, sizeof(GameObject*));
        *tmp = *GoIT;
        luaL_getmetatable(L, "GameObject");
        lua_setmetatable(L, -2);
        lua_rawseti(L, newTable, i);
        ++GoIT;
    }
    return 1;
}

int LuaScript::layer_getObjectsByName(lua_State* L)
{
    Layer** ly = checklayer(L);
    std::string name = luaL_checkstring(L, 2);
    auto Gos = (*ly)->GetObjectsByName(name);
    auto GoIT = Gos.begin();

    lua_createtable(L, (int)Gos.size(), 0);
    int newTable = lua_gettop(L);
    for (unsigned i = 1; i <= Gos.size(); ++i)
    {
        GameObject** tmp = (GameObject**)lua_newuserdata(L, sizeof(GameObject*));
        *tmp = *GoIT;
        luaL_getmetatable(L, "GameObject");
        lua_setmetatable(L, -2);
        lua_rawseti(L, newTable, i);
        ++GoIT;
    }
    return 1;
}

int LuaScript::layer_getObjectsBySubName(lua_State* L)
{
    Layer** ly = checklayer(L);
    std::string name = luaL_checkstring(L, 2);
    auto Gos = (*ly)->GetObjectByStrMatch(name);
    auto GoIT = Gos.begin();

    lua_createtable(L, (int)Gos.size(), 0);
    int newTable = lua_gettop(L);
    for (unsigned i = 1; i <= Gos.size(); ++i)
    {
        GameObject** tmp = (GameObject**)lua_newuserdata(L, sizeof(GameObject*));
        *tmp = *GoIT;
        luaL_getmetatable(L, "GameObject");
        lua_setmetatable(L, -2);
        lua_rawseti(L, newTable, i);
        ++GoIT;
    }
    return 1;
}

int LuaScript::layer_getObjectByID(lua_State* L)
{
    Layer** ly = checklayer(L);
    unsigned id = static_cast<unsigned>(luaL_checkinteger(L, 2));
    auto GO = (*ly)->GetObjectById(id);
    if (GO)
        pushGameObject(L, GO);
    else
        lua_pushnil(L);

    return 1;
}

int LuaScript::layer_getObjectsByTag(lua_State* L)
{
    Layer** ly = checklayer(L);
    std::string tag = luaL_checkstring(L, 2);

    auto Gos = (*ly)->GetObjectListByTag(tag);
    auto GoIT = Gos.begin();

    lua_createtable(L, (int)Gos.size(), 0);
    int newTable = lua_gettop(L);
    for (unsigned i = 1; i <= Gos.size(); ++i)
    {
        GameObject** tmp = (GameObject**)lua_newuserdata(L, sizeof(GameObject*));
        *tmp = *GoIT;
        luaL_getmetatable(L, "GameObject");
        lua_setmetatable(L, -2);
        lua_rawseti(L, newTable, i);
        ++GoIT;
    }
    return 1;
}

int LuaScript::create_Layer(lua_State* L)
{
    std::string layerName{luaL_checkstring(L, 1)};

    if (Application::InstancePtr())
    {
        auto ly = Application::Instance().GetCurrentScene()->CreateLayer(layerName);
        setlayer(L, (void*)ly);
        return 1;
    }
    return 0;
}

int LuaScript::layer_DestroyAllObjectsWithTag(lua_State* L)
{
    Layer** ly = checklayer(L);
    std::string tag{luaL_checkstring(L, 2)};
    auto Gos = (*ly)->GetObjectListByTag(tag);
    for (auto& c : Gos)
        c->Destroy();
    return 0;
}

// ============================================================================
// VECTOR 3
// ============================================================================
int LuaScript::vector3_metatable(lua_State* L)
{
    luaL_newmetatable(L, "vector3");
    luaL_setfuncs(L, vector3lib, 0);
    lua_pushvalue(L, -1);
    lua_setfield(L, -2, "__index");
    return 1;
}

Vector3* LuaScript::checkVector3(lua_State* L, int index, bool assert)
{
    return check<Vector3*>("vector3", L, index, assert);
}

int LuaScript::pushVector3(lua_State* L, Vector3 input)
{
    return pushCustomType<Vector3>("vector3", L, input);
}

int LuaScript::vector3_new(lua_State* L)
{
    int params = lua_gettop(L);
    if (params == 3) return pushVector3(L, Vector3{static_cast<float>(luaL_checknumber(L, 1)), static_cast<float>(luaL_checknumber(L, 2)), static_cast<float>(luaL_checknumber(L, 3))});

    return pushVector3(L, Vector3{0, 0, 0});
}

int LuaScript::vector3_index(lua_State* L)
{
    Vector3* vec3 = checkVector3(L, 1);
    std::string index = luaL_checkstring(L, 2);

    if (index == "x")
        lua_pushnumber(L, vec3->x);
    else if (index == "y")
        lua_pushnumber(L, vec3->y);
    else if (index == "z")
        lua_pushnumber(L, vec3->z);
    else
        return 0;
    return 1;
}

int LuaScript::vector3_newindex(lua_State* L)
{
    Vector3* vec3 = checkVector3(L, 1);
    std::string index = luaL_checkstring(L, 2);
    float value = static_cast<float>(luaL_checknumber(L, 3));
    if (index == "x")
        vec3->x = value;
    else if (index == "y")
        vec3->y = value;
    else if (index == "z")
        vec3->z = value;

    return 0;
}

int LuaScript::vector3_set(lua_State* L)
{
    Vector3* vec3 = checkVector3(L, 1);
    *vec3 = Vector3{static_cast<float>(luaL_checknumber(L, 2)), static_cast<float>(luaL_checknumber(L, 3)), static_cast<float>(luaL_checknumber(L, 4))};

    return pushVector3(L, *vec3);
}

int LuaScript::vector3_add(lua_State* L)
{
    Vector3* lhs = checkVector3(L, 1);
    Vector3* rhs = checkVector3(L, 2);
    return pushVector3(L, *lhs + *rhs);
}

int LuaScript::vector3_sub(lua_State* L)
{
    Vector3* lhs = checkVector3(L, 1);
    Vector3* rhs = checkVector3(L, 2);
    return pushVector3(L, *lhs - *rhs);
}

int LuaScript::vector3_mul(lua_State* L)
{
    Vector3* lhs = checkVector3(L, 1);
    float rhs = static_cast<float>(luaL_checknumber(L, 2));
    return pushVector3(L, *lhs * rhs);
}

int LuaScript::vector3_div(lua_State* L)
{
    Vector3* lhs = checkVector3(L, 1);
    float rhs = static_cast<float>(luaL_checknumber(L, 2));
    return pushVector3(L, *lhs / rhs);
}

int LuaScript::vector3_unm(lua_State* L)
{
    Vector3* lhs = checkVector3(L, 1);
    return pushVector3(L, -*lhs);
}

int LuaScript::vector3_eq(lua_State* L)
{
    Vector3* lhs = checkVector3(L, 1);
    Vector3* rhs = checkVector3(L, 2);
    lua_pushboolean(L, *lhs == *rhs);
    return 1;
}

int LuaScript::vector3_Normalize(lua_State* L)
{
    Vector3* tmp = checkVector3(L, 1);
    pushVector3(L, tmp->Normalize());
    return 1;
}

int LuaScript::vector3_Normalized(lua_State* L)
{
    Vector3* tmp = checkVector3(L, 1);
    return pushVector3(L, tmp->Normalized());
}

int LuaScript::vector3_Projection(lua_State* L)
{
    Vector3* lhs = checkVector3(L, 1);
    Vector3* rhs = checkVector3(L, 2);
    return pushVector3(L, lhs->Projection(*rhs));
}

int LuaScript::vector3_Cross(lua_State* L)
{
    Vector3* lhs = checkVector3(L, 1);
    Vector3* rhs = checkVector3(L, 2);
    return pushVector3(L, lhs->Cross(*rhs));
}

int LuaScript::vector3_Dot(lua_State* L)
{
    Vector3* lhs = checkVector3(L, 1);
    Vector3* rhs = checkVector3(L, 2);
    lua_pushnumber(L, lhs->Dot(*rhs));
    return 1;
}

int LuaScript::vector3_Length(lua_State* L)
{
    Vector3* tmp = checkVector3(L, 1);
    lua_pushnumber(L, tmp->Length());
    return 1;
}

int LuaScript::vector3_SquareLength(lua_State* L)
{
    Vector3* tmp = checkVector3(L, 1);
    lua_pushnumber(L, tmp->SquareLength());
    return 1;
}

int LuaScript::vector3_DistanceTo(lua_State* L)
{
    Vector3* lhs = checkVector3(L, 1);
    Vector3* rhs = checkVector3(L, 2);
    lua_pushnumber(L, lhs->Distance(*rhs));
    return 1;
}

int LuaScript::vector3_SquaredDistanceTo(lua_State* L)
{
    Vector3* lhs = checkVector3(L, 1);
    Vector3* rhs = checkVector3(L, 2);
    lua_pushnumber(L, lhs->SquareDistance(*rhs));
    return 1;
}

int LuaScript::vector3_One(lua_State* L)
{
    return pushVector3(L, Vector3{1, 1, 1});
}

int LuaScript::vector3_Rotate(lua_State* L)
{
    Vector3* tmp = checkVector3(L, 1);
    std::string index = luaL_checkstring(L, 2);
    float value = static_cast<float>(luaL_checknumber(L, 3));

    if (index == "x")
        return pushVector3(L, tmp->RotateX(value));
    else if (index == "y")
        return pushVector3(L, tmp->RotateY(value));
    else if (index == "z")
        return pushVector3(L, tmp->RotateZ(value));
    return 0;
}

int LuaScript::vector3_AngleFromWorldAxis(lua_State* L)
{
    Vector3* vec1 = checkVector3(L, 1);

    float angleInRadians = std::atan2(vec1->x, vec1->z);
    float result = (angleInRadians / PI) * 180.0f;

    lua_pushnumber(L, 1 * result);
    return 1;
}

int LuaScript::vector3_X(lua_State* L)
{
    Vector3* tmp = checkVector3(L, 1);
    lua_pushnumber(L, tmp->x);
    return 1;
}

int LuaScript::vector3_Y(lua_State* L)
{
    Vector3* tmp = checkVector3(L, 1);
    lua_pushnumber(L, tmp->y);
    return 1;
}

int LuaScript::vector3_Z(lua_State* L)
{
    Vector3* tmp = checkVector3(L, 1);
    lua_pushnumber(L, tmp->z);
    return 1;
}

int LuaScript::vector3_polarAngle(lua_State* L)
{
    Vector3* tmp = checkVector3(L, 1);
    pushVector3(L, Vector3{tmp->PolarAngles(), 0});
    return 1;
}

// ============================================================================
// COLOR
// ============================================================================
int LuaScript::color_metatable(lua_State* L)
{
    luaL_newmetatable(L, "color");
    luaL_setfuncs(L, colorlib, 0);
    lua_pushvalue(L, -1);
    lua_setfield(L, -2, "__index");
    return 1;
}

Vector4* LuaScript::checkColor(lua_State* L, int index, bool assert)
{
    return check<Vector4*>("color", L, index, assert);
}

int LuaScript::pushColor(lua_State* L, Vector4 input)
{
    return pushCustomType<Vector4>("color", L, input);
}

Vector4 LuaScript::clampColor(Vector4 input)
{
    // for (int i = 0; i < 4; ++i)
    //  input[i] = input[i] > 1.0 ? 1.0f : input[i] < 0.0f ? 0.0f : input[i];

    return input;
}

int LuaScript::color_new(lua_State* L)
{
    int params = lua_gettop(L);
    if (params == 4)
        return pushColor(L, clampColor(Vector4{static_cast<float>(luaL_checknumber(L, 1)), static_cast<float>(luaL_checknumber(L, 2)), static_cast<float>(luaL_checknumber(L, 3)),
                                               static_cast<float>(luaL_checknumber(L, 4))}));

    return pushColor(L, Vector4{1, 1, 1, 1});
}

int LuaScript::color_index(lua_State* L)
{
    Vector4* c = checkColor(L, 1);
    std::string index = luaL_checkstring(L, 2);
    if (index == "r")
        lua_pushnumber(L, c->x);
    else if (index == "g")
        lua_pushnumber(L, c->y);
    else if (index == "b")
        lua_pushnumber(L, c->z);
    else if (index == "a")
        lua_pushnumber(L, c->w);
    else
        return 0;
    return 1;
}

int LuaScript::color_newindex(lua_State* L)
{
    Vector4* c = checkColor(L, 1);
    std::string index = luaL_checkstring(L, 2);
    float value = static_cast<float>(luaL_checknumber(L, 3));
    if (index == "r")
        c->x = value;
    else if (index == "g")
        c->y = value;
    else if (index == "b")
        c->z = value;
    else if (index == "a")
        c->w = value;
    *c = clampColor(*c);
    return 0;
}

int LuaScript::color_set(lua_State* L)
{
    Vector4* c = checkColor(L, 1);
    *c = clampColor(Vector4{
        static_cast<float>(luaL_checknumber(L, 2)),
        static_cast<float>(luaL_checknumber(L, 3)),
        static_cast<float>(luaL_checknumber(L, 4)),
        static_cast<float>(luaL_checknumber(L, 5)),
    });

    return pushColor(L, *c);
}

int LuaScript::color_add(lua_State* L)
{
    Vector4* lhs = checkColor(L, 1);
    Vector4* rhs = checkColor(L, 2);
    return pushColor(L, clampColor(*lhs + *rhs));
}

int LuaScript::color_sub(lua_State* L)
{
    Vector4* lhs = checkColor(L, 1);
    Vector4* rhs = checkColor(L, 2);
    return pushColor(L, clampColor(*lhs - *rhs));
}

int LuaScript::color_mul(lua_State* L)
{
    Vector4* lhs = checkColor(L, 1);
    float rhs = static_cast<float>(luaL_checknumber(L, 2));
    return pushColor(L, clampColor(*lhs * rhs));
}

int LuaScript::color_div(lua_State* L)
{
    Vector4* lhs = checkColor(L, 1);
    float rhs = static_cast<float>(luaL_checknumber(L, 2));
    return pushColor(L, clampColor(*lhs / rhs));
}

int LuaScript::color_eq(lua_State* L)
{
    Vector4* lhs = checkColor(L, 1);
    Vector4* rhs = checkColor(L, 2);
    lua_pushboolean(L, *lhs == *rhs);
    return 1;
}

int LuaScript::color_R(lua_State* L)
{
    Vector4* tmp = checkColor(L, 1);
    lua_pushnumber(L, tmp->r);
    return 1;
}

int LuaScript::color_G(lua_State* L)
{
    Vector4* tmp = checkColor(L, 1);
    lua_pushnumber(L, tmp->g);
    return 1;
}

int LuaScript::color_B(lua_State* L)
{
    Vector4* tmp = checkColor(L, 1);
    lua_pushnumber(L, tmp->b);
    return 1;
}

int LuaScript::color_A(lua_State* L)
{
    Vector4* tmp = checkColor(L, 1);
    lua_pushnumber(L, tmp->a);
    return 1;
}

// ============================================================================
// MATH
// ============================================================================
int LuaScript::math_lerp(lua_State* L)
{
    float s = static_cast<float>(luaL_checknumber(L, 1));
    float f = static_cast<float>(luaL_checknumber(L, 2));
    float rate = static_cast<float>(luaL_checknumber(L, 3));
    lua_pushnumber(L, Math::Lerp(s, f, rate));
    return 1;
}

int LuaScript::math_Vector3lerp(lua_State* L)
{
    Vector3 s = *checkVector3(L, 1);
    Vector3 f = *checkVector3(L, 2);
    float rate = static_cast<float>(luaL_checknumber(L, 3));
    pushVector3(L, Math::Lerp(s, f, rate));
    return 1;
}

int LuaScript::math_randomRange(lua_State* L)
{
    lua_pushnumber(L, Math::RandomRange(static_cast<float>(luaL_checknumber(L, 1)), static_cast<float>(luaL_checknumber(L, 2))));
    return 1;
}

int LuaScript::math_randomRangeInt(lua_State* L)
{
    lua_pushinteger(L, Math::RandomRange(static_cast<int>(luaL_checkinteger(L, 1)), static_cast<int>(luaL_checkinteger(L, 2))));
    return 1;
}

int LuaScript::math_toRad(lua_State* L)
{
    lua_pushnumber(L, Math::ToRadians(static_cast<float>(luaL_checknumber(L, 1))));
    return 1;
}

int LuaScript::math_toDeg(lua_State* L)
{
    lua_pushnumber(L, Math::ToDegrees(static_cast<float>(luaL_checknumber(L, 1))));
    return 1;
}

int LuaScript::math_pow(lua_State* L)
{
    lua_pushnumber(L, pow(luaL_checknumber(L, 1), luaL_checknumber(L, 2)));
    return 1;
}

int LuaScript::math_cos(lua_State* L)
{
    lua_pushnumber(L, cos(luaL_checknumber(L, 1)));
    return 1;
}

int LuaScript::math_sin(lua_State* L)
{
    lua_pushnumber(L, sin(luaL_checknumber(L, 1)));
    return 1;
}

int LuaScript::math_tan(lua_State* L)
{
    lua_pushnumber(L, tan(luaL_checknumber(L, 1)));
    return 1;
}

int LuaScript::math_acos(lua_State* L)
{
    lua_pushnumber(L, acos(luaL_checknumber(L, 1)));
    return 1;
}

int LuaScript::math_asin(lua_State* L)
{
    lua_pushnumber(L, asin(luaL_checknumber(L, 1)));
    return 1;
}

int LuaScript::math_atan(lua_State* L)
{
    lua_pushnumber(L, atan(luaL_checknumber(L, 1)));
    return 1;
}

int LuaScript::math_cosh(lua_State* L)
{
    lua_pushnumber(L, cosh(luaL_checknumber(L, 1)));
    return 1;
}

int LuaScript::math_sinh(lua_State* L)
{
    lua_pushnumber(L, sinh(luaL_checknumber(L, 1)));
    return 1;
}

int LuaScript::math_tanh(lua_State* L)
{
    lua_pushnumber(L, tanh(luaL_checknumber(L, 1)));
    return 1;
}

int LuaScript::math_acosh(lua_State* L)
{
    lua_pushnumber(L, acosh(luaL_checknumber(L, 1)));
    return 1;
}

int LuaScript::math_asinh(lua_State* L)
{
    lua_pushnumber(L, asinh(luaL_checknumber(L, 1)));
    return 1;
}

int LuaScript::math_atanh(lua_State* L)
{
    lua_pushnumber(L, atanh(luaL_checknumber(L, 1)));
    return 1;
}

int LuaScript::math_abs(lua_State* L)
{
    lua_pushnumber(L, abs(luaL_checknumber(L, 1)));
    return 1;
}

int LuaScript::math_round(lua_State* L)
{
    lua_pushnumber(L, roundf((float)luaL_checknumber(L, 1)));
    return 1;
}

int LuaScript::math_isEven(lua_State* L)
{
    int value = static_cast<int>(luaL_checknumber(L, 1));
    bool isEven = (value % 2) == 0;
    lua_pushboolean(L, isEven);
    return 1;
}

int LuaScript::math_isOdd(lua_State* L)
{
    int value = static_cast<int>(luaL_checknumber(L, 1));
    bool isOdd = (value % 2) != 0;
    lua_pushboolean(L, isOdd);
    return 1;
}

int LuaScript::math_mod(lua_State* L)
{
    int value = static_cast<int>(luaL_checknumber(L, 1));
    int mod = static_cast<int>(luaL_checknumber(L, 2));
    lua_pushnumber(L, value % mod);
    return 1;
}

// ============================================================================
// RAYCASTHIT
// ============================================================================
int LuaScript::rayCastData_metatable(lua_State* L)
{
    luaL_newmetatable(L, "rayCastData");
    luaL_setfuncs(L, rayCastDatalib, 0);
    lua_pushvalue(L, -1);
    lua_setfield(L, -2, "__index");
    return 1;
}

RaycastData_tmp* LuaScript::checkRayCastdata(lua_State* L, int index, bool assert)
{
    return check<RaycastData_tmp*>("rayCastData", L, index, assert);
}

int LuaScript::pushRayCastdata(lua_State* L, RaycastData_tmp input)
{
    return pushCustomType<RaycastData_tmp>("rayCastData", L, input);
}

int LuaScript::rayCastData_point(lua_State* L)
{
    RaycastData_tmp* c = checkRayCastdata(L, 1);
    return pushVector3(L, c->m_WorldHitPt);
}

int LuaScript::rayCastData_normal(lua_State* L)
{
    RaycastData_tmp* c = checkRayCastdata(L, 1);
    return pushVector3(L, c->m_WorldNormal);
}

int LuaScript::rayCastData_hitFrac(lua_State* L)
{
    RaycastData_tmp* c = checkRayCastdata(L, 1);
    lua_pushnumber(L, c->m_HitFrac);
    return 1;
}

int LuaScript::rayCastData_gameObject(lua_State* L)
{
    RaycastData_tmp* c = checkRayCastdata(L, 1);
    if (c->m_HitObject) return pushGameObject(L, c->m_HitObject);

    lua_pushnil(L);
    return 1;
}

int LuaScript::physics_RayCast(lua_State* L)
{
    int args = lua_gettop(L);
    Vector3* s = checkVector3(L, 1);
    Vector3* e = checkVector3(L, 2);
    float t = (float)lua_tonumber(L, 3);
    auto rayCatData = PhysicsSystem::Instance().Raycast(Ray{*s, *e, t});

    std::vector<std::string> avoidTags;
    for (int i = 4; i <= args; ++i)
        avoidTags.push_back(std::string{luaL_checkstring(L, i)});

    RaycastData_tmp* first = nullptr;
    double shortestDist = 0;

    for (unsigned i = 0; i < rayCatData.size(); ++i)
    {
        // Check tag
        bool ignore = false;
        for (const auto& c : avoidTags)
            if (rayCatData[i].m_HitObject->GetTag() == c)
            {
                ignore = true;
                break;
            }

        if (ignore) continue;

        if (!first)
        {
            first = &rayCatData[i];
            shortestDist = (rayCatData[i].m_WorldHitPt - *s).SquareLength();
        }
        else
        {
            float tmpDistance = (rayCatData[i].m_WorldHitPt - *s).SquareLength();
            if (tmpDistance < shortestDist)
            {
                shortestDist = tmpDistance;
                first = &rayCatData[i];
            }
        }
    }

    return pushRayCastdata(L, first ? *first : RaycastData_tmp{});
}

int LuaScript::physics_RayCastFirstOfName(lua_State* L)
{
    Vector3* s = checkVector3(L, 1);
    Vector3* e = checkVector3(L, 2);
    float t = (float)lua_tonumber(L, 3);
    std::string name{luaL_checkstring(L, 4)};

    auto rayCatData = PhysicsSystem::Instance().Raycast(Ray{*s, *e, t});

    RaycastData_tmp* first = nullptr;
    double shortestDist = 0;

    for (unsigned i = 0; i < rayCatData.size(); ++i)
    {
        if (rayCatData[i].m_HitObject->GetName() != name) continue;

        if (!first)
        {
            first = &rayCatData[i];
            shortestDist = (rayCatData[i].m_WorldHitPt - *s).SquareLength();
        }
        else
        {
            float tmpDistance = (rayCatData[i].m_WorldHitPt - *s).SquareLength();
            if (tmpDistance < shortestDist)
            {
                shortestDist = tmpDistance;
                first = &rayCatData[i];
            }
        }
    }

    return pushRayCastdata(L, first ? *first : RaycastData_tmp{});
}

int LuaScript::physics_RayCastAll(lua_State* L)
{
    Vector3* s = checkVector3(L, 1);
    Vector3* e = checkVector3(L, 2);
    float t = (float)lua_tonumber(L, 3);
    return pushCustomTypeArray("rayCastData", L, PhysicsSystem::Instance().Raycast(Ray{*s, *e, t}));
}

// ============================================================================
// GAMEOBJECT
// ============================================================================
int LuaScript::gameObject_metatable(lua_State* L)
{
    luaL_newmetatable(L, "GameObject");
    lua_pushstring(L, "__index");
    lua_pushvalue(L, -2);
    lua_settable(L, -3);
    luaL_setfuncs(L, gameObjectlib, 0);
    return 1;
}

GameObject** LuaScript::checkGameObject(lua_State* L, int index, bool assert)
{
    return check<GameObject**>("GameObject", L, index, assert);
}

int LuaScript::gameObject_name(lua_State* L)
{
    GameObject** go = checkGameObject(L, 1);
    lua_pushstring(L, (*go)->GetName().c_str());
    return 1;
}

int LuaScript::gameObject_Setname(lua_State* L)
{
    GameObject** go = checkGameObject(L, 1);
    std::string newName{luaL_checkstring(L, 2)};
    (*go)->SetName(newName);
    return 1;
}

int LuaScript::gameObject_destroy(lua_State* L)
{
    GameObject** go = checkGameObject(L, 1);
    if (!(*go)->GetActive())
    {
        (*go)->Destroy();
        return 0;
    }
    else
    {
        // Find all reference to this object in all other lua scripts and set them to null
        for (auto& c : Application::Instance().GetCurrentScene()->GetLayers().front()->GetObjectsList())
            if (c->GetActive())
                for (auto& components : c->GetComponentList())
                {
                    if (components)
                    {
                        LuaScript* script = dynamic_cast<LuaScript*>(components);
                        if (script)
                            for (auto& variable : script->variables)
                                if (variable.second == "GameObject" && *go == script->get<GameObject*>(variable.first))
                                {
                                    lua_pushnil(L);
                                    lua_setglobal(L, variable.first.c_str());
                                }
                    }
                }

        (*go)->Destroy();
        return 0;
    }
}

int LuaScript::gameObject_getLuaScript(lua_State* L)
{
    GameObject** go = checkGameObject(L, 1);
    std::string fileName = luaL_checkstring(L, 2);
    for (auto& c : (*go)->GetComponentList())
    {
        LuaScript* tmp = dynamic_cast<LuaScript*>(c);
        if (tmp && tmp->filename == fileName)
        {
            pushluaScript(L, tmp);
            return 1;
        }
    }

    lua_pushnil(L);
    return 1;
}

int LuaScript::gameObject_addComponent(lua_State* L)
{
    GameObject** go = checkGameObject(L, 1);
    std::string component = luaL_checkstring(L, 2);

    if (component == "Transform")
        return pushTransform(L, (*go)->AddComponent<Transform>());
    else if (component == "RigidBody")
        return pushRigidBody(L, (*go)->AddComponent<RigidbodyComponent>());
    else if (component == "AudioEmitter")
        return pushAudioEmitter(L, (*go)->AddComponent<AudioEmitter>());
    else if (component == "AudioListener")
        return pushAudioListener(L, (*go)->AddComponent<AudioListener>());
    else if (component == "MeshAnimator")
        return pushMeshAnimator(L, (*go)->AddComponent<MeshAnimator>());
    else if (component == "MeshRenderer")
        return pushMeshRenderer(L, (*go)->AddComponent<MeshRenderer>());
    else if (component == "Camera")
        return pushCamera(L, (*go)->AddComponent<Camera>());
    else if (component == "ParticleEmitter_Box")
        return pushBoxParticle(L, (*go)->AddComponent<BoxParticleEmitter>());
    else if (component == "ParticleEmitter_Circle")
        return pushCircleParticle(L, (*go)->AddComponent<CircleParticleEmitter>());
    else if (component == "PathFinding")
        return pushPathFinding(L, (*go)->AddComponent<Pathfinding>());
    else if (component == "BoxCollider")
        return pushBoxCollider(L, (*go)->AddComponent<BoxCollider>());
    else if (component == "SphereCollider")
        return pushSphereCollider(L, (*go)->AddComponent<SphereCollider>());
    else if (component == "TextRenderer")
        return pushTextRenderer(L, (*go)->AddComponent<TextRenderer>());
    else if (component == "Attractor")
        return pushAttractor(L, (*go)->AddComponent<Attractor>());
    else if (component == "DirectionalLight")
        return pushDirectionalLight(L, (*go)->AddComponent<DirectionalLight>());
    else if (component == "PointLight")
        return pushPointLight(L, (*go)->AddComponent<PointLight>());
    else if (component == "SpotLight")
        return pushSpotLight(L, (*go)->AddComponent<SpotLight>());
    else if (component == "LuaScript")
    {
        std::string scriptName = luaL_checkstring(L, 3);
        auto script = (*go)->AddComponent<LuaScript>();
        script->SetScript(scriptName);
        return pushluaScript(L, script);
    }

    return 0;
}

int LuaScript::gameObject_removeComponent(lua_State* L)
{
    GameObject** go = checkGameObject(L, 1);
    std::string component = luaL_checkstring(L, 2);

    if (component == "Transform")
        (*go)->RemoveComponent<Transform>();
    else if (component == "RigidBody")
        (*go)->RemoveComponent<RigidbodyComponent>();
    else if (component == "AudioEmitter")
        (*go)->RemoveComponent<AudioEmitter>();
    else if (component == "AudioListener")
        (*go)->RemoveComponent<AudioListener>();
    else if (component == "MeshAnimator")
        (*go)->RemoveComponent<MeshAnimator>();
    else if (component == "MeshRenderer")
        (*go)->RemoveComponent<MeshRenderer>();
    else if (component == "Camera")
        (*go)->RemoveComponent<Camera>();
    else if (component == "ParticleEmitter_Box")
        (*go)->RemoveComponent<BoxParticleEmitter>();
    else if (component == "ParticleEmitter_Circle")
        (*go)->RemoveComponent<CircleParticleEmitter>();
    else if (component == "PathFinding")
        (*go)->RemoveComponent<Pathfinding>();
    else if (component == "BoxCollider")
        (*go)->RemoveComponent<BoxCollider>();
    else if (component == "SphereCollider")
        (*go)->RemoveComponent<SphereCollider>();
    else if (component == "TextRenderer")
        (*go)->RemoveComponent<TextRenderer>();
    else if (component == "Attractor")
        (*go)->RemoveComponent<Attractor>();
    else if (component == "DirectionalLight")
        (*go)->RemoveComponent<DirectionalLight>();
    else if (component == "PointLight")
        (*go)->RemoveComponent<PointLight>();
    else if (component == "SpotLight")
        (*go)->RemoveComponent<SpotLight>();
    return 0;
}

int LuaScript::pushGameObject(lua_State* L, void* go)
{
    GameObject** tmp = (GameObject**)lua_newuserdata(L, sizeof(GameObject*));
    *tmp = (GameObject*)go;
    luaL_getmetatable(L, "GameObject");
    lua_setmetatable(L, -2);

    return 1;
}

int LuaScript::setGameObjectActive(lua_State* L)
{
    GameObject** go = checkGameObject(L);
    (*go)->SetActive(lua_toboolean(L, 2));
    return 0;
}

int LuaScript::getGameObjectActive(lua_State* L)
{
    GameObject** go = checkGameObject(L, 1);
    lua_pushboolean(L, (*go)->GetActive());
    return 1;
}

int LuaScript::getGameObjectComponent(lua_State* L)
{
    GameObject** go = checkGameObject(L, 1);

    std::string component = luaL_checkstring(L, 2);

    if (component == "Transform")
    {
        Transform* t;
        if (GetComp<Transform>(L, &t)) return pushTransform(L, t);
    }
    else if (component == "RigidBody")
    {
        RigidbodyComponent* t;
        if (GetComp<RigidbodyComponent>(L, &t)) return pushRigidBody(L, t);
    }
    else if (component == "AudioListener")
    {
        AudioListener* t;
        if (GetComp<AudioListener>(L, &t)) return pushAudioListener(L, t);
    }
    else if (component == "AudioEmitter")
    {
        AudioEmitter* t;
        if (GetComp<AudioEmitter>(L, &t)) return pushAudioEmitter(L, t);
    }
    else if (component == "MeshRenderer")
    {
        MeshRenderer* t;
        if (GetComp<MeshRenderer>(L, &t)) return pushMeshRenderer(L, t);
    }
    else if (component == "MeshAnimator")
    {
        MeshAnimator* t;
        if (GetComp<MeshAnimator>(L, &t)) return pushMeshAnimator(L, t);
    }
    else if (component == "Camera")
    {
        Camera* t;
        if (GetComp<Camera>(L, &t)) return pushCamera(L, t);
    }
    else if (component == "ParticleEmitter_Box")
    {
        BoxParticleEmitter* t;
        if (GetComp<BoxParticleEmitter>(L, &t)) return pushBoxParticle(L, t);
    }
    else if (component == "ParticleEmitter_Circle")
    {
        CircleParticleEmitter* t;
        if (GetComp<CircleParticleEmitter>(L, &t)) return pushCircleParticle(L, t);
    }
    else if (component == "PathFinding")
    {
        Pathfinding* t;
        if (GetComp<Pathfinding>(L, &t)) return pushPathFinding(L, t);
    }
    else if (component == "BoxCollider")
    {
        BoxCollider* t;
        if (GetComp<BoxCollider>(L, &t)) return pushBoxCollider(L, t);
    }
    else if (component == "SphereCollider")
    {
        SphereCollider* t;
        if (GetComp<SphereCollider>(L, &t)) return pushSphereCollider(L, t);
    }
    else if (component == "TextRenderer")
    {
        TextRenderer* t;
        if (GetComp<TextRenderer>(L, &t)) return pushTextRenderer(L, t);
    }
    else if (component == "Attractor")
    {
        Attractor* t;
        if (GetComp<Attractor>(L, &t)) return pushAttractor(L, t);
    }
    else if (component == "DirectionalLight")
    {
        DirectionalLight* t;
        if (GetComp<DirectionalLight>(L, &t)) return pushDirectionalLight(L, t);
    }
    else if (component == "PointLight")
    {
        PointLight* t;
        if (GetComp<PointLight>(L, &t)) return pushPointLight(L, t);
    }
    else if (component == "SpotLight")
    {
        SpotLight* t;
        if (GetComp<SpotLight>(L, &t)) return pushSpotLight(L, t);
    }
    return 0;
}

int LuaScript::getGameObjectComponents(lua_State* L)
{
    GameObject** go = checkGameObject(L);
    auto complist = (*go)->GetComponentList();
    auto complistIT = complist.begin();

    lua_createtable(L, (int)complist.size(), 0);
    int newTable = lua_gettop(L);
    for (unsigned i = 1; i <= complist.size(); ++i)
    {
        if (dynamic_cast<LuaScript*>(*complistIT))
            lua_pushstring(L, dynamic_cast<LuaScript*>(*complistIT)->GetScript().c_str());
        else
            lua_pushstring(L, (*complistIT)->GetName().c_str());

        lua_rawseti(L, newTable, i);
        ++complistIT;
    }
    return 1;
}

int LuaScript::addGameObjectComponent(lua_State* L)
{
    GameObject** go = checkGameObject(L);
    std::string comp_name = luaL_checkstring(L, 2);
    try
    {
        auto comp_fac = (*Factory::m_Factories).at(comp_name);
        comp_fac->create(*go);
    }
    catch (...)
    {
        std::cout << "Component / Script (" << comp_name << ") does not exists" << std::endl;
    }
    return 0;
}

int LuaScript::getGameObjectLayer(lua_State* L)
{
    GameObject** go = checkGameObject(L);
    Layer* ly = (*go)->GetParentLayer();
    setlayer(L, (void*)ly);
    return 1;
}

int LuaScript::gameObject_GetID(lua_State* L)
{
    GameObject** go = checkGameObject(L);
    lua_pushinteger(L, (*go)->GetID());
    return 1;
}

int LuaScript::gameObject_GetTag(lua_State* L)
{
    GameObject** go = checkGameObject(L, 1);
    lua_pushstring(L, (*go)->GetTag().c_str());
    return 1;
}

int LuaScript::gameObject_SetParent(lua_State* L)
{
    GameObject** go = checkGameObject(L, 1);
    if (lua_isnumber(L, 2))
    {
        int parentId = int(lua_tointeger(L, 2));
        (*go)->SetParentObject(parentId);
        (*go)->SetIsChildren(true);

        Layer* ly = (*go)->GetParentLayer();
        GameObject* parent = ly->GetObjectById(parentId);
        ChildrenList childrens = parent->GetChildrenObjects();
        childrens.push_back((*go)->GetID());
        parent->SetChildrenObjects(childrens);
    }
    else
        std::cout << "Set Parent takes in Parent ID (unsigned int) only!" << std::endl;
    return 0;
}

int LuaScript::gameObject_GetParent(lua_State* L)
{
    GameObject** go = checkGameObject(L, 1);
    pushGameObject(L, (*go)->GetParentLayer()->GetObjectById((*go)->GetParentObject()));
    return 1;
}

int LuaScript::gameObject_UnParent(lua_State* L)
{
    GameObject** go = checkGameObject(L, 1);
    unsigned parentId = (*go)->GetParentObject();
    if (parentId)
    {
        Layer* ly = (*go)->GetParentLayer();
        GameObject* parent = (ly)->GetObjectById(parentId);
        if (parent != nullptr)
        {
            ChildrenList childrens = parent->GetChildrenObjects();

            ChildrenList::iterator itr = std::find(childrens.begin(), childrens.end(), (*go)->GetID());
            if (itr != childrens.end())
            {
                childrens.erase(itr);
                parent->SetChildrenObjects(childrens);

                (*go)->SetParentObject(0);
                (*go)->SetIsChildren(false);
                // std::cout << "Unparented!" << std::endl;
            }
        }
    }
    else
        std::cout << "No parent. Nothing done." << std::endl;
    return 0;
}

int LuaScript::gameObject_GetChild(lua_State* L)
{
    GameObject** go = checkGameObject(L, 1);
    std::string childName{lua_tostring(L, 2)};
    pushGameObject(L, (*go)->GetParentLayer()->GetObjectById((*go)->GetChildObjectByName(childName)));
    return 1;
}

// ============================================================================
// COMPONENTS
// ============================================================================
IComponent** LuaScript::checkIsComponent(lua_State* L, int index)
{
    void* com = (void*)checkTransform(L, index, false);
    com = com ? com : (void*)checkRigidbody(L, index, false);
    com = com ? com : (void*)checkAudioListener(L, index, false);
    com = com ? com : (void*)checkAudioEmitter(L, index, false);
    com = com ? com : (void*)checkMeshRenderer(L, index, false);
    com = com ? com : (void*)checkMeshAnimator(L, index, false);
    com = com ? com : (void*)checkluaScript(L, index, false);
    com = com ? com : (void*)checkCamera(L, index, false);
    com = com ? com : (void*)checkBoxParticle(L, index, false);
    com = com ? com : (void*)checkCircleParticle(L, index, false);
    com = com ? com : (void*)checkPathFinding(L, index, false);
    com = com ? com : (void*)checkBoxCollider(L, index, false);
    com = com ? com : (void*)checkSphereCollider(L, index, false);
    com = com ? com : (void*)checkTextRenderer(L, index, false);
    com = com ? com : (void*)checkAttractor(L, index, false);
    com = com ? com : (void*)checkDirectionalLight(L, index, false);
    com = com ? com : (void*)checkPointLight(L, index, false);
    com = com ? com : (void*)checkSpotLight(L, index, false);

    if (com) return (IComponent**)com;

    return nullptr;
}

int LuaScript::component_Owner(lua_State* L)
{
    IComponent** com = checkIsComponent(L, 1);

    if (com) return pushGameObject(L, (*com)->GetOwner());

    lua_pushnil(L);
    return 1;
}

int LuaScript::component_SetActive(lua_State* L)
{
    IComponent** com = checkIsComponent(L, 1);

    if (com)
    {
        bool active = lua_toboolean(L, 2);
        active ? (*com)->OnActive() : (*com)->OnInactive();
    }

    return 0;
}

// ============================================================================
// TRANSFORM
// ============================================================================
int LuaScript::transform_metatable(lua_State* L)
{
    luaL_newmetatable(L, "Transform");
    lua_pushstring(L, "__index");
    lua_pushvalue(L, -2);
    lua_settable(L, -3);
    luaL_setfuncs(L, transformlib, 0);
    return 1;
}

Transform** LuaScript::checkTransform(lua_State* L, int index, bool assert)
{
    return check<Transform**>("Transform", L, index, assert);
}

int LuaScript::pushTransform(lua_State* L, Transform* input)
{
    return pushCustomType<Transform*>("Transform", L, input);
}

// int LuaScript::transform_GetLocalPosition(lua_State* L)
//{
//  Transform ** t = checkTransform(L, 1);
//  return pushVector3(L, (*t)->GetLocalPosition());
//}

// int LuaScript::transform_GetLocalRotation(lua_State* L)
//{
//  Transform ** t = checkTransform(L, 1);
//  return pushVector3(L, (*t)->GetLocalRotation());
//}
//
// int LuaScript::transform_GetLocalScale(lua_State* L)
//{
//  Transform ** t = checkTransform(L, 1);
//  return pushVector3(L, (*t)->GetLocalScale());
//}

int LuaScript::transform_GetWorldPosition(lua_State* L)
{
    Transform** t = checkTransform(L, 1);
    return pushVector3(L, (*t)->GetWorldPosition());
}

int LuaScript::transform_GetWorldRotation(lua_State* L)
{
    Transform** t = checkTransform(L, 1);
    return pushVector3(L, (*t)->GetWorldRotation());
}

int LuaScript::transform_GetWorldScale(lua_State* L)
{
    Transform** t = checkTransform(L, 1);
    return pushVector3(L, (*t)->GetWorldScale());
}

// int LuaScript::transform_SetLocalPosition(lua_State* L)
//{
//  Transform ** t = checkTransform(L, 1);
//
//  Vector3 * tmp = checkVector3(L, 2, false);
//  if (tmp)
//    (*t)->SetLocalPosition(*tmp);
//  else if (lua_isnumber(L, 2) && lua_isnumber(L, 3) && lua_isnumber(L, 4))
//    (*t)->SetLocalPosition(Vector3{ static_cast<float>(lua_tonumber(L, 2)),
//                                    static_cast<float>(lua_tonumber(L, 3)) ,
//                                    static_cast<float>(lua_tonumber(L, 4)) });
//  else if (lua_isstring(L, 2) && lua_isnumber(L, 3))
//  {
//    Vector3 tmp = (*t)->GetLocalPosition();
//    std::string index = luaL_checkstring(L, 2);
//    float       value = static_cast<float>(luaL_checknumber(L, 3));
//
//    if (index == "x") tmp.x = value;
//    else if (index == "y") tmp.y = value;
//    else if (index == "z") tmp.z = value;
//
//    (*t)->SetLocalPosition(tmp);
//  }
//
//  return 0;
//}

// int LuaScript::transform_SetLocalRotation(lua_State* L)
//{
//  Transform ** t = checkTransform(L, 1);
//  Vector3 * tmp = checkVector3(L, 2, false);
//  if (tmp)
//    (*t)->SetLocalRotation(*tmp);
//  else if (lua_isnumber(L, 2) && lua_isnumber(L, 3) && lua_isnumber(L, 4))
//    (*t)->SetLocalRotation(Vector3{ static_cast<float>(lua_tonumber(L, 2)),
//                                    static_cast<float>(lua_tonumber(L, 3)) ,
//                                    static_cast<float>(lua_tonumber(L, 4)) });
//  else if (lua_isstring(L, 2) && lua_isnumber(L, 3))
//  {
//    Vector3 tmp = (*t)->GetLocalRotation();
//    std::string index = luaL_checkstring(L, 2);
//    float       value = static_cast<float>(luaL_checknumber(L, 3));
//
//    if (index == "x") tmp.x = value;
//    else if (index == "y") tmp.y = value;
//    else if (index == "z") tmp.z = value;
//
//    (*t)->SetLocalRotation(tmp);
//  }
//
//  return 0;
//}
//
// int LuaScript::transform_SetLocalScale(lua_State* L)
//{
//  Transform ** t = checkTransform(L, 1);
//  Vector3 * tmp = checkVector3(L, 2, false);
//  if (tmp)
//    (*t)->SetLocalScale(*tmp);
//  else if (lua_isnumber(L, 2) && lua_isnumber(L, 3) && lua_isnumber(L, 4))
//    (*t)->SetLocalScale(Vector3{ static_cast<float>(lua_tonumber(L, 2)),
//                                 static_cast<float>(lua_tonumber(L, 3)) ,
//                                 static_cast<float>(lua_tonumber(L, 4)) });
//  else if (lua_isstring(L, 2) && lua_isnumber(L, 3))
//  {
//    Vector3 tmp = (*t)->GetLocalScale();
//    std::string index = luaL_checkstring(L, 2);
//    float       value = static_cast<float>(luaL_checknumber(L, 3));
//
//    if (index == "x") tmp.x = value;
//    else if (index == "y") tmp.y = value;
//    else if (index == "z") tmp.z = value;
//
//    (*t)->SetLocalScale(tmp);
//  }
//  return 0;
//}

int LuaScript::transform_SetWorldPosition(lua_State* L)
{
    Transform** t = checkTransform(L, 1);
    Vector3* tmp = checkVector3(L, 2, false);
    if (tmp)
        (*t)->SetWorldPosition(*tmp);
    else if (lua_isnumber(L, 2) && lua_isnumber(L, 3) && lua_isnumber(L, 4))
        (*t)->SetWorldPosition(Vector3{static_cast<float>(lua_tonumber(L, 2)), static_cast<float>(lua_tonumber(L, 3)), static_cast<float>(lua_tonumber(L, 4))});
    else if (lua_isstring(L, 2) && lua_isnumber(L, 3))
    {
        Vector3 tmp = (*t)->GetWorldPosition();
        std::string index = luaL_checkstring(L, 2);
        float value = static_cast<float>(luaL_checknumber(L, 3));

        if (index == "x")
            tmp.x = value;
        else if (index == "y")
            tmp.y = value;
        else if (index == "z")
            tmp.z = value;

        (*t)->SetWorldPosition(tmp);
    }
    return 0;
}

int LuaScript::transform_SetWorldRotation(lua_State* L)
{
    Transform** t = checkTransform(L, 1);
    Vector3* tmp = checkVector3(L, 2);
    if (tmp)
        (*t)->SetWorldRotation(*tmp);
    else if (lua_isnumber(L, 2) && lua_isnumber(L, 3) && lua_isnumber(L, 4))
        (*t)->SetWorldRotation(Vector3{static_cast<float>(lua_tonumber(L, 2)), static_cast<float>(lua_tonumber(L, 3)), static_cast<float>(lua_tonumber(L, 4))});
    else if (lua_isstring(L, 2) && lua_isnumber(L, 3))
    {
        Vector3 tmp = (*t)->GetWorldRotation();
        std::string index = luaL_checkstring(L, 2);
        float value = static_cast<float>(luaL_checknumber(L, 3));

        if (index == "x")
            tmp.x = value;
        else if (index == "y")
            tmp.y = value;
        else if (index == "z")
            tmp.z = value;

        (*t)->SetWorldRotation(tmp);
    }
    return 0;
}

int LuaScript::transform_SetWorldScale(lua_State* L)
{
    Transform** t = checkTransform(L, 1);
    Vector3* tmp = checkVector3(L, 2);
    if (tmp)
        (*t)->SetWorldScale(*tmp);
    else if (lua_isnumber(L, 2) && lua_isnumber(L, 3) && lua_isnumber(L, 4))
        (*t)->SetWorldScale(Vector3{static_cast<float>(lua_tonumber(L, 2)), static_cast<float>(lua_tonumber(L, 3)), static_cast<float>(lua_tonumber(L, 4))});
    else if (lua_isstring(L, 2) && lua_isnumber(L, 3))
    {
        Vector3 tmp = (*t)->GetWorldScale();
        std::string index = luaL_checkstring(L, 2);
        float value = static_cast<float>(luaL_checknumber(L, 3));

        if (index == "x")
            tmp.x = value;
        else if (index == "y")
            tmp.y = value;
        else if (index == "z")
            tmp.z = value;

        (*t)->SetWorldScale(tmp);
    }
    return 0;
}

int LuaScript::transform_Translate(lua_State* L)
{
    Transform** t = checkTransform(L, 1);
    Vector3* tmp = checkVector3(L, 2);
    if (tmp)
        (*t)->Translate(*tmp);
    else if (lua_isnumber(L, 2) && lua_isnumber(L, 3) && lua_isnumber(L, 4))
        (*t)->Translate(Vector3{static_cast<float>(lua_tonumber(L, 2)), static_cast<float>(lua_tonumber(L, 3)), static_cast<float>(lua_tonumber(L, 4))});
    return 0;
}

int LuaScript::transform_Rotate(lua_State* L)
{
    Transform** t = checkTransform(L, 1);
    Vector3* tmp = checkVector3(L, 2);
    if (tmp)
        (*t)->Rotate(*tmp);
    else if (lua_isnumber(L, 2) && lua_isnumber(L, 3) && lua_isnumber(L, 4))
        (*t)->Rotate(Vector3{static_cast<float>(lua_tonumber(L, 2)), static_cast<float>(lua_tonumber(L, 3)), static_cast<float>(lua_tonumber(L, 4))});
    return 0;
}

int LuaScript::transform_Scale(lua_State* L)
{
    Transform** t = checkTransform(L, 1);
    Vector3* tmp = checkVector3(L, 2);
    (*t)->Scale(*tmp);
    return 0;
}

int LuaScript::transform_GetForwardVector(lua_State* L)
{
    Transform** t = checkTransform(L, 1);
    return pushVector3(L, (*t)->GetForwardVector());
}

int LuaScript::transform_GetUpwardVector(lua_State* L)
{
    Transform** t = checkTransform(L, 1);
    return pushVector3(L, (*t)->GetUpwardVector());
}

int LuaScript::transform_GetRightVector(lua_State* L)
{
    Transform** t = checkTransform(L, 1);
    return pushVector3(L, (*t)->GetRightVector());
}

int LuaScript::transform_LookAt(lua_State* L)
{
    // Transform ** t1 = checkTransform(L, 1);
    // Transform ** t2 = checkTransform(L, 2);
    //
    // Vector3   source = (*t1)->GetWorldPosition();
    // Vector3   target = (*t2)->GetWorldPosition();
    //
    //
    // auto mat = Matrix4::LookAt(target, source, Transform::worldForward);
    //
    // Vector3 forwardVector = (target - source).Normalize();
    // Vector3 rotAxis = Transform::worldForward.Cross(forwardVector);
    // float   dot     = Transform::worldForward.Dot(forwardVector);
    //
    // Quaternion q;
    // q.x = rotAxis.x;
    // q.y = rotAxis.y;
    // q.z = rotAxis.z;
    // q.w = dot + 1;
    //
    // Vector4 rot = q.EulerTransform() *  Vector4 { 90, 0, 0, 0 };
    //(*t1)->SetWorldRotation(Vector3{ rot.x, rot.y, rot.z });

    return 0;
}

int LuaScript::transform_LookAtV(lua_State* L)
{
    // Transform ** t      = checkTransform(L, 1);
    // Vector3   source    = (*t)->GetWorldPosition();
    // Vector3   target    = *checkVector3(L, 2);
    //
    // Vector3 forwardVector = (target - source).Normalize();
    // Vector3 rotAxis       = Transform::worldForward.Cross(forwardVector);
    // float   dot           = Transform::worldForward.Dot  (forwardVector);
    //
    // Quaternion q;
    // q.x = rotAxis.x;
    // q.y = rotAxis.z;
    // q.z = rotAxis.y;
    // q.w = dot + 1;
    //
    // Vector4 rot = q.EulerTransform() *  Vector4 { 90, 0, 0, 0 };
    //(*t)->SetWorldRotation(Vector3{ rot.x, rot.y, rot.z });

    return 0;
}

// ============================================================================
// RIGIDBODY
// ============================================================================
int LuaScript::rigidbody_metatable(lua_State* L)
{
    luaL_newmetatable(L, "RigidBody");
    lua_pushstring(L, "__index");
    lua_pushvalue(L, -2);
    lua_settable(L, -3);
    luaL_setfuncs(L, rigidbodylib, 0);
    return 1;
}

RigidbodyComponent** LuaScript::checkRigidbody(lua_State* L, int index, bool assert)
{
    return check<RigidbodyComponent**>("RigidBody", L, index, assert);
}

int LuaScript::pushRigidBody(lua_State* L, RigidbodyComponent* input)
{
    return pushCustomType<RigidbodyComponent*>("RigidBody", L, input);
}

int LuaScript::rigidbody_SetGhost(lua_State* L)
{
    RigidbodyComponent** t = checkRigidbody(L, 1);
    (*t)->SetGhost(static_cast<bool>(lua_toboolean(L, 2)));
    return 0;
}

// int LuaScript::rigidbody_SetCollideWithStatic(lua_State* L)
//{
//  RigidbodyComponent ** t = checkRigidbody(L, 1);
//  (*t)->SetCollideWithStatic(static_cast<bool>(lua_toboolean(L, 2)));
//  return 0;
//}

int LuaScript::rigidbody_SetMass(lua_State* L)
{
    RigidbodyComponent** t = checkRigidbody(L, 1);
    (*t)->SetMass(static_cast<float>(luaL_checknumber(L, 2)));
    return 0;
}

// int LuaScript::rigidbody_SetDrag(lua_State* L)
//{
//  RigidbodyComponent ** t = checkRigidbody(L, 1);
//  (*t)->SetDrag(static_cast<float>(luaL_checknumber(L, 2)));
//  return 0;
//}

int LuaScript::rigidbody_SetBodytype(lua_State* L)
{
    RigidbodyComponent** t = checkRigidbody(L, 1);
    (*t)->SetBodytype(static_cast<int>(luaL_checkinteger(L, 2)));
    return 0;
}

// int LuaScript::rigidbody_SetFreezeRotation(lua_State* L)
//{
//  RigidbodyComponent ** t = checkRigidbody(L, 1);
//  (*t)->SetFreezeRotation(static_cast<bool>(lua_toboolean(L, 2)));
//  return 0;
//}

int LuaScript::rigidbody_SetRestitution(lua_State* L)
{
    RigidbodyComponent** t = checkRigidbody(L, 1);
    (*t)->SetRestitution(static_cast<float>(luaL_checknumber(L, 2)));
    return 0;
}

int LuaScript::rigidbody_SetVelocity(lua_State* L)
{
    RigidbodyComponent** t = checkRigidbody(L, 1);
    Vector3* tmp = checkVector3(L, 2);
    if (tmp)
        (*t)->SetVelocity(*tmp);
    else if (lua_isnumber(L, 2) && lua_isnumber(L, 3) && lua_isnumber(L, 4))
        (*t)->SetVelocity(Vector3{static_cast<float>(lua_tonumber(L, 2)), static_cast<float>(lua_tonumber(L, 3)), static_cast<float>(lua_tonumber(L, 4))});
    else if (lua_isstring(L, 2) && lua_isnumber(L, 3))
    {
        Vector3 tmp = (*t)->GetVelocity();
        std::string index = luaL_checkstring(L, 2);
        float value = static_cast<float>(luaL_checknumber(L, 3));

        if (index == "x")
            tmp.x = value;
        else if (index == "y")
            tmp.y = value;
        else if (index == "z")
            tmp.z = value;

        (*t)->SetVelocity(tmp);
    }
    return 0;
}

int LuaScript::rigidbody_SetAcceleration(lua_State* L)
{
    RigidbodyComponent** t = checkRigidbody(L, 1);
    Vector3* tmp = checkVector3(L, 2);
    if (tmp)
        (*t)->SetAcceleration(*tmp);
    else if (lua_isnumber(L, 2) && lua_isnumber(L, 3) && lua_isnumber(L, 4))
        (*t)->SetAcceleration(Vector3{static_cast<float>(lua_tonumber(L, 2)), static_cast<float>(lua_tonumber(L, 3)), static_cast<float>(lua_tonumber(L, 4))});
    else if (lua_isstring(L, 2) && lua_isnumber(L, 3))
    {
        Vector3 tmp = (*t)->GetAcceleration();
        std::string index = luaL_checkstring(L, 2);
        float value = static_cast<float>(luaL_checknumber(L, 3));

        if (index == "x")
            tmp.x = value;
        else if (index == "y")
            tmp.y = value;
        else if (index == "z")
            tmp.z = value;

        (*t)->SetAcceleration(tmp);
    }
    return 0;
}

/*int LuaScript::rigidbody_SetOffset(lua_State* L)
{
  RigidbodyComponent ** t = checkRigidbody(L, 1);
  Vector3 * tmp = checkVector3(L, 2);
  if (tmp)
    (*t)->SetOffset(*tmp);
  else if (lua_isnumber(L, 2) && lua_isnumber(L, 3) && lua_isnumber(L, 4))
    (*t)->SetOffset(Vector3{ static_cast<float>(lua_tonumber(L, 2)),
                             static_cast<float>(lua_tonumber(L, 3)) ,
                             static_cast<float>(lua_tonumber(L, 4)) });
  else if (lua_isstring(L, 2) && lua_isnumber(L, 3))
  {
    Vector3 tmp = (*t)->GetOffset();
    std::string index = luaL_checkstring(L, 2);
    float       value = static_cast<float>(luaL_checknumber(L, 3));

    if (index == "x") tmp.x = value;
    else if (index == "y") tmp.y = value;
    else if (index == "z") tmp.z = value;

    (*t)->SetOffset(tmp);
  }
  return 0;
}*/

// int LuaScript::rigidbody_SetAngularVelocity(lua_State* L)
//{
//  RigidbodyComponent ** t = checkRigidbody(L, 1);
//  Vector3 * tmp = checkVector3(L, 2);
//  if (tmp)
//    (*t)->SetAngularVelocity(*tmp);
//  else if (lua_isnumber(L, 2) && lua_isnumber(L, 3) && lua_isnumber(L, 4))
//    (*t)->SetAngularVelocity(Vector3{ static_cast<float>(lua_tonumber(L, 2)),
//                                      static_cast<float>(lua_tonumber(L, 3)) ,
//                                      static_cast<float>(lua_tonumber(L, 4)) });
//  else if (lua_isstring(L, 2) && lua_isnumber(L, 3))
//  {
//    Vector3 tmp = (*t)->GetAngularVelocity();
//    std::string index = luaL_checkstring(L, 2);
//    float       value = static_cast<float>(luaL_checknumber(L, 3));
//
//    if (index == "x") tmp.x = value;
//    else if (index == "y") tmp.y = value;
//    else if (index == "z") tmp.z = value;
//
//    (*t)->SetAngularVelocity(tmp);
//  }
//  return 0;
//}

int LuaScript::rigidbody_SetGravityEnabled(lua_State* L)
{
    RigidbodyComponent** t = checkRigidbody(L, 1);
    (*t)->SetGravityEnabled(static_cast<bool>(lua_toboolean(L, 2)));
    return 0;
}

/*int LuaScript::rigidbody_SetIsAffectedByResistance(lua_State* L)
{
  RigidbodyComponent ** t = checkRigidbody(L, 1);
  (*t)->SetIsAffectedByResistance(static_cast<bool>(lua_toboolean(L, 2)));
  return 0;
}*/

int LuaScript::rigidbody_GetGhost(lua_State* L)
{
    RigidbodyComponent** t = checkRigidbody(L, 1);
    lua_pushboolean(L, (*t)->GetGhost());
    return 1;
}

// int LuaScript::rigidbody_GetCollideWithStatic(lua_State* L)
//{
//  RigidbodyComponent ** t = checkRigidbody(L, 1);
//  lua_pushboolean(L, (*t)->GetCollideWithStatic());
//  return 1;
//}

int LuaScript::rigidbody_GetMass(lua_State* L)
{
    RigidbodyComponent** t = checkRigidbody(L, 1);
    lua_pushnumber(L, (*t)->GetMass());
    return 1;
}

// int LuaScript::rigidbody_GetDrag(lua_State* L)
//{
//  RigidbodyComponent ** t = checkRigidbody(L, 1);
//  lua_pushnumber(L, (*t)->GetMass());
//  return 1;
//}

int LuaScript::rigidbody_GetBodytype(lua_State* L)
{
    RigidbodyComponent** t = checkRigidbody(L, 1);
    lua_pushnumber(L, (*t)->GetBodytypeInt());
    return 1;
}

// int LuaScript::rigidbody_GetFreezeRotation(lua_State* L)
//{
//  RigidbodyComponent ** t = checkRigidbody(L, 1);
//  lua_pushboolean(L, (*t)->GetFreezeRotation());
//  return 1;
//}

int LuaScript::rigidbody_GetRestitution(lua_State* L)
{
    RigidbodyComponent** t = checkRigidbody(L, 1);
    lua_pushnumber(L, (*t)->GetRestitution());
    return 1;
}

int LuaScript::rigidbody_GetVelocity(lua_State* L)
{
    RigidbodyComponent** t = checkRigidbody(L, 1);
    return pushVector3(L, (*t)->GetVelocity());
}

int LuaScript::rigidbody_GetAcceleration(lua_State* L)
{
    RigidbodyComponent** t = checkRigidbody(L, 1);
    return pushVector3(L, (*t)->GetAcceleration());
}

/*int LuaScript::rigidbody_GetOffset(lua_State* L)
{
  RigidbodyComponent ** t = checkRigidbody(L, 1);
  return pushVector3(L, (*t)->GetOffset());
}*/

// int LuaScript::rigidbody_GetAngularVelocity(lua_State* L)
//{
//  RigidbodyComponent ** t = checkRigidbody(L, 1);
//  return pushVector3(L, (*t)->GetAngularVelocity());
//}

int LuaScript::rigidbody_GetGravityEnabled(lua_State* L)
{
    RigidbodyComponent** t = checkRigidbody(L, 1);
    lua_pushboolean(L, (*t)->GetGravityEnabled());
    return 1;
}

/*int LuaScript::rigidbody_GetIsAffectedByResistance(lua_State* L)
{
  RigidbodyComponent ** t = checkRigidbody(L, 1);
  lua_pushboolean(L, (*t)->GetIsAffectedByResistance());
  return 1;
}*/

int LuaScript::rigidbody_AddVelocity(lua_State* L)
{
    RigidbodyComponent** t = checkRigidbody(L, 1);
    (*t)->AddVelocity(*checkVector3(L, 2));
    return 0;
}

int LuaScript::rigidbody_AddForce(lua_State* L)
{
    RigidbodyComponent** t = checkRigidbody(L, 1);
    (*t)->AddForce(*checkVector3(L, 2));
    return 0;
}

int LuaScript::rigidbody_GetForce(lua_State* L)
{
    RigidbodyComponent** t = checkRigidbody(L, 1);
    return pushVector3(L, (*t)->GetForce());
}

int LuaScript::rigidbody_SetYVelocity(lua_State* L)
{
    RigidbodyComponent** t = checkRigidbody(L, 1);
    (*t)->SetYVelocity(static_cast<float>(luaL_checknumber(L, 2)));
    return 0;
}

// ============================================================================
// AUDIO LISTENER
// ============================================================================
int LuaScript::audioListener_metatable(lua_State* L)
{
    luaL_newmetatable(L, "AudioListener");
    lua_pushstring(L, "__index");
    lua_pushvalue(L, -2);
    lua_settable(L, -3);
    luaL_setfuncs(L, audioListenerlib, 0);
    return 1;
}

AudioListener** LuaScript::checkAudioListener(lua_State* L, int index, bool assert)
{
    return check<AudioListener**>("AudioListener", L, index, assert);
}

int LuaScript::pushAudioListener(lua_State* L, AudioListener* input)
{
    return pushCustomType<AudioListener*>("AudioListener", L, input);
}

int LuaScript::audioListener_IsMain(lua_State* L)
{
    AudioListener** t = checkAudioListener(L, 1);
    lua_pushboolean(L, (*t)->IsMain());
    return 1;
}

int LuaScript::audioListener_SetMain(lua_State* L)
{
    AudioListener** t = checkAudioListener(L, 1);

    if (lua_isboolean(L, 2)) (*t)->MakeMain(lua_toboolean(L, 2));

    return 0;
}

// ============================================================================
// AUDIO EMITTER
// ============================================================================
int LuaScript::audioEmitter_metatable(lua_State* L)
{
    luaL_newmetatable(L, "AudioEmitter");
    lua_pushstring(L, "__index");
    lua_pushvalue(L, -2);
    lua_settable(L, -3);
    luaL_setfuncs(L, audioEmitterlib, 0);
    return 1;
}

AudioEmitter** LuaScript::checkAudioEmitter(lua_State* L, int index, bool assert)
{
    return check<AudioEmitter**>("AudioEmitter", L, index, assert);
}

int LuaScript::pushAudioEmitter(lua_State* L, AudioEmitter* input)
{
    return pushCustomType<AudioEmitter*>("AudioEmitter", L, input);
}

int LuaScript::audioEmitter_Play(lua_State* L)
{
    AudioEmitter** t = checkAudioEmitter(L, 1);
    (*t)->Play();
    return 0;
}

int LuaScript::audioEmitter_Stop(lua_State* L)
{
    AudioEmitter** t = checkAudioEmitter(L, 1);
    (*t)->Stop();
    return 0;
}

int LuaScript::audioEmitter_Pause(lua_State* L)
{
    AudioEmitter** t = checkAudioEmitter(L, 1);
    (*t)->Pause(static_cast<bool>(lua_toboolean(L, 2)));
    return 0;
}

int LuaScript::audioEmitter_IsPlaying(lua_State* L)
{
    AudioEmitter** t = checkAudioEmitter(L, 1);
    lua_pushboolean(L, (*t)->IsPlaying());
    return 1;
}

int LuaScript::audioEmitter_IsPaused(lua_State* L)
{
    AudioEmitter** t = checkAudioEmitter(L, 1);
    lua_pushboolean(L, (*t)->IsPaused());
    return 1;
}

int LuaScript::audioEmitter_SetVolume(lua_State* L)
{
    AudioEmitter** t = checkAudioEmitter(L, 1);
    (*t)->SetVolume(static_cast<float>(lua_tonumber(L, 2)));
    return 0;
}

int LuaScript::audioEmitter_GetVolume(lua_State* L)
{
    AudioEmitter** t = checkAudioEmitter(L, 1);
    lua_pushnumber(L, (*t)->GetVolume());
    return 1;
}

int LuaScript::audioEmitter_SetMute(lua_State* L)
{
    AudioEmitter** t = checkAudioEmitter(L, 1);
    (*t)->SetMute(lua_toboolean(L, 2));
    return 0;
}

int LuaScript::audioEmitter_GetMute(lua_State* L)
{
    AudioEmitter** t = checkAudioEmitter(L, 1);
    lua_pushboolean(L, (*t)->GetMute());
    return 1;
}

int LuaScript::audioEmitter_SetPitch(lua_State* L)
{
    AudioEmitter** t = checkAudioEmitter(L, 1);
    (*t)->SetPitch(static_cast<float>(lua_tonumber(L, 2)));
    return 0;
}

int LuaScript::audioEmitter_GetPitch(lua_State* L)
{
    AudioEmitter** t = checkAudioEmitter(L, 1);
    lua_pushnumber(L, (*t)->GetPitch());
    return 1;
}

int LuaScript::audioEmitter_SetLoop(lua_State* L)
{
    AudioEmitter** t = checkAudioEmitter(L, 1);
    (*t)->SetLoop(static_cast<bool>(lua_toboolean(L, 2)));
    return 0;
}

int LuaScript::audioEmitter_SetLoopCount(lua_State* L)
{
    AudioEmitter** t = checkAudioEmitter(L, 1);
    (*t)->SetLoopCount((int)lua_tointeger(L, 2));
    return 0;
}

int LuaScript::audioEmitter_GetLoopCount(lua_State* L)
{
    AudioEmitter** t = checkAudioEmitter(L, 1);
    lua_pushinteger(L, (*t)->GetLoopCount());
    return 1;
}

int LuaScript::audioEmitter_SetAudioClip(lua_State* L)
{
    AudioEmitter** t = checkAudioEmitter(L, 1);
    (*t)->SetAudioClip(std::string{luaL_checkstring(L, 2)});
    return 0;
}

int LuaScript::audioEmitter_SetAndPlayAudioClip(lua_State* L)
{
    AudioEmitter** t = checkAudioEmitter(L, 1);
    (*t)->SetAndPlayAudioClip(std::string{luaL_checkstring(L, 2)});
    return 0;
}

int LuaScript::audioEmitter_SetMinDistance3D(lua_State* L)
{
    AudioEmitter** t = checkAudioEmitter(L, 1);
    (*t)->SetMinDistance3D(static_cast<float>(lua_tonumber(L, 2)));
    return 0;
}

int LuaScript::audioEmitter_SetMaxDistance3D(lua_State* L)
{
    AudioEmitter** t = checkAudioEmitter(L, 1);
    (*t)->SetMaxDistance3D(static_cast<float>(lua_tonumber(L, 2)));
    return 0;
}

int LuaScript::audioEmitter_GetMaxDistance3D(lua_State* L)
{
    AudioEmitter** t = checkAudioEmitter(L, 1);
    lua_pushnumber(L, (*t)->GetMaxDistance3D());
    return 1;
}

int LuaScript::audioEmitter_GetMinDistance3D(lua_State* L)
{
    AudioEmitter** t = checkAudioEmitter(L, 1);
    lua_pushnumber(L, (*t)->GetMinDistance3D());
    return 1;
}

int LuaScript::audioEmitter_GetSoundName(lua_State* L)
{
    AudioEmitter** t = checkAudioEmitter(L, 1);
    lua_pushstring(L, (*t)->GetSoundName().c_str());
    return 1;
}

int LuaScript::audioEmitter_FadeOut(lua_State* L)
{
    AudioEmitter** t = checkAudioEmitter(L, 1);
    (*t)->FadeOut(static_cast<float>(lua_tonumber(L, 2)));
    return 0;
}

int LuaScript::audioEmitter_SetChannelGroup(lua_State* L)
{
    AudioEmitter** t = checkAudioEmitter(L, 1);
    (*t)->SetChannelGroup(lua_tostring(L, 2));
    return 0;
}

int LuaScript::audioSystem_PlayAudioAtLocation(lua_State* L)
{
    int params = lua_gettop(L);
    switch (params)
    {
        case 2: AudioSystem::Instance().PlayAtLocation(luaL_checkstring(L, 1), *checkVector3(L, 2)); break;
        case 3: AudioSystem::Instance().PlayAtLocation(luaL_checkstring(L, 1), *checkVector3(L, 2), static_cast<float>(lua_tonumber(L, 3))); break;
        case 4: AudioSystem::Instance().PlayAtLocation(luaL_checkstring(L, 1), *checkVector3(L, 2), static_cast<float>(lua_tonumber(L, 3)), static_cast<float>(lua_tonumber(L, 4))); break;
        case 5:
            AudioSystem::Instance().PlayAtLocation(luaL_checkstring(L, 1), *checkVector3(L, 2), static_cast<float>(lua_tonumber(L, 3)), static_cast<float>(lua_tonumber(L, 4)),
                                                   static_cast<float>(lua_tonumber(L, 5)));
            break;
        case 6:
            AudioSystem::Instance().PlayAtLocation(luaL_checkstring(L, 1), *checkVector3(L, 2), static_cast<float>(lua_tonumber(L, 3)), static_cast<float>(lua_tonumber(L, 4)),
                                                   static_cast<float>(lua_tonumber(L, 5)), luaL_checkstring(L, 6));
            break;
    }
    return 0;
}

int LuaScript::audioSystem_SetChannelGrpVolume(lua_State* L)
{
    std::string channelName{luaL_checkstring(L, 1)};
    float vol = static_cast<float>(lua_tonumber(L, 2));

    AudioSystem::Instance().SetChannelGrpVol(channelName, vol);
    return 0;
}

int LuaScript::audioSystem_GetChannelGrpVolume(lua_State* L)
{
    std::string channelName{luaL_checkstring(L, 1)};
    lua_pushnumber(L, AudioSystem::Instance().GetChannelGrpVol(channelName));
    return 1;
}

// ============================================================================
// MESH RENDERER
// ============================================================================
int LuaScript::meshRenderer_metatable(lua_State* L)
{
    luaL_newmetatable(L, "MeshRenderer");
    lua_pushstring(L, "__index");
    lua_pushvalue(L, -2);
    lua_settable(L, -3);
    luaL_setfuncs(L, meshRendererlib, 0);
    return 1;
}

MeshRenderer** LuaScript::checkMeshRenderer(lua_State* L, int index, bool assert)
{
    return check<MeshRenderer**>("MeshRenderer", L, index, assert);
}

int LuaScript::pushMeshRenderer(lua_State* L, MeshRenderer* input)
{
    return pushCustomType<MeshRenderer*>("MeshRenderer", L, input);
}

int LuaScript::meshRenderer_SetColor(lua_State* L)
{
    MeshRenderer** m = checkMeshRenderer(L, 1);
    Vector4* c = checkColor(L, 2);

    (*m)->SetMeshColor(*c);
    return 0;
}

int LuaScript::meshRenderer_GetColor(lua_State* L)
{
    MeshRenderer** m = checkMeshRenderer(L, 1);
    return pushColor(L, (*m)->GetMeshColor());
}

int LuaScript::meshRenderer_SetDiffuse(lua_State* L)
{
    MeshRenderer** m = checkMeshRenderer(L, 1);
    std::string resource = {luaL_checkstring(L, 2)};
    HTexture result = RESOURCEMANAGER.GetResource<Texture>(resource);
    if (result.Validate()) (*m)->SetDiffuseTexture(result);

    return 0;
}

int LuaScript::meshRenderer_GetMesh(lua_State* L)
{
    MeshRenderer** m = checkMeshRenderer(L, 1);
    auto mesh = (*m)->GetMesh();
    lua_pushstring(L, mesh->m_Name.c_str());

    return 1;
}

int LuaScript::meshRenderer_SetMesh(lua_State* L)
{
    MeshRenderer** m = checkMeshRenderer(L, 1);
    std::string resource = {luaL_checkstring(L, 2)};
    HMesh result = RESOURCEMANAGER.GetResource<Mesh>(resource);
    if (result.Validate())
        (*m)->SetMesh(result);
    else
        std::cout << "ERROR : Mesh cannot be found" << std::endl;

    return 0;
}

int LuaScript::meshRenderer_SetEmissive(lua_State* L)
{
    MeshRenderer** m = checkMeshRenderer(L, 1);
    Vector4* c = checkColor(L, 2);
    (*m)->m_EmissiveEnabled = true;
    (*m)->SetEmissiveColor(Color3(c->x, c->y, c->z));
    return 0;
}

int LuaScript::meshRenderer_SetEmissiveTexture(lua_State* L)
{
    MeshRenderer** m = checkMeshRenderer(L, 1);
    std::string resource = {luaL_checkstring(L, 2)};
    HTexture result = RESOURCEMANAGER.GetResource<Texture>(resource);
    if (result.Validate()) (*m)->SetEmissiveTexture(result);

    return 0;
}

int LuaScript::meshRenderer_GetEnableEmissive(lua_State* L)
{
    MeshRenderer** m = checkMeshRenderer(L, 1);
    lua_pushboolean(L, (*m)->m_EmissiveEnabled);

    return 1;
}

int LuaScript::meshRenderer_SetEnableEmissive(lua_State* L)
{
    MeshRenderer** m = checkMeshRenderer(L, 1);
    (*m)->m_EmissiveEnabled = lua_toboolean(L, 2);

    return 0;
}

// ============================================================================
// MESH ANIMATOR
// ============================================================================
int LuaScript::meshAnimator_metatable(lua_State* L)
{
    luaL_newmetatable(L, "MeshAnimator");
    lua_pushstring(L, "__index");
    lua_pushvalue(L, -2);
    lua_settable(L, -3);
    luaL_setfuncs(L, meshAnimatorlib, 0);
    return 1;
}

MeshAnimator** LuaScript::checkMeshAnimator(lua_State* L, int index, bool assert)
{
    return check<MeshAnimator**>("MeshAnimator", L, index, assert);
}

int LuaScript::pushMeshAnimator(lua_State* L, MeshAnimator* input)
{
    return pushCustomType<MeshAnimator*>("MeshAnimator", L, input);
}

int LuaScript::meshAnimator_PlayAnimation(lua_State* L)
{
    MeshAnimator** t = checkMeshAnimator(L, 1);
    (*t)->Play(luaL_checkstring(L, 2));
    return 0;
}

int LuaScript::meshAnimator_PlayAnimationOnce(lua_State* L)
{
    MeshAnimator** t = checkMeshAnimator(L, 1);
    (*t)->PlayOnce(luaL_checkstring(L, 2));
    return 0;
}

int LuaScript::meshAnimator_StopAnimation(lua_State* L)
{
    MeshAnimator** t = checkMeshAnimator(L, 1);
    (*t)->Stop();
    return 0;
}

int LuaScript::meshAnimator_PauseAnimation(lua_State* L)
{
    MeshAnimator** t = checkMeshAnimator(L, 1);
    bool pauseState = lua_toboolean(L, 2);
    if (pauseState)
        (*t)->Pause();
    else
        (*t)->Resume();
    return 0;
}

int LuaScript::meshAnimator_IsPlaying(lua_State* L)
{
    MeshAnimator** t = checkMeshAnimator(L, 1);
    lua_pushboolean(L, (*t)->IsPlaying());
    return 1;
}

int LuaScript::meshAnimator_GetAnimationSet(lua_State* L)
{
    MeshAnimator** t = checkMeshAnimator(L, 1);
    return pushResAnimation(L, (*t)->GetAnimationSet());
}

int LuaScript::meshAnimator_SetAnimationSet(lua_State* L)
{
    MeshAnimator** t = checkMeshAnimator(L, 1);
    HAnimationSet* anim = checkResAnimation(L, 2);

    (*t)->SetAnimationSet(*anim);
    return 0;
}

int LuaScript::meshAnimator_SetCrossFadeBoolean(lua_State* L)
{
    MeshAnimator** t = checkMeshAnimator(L, 1);
    (*t)->m_CrossFadeAnimations = lua_toboolean(L, 2);
    return 0;
}

int LuaScript::meshAnimator_SetTimeScale(lua_State* L)
{
    MeshAnimator** t = checkMeshAnimator(L, 1);
    (*t)->m_AnimationTimeScale = lua_tonumber(L, 2);
    return 0;
}

// ============================================================================
// ANIMATION RESOURCE
// ============================================================================
int LuaScript::resAnimation_metatable(lua_State* L)
{
    luaL_newmetatable(L, "Resource_Animation");
    lua_pushstring(L, "__index");
    lua_pushvalue(L, -2);
    lua_settable(L, -3);
    luaL_setfuncs(L, resAnimationlib, 0);
    return 1;
}
HAnimationSet* LuaScript::checkResAnimation(lua_State* L, int index, bool assert)
{
    return check<HAnimationSet*>("Resource_Animation", L, index, assert);
}

int LuaScript::pushResAnimation(lua_State* L, HAnimationSet input)
{
    return pushCustomType<HAnimationSet>("Resource_Animation", L, input);
}

int LuaScript::resAnimation_Validate(lua_State* L)
{
    HAnimationSet* t = checkResAnimation(L, 1);
    lua_pushboolean(L, t->Validate());
    return 1;
}

int LuaScript::resAnimation_Get(lua_State* L)
{
    std::string animationName{luaL_checkstring(L, 1)};
    HAnimationSet result = RESOURCEMANAGER.GetResource<AnimationSet>(animationName);
    if (result.Validate()) return pushResAnimation(L, result);

    lua_pushnil(L);
    return 1;
}

// ============================================================================
// LUASCRIPT
// ============================================================================
int LuaScript::luaScript_metatable(lua_State* L)
{
    luaL_newmetatable(L, "LuaScript");
    lua_pushstring(L, "__index");
    lua_pushvalue(L, -2);
    lua_settable(L, -3);
    luaL_setfuncs(L, luaScriptlib, 0);
    return 1;
}

LuaScript** LuaScript::checkluaScript(lua_State* L, int index, bool assert)
{
    return check<LuaScript**>("LuaScript", L, index, assert);
}

int LuaScript::pushluaScript(lua_State* L, LuaScript* input)
{
    return pushCustomType<LuaScript*>("LuaScript", L, input);
}

int LuaScript::lua_GetVariable(lua_State* L)
{
    int params = lua_gettop(L);
    LuaScript** t = checkluaScript(L, 1);
    std::string variableName = luaL_checkstring(L, 2);

    // if (params == 3 && (bool)lua_toboolean(L, 3) == true)
    //{
    //  std::cout << "Variables -----" << std::endl;
    //  for (auto & c : (*t)->variables)
    //    std::cout << c.first << std::endl;
    //  std::cout << "---------------" << std::endl;
    //}

    for (auto& c : (*t)->variables)
        if (c.first == variableName)
        {
            if (c.second == "bool")
            {
                auto tmp = (*t)->get<bool>(variableName);
                lua_pushboolean(L, tmp);
                return 1;
            }
            else if (c.second == "float")
            {
                auto tmp = (*t)->get<float>(variableName);
                (tmp - (int)tmp > 0) ? lua_pushnumber(L, tmp) : lua_pushinteger(L, static_cast<lua_Integer>(tmp));
                return 1;
            }
            else if (c.second == "string")
            {
                auto tmp = (*t)->get<std::string>(variableName);
                lua_pushstring(L, tmp.c_str());
                return 1;
            }
            else if (c.second == "int")
            {
                auto tmp = (*t)->get<int>(variableName);
                lua_pushinteger(L, tmp);
                return 1;
            }
            else if (c.second == "vector3")
            {
                auto tmp = (*t)->get<Vector3>(variableName);
                return pushVector3(L, tmp);
            }
            else if (c.second == "color")
            {
                auto tmp = (*t)->get<Vector4>(variableName);
                return pushColor(L, tmp);
            }
            else if (c.second == "Resource_Animation")
            {
                auto tmp = (*t)->get<HAnimationSet>(variableName);
                return pushResAnimation(L, tmp);
            }
            else if (c.second == "rayCastData")
            {
                auto tmp = (*t)->get<RaycastData_tmp>(variableName);
                return pushRayCastdata(L, tmp);
            }
            else if (c.second == "GameObject")
            {
                auto tmp = (*t)->get<GameObject*>(variableName);
                return pushGameObject(L, tmp);
            }
            else if (c.second == "Transform")
            {
                auto tmp = (*t)->get<Transform*>(variableName);
                return pushTransform(L, tmp);
            }
            else if (c.second == "RigidBody")
            {
                auto tmp = (*t)->get<RigidbodyComponent*>(variableName);
                return pushRigidBody(L, tmp);
            }
            else if (c.second == "AudioListener")
            {
                auto tmp = (*t)->get<AudioListener*>(variableName);
                return pushAudioListener(L, tmp);
            }
            else if (c.second == "AudioEmitter")
            {
                auto tmp = (*t)->get<AudioEmitter*>(variableName);
                return pushAudioEmitter(L, tmp);
            }
            else if (c.second == "MeshRenderer")
            {
                auto tmp = (*t)->get<MeshRenderer*>(variableName);
                return pushMeshRenderer(L, tmp);
            }
            else if (c.second == "MeshAnimator")
            {
                auto tmp = (*t)->get<MeshAnimator*>(variableName);
                return pushMeshAnimator(L, tmp);
            }
            else if (c.second == "LuaScript")
            {
                auto tmp = (*t)->get<LuaScript*>(variableName);
                return pushluaScript(L, tmp);
            }
            else if (c.second == "Camera")
            {
                auto tmp = (*t)->get<Camera*>(variableName);
                return pushCamera(L, tmp);
            }
            else if (c.second == "ParticleEmitter_Box")
            {
                auto tmp = (*t)->get<BoxParticleEmitter*>(variableName);
                return pushBoxParticle(L, tmp);
            }
            else if (c.second == "ParticleEmitter_Circle")
            {
                auto tmp = (*t)->get<CircleParticleEmitter*>(variableName);
                return pushCircleParticle(L, tmp);
            }
            else if (c.second == "PathFinding")
            {
                auto tmp = (*t)->get<Pathfinding*>(variableName);
                return pushPathFinding(L, tmp);
            }
            else if (c.second == "BoxCollider")
            {
                auto tmp = (*t)->get<BoxCollider*>(variableName);
                return pushBoxCollider(L, tmp);
            }
            else if (c.second == "SphereCollider")
            {
                auto tmp = (*t)->get<SphereCollider*>(variableName);
                return pushSphereCollider(L, tmp);
            }
            else if (c.second == "TextRenderer")
            {
                auto tmp = (*t)->get<TextRenderer*>(variableName);
                return pushTextRenderer(L, tmp);
            }
            else if (c.second == "Attractor")
            {
                auto tmp = (*t)->get<Attractor*>(variableName);
                return pushAttractor(L, tmp);
            }
            else if (c.second == "DirectionalLight")
            {
                auto tmp = (*t)->get<DirectionalLight*>(variableName);
                return pushDirectionalLight(L, tmp);
            }
            else if (c.second == "PointLight")
            {
                auto tmp = (*t)->get<PointLight*>(variableName);
                return pushPointLight(L, tmp);
            }
            else if (c.second == "SpotLight")
            {
                auto tmp = (*t)->get<SpotLight*>(variableName);
                return pushSpotLight(L, tmp);
            }

            else
                PrintError(variableName, " lua_GetVariable :Type not supported " + (*t)->filename);
        }

    PrintError(variableName, " lua_GetVariable : Variable does not exist " + (*t)->filename);
    return 0;
}

int LuaScript::lua_GetVariableArray(lua_State* L)
{
    LuaScript** t = checkluaScript(L, 1);
    std::string variableName = luaL_checkstring(L, 2);

    for (auto& c : (*t)->variables)
    {
        if (c.first == variableName)
        {
            //     if (c.second == "float"                 ) { return pushCustomTypeArray("float"                 ,L, (*t)->get<std::vector<float                 >>(variableName)); }
            // else if (c.second == "string"                ) { return pushCustomTypeArray("string"                ,L, (*t)->get<std::vector<std::string           >>(variableName)); }
            // else if (c.second == "bool"                  ) { return pushCustomTypeArray("bool"                  ,L, (*t)->get<std::vector<bool                  >>(variableName)); }
            // else if (c.second == "int"                   ) { return pushCustomTypeArray("int"                   ,L, (*t)->get<std::vector<int                   >>(variableName)); }
            if (c.second == "vector3")
            {
                return pushCustomTypeArray("vector3", L, (*t)->get<std::vector<Vector3>>(variableName));
            }
            else if (c.second == "color")
            {
                return pushCustomTypeArray("color", L, (*t)->get<std::vector<Vector4>>(variableName));
            }
            else if (c.second == "Resource_Animation")
            {
                return pushCustomTypeArray("Resource_Animation", L, (*t)->get<std::vector<HAnimationSet>>(variableName));
            }
            else if (c.second == "rayCastData")
            {
                return pushCustomTypeArray("rayCastData", L, (*t)->get<std::vector<RaycastData_tmp>>(variableName));
            }
            else if (c.second == "GameObject")
            {
                return pushCustomTypeArray("GameObject", L, (*t)->get<std::vector<GameObject*>>(variableName));
            }
            else if (c.second == "Transform")
            {
                return pushCustomTypeArray("Transform", L, (*t)->get<std::vector<Transform*>>(variableName));
            }
            else if (c.second == "RigidBody")
            {
                return pushCustomTypeArray("RigidBody", L, (*t)->get<std::vector<RigidbodyComponent*>>(variableName));
            }
            else if (c.second == "AudioListener")
            {
                return pushCustomTypeArray("AudioListener", L, (*t)->get<std::vector<AudioListener*>>(variableName));
            }
            else if (c.second == "AudioEmitter")
            {
                return pushCustomTypeArray("AudioEmitter", L, (*t)->get<std::vector<AudioEmitter*>>(variableName));
            }
            else if (c.second == "MeshRenderer")
            {
                return pushCustomTypeArray("MeshRenderer", L, (*t)->get<std::vector<MeshRenderer*>>(variableName));
            }
            else if (c.second == "MeshAnimator")
            {
                return pushCustomTypeArray("MeshAnimator", L, (*t)->get<std::vector<MeshAnimator*>>(variableName));
            }
            else if (c.second == "LuaScript")
            {
                return pushCustomTypeArray("LuaScript", L, (*t)->get<std::vector<LuaScript*>>(variableName));
            }
            else if (c.second == "Camera")
            {
                return pushCustomTypeArray("Camera", L, (*t)->get<std::vector<Camera*>>(variableName));
            }
            else if (c.second == "ParticleEmitter_Box")
            {
                return pushCustomTypeArray("ParticleEmitter_Box", L, (*t)->get<std::vector<BoxParticleEmitter*>>(variableName));
            }
            else if (c.second == "ParticleEmitter_Circle")
            {
                return pushCustomTypeArray("ParticleEmitter_Circle", L, (*t)->get<std::vector<CircleParticleEmitter*>>(variableName));
            }
            else if (c.second == "PathFinding")
            {
                return pushCustomTypeArray("PathFinding", L, (*t)->get<std::vector<Pathfinding*>>(variableName));
            }
            else if (c.second == "BoxCollider")
            {
                return pushCustomTypeArray("BoxCollider", L, (*t)->get<std::vector<BoxCollider*>>(variableName));
            }
            else if (c.second == "SphereCollider")
            {
                return pushCustomTypeArray("SphereCollider", L, (*t)->get<std::vector<SphereCollider*>>(variableName));
            }
            else if (c.second == "TextRenderer")
            {
                return pushCustomTypeArray("TextRenderer", L, (*t)->get<std::vector<TextRenderer*>>(variableName));
            }
            else if (c.second == "Attractor")
            {
                return pushCustomTypeArray("Attractor", L, (*t)->get<std::vector<Attractor*>>(variableName));
            }
            else if (c.second == "DirectionalLight")
            {
                return pushCustomTypeArray("DirectionalLight", L, (*t)->get<std::vector<DirectionalLight*>>(variableName));
            }
            else if (c.second == "PointLight")
            {
                return pushCustomTypeArray("PointLight", L, (*t)->get<std::vector<PointLight*>>(variableName));
            }
            else if (c.second == "SpotLight")
            {
                return pushCustomTypeArray("SpotLight", L, (*t)->get<std::vector<SpotLight*>>(variableName));
            }

            else
                PrintError(variableName, " lua_GetVariable :Type not supported (no to default types) " + (*t)->filename);
            return 0;
        }
    }

    PrintError(variableName, " lua_GetVariable : Variable does not exist " + (*t)->filename);
    return 0;
}

int LuaScript::lua_SetVariable(lua_State* L)
{
    LuaScript** t = checkluaScript(L, 1);
    std::string variableName = luaL_checkstring(L, 2);

    for (auto& c : (*t)->variables)
    {
        if (c.first == variableName)
        {
            // Comment this block out if not working (Line : 2872 - 2895) and put in (Line : 2897 - 2920)
            // https://www.youtube.com/watch?v=poz6W0znOfk // https://www.youtube.com/watch?v=v4xZUr0BEfE
            if (lua_isstring(L, 3))
            {
                (*t)->set<std::string>(variableName, std::string{luaL_checkstring(L, 3)});
                return 0;
            }
            if (lua_isnumber(L, 3))
            {
                (*t)->set<float>(variableName, (float)lua_tonumber(L, 3));
                return 0;
            }
            if (lua_isinteger(L, 3))
            {
                (*t)->set<int>(variableName, (int)lua_tointeger(L, 3));
                return 0;
            }
            if (lua_isboolean(L, 3))
            {
                (*t)->set<bool>(variableName, lua_toboolean(L, 3));
                return 0;
            }
            if (lua_isnil(L, 3))
            {
                lua_pushnil(L);
                lua_setglobal(L, variableName.c_str());
                return 0;
            }

            auto t1 = checkVector3(L, 3, false);
            if (t1)
            {
                (*t)->set<Vector3>(variableName, *t1);
                return 0;
            }
            auto t2 = checkColor(L, 3, false);
            if (t2)
            {
                (*t)->set<Vector4>(variableName, *t2);
                return 0;
            }
            auto t3 = checkResAnimation(L, 3, false);
            if (t3)
            {
                (*t)->set<HAnimationSet>(variableName, *t3);
                return 0;
            }
            auto t4 = checkRayCastdata(L, 3, false);
            if (t4)
            {
                (*t)->set<RaycastData_tmp>(variableName, *t4);
                return 0;
            }
            auto t5 = checkGameObject(L, 3, false);
            if (t5)
            {
                (*t)->set<GameObject*>(variableName, *t5);
                return 0;
            }
            auto t6 = checkTransform(L, 3, false);
            if (t6)
            {
                (*t)->set<Transform*>(variableName, *t6);
                return 0;
            }
            auto t7 = checkRigidbody(L, 3, false);
            if (t7)
            {
                (*t)->set<RigidbodyComponent*>(variableName, *t7);
                return 0;
            }
            auto t8 = checkAudioListener(L, 3, false);
            if (t8)
            {
                (*t)->set<AudioListener*>(variableName, *t8);
                return 0;
            }
            auto t9 = checkAudioEmitter(L, 3, false);
            if (t9)
            {
                (*t)->set<AudioEmitter*>(variableName, *t9);
                return 0;
            }
            auto t10 = checkMeshRenderer(L, 3, false);
            if (t10)
            {
                (*t)->set<MeshRenderer*>(variableName, *t10);
                return 0;
            }
            auto t11 = checkMeshAnimator(L, 3, false);
            if (t11)
            {
                (*t)->set<MeshAnimator*>(variableName, *t11);
                return 0;
            }
            auto t12 = checkluaScript(L, 3, false);
            if (t12)
            {
                (*t)->set<LuaScript*>(variableName, *t12);
                return 0;
            }
            auto t13 = checkCamera(L, 3, false);
            if (t13)
            {
                (*t)->set<Camera*>(variableName, *t13);
                return 0;
            }
            auto t14 = checkBoxParticle(L, 3, false);
            if (t14)
            {
                (*t)->set<BoxParticleEmitter*>(variableName, *t14);
                return 0;
            }
            auto t15 = checkCircleParticle(L, 3, false);
            if (t15)
            {
                (*t)->set<CircleParticleEmitter*>(variableName, *t15);
                return 0;
            }
            auto t16 = checkPathFinding(L, 3, false);
            if (t16)
            {
                (*t)->set<Pathfinding*>(variableName, *t16);
                return 0;
            }
            auto t17 = checkBoxCollider(L, 3, false);
            if (t17)
            {
                (*t)->set<BoxCollider*>(variableName, *t17);
                return 0;
            }
            auto t18 = checkSphereCollider(L, 3, false);
            if (t18)
            {
                (*t)->set<SphereCollider*>(variableName, *t18);
                return 0;
            }
            auto t19 = checkTextRenderer(L, 3, false);
            if (t19)
            {
                (*t)->set<TextRenderer*>(variableName, *t19);
                return 0;
            }
            auto t20 = checkAttractor(L, 3, false);
            if (t20)
            {
                (*t)->set<Attractor*>(variableName, *t20);
                return 0;
            }
            auto t21 = checkDirectionalLight(L, 3, false);
            if (t21)
            {
                (*t)->set<DirectionalLight*>(variableName, *t21);
                return 0;
            }
            auto t22 = checkPointLight(L, 3, false);
            if (t22)
            {
                (*t)->set<PointLight*>(variableName, *t22);
                return 0;
            }
            auto t23 = checkSpotLight(L, 3, false);
            if (t23)
            {
                (*t)->set<SpotLight*>(variableName, *t23);
                return 0;
            }

            PrintError(variableName, " lua_SetVariable : variable exist but cannot set " + (*t)->filename);
            return 0;

            /*
            if (c.second == "bool"                  ) { (*t)->set<bool       >(variableName,              lua_toboolean   (L, 3) ); return 0; }
            if (c.second == "float"                 ) { (*t)->set<float      >(variableName, (float)      lua_tonumber    (L, 3) ); return 0; }
            if (c.second == "string"                ) { (*t)->set<std::string>(variableName, std::string{ luaL_checkstring(L, 3)}); return 0; }
            if (c.second == "int"                   ) { (*t)->set<int        >(variableName, (int)        lua_tointeger   (L, 3) ); return 0; }
            if (c.second == "vector3"               ) { auto tmp = *checkVector3       (L, 3); (*t)->set<Vector3               >(variableName, tmp); return 0; }
            if (c.second == "color"                 ) { auto tmp = *checkColor         (L, 3); (*t)->set<Vector4               >(variableName, tmp); return 0; }
            if (c.second == "Resource_Animation"    ) { auto tmp = *checkResAnimation  (L, 3); (*t)->set<HAnimationSet         >(variableName, tmp); return 0; }
            if (c.second == "rayCastData"           ) { auto tmp = *checkRayCastdata   (L, 3); (*t)->set<RaycastData_tmp       >(variableName, tmp); return 0; }
            if (c.second == "GameObject"            ) { auto tmp = *checkGameObject    (L, 3); (*t)->set<GameObject*           >(variableName, tmp); return 0; }
            if (c.second == "Transform"             ) { auto tmp = *checkTransform     (L, 3); (*t)->set<Transform*            >(variableName, tmp); return 0; }
            if (c.second == "RigidBody"             ) { auto tmp = *checkRigidbody     (L, 3); (*t)->set<RigidbodyComponent*   >(variableName, tmp); return 0; }
            if (c.second == "AudioListener"         ) { auto tmp = *checkAudioListener (L, 3); (*t)->set<AudioListener*        >(variableName, tmp); return 0; }
            if (c.second == "AudioEmitter"          ) { auto tmp = *checkAudioEmitter  (L, 3); (*t)->set<AudioEmitter*         >(variableName, tmp); return 0; }
            if (c.second == "MeshRenderer"          ) { auto tmp = *checkMeshRenderer  (L, 3); (*t)->set<MeshRenderer*         >(variableName, tmp); return 0; }
            if (c.second == "MeshAnimator"          ) { auto tmp = *checkMeshAnimator  (L, 3); (*t)->set<MeshAnimator*         >(variableName, tmp); return 0; }
            if (c.second == "LuaScript"             ) { auto tmp = *checkluaScript     (L, 3); (*t)->set<LuaScript*            >(variableName, tmp); return 0; }
            if (c.second == "Camera"                ) { auto tmp = *checkCamera        (L, 3); (*t)->set<Camera*               >(variableName, tmp); return 0; }
            if (c.second == "ParticleEmitter_Box"   ) { auto tmp = *checkBoxParticle   (L, 3); (*t)->set<BoxParticleEmitter*   >(variableName, tmp); return 0; }
            if (c.second == "ParticleEmitter_Circle") { auto tmp = *checkCircleParticle(L, 3); (*t)->set<CircleParticleEmitter*>(variableName, tmp); return 0; }
            if (c.second == "PathFinding"           ) { auto tmp = *checkPathFinding   (L, 3); (*t)->set<Pathfinding*          >(variableName, tmp); return 0; }
            if (c.second == "BoxCollider"           ) { auto tmp = *checkBoxCollider   (L, 3); (*t)->set<BoxCollider*          >(variableName, tmp); return 0; }
            if (c.second == "SphereCollider"        ) { auto tmp = *checkSphereCollider(L, 3); (*t)->set<SphereCollider*       >(variableName, tmp); return 0; }
            */
        }
    }
    PrintError(variableName, " lua_SetVariable : Variable does not exist " + (*t)->filename);
    return 0;
}

// ============================================================================
// CAMERA
// ============================================================================
int LuaScript::camera_metatable(lua_State* L)
{
    luaL_newmetatable(L, "Camera");
    lua_pushstring(L, "__index");
    lua_pushvalue(L, -2);
    lua_settable(L, -3);
    luaL_setfuncs(L, cameralib, 0);
    return 1;
}

Camera** LuaScript::checkCamera(lua_State* L, int index, bool assert)
{
    return check<Camera**>("Camera", L, index, assert);
}

int LuaScript::pushCamera(lua_State* L, Camera* input)
{
    return pushCustomType<Camera*>("Camera", L, input);
}

int LuaScript::camera_GetLookAt(lua_State* L)
{
    Camera** t = checkCamera(L, 1);
    return pushVector3(L, (*t)->GetLookAtDirection());
}

int LuaScript::camera_SetLookAt(lua_State* L)
{
    Camera** t = checkCamera(L, 1);
    Vector3* d = checkVector3(L, 2);

    (*t)->SetLookAtDirection(*d);
    return 0;
}

int LuaScript::camera_SetUp(lua_State* L)
{
    Camera** t = checkCamera(L, 1);
    Vector3* d = checkVector3(L, 2);

    (*t)->SetUpDirection(*d);
    return 0;
}

int LuaScript::camera_GetViewportSize(lua_State* L)
{
    Camera** t = checkCamera(L, 1);
    auto v = (*t)->GetViewportSize();
    return pushVector3(L, Vector3(float(v.x), float(v.y), 0.0f));
}

int LuaScript::camera_SetViewportSize(lua_State* L)
{
    Camera** t = checkCamera(L, 1);
    Vector3 size = *checkVector3(L, 2);
    (*t)->SetViewportSize(iVector2{(int)size.x, (int)size.y});
    return 0;
}

int LuaScript::camera_SetUICamera(lua_State* L)
{
    Camera** t = checkCamera(L, 1);
    bool b = lua_toboolean(L, 2);
    (*t)->m_IsUICamera = b;
    return 0;
}

int LuaScript::camera_SetDirectionalLightIntensity(lua_State* L)
{
    Camera** t = checkCamera(L, 1);
    auto light = (*t)->GetOwner()->GetComponent<DirectionalLight>();
    if (light)
    {
        light->SetIntensity(static_cast<float>(lua_tonumber(L, 2)));
    }
    return 0;
}

int LuaScript::camera_SetDirectionalLightDirection(lua_State* L)
{
    Camera** t = checkCamera(L, 1);
    auto light = (*t)->GetOwner()->GetComponent<DirectionalLight>();
    Vector3 direction = *checkVector3(L, 2);
    if (light)
    {
        light->SetDirection(direction);
    }
    return 0;
}

int LuaScript::camera_SetShadowCasted(lua_State* L)
{
    Camera** t = checkCamera(L, 1);
    auto light = (*t)->GetOwner()->GetComponent<DirectionalLight>();
    bool b = lua_toboolean(L, 2);
    if (light)
    {
        light->SetCastShadows(b);
    }
    return 0;
}

int LuaScript::camera_SetColor(lua_State* L)
{
    Camera** t = checkCamera(L, 1);
    auto light = (*t)->GetOwner()->GetComponent<DirectionalLight>();
    Color4* col = checkColor(L, 2);
    if (light)
    {
        light->SetLightColor(Color3(col->x, col->y, col->z));
    }
    return 0;
}

// ============================================================================
// PARTICLE EMIITER
// ============================================================================
ParticleEmitter** LuaScript::checkIsParticleEmitter(lua_State* L, int index)
{
    void* com = (void*)checkBoxParticle(L, index, false);
    com = com ? com : (void*)checkCircleParticle(L, index, false);

    if (com) return (ParticleEmitter**)com;

    return nullptr;
}

int LuaScript::particle_GetStartMinColor(lua_State* L)
{
    ParticleEmitter** com = checkIsParticleEmitter(L, 1);
    if (com) return pushColor(L, (*com)->m_StartMinColor);
    return 0;
}

int LuaScript::particle_GetStartMaxColor(lua_State* L)
{
    ParticleEmitter** com = checkIsParticleEmitter(L, 1);
    if (com) return pushColor(L, (*com)->m_StartMaxColor);
    return 0;
}

int LuaScript::particle_GetEndMinColor(lua_State* L)
{
    ParticleEmitter** com = checkIsParticleEmitter(L, 1);
    if (com) return pushColor(L, (*com)->m_EndMinColor);
    return 0;
}

int LuaScript::particle_GetEndMaxColor(lua_State* L)
{
    ParticleEmitter** com = checkIsParticleEmitter(L, 1);
    if (com) return pushColor(L, (*com)->m_EndMaxColor);
    return 0;
}

int LuaScript::particle_GetMinSize(lua_State* L)
{
    ParticleEmitter** com = checkIsParticleEmitter(L, 1);
    if (com)
    {
        lua_pushnumber(L, (*com)->m_MinSize);
        return 1;
    }
    return 0;
}

int LuaScript::particle_GetMaxSize(lua_State* L)
{
    ParticleEmitter** com = checkIsParticleEmitter(L, 1);
    if (com)
    {
        lua_pushnumber(L, (*com)->m_MaxSize);
        return 1;
    }
    return 0;
}

int LuaScript::particle_GetEndTexture(lua_State* L)
{
    ParticleEmitter** com = checkIsParticleEmitter(L, 1);
    if (com)
    {
        lua_pushstring(L, (*com)->m_EndTexture->GetName().c_str());
        return 1;
    }
    return 0;
}

int LuaScript::particle_GetTextureFade(lua_State* L)
{
    ParticleEmitter** com = checkIsParticleEmitter(L, 1);
    if (com)
    {
        lua_pushnumber(L, (*com)->m_TextureFade);
        return 1;
    }
    return 0;
}

int LuaScript::particle_SetStartMinColor(lua_State* L)
{
    ParticleEmitter** com = checkIsParticleEmitter(L, 1);
    if (com) (*com)->m_StartMinColor = *checkColor(L, 2);

    return 0;
}

int LuaScript::particle_SetStartMaxColor(lua_State* L)
{
    ParticleEmitter** com = checkIsParticleEmitter(L, 1);
    if (com) (*com)->m_StartMaxColor = *checkColor(L, 2);

    return 0;
}

int LuaScript::particle_SetEndMinColor(lua_State* L)
{
    ParticleEmitter** com = checkIsParticleEmitter(L, 1);
    if (com) (*com)->m_EndMinColor = *checkColor(L, 2);

    return 0;
}

int LuaScript::particle_SetEndMaxColor(lua_State* L)
{
    ParticleEmitter** com = checkIsParticleEmitter(L, 1);
    if (com) (*com)->m_EndMaxColor = *checkColor(L, 2);

    return 0;
}

int LuaScript::particle_SetMinSize(lua_State* L)
{
    ParticleEmitter** com = checkIsParticleEmitter(L, 1);
    if (com) (*com)->m_MinSize = (float)luaL_checknumber(L, 2);

    return 0;
}

int LuaScript::particle_SetMaxSize(lua_State* L)
{
    ParticleEmitter** com = checkIsParticleEmitter(L, 1);
    if (com) (*com)->m_MaxSize = (float)luaL_checknumber(L, 2);

    return 0;
}

int LuaScript::particle_SetEndTexture(lua_State* L)
{
    ParticleEmitter** com = checkIsParticleEmitter(L, 1);
    if (com) (*com)->m_EndTexture = *(ResourceManager::Instance().GetResource<HTexture>(luaL_checkstring(L, 2)));

    return 0;
}

int LuaScript::particle_SetTextureFade(lua_State* L)
{
    ParticleEmitter** com = checkIsParticleEmitter(L, 1);
    if (com) (*com)->m_TextureFade = static_cast<float>(luaL_checknumber(L, 2));

    return 0;
}

int LuaScript::particle_SetFloorHeight(lua_State* L)
{
    ParticleEmitter** com = checkIsParticleEmitter(L, 1);
    if (com) (*com)->m_FloorHeight = static_cast<float>(luaL_checknumber(L, 2));

    return 0;
}

// ============================================================================
// BOX PARTICLE EMIITER
// ============================================================================
int LuaScript::boxParticle_metatable(lua_State* L)
{
    luaL_newmetatable(L, "ParticleEmitter_Box");
    lua_pushstring(L, "__index");
    lua_pushvalue(L, -2);
    lua_settable(L, -3);
    luaL_setfuncs(L, boxParticlelib, 0);
    return 1;
}

BoxParticleEmitter** LuaScript::checkBoxParticle(lua_State* L, int index, bool assert)
{
    return check<BoxParticleEmitter**>("ParticleEmitter_Box", L, index, assert);
}

int LuaScript::pushBoxParticle(lua_State* L, BoxParticleEmitter* input)
{
    return pushCustomType<BoxParticleEmitter*>("ParticleEmitter_Box", L, input);
}

int LuaScript::boxParticle_GetPos(lua_State* L)
{
    BoxParticleEmitter** t = checkBoxParticle(L, 1);
    return pushVector3(L, (*t)->m_Position);
}

int LuaScript::boxParticle_GetStartPosOff(lua_State* L)
{
    BoxParticleEmitter** t = checkBoxParticle(L, 1);
    return pushVector3(L, (*t)->m_StartPositionOffset);
}

int LuaScript::boxParticle_GetMinVel(lua_State* L)
{
    BoxParticleEmitter** t = checkBoxParticle(L, 1);
    return pushVector3(L, (*t)->m_MinVelocity);
}

int LuaScript::boxParticle_GetMaxVel(lua_State* L)
{
    BoxParticleEmitter** t = checkBoxParticle(L, 1);
    return pushVector3(L, (*t)->m_MaxVelocity);
}

int LuaScript::boxParticle_GetMinTime(lua_State* L)
{
    BoxParticleEmitter** t = checkBoxParticle(L, 1);
    lua_pushnumber(L, (*t)->m_MinTime);
    return 1;
}

int LuaScript::boxParticle_GetMaxTime(lua_State* L)
{
    BoxParticleEmitter** t = checkBoxParticle(L, 1);
    lua_pushnumber(L, (*t)->m_MaxTime);
    return 1;
}

int LuaScript::boxParticle_SetPos(lua_State* L)
{
    BoxParticleEmitter** t = checkBoxParticle(L, 1);
    Vector3* v = checkVector3(L, 2);

    (*t)->m_Position = *v;
    return 0;
}

int LuaScript::boxParticle_SetStartPosOff(lua_State* L)
{
    BoxParticleEmitter** t = checkBoxParticle(L, 1);
    Vector3* v = checkVector3(L, 2);

    (*t)->m_StartPositionOffset = *v;
    return 0;
}

int LuaScript::boxParticle_SetMinVel(lua_State* L)
{
    BoxParticleEmitter** t = checkBoxParticle(L, 1);
    Vector3* v = checkVector3(L, 2);

    (*t)->m_MinVelocity = *v;
    return 0;
}

int LuaScript::boxParticle_SetMaxVel(lua_State* L)
{
    BoxParticleEmitter** t = checkBoxParticle(L, 1);
    Vector3* v = checkVector3(L, 2);

    (*t)->m_MaxVelocity = *v;
    return 0;
}

int LuaScript::boxParticle_SetMinTime(lua_State* L)
{
    BoxParticleEmitter** t = checkBoxParticle(L, 1);
    float v = (float)lua_tonumber(L, 2);

    (*t)->m_MinTime = v;
    return 0;
}

int LuaScript::boxParticle_SetMaxTime(lua_State* L)
{
    BoxParticleEmitter** t = checkBoxParticle(L, 1);
    float v = (float)lua_tonumber(L, 2);

    (*t)->m_MaxTime = v;
    return 0;
}

int LuaScript::boxParticle_GetEmitRate(lua_State* L)
{
    BoxParticleEmitter** t = checkBoxParticle(L, 1);
    lua_pushnumber(L, (*t)->m_EmissionRate);
    return 1;
}

int LuaScript::boxParticle_SetEmitRate(lua_State* L)
{
    BoxParticleEmitter** t = checkBoxParticle(L, 1);
    float v = (float)lua_tonumber(L, 2);

    (*t)->m_EmissionRate = v;
    return 0;
}

int LuaScript::boxParticle_AddAttractor(lua_State* L)
{
    BoxParticleEmitter** t = checkBoxParticle(L, 1);
    unsigned id = static_cast<unsigned int>(lua_tonumber(L, 2));
    (*t)->m_Attractors.push_back(id);
    return 0;
}

int LuaScript::boxParticle_RemoveAttractor(lua_State* L)
{
    BoxParticleEmitter** t = checkBoxParticle(L, 1);
    unsigned id = static_cast<unsigned int>(lua_tonumber(L, 2));
    auto itr = std::find((*t)->m_Attractors.begin(), (*t)->m_Attractors.end(), id);
    (*t)->m_Attractors.erase(itr);
    return 0;
}

int LuaScript::circleParticle_metatable(lua_State* L)
{
    luaL_newmetatable(L, "ParticleEmitter_Circle");
    lua_pushstring(L, "__index");
    lua_pushvalue(L, -2);
    lua_settable(L, -3);
    luaL_setfuncs(L, circleParticlelib, 0);
    return 1;
}

CircleParticleEmitter** LuaScript::checkCircleParticle(lua_State* L, int index, bool assert)
{
    return check<CircleParticleEmitter**>("ParticleEmitter_Circle", L, index, assert);
}

int LuaScript::pushCircleParticle(lua_State* L, CircleParticleEmitter* input)
{
    return pushCustomType<CircleParticleEmitter*>("ParticleEmitter_Circle", L, input);
}

int LuaScript::circleParticle_GetEmitRate(lua_State* L)
{
    CircleParticleEmitter** t = checkCircleParticle(L, 1);
    lua_pushnumber(L, (*t)->m_EmissionRate);
    return 1;
}

int LuaScript::circleParticle_SetEmitRate(lua_State* L)
{
    CircleParticleEmitter** t = checkCircleParticle(L, 1);
    float v = (float)lua_tonumber(L, 2);

    (*t)->m_EmissionRate = v;
    return 0;
}

int LuaScript::circleParticle_AddAttractor(lua_State* L)
{
    CircleParticleEmitter** t = checkCircleParticle(L, 1);
    unsigned id = static_cast<unsigned int>(lua_tonumber(L, 2));
    (*t)->m_Attractors.push_back(id);
    return 0;
}

int LuaScript::circleParticle_RemoveAttractor(lua_State* L)
{
    CircleParticleEmitter** t = checkCircleParticle(L, 1);
    unsigned id = static_cast<unsigned int>(lua_tonumber(L, 2));
    auto itr = std::find((*t)->m_Attractors.begin(), (*t)->m_Attractors.end(), id);
    (*t)->m_Attractors.erase(itr);
    return 0;
}

// ============================================================================
// PATH FINDING
// ============================================================================
int LuaScript::pathFinding_metatable(lua_State* L)
{
    luaL_newmetatable(L, "PathFinding");
    lua_pushstring(L, "__index");
    lua_pushvalue(L, -2);
    lua_settable(L, -3);
    luaL_setfuncs(L, pathFindinglib, 0);
    return 1;
}

Pathfinding** LuaScript::checkPathFinding(lua_State* L, int index, bool assert)
{
    return check<Pathfinding**>("PathFinding", L, index, assert);
}

int LuaScript::pushPathFinding(lua_State* L, Pathfinding* input)
{
    return pushCustomType<Pathfinding*>("PathFinding", L, input);
}

int LuaScript::pathFinding_AStarFindPath(lua_State* L)
{
    Pathfinding** t = checkPathFinding(L, 1);
    Vector3* p = checkVector3(L, 2);
    lua_pushboolean(L, (*t)->AStarFindPath(*p, static_cast<bool>(lua_toboolean(L, 3))));
    return 1;
}

int LuaScript::pathFinding_GetPath(lua_State* L)
{
    Pathfinding** t = checkPathFinding(L, 1);
    pushCustomTypeArray("vector3", L, (*t)->m_paths);
    return 1;
}

int LuaScript::pathFinding_GetPathByIndex(lua_State* L)
{
    Pathfinding** t = checkPathFinding(L, 1);
    int index = static_cast<int>(lua_tointeger(L, 2)) - 1;
    pushVector3(L, (*t)->m_paths[index]);
    return 1;
}

int LuaScript::pathFinding_GetPathSize(lua_State* L)
{
    Pathfinding** t = checkPathFinding(L, 1);
    lua_pushinteger(L, (*t)->m_paths.size());
    return 1;
}

int LuaScript::pathFinding_ChangeLocalToBaseMap(lua_State* L)
{
    Pathfinding** t = checkPathFinding(L, 1);
    (*t)->ChangeLocalToBaseGrid();
    return 1;
}

// ============================================================================
// Box Collider
// ============================================================================
int LuaScript::boxCollider_metatable(lua_State* L)
{
    // luaL_newmetatable(L, "BoxCollider");
    // lua_pushstring(L, "__index");
    // lua_pushvalue(L, -2);
    // lua_settable(L, -3);
    // luaL_setfuncs(L, boxColliderlib, 0);
    // return 1;

    luaL_newmetatable(L, "BoxCollider");
    luaL_setfuncs(L, boxColliderlib, 0);
    lua_pushvalue(L, -1);
    lua_setfield(L, -2, "__index");
    return 1;
}

BoxCollider** LuaScript::checkBoxCollider(lua_State* L, int index, bool assert)
{
    return check<BoxCollider**>("BoxCollider", L, index, assert);
}

int LuaScript::pushBoxCollider(lua_State* L, BoxCollider* input)
{
    return pushCustomType<BoxCollider*>("BoxCollider", L, input);
}

int LuaScript::boxCollider_GetHalfExtent(lua_State* L)
{
    BoxCollider** t = checkBoxCollider(L, 1);
    pushVector3(L, (*t)->GetHalfExtent());
    return 1;
}

// ============================================================================
// Sphere Collider
// ============================================================================
int LuaScript::sphereCollider_metatable(lua_State* L)
{
    luaL_newmetatable(L, "SphereCollider");
    lua_pushstring(L, "__index");
    lua_pushvalue(L, -2);
    lua_settable(L, -3);
    luaL_setfuncs(L, sphereColliderlib, 0);
    return 1;
}

SphereCollider** LuaScript::checkSphereCollider(lua_State* L, int index, bool assert)
{
    return check<SphereCollider**>("SphereCollider", L, index, assert);
}

int LuaScript::pushSphereCollider(lua_State* L, SphereCollider* input)
{
    return pushCustomType<SphereCollider*>("SphereCollider", L, input);
}

int LuaScript::sphereCollider_SetRadius(lua_State* L)
{
    SphereCollider** t = checkSphereCollider(L, 1);
    float r = (float)lua_tonumber(L, 2);
    (*t)->SetRadius(r);
    return 0;
}

int LuaScript::sphereCollider_GetRadius(lua_State* L)
{
    SphereCollider** t = checkSphereCollider(L, 1);
    lua_pushnumber(L, (*t)->GetRadius());
    return 1;
}

// ============================================================================
// Text Renderer
// ============================================================================
int LuaScript::textRenderer_metatable(lua_State* L)
{
    // luaL_newmetatable(L, "TextRenderer");
    // lua_pushstring(L, "__index");
    // lua_pushvalue(L, -2);
    // lua_settable(L, -3);
    // luaL_setfuncs(L, textRendererlib, 0);
    // return 1;
    luaL_newmetatable(L, "TextRenderer");
    luaL_setfuncs(L, textRendererlib, 0);
    lua_pushvalue(L, -1);
    lua_setfield(L, -2, "__index");
    return 1;
}

TextRenderer** LuaScript::checkTextRenderer(lua_State* L, int index, bool assert)
{
    return check<TextRenderer**>("TextRenderer", L, index, assert);
}

int LuaScript::pushTextRenderer(lua_State* L, TextRenderer* input)
{
    return pushCustomType<TextRenderer*>("TextRenderer", L, input);
}

int LuaScript::TextRenderer_SetText(lua_State* L)
{
    TextRenderer** t = checkTextRenderer(L, 1);
    (*t)->m_Text = std::string{luaL_checkstring(L, 2)};
    return 0;
}

int LuaScript::TextRenderer_GetText(lua_State* L)
{
    TextRenderer** t = checkTextRenderer(L, 1);
    lua_pushstring(L, (*t)->m_Text.c_str());
    return 0;
}

int LuaScript::TextRenderer_SetSize(lua_State* L)
{
    TextRenderer** t = checkTextRenderer(L, 1);
    (*t)->m_FontSize = static_cast<float>(lua_tonumber(L, 2));
    return 0;
}

int LuaScript::TextRenderer_GetSize(lua_State* L)
{
    TextRenderer** t = checkTextRenderer(L, 1);
    lua_pushnumber(L, (*t)->m_FontSize);
    return 0;
}

int LuaScript::TextRenderer_SetColor(lua_State* L)
{
    TextRenderer** t = checkTextRenderer(L, 1);
    Vector4 tmp = *checkColor(L, 2);
    (*t)->m_FontColor = tmp;
    return 0;
}

int LuaScript::TextRenderer_GetColor(lua_State* L)
{
    TextRenderer** t = checkTextRenderer(L, 1);
    return pushColor(L, (*t)->m_FontColor);
}

int LuaScript::TextRenderer_SetFaceCamera(lua_State* L)
{
    TextRenderer** t = checkTextRenderer(L, 1);
    (*t)->m_FaceCamera = lua_toboolean(L, 2);
    return 0;
}

int LuaScript::TextRenderer_GetFaceCamera(lua_State* L)
{
    TextRenderer** t = checkTextRenderer(L, 1);
    lua_pushboolean(L, (*t)->m_FaceCamera);
    return 0;
}

int LuaScript::TextRenderer_SetAnchorPoint(lua_State* L)
{
    TextRenderer** t = checkTextRenderer(L, 1);
    std::string index{luaL_checkstring(L, 2)};
    if (index == "Top")
        (*t)->m_AnchorPoint = TextAnchorPoint::eTextAnchorPoint_Top;
    else if (index == "Bot")
        (*t)->m_AnchorPoint = TextAnchorPoint::eTextAnchorPoint_Bottom;
    else if (index == "Rgt")
        (*t)->m_AnchorPoint = TextAnchorPoint::eTextAnchorPoint_Right;
    else if (index == "Lft")
        (*t)->m_AnchorPoint = TextAnchorPoint::eTextAnchorPoint_Left;
    return 0;
}

int LuaScript::TextRenderer_GetAnchorPoint(lua_State* L)
{
    TextRenderer** t = checkTextRenderer(L, 1);
    switch ((*t)->m_AnchorPoint)
    {
        case TextAnchorPoint::eTextAnchorPoint_Top: lua_pushstring(L, "Top"); break;
        case TextAnchorPoint::eTextAnchorPoint_Bottom: lua_pushstring(L, "Bot"); break;
        case TextAnchorPoint::eTextAnchorPoint_Right: lua_pushstring(L, "Rgt"); break;
        case TextAnchorPoint::eTextAnchorPoint_Left: lua_pushstring(L, "Lft"); break;
        default: lua_pushstring(L, "Undefined anchor point"); break;
    }
    return 1;
}

int LuaScript::TextRenderer_SetFont(lua_State* L)
{
    TextRenderer** t = checkTextRenderer(L, 1);
    std::string font{luaL_checkstring(L, 2)};
    HFont f = ResourceManager::Instance().GetResource<Font>(font);
    (*t)->m_Font = f;
    return 0;
}

int LuaScript::TextRenderer_GetFont(lua_State* L)
{
    TextRenderer** t = checkTextRenderer(L, 1);
    lua_pushstring(L, (*t)->m_Font->GetName().c_str());
    return 1;
}

int LuaScript::TextRenderer_SetOutline(lua_State* L)
{
    TextRenderer** t = checkTextRenderer(L, 1);
    (*t)->m_OutlineEnabled = lua_toboolean(L, 2);
    return 0;
}

int LuaScript::TextRenderer_SetOutlineSize(lua_State* L)
{
    TextRenderer** t = checkTextRenderer(L, 1);
    (*t)->m_OutlineWidth = static_cast<float>(lua_tonumber(L, 2));
    return 0;
}

// ============================================================================
// ATTRACTOR
// ============================================================================
int LuaScript::attractor_metatable(lua_State* L)
{
    luaL_newmetatable(L, "Attractor");
    lua_pushstring(L, "__index");
    lua_pushvalue(L, -2);
    lua_settable(L, -3);
    luaL_setfuncs(L, attractorlib, 0);
    return 1;
}

Attractor** LuaScript::checkAttractor(lua_State* L, int index, bool assert)
{
    return check<Attractor**>("Attractor", L, index, assert);
}

int LuaScript::pushAttractor(lua_State* L, Attractor* input)
{
    return pushCustomType<Attractor*>("Attractor", L, input);
}

int LuaScript::attractor_SetForce(lua_State* L)
{
    Attractor** t = checkAttractor(L, 1);
    double force = lua_tonumber(L, 2);
    (*t)->m_Force = static_cast<float>(force);
    return 0;
}

int LuaScript::attractor_GetForce(lua_State* L)
{
    Attractor** t = checkAttractor(L, 1);
    lua_pushnumber(L, (*t)->GetForce());
    return 1;
}

// ============================================================================
// LIGHT
// ============================================================================
LightBase** LuaScript::checkLightBase(lua_State* L, int index, bool assert)
{
    void* base = checkDirectionalLight(L, index, false);
    base = base ? base : checkPointLight(L, index, false);
    base = base ? base : checkSpotLight(L, index, false);
    return base ? static_cast<LightBase**>(base) : nullptr;
}

int LuaScript::Light_GetIntensity(lua_State* L)
{
    LightBase** t = checkLightBase(L, 1);
    lua_pushnumber(L, (*t)->GetIntensity());
    return 1;
}
int LuaScript::Light_SetIntensity(lua_State* L)
{
    LightBase** t = checkLightBase(L, 1);
    (*t)->SetIntensity((float)lua_tonumber(L, 2));
    return 0;
}
int LuaScript::Light_GetLightColor(lua_State* L)
{
    LightBase** t = checkLightBase(L, 1);
    Color3 tCol = (*t)->GetLightColor();
    pushColor(L, Color4{tCol.x, tCol.y, tCol.z, 1});
    return 1;
}
int LuaScript::Light_SetLightColor(lua_State* L)
{
    LightBase** t = checkLightBase(L, 1);
    Color4* col = checkColor(L, 2);
    (*t)->SetLightColor(Color3(col->x, col->y, col->z));
    return 0;
}
int LuaScript::Light_IsCastShadows(lua_State* L)
{
    LightBase** t = checkLightBase(L, 1);
    lua_pushboolean(L, (*t)->IsCastShadows());
    return 1;
}
int LuaScript::Light_SetCastShadows(lua_State* L)
{
    LightBase** t = checkLightBase(L, 1);
    (*t)->SetCastShadows(lua_toboolean(L, 2));
    return 0;
}
int LuaScript::Light_GetShadowDistance(lua_State* L)
{
    LightBase** t = checkLightBase(L, 1);
    lua_pushnumber(L, (*t)->GetShadowDistance());
    return 1;
}
int LuaScript::Light_GetShadowBias(lua_State* L)
{
    LightBase** t = checkLightBase(L, 1);
    lua_pushnumber(L, (*t)->GetShadowBias());
    return 1;
}
int LuaScript::Light_SetShadowBias(lua_State* L)
{
    LightBase** t = checkLightBase(L, 1);
    (*t)->SetShadowBias((float)lua_tonumber(L, 2));
    return 0;
}

int LuaScript::directionalLight_metatable(lua_State* L)
{
    luaL_newmetatable(L, "DirectionalLight");
    luaL_setfuncs(L, directionalLightlib, 0);
    lua_pushvalue(L, -1);
    lua_setfield(L, -2, "__index");
    return 1;
}

DirectionalLight** LuaScript::checkDirectionalLight(lua_State* L, int index, bool assert)
{
    return check<DirectionalLight**>("DirectionalLight", L, index, assert);
}
int LuaScript::pushDirectionalLight(lua_State* L, DirectionalLight* input)
{
    return pushCustomType<DirectionalLight*>("DirectionalLight", L, input);
}

int LuaScript::directionalLight_GetDirection(lua_State* L)
{
    DirectionalLight** t = checkDirectionalLight(L, 1);
    pushVector3(L, (*t)->GetDirection());
    return 1;
}
int LuaScript::directionalLight_SetDirection(lua_State* L)
{
    DirectionalLight** t = checkDirectionalLight(L, 1);
    (*t)->SetDirection(*checkVector3(L, 2));
    return 0;
}

int LuaScript::pointLight_metatable(lua_State* L)
{
    luaL_newmetatable(L, "PointLight");
    luaL_setfuncs(L, pointLightlib, 0);
    lua_pushvalue(L, -1);
    lua_setfield(L, -2, "__index");
    return 1;
}

PointLight** LuaScript::checkPointLight(lua_State* L, int index, bool assert)
{
    return check<PointLight**>("PointLight", L, index, assert);
}
int LuaScript::pushPointLight(lua_State* L, PointLight* input)
{
    return pushCustomType<PointLight*>("PointLight", L, input);
}

int LuaScript::spotLight_metatable(lua_State* L)
{
    luaL_newmetatable(L, "SpotLight");
    luaL_setfuncs(L, spotLightlib, 0);
    lua_pushvalue(L, -1);
    lua_setfield(L, -2, "__index");
    return 1;
}

SpotLight** LuaScript::checkSpotLight(lua_State* L, int index, bool assert)
{
    return check<SpotLight**>("SpotLight", L, index, assert);
}
int LuaScript::pushSpotLight(lua_State* L, SpotLight* input)
{
    return pushCustomType<SpotLight*>("SpotLight", L, input);
}

int LuaScript::spotLight_GetDirection(lua_State* L)
{
    SpotLight** t = checkSpotLight(L, 1);
    pushVector3(L, (*t)->GetDirection());
    return 1;
}
int LuaScript::spotLight_SetDirection(lua_State* L)
{
    SpotLight** t = checkSpotLight(L, 1);
    (*t)->SetDirection(*checkVector3(L, 2));
    return 0;
}
// ============================================================================
// GAME
// ============================================================================
int LuaScript::ToggleFullscreen(lua_State* L)
{
    bool state = lua_toboolean(L, 1);
    Application::Instance().ToggleFullScreen(state);

    return 0;
}

// ============================================================================
// OTHERS
// ============================================================================
int LuaScript::AISystem_MarkGridValidity(lua_State* L)
{
    bool haveValue = lua_gettop(L) == 3;
    Vector3* pos = checkVector3(L, 1);
    bool state = lua_toboolean(L, 2);
    int value = haveValue ? int(lua_tointeger(L, 3)) : -1;

    state ? AISystem::Instance().MarkValidByPosition(*pos) : AISystem::Instance().MarkInvalidByPosition(*pos, value);
    return 0;
}

int LuaScript::AISystem_GetSlimePaths(lua_State* L)
{
    return pushCustomTypeArray<Vector3>("vector3", L, AISystem::Instance().slimePaths);
}

int LuaScript::AISystem_GetOriginalPathChanged(lua_State* L)
{
    lua_pushboolean(L, AISystem::Instance().isOriginalPathChanged);
    return 1;
}

int LuaScript::AISystem_SetOriginalPathChanged(lua_State* L)
{
    AISystem::Instance().isOriginalPathChanged = static_cast<bool>(lua_toboolean(L, 1));
    return 0;
}

int LuaScript::AISystem_NearestPathIndex(lua_State* L)
{
    lua_pushinteger(L, AISystem::Instance().NearestPathIndex(*checkVector3(L, 1)));
    return 1;
}

int LuaScript::AISystem_RetrieveGridPos(lua_State* L)
{
    pushVector3(L, AISystem::Instance().RetrieveGridPos(*checkVector3(L, 1)));
    return 1;
}

int LuaScript::AISystem_RetrieveGridPosGeneral(lua_State* L)
{
    pushVector3(L, AISystem::Instance().RetrieveGridPosGeneral(*checkVector3(L, 1)));
    return 1;
}

int LuaScript::AISystem_CheckValidGrid(lua_State* L)
{
    lua_pushboolean(L, AISystem::Instance().CheckValidGrid(*checkVector3(L, 1)));
    return 1;
}

int LuaScript::AISystem_PushBackStartPath(lua_State* L)
{
    std::vector<Vector3> elems;
    int type = lua_type(L, 1);
    if (type != LUA_TTABLE)
    {
        std::cout << "AISystem_PushBackStartPath : ERROR - PARAMETER MUST BE AN ARRAY" << std::endl;
        return 0;
    }

    int tableSize = (int)lua_rawlen(L, 1);  /// get size of table
    for (int i = 1; i <= tableSize; i++)
    {
        lua_rawgeti(L, 1, i);
        int Top = lua_gettop(L);
        elems.push_back(*checkVector3(L, Top));
    }

    AISystem::Instance().originalPathsArr.push_back(elems);
    std::cout << std::endl;
    return 0;
}

int LuaScript::AISystem_GetStartPath(lua_State* L)
{
    int index = static_cast<int>(lua_tonumber(L, 1));
    pushCustomTypeArray("vector3", L, AISystem::Instance().originalPathsArr[index - 1]);
    return 1;
}

int LuaScript::AISystem_ClearOriginalPath(lua_State* L)
{
    AISystem::Instance().originalPathsArr.clear();
    return 0;
}

int LuaScript::AISystem_ReplacePathAtIndex(lua_State* L)
{
    int index = static_cast<int>(lua_tonumber(L, 1));

    AISystem::Instance().path_index = index - 1;

    return 0;
}

int LuaScript::AISystem_GetAvilableGridOutsidePos(lua_State* L)
{
    Vector3 pos = *checkVector3(L, 1);
    float dist = static_cast<float>(lua_tonumber(L, 2));
    dist *= dist;

    std::vector<NodePath*> freeGrids;
    for (int i = 0; i < AISystem::Instance().row_size; ++i)
        for (int j = 0; j < AISystem::Instance().col_size; ++j)
            if (AISystem::Instance().m_gridMap[i][j].valid == 1) freeGrids.push_back(&(AISystem::Instance().m_gridMap[i][j]));

    std::vector<NodePath*> availableGrids;
    for (auto& c : freeGrids)
    {
        Vector3 distVec = c->pos - pos;
        if (distVec.SquareLength() >= dist) availableGrids.push_back(c);
    }

    if (availableGrids.size())
        pushVector3(L, availableGrids[rand() % availableGrids.size()]->pos);
    else if (freeGrids.size())
        pushVector3(L, freeGrids[rand() % availableGrids.size()]->pos);
    else
        pushVector3(L, Vector3());

    return 1;
}

int LuaScript::AISystem_GetStartPathByIndex(lua_State* L)
{
    int pathindex = static_cast<int>(lua_tointeger(L, 1)) - 1;
    int index = static_cast<int>(lua_tointeger(L, 2)) - 1;
    pushVector3(L, AISystem::Instance().originalPathsArr[pathindex][index]);
    return 1;
}

int LuaScript::AISystem_GetStartPathSize(lua_State* L)
{
    int pathindex = static_cast<int>(lua_tointeger(L, 1)) - 1;
    lua_pushinteger(L, AISystem::Instance().originalPathsArr[pathindex].size());
    return 1;
}

// ============================================================================
// OTHERS
// ============================================================================
void LuaScript::BlacklistSnapshot(lua_State* L)
{
    Blacklist.clear();
    lua_pushglobaltable(L);
    lua_pushnil(L);
    while (lua_next(L, -2) != 0)
    {
        std::string tmp = lua_tostring(L, -2);  // pop NIL, push name,value
        Blacklist.insert((unsigned)fn(tmp));    // insert to hash table
        lua_pop(L, 1);                          // remove value
    }
    lua_pop(L, 1);  // Remove global table
}

void LuaScript::GetGlobals(lua_State* L)
{
    std::vector<std::string> vars;
    variables.clear();

    lua_pushglobaltable(L);
    lua_pushnil(L);

    while (lua_next(L, -2) != 0)
    {  // pop NIL, push name,value

        std::string tmp = lua_tostring(L, -2);
        unsigned val = (unsigned)fn(tmp);
        // Check if the global is present in our blacklist
        if (Blacklist.find(val) == Blacklist.end())
        {
            // Not present, print it...
            vars.push_back(tmp);
        }
        lua_pop(L, 1);  // remove value
    }

    lua_pop(L, 1);  // remove global table
    for (auto& tmp : vars)
    {
        /// Default types
        lua_getglobal(L, tmp.c_str());

        if (lua_isboolean(L, -1))
            variables.push_back(std::make_pair(tmp, "bool"));
        else if (lua_isnumber(L, -1))
            variables.push_back(std::make_pair(tmp, "float"));
        else if (lua_isstring(L, -1))
            variables.push_back(std::make_pair(tmp, "string"));
        else if (lua_isinteger(L, -1))
            variables.push_back(std::make_pair(tmp, "int"));
        /// None default types
        else if (lua_isuserdata(L, -1))
        {
            if (checkVector3(L, -1, false))
                variables.push_back((std::make_pair(tmp, "vector3")));
            else if (checkColor(L, -1, false))
                variables.push_back((std::make_pair(tmp, "color")));
            else if (checkGameObject(L, -1, false))
                variables.push_back((std::make_pair(tmp, "GameObject")));
            else if (checkTransform(L, -1, false))
                variables.push_back((std::make_pair(tmp, "Transform")));
            else if (checkRigidbody(L, -1, false))
                variables.push_back((std::make_pair(tmp, "RigidBody")));
            else if (checkAudioListener(L, -1, false))
                variables.push_back((std::make_pair(tmp, "AudioListener")));
            else if (checkAudioEmitter(L, -1, false))
                variables.push_back((std::make_pair(tmp, "AudioEmitter")));
            else if (checkMeshRenderer(L, -1, false))
                variables.push_back((std::make_pair(tmp, "MeshRenderer")));
            else if (checkMeshAnimator(L, -1, false))
                variables.push_back((std::make_pair(tmp, "MeshAnimator")));
            else if (checkResAnimation(L, -1, false))
                variables.push_back((std::make_pair(tmp, "Resource_Animation")));
            else if (checkluaScript(L, -1, false))
                variables.push_back((std::make_pair(tmp, "LuaScript")));
            else if (checkCamera(L, -1, false))
                variables.push_back((std::make_pair(tmp, "Camera")));
            else if (checkRayCastdata(L, -1, false))
                variables.push_back((std::make_pair(tmp, "rayCastData")));
            else if (checkBoxParticle(L, -1, false))
                variables.push_back((std::make_pair(tmp, "ParticleEmitter_Box")));
            else if (checkCircleParticle(L, -1, false))
                variables.push_back((std::make_pair(tmp, "ParticleEmitter_Circle")));
            else if (checkPathFinding(L, -1, false))
                variables.push_back((std::make_pair(tmp, "PathFinding")));
            else if (checkBoxCollider(L, -1, false))
                variables.push_back((std::make_pair(tmp, "BoxCollider")));
            else if (checkTextRenderer(L, -1, false))
                variables.push_back((std::make_pair(tmp, "TextRenderer")));
            else if (checkAttractor(L, -1, false))
                variables.push_back((std::make_pair(tmp, "Attractor")));
            else if (checkDirectionalLight(L, -1, false))
                variables.push_back((std::make_pair(tmp, "DirectionalLight")));
            else if (checkPointLight(L, -1, false))
                variables.push_back((std::make_pair(tmp, "PointLight")));
            else if (checkSpotLight(L, -1, false))
                variables.push_back((std::make_pair(tmp, "SpotLight")));
        }
        /// Table
        if (lua_istable(L, -1))
        {
            /// get size of table
            int tableSize = (int)lua_rawlen(L, -1);
            if (tableSize < 1)
                std::cout << tmp << " is a table with no values. This table will not be able to be acessed by other scripts. [" << filename << "]" << std::endl;
            else
            {
                lua_rawgeti(L, -1, 1);
                int Top = lua_gettop(L);

                if (lua_isboolean(L, Top))
                    variables.push_back(std::make_pair(tmp, "bool"));
                else if (lua_isnumber(L, Top))
                    variables.push_back(std::make_pair(tmp, "float"));
                else if (lua_isstring(L, Top))
                    variables.push_back(std::make_pair(tmp, "string"));
                else if (lua_isinteger(L, Top))
                    variables.push_back(std::make_pair(tmp, "int"));
                else if (checkVector3(L, Top, false))
                    variables.push_back(std::make_pair(tmp, "vector3"));
                else if (checkColor(L, Top, false))
                    variables.push_back(std::make_pair(tmp, "color"));
                else if (checkGameObject(L, Top, false))
                    variables.push_back(std::make_pair(tmp, "GameObject"));
                else if (checkTransform(L, Top, false))
                    variables.push_back(std::make_pair(tmp, "Transform"));
                else if (checkRigidbody(L, Top, false))
                    variables.push_back(std::make_pair(tmp, "RigidBody"));
                else if (checkAudioListener(L, Top, false))
                    variables.push_back(std::make_pair(tmp, "AudioListener"));
                else if (checkAudioEmitter(L, Top, false))
                    variables.push_back(std::make_pair(tmp, "AudioEmitter"));
                else if (checkMeshRenderer(L, Top, false))
                    variables.push_back(std::make_pair(tmp, "MeshRenderer"));
                else if (checkMeshAnimator(L, Top, false))
                    variables.push_back(std::make_pair(tmp, "MeshAnimator"));
                else if (checkResAnimation(L, Top, false))
                    variables.push_back(std::make_pair(tmp, "Resource_Animation"));
                else if (checkluaScript(L, Top, false))
                    variables.push_back(std::make_pair(tmp, "LuaScript"));
                else if (checkCamera(L, Top, false))
                    variables.push_back(std::make_pair(tmp, "Camera"));
                else if (checkRayCastdata(L, Top, false))
                    variables.push_back(std::make_pair(tmp, "rayCastData"));
                else if (checkBoxParticle(L, Top, false))
                    variables.push_back(std::make_pair(tmp, "ParticleEmitter_Box"));
                else if (checkCircleParticle(L, Top, false))
                    variables.push_back(std::make_pair(tmp, "ParticleEmitter_Circle"));
                else if (checkPathFinding(L, Top, false))
                    variables.push_back(std::make_pair(tmp, "PathFinding"));
                else if (checkBoxCollider(L, Top, false))
                    variables.push_back(std::make_pair(tmp, "BoxCollider"));
                else if (checkTextRenderer(L, Top, false))
                    variables.push_back(std::make_pair(tmp, "TextRenderer"));
                else if (checkAttractor(L, Top, false))
                    variables.push_back(std::make_pair(tmp, "Attractor"));
                else if (checkDirectionalLight(L, Top, false))
                    variables.push_back(std::make_pair(tmp, "DirectionalLight"));
                else if (checkPointLight(L, Top, false))
                    variables.push_back(std::make_pair(tmp, "PointLight"));
                else if (checkSpotLight(L, Top, false))
                    variables.push_back(std::make_pair(tmp, "SpotLight"));
                else
                    std::cout << "Some weird type detected" << std::endl;
            }
        }
    }
}

void LuaScript::InitScript()
{
    /// Initailize lua scripting
    L = luaL_newstate();

    /// Open Library may check if need to call for all the script
    // luaL_openlibs(L);

    /// Create metatables
    layer_metatable(L);
    vector3_metatable(L);
    color_metatable(L);
    gameObject_metatable(L);
    transform_metatable(L);
    rigidbody_metatable(L);
    audioListener_metatable(L);
    audioEmitter_metatable(L);
    meshRenderer_metatable(L);
    meshAnimator_metatable(L);
    luaScript_metatable(L);
    camera_metatable(L);
    boxParticle_metatable(L);
    circleParticle_metatable(L);
    // sphereCollider_metatable(L);
    pathFinding_metatable(L);
    // boxCollider_metatable(L);
    textRenderer_metatable(L);
    rayCastData_metatable(L);
    resAnimation_metatable(L);
    attractor_metatable(L);
    directionalLight_metatable(L);
    pointLight_metatable(L);
    spotLight_metatable(L);

    if (luaL_loadfile(L, (ResourceManager::s_ResourcePathScripts / filename).string().c_str()))
    // if (luaL_loadfile(L, (filename).c_str()))
    {
        std::cout << "Lua Error : Script not compiled (" << filename << ")" << std::endl;
        if (L) lua_close(L);
        L = nullptr;
        variables.clear();
        return;
    }
    if (L)
    {
        /// Function shortcuts
        lua_register(L, "write", lua_write);
        lua_register(L, "SceneLoad", Scene_Load);
        lua_register(L, "SceneReload", Scene_Reload);
        lua_register(L, "SceneQuit", Scene_Quit);
        lua_register(L, "ToString", ToString);

        /// Input
        lua_register(L, "IsKeyDown", lua_down);
        lua_register(L, "IsKeyPressed", lua_pressed);
        lua_register(L, "IsKeyUp", lua_up);
        lua_register(L, "IsPadUp", lua_gamepad_up);
        lua_register(L, "IsPadPressed", lua_gamepad_pressed);
        lua_register(L, "IsPadDown", lua_gamepad_down);
        lua_register(L, "ControllerDown", lua_controllerMap_down);
        lua_register(L, "ControllerUp", lua_controllerMap_up);
        lua_register(L, "ControllerPress", lua_controllerMap_pressed);
        lua_register(L, "ControllerAxis", lua_controllerMap_axis);
        lua_register(L, "MousePosition", lua_mousePosition);
        lua_register(L, "MouseDeltaPosition", lua_mouseDeltaPosition);
        lua_register(L, "ShowMouseCursor", lua_displayMouse);
        lua_register(L, "SetMouseIcon", lua_setMouseIcon);
        lua_register(L, "LockMouseToCenter", lua_lockMouseToCenter);
        lua_register(L, "ChangeControllerInput", lua_changeControllerInput);
        lua_register(L, "GetControllerInput", lua_getControllerInput);
        lua_register(L, "SetMouseWrap", lua_SetMouseWrap);
        lua_register(L, "ControllerVibrateLeft", lua_ControllerVibrateLeft);
        lua_register(L, "ControllerVibrateRight", lua_ControllerVibrateRight);
        lua_register(L, "ControllerVibrateBoth", lua_ControllerVibrateBoth);

        /// Math
        lua_register(L, "Lerp", math_lerp);
        lua_register(L, "Vector3Lerp", math_Vector3lerp);
        lua_register(L, "RandomRange", math_randomRange);
        lua_register(L, "RandomRangeInt", math_randomRangeInt);
        lua_register(L, "ToRad", math_toRad);
        lua_register(L, "ToDeg", math_toDeg);
        lua_register(L, "Pow", math_pow);
        lua_register(L, "Cos", math_cos);
        lua_register(L, "Sin", math_sin);
        lua_register(L, "Tan", math_tan);
        lua_register(L, "ACos", math_acos);
        lua_register(L, "ASin", math_asin);
        lua_register(L, "ATan", math_atan);
        lua_register(L, "Cosh", math_cosh);
        lua_register(L, "Sinh", math_sinh);
        lua_register(L, "Tanh", math_tanh);
        lua_register(L, "ACosh", math_acosh);
        lua_register(L, "ASinh", math_asinh);
        lua_register(L, "ATanh", math_atanh);
        lua_register(L, "Abs", math_abs);
        lua_register(L, "Round", math_round);
        lua_register(L, "IsEven", math_isEven);
        lua_register(L, "IsOdd", math_isOdd);
        lua_register(L, "Mod", math_mod);
        lua_register(L, "ToInt", ToInt);
        /// Layer
        lua_register(L, "CurrentLayer", layer_currentLayer);
        lua_register(L, "GetLayer", layer_getLayer);
        /// Vector3
        lua_register(L, "Vector3", vector3_new);
        lua_register(L, "VectorSet", vector3_set);
        lua_register(L, "VectorNormalized", vector3_Normalized);
        lua_register(L, "VectorProject", vector3_Projection);
        lua_register(L, "VectorCross", vector3_Cross);
        lua_register(L, "VectorDot", vector3_Dot);
        lua_register(L, "VectorLength", vector3_Length);
        lua_register(L, "VectorSquareLength", vector3_SquareLength);
        lua_register(L, "VectorDistance", vector3_DistanceTo);
        lua_register(L, "VectorSquareDistance", vector3_SquaredDistanceTo);
        lua_register(L, "VectorOne", vector3_One);
        lua_register(L, "VectorRotate", vector3_Rotate);
        lua_register(L, "VectorAngleWorldAxis", vector3_AngleFromWorldAxis);
        /// Color
        lua_register(L, "Color", color_new);
        lua_register(L, "ColorSet", color_set);
        /// GameObject
        lua_register(L, "CreatePrefab", create_prefab);
        /// Physics
        lua_register(L, "RayCast", physics_RayCast);
        lua_register(L, "RayCastFirstOfName", physics_RayCastFirstOfName);
        lua_register(L, "RayCastAll", physics_RayCastAll);
        /// Debug
        lua_register(L, "DebugDrawLine", debug_DrawLine);
        /// Resource
        lua_register(L, "GetResource_Animation", resAnimation_Get);
        /// Audio_System
        lua_register(L, "AudioSystem_PlayAudioAtLocation", audioSystem_PlayAudioAtLocation);
        lua_register(L, "AudioSystem_SetChannelGrpVolume", audioSystem_SetChannelGrpVolume);
        lua_register(L, "AudioSystem_GetChannelGrpVolume", audioSystem_GetChannelGrpVolume);
        /// AI_System
        lua_register(L, "AISystem_SetPosValid", AISystem_MarkGridValidity);
        lua_register(L, "AISystem_GetSlimePaths", AISystem_GetSlimePaths);
        lua_register(L, "AISystem_GetPathChanged", AISystem_GetOriginalPathChanged);
        lua_register(L, "AISystem_SetPathChanged", AISystem_SetOriginalPathChanged);
        lua_register(L, "AISystem_NearestPathIndex", AISystem_NearestPathIndex);
        lua_register(L, "AISystem_RetrieveGridPos", AISystem_RetrieveGridPos);            // returns when its valid
        lua_register(L, "AISystem_RetrieveGridPosAll", AISystem_RetrieveGridPosGeneral);  // Return doesnt matter if is valid or invalid
        lua_register(L, "AISystem_CheckValidGrid", AISystem_CheckValidGrid);
        lua_register(L, "AISystem_PushBackStartPath", AISystem_PushBackStartPath);
        lua_register(L, "AISystem_GetStartPath", AISystem_GetStartPath);
        lua_register(L, "AISystem_GetStartPathByIndex", AISystem_GetStartPathByIndex);
        lua_register(L, "AISystem_GetStartPathSize", AISystem_GetStartPathSize);
        lua_register(L, "AISystem_ClearOriginalPath", AISystem_ClearOriginalPath);
        lua_register(L, "AISystem_ReplacePathAtIndex", AISystem_ReplacePathAtIndex);
        lua_register(L, "AISystem_GetAvilableGridOutsidePos", AISystem_GetAvilableGridOutsidePos);

        /// Delta time
        lua_register(L, "UnscaledDT", UnscaledDT);
        lua_register(L, "SetTimeScale", SetTimeScale);
        lua_register(L, "GetTimeScale", GetTimeScale);
        /// Player pref
        lua_register(L, "PlayerPref_CheckExist", Playerpref_CheckExist);
        lua_register(L, "PlayerPref_RemoveAll", Playerpref_RemoveAll);
        lua_register(L, "PlayerPref_RemoveVariable", Playerpref_RemoveVariable);
        lua_register(L, "PlayerPref_GetInteger", Playerpref_GetInteger);
        lua_register(L, "PlayerPref_SetInteger", Playerpref_SetInteger);
        lua_register(L, "PlayerPref_GetFloat", Playerpref_GetFloat);
        lua_register(L, "PlayerPref_SetFloat", Playerpref_SetFloat);
        lua_register(L, "PlayerPref_GetBool", Playerpref_GetBool);
        lua_register(L, "PlayerPref_SetBool", Playerpref_SetBool);
        lua_register(L, "PlayerPref_GetString", Playerpref_GetString);
        lua_register(L, "PlayerPref_SetString", Playerpref_SetString);
        lua_register(L, "PlayerPref_GetVector3", Playerpref_GetVector3);
        lua_register(L, "PlayerPref_SetVector3", Playerpref_SetVector3);
        lua_register(L, "PlayerPref_GetIntegerArray", Playerpref_GetIntegerArray);
        lua_register(L, "PlayerPref_SetIntegerArray", Playerpref_SetIntegerArray);
        lua_register(L, "PlayerPref_GetFloatArray", Playerpref_GetFloatArray);
        lua_register(L, "PlayerPref_SetFloatArray", Playerpref_SetFloatArray);
        lua_register(L, "PlayerPref_GetBoolArray", Playerpref_GetBoolArray);
        lua_register(L, "PlayerPref_SetBoolArray", Playerpref_SetBoolArray);
        lua_register(L, "PlayerPref_GetStringArray", Playerpref_GetStringArray);
        lua_register(L, "PlayerPref_SetStringArray", Playerpref_SetStringArray);
        lua_register(L, "PlayerPref_GetVector3Array", Playerpref_GetVector3Array);
        lua_register(L, "PlayerPref_SetVector3Array", Playerpref_SetVector3Array);
        /// Game
        lua_register(L, "ToggleFullscreen", ToggleFullscreen);
        /// push constants
        load_defines(L);
        pushGameObject(L, m_OwnerObject);
        lua_setglobal(L, "owner");

        lua_register(L, "CreateLayer", create_Layer);

        lua_register(L, "GotFocus", GotFocus);
        /*********** Constant for this
        // Replace the go with the parent GameObject
        GameObject* go = Application::Instance().GetCurrentScene()->GetLayers().front()->GetObjectByName("tmp");
        pushGameObject(L, go);
        lua_setglobal(L, "this");
        *****************************/
        BlacklistSnapshot(L);
    }
    if (lua_pcall(L, 0, 0, 0))
    {
        std::cout << "Lua Error : Script has errors and could be not compiled (" << filename << ")" << std::endl;
        if (L) lua_close(L);
        L = nullptr;
        return;
    }
    if (L) GetGlobals(L);
    // Constructor
    lua_getglobal(L, "Constructor");
    if (lua_type(L, -1) == LUA_TFUNCTION) lua_pcall(L, 0, 0, 0);
}

void LuaScript::OnCollisionEnter(GameObject* other)
{
    // IF YOU CAME HERE IT'S PROBABLY BECAUSE YOUR LUA SCRIPT HAVE A SYNTAX ERROR
    if (!m_dt || !isActive) return;
    lua_getglobal(L, "OnCollisionEnter");
    if (lua_type(L, -1) == LUA_TFUNCTION)
    {
        pushGameObject(L, (void*)other);
        lua_pcall(L, 1, 0, 0);
    }
    clean();
}

void LuaScript::OnCollisionPersist(GameObject* other)
{
    if (!m_dt || !isActive) return;
    lua_getglobal(L, "OnCollisionPersist");
    if (lua_type(L, -1) == LUA_TFUNCTION)
    {
        pushGameObject(L, (void*)other);
        lua_pcall(L, 1, 0, 0);
    }
    clean();
}

void LuaScript::OnCollisionExit(GameObject* other)
{
    if (!m_dt || !isActive) return;
    lua_getglobal(L, "OnCollisionExit");
    if (lua_type(L, -1) == LUA_TFUNCTION)
    {
        pushGameObject(L, (void*)other);
        lua_pcall(L, 1, 0, 0);
    }
    clean();
}

int LuaScript::GotFocus(lua_State* L)
{
    bool b = Application::Instance().m_focus;
    lua_pushboolean(L, b);
    return 1;
}

LuaScript::LuaScript(GameObject* parentObject)
    : IComponent(parentObject, "LuaScript")
    , L(nullptr)
    , level{0}
    , filename()
    , m_dt(0)
{
}

LuaScript::LuaScript(const std::string& filename)
    : L(nullptr)
    , level{0}
    , filename(filename)
    , IComponent(nullptr, "LuaScript")
    , m_dt(0)
{
    InitScript();
}

LuaScript::~LuaScript()
{
    if (L)
    {
        lua_getglobal(L, "Destructor");
        if (lua_type(L, -1) == LUA_TFUNCTION) lua_pcall(L, 0, 0, 0);
        lua_close(L);
    }
}

void LuaScript::SetScript(std::string name)
{
    filename = name;
}

void LuaScript::PrintError(const std::string& var, const std::string& err)
{
    std::cout << "Lua Error : unable to get [" << var << "]." << err << std::endl;
}

void LuaScript::OnUpdate(float dt)
{
    if (!isActive || (filename[0] != 'U' && filename[1] != 'I' && dt <= 0.001) && (filename != "SettingsLogic.lua") && (filename != "PauseLogic.lua")) return;

    m_dt = dt;

    if (!dt) return;
    if (L)
    {
        // float dt = 1 / 60.f;
        lua_getglobal(L, "OnUpdate");
        if (lua_type(L, -1) == LUA_TFUNCTION)
        {
            lua_pushnumber(L, dt);
            lua_pcall(L, 1, 0, 0);
        }
        else
            std::cout << "There is no OnUpdate Function" << std::endl;
        // Testing for OnCollisionEnter, OnCollisionPersist and OnCollisionEnded
        // The code below is to be removed
        // lua_getglobal(L, "Test");
        clean();
    }
    else
    {
        InitScript();
    }
}

const std::string& LuaScript::GetScript() const
{
    return filename;
}

REFLECT_INIT(LuaScript)
REFLECT_PROPERTY(filename)
REFLECT_END()
