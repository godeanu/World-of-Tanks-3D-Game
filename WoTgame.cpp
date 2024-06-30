#include "lab_m1/WoTgame/WoTgame.h"

#include <vector>
#include <string>
#include <iostream>
#include <stdlib.h>  

#include "lab_m1/WoTgame/transform3D.h"

using namespace std;
using namespace m1;

const float WORLD_SIZE = 23.0f;  
float gameTime = 0.0f;
bool gameActive = true;
int score = 0;
bool messagePrinted = false;

float triggerDistance = 5.0f;
struct Bullet {
    glm::vec3 position;
    glm::vec3 direction;
    float speed;
    float lifetime;
};

std::vector<Bullet> bullets;


struct EnemyTank {
    glm::vec3 enemy_position;
    float angle;
    float movementTimer;
    float rotationTimer;
    float targetRotation;
    bool isRotating;
    bool isMovingForward;
    float turretAngle;
    float gunAngle;
    float shootTimer;
    float health;
    float speed;
    float damageLevel=0;
    bool isActive;

};

struct Building {
	glm::vec3 position;
	
};

float turretAngle = 0.0f;
float tunAngle = 0.0f;


std::vector<Building> buildings;

std::vector<glm::vec3> occupiedPositions;


glm::vec3 tankPosition = glm::vec3(0, 0, 0);
float tankAngle = 0.0f;
float tankSpeed = 2.5f;


std::vector<EnemyTank> enemyTanks;
float shootCooldownTimer = 0.0f;
glm::mat4 projectionMatrixMinimap = glm::ortho(-10.f, 10.f, -8.f, 8.f, 0.01f, 50.0f); 




WoTgame::WoTgame()
{
}


WoTgame::~WoTgame()
{
}

bool AreAllTanksInactive(const std::vector<EnemyTank>& tanks) {
    for (const auto& tank : tanks) {
        if (tank.isActive) {
            return false;
        }
    }
    return true;
}


bool IsPlayerCollidingWithBuilding(const glm::vec3& playerPosition, const Building& building) {
    float tankRadius = 2.0f; 

    glm::vec3 buildingMin = building.position - glm::vec3(2.6f, 0, 1.6f);
    glm::vec3 buildingMax = building.position + glm::vec3(2.6f, 0, 1.6f);

    glm::vec3 closestPoint = glm::clamp(playerPosition, buildingMin, buildingMax);

    float distance = glm::length(playerPosition - closestPoint);
    return distance < tankRadius;
}

void ResolvePlayerCollisionWithBuilding(glm::vec3& playerPosition, const Building& building) {
    float tankRadius = 2.0f;

    glm::vec3 buildingMin = building.position - glm::vec3(2.6f, 0, 1.6f);
    glm::vec3 buildingMax = building.position + glm::vec3(2.6f, 0, 1.6f);

    glm::vec3 closestPoint = glm::clamp(playerPosition, buildingMin, buildingMax);

    float distance = glm::length(playerPosition - closestPoint);
    float overlap = tankRadius - distance;

    if (overlap > 0) {
        glm::vec3 displacement = glm::normalize(playerPosition - closestPoint) * overlap;
        playerPosition += displacement;
    }
}



bool IsCollidingWithBuilding(const EnemyTank& tank, const Building& building) {
    glm::vec3 tankPos = tank.enemy_position;
    float tankRadius = 2.0f; 

    glm::vec3 buildingMin = building.position - glm::vec3(2.6f, 0, 1.6f); 
    glm::vec3 buildingMax = building.position + glm::vec3(2.6f, 0, 1.6f);

  
    glm::vec3 closestPoint = glm::clamp(tankPos, buildingMin, buildingMax);

    float distance = glm::length(tankPos - closestPoint);
    return distance < tankRadius;
}

void ResolveCollisionWithBuilding(EnemyTank& tank, const Building& building) {
    glm::vec3 tankPos = tank.enemy_position;
    float tankRadius = 2.0f;

    glm::vec3 buildingMin = building.position - glm::vec3(2.6f, 0, 1.6f);
    glm::vec3 buildingMax = building.position + glm::vec3(2.6f, 0, 1.6f);

    glm::vec3 closestPoint = glm::clamp(tankPos, buildingMin, buildingMax);

    float distance = glm::length(tankPos - closestPoint);
    float overlap = tankRadius - distance;

    if (overlap > 0) {
        glm::vec3 displacement = glm::normalize(tankPos - closestPoint) * overlap;
        tank.enemy_position += displacement;
    }
}


bool IsCollidingWithPlayerTank(const EnemyTank& enemyTank, const glm::vec3& playerTankPosition) {
    float tankRadius = 2.0f; 
    glm::vec3 diff = enemyTank.enemy_position - playerTankPosition;
    float distance = glm::length(diff);
    return distance < (tankRadius * 2);
}

void ResolveCollisionWithPlayerTank(EnemyTank& enemyTank, const glm::vec3& playerTankPosition) {
    float tankRadius = 2.0f;
    glm::vec3 diff = enemyTank.enemy_position - playerTankPosition;
    float distance = glm::length(diff);
    float overlap = (tankRadius * 2) - distance;

    if (overlap > 0) {
        glm::vec3 displacement = glm::normalize(diff) * overlap;
        enemyTank.enemy_position += displacement;
    }
}



bool AreTanksColliding(const EnemyTank& tank1, const EnemyTank& tank2) {
    float tankRadius = 2.0f; 
    glm::vec3 diff = tank2.enemy_position - tank1.enemy_position;
    float distance = glm::length(diff);
    return distance < (tankRadius * 2);
}

void ResolveTankCollision(EnemyTank& tank1, EnemyTank& tank2) {
    float tankRadius = 2.0f;
    glm::vec3 diff = tank2.enemy_position - tank1.enemy_position;
    float distance = glm::length(diff);
    float overlap = (tankRadius * 2) - distance;

    if (overlap > 0) {
        glm::vec3 displacement = glm::normalize(diff) * overlap;
        tank1.enemy_position -= displacement * 0.5f;
        tank2.enemy_position += displacement * 0.5f;
    }
}



bool IsEnemyClose(const glm::vec3& enemyPosition, const glm::vec3& playerPosition, float triggerDistance) {
    return glm::distance(enemyPosition, playerPosition) <= triggerDistance;
}

float CalculateTargetAngle(const glm::vec3& fromPosition, const glm::vec3& toPosition) {


	float angle = atan2(toPosition.z - fromPosition.z, toPosition.x - fromPosition.x);
	return angle;
}


bool IsOutOfBounds(const glm::vec3& position, float boundary) {
    return position.x > boundary || position.x < -boundary ||
        position.z > boundary || position.z < -boundary;
}

glm::vec3 GenerateRandomPosition(float worldSize) {
    float x = static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / worldSize)) - worldSize / 2;
    float z = static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / worldSize)) - worldSize / 2;
    return glm::vec3(x, 0, z); 
}

bool IsOverlapping(const glm::vec3& position, const std::vector<glm::vec3>& existingPositions, float minDistance) {
    for (const auto& existingPos : existingPositions) {
        if (glm::distance(position, existingPos) < minDistance) {
            return true;
        }
    }

if (glm::distance(position, glm::vec3(0, 0, 0)) < minDistance) {
		return true;
	}
    return false;
}

float CalculateAngleTowards(glm::vec3 from, glm::vec3 to) {
    glm::vec3 direction = glm::normalize(to - from);
    return atan2(direction.z, direction.x);
}


bool IsCollidingWithTank(const Bullet& bullet, const EnemyTank& tank) {
    float combinedRadius = 0.25f + 2.0f; 
    return glm::distance(bullet.position, tank.enemy_position) < combinedRadius;
}

bool IsCollidingWithBuilding(const Bullet& bullet, const Building& building) {
   

    glm::vec3 minCorner = building.position - 5 * 0.5f;
    glm::vec3 maxCorner = building.position + 5 * 0.5f;

    return ( bullet.position.x >= minCorner.x && bullet.position.x <= maxCorner.x &&
        			 bullet.position.y >= minCorner.y && bullet.position.y <= maxCorner.y &&
        			 bullet.position.z >= minCorner.z && bullet.position.z <= maxCorner.z );
}

float RandomFloat(float min, float max) {
    float random = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
    float range = max - min;
    return min + random * range;
}

void RotateTowardsTarget(float& currentAngle, float targetAngle, float rotationSpeed, float deltaTime) {
    float angleDiff = targetAngle - currentAngle;
  
    while (angleDiff > M_PI) angleDiff -= 2 * M_PI;
    while (angleDiff < -M_PI) angleDiff += 2 * M_PI;

    float rotationAmount = rotationSpeed * deltaTime;
    if (std::abs(angleDiff) < rotationAmount) {
        currentAngle = targetAngle;
    }
    else {
        currentAngle += (angleDiff > 0 ? 1 : -1) * rotationAmount;
    }
}


void WoTgame::Init()
{
    polygonMode = GL_FILL;

    const string sourceTextureDir = PATH_JOIN(window->props.selfDir, SOURCE_PATH::M1, "WoTgame", "textures");

    
    {
        Texture2D* texture = new Texture2D();
        texture->Load2D(PATH_JOIN(sourceTextureDir, "wildgrass.png").c_str(), GL_REPEAT);
        mapTextures["grass"] = texture;
    }


    {
		Texture2D* texture = new Texture2D();
        texture->Load2D(PATH_JOIN(sourceTextureDir, "gray.jpg").c_str(), GL_CLAMP_TO_EDGE);
		mapTextures["block"] = texture;
    }


    camera = new implemented::Camera();
    minimapCamera = new implemented::Camera();

  
    camera->Set(glm::vec3(0, 2, 2), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
    minimapCamera->Set(glm::vec3(0, 5, 1), glm::vec3(0, 1, 0), glm::vec3(0, 1, 0));


    Mesh* senile = new Mesh("senile");
    senile->LoadMesh(PATH_JOIN(window->props.selfDir, RESOURCE_PATH::MODELS, "primitives"), "senile.obj");
    meshes[senile->GetMeshID()] = senile;

    Mesh* turret = new Mesh("turret");
    turret->LoadMesh(PATH_JOIN(window->props.selfDir, RESOURCE_PATH::MODELS, "primitives"), "turret.obj");
    meshes[turret->GetMeshID()] = turret;

    Mesh* tun = new Mesh("tun");
    tun->LoadMesh(PATH_JOIN(window->props.selfDir, RESOURCE_PATH::MODELS, "primitives"), "barrel.obj");
    meshes[tun->GetMeshID()] = tun;

    Mesh *body = new Mesh("body");
    body->LoadMesh(PATH_JOIN(window->props.selfDir, RESOURCE_PATH::MODELS, "primitives"), "body.obj");
    meshes[body->GetMeshID()] = body;

    Mesh *block = new Mesh("block");
    block->LoadMesh(PATH_JOIN(window->props.selfDir, RESOURCE_PATH::MODELS, "primitives"), "blockbuilding.obj");
    meshes[block->GetMeshID()] = block;

    Mesh *sphere = new Mesh("sphere");
    sphere->LoadMesh(PATH_JOIN(window->props.selfDir, RESOURCE_PATH::MODELS, "primitives"), "sphere.obj");
    meshes[sphere->GetMeshID()] = sphere;

    Mesh* mesh = new Mesh("plane");
    mesh->LoadMesh(PATH_JOIN(window->props.selfDir, RESOURCE_PATH::MODELS, "primitives"), "plane50.obj");
    meshes[mesh->GetMeshID()] = mesh;

    
    {
        Shader* shader = new Shader("LabShader");
        shader->AddShader(PATH_JOIN(window->props.selfDir, SOURCE_PATH::M1, "WoTgame", "shaders", "VertexShader.glsl"), GL_VERTEX_SHADER);
        shader->AddShader(PATH_JOIN(window->props.selfDir, SOURCE_PATH::M1, "WoTgame", "shaders", "FragmentShader.glsl"), GL_FRAGMENT_SHADER);
        shader->CreateAndLink();
        shaders[shader->GetName()] = shader;
    }

    projectionMatrix = glm::perspective(RADIANS(60), window->props.aspectRatio, 0.01f, 200.0f);

    glm::ivec2 resolution = window->GetResolution();
    miniViewportArea = ViewportArea(50, 50, resolution.x / 5.f, resolution.y / 5.f);


    int numberOfTanks = 5;
    int numberOfBuildings = 5;
	
    for (int i = 0; i < numberOfTanks; ++i) {
        glm::vec3 newPos;
        do {
            newPos = GenerateRandomPosition(50);
        } while (IsOverlapping(newPos, occupiedPositions, 5.0f)); 

        EnemyTank newTank;
        newTank.enemy_position = newPos;
        newTank.angle = static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / 360));
        newTank.movementTimer = RandomFloat(1.0f, 3.0f);
        newTank.rotationTimer = 0;
        newTank.targetRotation = 0;
        newTank.isRotating = false;
        newTank.isMovingForward = rand() % 2 == 0; 
        newTank.health = 3;
        newTank.isActive = true;


        enemyTanks.push_back(newTank);
    }

    for (int i = 0; i < numberOfBuildings; ++i) {
        glm::vec3 newPos;
        do {
            newPos = GenerateRandomPosition(40);
        } while (IsOverlapping(newPos, occupiedPositions, 7.0f)); 

     
        Building newBuilding;
        newBuilding.position = newPos;
       
        buildings.push_back(newBuilding);
        occupiedPositions.push_back(newPos);
    }
    
    mixTextures = false;
}

void WoTgame::FrameStart()
{
   
    glClearColor(0, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glm::ivec2 resolution = window->GetResolution();
   
    glViewport(0, 0, resolution.x, resolution.y);
}



void WoTgame::RenderScene(implemented::Camera* cameraTo, glm::mat4 viewMatrix, glm::mat4 proj) {
   

    glm::mat4 modelMatrix = glm::mat4(1);
    modelMatrix = glm::translate(modelMatrix, glm::vec3(0, 0.1f, 0));
    modelMatrix = glm::scale(modelMatrix, glm::vec3(1.0f));
    mixTextures = true;
    RenderSimpleMesh(meshes["plane"], shaders["LabShader"], modelMatrix, glm::vec3(0.3, 0.3, 0.3), 0, proj, viewMatrix, mapTextures["grass"]);
    mixTextures = false;

    modelMatrix = glm::mat4(1);
    modelMatrix *= transform3D::Translate(tankPosition.x, tankPosition.y, tankPosition.z);
    modelMatrix *= transform3D::RotateOY(-tankAngle);
    modelMatrix *= transform3D::Scale(0.04f, 0.04f, 0.04f);
    RenderSimpleMesh(meshes["senile"], shaders["LabShader"], modelMatrix, glm::vec3(0.2, 0.3, 0.3), 0,  proj, viewMatrix);

    modelMatrix = glm::mat4(1);
    modelMatrix *= transform3D::Translate(tankPosition.x, tankPosition.y, tankPosition.z);
    modelMatrix *= transform3D::RotateOY(-tankAngle);
    modelMatrix *= transform3D::RotateOY(turretAngle); 
    modelMatrix *= transform3D::Scale(0.04f, 0.04f, 0.04f);  
    RenderSimpleMesh(meshes["turret"], shaders["LabShader"], modelMatrix, glm::vec3(0.0, 0.3, 0.0), 0, proj, viewMatrix);

    modelMatrix = glm::mat4(1);
    modelMatrix *= transform3D::Translate(tankPosition.x, tankPosition.y, tankPosition.z);
    modelMatrix *= transform3D::RotateOY(-tankAngle);
    modelMatrix *= transform3D::RotateOY(tunAngle); 
    modelMatrix *= transform3D::Scale(0.04f, 0.04f, 0.04f);
	RenderSimpleMesh(meshes["tun"], shaders["LabShader"], modelMatrix, glm::vec3(0.2, 0.3, 0.3), 0, proj, viewMatrix);

	modelMatrix = glm::mat4(1);
    modelMatrix *= transform3D::Translate(tankPosition.x, tankPosition.y, tankPosition.z);
    modelMatrix *= transform3D::RotateOY(-tankAngle);
	modelMatrix *= transform3D::Scale(0.04f, 0.04f, 0.04f);

	RenderSimpleMesh(meshes["body"], shaders["LabShader"], modelMatrix, glm::vec3(0.1, 0.5, 0.1), 0, proj, viewMatrix);


    for (const auto& building : buildings) {
        glm::mat4 modelMatrix = glm::mat4(1);
        modelMatrix *= transform3D::Translate(building.position.x, building.position.y, building.position.z);
        modelMatrix *= transform3D::Scale(0.7f, 0.7f, 0.7f);
        mixTextures = true;
        RenderSimpleMesh(meshes["block"], shaders["LabShader"], modelMatrix, glm::vec3(0.6, 0.6, 0.7), 0, proj, viewMatrix, mapTextures["block"]);
    mixTextures = false;
     
    }

    for (const auto& enemyTank : enemyTanks) {
        modelMatrix = glm::mat4(1);
        modelMatrix *= transform3D::Translate(enemyTank.enemy_position.x, enemyTank.enemy_position.y, enemyTank.enemy_position.z);
        modelMatrix *= transform3D::RotateOY(enemyTank.angle);
        modelMatrix *= transform3D::Scale(0.04f, 0.04f, 0.04f); 
        RenderSimpleMesh(meshes["senile"], shaders["LabShader"], modelMatrix, glm::vec3(0.6, 0.6, 0.7), enemyTank.damageLevel, proj, viewMatrix);

        modelMatrix = glm::mat4(1);
        modelMatrix *= transform3D::Translate(enemyTank.enemy_position.x, enemyTank.enemy_position.y, enemyTank.enemy_position.z);
        modelMatrix *= transform3D::RotateOY(enemyTank.angle);
        modelMatrix *= transform3D::RotateOY(enemyTank.turretAngle); 

        modelMatrix *= transform3D::Scale(0.04f, 0.04f, 0.04f);
        RenderSimpleMesh(meshes["turret"], shaders["LabShader"], modelMatrix, glm::vec3(0.8, 0.4, 0.4), enemyTank.damageLevel, proj, viewMatrix);

        modelMatrix = glm::mat4(1);
		modelMatrix *= transform3D::Translate(enemyTank.enemy_position.x, enemyTank.enemy_position.y, enemyTank.enemy_position.z);
		modelMatrix *= transform3D::RotateOY(enemyTank.angle);
        modelMatrix *= transform3D::RotateOY(enemyTank.gunAngle); 

		modelMatrix *= transform3D::Scale(0.04f, 0.04f, 0.04f); 
		RenderSimpleMesh(meshes["tun"], shaders["LabShader"], modelMatrix, glm::vec3(0.2, 0.3, 0.3), enemyTank.damageLevel, proj, viewMatrix);

        modelMatrix = glm::mat4(1);
		modelMatrix *= transform3D::Translate(enemyTank.enemy_position.x, enemyTank.enemy_position.y, enemyTank.enemy_position.z);
         modelMatrix *= transform3D::RotateOY(enemyTank.angle);
		modelMatrix *= transform3D::Scale(0.04f, 0.04f, 0.04f); 
        RenderSimpleMesh(meshes["body"], shaders["LabShader"], modelMatrix, glm::vec3(0.5, 0.6, 0.2), enemyTank.damageLevel, proj, viewMatrix);


    }

    for (const auto& bullet : bullets) {
        modelMatrix = glm::mat4(1);
        modelMatrix *= transform3D::Translate(bullet.position.x, bullet.position.y, bullet.position.z);
        modelMatrix *= transform3D::Scale(0.25f, 0.25f, 0.25f); 
        RenderSimpleMesh(meshes["sphere"], shaders["LabShader"], modelMatrix, glm::vec3(1, 0, 0.1), 0, proj, viewMatrix); 
    }


}

void WoTgame::RenderSimpleMesh(Mesh* mesh, Shader* shader, const glm::mat4& modelMatrix, const glm::vec3& color, float damageLevel, glm::mat4 proj, glm::mat4 view, Texture2D* texture1)
{
    if (!mesh || !shader || !shader->GetProgramID())
        return;

    glUseProgram(shader->program);
    
    GLint objectColorLoc = glGetUniformLocation(shader->program, "object_color");
    glUniform3fv(objectColorLoc, 1, glm::value_ptr(color));

    
    GLint modelLocation = glGetUniformLocation(shader->GetProgramID(), "Model");
    
    glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(modelMatrix));
  
    GLint viewLocation = glGetUniformLocation(shader->GetProgramID(), "View");
    
    glm::mat4 viewMatrix = view;
    glUniformMatrix4fv(viewLocation, 1, GL_FALSE, glm::value_ptr(viewMatrix));

 
    GLint projectionLocation = glGetUniformLocation(shader->GetProgramID(), "Projection");
  
    glm::mat4 projectionMatrix = proj;
    glUniformMatrix4fv(projectionLocation, 1, GL_FALSE, glm::value_ptr(projectionMatrix));


    GLint damageLevelLoc = glGetUniformLocation(shader->program, "damageLevel");
    glUniform1f(damageLevelLoc, damageLevel);

    glUniform1i(glGetUniformLocation(shader->program, "mixtext"), mixTextures);


    if (texture1)
    {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture1->GetTextureID());
        glUniform1i(glGetUniformLocation(shader->program, "texture_1"), 0);
    }

    glBindVertexArray(mesh->GetBuffers()->m_VAO);
    glDrawElements(mesh->GetDrawMode(), static_cast<int>(mesh->indices.size()), GL_UNSIGNED_INT, 0);
}



void WoTgame::Update(float deltaTimeSeconds)
{

    gameTime += deltaTimeSeconds;
  
    if (gameTime >= 60.0f) {
        gameActive = false;
        if (!messagePrinted) {
			printf("Game over! You lost!\n");
			printf("Score: %i\n", score);
			messagePrinted = true;
		}
    }

    if (AreAllTanksInactive(enemyTanks)) {
		gameActive = false;
		if (!messagePrinted) {
        printf("Game over! You won!\n");
        messagePrinted = true;
		}
    }

    float distanceBehindTank = 6.0f; 
    float cameraHeight = 3.0f; 
   glm::vec3 cameraOffset = glm::vec3(cos(tankAngle) * distanceBehindTank, cameraHeight, sin(tankAngle) * distanceBehindTank);


    camera->Set(tankPosition + cameraOffset, tankPosition, glm::vec3(0, 1, 0));

    if (gameActive) {

        if (shootCooldownTimer > 0) {
            shootCooldownTimer -= deltaTimeSeconds;
        }

        for (size_t i = 0; i < bullets.size();) {
            bullets[i].position += bullets[i].direction * bullets[i].speed * deltaTimeSeconds;
        
            bullets[i].lifetime -= deltaTimeSeconds;

            bool bulletRemoved = false;

            if (bullets[i].lifetime <= 0) {
                bullets.erase(bullets.begin() + i);
                bulletRemoved = true;
            }
            else {
                for (auto& tank : enemyTanks) {
                    if (IsCollidingWithTank(bullets[i], tank)) {
                        printf("hit tank\n");
                        bullets.erase(bullets.begin() + i);
                        bulletRemoved = true;
                        if (tank.isActive) {
                            tank.health -= 1;

                            tank.damageLevel = 3 - tank.health;
                            if (tank.health <= 0) {
								score += 2;
								
							}
                        }
               

                        break;
                    }
                }

                if (!bulletRemoved) {
                    for (const auto& building : buildings) {
                        if (IsCollidingWithBuilding(bullets[i], building)) {
                            printf("hit building\n");
                            bullets.erase(bullets.begin() + i);
                            bulletRemoved = true;
                            break;
                        }
                    }
                }
            }

            if (!bulletRemoved) {
                ++i;
            }
        }


        for (auto& enemyTank : enemyTanks) {

            if (enemyTank.health <= 0) {
                enemyTank.isActive = false;
            }
            if (enemyTank.isActive) {
                if (enemyTank.isRotating) {

                    enemyTank.rotationTimer += deltaTimeSeconds;
                    if (enemyTank.rotationTimer >= 2.0f) { 
                        enemyTank.isRotating = false;
                        enemyTank.rotationTimer = 0;
                        enemyTank.movementTimer = RandomFloat(1.0f, 3.0f);
                        enemyTank.isMovingForward = rand() % 2 == 0;
                    }
                    else {
                        enemyTank.angle += deltaTimeSeconds * enemyTank.targetRotation * 0.8f; 
                    }
                }
                else {
                    enemyTank.movementTimer -= deltaTimeSeconds;
                    if (enemyTank.movementTimer <= 0) {
                        enemyTank.isRotating = true;
                        float maxRotationAngle = M_PI / 2; 
                        enemyTank.targetRotation = RandomFloat(-maxRotationAngle, maxRotationAngle);

                    }
                    else {
                       
                        glm::vec3 forwardDir = glm::vec3(-cos(enemyTank.angle), 0, sin(enemyTank.angle));
                        float moveDirection = enemyTank.isMovingForward ? 1.0f : -1.0f;
                        enemyTank.enemy_position += forwardDir * 3.0f * moveDirection * deltaTimeSeconds;

                    }
                }


                if (IsOutOfBounds(enemyTank.enemy_position, WORLD_SIZE)) {
                    enemyTank.enemy_position.x = std::max(std::min(enemyTank.enemy_position.x, WORLD_SIZE), -WORLD_SIZE);
                    enemyTank.enemy_position.z = std::max(std::min(enemyTank.enemy_position.z, WORLD_SIZE), -WORLD_SIZE);
                    enemyTank.isRotating = true;
                }
            }
        }

       

        for (const auto& building : buildings) {
            if (IsPlayerCollidingWithBuilding(tankPosition, building)) {
                ResolvePlayerCollisionWithBuilding(tankPosition, building);
            }
        }

        for (auto& enemyTank : enemyTanks) {
            for (const auto& building : buildings) {
                if (IsCollidingWithBuilding(enemyTank, building)) {
                    ResolveCollisionWithBuilding(enemyTank, building);
                }
            }
        }

        for (auto& enemyTank : enemyTanks) {
            if (IsCollidingWithPlayerTank(enemyTank, tankPosition)) {
                ResolveCollisionWithPlayerTank(enemyTank, tankPosition);
            }
        }

        for (size_t i = 0; i < enemyTanks.size(); i++) {
            for (size_t j = i + 1; j < enemyTanks.size(); j++) {
                if (AreTanksColliding(enemyTanks[i], enemyTanks[j])) {
                    ResolveTankCollision(enemyTanks[i], enemyTanks[j]);
                }
            }
        }
    }

        glm::ivec2 resolution = window->GetResolution();
        glViewport(0, 0, resolution.x, resolution.y);
        RenderScene(this->camera, camera->GetViewMatrix(), GetSceneCamera()->GetProjectionMatrix());
        DrawCoordinateSystem(camera->GetViewMatrix(), GetSceneCamera()->GetProjectionMatrix());


        glClear(GL_DEPTH_BUFFER_BIT);
        
        glViewport(miniViewportArea.x, miniViewportArea.y, miniViewportArea.width, miniViewportArea.height);
      
        glm::vec3 cameraPosition = tankPosition + glm::vec3(0, 10, 0);

       
        glm::vec3 lookAtPosition = tankPosition;

        glm::vec3 upDirection = glm::vec3(0, 0, -1);

        glm::mat4 view = glm::lookAt(cameraPosition, lookAtPosition, upDirection);
       RenderScene(minimapCamera, view, projectionMatrixMinimap );
       DrawCoordinateSystem(minimapCamera->GetViewMatrix(), projectionMatrixMinimap);
   
}


void WoTgame::FrameEnd()
{
    
}


void WoTgame::OnInputUpdate(float deltaTime, int mods)
{


    if (gameActive==true){
        glm::vec3 forwardDir = glm::vec3(cos(tankAngle), 0, sin(tankAngle));

    if (window->KeyHold(GLFW_KEY_W)) {
        camera->TranslateForward(deltaTime * 2);
        tankPosition -= forwardDir * tankSpeed * deltaTime;
    }

    if (window->KeyHold(GLFW_KEY_S)) {
        camera->TranslateUpward(deltaTime * 2);
        tankPosition += forwardDir * tankSpeed * deltaTime;
    }

    float tankRotationSpeed = 1.0f; 

    if (window->KeyHold(GLFW_KEY_A)) {
        camera->RotateThirdPerson_OY(-deltaTime * 2);
        tankAngle -= tankRotationSpeed * deltaTime;
    }

    if (window->KeyHold(GLFW_KEY_D)) {
        camera->RotateThirdPerson_OY(-deltaTime * 2);
        tankAngle += tankRotationSpeed * deltaTime;
    }
}
}


void WoTgame::OnKeyPress(int key, int mods)
{


}


void WoTgame::OnKeyRelease(int key, int mods)
{
}


void WoTgame::OnMouseMove(int mouseX, int mouseY, int deltaX, int deltaY)
{
    float sensitivity = 0.005f;
    turretAngle -= deltaX * sensitivity;
    tunAngle -= deltaX * sensitivity;
}


void WoTgame::OnMouseBtnPress(int mouseX, int mouseY, int button, int mods)
{

    if (gameActive == true) {
        if (button == 1) {
            if (shootCooldownTimer <= 0) {
                Bullet newBullet;

                newBullet.direction = glm::vec3(-cos(tankAngle - turretAngle), 0, -sin(tankAngle - turretAngle + 0.12f));
                float barrelLength = 2.0f;

                glm::vec3 barrelEndLocalPosition = glm::vec3(-3.3, 1, -0.2);

                glm::mat4 rotationMatrix = glm::rotate(glm::mat4(1.0f), -tankAngle + turretAngle, glm::vec3(0, 1, 0));

                glm::vec3 barrelEndWorldPosition = tankPosition + glm::vec3(rotationMatrix * glm::vec4(barrelEndLocalPosition, 1.0f));

                newBullet.position = barrelEndWorldPosition;
                newBullet.speed = 6.0f; 
                newBullet.lifetime = 3.0f; 

                bullets.push_back(newBullet);
                shootCooldownTimer = 1.0f;
            }
        }
    }
}


void WoTgame::OnMouseBtnRelease(int mouseX, int mouseY, int button, int mods)
{

}


void WoTgame::OnMouseScroll(int mouseX, int mouseY, int offsetX, int offsetY)
{
}


void WoTgame::OnWindowResize(int width, int height)
{
}
