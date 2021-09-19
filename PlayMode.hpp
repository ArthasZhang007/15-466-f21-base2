#pragma once
#include "Mode.hpp"
#include "Scene.hpp"

#include <glm/glm.hpp>

#include <vector>
#include <deque>
namespace {
const float deacc_const = 0.01f;
const float ball_radius = 0.5f;
struct ball{
	glm::vec3 velocity;
	Scene::Transform *cur;
	bool pushable;
	ball(){velocity = glm::vec3(); cur = nullptr; pushable = true;}
	void update(float elapsed){
		cur->position += velocity * elapsed;
		velocity *= (1.0f - deacc_const * elapsed);
	}
	void push(glm::vec3 acc){
		velocity += acc;
	}
};
// void collide(ball &ball_A, ball & ball_B){
	
// }
}

struct PlayMode : Mode {
	PlayMode();
	virtual ~PlayMode();

	//functions called by main loop:
	virtual bool handle_event(SDL_Event const &, glm::uvec2 const &window_size) override;
	virtual void update(float elapsed) override;
	virtual void draw(glm::uvec2 const &drawable_size) override;

	//----- game state -----

	//input tracking:
	struct Button {
		uint8_t downs = 0;
		uint8_t pressed = 0;
	} left, right, down, up;

	//local copy of the game scene (so code can change it during gameplay):
	Scene scene;

	//hexapod leg to wobble:
	Scene::Transform *hip = nullptr;
	Scene::Transform *upper_leg = nullptr;
	Scene::Transform *lower_leg = nullptr;

	//self-defined
	ball player1_t;
	Scene::Transform *arrow1_t = nullptr;
	glm::quat arrow_base_rotation;


	glm::quat hip_base_rotation;
	glm::quat upper_leg_base_rotation;
	glm::quat lower_leg_base_rotation;
	float wobble = 0.0f;

	//----- game state -----
    glm::vec2 court_radius = glm::vec2(7.0f, 5.0f);
    glm::vec2 paddle_radius = glm::vec2(0.2f, 1.0f);
    glm::vec2 ball_radius = glm::vec2(0.2f, 0.2f);

	bool left_turn = true;
	int left_force = 5;
	int right_force = 5;
	
	//camera:
	Scene::Camera *camera = nullptr;

};
