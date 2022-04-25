#pragma once
#include "Gameplay/Physics/RigidBody.h"
#include "Gameplay/Components/IComponent.h"
#include "Application/Timing.h"
class EnemyMoving : public Gameplay::IComponent
{
public:
	typedef std::shared_ptr<EnemyMoving> Sptr;
	EnemyMoving();
	virtual ~EnemyMoving();

	virtual void Awake() override;
	virtual void Update(float deltaTime) override;
	virtual void OnTriggerVolumeEntered(const std::shared_ptr<Gameplay::Physics::RigidBody>& body) override;
	virtual void OnTriggerVolumeLeaving(const std::shared_ptr<Gameplay::Physics::RigidBody>& body) override;


public:
	virtual void RenderImGui() override;


	MAKE_TYPENAME(EnemyMoving);
	virtual nlohmann::json ToJson() const override;
	static EnemyMoving::Sptr FromJson(const nlohmann::json& blob);

protected:
	float _impulse;
	bool _playerInTrigger;
	float ResetHitTimer = Timing::Current().TimeSinceAppLoad();
	Gameplay::Scene* _scene;
};

