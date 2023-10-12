#pragma once

#include <functional>
#include <iostream>
#include <set>
#include <string>
#include <vector>

#include "AISystem.h"
#include "IComponent.h"
#include "Layer.h"
#include "RaycastInfo.h"
#include "ResourceHandle.h"

extern "C"
{
#include "lauxlib.h"
#include "lua.h"
#include "lualib.h"
}

#define lua_setConstant(L, name) \
    {                            \
        lua_pushnumber(L, name); \
        lua_setglobal(L, #name); \
    }

/* DOCUMENTATION
  https://eliasdaler.wordpress.com/2013/10/11/lua_cpp_binder/
  https://github.com/eliasdaler/unnamed_lua_binder
  https://www.lua.org/pil/26.1.html
  http://www.randygaul.net/2014/01/01/automated-lua-binding/
  https://www.lua.org/pil/28.1.html
  https://stackoverflow.com/questions/11689135/share-array-between-lua-and-c (vectors or list)
  http://lua-users.org/wiki/SimpleLuaApiExample
  https://eliasdaler.wordpress.com/2013/10/20/lua_and_cpp_pt2/ (Getting arrays from other scripts)

  Create stucts in lua using metatables

  http://www.lua.org/manual/5.1/manual.html#2.8
  https://eliasdaler.wordpress.com/2013/11/30/lua_and_cpp_pt3/
  https://eliasdaler.wordpress.com/2016/01/07/using-lua-with-c-in-practice-part4/
  http://vinniefalco.github.io/LuaBridge/Manual.html
*/

/// Forward declare
class AudioEmitter;
class AudioListener;
class RigidbodyComponent;
class Transform;
class MeshAnimator;
class MeshRenderer;
class Camera;
class BoxParticleEmitter;
class Pathfinding;
class BoxCollider;
class SphereCollider;
class CircleParticleEmitter;
class ParticleEmitter;
class TextRenderer;
class Attractor;
class DirectionalLight;
class PointLight;
class SpotLight;

class LuaScript : public IComponent
{
    lua_State* L = nullptr;
    int level = 0;
    std::string filename;
    bool isActive = true;

    /// LUA functions -----------------------------------------------------------
    static int lua_write(lua_State* L);
    static int create_prefab(lua_State* L);
    static int Scene_Load(lua_State* L);
    static int Scene_Reload(lua_State* L);
    static int Scene_Quit(lua_State* L);
    static int ToString(lua_State* L);
    static int ToInt(lua_State* L);
    void SetActive(bool L);

    /// player pref related functions -------------------------------------------
    static int Playerpref_CheckExist(lua_State* L);
    static int Playerpref_RemoveAll(lua_State* L);
    static int Playerpref_RemoveVariable(lua_State* L);
    static int Playerpref_GetInteger(lua_State* L);
    static int Playerpref_SetInteger(lua_State* L);
    static int Playerpref_GetFloat(lua_State* L);
    static int Playerpref_SetFloat(lua_State* L);
    static int Playerpref_GetBool(lua_State* L);
    static int Playerpref_SetBool(lua_State* L);
    static int Playerpref_GetString(lua_State* L);
    static int Playerpref_SetString(lua_State* L);
    static int Playerpref_GetVector3(lua_State* L);
    static int Playerpref_SetVector3(lua_State* L);
    static int Playerpref_GetIntegerArray(lua_State* L);
    static int Playerpref_SetIntegerArray(lua_State* L);
    static int Playerpref_GetFloatArray(lua_State* L);
    static int Playerpref_SetFloatArray(lua_State* L);
    static int Playerpref_GetBoolArray(lua_State* L);
    static int Playerpref_SetBoolArray(lua_State* L);
    static int Playerpref_GetStringArray(lua_State* L);
    static int Playerpref_SetStringArray(lua_State* L);
    static int Playerpref_GetVector3Array(lua_State* L);
    static int Playerpref_SetVector3Array(lua_State* L);

    /// Delta time related functions --------------------------------------------
    static int UnscaledDT(lua_State* L);
    static int SetTimeScale(lua_State* L);
    static int GetTimeScale(lua_State* L);

    /// Debug related functions -------------------------------------------------
    static int debug_DrawLine(lua_State* L);

    /// Input related functions -------------------------------------------------
    void load_defines(lua_State* L);
    static int lua_down(lua_State* L);
    static int lua_pressed(lua_State* L);
    static int lua_up(lua_State* L);
    static int lua_gamepad_pressed(lua_State* L);
    static int lua_gamepad_down(lua_State* L);
    static int lua_gamepad_up(lua_State* L);
    static int lua_controllerMap_pressed(lua_State* L);
    static int lua_controllerMap_down(lua_State* L);
    static int lua_controllerMap_up(lua_State* L);
    static int lua_controllerMap_axis(lua_State* L);
    static int lua_mousePosition(lua_State* L);
    static int lua_mouseDeltaPosition(lua_State* L);
    static int lua_displayMouse(lua_State* L);
    static int lua_setMouseIcon(lua_State* L);
    static int lua_lockMouseToCenter(lua_State* L);
    static int lua_changeControllerInput(lua_State* L);
    static int lua_getControllerInput(lua_State* L);
    static int lua_ControllerVibrateLeft(lua_State* L);
    static int lua_ControllerVibrateRight(lua_State* L);
    static int lua_ControllerVibrateBoth(lua_State* L);
    static int lua_SetMouseWrap(lua_State* L);

    /// Layer related functions -------------------------------------------------
    static const luaL_Reg layerlib[];
    int layer_metatable(lua_State* L);
    static int layer_currentLayer(lua_State* L);
    static int layer_getLayer(lua_State* L);
    static int layer_name(lua_State* L);
    static int layer_createObject(lua_State* L);
    static SceneLayer** checklayer(lua_State* L, int index = 1, bool assert = true);
    static void setlayer(lua_State* L, void* ly);
    static int layer_find(lua_State* L);
    static int layer_getObjects(lua_State* L);
    static int layer_getObjectsByName(lua_State* L);
    static int layer_getObjectsBySubName(lua_State* L);
    static int layer_getObjectByID(lua_State* L);
    static int layer_getObjectsByTag(lua_State* L);
    static int create_Layer(lua_State* L);
    static int layer_DestroyAllObjectsWithTag(lua_State* L);

    /// Vec3 related functions --------------------------------------------------
    static const luaL_Reg vector3lib[];
    int vector3_metatable(lua_State* L);
    static Vector3* checkVector3(lua_State* L, int index = 1, bool assert = true);
    static int pushVector3(lua_State* L, Vector3 input);

    static int vector3_new(lua_State* L);
    static int vector3_index(lua_State* L);
    static int vector3_newindex(lua_State* L);
    static int vector3_set(lua_State* L);
    static int vector3_add(lua_State* L);
    static int vector3_sub(lua_State* L);
    static int vector3_mul(lua_State* L);
    static int vector3_div(lua_State* L);
    static int vector3_unm(lua_State* L);
    static int vector3_eq(lua_State* L);
    static int vector3_Normalize(lua_State* L);
    static int vector3_Normalized(lua_State* L);
    static int vector3_Projection(lua_State* L);
    static int vector3_Cross(lua_State* L);
    static int vector3_Dot(lua_State* L);
    static int vector3_Length(lua_State* L);
    static int vector3_SquareLength(lua_State* L);
    static int vector3_DistanceTo(lua_State* L);
    static int vector3_SquaredDistanceTo(lua_State* L);
    static int vector3_One(lua_State* L);
    static int vector3_Rotate(lua_State* L);
    static int vector3_AngleFromWorldAxis(lua_State* L);
    static int vector3_X(lua_State* L);
    static int vector3_Y(lua_State* L);
    static int vector3_Z(lua_State* L);
    static int vector3_polarAngle(lua_State* L);

    /// Colour related functions ------------------------------------------------
    static const luaL_Reg colorlib[];
    int color_metatable(lua_State* L);
    static Vector4* checkColor(lua_State* L, int index = 1, bool assert = true);
    static int pushColor(lua_State* L, Vector4 input);

    static Vector4 clampColor(Vector4 input);
    static int color_index(lua_State* L);
    static int color_newindex(lua_State* L);
    static int color_new(lua_State* L);
    static int color_set(lua_State* L);
    static int color_add(lua_State* L);
    static int color_sub(lua_State* L);
    static int color_mul(lua_State* L);
    static int color_div(lua_State* L);
    static int color_eq(lua_State* L);
    static int color_R(lua_State* L);
    static int color_G(lua_State* L);
    static int color_B(lua_State* L);
    static int color_A(lua_State* L);

    /// Math related functions --------------------------------------------------
    static int math_lerp(lua_State* L);
    static int math_Vector3lerp(lua_State* L);
    static int math_randomRange(lua_State* L);
    static int math_randomRangeInt(lua_State* L);
    static int math_toRad(lua_State* L);
    static int math_toDeg(lua_State* L);
    static int math_pow(lua_State* L);
    static int math_cos(lua_State* L);
    static int math_sin(lua_State* L);
    static int math_tan(lua_State* L);
    static int math_acos(lua_State* L);
    static int math_asin(lua_State* L);
    static int math_atan(lua_State* L);
    static int math_cosh(lua_State* L);
    static int math_sinh(lua_State* L);
    static int math_tanh(lua_State* L);
    static int math_acosh(lua_State* L);
    static int math_asinh(lua_State* L);
    static int math_atanh(lua_State* L);
    static int math_abs(lua_State* L);
    static int math_round(lua_State* L);
    static int math_isEven(lua_State* L);
    static int math_isOdd(lua_State* L);
    static int math_mod(lua_State* L);

    /// RayCastInfo -------------------------------------------------------------
    static const luaL_Reg rayCastDatalib[];
    int rayCastData_metatable(lua_State* L);
    static RaycastData_tmp* checkRayCastdata(lua_State* L, int index = 1, bool assert = true);
    static int pushRayCastdata(lua_State* L, RaycastData_tmp input);

    static int rayCastData_point(lua_State* L);
    static int rayCastData_normal(lua_State* L);
    static int rayCastData_hitFrac(lua_State* L);
    static int rayCastData_gameObject(lua_State* L);

    static int physics_RayCast(lua_State* L);
    static int physics_RayCastFirstOfName(lua_State* L);
    static int physics_RayCastAll(lua_State* L);

    /// GameObject related functions --------------------------------------------
    static const luaL_Reg gameObjectlib[];
    int gameObject_metatable(lua_State* L);
    static GameObject** checkGameObject(lua_State* L, int index = 1, bool assert = true);
    static int pushGameObject(lua_State* L, void* go);

    static int gameObject_name(lua_State* L);
    static int gameObject_Setname(lua_State* L);
    static int gameObject_destroy(lua_State* L);
    static int gameObject_getLuaScript(lua_State* L);
    static int gameObject_addComponent(lua_State* L);
    static int gameObject_removeComponent(lua_State* L);
    static int setGameObjectActive(lua_State* L);
    static int getGameObjectActive(lua_State* L);
    static int getGameObjectComponent(lua_State* L);
    static int getGameObjectComponents(lua_State* L);
    static int addGameObjectComponent(lua_State* L);
    static int getGameObjectLayer(lua_State* L);
    static int gameObject_GetID(lua_State* L);
    static int gameObject_GetTag(lua_State* L);
    static int gameObject_SetParent(lua_State* L);
    static int gameObject_GetParent(lua_State* L);
    static int gameObject_UnParent(lua_State* L);
    static int gameObject_GetChild(lua_State* L);

    /// Component related functions ------------------------------------------------
    static IComponent** checkIsComponent(lua_State* L, int index);
    static int component_Owner(lua_State* L);
    static int component_SetActive(lua_State* L);

    /// Transform
    static const luaL_Reg transformlib[];
    int transform_metatable(lua_State* L);
    static Transform** checkTransform(lua_State* L, int index = 1, bool assert = true);
    static int pushTransform(lua_State* L, Transform* input);

    // static int transform_GetLocalPosition          (lua_State* L);
    // static int transform_GetLocalRotation          (lua_State* L);
    // static int transform_GetLocalScale             (lua_State* L);
    static int transform_GetWorldPosition(lua_State* L);
    static int transform_GetWorldRotation(lua_State* L);
    static int transform_GetWorldScale(lua_State* L);
    // static int transform_SetLocalPosition          (lua_State* L);
    // static int transform_SetLocalRotation          (lua_State* L);
    // static int transform_SetLocalScale             (lua_State* L);
    static int transform_SetWorldPosition(lua_State* L);
    static int transform_SetWorldRotation(lua_State* L);
    static int transform_SetWorldScale(lua_State* L);
    static int transform_Translate(lua_State* L);
    static int transform_Rotate(lua_State* L);
    static int transform_Scale(lua_State* L);
    static int transform_GetForwardVector(lua_State* L);
    static int transform_GetUpwardVector(lua_State* L);
    static int transform_GetRightVector(lua_State* L);
    static int transform_LookAt(lua_State* L);
    static int transform_LookAtV(lua_State* L);

    /// Rigibody
    static const luaL_Reg rigidbodylib[];
    int rigidbody_metatable(lua_State* L);
    static RigidbodyComponent** checkRigidbody(lua_State* L, int index = 1, bool assert = true);
    static int pushRigidBody(lua_State* L, RigidbodyComponent* input);

    static int rigidbody_SetGhost(lua_State* L);
    // static int rigidbody_SetCollideWithStatic      (lua_State* L);
    static int rigidbody_SetMass(lua_State* L);
    // static int rigidbody_SetDrag                   (lua_State* L);
    static int rigidbody_SetBodytype(lua_State* L);
    // static int rigidbody_SetFreezeRotation         (lua_State* L);
    static int rigidbody_SetRestitution(lua_State* L);
    static int rigidbody_SetVelocity(lua_State* L);
    static int rigidbody_SetAcceleration(lua_State* L);
    // static int rigidbody_SetOffset                 (lua_State* L);
    // static int rigidbody_SetAngularVelocity        (lua_State* L);
    static int rigidbody_SetGravityEnabled(lua_State* L);
    // static int rigidbody_SetIsAffectedByResistance (lua_State* L);

    static int rigidbody_GetGhost(lua_State* L);
    // static int rigidbody_GetCollideWithStatic      (lua_State* L);
    static int rigidbody_GetMass(lua_State* L);
    // static int rigidbody_GetDrag                   (lua_State* L);
    static int rigidbody_GetBodytype(lua_State* L);
    // static int rigidbody_GetFreezeRotation         (lua_State* L);
    static int rigidbody_GetRestitution(lua_State* L);
    static int rigidbody_GetVelocity(lua_State* L);
    static int rigidbody_GetAcceleration(lua_State* L);
    // static int rigidbody_GetOffset                 (lua_State* L);
    // static int rigidbody_GetAngularVelocity        (lua_State* L);
    static int rigidbody_GetGravityEnabled(lua_State* L);
    // static int rigidbody_GetIsAffectedByResistance (lua_State* L);
    static int rigidbody_AddVelocity(lua_State* L);
    static int rigidbody_AddForce(lua_State* L);
    static int rigidbody_GetForce(lua_State* L);
    static int rigidbody_SetYVelocity(lua_State* L);

    /// Audio Listener
    static const luaL_Reg audioListenerlib[];
    int audioListener_metatable(lua_State* L);
    static AudioListener** checkAudioListener(lua_State* L, int index = 1, bool assert = true);
    static int pushAudioListener(lua_State* L, AudioListener* input);

    static int audioListener_IsMain(lua_State* L);
    static int audioListener_SetMain(lua_State* L);

    /// Audio Emitter
    static const luaL_Reg audioEmitterlib[];
    int audioEmitter_metatable(lua_State* L);
    static AudioEmitter** checkAudioEmitter(lua_State* L, int index = 1, bool assert = true);
    static int pushAudioEmitter(lua_State* L, AudioEmitter* input);

    static int audioEmitter_Play(lua_State* L);
    static int audioEmitter_Stop(lua_State* L);
    static int audioEmitter_Pause(lua_State* L);
    static int audioEmitter_IsPlaying(lua_State* L);
    static int audioEmitter_IsPaused(lua_State* L);
    static int audioEmitter_SetVolume(lua_State* L);
    static int audioEmitter_GetVolume(lua_State* L);
    static int audioEmitter_SetMute(lua_State* L);
    static int audioEmitter_GetMute(lua_State* L);
    static int audioEmitter_SetPitch(lua_State* L);
    static int audioEmitter_GetPitch(lua_State* L);
    static int audioEmitter_SetLoop(lua_State* L);
    static int audioEmitter_SetLoopCount(lua_State* L);
    static int audioEmitter_GetLoopCount(lua_State* L);
    static int audioEmitter_SetAudioClip(lua_State* L);
    static int audioEmitter_SetAndPlayAudioClip(lua_State* L);
    static int audioEmitter_SetMinDistance3D(lua_State* L);
    static int audioEmitter_SetMaxDistance3D(lua_State* L);
    static int audioEmitter_GetMaxDistance3D(lua_State* L);
    static int audioEmitter_GetMinDistance3D(lua_State* L);
    static int audioEmitter_GetSoundName(lua_State* L);
    static int audioEmitter_FadeOut(lua_State* L);
    static int audioEmitter_SetChannelGroup(lua_State* L);

    /// Audio System
    static int audioSystem_PlayAudioAtLocation(lua_State* L);
    static int audioSystem_SetChannelGrpVolume(lua_State* L);
    static int audioSystem_GetChannelGrpVolume(lua_State* L);

    /// MeshRenderer
    static const luaL_Reg meshRendererlib[];
    int meshRenderer_metatable(lua_State* L);
    static MeshRenderer** checkMeshRenderer(lua_State* L, int index = 1, bool assert = true);
    static int pushMeshRenderer(lua_State* L, MeshRenderer* input);

    static int meshRenderer_SetColor(lua_State* L);
    static int meshRenderer_GetColor(lua_State* L);
    static int meshRenderer_SetDiffuse(lua_State* L);
    static int meshRenderer_GetMesh(lua_State* L);
    static int meshRenderer_SetMesh(lua_State* L);
    static int meshRenderer_SetEmissive(lua_State* L);
    static int meshRenderer_SetEmissiveTexture(lua_State* L);
    static int meshRenderer_SetEnableEmissive(lua_State* L);
    static int meshRenderer_GetEnableEmissive(lua_State* L);

    /// MeshAnimator
    static const luaL_Reg meshAnimatorlib[];
    int meshAnimator_metatable(lua_State* L);
    static MeshAnimator** checkMeshAnimator(lua_State* L, int index = 1, bool assert = true);
    static int pushMeshAnimator(lua_State* L, MeshAnimator* input);

    static int meshAnimator_PlayAnimation(lua_State* L);
    static int meshAnimator_PlayAnimationOnce(lua_State* L);
    static int meshAnimator_StopAnimation(lua_State* L);
    static int meshAnimator_PauseAnimation(lua_State* L);
    static int meshAnimator_IsPlaying(lua_State* L);
    static int meshAnimator_GetAnimationSet(lua_State* L);
    static int meshAnimator_SetAnimationSet(lua_State* L);
    static int meshAnimator_SetCrossFadeBoolean(lua_State* L);
    static int meshAnimator_SetTimeScale(lua_State* L);

    /// LuaScript
    static const luaL_Reg luaScriptlib[];
    int luaScript_metatable(lua_State* L);
    static LuaScript** checkluaScript(lua_State* L, int index = 1, bool assert = true);
    static int pushluaScript(lua_State* L, LuaScript* input);

    static int lua_GetVariable(lua_State* L);
    static int lua_GetVariableArray(lua_State* L);
    static int lua_SetVariable(lua_State* L);
    static int lua_callFunction(lua_State* L);
    static int lua_EnableScript(lua_State* L);

    /// Camera
    static const luaL_Reg cameralib[];
    int camera_metatable(lua_State* L);
    static Camera** checkCamera(lua_State* L, int index = 1, bool assert = true);
    static int pushCamera(lua_State* L, Camera* input);

    static int camera_GetLookAt(lua_State* L);
    static int camera_SetLookAt(lua_State* L);
    static int camera_SetUp(lua_State* L);
    static int camera_GetViewportSize(lua_State* L);
    static int camera_SetViewportSize(lua_State* L);
    static int camera_SetUICamera(lua_State* L);
    static int camera_SetDirectionalLightIntensity(lua_State* L);
    static int camera_SetDirectionalLightDirection(lua_State* L);
    static int camera_SetShadowCasted(lua_State* L);
    static int camera_SetColor(lua_State* L);

    /// Particle
    static ParticleEmitter** checkIsParticleEmitter(lua_State* L, int index);
    static int particle_GetStartMinColor(lua_State* L);
    static int particle_GetStartMaxColor(lua_State* L);
    static int particle_GetEndMinColor(lua_State* L);
    static int particle_GetEndMaxColor(lua_State* L);
    static int particle_GetMinSize(lua_State* L);
    static int particle_GetMaxSize(lua_State* L);
    static int particle_GetEndTexture(lua_State* L);
    static int particle_GetTextureFade(lua_State* L);
    static int particle_SetStartMinColor(lua_State* L);
    static int particle_SetStartMaxColor(lua_State* L);
    static int particle_SetEndMinColor(lua_State* L);
    static int particle_SetEndMaxColor(lua_State* L);
    static int particle_SetMinSize(lua_State* L);
    static int particle_SetMaxSize(lua_State* L);
    static int particle_SetEndTexture(lua_State* L);
    static int particle_SetTextureFade(lua_State* L);
    static int particle_SetFloorHeight(lua_State* L);

    static const luaL_Reg boxParticlelib[];
    int boxParticle_metatable(lua_State* L);
    static BoxParticleEmitter** checkBoxParticle(lua_State* L, int index = 1, bool assert = true);
    static int pushBoxParticle(lua_State* L, BoxParticleEmitter* input);

    static int boxParticle_GetPos(lua_State* L);
    static int boxParticle_GetStartPosOff(lua_State* L);
    static int boxParticle_GetMinVel(lua_State* L);
    static int boxParticle_GetMaxVel(lua_State* L);
    static int boxParticle_GetMinTime(lua_State* L);
    static int boxParticle_GetMaxTime(lua_State* L);
    static int boxParticle_SetPos(lua_State* L);
    static int boxParticle_SetStartPosOff(lua_State* L);
    static int boxParticle_SetMinVel(lua_State* L);
    static int boxParticle_SetMaxVel(lua_State* L);
    static int boxParticle_SetMinTime(lua_State* L);
    static int boxParticle_SetMaxTime(lua_State* L);
    static int boxParticle_GetEmitRate(lua_State* L);
    static int boxParticle_SetEmitRate(lua_State* L);
    static int boxParticle_AddAttractor(lua_State* L);
    static int boxParticle_RemoveAttractor(lua_State* L);

    static const luaL_Reg circleParticlelib[];
    int circleParticle_metatable(lua_State* L);
    static CircleParticleEmitter** checkCircleParticle(lua_State* L, int index = 1, bool assert = true);
    static int pushCircleParticle(lua_State* L, CircleParticleEmitter* input);

    static int circleParticle_GetEmitRate(lua_State* L);
    static int circleParticle_SetEmitRate(lua_State* L);
    static int circleParticle_AddAttractor(lua_State* L);
    static int circleParticle_RemoveAttractor(lua_State* L);

    /// Path finding
    static const luaL_Reg pathFindinglib[];
    int pathFinding_metatable(lua_State* L);
    static Pathfinding** checkPathFinding(lua_State* L, int index = 1, bool assert = true);
    static int pushPathFinding(lua_State* L, Pathfinding* input);

    static int pathFinding_AStarFindPath(lua_State* L);
    static int pathFinding_GetPath(lua_State* L);
    static int pathFinding_GetPathByIndex(lua_State* L);
    static int pathFinding_GetPathSize(lua_State* L);
    static int pathFinding_ChangeLocalToBaseMap(lua_State* L);

    /// Box Collider
    static const luaL_Reg boxColliderlib[];
    int boxCollider_metatable(lua_State* L);
    static BoxCollider** checkBoxCollider(lua_State* L, int index = 1, bool assert = true);
    static int pushBoxCollider(lua_State* L, BoxCollider* input);

    static int boxCollider_GetHalfExtent(lua_State* L);

    /// Sphere collider
    static const luaL_Reg sphereColliderlib[];
    int sphereCollider_metatable(lua_State* L);
    static SphereCollider** checkSphereCollider(lua_State* L, int index = 1, bool assert = true);
    static int pushSphereCollider(lua_State* L, SphereCollider* input);

    static int sphereCollider_SetRadius(lua_State* L);
    static int sphereCollider_GetRadius(lua_State* L);

    /// Text renderer
    static const luaL_Reg textRendererlib[];
    int textRenderer_metatable(lua_State* L);
    static TextRenderer** checkTextRenderer(lua_State* L, int index = 1, bool assert = true);
    static int pushTextRenderer(lua_State* L, TextRenderer* input);

    static int TextRenderer_SetText(lua_State* L);
    static int TextRenderer_GetText(lua_State* L);
    static int TextRenderer_SetSize(lua_State* L);
    static int TextRenderer_GetSize(lua_State* L);
    static int TextRenderer_SetColor(lua_State* L);
    static int TextRenderer_GetColor(lua_State* L);
    static int TextRenderer_SetFaceCamera(lua_State* L);
    static int TextRenderer_GetFaceCamera(lua_State* L);
    static int TextRenderer_SetAnchorPoint(lua_State* L);
    static int TextRenderer_GetAnchorPoint(lua_State* L);
    static int TextRenderer_SetFont(lua_State* L);
    static int TextRenderer_GetFont(lua_State* L);
    static int TextRenderer_SetOutline(lua_State* L);
    static int TextRenderer_SetOutlineSize(lua_State* L);

    /// AI system ---------------------------------------------------------------
    static int AISystem_MarkGridValidity(lua_State* L);
    static int AISystem_GetSlimePaths(lua_State* L);
    static int AISystem_GetOriginalPathChanged(lua_State* L);
    static int AISystem_SetOriginalPathChanged(lua_State* L);
    static int AISystem_NearestPathIndex(lua_State* L);
    static int AISystem_RetrieveGridPos(lua_State* L);
    static int AISystem_RetrieveGridPosGeneral(lua_State* L);
    static int AISystem_CheckValidGrid(lua_State* L);
    static int AISystem_PushBackStartPath(lua_State* L);
    static int AISystem_GetStartPath(lua_State* L);
    static int AISystem_ClearOriginalPath(lua_State* L);
    static int AISystem_ReplacePathAtIndex(lua_State* L);
    static int AISystem_GetAvilableGridOutsidePos(lua_State* L);
    static int AISystem_GetStartPathByIndex(lua_State* L);
    static int AISystem_GetStartPathSize(lua_State* L);

    /// Lights -------------------------------------------------------------------
    static LightBase** checkLightBase(lua_State* L, int index = 1, bool assert = true);
    static int Light_GetIntensity(lua_State* L);
    static int Light_SetIntensity(lua_State* L);
    static int Light_GetLightColor(lua_State* L);
    static int Light_SetLightColor(lua_State* L);
    static int Light_IsCastShadows(lua_State* L);
    static int Light_SetCastShadows(lua_State* L);
    static int Light_GetShadowDistance(lua_State* L);
    static int Light_GetShadowBias(lua_State* L);
    static int Light_SetShadowBias(lua_State* L);

    static const luaL_Reg directionalLightlib[];
    int directionalLight_metatable(lua_State* L);
    static DirectionalLight** checkDirectionalLight(lua_State* L, int index = 1, bool assert = true);
    static int pushDirectionalLight(lua_State* L, DirectionalLight* input);

    static int directionalLight_GetDirection(lua_State* L);
    static int directionalLight_SetDirection(lua_State* L);

    static const luaL_Reg pointLightlib[];
    int pointLight_metatable(lua_State* L);
    static PointLight** checkPointLight(lua_State* L, int index = 1, bool assert = true);
    static int pushPointLight(lua_State* L, PointLight* input);

    static const luaL_Reg spotLightlib[];
    int spotLight_metatable(lua_State* L);
    static SpotLight** checkSpotLight(lua_State* L, int index = 1, bool assert = true);
    static int pushSpotLight(lua_State* L, SpotLight* input);

    static int spotLight_GetDirection(lua_State* L);
    static int spotLight_SetDirection(lua_State* L);

    /// Resources ---------------------------------------------------------------

    /// AnimationRsource
    static const luaL_Reg resAnimationlib[];
    int resAnimation_metatable(lua_State* L);
    static HAnimationSet* checkResAnimation(lua_State* L, int index = 1, bool assert = true);
    static int pushResAnimation(lua_State* L, HAnimationSet input);

    static int resAnimation_Validate(lua_State* L);
    static int resAnimation_Get(lua_State* L);

    /// Attractor
    static const luaL_Reg attractorlib[];
    int attractor_metatable(lua_State* L);
    static Attractor** checkAttractor(lua_State* L, int index = 1, bool assert = true);
    static int pushAttractor(lua_State* L, Attractor* input);

    static int attractor_SetForce(lua_State* L);
    static int attractor_GetForce(lua_State* L);

    /// Game --------------------------------------------------------------------
    static int ToggleFullscreen(lua_State* L);

    /// Helper functions --------------------------------------------------------
    template <typename T>
    friend static bool GetComp(lua_State* L, T** result);

    template <typename T>
    friend static T check(std::string name, lua_State* L, int index, bool assert);

    template <typename T>
    friend static int pushCustomType(std::string name, lua_State* L, T input);

    template <typename T>
    friend static int pushCustomTypeArray(std::string name, lua_State* L, std::vector<T> input);

    /// Other functions ---------------------------------------------------------
    std::set<unsigned> Blacklist;
    std::vector<std::pair<std::string, std::string>> variables;

    void BlacklistSnapshot(lua_State* L);
    void GetGlobals(lua_State* L);

    std::hash<std::string> fn;

    float m_dt;

    /// Focus Function
    static int GotFocus(lua_State* L);

public:

    LuaScript(GameObject* parentObject);
    LuaScript(const std::string& filename);
    ~LuaScript();
    void SetScript(std::string name);
    static void PrintError(const std::string& var, const std::string& err);
    void OnUpdate(float dt) override;
    const std::string& GetScript() const;
    std::vector<std::pair<std::string, std::string>> GetVariables() { return variables; }
    void InitScript();
    void OnCollisionEnter(GameObject* other);
    void OnCollisionPersist(GameObject* other);
    void OnCollisionExit(GameObject* other);

    inline void clean()
    {
        int n = lua_gettop(L);
        lua_pop(L, n);
    }

    template <typename T>
    T get(const std::string& var)
    {
        if (!L)
        {
            PrintError(var, "Script not loaded");
            return get_default<T>();
        }

        T result;

        if (getToStack(var))
            result = lua_get<T>(var);
        else
            result = get_default<T>();

        clean();
        return result;
    }

    template <typename T>
    void set(const std::string& var, const T& val)
    {
        if (!L)
        {
            PrintError(var, "Script not loaded");
            return;
        }

        // if (getToStack(var))
        lua_set<T>(var, val);

        clean();
    }

    bool getToStack(const std::string& var)
    {
        level = 0;
        std::string variable = "";

        for (unsigned i = 0; i < var.size(); ++i)
        {
            if (var.at(i) == '.')
            {
                if (!level)
                    lua_getglobal(L, variable.c_str());
                else
                    lua_getfield(L, -1, variable.c_str());
                if (lua_isnil(L, -1))
                {
                    PrintError(var, variable + " is not defined");
                    return false;
                }
                else
                {
                    variable.clear();
                    ++level;
                }
            }
            else
                variable += var.at(i);
        }

        if (level == 0)
            lua_getglobal(L, variable.c_str());
        else
            lua_getfield(L, -1, variable.c_str());

        if (lua_isnil(L, -1))
        {
            PrintError(var, variable + " is not defined");
            return false;
        }
        return true;
    }

    // generic Gets
    template <typename T>
    T lua_get(const std::string& var)
    {
        return T {};
    }
    template <>
    inline bool lua_get(const std::string& var)
    {
        return (bool)lua_toboolean(L, -1);
    }
    template <>
    inline int lua_get<int>(const std::string& var)
    {
        if (!lua_isnumber(L, -1))
        {
            PrintError(var, "not a integer");
            return 0;
        }
        return (int)lua_tonumber(L, -1);
    }
    template <>
    inline float lua_get<float>(const std::string& var)
    {
        if (!lua_isnumber(L, -1))
        {
            PrintError(var, "not a number");
            return 0;
        }
        return (float)lua_tonumber(L, -1);
    }
    template <>
    inline std::string lua_get<std::string>(const std::string& var)
    {
        if (!lua_isstring(L, -1))
        {
            PrintError(var, "not a string");
            return "Null";
        }
        return std::string { lua_tostring(L, -1) };
    }
    template <>
    inline Vector3 lua_get<Vector3>(const std::string& var)
    {
        if (!checkVector3(L, -1, false))
        {
            PrintError(var, "not a Vec3");
            return Vector3();
        }
        return (Vector3)*checkVector3(L, -1);
    }
    template <>
    inline Vector4 lua_get<Vector4>(const std::string& var)
    {
        if (!checkColor(L, -1, false))
        {
            PrintError(var, "not a Vec4");
            return Vector4();
        }
        return (Vector4)*checkColor(L, -1);
    }
    template <>
    inline HAnimationSet lua_get<HAnimationSet>(const std::string& var)
    {
        if (!checkResAnimation(L, -1, false))
        {
            PrintError(var, "not a HAnimationSet");
            return HAnimationSet {};
        }
        return (HAnimationSet)*checkResAnimation(L, -1);
    }
    template <>
    inline RaycastData_tmp lua_get<RaycastData_tmp>(const std::string& var)
    {
        if (!checkRayCastdata(L, -1, false))
        {
            PrintError(var, "not a RayCastData");
            return RaycastData_tmp {};
        }
        return (RaycastData_tmp)*checkRayCastdata(L, -1);
    }
    template <>
    inline GameObject* lua_get<GameObject*>(const std::string& var)
    {
        if (!checkGameObject(L, -1, false))
        {
            PrintError(var, "not a GameObject");
            return nullptr;
        }
        return (GameObject*)*checkGameObject(L, -1);
    }
    template <>
    inline Transform* lua_get<Transform*>(const std::string& var)
    {
        if (!checkTransform(L, -1, false))
        {
            PrintError(var, "not a Transform");
            return nullptr;
        }
        return (Transform*)*checkTransform(L, -1);
    }
    template <>
    inline RigidbodyComponent* lua_get<RigidbodyComponent*>(const std::string& var)
    {
        if (!checkRigidbody(L, -1, false))
        {
            PrintError(var, "not a Rigidbody");
            return nullptr;
        }
        return (RigidbodyComponent*)*checkRigidbody(L, -1);
    }
    template <>
    inline AudioListener* lua_get<AudioListener*>(const std::string& var)
    {
        if (!checkAudioListener(L, -1, false))
        {
            PrintError(var, "not a AudioListener");
            return nullptr;
        }
        return (AudioListener*)*checkAudioListener(L, -1);
    }
    template <>
    inline AudioEmitter* lua_get<AudioEmitter*>(const std::string& var)
    {
        if (!checkAudioEmitter(L, -1, false))
        {
            PrintError(var, "not a AudioEmitter");
            return nullptr;
        }
        return (AudioEmitter*)*checkAudioEmitter(L, -1);
    }
    template <>
    inline MeshRenderer* lua_get<MeshRenderer*>(const std::string& var)
    {
        if (!checkMeshRenderer(L, -1, false))
        {
            PrintError(var, "not a MeshRenderer");
            return nullptr;
        }
        return (MeshRenderer*)*checkMeshRenderer(L, -1);
    }
    template <>
    inline MeshAnimator* lua_get<MeshAnimator*>(const std::string& var)
    {
        if (!checkMeshAnimator(L, -1, false))
        {
            PrintError(var, "not a MeshAnimator");
            return nullptr;
        }
        return (MeshAnimator*)*checkMeshAnimator(L, -1);
    }
    template <>
    inline LuaScript* lua_get<LuaScript*>(const std::string& var)
    {
        if (!checkluaScript(L, -1, false))
        {
            PrintError(var, "not a luaScript");
            return nullptr;
        }
        return (LuaScript*)*checkluaScript(L, -1);
    }
    template <>
    inline Camera* lua_get<Camera*>(const std::string& var)
    {
        if (!checkCamera(L, -1, false))
        {
            PrintError(var, "not a Camera");
            return nullptr;
        }
        return (Camera*)*checkCamera(L, -1);
    }
    template <>
    inline BoxParticleEmitter* lua_get<BoxParticleEmitter*>(const std::string& var)
    {
        if (!checkBoxParticle(L, -1, false))
        {
            PrintError(var, "not a BoxParticle");
            return nullptr;
        }
        return (BoxParticleEmitter*)*checkBoxParticle(L, -1);
    }
    template <>
    inline CircleParticleEmitter* lua_get<CircleParticleEmitter*>(const std::string& var)
    {
        if (!checkCircleParticle(L, -1, false))
        {
            PrintError(var, "not a CircleParticle");
            return nullptr;
        }
        return (CircleParticleEmitter*)*checkCircleParticle(L, -1);
    }
    template <>
    inline Pathfinding* lua_get<Pathfinding*>(const std::string& var)
    {
        if (!checkPathFinding(L, -1, false))
        {
            PrintError(var, "not a PathFinding");
            return nullptr;
        }
        return (Pathfinding*)*checkPathFinding(L, -1);
    }
    template <>
    inline SphereCollider* lua_get<SphereCollider*>(const std::string& var)
    {
        if (!checkSphereCollider(L, -1, false))
        {
            PrintError(var, "not a SphereCollider");
            return nullptr;
        }
        return (SphereCollider*)*checkSphereCollider(L, -1);
    }
    template <>
    inline TextRenderer* lua_get<TextRenderer*>(const std::string& var)
    {
        if (!checkTextRenderer(L, -1, false))
        {
            PrintError(var, "not a TextRenderer");
            return nullptr;
        }
        return (TextRenderer*)*checkTextRenderer(L, -1);
    }

    // Get array helper function
    template <typename T>
    std::vector<T> GetArrayVariables(T* (*func)(lua_State*, int, bool))
    {
        std::vector<T> result;
        int tableSize = (int)lua_rawlen(L, 1);  /// get size of table
        for (int i = 1; i <= tableSize; i++)
        {
            lua_rawgeti(L, 1, i);
            int Top = lua_gettop(L);
            result.push_back(*(func(L, Top, true)));
        }

        clean();
        return result;
    }

    // Arrays
    template <>
    inline std::vector<float> lua_get(const std::string& name)
    {
        std::cout << "GetArray : default types are not supported" << std::endl;
        return std::vector<float>();
    }
    template <>
    inline std::vector<int> lua_get(const std::string& name)
    {
        std::cout << "GetArray : default types are not supported" << std::endl;
        return std::vector<int>();
    }
    template <>
    inline std::vector<bool> lua_get(const std::string& name)
    {
        std::cout << "GetArray : default types are not supported" << std::endl;
        return std::vector<bool>();
    }
    template <>
    inline std::vector<std::string> lua_get(const std::string& name)
    {
        std::cout << "GetArray : default types are not supported" << std::endl;
        return std::vector<std::string>();
    }

    template <>
    inline std::vector<Vector3> lua_get(const std::string& name)
    {
        if (getToStack(name)) try
            {
                return GetArrayVariables(checkVector3);
            }
            catch (...)
            {
                PrintError(name, "not an array of Vec3");
            }
        return std::vector<Vector3>();
    }
    template <>
    inline std::vector<Vector4> lua_get(const std::string& name)
    {
        if (getToStack(name)) try
            {
                return GetArrayVariables(checkColor);
            }
            catch (...)
            {
                PrintError(name, "not an array of Vec4");
            }
        return std::vector<Vector4>();
    }
    template <>
    inline std::vector<HAnimationSet> lua_get(const std::string& name)
    {
        if (getToStack(name)) try
            {
                return GetArrayVariables(checkResAnimation);
            }
            catch (...)
            {
                PrintError(name, "not an array of HAnimationSet");
            }
        return std::vector<HAnimationSet>();
    }
    template <>
    inline std::vector<RaycastData_tmp> lua_get(const std::string& name)
    {
        if (getToStack(name)) try
            {
                return GetArrayVariables(checkRayCastdata);
            }
            catch (...)
            {
                PrintError(name, "not an array of RayCastData");
            }
        return std::vector<RaycastData_tmp>();
    }
    template <>
    inline std::vector<GameObject*> lua_get(const std::string& name)
    {
        if (getToStack(name)) try
            {
                return GetArrayVariables(checkGameObject);
            }
            catch (...)
            {
                PrintError(name, "not an array of GameObject");
            }
        return std::vector<GameObject*>();
    }
    template <>
    inline std::vector<Transform*> lua_get(const std::string& name)
    {
        if (getToStack(name)) try
            {
                return GetArrayVariables(checkTransform);
            }
            catch (...)
            {
                PrintError(name, "not an array of Transform");
            }
        return std::vector<Transform*>();
    }
    template <>
    inline std::vector<RigidbodyComponent*> lua_get(const std::string& name)
    {
        if (getToStack(name)) try
            {
                return GetArrayVariables(checkRigidbody);
            }
            catch (...)
            {
                PrintError(name, "not an array of Rigidbody");
            }
        return std::vector<RigidbodyComponent*>();
    }
    template <>
    inline std::vector<AudioListener*> lua_get(const std::string& name)
    {
        if (getToStack(name)) try
            {
                return GetArrayVariables(checkAudioListener);
            }
            catch (...)
            {
                PrintError(name, "not an array of AudioListener");
            }
        return std::vector<AudioListener*>();
    }
    template <>
    inline std::vector<AudioEmitter*> lua_get(const std::string& name)
    {
        if (getToStack(name)) try
            {
                return GetArrayVariables(checkAudioEmitter);
            }
            catch (...)
            {
                PrintError(name, "not an array of AudioEmitter");
            }
        return std::vector<AudioEmitter*>();
    }
    template <>
    inline std::vector<MeshRenderer*> lua_get(const std::string& name)
    {
        if (getToStack(name)) try
            {
                return GetArrayVariables(checkMeshRenderer);
            }
            catch (...)
            {
                PrintError(name, "not an array of MeshRenderer");
            }
        return std::vector<MeshRenderer*>();
    }
    template <>
    inline std::vector<MeshAnimator*> lua_get(const std::string& name)
    {
        if (getToStack(name)) try
            {
                return GetArrayVariables(checkMeshAnimator);
            }
            catch (...)
            {
                PrintError(name, "not an array of MeshAnimator");
            }
        return std::vector<MeshAnimator*>();
    }
    template <>
    inline std::vector<LuaScript*> lua_get(const std::string& name)
    {
        if (getToStack(name)) try
            {
                return GetArrayVariables(checkluaScript);
            }
            catch (...)
            {
                PrintError(name, "not an array of luaScript");
            }
        return std::vector<LuaScript*>();
    }
    template <>
    inline std::vector<Camera*> lua_get(const std::string& name)
    {
        if (getToStack(name)) try
            {
                return GetArrayVariables(checkCamera);
            }
            catch (...)
            {
                PrintError(name, "not an array of Camera");
            }
        return std::vector<Camera*>();
    }
    template <>
    inline std::vector<BoxParticleEmitter*> lua_get(const std::string& name)
    {
        if (getToStack(name)) try
            {
                return GetArrayVariables(checkBoxParticle);
            }
            catch (...)
            {
                PrintError(name, "not an array of BoxParticle");
            }
        return std::vector<BoxParticleEmitter*>();
    }
    template <>
    inline std::vector<CircleParticleEmitter*> lua_get(const std::string& name)
    {
        if (getToStack(name)) try
            {
                return GetArrayVariables(checkCircleParticle);
            }
            catch (...)
            {
                PrintError(name, "not an array of CircleParticle");
            }
        return std::vector<CircleParticleEmitter*>();
    }
    template <>
    inline std::vector<Pathfinding*> lua_get(const std::string& name)
    {
        if (getToStack(name)) try
            {
                return GetArrayVariables(checkPathFinding);
            }
            catch (...)
            {
                PrintError(name, "not an array of PathFinding");
            }
        return std::vector<Pathfinding*>();
    }
    template <>
    inline std::vector<SphereCollider*> lua_get(const std::string& name)
    {
        if (getToStack(name)) try
            {
                return GetArrayVariables(checkSphereCollider);
            }
            catch (...)
            {
                PrintError(name, "not an array of SphereCollider");
            }
        return std::vector<SphereCollider*>();
    }
    template <>
    inline std::vector<TextRenderer*> lua_get(const std::string& name)
    {
        if (getToStack(name)) try
            {
                return GetArrayVariables(checkTextRenderer);
            }
            catch (...)
            {
                PrintError(name, "not an array of TextRenderer");
            }
        return std::vector<TextRenderer*>();
    }

    // Set
    template <typename T>
    void lua_set(const std::string& var, T b)
    {
        return;
    }
    template <>
    inline void lua_set(const std::string& var, bool b)
    {
        lua_pushboolean(L, b);
        lua_setglobal(L, var.c_str());
    }
    template <>
    inline void lua_set(const std::string& var, float f)
    {
        lua_pushnumber(L, f);
        lua_setglobal(L, var.c_str());
    }
    template <>
    inline void lua_set(const std::string& var, int i)
    {
        lua_pushinteger(L, i);
        lua_setglobal(L, var.c_str());
    }
    template <>
    inline void lua_set(const std::string& var, std::string s)
    {
        lua_pushstring(L, s.c_str());
        lua_setglobal(L, var.c_str());
    }
    template <>
    inline void lua_set(const std::string& var, Vector3 b)
    {
        pushVector3(L, b);
        lua_setglobal(L, var.c_str());
    }
    template <>
    inline void lua_set(const std::string& var, Vector4 b)
    {
        pushColor(L, b);
        lua_setglobal(L, var.c_str());
    }
    template <>
    inline void lua_set(const std::string& var, HAnimationSet b)
    {
        pushResAnimation(L, b);
        lua_setglobal(L, var.c_str());
    }
    template <>
    inline void lua_set(const std::string& var, RaycastData_tmp b)
    {
        pushRayCastdata(L, b);
        lua_setglobal(L, var.c_str());
    }
    template <>
    inline void lua_set(const std::string& var, GameObject* b)
    {
        pushGameObject(L, b);
        lua_setglobal(L, var.c_str());
    }
    template <>
    inline void lua_set(const std::string& var, Transform* b)
    {
        pushTransform(L, b);
        lua_setglobal(L, var.c_str());
    }
    template <>
    inline void lua_set(const std::string& var, RigidbodyComponent* b)
    {
        pushRigidBody(L, b);
        lua_setglobal(L, var.c_str());
    }
    template <>
    inline void lua_set(const std::string& var, AudioListener* b)
    {
        pushAudioListener(L, b);
        lua_setglobal(L, var.c_str());
    }
    template <>
    inline void lua_set(const std::string& var, AudioEmitter* b)
    {
        pushAudioEmitter(L, b);
        lua_setglobal(L, var.c_str());
    }
    template <>
    inline void lua_set(const std::string& var, MeshRenderer* b)
    {
        pushMeshRenderer(L, b);
        lua_setglobal(L, var.c_str());
    }
    template <>
    inline void lua_set(const std::string& var, MeshAnimator* b)
    {
        pushMeshAnimator(L, b);
        lua_setglobal(L, var.c_str());
    }
    template <>
    inline void lua_set(const std::string& var, LuaScript* b)
    {
        pushluaScript(L, b);
        lua_setglobal(L, var.c_str());
    }
    template <>
    inline void lua_set(const std::string& var, Camera* b)
    {
        pushCamera(L, b);
        lua_setglobal(L, var.c_str());
    }
    template <>
    inline void lua_set(const std::string& var, BoxParticleEmitter* b)
    {
        pushBoxParticle(L, b);
        lua_setglobal(L, var.c_str());
    }
    template <>
    inline void lua_set(const std::string& var, CircleParticleEmitter* b)
    {
        pushCircleParticle(L, b);
        lua_setglobal(L, var.c_str());
    }
    template <>
    inline void lua_set(const std::string& var, Pathfinding* b)
    {
        pushPathFinding(L, b);
        lua_setglobal(L, var.c_str());
    }
    template <>
    inline void lua_set(const std::string& var, SphereCollider* b)
    {
        pushSphereCollider(L, b);
        lua_setglobal(L, var.c_str());
    }
    template <>
    inline void lua_set(const std::string& var, TextRenderer* b)
    {
        pushTextRenderer(L, b);
        lua_setglobal(L, var.c_str());
    }

    template <typename T>
    T get_default()
    {
        return T {};
    }

    template <>
    inline std::string get_default<std::string>()
    {
        return "null";
    }

    void PrintName() { std::cout << m_Name << std::endl; }

    REFLECT()
};
