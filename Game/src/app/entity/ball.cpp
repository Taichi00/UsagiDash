#include "ball.h"
#include "game/component/collider/sphere_collider.h"
#include "game/component/sphere_mesh.h"
#include "game/component/mesh_renderer.h"
#include "game/component/rigidbody.h"
#include "app/component/pause_behavior.h"
#include "app/component/entity_height_destroyer.h"

Ball::Ball(const float radius, const float mass) : Entity("ball", "object", "object")
{
	std::shared_ptr model = SphereMesh::Load(radius, 0.8f, 0.2f, 0);

	AddComponent(new MeshRenderer(model));
	AddComponent(new SphereCollider(radius));
	AddComponent(new Rigidbody(mass, true, false, 0.5f));
	AddComponent(new PauseBehavior());
	AddComponent(new EntityHeightDestroyer(-40));
}
