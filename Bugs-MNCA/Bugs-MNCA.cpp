#include <iostream>
#include <SFML/Graphics.hpp>

#define resolution 200


using namespace std;
using namespace sf;

int main()
{
    bool drag = false;
    Glsl::Vec2 current_offset;
    Glsl::Vec2 dragOrigin;
    long long generation = 0;
    float zoom = 1.0;
    Clock uptime;
    Event event;

    RenderWindow window(VideoMode(resolution, resolution), "Game of Life GLSL");
    window.setFramerateLimit(60);

    VertexArray canvas(Quads, 4);
    canvas[0] = Vertex(Vector2f(0, 0), Color::White, Vector2f(0, 0));
    canvas[1] = Vertex(Vector2f(0, resolution), Color::White, Vector2f(0, resolution));
    canvas[2] = Vertex(Vector2f(resolution, resolution), Color::White, Vector2f(resolution, resolution));
    canvas[3] = Vertex(Vector2f(resolution, 0), Color::White, Vector2f(resolution, 0));

    RenderTexture* nextGen = new RenderTexture;
    nextGen->create(resolution, resolution, ContextSettings());
    nextGen->setRepeated(true);

    RenderTexture* currentGen = new RenderTexture;
    currentGen->create(resolution, resolution, ContextSettings());
    currentGen->setRepeated(true);

    Image* initState = new Image;
    initState->create(resolution, resolution);
    for (Uint8 y = 0; y < resolution; y++)
    {
        for (Uint8 x = 0; x < resolution; x++)
        {
            unsigned char color = (rand() % 2) * 255;
            initState->setPixel(x, y, Color(color, color, color, 255));
        }
    }

    Texture* upload = new Texture;
    upload->loadFromImage(*initState);
    currentGen->draw(canvas, upload);
    currentGen->display();

    delete upload;
    delete initState;

    Shader shader;
    shader.loadFromFile("fragment.glsl", Shader::Fragment);
    shader.setUniform("resolution", float(resolution));

    Shader dispShader;
    dispShader.loadFromFile("display_fragment.glsl", Shader::Fragment);
    dispShader.setUniform("zoom", zoom);

    RenderStates states;
    states.shader = &shader;
    states.texture = &currentGen->getTexture();

    RenderStates dispStates;
    dispStates.shader = &dispShader;

    float start = 0;
    float end = 0;

    while (window.isOpen())
    {

        while (window.pollEvent(event))
        {
            switch (event.type)
            {
            case Event::Closed:
                window.close();
                break;
            case Event::MouseWheelScrolled:
                zoom += event.mouseWheelScroll.delta * .1;
                zoom = zoom < .25f ? .25f : zoom;
                dispShader.setUniform("zoom", zoom);
                break;
            case Event::MouseButtonPressed:
                if (event.mouseButton.button == Mouse::Left)
                {
                    drag = true;
                    dragOrigin = Glsl::Vec2(Mouse::getPosition());
                }
                break;
            case Event::MouseButtonReleased:
                if (event.mouseButton.button == Mouse::Left)
                {
                    drag = false;
                    dragOrigin = Glsl::Vec2(0, 0);
                }
                break;
            default:
                break;

            }
        }


        nextGen->draw(canvas, states);
        nextGen->display();

        RenderTexture* swap = currentGen;
        currentGen = nextGen;
        nextGen = swap;

        if (drag)
        {
            Glsl::Vec2 offset;
            offset = Glsl::Vec2(Mouse::getPosition());
            offset = dragOrigin - offset;
            offset.y *= -1;
            offset.x /= resolution;
            offset.y /= resolution;
            current_offset += offset;
            dragOrigin = Glsl::Vec2(Mouse::getPosition());
        }

        dispStates.texture = &currentGen->getTexture();
        dispShader.setUniform("offset", current_offset);


        window.draw(canvas, dispStates);
        window.display();

        end = uptime.getElapsedTime().asSeconds();

        start = uptime.getElapsedTime().asSeconds();
        generation++;
    }

    delete currentGen;
    delete nextGen;

    return 0;
}