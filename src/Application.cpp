#include <Engine.hpp>

int main(int argc, char* argv[]) {

	Engine engine;
	engine.Init();
	engine.Run();
	engine.CleanUp();

	return 0;
}