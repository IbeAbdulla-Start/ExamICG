#include "DefaultSceneLayer.h"

// GLM math library
#include <GLM/glm.hpp>
#include <GLM/gtc/matrix_transform.hpp>
#include <GLM/gtc/type_ptr.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <GLM/gtx/common.hpp> // for fmod (floating modulus)

#include <filesystem>
#include "Application/Timing.h"

// Graphics
#include "Graphics/Buffers/IndexBuffer.h"
#include "Graphics/Buffers/VertexBuffer.h"
#include "Graphics/VertexArrayObject.h"
#include "Graphics/ShaderProgram.h"
#include "Graphics/Textures/Texture2D.h"
#include "Graphics/Textures/TextureCube.h"
#include "Graphics/Textures/Texture2DArray.h"
#include "Graphics/VertexTypes.h"
#include "Graphics/Font.h"
#include "Graphics/GuiBatcher.h"
#include "Graphics/Framebuffer.h"
#include "Graphics/Framebuffer.h"

// Utilities
#include "Utils/MeshBuilder.h"
#include "Utils/MeshFactory.h"
#include "Utils/ObjLoader.h"
#include "Utils/ImGuiHelper.h"
#include "Utils/ResourceManager/ResourceManager.h"
#include "Utils/FileHelpers.h"
#include "Utils/JsonGlmHelpers.h"
#include "Utils/StringUtils.h"
#include "Utils/GlmDefines.h"
#include "ToneFire.h"

// Gameplay
#include "Gameplay/Material.h"
#include "Gameplay/GameObject.h"
#include "Gameplay/Scene.h"

// Components
#include "Gameplay/Components/IComponent.h"
#include "Gameplay/Components/Camera.h"
#include "Gameplay/Components/RotatingBehaviour.h"
#include "Gameplay/Components/JumpBehaviour.h"
#include "Gameplay/Components/RenderComponent.h"
#include "Gameplay/Components/MaterialSwapBehaviour.h"
#include "Gameplay/Components/TriggerVolumeEnterBehaviour.h"
#include "Gameplay/Components/SimpleCameraControl.h"
#include "Gameplay/Components/PlayerMovementBehavior.h"
#include "Gameplay/Components/EnemyMoving.h"
#include "Gameplay/Components/GroundBehaviour.h"
#include "Gameplay/Components/AudioEngine.h"
#include "Gameplay/Components/ShadowCamera.h"
#include "Gameplay/Components/MorphAnimator.h"
#include "Gameplay/Components/MorphMeshRenderer.h"
#include "Gameplay/Components/DeleteObjectBehaviour.h"
#include "Gameplay/Components//Light.h"

//#include "Gameplay/Components/bullet.h"
//#include "Gameplay/Components/EnemyMoving.h"

// Physics
#include "Gameplay/Physics/RigidBody.h"
#include "Gameplay/Physics/Colliders/BoxCollider.h"
#include "Gameplay/Physics/Colliders/PlaneCollider.h"
#include "Gameplay/Physics/Colliders/SphereCollider.h"
#include "Gameplay/Physics/Colliders/ConvexMeshCollider.h"
#include "Gameplay/Physics/Colliders/CylinderCollider.h"
#include "Gameplay/Physics/TriggerVolume.h"
#include "Graphics/DebugDraw.h"

// GUI
#include "Gameplay/Components/GUI/RectTransform.h"
#include "Gameplay/Components/GUI/GuiPanel.h"
#include "Gameplay/Components/GUI/GuiText.h"
#include "Gameplay/InputEngine.h"
#include "Application/Layers/RenderLayer.h"

#include "Application/Application.h"
#include "Gameplay/Components/ParticleSystem.h"
#include "Graphics/Textures/Texture3D.h"
#include "Graphics/Textures/Texture1D.h"

//int ammo;
//float playerX, playerY;
// float BulletX, BulletY, BulletZ;
//bool Shooting;

DefaultSceneLayer::DefaultSceneLayer() :
	ApplicationLayer()
{
	Name = "Default Scene";
	Overrides = AppLayerFunctions::OnAppLoad;
}

DefaultSceneLayer::~DefaultSceneLayer() = default;

void DefaultSceneLayer::OnAppLoad(const nlohmann::json& config) {
	_CreateScene();
}

void DefaultSceneLayer::OnUpdate() {
	Application& app = Application::Get();
	_currentScene = app.CurrentScene();

	if (!activated) {
		specBox = _currentScene->FindObjectByName("Player");
		
		activated = true;
	}
	//Bullet::Sptr bullettest;
	//.playerX = specBox->GetPosition().x;
	///playerY = specBox->GetPosition().y;
}

void DefaultSceneLayer::_CreateScene()
{
	using namespace Gameplay;
	using namespace Gameplay::Physics;

	Application& app = Application::Get();

	bool loadScene = false;
	// For now we can use a toggle to generate our scene vs load from file
	if (loadScene && std::filesystem::exists("scene.json")) {
		app.LoadScene("scene.json");
	} else {
		// This time we'll have 2 different shaders, and share data between both of them using the UBO
		// This shader will handle reflective materials 

				// This shader handles our basic materials without reflections (cause they expensive)
		ShaderProgram::Sptr rackShader = ResourceManager::CreateAsset<ShaderProgram>(std::unordered_map<ShaderPartType, std::string>{
			{ ShaderPartType::Vertex, "shaders/vertex_shaders/basic.glsl" },
			{ ShaderPartType::Fragment, "shaders/fragment_shaders/deferred_forwardRACK.glsl" }
		});

		// ANIMATION SHADER??
		ShaderProgram::Sptr animShader = ResourceManager::CreateAsset<ShaderProgram>(std::unordered_map<ShaderPartType, std::string>{
			{ ShaderPartType::Vertex, "shaders/vertex_shaders/morph.vert" },
			{ ShaderPartType::Fragment, "shaders/fragment_shaders/deferred_forward.glsl" }
		});


		// This shader handles our foliage vertex shader example
		ShaderProgram::Sptr foliageShader = ResourceManager::CreateAsset<ShaderProgram>(std::unordered_map<ShaderPartType, std::string>{
			{ ShaderPartType::Vertex, "shaders/vertex_shaders/foliage.glsl" },
			{ ShaderPartType::Fragment, "shaders/fragment_shaders/deferred_forward.glsl" }
		});


		///////////////////// NEW SHADERS ////////////////////////////////////////////

			// This shader handles our displacement mapping example
		ShaderProgram::Sptr displacementShader = ResourceManager::CreateAsset<ShaderProgram>(std::unordered_map<ShaderPartType, std::string>{
			{ ShaderPartType::Vertex, "shaders/vertex_shaders/displacement_mapping.glsl" },
			{ ShaderPartType::Fragment, "shaders/fragment_shaders/deferred_forward.glsl" }
		});
		displacementShader->SetDebugName("Displacement Mapping");

		// This shader handles our cel shading example
		ShaderProgram::Sptr celShader = ResourceManager::CreateAsset<ShaderProgram>(std::unordered_map<ShaderPartType, std::string>{
			{ ShaderPartType::Vertex, "shaders/vertex_shaders/displacement_mapping.glsl" },
			{ ShaderPartType::Fragment, "shaders/fragment_shaders/cel_shader.glsl" }
		});
		celShader->SetDebugName("Cel Shader");

		// Basic gbuffer generation with no vertex manipulation
		ShaderProgram::Sptr deferredForward = ResourceManager::CreateAsset<ShaderProgram>(std::unordered_map<ShaderPartType, std::string>{
			{ ShaderPartType::Vertex, "shaders/vertex_shaders/basic.glsl" },
			{ ShaderPartType::Fragment, "shaders/fragment_shaders/deferred_forward.glsl" }
		});
		deferredForward->SetDebugName("Deferred - GBuffer Generation");

#pragma region Basic Texture Creation
		Texture2DDescription singlePixelDescriptor;
		singlePixelDescriptor.Width = singlePixelDescriptor.Height = 1;
		singlePixelDescriptor.Format = InternalFormat::RGB8;

		float normalMapDefaultData[3] = { 0.5f, 0.5f, 1.0f };
		Texture2D::Sptr normalMapDefault = ResourceManager::CreateAsset<Texture2D>(singlePixelDescriptor);
		normalMapDefault->LoadData(1, 1, PixelFormat::RGB, PixelType::Float, normalMapDefaultData);

		float solidBlack[3] = { 0.5f, 0.5f, 0.5f };
		Texture2D::Sptr solidBlackTex = ResourceManager::CreateAsset<Texture2D>(singlePixelDescriptor);
		solidBlackTex->LoadData(1, 1, PixelFormat::RGB, PixelType::Float, solidBlack);

		float solidGrey[3] = { 0.0f, 0.0f, 0.0f };
		Texture2D::Sptr solidGreyTex = ResourceManager::CreateAsset<Texture2D>(singlePixelDescriptor);
		solidGreyTex->LoadData(1, 1, PixelFormat::RGB, PixelType::Float, solidGrey);

		float solidWhite[3] = { 1.0f, 1.0f, 1.0f };
		Texture2D::Sptr solidWhiteTex = ResourceManager::CreateAsset<Texture2D>(singlePixelDescriptor);
		solidWhiteTex->LoadData(1, 1, PixelFormat::RGB, PixelType::Float, solidWhite);




		// This shader handles our multitexturing example

		// Load in the meshes
		MeshResource::Sptr monkeyMesh = ResourceManager::CreateAsset<MeshResource>("Monkey.obj");

		// Load in some textures
		Texture2D::Sptr    boxTexture   = ResourceManager::CreateAsset<Texture2D>("textures/box-diffuse.png");
		Texture2D::Sptr    boxSpec      = ResourceManager::CreateAsset<Texture2D>("textures/box-specular.png");
		Texture2D::Sptr    monkeyTex    = ResourceManager::CreateAsset<Texture2D>("textures/monkey-uvMap.png");
		Texture2D::Sptr    leafTex      = ResourceManager::CreateAsset<Texture2D>("textures/leaves.png");
		leafTex->SetMinFilter(MinFilter::Nearest);
		leafTex->SetMagFilter(MagFilter::Nearest);

		Texture2DArray::Sptr particleTex = ResourceManager::CreateAsset<Texture2DArray>("textures/particlesRR.png", 2, 2);
		// Loading in a 1D LUT
		Texture1D::Sptr toonLut = ResourceManager::CreateAsset<Texture1D>("luts/toon-1D.png"); 
		toonLut->SetWrap(WrapMode::ClampToEdge);

		// Here we'll load in the cubemap, as well as a special shader to handle drawing the skybox
		TextureCube::Sptr testCubemap = ResourceManager::CreateAsset<TextureCube>("cubemaps/ocean/ocean.jpg");
		ShaderProgram::Sptr      skyboxShader = ResourceManager::CreateAsset<ShaderProgram>(std::unordered_map<ShaderPartType, std::string>{
			{ ShaderPartType::Vertex, "shaders/vertex_shaders/skybox_vert.glsl" },
			{ ShaderPartType::Fragment, "shaders/fragment_shaders/skybox_frag.glsl" }
		});

		// Create an empty scene
		Scene::Sptr scene = std::make_shared<Scene>(); 

		// Setting up our enviroment map
		scene->SetSkyboxTexture(testCubemap); 
		scene->SetSkyboxShader(skyboxShader);
		// Since the skybox I used was for Y-up, we need to rotate it 90 deg around the X-axis to convert it to z-up 
		scene->SetSkyboxRotation(glm::rotate(MAT4_IDENTITY, glm::half_pi<float>(), glm::vec3(1.0f, 0.0f, 0.0f)));

		// Loading in a color lookup table
		//Texture3D::Sptr lut = ResourceManager::CreateAsset<Texture3D>("luts/Screenshot_6.CUBE");  

		// Configure the color correction LUT
		//scene->SetColorLUT(lut);

		// Create our materials
	
		// Our toon shader material

		Texture2D::Sptr planeTex = ResourceManager::CreateAsset<Texture2D>("textures/box-diffuse.png");


		Material::Sptr boxMaterial = ResourceManager::CreateAsset<Material>(deferredForward); {
			boxMaterial->Name = "Plane";
			boxMaterial->Set("u_Material.AlbedoMap", planeTex);
			boxMaterial->Set("u_Material.Shininess", 1.0f);
			boxMaterial->Set("u_Material.NormalMap", normalMapDefault);
		}

		Material::Sptr monkeyMaterial = ResourceManager::CreateAsset<Material>(deferredForward);
		{
			monkeyMaterial->Name = "Monkey";
			monkeyMaterial->Set("u_Material.Diffuse", monkeyTex);
			monkeyMaterial->Set("u_Material.Shininess", 0.5f);
		}



		Gameplay::GameObject::Sptr lightParent = scene->CreateGameObject("Lights");
		//light collection
		//set this up for every light
		{
			{
				Gameplay::GameObject::Sptr light = scene->CreateGameObject("Light 1");
				light->SetPostion(glm::vec3(-19.98f, 12.350f, 3.0f));
				lightParent->AddChild(light);

				Light::Sptr lightComponent = light->Add<Light>();
				lightComponent->SetRadius(159.600f);
				lightComponent->SetIntensity(237.f);
				lightComponent->SetColor(glm::vec3(0.326f, 0.326f, 0.326f));

			}
		}

		

		//scene->Lights[3].Position = bullet->GetPosition();

		// We'll create a mesh that is a simple plane that we can resize later
		MeshResource::Sptr planeMesh = ResourceManager::CreateAsset<MeshResource>();
		planeMesh->AddParam(MeshBuilderParam::CreatePlane(ZERO, UNIT_Z, UNIT_X, glm::vec2(1.0f)));
		planeMesh->GenerateMesh();

		MeshResource::Sptr sphere = ResourceManager::CreateAsset<MeshResource>();
		sphere->AddParam(MeshBuilderParam::CreateIcoSphere(ZERO, ONE, 5));
		sphere->GenerateMesh();

		// Set up the scene's camera
		GameObject::Sptr camera = scene->MainCamera->GetGameObject()->SelfRef();
		{
			camera->SetPostion({ 3.689, 55.106, 8.623 });
			camera->SetRotation ({0,0,-180 });
			camera->LookAt(glm::vec3(0.0f));


			// This is now handled by scene itself!
			//Camera::Sptr cam = camera->Add<Camera>();
			// Make sure that the camera is set as the scene's main camera!
			//scene->MainCamera = cam;
		}

		Gameplay::GameObject::Sptr shadowCaster = scene->CreateGameObject("Shadow Light");
		{
			// Set position in the scene
			shadowCaster->SetPostion(glm::vec3(-8.390f,32.05f, 30.820f));
			//shadowCaster->LookAt(glm::vec3(0.0f));
			shadowCaster->SetRotation(glm::vec3(3.0f, 40.0f, 161.0f));

			// Create and attach a renderer for the monkey
			ShadowCamera::Sptr shadowCam = shadowCaster->Add<ShadowCamera>();
			shadowCam->SetProjection(glm::perspective(glm::radians(120.0f), 1.0f, 0.1f, 100.0f));
			//shadowCam->SetProjection(glm::ortho(15.0f, 30.0f, 30.0f, 15.0f, 0.225f, 22555.f));
		}

		// Set up all our sample objects
		GameObject::Sptr plane = scene->CreateGameObject("Plane");
		{
			// Make a big tiled mesh
			MeshResource::Sptr tiledMesh = ResourceManager::CreateAsset<MeshResource>();
			tiledMesh->AddParam(MeshBuilderParam::CreatePlane(ZERO, UNIT_Z, UNIT_X, glm::vec2(100.0f), glm::vec2(20.0f)));
			tiledMesh->GenerateMesh();

			// Create and attach a RenderComponent to the object to draw our mesh
			RenderComponent::Sptr renderer = plane->Add<RenderComponent>();
			renderer->SetMesh(tiledMesh);
			renderer->SetMaterial(boxMaterial);
			GroundBehaviour::Sptr ground = plane->Add<GroundBehaviour>();
			// Attach a plane collider that extends infinitely along the X/Y axis
			RigidBody::Sptr physics = plane->Add<RigidBody>(/*static by default*/);
			physics->AddCollider(BoxCollider::Create(glm::vec3(50.0f, 50.0f, 1.0f)))->SetPosition({ 0,0,-0.5 });
		}


		GameObject::Sptr demoBase = scene->CreateGameObject("Demo Parent");

		GameObject::Sptr specBox = scene->CreateGameObject("Player");
		{
			MeshResource::Sptr boxMesh = ResourceManager::CreateAsset<MeshResource>();
			boxMesh->AddParam(MeshBuilderParam::CreateCube(ZERO, ONE));
			boxMesh->GenerateMesh();

			// Set and rotation position in the scene
			specBox->SetPostion(glm::vec3(6.212, 45.600f, 1.5f));
			specBox->SetRotation(glm::vec3(90.f, 0.f, 0.f));
			specBox->SetScale(glm::vec3(1.5f, 1.5f, 1.0f));

			// Add a render component
			RenderComponent::Sptr renderer = specBox->Add<RenderComponent>();
			renderer->SetMesh(boxMesh);
			renderer->SetMaterial(boxMaterial);
			//Movement::Sptr movement = specBox->Add<Movement>();
			PlayerMovementBehavior::Sptr moveement = specBox->Add<PlayerMovementBehavior>();
			JumpBehaviour::Sptr Jump = specBox->Add<JumpBehaviour>();
			Gameplay::Physics::RigidBody::Sptr physics = specBox->Add<Gameplay::Physics::RigidBody>(RigidBodyType::Dynamic);
			Gameplay::Physics::BoxCollider::Sptr box = Gameplay::Physics::BoxCollider::Create();

			//box->SetExtents(glm::vec3(0.8, 2.68, 0.83));
			//physics->AddCollider(box);
			physics->AddCollider(BoxCollider::Create());
			//physics->SetMass(10.0f);
			//add trigger for collisions and behaviours
			Gameplay::Physics::TriggerVolume::Sptr volume = specBox->Add<Gameplay::Physics::TriggerVolume>();
			volume->AddCollider(BoxCollider::Create());

			Gameplay::GameObject::Sptr particles = scene->CreateGameObject("Particles");
			specBox->AddChild(particles);
			particles->SetPostion({ 0.0f, 0.0f, 0.24f });

			ParticleSystem::Sptr particleManager = particles->Add<ParticleSystem>();
			particleManager->Atlas = particleTex;

			particleManager->_gravity = glm::vec3(0.0f);

			ParticleSystem::ParticleData emitter;
			emitter.Type = ParticleType::SphereEmitter;
			emitter.TexID = 2;
			emitter.Position = glm::vec3(0.0f);
			emitter.Color = glm::vec4(0.966f, 0.878f, 0.767f, 1.0f);
			emitter.Lifetime = 1.0f / 50.0f;
			emitter.SphereEmitterData.Timer = 1.0f / 10.0f;
			emitter.SphereEmitterData.Velocity = 0.5f;
			emitter.SphereEmitterData.LifeRange = { 1.0f, 1.5f };
			emitter.SphereEmitterData.Radius = 0.5f;
			emitter.SphereEmitterData.SizeRange = { 0.20f, 0.55f };


			particleManager->AddEmitter(emitter);


			demoBase->AddChild(specBox);
		}

		GameObject::Sptr bullet = scene->CreateGameObject("Bullet");
		{

			// Set and rotation position in the scene
			bullet->SetPostion(glm::vec3(-0.490, -17.0f, 1.5f));
			bullet->SetRotation(glm::vec3(0.f, 0.f, 0.f));
			bullet->SetScale(glm::vec3(0.5f, 0.5f, 0.5f));

			// Add a render component
			RenderComponent::Sptr renderer = bullet->Add<RenderComponent>();
			renderer->SetMesh(monkeyMesh);
			

			//Bullet::Sptr testbullet = bullet->Add<Bullet>();

			

		}

		GameObject::Sptr enemy = scene->CreateGameObject("Enemy");
		{
			enemy->SetPostion(glm::vec3(-6, 45.600, 0.5));
			enemy->SetRotation(glm::vec3(15, -14, 177));
			RenderComponent::Sptr renderer = enemy->Add<RenderComponent>();
			renderer->SetMesh(monkeyMesh);
			renderer->SetMaterial(monkeyMaterial);
			Gameplay::Physics::RigidBody::Sptr physics = enemy->Add<Gameplay::Physics::RigidBody>(RigidBodyType::Dynamic);
			Gameplay::Physics::BoxCollider::Sptr box = Gameplay::Physics::BoxCollider::Create();
			physics->AddCollider(BoxCollider::Create());
			//physics->SetMass(10.0f);
			//add trigger for collisions and behaviours
			Gameplay::Physics::TriggerVolume::Sptr volume = enemy->Add<Gameplay::Physics::TriggerVolume>();
			volume->AddCollider(BoxCollider::Create());
			EnemyMoving::Sptr enemymov = enemy->Add<EnemyMoving>();
		}

		Gameplay::GameObject::Sptr layoutwall1 = scene->CreateGameObject("Layout Wall Front");
		{
			layoutwall1->SetPostion(glm::vec3(1.00f, 40.92f, 0.0f));
			layoutwall1->SetScale(glm::vec3(20.5f, 0.34f, 3.0f));
			Gameplay::Physics::RigidBody::Sptr wall1Phys = layoutwall1->Add<Gameplay::Physics::RigidBody>(RigidBodyType::Static);
			Gameplay::Physics::BoxCollider::Sptr wall1 = Gameplay::Physics::BoxCollider::Create();
			//wall1->SetPosition(glm::vec3(11.85f, 3.23f, 1.05f));
			wall1->SetPosition(glm::vec3(-2.90f, 0.570f, 3.0f));
			wall1->SetScale(glm::vec3(32.240f, -2.480f, 3.0f));
			wall1->SetExtents(glm::vec3(1.0f, 1.0f, 1.0f));
			wall1Phys->AddCollider(wall1);
		}

		Gameplay::GameObject::Sptr layoutwall2 = scene->CreateGameObject("Layout Wall Bottom");
		{
			layoutwall2->SetPostion(glm::vec3(1.00f, 40.92f, 0.0f));
			layoutwall2->SetScale(glm::vec3(20.5f, 0.34f, 3.0f));
			Gameplay::Physics::RigidBody::Sptr wall2Phys = layoutwall2->Add<Gameplay::Physics::RigidBody>(RigidBodyType::Static);
			Gameplay::Physics::BoxCollider::Sptr wall2 = Gameplay::Physics::BoxCollider::Create();
			//wall1->SetPosition(glm::vec3(11.85f, 3.23f, 1.05f));
			wall2->SetPosition(glm::vec3(-2.90f, 6.60f, 3.0f));
			wall2->SetScale(glm::vec3(32.240f, -2.480f, 3.0f));
			wall2->SetExtents(glm::vec3(1.0f, 1.0f, 1.0f));
			wall2Phys->AddCollider(wall2);
		}


		// Create a trigger volume for testing how we can detect collisions with objects!
		GameObject::Sptr trigger = scene->CreateGameObject("Trigger");
		{
			TriggerVolume::Sptr volume = trigger->Add<TriggerVolume>();
			CylinderCollider::Sptr collider = CylinderCollider::Create(glm::vec3(3.0f, 3.0f, 1.0f));
			collider->SetPosition(glm::vec3(0.0f, 0.0f, 0.5f));
			volume->AddCollider(collider);

			trigger->Add<TriggerVolumeEnterBehaviour>();
		}

		/////////////////////////// UI //////////////////////////////
		/*
		GameObject::Sptr canvas = scene->CreateGameObject("UI Canvas");
		{
			RectTransform::Sptr transform = canvas->Add<RectTransform>();
			transform->SetMin({ 16, 16 });
			transform->SetMax({ 256, 256 });

			GuiPanel::Sptr canPanel = canvas->Add<GuiPanel>();


			GameObject::Sptr subPanel = scene->CreateGameObject("Sub Item");
			{
				RectTransform::Sptr transform = subPanel->Add<RectTransform>();
				transform->SetMin({ 10, 10 });
				transform->SetMax({ 128, 128 });

				GuiPanel::Sptr panel = subPanel->Add<GuiPanel>();
				panel->SetColor(glm::vec4(1.0f, 0.0f, 0.0f, 1.0f));

				panel->SetTexture(ResourceManager::CreateAsset<Texture2D>("textures/upArrow.png"));

				Font::Sptr font = ResourceManager::CreateAsset<Font>("fonts/Roboto-Medium.ttf", 16.0f);
				font->Bake();

				GuiText::Sptr text = subPanel->Add<GuiText>();
				text->SetText("Hello world!");
				text->SetFont(font);

				monkey1->Get<JumpBehaviour>()->Panel = text;
			}

			canvas->AddChild(subPanel);
		}
		*/

		// Save the asset manifest for all the resources we just loaded
		ResourceManager::SaveManifest("scene-manifest.json");
		// Save the scene to a JSON file
		scene->Save("scene.json");

		// Send the scene to the application
		app.LoadScene(scene);
	}
}
