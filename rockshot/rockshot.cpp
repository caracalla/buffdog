#include <cctype>
#include <chrono>
#include <cstdlib>
#include <list>

#include "../device.h"
#include "../matrix.h"
#include "../vector.h"

#include "bmp.h"
#include "entity.h"
#include "model.h"
#include "obj.h"
#include "ppm.h"
#include "renderer.h"
#include "scene.h"
#include "texture.h"
#include "triangle.h"


// ugh
#ifdef _MSC_VER
// const char* city_model_file = "rockshot/assets/models/city.obj";
// const char* city_texture_file = "rockshot/assets/textures/city.ppm";
const char* crate_texture_file = "rockshot/assets/textures/crate.bmp";
const char* basic_level_file = "rockshot/assets/basic.level";
#else
// const char* city_model_file = "assets/models/city.obj";
// const char* city_texture_file = "assets/textures/city.ppm";
const char* crate_texture_file = "assets/textures/crate.bmp";
const char* basic_level_file = "assets/basic.level";
#endif



#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

struct Fighter {
	Vector pos = Vector::origin();
	Vector rot = Vector::direction(0.0, 0.0, 0.0);

	struct Dimensions {
		float height = 0.0;
		float width = 0.0;
		float eye_height = 0.0;
	};

	Dimensions dimensions;

	static Model buildModel(Dimensions dims) {
		// the model will have its bottom centered at the origin
		Vector start = Vector::point(-dims.width, -dims.eye_height, -dims.width);
		Vector end = Vector::point(dims.width, dims.height - dims.eye_height, dims.width);

		return Model::buildHexahedron(start, end);
	}
};

struct Platform {
	Vector start_pos = Vector::origin();;
	Vector end_pos = Vector::origin();;
	Model model;
};

struct LevelData {
	std::vector<Fighter> fighters;
	std::vector<Platform> platforms;
	bool valid = false;

	Model fighter_model; // fighters share the same model for now
};

void logLevelLoadError(const char* message, std::string line) {
	printf("%s: %s\n", message, line.c_str());
}

LevelData loadLevelFromFile(const char* filename) {
	std::ifstream level_file(filename);
	LevelData level_data;

	Fighter::Dimensions fighter_dimensions;

	bool got_fighter_dimensions = false;

	if (!level_file.is_open()) {
		printf("couldn't read level file %s!\n", filename);
		return level_data;
	}

	std::string line;

	while (std::getline(level_file, line)) {
		if (line.size() == 0 || line[0] == '#') {
			continue;
		}

		std::istringstream line_stream(line);
		char first_char;
		line_stream >> first_char;

		if (first_char == 'i') {
			// shared fighter info
			if (!(line_stream >> fighter_dimensions.height >> fighter_dimensions.width >> fighter_dimensions.eye_height)) {
				logLevelLoadError("fighter shared info improperly formatted!", line);
				return level_data;
			}

			got_fighter_dimensions = true;
		} else if (first_char == 'f') {
			// load a fighter
			if (!got_fighter_dimensions) {
				logLevelLoadError("need shared fighter info before loading fighters!", line);
				return level_data;
			}

			Fighter fighter;

			if (!(line_stream >> fighter.pos.x >> fighter.pos.y >> fighter.pos.z)) {
				logLevelLoadError("fighter position improperly formatted!", line);
				return level_data;
			}

			if (!(line_stream >> fighter.rot.x >> fighter.rot.y >> fighter.rot.z)) {
				logLevelLoadError("fighter rotation improperly formatted!", line);
				return level_data;
			}

			fighter.dimensions = fighter_dimensions;

			level_data.fighters.push_back(fighter);
		} else if (first_char == 'p') {
			Platform platform;

			if (!(line_stream >> platform.start_pos.x >> platform.start_pos.y >> platform.start_pos.z)) {
				logLevelLoadError("platform start position improperly formatted!", line);
				return level_data;
			}

			if (!(line_stream >> platform.end_pos.x >> platform.end_pos.y >> platform.end_pos.z)) {
				logLevelLoadError("platform end position improperly formatted!", line);
				return level_data;
			}

			platform.model = Model::buildHexahedron(platform.start_pos, platform.end_pos);

			level_data.platforms.push_back(platform);
		} else {
			logLevelLoadError("unrecognized line", line);
		}
	}

	if (level_data.fighters.size() < 1) {
		printf("no fighters found in level %s!\n", filename);
	} else if (level_data.platforms.size() < 1) {
		printf("no platforms found in level %s!\n", filename);
	} else {
		level_data.fighter_model = Fighter::buildModel(fighter_dimensions);
		level_data.valid = true;
	}

	return level_data;
}



int main(int argc, char** argv) {
	if (!device::setUp()) {
		return 1;
	}

	auto last_frame_time = std::chrono::steady_clock::now();

	LevelData basic_level = loadLevelFromFile(basic_level_file);

	if (!basic_level.valid) {
		printf("couldn't load level\n");
		return 0;
	}

	// for now, player is the first fighter in the level
	// later, others will be enemies
	Player player;
	player.position = basic_level.fighters[0].pos;
	player.rotation = basic_level.fighters[0].rot;
	player.model = &basic_level.fighter_model;

	player.weapon.player_local_position = Vector::direction(0.25, player.eye_height, 0);
	player.weapon.bullet = Model::buildTetrahedron();
	player.weapon.explosion = Model::buildExplosionModel();
	Model weapon_model = player.weapon.buildModel();
	player.weapon.model = &weapon_model;
	player.weapon.translucency = 2;

	spit("Player created successfully");

	Scene scene;
	scene.init(std::move(player));

	spit("Scene initialized successfully");

	// add spinning cube
	Model cube_model = Model::buildCube();
	BMPTexture crate_texture = BMPTexture::load(crate_texture_file);
	cube_model.setTexture(&crate_texture);

	Entity cube_entity;
	cube_entity.model = &cube_model;
	cube_entity.position = Vector::point(15, 2, 0);

	scene.addEntityWithAction(std::move(cube_entity), [](Entity* self, std::chrono::microseconds frame_duration) {
		self->rotation.x += 0.005;
		self->rotation.y += 0.007;
		self->rotation.z += 0.009;

		key_input key = device::getNextKey();

		if (key == z_key) {
			self->translucency += 1;
			printf("translucency is now %d\n", self->translucency);
		}

		if (key == x_key) {
			self->translucency -= 1;
			printf("translucency is now %d\n", self->translucency);
		}
	});

	spit("Spinning crate created successfully");

	// load Rocket Fighters entities (just platforms for now)
	std::list<Model> platform_models;

	for (int i = 0; i < basic_level.platforms.size(); i++) {
		Platform& platform = basic_level.platforms[i];
		Model platform_model = Model::buildHexahedron(platform.start_pos, platform.end_pos);
		platform_models.push_back(platform_model);

		Entity platform_entity;
		platform_entity.model = &(platform_models.back());
		// all platforms are "positioned" at the origin, because their start and
		// end positions convey where it actually is.  This sucks, fix it.
		platform_entity.position = Vector::origin();
		platform_entity.rotation = Vector::direction(0.0, 0.0, 0.0);
		scene.addEntity(std::move(platform_entity));
	}

	spit("Finished setting up the scene");

	Renderer renderer = Renderer::create(scene.camera.viewport);

	spit("Renderer created successfully");

	while (device::running()) {
		// draw the level and models
		renderer.drawScene(scene);

		// paint the screen
		device::updateScreen();

		// grab keyboard and mouse input
		device::processInput();

		// update player, models, and level by one time step
		auto now = std::chrono::steady_clock::now();
		auto frame_duration =
				std::chrono::duration_cast<std::chrono::microseconds>(now - last_frame_time);
		last_frame_time = now;

		scene.step(frame_duration);
	}

	device::tearDown();

	return 0;
}
