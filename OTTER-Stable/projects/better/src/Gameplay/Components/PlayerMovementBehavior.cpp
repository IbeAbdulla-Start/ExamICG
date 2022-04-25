#include "Gameplay/Components/PlayerMovementBehavior.h"
#include <GLFW/glfw3.h>
#include "Gameplay/GameObject.h"
#include "Gameplay/Scene.h"
#include "Utils/ImGuiHelper.h"
#include "Application/Application.h"
#include "Application/Layers/PostProcessingLayer.h"
#include "Gameplay/InputEngine.h"
#include "AudioEngine.h"



void PlayerMovementBehavior::Awake()
{
	_body = GetComponent<Gameplay::Physics::RigidBody>(); //Gets the Physics Body that it's attached to
	if (_body == nullptr) {
		IsEnabled = false;
	}

	/*_body->SetLinearDamping(0.9f);
	_body->SetAngularDamping(1.f);*/
}

void PlayerMovementBehavior::RenderImGui() {
	LABEL_LEFT(ImGui::DragFloat, "Impulse", &_impulse, 1.0f);
}

nlohmann::json PlayerMovementBehavior::ToJson() const {
	return {
		{ "impulse", _impulse }, {"hurt_state",hurting}, {"moving", is_moving}
	};
}

PlayerMovementBehavior::PlayerMovementBehavior() :
	IComponent(),
	_impulse(0.0f) //Movement Value
{ }

PlayerMovementBehavior::~PlayerMovementBehavior() = default;

PlayerMovementBehavior::Sptr PlayerMovementBehavior::FromJson(const nlohmann::json & blob) {
	PlayerMovementBehavior::Sptr result = std::make_shared<PlayerMovementBehavior>();
	result->_impulse = blob["impulse"];
	result->hurting = blob["hurt_state"];
	result->is_moving = blob["moving"];
	return result;
}

void PlayerMovementBehavior::Hurt(bool state)
{
	hurting = state;
}

void PlayerMovementBehavior::Update(float deltaTime) {

	Application& app = Application::Get();
	//not moving
	//input = false;
	
	if (hurting)
	{
		app.GetLayer<PostProcessingLayer>()->SetLUT(true);

	}
	else
	{
		//_impulse = 0.0f;
		app.GetLayer<PostProcessingLayer>()->SetLUT(false);
	}

	is_moving = false;
	if (glfwGetKey(app.GetWindow(), GLFW_KEY_W) || glfwGetKey(app.GetWindow(), GLFW_KEY_UP)) {
		if (_body->GetLinearVelocity().y >= -5.0f) {
			
			_body->ApplyImpulse(glm::vec3(0.0f, -_impulse, 0.0f));
			is_moving = true;
			_impulse += acceleration;
			if (_impulse > max_speed)
			{
				_impulse = max_speed;
			}
		}
	}

	if (glfwGetKey(app.GetWindow(), GLFW_KEY_S) || glfwGetKey(app.GetWindow(), GLFW_KEY_DOWN)) {
		if (_body->GetLinearVelocity().y <= 5.0f) {
			_body->ApplyImpulse(glm::vec3(0.0f, _impulse, 0.0f));
			
		
			is_moving = true;
			_impulse += acceleration;
			if (_impulse > max_speed)
			{
				_impulse = max_speed;
			}
		}
	}

	if (glfwGetKey(app.GetWindow(), GLFW_KEY_A) || glfwGetKey(app.GetWindow(), GLFW_KEY_LEFT)) {
		if (_body->GetLinearVelocity().x <= 5.0f) {
			_body->ApplyImpulse(glm::vec3(_impulse, 0.0f, 0.0f));

			

			//GetGameObject()->SetRotation(glm::vec3(90.0f, 0.0f, 90.0f));
			//_body->GetGameObject()->SetPostion(_body->GetGameObject()->GetPosition() + glm::vec3(_impulse, 0.0f, 0.0f));
			is_moving = true;
			_impulse += acceleration;
			if (_impulse > max_speed)
			{
				_impulse = max_speed;
			}
		}
	}

	if (glfwGetKey(app.GetWindow(), GLFW_KEY_D) || glfwGetKey(app.GetWindow(), GLFW_KEY_RIGHT)) {
		if (_body->GetLinearVelocity().x >= -5.0f) {
			_body->ApplyImpulse(glm::vec3(-_impulse, 0.0f, 0.0f));
			

			
			is_moving = true;
			_impulse += acceleration;
			if (_impulse > max_speed)
			{
				_impulse = max_speed;
			}
		}
	}
	

	if (!is_moving)
	{
		if (_impulse < 0)
		{
			_impulse += acceleration;
			if (_impulse >= 0)
			{
				_impulse = 0.0f;
			}
		}
		else
		{
			_impulse -= acceleration;
			if (_impulse <= 0)
			{
				_impulse = 0.0f;
			}
		}

	}

	//Rotate when the key is pressed

	//Rotate when the key is pressed
	if (InputEngine::GetKeyState(GLFW_KEY_W) == ButtonState::Down || InputEngine::GetKeyState(GLFW_KEY_UP) == ButtonState::Down) {
		currentRotation = GetGameObject()->GetRotation();
		targetRotation = glm::quat(glm::radians(glm::vec3(90.0f, 0.0f, 180.0f)));
		currentRotation = glm::slerp(currentRotation, targetRotation, turnspeed * deltaTime);
		GetGameObject()->SetRotation(currentRotation);

	}
	if (InputEngine::GetKeyState(GLFW_KEY_A) == ButtonState::Down || InputEngine::GetKeyState(GLFW_KEY_LEFT) == ButtonState::Down) {
		currentRotation = GetGameObject()->GetRotation();
		targetRotation = glm::quat(glm::radians(glm::vec3(90.0f, 0.0f, 270.0f)));
		currentRotation = glm::slerp(currentRotation, targetRotation, turnspeed * deltaTime);
		GetGameObject()->SetRotation(currentRotation);
	}

	if (InputEngine::GetKeyState(GLFW_KEY_S) == ButtonState::Down || InputEngine::GetKeyState(GLFW_KEY_DOWN) == ButtonState::Down) {
		currentRotation = GetGameObject()->GetRotation();
		targetRotation = glm::quat(glm::radians(glm::vec3(90.0f, 0.0f, 0.0f)));
		currentRotation = glm::slerp(currentRotation, targetRotation, turnspeed * deltaTime);
		GetGameObject()->SetRotation(currentRotation);
	}
	if (InputEngine::GetKeyState(GLFW_KEY_D) == ButtonState::Down || InputEngine::GetKeyState(GLFW_KEY_RIGHT) == ButtonState::Down) {
		currentRotation = GetGameObject()->GetRotation();
		targetRotation = glm::quat(glm::radians(glm::vec3(90.0f, 0.0f, 90.0f)));
		currentRotation = glm::slerp(currentRotation, targetRotation, turnspeed * deltaTime);
		GetGameObject()->SetRotation(currentRotation);
	}

	
}

