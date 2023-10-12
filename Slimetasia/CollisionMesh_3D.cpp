#include "CollisionMesh_3D.h"

#include "GameObject.h"
#include "Octree.h"
#include "PhysicsSystem.h"
#include "Renderer.h"
#include "RigidbodyComponent.h"

// Vector3 CuboidColliderMesh::s_Compass[6] = {
//        Vector3(0.f, 1.f, 0.f),	// up
//        Vector3(0.f, -1.f, 0.f),	// down
//        Vector3(-1.f, 0.f, 0.f),	// left
//        Vector3(1.f, 0.f, 0.f),	// right
//        Vector3(0.f, 0.f, 1.f),		// front
//        Vector3(0.f, 0.f, -1.f)	// back
//};

// void CollisionCheck(void * in)
//{
//  CollisionInfoIn * m_in = static_cast<CollisionInfoIn *>(in);
//  IntersectionData * col_data = &(m_in->obj1->GetComponent< CollisionMesh_3D>()->IsColliding(m_in->obj2->GetComponent< CollisionMesh_3D>()));
//  if (col_data->m_is_intersect)
//  {
//    m_in->obj1->GetComponent<CollisionMesh_3D>()->AddIntersection(*col_data);
//    m_in->obj2->GetComponent<CollisionMesh_3D>()->AddIntersection(-(*col_data));
//  }
//}
//
// void RaycastCheck(void* in, std::vector<RaycastData_tmp>& vec)
//{
//	CollisionRayIn * m_in = static_cast<CollisionRayIn *>(in);
//	RaycastData_tmp info;
//	if (m_in->obj1->GetComponent<CollisionMesh_3D>()->Raycast(m_in->ray, info))
//		vec.push_back(info);
//}

CollisionMesh_3D::CollisionMesh_3D(GameObject* parentObject, const std::string& name, const CollisionShapeType& shapetype, const CollisionShape& shape, const float& rad)
    : IComponent { parentObject, name }
    , m_ColShapeType { shapetype }
    , m_ColShape { shape }
    , m_radius { rad }
{
    if (parentObject)
    {
        parentObject->AddIfDoesntExist<Transform>();

        if (parentObject->GetParentLayer() && parentObject->GetParentLayer()->GetParentScene()) GetInstance(PhysicsSystem).Register3DCollider(this);
    }
}

void CollisionMesh_3D::SetPosition(const Vector3& newPos)
{
    assert(m_OwnerObject);

    m_OwnerObject->GetComponent<Transform>()->SetWorldPosition(newPos);
}

// IntersectionData CollisionMesh_3D::IsColliding(CollisionMesh_3D* othermesh)
//{
//  if (othermesh == nullptr)
//    return IntersectionData();
//  switch (m_mesh_type)
//  {
//  case SPHERE:
//    switch (othermesh->m_mesh_type)
//    {
//    case SPHERE:
//      return IsCollidingSpherevsSphere(othermesh);
//    case CUBOID:
//      return IsCollidingSpherevsCuboid(othermesh);
//    default:
//      std::cout << "The CollisionMesh's m_mesh_type is invalid." << std::endl;
//      assert(0);
//      return IntersectionData{};
//    }
//  case CUBOID:
//    switch (othermesh->m_mesh_type)
//    {
//    case SPHERE:
//      return IsCollidingCuboidvsSphere(othermesh);
//    case CUBOID:
//      return IsCollidingCuboidvsCuboid(othermesh);
//    default:
//      std::cout << "The CollisionMesh's m_mesh_type is invalid." << std::endl;
//      assert(0);
//      return IntersectionData{};
//    }
//  default:
//    std::cout << "The othermesh parameter in CollisionMesh2D_IsColliding is invalid." << std::endl;
//    assert(0);
//    return IntersectionData{};
//  }
//}
//
// IntersectionData CollisionMesh_3D::IsCollidingCuboidvsCuboid(CollisionMesh_3D*& othermesh)
//{
//  const auto first = dynamic_cast<CuboidColliderMesh*>(this);
//  const auto second = dynamic_cast<CuboidColliderMesh*>(othermesh);
//  const auto firstpos = GetPosition() + GetOffset();
//  const auto secondpos = second->GetPosition() + second->GetOffset();
//  auto firstmax = firstpos, firstmin = firstpos;
//  auto secondmax = secondpos, secondmin = secondpos;
//
//  firstmax[0] += first->GetWidth() / 2.f;
//  firstmax[1] += first->GetHeight() / 2.f;
//  firstmax[2] += first->GetDepth() / 2.f;
//  firstmin[0] -= first->GetWidth() / 2.f;
//  firstmin[1] -= first->GetHeight() / 2.f;
//  firstmin[2] -= first->GetDepth() / 2.f;
//
//  secondmax[0] += second->GetWidth() / 2.f;
//  secondmax[1] += second->GetHeight() / 2.f;
//  secondmax[2] += second->GetDepth() / 2.f;
//  secondmin[0] -= second->GetWidth() / 2.f;
//  secondmin[1] -= second->GetHeight() / 2.f;
//  secondmin[2] -= second->GetDepth() / 2.f;
//
//  if ((firstmax[0] >= secondmin[0] && firstmax[0] <= secondmax[0] &&
//	   firstmax[1] >= secondmin[1] && firstmax[1] <= secondmax[1] &&
//	   firstmax[2] >= secondmin[2] && firstmax[2] <= secondmax[2]) ||
//	  (firstmin[0] >= secondmin[0] && firstmin[0] <= secondmax[0] &&
//       firstmin[1] >= secondmin[1] && firstmin[1] <= secondmax[1] &&
//       firstmin[2] >= secondmin[2] && firstmin[2] <= secondmax[2]) ||
//      (secondmax[0] >= firstmin[0] && secondmax[0] <= firstmax[0] &&
//       secondmax[1] >= firstmin[1] && secondmax[1] <= firstmax[1] &&
//       secondmax[2] >= firstmin[2] && secondmax[2] <= firstmax[2]) ||
//      (secondmin[0] >= firstmin[0] && secondmin[0] <= firstmax[0] &&
//       secondmin[1] >= firstmin[1] && secondmin[1] <= firstmax[1] &&
//       secondmin[2] >= firstmin[2] && secondmin[2] <= firstmax[2]))
//  {
//
//    Vector3 firstclosestpt = first->GetClosestPointInCuboid(secondpos);
//    Vector3 secondclosestpt = second->GetClosestPointInCuboid(firstpos);
//    auto dist_len = firstclosestpt - secondclosestpt;
//	auto dist = firstpos - secondpos;
//	  Vector3 direction{};
//	auto absx = abs(dist[0]);
//	auto absy = abs(dist[1]);
//	auto absz = abs(dist[2]);
//
//	  if(absx > absy && absx > absz)
//	  {
//		  if(dist[0] < 0.f)
//			direction = dist.Normalized().Dot(Vector3(1.f, 0.f, 0.f)) * dist;
//		  else
//			direction = dist.Normalized().Dot(Vector3(-1.f, 0.f, 0.f)) * dist;
//	  }
//	  else if(absy > absz)
//	  {
//		  if(dist[1] < 0.f)
//			direction = dist.Normalized().Dot(Vector3(0.f, -1.f, 0.f)) * dist;
//		  else
//			direction = dist.Normalized().Dot(Vector3(0.f, 1.f, 0.f)) * dist;
//	  }
//	  else
//	  {
//		  if(dist[2] < 0.f)
//			direction = dist.Normalized().Dot(Vector3(0.f, 0.f, 1.f)) * dist;
//		  else
//			direction = dist.Normalized().Dot(Vector3(0.f, 0.f, -1.f)) * dist;
//	  }
//
//    IntersectionData tmp{ true , dist_len.Length(), direction, dist_len };
//
//	tmp.m_FirstRigidbody = m_OwnerObject->GetComponent<RigidbodyComponent>();
//	tmp.m_SecondRigidbody = othermesh->m_OwnerObject->GetComponent<RigidbodyComponent>();
//
//	return tmp;
//  }
//
//  return IntersectionData(false);
//}
//
// IntersectionData CollisionMesh_3D::IsCollidingCuboidvsSphere(CollisionMesh_3D*& othermesh)
//{
//  const auto first = dynamic_cast<CuboidColliderMesh*>(this);
//  const auto second = dynamic_cast<SphereColliderMesh*>(othermesh);
//  const auto firstpos = GetPosition() + GetOffset();
//  const auto secondpos = second->GetPosition() + second->GetOffset();
//
//  Vector3 firstclosestpt = first->GetClosestPointInCuboid(secondpos);
//  float radsq = second->m_radius * second->m_radius;
//  Vector3 dist_vec = firstclosestpt - secondpos;
//  float dist = dist_vec.SquareLength();
//
//  if (dist > radsq || dist < EPSILON)
//    return IntersectionData(false);
//
//  Vector3 normal = dist_vec.Normalized();
//  auto dist_len = sqrtf(dist);
//
//  IntersectionData tmp(true, dist, dist_vec, normal * (second->m_radius - dist_len));
//
//  tmp.m_FirstRigidbody = m_OwnerObject->GetComponent<RigidbodyComponent>();
//  tmp.m_SecondRigidbody = othermesh->m_OwnerObject->GetComponent<RigidbodyComponent>();
//
//  return tmp;
//}
//
// IntersectionData CollisionMesh_3D::IsCollidingCuboidvsPolyhedra(CollisionMesh_3D*& othermesh)
//{
//  //to be implemented.
//  return IntersectionData{};
//}
//
// IntersectionData CollisionMesh_3D::IsCollidingSpherevsCuboid(CollisionMesh_3D*& othermesh)
//{
//  const auto first = dynamic_cast<SphereColliderMesh*>(this);
//  const auto second = dynamic_cast<CuboidColliderMesh*>(othermesh);
//  const auto firstpos = GetPosition() + GetOffset();
//  const auto secondpos = second->GetPosition() + second->GetOffset();
//
//  Vector3 secondclosestpt = second->GetClosestPointInCuboid(firstpos);
//  float radsq = m_radius * m_radius;
//  Vector3 dist_vec = firstpos - secondclosestpt;
//  float dist = dist_vec.SquareLength();
//
//  if (dist > radsq || dist < EPSILON)
//    return IntersectionData(false);
//
//  Vector3 normal = dist_vec.Normalized();
//  auto dist_len = sqrtf(dist);
//
//  IntersectionData tmp(true, dist, dist_vec, normal * (m_radius - dist_len));
//
//  tmp.m_FirstRigidbody = m_OwnerObject->GetComponent<RigidbodyComponent>();
//  tmp.m_SecondRigidbody = othermesh->m_OwnerObject->GetComponent<RigidbodyComponent>();
//
//  return tmp;
//}
//
// IntersectionData CollisionMesh_3D::IsCollidingSpherevsSphere(CollisionMesh_3D*& othermesh)
//{
//  const auto first = dynamic_cast<SphereColliderMesh*>(this);
//  const auto second = dynamic_cast<SphereColliderMesh*>(othermesh);
//  const auto firstpos = GetPosition() + GetOffset();
//  const auto secondpos = second->GetPosition() + second->GetOffset();
//
//  auto dist_vec = firstpos - secondpos;
//  auto dist = dist_vec.SquareLength();
//  auto rad = first->GetRadius() + second->GetRadius();
//
//  if (dist > rad * rad || dist < EPSILON)
//    return IntersectionData(false);
//  auto dist_len = sqrtf(dist);
//
//  IntersectionData data(true, dist, dist_vec, dist_vec.Normalized() * (rad - dist_len), true);
//  data.m_FirstRigidbody = m_OwnerObject->GetComponent<RigidbodyComponent>();
//  data.m_SecondRigidbody = othermesh->m_OwnerObject->GetComponent<RigidbodyComponent>();
//  return data;
//}
//
// IntersectionData CollisionMesh_3D::IsCollidingSpherevsPolyhedra(CollisionMesh_3D*& othermesh)
//{
//  //to be implemented.
//  return IntersectionData{};
//}
//
// IntersectionData CollisionMesh_3D::IsCollidingPolyhedravsCuboid(CollisionMesh_3D*& othermesh)
//{
//  //to be implemented.
//  return IntersectionData{};
//}
//
// IntersectionData CollisionMesh_3D::IsCollidingPolyhedravsSphere(CollisionMesh_3D*& othermesh)
//{
//  //to be implemented.
//  return IntersectionData{};
//}
//
// IntersectionData CollisionMesh_3D::IsCollidingPolyhedravsPolyhedra(CollisionMesh_3D*& othermesh)
//{
//  //to be implemented.
//  return IntersectionData{};
//}

bool CollisionMesh_3D::ContainsPoint(const Vector3& pt)
{
    return false;
}

bool CollisionMesh_3D::Raycast(const Ray& ray, RaycastData_tmp& data)
{
    std::cout << "Failed to override the base function\n";
    assert(false);
    return false;
}

void CollisionMesh_3D::ComputeBounds(Vector3& min, Vector3& max) const
{
    std::cout << "Failed to override the base function\n";
    assert(false);
    return;
}

// bool CollisionMesh_3D::IsCollidingWithMouse(const Vector2& mousepos)
//{
//  switch (m_mesh_type)
//  {
//  case SPHERE:
//    return IsCollidingWithMouseSphere(mousepos);
//    break;
//  case CUBOID:
//    return IsCollidingWithMouseCuboid(mousepos);
//    break;
//    /*case POLYHEDRA:
//      return IsCollidingWithMousePolyhedra(mousepos);*/
//    break;
//  default:
//    std::cout << "The CollisionMesh's m_mesh_type is invalid." << std::endl;
//    return false;
//  }
//}
//
// bool CollisionMesh_3D::IsCollidingWithMouseCuboid(const Vector2& mousepos)
//{
//  auto tmp = dynamic_cast<CuboidColliderMesh*>(this);
//  auto pos = GetPosition() + GetOffset();
//  auto max = pos, min = pos;
//  max[0] += tmp->GetWidth() / 2.f;
//  max[1] += tmp->GetHeight() / 2.f;
//  min[0] -= tmp->GetWidth() / 2.f;
//  min[1] -= tmp->GetHeight() / 2.f;
//  return mousepos[0] >= min[0] && mousepos[0] <= max[0] && mousepos[1] >= min[1] && mousepos[1] <= max[1];
//}
//
// bool CollisionMesh_3D::IsCollidingWithMouseSphere(const Vector2& mousepos)
//{
//  //to be implemented.
//  return false;
//}
//
// bool CollisionMesh_3D::IsCollidingWithMousePolyhedra(const Vector2& mousepos)
//{
//  //to be implemented.
//  return false;
//}

void CollisionMesh_3D::Register()
{
    // ISystem<PhysicsSystem>::InstancePtr()->Register3DCollider(this);

    // ISystem<PhysicsSystem>::Instance().Register3DCollider(this);
}

void CollisionMesh_3D::DebugDraw() {}

// void CollisionMesh_3D::EnableLayerCollision(const unsigned& layer)
//{
//  if (layer > 8)
//  {
//    std::cout << "layer passed into CollisionMesh_3D::EnableLayerCollision is greater than the max layers allowed." << std::endl;
//    return;
//  }
//  char tmp = 0x01;
//  tmp <<= layer;
//  m_collision_layer |= tmp;
//}
//
// void CollisionMesh_3D::DisableLayerCollision(const unsigned& layer)
//{
//  if (layer > 8)
//  {
//    std::cout << "layer passed into CollisionMesh_3D::DisableLayerCollision is greater than the max layers allowed." << std::endl;
//    return;
//  }
//  char tmp = 0x01;
//  tmp = ~tmp;
//  tmp <<= layer;
//  m_collision_layer &= tmp;
//}

void CollisionMesh_3D::AddIntersection(const IntersectionData& data)
{
    m_intersectList.emplace_back(data);
}

// void CollisionMesh_3D::ComputeRadius()
//{
//  switch (m_mesh_type)
//  {
//  case CUBOID:
//    m_radius = dynamic_cast<CuboidColliderMesh*>(this)->GetLongestEdge() / 2.f;
//    break;
//  default:
//    break;
//  }
//}

void CollisionMesh_3D::SendMessages()
{
    RigidbodyComponent* r = GetOwner()->GetComponent<RigidbodyComponent>();
    if (!r) return;
    m_prevIntersect = m_currentIntersect;
    m_currentIntersect = m_intersectList;
    m_intersectList.clear();
    for (auto& pIntersect : m_prevIntersect)
    {
        if (!pIntersect.m_SecondRigidbody || !pIntersect.m_FirstRigidbody) continue;
        if (std::find(m_currentIntersect.begin(), m_currentIntersect.end(), pIntersect) == m_currentIntersect.end())
        {
            // TODO: FIX
            auto scripts = GetOwner()->GetLuaScripts();
            for (auto& s : scripts)
            {
                if (pIntersect.m_FirstRigidbody == r && pIntersect.m_SecondRigidbody)
                    s->OnCollisionExit(pIntersect.m_SecondRigidbody->GetOwner());
                else if (pIntersect.m_FirstRigidbody && pIntersect.m_SecondRigidbody == r)
                    s->OnCollisionExit(pIntersect.m_FirstRigidbody->GetOwner());
            }
        }
        else
        {
            // TODO: FIX
            auto scripts = GetOwner()->GetLuaScripts();
            for (auto& s : scripts)
            {
                if (pIntersect.m_FirstRigidbody == r && pIntersect.m_SecondRigidbody)
                    s->OnCollisionPersist(pIntersect.m_SecondRigidbody->GetOwner());
                else if (pIntersect.m_FirstRigidbody && pIntersect.m_SecondRigidbody == r)
                    s->OnCollisionPersist(pIntersect.m_FirstRigidbody->GetOwner());
            }
        }
    }
    for (auto& cIntersect : m_currentIntersect)
    {
        if (std::find(m_prevIntersect.begin(), m_prevIntersect.end(), cIntersect) == m_prevIntersect.end())
        {
            // TODO: FIX
            auto scripts = GetOwner()->GetLuaScripts();
            for (auto& s : scripts)
            {
                if (cIntersect.m_FirstRigidbody == r && cIntersect.m_SecondRigidbody)
                    s->OnCollisionEnter(cIntersect.m_SecondRigidbody->GetOwner());
                else if (cIntersect.m_FirstRigidbody && cIntersect.m_SecondRigidbody == r)
                    s->OnCollisionEnter(cIntersect.m_FirstRigidbody->GetOwner());
            }
        }
    }
}

void CollisionMesh_3D::ComputeMass() {}

void CollisionMesh_3D::ComputeInertiaTensor(Matrix3& tensor, const float& mass) const {}

// Vector3 SphereColliderMesh::GetClosestPointInSphere(const Vector2& pt) const
//{
//  const auto pos = GetPosition();
//  Vector3 diff(pos[0] - pt[0], pos[1] - pt[1], pos[2]);
//  if (diff[0] > -EPSILON && diff[0] < EPSILON &&
//    diff[1] > -EPSILON && diff[1] < EPSILON &&
//    diff[2] > -EPSILON && diff[2] < EPSILON)
//    return Vector3(0.f, 0.f, 0.f);
//
//  return pos - diff.Normalize() * m_radius;
//}
//
// Vector3 SphereColliderMesh::GetClosestPointInSphere(const Vector3& pt) const
//{
//  const auto pos = GetPosition();
//  auto diff(pos - pt);
//  if (diff[0] > -EPSILON && diff[0] < EPSILON &&
//    diff[1] > -EPSILON && diff[1] < EPSILON &&
//    diff[2] > -EPSILON && diff[2] < EPSILON)
//    return Vector3(0.f, 0.f, 0.f);
//
//  return pos - diff.Normalize() * m_radius;
//}
//
// bool SphereColliderMesh::Raycast(const Ray & ray, RaycastData_tmp & data)
//{
//	if (ContainsPoint(ray.m_start))
//		return false;
//
//	Vector3 m = GetPosition() + GetOffset() - ray.m_start;
//	if (m.SquareLength() < m_radius * m_radius)
//	{
//		data.m_HitFrac = 0.f;
//		data.m_WorldHitPt = ray.m_start;
//		data.m_WorldNormal = Vector3(0.f, 1.f, 0.f);
//		data.m_HitObject = m_OwnerObject;
//		return true;
//	}
//
//	float t = 0.f;
//	auto a = ray.m_dir.Dot(ray.m_dir);
//	auto b = m.Dot(ray.m_dir) * -2.f;
//	auto c = m.Dot(m) - m_radius * m_radius;
//	auto tmp = b * b - 4.f * a * c;
//	if (tmp < 0.f)
//		return false;
//	if (!tmp)
//		t = (-b + sqrtf(tmp)) / (2.f * a);
//	else
//	{
//		auto t1 = (-b + sqrtf(tmp)) / (2.f * a);
//		auto t2 = (-b - sqrtf(tmp)) / (2.f * a);
//		t = t1 < t2 && t1 >= 0.f ? t1 : t2;
//	}
//
//	if (t > 0.f)
//	{
//		data.m_HitFrac = t;
//		data.m_WorldHitPt = ray.m_start + t * ray.m_dir;
//		data.m_WorldNormal = m.Normalized();
//		data.m_HitObject = m_OwnerObject;
//		return true;
//	}
//
//	return false;
//}

// bool SphereColliderMesh::ContainsPoint(const Vector3 & pt)
//{
//	float lenSq = (GetPosition() - pt).SquareLength();
//
//	return lenSq <= m_radius * m_radius;
//}
//
// void SphereColliderMesh::DebugDraw()
//{
//  auto pid = GetParentObject()->GetParentLayer()->GetId();
//  auto currentLayerID = Renderer::Instance().GetCurrentEditorLayer()->GetId();
//  if (pid != currentLayerID) return;
//  std::vector<Vector3> pts;
//  auto u = Vector3{ 1.f, 0.f, 0.f }, v = Vector3{ 0.f, 1.f, 0.f }, w = Vector3{ 0.f, 0.f, 1.f };
//  const auto mypos = GetPosition();
//  //drawing the disc.
//  for (auto i = 0u, j = 0u; i < discpoints; ++i)
//  {
//    j = i + 1u == discpoints ? 0u : i + 1u;
//
//    pts.emplace_back(mypos + m_radius * (v * cosf(j * DITHER) + w * sinf(j * DITHER)));
//    pts.emplace_back(mypos + m_radius * (v * cosf(i * DITHER) + w * sinf(i * DITHER)));
//
//    pts.emplace_back(mypos + m_radius * (u * cosf(j * DITHER) + w * sinf(j * DITHER)));
//    pts.emplace_back(mypos + m_radius * (u * cosf(i * DITHER) + w * sinf(i * DITHER)));
//
//    pts.emplace_back(mypos + m_radius * (u * cosf(j * DITHER) + v * sinf(j * DITHER)));
//    pts.emplace_back(mypos + m_radius * (u * cosf(i * DITHER) + v * sinf(i * DITHER)));
//  }
//  auto cam = const_cast<SceneLayer*>(Renderer::Instance().GetCurrentEditorLayer())->GetEditorCamera();
//  if (cam)
//  {
//    const auto eye = cam->GetTransform()->GetWorldPosition();
//    const auto eyevec = mypos - eye;
//    const auto d = eyevec.Length();
//    const auto l = sqrtf(d * d - (m_radius * m_radius));
//    const auto newrad = (m_radius / d) * l;
//    const auto z = (m_radius / d) * sqrt(d * d - (l * l));
//    const auto newcenter = mypos + z * ((eye - mypos) / d);
//
//    auto new_v = w.Cross(eyevec);
//    new_v = new_v.Normalized();
//    auto new_w = eyevec.Cross(new_v);
//    new_w = new_w.Normalized();
//
//    for (auto i = 0u, j = 0u; i < discpoints; ++i)
//    {
//      j = i + 1u >= discpoints ? 0u : i + 1u;
//      pts.emplace_back(newcenter + newrad * (new_v * cosf(j * DITHER) + new_w * sinf(j * DITHER)));
//      pts.emplace_back(newcenter + newrad * (new_v * cosf(i * DITHER) + new_w * sinf(i * DITHER)));
//    }
//  }
//
//  Renderer::Instance().DrawDebug(currentLayerID, pts, Color4(0.0f, 1.0f, 0.0f, 1.0f), DebugPrimitiveType::Lines);
//}
//
// Vector3 CuboidColliderMesh::GetClosestPointInCuboid(const Vector2 & pt) const
//{
//  auto tmp = GetPosition();
//  auto max = tmp + Vector3(m_width / 2.f, m_height / 2.f, m_depth / 2.f);
//  auto min = tmp - Vector3(m_width / 2.f, m_height / 2.f, m_depth / 2.f);
//  tmp[0] = pt[0] > max[0] ? max[0] : pt[0] < min[0] ? min[0] : pt[0];
//  tmp[1] = pt[1] > max[1] ? max[1] : pt[1] < min[1] ? min[1] : pt[1];
//
//  return tmp;
//}
//
// Vector3 CuboidColliderMesh::GetClosestPointInCuboid(const Vector3 & pt) const
//{
//  auto tmp = GetPosition();
//  auto max = tmp + Vector3(m_width / 2.f, m_height / 2.f, m_depth / 2.f);
//  auto min = tmp - Vector3(m_width / 2.f, m_height / 2.f, m_depth / 2.f);
//  tmp[0] = pt[0] > max[0] ? max[0] : pt[0] < min[0] ? min[0] : pt[0];
//  tmp[1] = pt[1] > max[1] ? max[1] : pt[1] < min[1] ? min[1] : pt[1];
//  tmp[2] = pt[2] > max[2] ? max[2] : pt[2] < min[2] ? min[2] : pt[2];
//
//  return tmp;
//}
//
// float CuboidColliderMesh::GetLongestEdge()const
//{
//  return m_height > m_width ? m_height > m_depth ? m_height : m_depth : m_width > m_depth ? m_width : m_depth;
//}
//
// void CuboidColliderMesh::DebugDraw()
//{
//  auto pid = GetParentObject()->GetParentLayer()->GetId();
//  auto currentLayerID = Renderer::Instance().GetCurrentEditorLayer()->GetId();
//  if (pid != currentLayerID) return;
//
//  std::vector<Vector3> pts;
//  float halfWidth = m_width * 0.5f;
//  float halfHeight = m_height * 0.5f;
//  float halfDepth = m_depth * 0.5f;
//  Vector3 minPt(GetPosition()[0] - halfWidth, GetPosition()[1] - halfHeight, GetPosition()[2] - halfDepth);
//  Vector3 maxPt(GetPosition()[0] + halfWidth, GetPosition()[1] + halfHeight, GetPosition()[2] + halfDepth);
//  pts.emplace_back(maxPt);
//  pts.emplace_back(Vector3{ maxPt[0], maxPt[1] ,minPt[2] });
//
//  pts.emplace_back(maxPt);
//  pts.emplace_back(Vector3{ maxPt[0], minPt[1] ,maxPt[2] });
//
//  pts.emplace_back(maxPt);
//  pts.emplace_back(Vector3{ minPt[0], maxPt[1] ,maxPt[2] });
//
//  pts.emplace_back(Vector3{ minPt[0], minPt[1] ,maxPt[2] });
//  pts.emplace_back(Vector3{ maxPt[0], minPt[1] ,maxPt[2] });
//
//  pts.emplace_back(Vector3{ maxPt[0], minPt[1] ,minPt[2] });
//  pts.emplace_back(Vector3{ maxPt[0], minPt[1] ,maxPt[2] });
//
//  pts.emplace_back(Vector3{ maxPt[0], maxPt[1] ,minPt[2] });
//  pts.emplace_back(Vector3{ maxPt[0], minPt[1] ,minPt[2] });
//
//  pts.emplace_back(minPt);
//  pts.emplace_back(Vector3{ maxPt[0], minPt[1] ,minPt[2] });
//
//  pts.emplace_back(minPt);
//  pts.emplace_back(Vector3{ minPt[0], minPt[1] ,maxPt[2] });
//
//  pts.emplace_back(minPt);
//  pts.emplace_back(Vector3{ minPt[0], maxPt[1] ,minPt[2] });
//
//  pts.emplace_back(Vector3{ minPt[0], maxPt[1] ,maxPt[2] });
//  pts.emplace_back(Vector3{ minPt[0], minPt[1] ,maxPt[2] });
//
//  pts.emplace_back(Vector3{ minPt[0], maxPt[1] ,minPt[2] });
//  pts.emplace_back(Vector3{ minPt[0], maxPt[1] ,maxPt[2] });
//
//  pts.emplace_back(Vector3{ minPt[0], maxPt[1] ,minPt[2] });
//  pts.emplace_back(Vector3{ maxPt[0], maxPt[1] ,minPt[2] });
//
//  Renderer::Instance().DrawDebug(currentLayerID, pts, Color4(0.0f, 1.0f, 0.0f, 1.0f), DebugPrimitiveType::Lines);
//}
//
// bool CuboidColliderMesh::Raycast(const Ray & ray, RaycastData_tmp & data)
//{
//	Vector3 he(m_width / 2.f, m_height / 2.f, m_depth / 2.f);
//	Vector3 pos = GetPosition() + GetOffset();
//	Vector3 maxPt(pos + he);
//	Vector3 minPt(pos - he);
//	float t = 0.f;
//	Vector4 plane_l{ 1.f, 0.f, 0.f, minPt[0] }, plane_bo{ 0.f, 1.f, 0.f, minPt[1] }, plane_f{ 0.f, 0.f, 1.f, minPt[2] };
//	auto denom = 0.f;
//	t = 0.f;
//	auto tmax = FLT_MAX, t1 = 0.f, t2 = 0.f;
//	//checking for x axis
//	if (ray.m_dir[0] == 0.f)
//	{
//		if (minPt[0] > ray.m_start[0] || maxPt[0] < ray.m_start[0])
//			return false;
//	}
//	else
//	{
//		denom = 1.f / ray.m_dir[0];
//		t1 = (minPt[0] - ray.m_start[0]) * denom;
//		t2 = (maxPt[0] - ray.m_start[0]) * denom;
//		if (t1 > t2)
//		{
//			auto tmp = t2;
//			t2 = t1;
//			t1 = tmp;
//		}
//		t = t1 > t ? t1 : t;
//		tmax = t2 < tmax ? t2 : tmax;
//		if (t > tmax)
//			return false;
//	}
//	//checking for y axis
//	if (ray.m_dir[1] == 0.f)
//	{
//		if (minPt[1] > ray.m_start[1] || maxPt[1] < ray.m_start[1])
//			return false;
//	}
//	else
//	{
//		denom = 1.f / ray.m_dir[1];
//		t1 = (minPt[1] - ray.m_start[1]) * denom;
//		t2 = (maxPt[1] - ray.m_start[1]) * denom;
//		if (t1 > t2)
//		{
//			auto tmp = t2;
//			t2 = t1;
//			t1 = tmp;
//		}
//		t = t1 > t ? t1 : t;
//		tmax = t2 < tmax ? t2 : tmax;
//		if (t > tmax)
//			return false;
//	}
//	//checking for z axis
//	if (ray.m_dir[2] == 0.f)
//	{
//		if (minPt[2] > ray.m_start[2] || maxPt[2] < ray.m_start[2])
//			return false;
//	}
//	else
//	{
//		denom = 1.f / ray.m_dir[2];
//		t1 = (minPt[2] - ray.m_start[2]) * denom;
//		t2 = (maxPt[2] - ray.m_start[2]) * denom;
//		if (t1 > t2)
//		{
//			auto tmp = t2;
//			t2 = t1;
//			t1 = tmp;
//		}
//		t = t1 > t ? t1 : t;
//		tmax = t2 < tmax ? t2 : tmax;
//		if (t > tmax)
//			return false;
//	}
//
//	data.m_HitFrac = t;
//	data.m_WorldHitPt = ray.m_start + t * ray.m_dir;
//	data.m_WorldNormal = (GetPosition() - ray.m_start).Normalized();
//	data.m_HitObject = m_OwnerObject;
//
//	return true;
//}
//
// bool CuboidColliderMesh::ContainsPoint(const Vector3 & pt)
//{
//	Vector3 he(m_width / 2.f, m_height / 2.f, m_depth / 2.f);
//	Vector3 maxPt(GetPosition() + he);
//	Vector3 minPt(GetPosition() - he);
//
//	return (pt[0] >= minPt[0] && pt[0] <= maxPt[0] &&
//			pt[1] >= minPt[1] && pt[1] <= maxPt[1] &&
//			pt[2] >= minPt[2] && pt[2] <= maxPt[2]);
//}

REFLECT_VIRTUAL(CollisionMesh_3D)
REFLECT_PARENT(IComponent)
// REFLECT_PROPERTY(m_collision_layer)
REFLECT_PROPERTY(m_offset)
// REFLECT_PROPERTY(m_mesh_type)
REFLECT_PROPERTY(m_radius)
REFLECT_END()

// REFLECT_INIT(SphereColliderMesh)
// REFLECT_PARENT(CollisionMesh_3D)
// REFLECT_END()

// REFLECT_INIT(CuboidColliderMesh)
// REFLECT_PARENT(CollisionMesh_3D)
// REFLECT_PROPERTY(m_height)
// REFLECT_PROPERTY(m_width)
// REFLECT_PROPERTY(m_depth)
// REFLECT_END()
