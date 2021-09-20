#include "PlayMode.hpp"

#include "LitColorTextureProgram.hpp"

#include "DrawLines.hpp"
#include "Mesh.hpp"
#include "Load.hpp"
#include "gl_errors.hpp"
#include "data_path.hpp"

#include <glm/gtc/type_ptr.hpp>
#include <cmath>
#include <random>
#include <string>
#include <algorithm>

namespace
{
	const float collide_radius = 6.0f;
	const float inf = 9999999.0f;
	float len(glm::vec3 x)
	{
		return std::sqrt(x.x * x.x + x.y * x.y + x.z * x.z);
	}
	glm::vec3 extend(glm::vec3 x, float k)
	{
		return x / len(x) * k;
	}
	glm::vec3 real_pos(Scene::Transform &A)
	{
		glm::vec4 b(A.position, 1.0f);
		return A.make_local_to_world() * b;
	}
	void collide(ball &ball_A, ball &ball_B)
	{
		std::swap(ball_A.velocity, ball_B.velocity);
	}
	bool iscollide(Scene::Transform &A, Scene::Transform &B)
	{
		return std::abs(len(real_pos(A) - real_pos(B))) <= collide_radius;
	}
	void remove_off(Scene::Transform &A)
	{
		A.position = glm::vec3(inf, inf, inf);
	}

} // namespace

GLuint hexapod_meshes_for_lit_color_texture_program = 0;
Load<MeshBuffer> hexapod_meshes(LoadTagDefault, []() -> MeshBuffer const * {
	MeshBuffer const *ret = new MeshBuffer(data_path("playground.pnct"));
	hexapod_meshes_for_lit_color_texture_program = ret->make_vao_for_program(lit_color_texture_program->program);
	return ret;
});

Load<Scene> hexapod_scene(LoadTagDefault, []() -> Scene const * {
	return new Scene(data_path("playground.scene"), [&](Scene &scene, Scene::Transform *transform, std::string const &mesh_name) {
		Mesh const &mesh = hexapod_meshes->lookup(mesh_name);

		scene.drawables.emplace_back(transform);
		Scene::Drawable &drawable = scene.drawables.back();

		drawable.pipeline = lit_color_texture_program_pipeline;

		drawable.pipeline.vao = hexapod_meshes_for_lit_color_texture_program;
		drawable.pipeline.type = mesh.type;
		drawable.pipeline.start = mesh.start;
		drawable.pipeline.count = mesh.count;
	});
});

PlayMode::PlayMode() : scene(*hexapod_scene)
{
	//get pointers to leg for convenience:
	for (auto &transform : scene.transforms)
	{
		//std::cout << transform.name << std::endl;
		if (transform.name == "player1")
		{
			//std::cout << transform.name << "\n";
			//std::cout << "coin.x: " << transform.position.x << "coin.y" << transform.position.y << "coin.z" << transform.position.z << "\n";
			player1_t.cur = &transform;
		}
		if (transform.name == "player2")
		{
			//std::cout << transform.name << "\n";
			//std::cout << "coin.x: " << transform.position.x << "coin.y" << transform.position.y << "coin.z" << transform.position.z << "\n";
			player2_t.cur = &transform;
		}

		if (transform.name == "arrow1")
		{
			arrow1_t = &transform;
			arrow1_base_rotation = arrow1_t->rotation;
		}

		if (transform.name == "arrow2")
		{
			arrow2_t = &transform;
			arrow2_base_rotation = arrow2_t->rotation;
		}
		if (transform.name.substr(0, 4) == "Coin")
		{

			coins.emplace_back(&transform);
		}
		std::cout << transform.name << "\n";
		auto &A = real_pos(transform);
		std::cout << " coin.x: " << A.x << " coin.y " << A.y << " coin.z " << A.z << "\n";
	}
	//get pointer to camera for convenience:
	if (scene.cameras.size() != 1)
		throw std::runtime_error("Expecting scene to have exactly one camera, but it has " + std::to_string(scene.cameras.size()));
	camera = &scene.cameras.front();
}

PlayMode::~PlayMode()
{
}

bool PlayMode::handle_event(SDL_Event const &evt, glm::uvec2 const &window_size)
{

	if (evt.type == SDL_KEYDOWN)
	{
		if (evt.key.keysym.sym == SDLK_ESCAPE)
		{
			SDL_SetRelativeMouseMode(SDL_FALSE);
			return true;
		}
		else if (evt.key.keysym.sym == SDLK_a)
		{
			left.downs += 1;
			left.pressed = true;
			return true;
		}
		else if (evt.key.keysym.sym == SDLK_d)
		{
			right.downs += 1;
			right.pressed = true;
			return true;
		}
		else if (evt.key.keysym.sym == SDLK_w)
		{
			up.downs += 1;
			up.pressed = true;
			return true;
		}
		else if (evt.key.keysym.sym == SDLK_s)
		{
			down.downs += 1;
			down.pressed = true;
			return true;
		}
		else if (evt.key.keysym.sym == SDLK_UP)
		{
			if (left_turn)
				left_force = std::min(++left_force, 10);
			else
				right_force = std::min(++right_force, 10);
		}
		else if (evt.key.keysym.sym == SDLK_DOWN)
		{
			if (left_turn)
				left_force = std::max(--left_force, 1);
			else
				right_force = std::max(--right_force, 1);
		}
		else if (evt.key.keysym.sym == SDLK_SPACE)
		{
			if (player1_t.pushable && left_turn)
			{
				player1_t.pushable = false;
				float angle = 2.0f * float(M_PI) * wobble_1;
				glm::vec3 dir(cos(angle), sin(angle), 0.0f);
				player1_t.push(extend(dir, 0.0f - left_force));
			}
			if (player2_t.pushable && !left_turn)
			{
				player2_t.pushable = false;
				float angle = 2.0f * float(M_PI) * wobble_2;
				glm::vec3 dir(cos(angle), sin(angle), 0.0f);
				player2_t.push(extend(dir, 0.0f - right_force));
			}
			left_turn = !left_turn;
		}
		else if (evt.key.keysym.sym == SDLK_r)
		{
			PlayMode();
		}
	}
	else if (evt.type == SDL_KEYUP)
	{
		if (evt.key.keysym.sym == SDLK_a)
		{
			left.pressed = false;
			return true;
		}
		else if (evt.key.keysym.sym == SDLK_d)
		{
			right.pressed = false;
			return true;
		}
		else if (evt.key.keysym.sym == SDLK_w)
		{
			up.pressed = false;
			return true;
		}
		else if (evt.key.keysym.sym == SDLK_s)
		{
			down.pressed = false;
			return true;
		}
	}
	else if (evt.type == SDL_MOUSEBUTTONDOWN)
	{
		if (SDL_GetRelativeMouseMode() == SDL_FALSE)
		{
			SDL_SetRelativeMouseMode(SDL_TRUE);
			return true;
		}
	}
	else if (evt.type == SDL_MOUSEMOTION)
	{
		if (SDL_GetRelativeMouseMode() == SDL_TRUE)
		{
			glm::vec2 motion = glm::vec2(
				evt.motion.xrel / float(window_size.y),
				-evt.motion.yrel / float(window_size.y));
			camera->transform->rotation = glm::normalize(
				camera->transform->rotation * glm::angleAxis(-motion.x * camera->fovy, glm::vec3(0.0f, 1.0f, 0.0f)) * glm::angleAxis(motion.y * camera->fovy, glm::vec3(1.0f, 0.0f, 0.0f)));
			return true;
		}
	}

	return false;
}

void PlayMode::update(float elapsed)
{

	//slowly rotates through [0,1):
	if (player1_t.pushable)
	{
		wobble_1 += elapsed / 5.0f;
		wobble_1 -= std::floor(wobble_1);
		float angle_1 = 2.0f * float(M_PI) * wobble_1;
		arrow1_t->rotation = arrow1_base_rotation * glm::angleAxis(
														angle_1,
														glm::vec3(0.0f, 0.0f, 1.0f));
	}
	else if (player2_t.pushable)
	{
		wobble_2 += elapsed / 5.0f;
		wobble_2 -= std::floor(wobble_2);
		float angle_2 = 2.0f * float(M_PI) * wobble_2;
		arrow2_t->rotation = arrow2_base_rotation * glm::angleAxis(
														angle_2,
														glm::vec3(0.0f, 0.0f, 1.0f));
	}

	if (!player2_t.pushable && !player1_t.pushable)
	{
		player1_t.update(elapsed);
		player2_t.update(elapsed);
		//collide
		for (auto &c : coins)
		{
			if (iscollide(*c, *player1_t.cur))
			{
				++left_pts;
				remove_off(*c);
			}
			else if (iscollide(*c, *player2_t.cur))
			{
				++right_pts;
				remove_off(*c);
			}
		}
		if (iscollide(*player1_t.cur, *player2_t.cur))
		{
			collide(player1_t, player2_t);
		}
	}

	//move camera:
	{
		//combine inputs into a move:
		constexpr float PlayerSpeed = 30.0f;
		glm::vec2 move = glm::vec2(0.0f);
		if (left.pressed && !right.pressed)
			move.x = -1.0f;
		if (!left.pressed && right.pressed)
			move.x = 1.0f;
		if (down.pressed && !up.pressed)
			move.y = -1.0f;
		if (!down.pressed && up.pressed)
			move.y = 1.0f;

		//make it so that moving diagonally doesn't go faster:
		if (move != glm::vec2(0.0f))
			move = glm::normalize(move) * PlayerSpeed * elapsed;

		glm::mat4x3 frame = camera->transform->make_local_to_parent();
		glm::vec3 right = frame[0];
		// glm::vec3 up = frame[1];
		glm::vec3 forward = -frame[2];

		camera->transform->position += move.x * right + move.y * forward;
	}

	//reset button press counters:
	left.downs = 0;
	right.downs = 0;
	up.downs = 0;
	down.downs = 0;
}

void PlayMode::draw(glm::uvec2 const &drawable_size)
{
	//update camera aspect ratio for drawable:
	camera->aspect = float(drawable_size.x) / float(drawable_size.y);

	//set up light type and position for lit_color_texture_program:
	// TODO: consider using the Light(s) in the scene to do this
	glUseProgram(lit_color_texture_program->program);
	glUniform1i(lit_color_texture_program->LIGHT_TYPE_int, 1);
	glUniform3fv(lit_color_texture_program->LIGHT_DIRECTION_vec3, 1, glm::value_ptr(glm::vec3(0.0f, 0.0f, -1.0f)));
	glUniform3fv(lit_color_texture_program->LIGHT_ENERGY_vec3, 1, glm::value_ptr(glm::vec3(1.0f, 1.0f, 0.95f)));
	glUseProgram(0);

	glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
	glClearDepth(1.0f); //1.0 is actually the default value to clear the depth buffer to, but FYI you can change it.
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS); //this is the default depth comparison function, but FYI you can change it.

	GL_ERRORS(); //print any errors produced by this setup code

	scene.draw(*camera);

	{
		// draw force bars
		glDisable(GL_DEPTH_TEST);
		float aspect = float(drawable_size.x) / float(drawable_size.y);
		DrawLines bars(glm::mat4(
			1.0f / aspect, 0.0f, 0.0f, 0.0f,
			0.0f, 1.0f, 0.0f, 0.0f,
			0.0f, 0.0f, 1.0f, 0.0f,
			0.0f, 0.0f, 0.0f, 1.0f));
		constexpr float H = 0.09f;
		std::string lforce = "Left force at: ";
		lforce += std::to_string(left_force);
		lforce += " Current Score is: " + std::to_string(left_pts);
		bars.draw_text(lforce,
					   glm::vec3(-aspect + 0.9f * H, -1.0 + 20.0f * H, 0.0),
					   glm::vec3(H, 0.0f, 0.0f), glm::vec3(0.0f, H, 0.0f),
					   glm::u8vec4(0xff, 0xff, 0xff, 0xff));
		std::string rforce = "Right force at: ";
		rforce += std::to_string(right_force);
		rforce += " Current Score is: " + std::to_string(right_pts);
		bars.draw_text(rforce,
					   glm::vec3(-aspect + 25.2f * H, -1.0 + 20.0f * H, 0.0),
					   glm::vec3(H, 0.0f, 0.0f), glm::vec3(0.0f, H, 0.0f),
					   glm::u8vec4(0xff, 0xff, 0xff, 0xff));
	}

	{ //use DrawLines to overlay some text:
		glDisable(GL_DEPTH_TEST);
		float aspect = float(drawable_size.x) / float(drawable_size.y);
		DrawLines lines(glm::mat4(
			1.0f / aspect, 0.0f, 0.0f, 0.0f,
			0.0f, 1.0f, 0.0f, 0.0f,
			0.0f, 0.0f, 1.0f, 0.0f,
			0.0f, 0.0f, 0.0f, 1.0f));

		constexpr float H = 0.09f;
		lines.draw_text("Mouse motion rotates camera; WASD moves; escape ungrabs mouse",
						glm::vec3(-aspect + 0.1f * H, -1.0 + 0.1f * H, 0.0),
						glm::vec3(H, 0.0f, 0.0f), glm::vec3(0.0f, H, 0.0f),
						glm::u8vec4(0x00, 0x00, 0x00, 0x00));
		float ofs = 2.0f / drawable_size.y;
		lines.draw_text("Mouse motion rotates camera; WASD moves; escape ungrabs mouse",
						glm::vec3(-aspect + 0.1f * H + ofs, -1.0 + +0.1f * H + ofs, 0.0),
						glm::vec3(H, 0.0f, 0.0f), glm::vec3(0.0f, H, 0.0f),
						glm::u8vec4(0xff, 0xff, 0xff, 0x00));
	}
}
