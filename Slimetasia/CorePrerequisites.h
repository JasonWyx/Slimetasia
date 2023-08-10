#pragma once
#include <cassert>
#include <list>
#include <memory>
#include <vector>
#include <array>

#include "Logger.h"

// Math includes
#include "MathDefs.h"

// Resource includes
#include "Animation.h"
#include "Font.h"
#include "Material.h"
#include "Mesh.h"
#include "ResourceHandle.h"
#include "Shader.h"
#include "Texture.h"

// Memory Manager/ Allocators.
#include "MemoryManager.h"

using HAnimationSet = ResourceHandle<AnimationSet>;
using HMesh = ResourceHandle<Mesh>;
using HMaterial = ResourceHandle<Material>;
using HTexture = ResourceHandle<Texture>;
using HShader = ResourceHandle<Shader>;
using HFont = ResourceHandle<Font>;

class Scene;
class Layer;
class GameObject;
class IComponent;

using LayerList = std::list<Layer*>;
using GameObjectList = std::list<GameObject*>;
using IComponentList = std::list<IComponent*>;

// to allocate memory, s_MemoryMgr.Allocate(AllocationType::Base, sizeof(object)); returns a void pointer
// to free memory, s_MemoryMgr.ReleaseMemory(AllocationType::Base, object*, sizeof(object));
// there are 3 types of allocation type; Base, Frame and Pool, by default only need to use Base.
