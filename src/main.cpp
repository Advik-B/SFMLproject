#include <SFML/Graphics.hpp>
#include <iostream>
#include <vector>
#include <thread>
#include <imgui-SFML.h>
#include <imgui.h>

int dimRatio[] = { 1920, 1080 };
int maxParticles = 1000;

class Particle : public sf::CircleShape {
public:
    Particle(float radius, sf::Vector2f position, sf::Vector2f velocity)
        : sf::CircleShape(radius), velocity(velocity) {
        setPosition(position);
//        setFillColor(sf::Color(255, 213, 78)); // Set the particle color
        setOrigin(radius, radius); // Set origin to center of circle

    }

    void update(float dt) {
        velocity.y += gravity * dt; // Apply gravity
        move(velocity * dt); // Update position based on velocity
    }

        Particle& operator=(const Particle& other) {
        if (this != &other) {
            // Perform member-wise assignment here
            // Example:
            // someMember = other.someMember;
        }
        return *this;
    }

    void invertVelocity() {
        velocity = -velocity;
    }

    ImVec4 ImVec4PointerColor() {
        sf::Color sfColor = getFillColor();
        ImVec4 color = ImVec4(sfColor.r / 255.f, sfColor.g / 255.f, sfColor.b / 255.f, sfColor.a / 255.f);
        return color;
    }

private:
    sf::Vector2f velocity;
    const float gravity = 9.8f; // Gravity force
};

class MousePointer {
public:
    MousePointer(sf::Color color, sf::Color outlineColor) {
        shape.setOutlineColor(outlineColor);
        shape.setOutlineThickness(2.f);
        shape.setFillColor(color);
        shape.setOrigin(shape.getRadius(), shape.getRadius()); // Set origin to center of circle
    }

    void drawTo(sf::RenderWindow& window) {
        window.draw(shape);
    }

    void updateMousePosition(sf::Vector2i mousePos) {
        shape.setPosition(static_cast<float>(mousePos.x), static_cast<float>(mousePos.y));
    }

    ImVec4 ImVec4PointerColor() {
        sf::Color sfColor = shape.getFillColor();
        ImVec4 color = ImVec4(sfColor.r / 255.f, sfColor.g / 255.f, sfColor.b / 255.f, sfColor.a / 255.f);
        return color;
    }

    void setMousePointerColor(ImVec4 color) {
        // Convert the color from ImGui to SFML
        std::cout << "Color: " << color.x << ", " << color.y << ", " << color.z << ", " << color.w << std::endl;
        sf::Color sfColor = sf::Color(color.x * 255, color.y * 255, color.z * 255, color.w * 255);
        shape.setFillColor(sfColor);
    }

private:
    sf::CircleShape shape{ 10.f };
};

// On resize, update the view to the new size
void resizeView(const sf::RenderWindow& window, sf::View& view) {
    view.setSize(static_cast<float>(window.getSize().x), static_cast<float>(window.getSize().y));
    view.setCenter(static_cast<float>(window.getSize().x) / 2.f, static_cast<float>(window.getSize().y) / 2.f);
    dimRatio[0] = window.getSize().x;
    dimRatio[1] = window.getSize().y;
}

ImVec4 fromSfColor(sf::Color color) {
    return ImVec4(color.r / 255.f, color.g / 255.f, color.b / 255.f, color.a / 255.f);
}



int main() {
    unsigned int width = dimRatio[0];
    unsigned int height = dimRatio[1];
    sf::VideoMode desktop = sf::VideoMode::getDesktopMode();
    width = desktop.width;
    height = desktop.height;
    dimRatio[0] = width;
    dimRatio[1] = height;
    auto window = sf::RenderWindow{
        desktop,
        "SFML test",
        sf::Style::Fullscreen,
        sf::ContextSettings{ 32, 8, 16 }
    };

    std::vector<Particle> particles;
    particles.reserve(maxParticles);

    MousePointer mousePointer(sf::Color(151, 167, 198, 0), sf::Color::White);

    window.setVerticalSyncEnabled(true);
    window.setMouseCursorVisible(false);

    sf::View view = window.getDefaultView();


    // Initialize ImGui, but make sure it doesn't turn on the mouse cursor
    // Ignore *.ini file for ImGui settings
    ImGui::SFML::Init(window);

    // Disable ImGui mouse cursor
    ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange;

    // Ignore *.ini file for ImGui settings
    ImGui::GetIO().IniFilename = nullptr;

    // Move the ImGui window to the top left corner
    ImGui::GetStyle().WindowRounding = 0.f;
    // Make the SFML window fullscreen

    auto updateParticles = [&particles] {
        while (true) {
            for (auto& particle : particles) {
                particle.update(0.1f);
            }
            // Adjust the sleep duration to control the update rate
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            if (particles.empty()) {
                break;
            }
        }
    };

//        Start the thread for updating particles
    std::thread updateThread(updateParticles);
    sf::Clock deltaClock;
    float currentTime = 0;
    ImVec4 particleColour = fromSfColor(sf::Color(255, 213, 78, 255));
    while (window.isOpen()) {
        currentTime = deltaClock.restart().asSeconds();
        for (auto event = sf::Event{}; window.pollEvent(event);) {
            ImGui::SFML::ProcessEvent(event);
            if (event.type == sf::Event::Closed) {
                window.close();
            }

            else if (event.type == sf::Event::MouseButtonPressed) {
                if (event.mouseButton.button == sf::Mouse::Right) {
                    particles.clear();
                }
            }

            else if (event.type == sf::Event::Resized) {
                resizeView(window, view);
                window.setView(view);
                width = dimRatio[0];
                height = dimRatio[1];

            }

            else if (event.type == sf::Event::KeyPressed) {
                if (event.key.code == sf::Keyboard::Escape) {
                    window.close();
                }
            }
        }
        window.clear(sf::Color(35, 39, 46));

        // If debug mode is enabled, dont check for IsWindowFocused()
        // Otherwise, ImGui will crash the program when the mouse is clicked (for some reason)

#ifdef DEBUG
        if (sf::Mouse::isButtonPressed(sf::Mouse::Left) && particles.size() < 1000) {
#else

        if (sf::Mouse::isButtonPressed(sf::Mouse::Left) && particles.size() < 1000 && ImGui::IsWindowFocused()) {

#endif
            sf::Vector2i mousePos = sf::Mouse::getPosition(window);
            Particle particle(5.f, sf::Vector2f(mousePos), sf::Vector2f(0.f, 0.f));
            particle.setFillColor(sf::Color(particleColour.x * 255, particleColour.y * 255, particleColour.z * 255, particleColour.w * 255));
            // No need to push_back() here, since we reserved enough space in the vector
            particles.emplace_back(particle);
            if (particles.size() > maxParticles) {
                while (particles.size() > maxParticles) {
                    particles.pop_back();
                }
            }

        }

        ImGui::SFML::Update(window, deltaClock.getElapsedTime());

        for (auto& particle : particles) {
            particle.update(0.1f);

            // Check collision with window borders
            if (particle.getPosition().y <= 0 || particle.getPosition().y >= height) {
                particle.invertVelocity();
            }
        }

        for (auto& particle : particles) {
            window.draw(particle);
        }


        // Set the window position to the top left corner
        ImGui::SetNextWindowPos(ImVec2(0, 0));
        ImGui::Begin("Particle Simulation", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse);
        ImGui::Text("Number of particles: %zu   ", particles.size());
        ImGui::Text("Window size: %d x %d", width, height);
        ImGui::Text("Mouse position: %d, %d", sf::Mouse::getPosition(window).x, sf::Mouse::getPosition(window).y);
        ImGui::Text("FPS: %.1f", 1.f / currentTime);
        ImGui::Spacing();
        ImGui::Spacing();
        // Add a spacer
         ImGui::SetColorEditOptions(ImGuiColorEditFlags_NoInputs);
        // Enable Alpha channel

        ImGui::SetColorEditOptions(ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_NoInputs);
        if (ImGui::ColorEdit4("Particle Colour",(float*)&particleColour)) {
            for (auto& particle : particles) {
                particle.setFillColor(sf::Color(particleColour.x * 255, particleColour.y * 255, particleColour.z * 255, particleColour.w * 255));
            }
        }

        ImGui::PushItemWidth(100.0f);
        if (ImGui::SliderInt("Max particles", &maxParticles, 0, 10000, "%d")) {
            particles.reserve(maxParticles);
        }
        ImGui::Spacing();
        ImGui::Spacing();
        ImGui::Text("Left click to create particles");
        ImGui::Text("Right click to clear all particles");
        ImGui::Text("Press ESC to exit");


        ImGui::End();

        ImGui::SetNextWindowPos(ImVec2(0, 0));

        ImGui::ShowDemoWindow();

        ImGui::SFML::Render(window);

        mousePointer.updateMousePosition(sf::Mouse::getPosition(window));
        mousePointer.drawTo(window);
        window.display();


    }

    // Update the particles one last time before exiting
    particles.clear();

    // Join the thread before exiting
    updateThread.join();

    return 0;
}
