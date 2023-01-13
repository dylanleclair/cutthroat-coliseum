#include <GLFW/glfw3.h>
#include <GL/glew.h>
#include "Window.h"

enum Primitive { sphere, cube };

//a package of data telling the system what primitive to draw
struct render_packet {
	Primitive shape;
	int radius;
	float x;
	float y;
	render_packet(Primitive _primitive, int _radius, float _x, float _y) : shape(_primitive), radius(_radius), x(_x), y(_y) {}
};



class GraphicsSystem {
public:
	GraphicsSystem(Window& _window);
	void addPrimitive(render_packet _packet);
	void render();
	Window getWindow();
private:
	ShaderProgram shader;

};