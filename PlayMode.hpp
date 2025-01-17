#pragma once
#include "Mode.hpp"
#include "Scene.hpp"

#include <glm/glm.hpp>

#include <vector>
#include <deque>
#include <iostream>
#include <memory>
namespace {
const float deacc_const = 0.5f;
const float ball_radius = 0.5f;
struct ball{
	glm::vec3 velocity;
	Scene::Transform *cur;
	bool pushable;
	ball(){velocity = glm::vec3(); cur = nullptr; pushable = true;}
	ball(glm::vec3 v, Scene::Transform *c, bool b) : velocity(std::move(v)), cur(c), pushable(b) {};
	void update(float elapsed){
		cur->position += velocity * elapsed;
		velocity *= (1.0f - deacc_const * elapsed);
		// collide checking
		// use central coin as the origin point and do detect boundary collision testing
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

	//self-defined
	ball player1_t;
	ball player2_t;
	Scene::Transform *arrow1_t = nullptr;
	Scene::Transform *arrow2_t = nullptr;
	glm::quat arrow1_base_rotation;
	glm::quat arrow2_base_rotation;


	float wobble_1 = 0.0f;
	float wobble_2 = 0.0f;
	bool left_turn = true;
	int left_force = 5;
	int right_force = 5;
	int left_pts = 0;
	int right_pts = 0;

	Scene::Transform *endpt1;
	Scene::Transform *endpt2;

	glm::vec3 world_origin = glm::vec3(0.0f, 0.0f, 0.0f);

	std::vector<std::shared_ptr<Scene::Transform>> coins;
	// std::vector<ball> fake_coins;
	float total_time = 0.0f;
	//camera:
	Scene::Camera *camera = nullptr;

};
