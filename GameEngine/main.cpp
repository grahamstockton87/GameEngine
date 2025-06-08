#include "Game.h"
int main() {



	Game game;
	if (game.Initialize()) {
		std::cout << "OpenGL Version: "
			<< reinterpret_cast<const char*>(glGetString(GL_VERSION)) << std::endl;
		game.Run();
	} else {
		return -1; // Initialization failed
	}
	return 0;
}