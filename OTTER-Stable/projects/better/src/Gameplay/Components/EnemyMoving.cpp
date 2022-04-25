#include "Gameplay/Components/EnemyMoving.h"
#include <GLFW/glfw3.h>
#include "Gameplay/GameObject.h"
#include "Gameplay/Scene.h"
#include "Gameplay//Components/PlayerMovementBehavior.h"
#include "Gameplay/Components/ComponentManager.h"
#include "Gameplay/Physics/RigidBody.h"
#include "Utils/GlmBulletConversions.h"
#include "Gameplay/GameObject.h"
#include "Gameplay/Scene.h"
#include "Gameplay/InputEngine.h"
#include "Application/Application.h"


EnemyMoving::EnemyMoving(): 
   IComponent(),
_impulse(10.0f)
{
}

EnemyMoving::~EnemyMoving()
{
}

void EnemyMoving::Awake()
{
	
}

void EnemyMoving::Update(float deltaTime)
{
    glm::vec3 worldLocationl = glm::vec3(1, 0, 0);
    glm::vec3 worldLocationr = glm::vec3(-1, 0, 0);

    worldLocationl *= deltaTime;
    GetGameObject()->SetPostion(GetGameObject()->GetPosition() + worldLocationl);


}

void EnemyMoving::OnTriggerVolumeEntered(const std::shared_ptr<Gameplay::Physics::RigidBody>& body)
{
	LOG_INFO("Body has entered our trigger volume: {}", body->GetGameObject()->Name);
	_playerInTrigger = true;
	
		if (body->GetGameObject()->Name == "Player")
		{
			body->SetLinearVelocity(body->GetLinearVelocity() - 10.f);
			body->GetGameObject()->Get<PlayerMovementBehavior>()->Hurt(true);
			
			
		}
		
}

void EnemyMoving::OnTriggerVolumeLeaving(const std::shared_ptr<Gameplay::Physics::RigidBody>& body) {
	LOG_INFO("Body has left our trigger volume: {}", body->GetGameObject()->Name);
	_playerInTrigger = false;
	//check its the player first
	if (body->GetGameObject()->Name == "Player")
	{//default
		body->GetGameObject()->Get<PlayerMovementBehavior>()->Hurt(false);
	}
}

void EnemyMoving::RenderImGui()
{
}

nlohmann::json EnemyMoving::ToJson() const
{
    return nlohmann::json();
}

EnemyMoving::Sptr EnemyMoving::FromJson(const nlohmann::json& blob)
{
    return EnemyMoving::Sptr();
}
