#include <SFML/Graphics.hpp>
#include <iostream>
#include <vector>
#include <thread>
#include <mutex>

const int dimRatio[] = { 800, 600 };

class Particle : public sf::CircleShape {
public:
    Particle(float radius, sf::Vector2f position, sf::Vector2f velocity)
        : sf::CircleShape(radius), velocity(velocity) {
        setPosition(position);
        setFillColor(sf::Color(255, 213, 78)); // Set the particle color
        setOrigin(radius, radius); // Set origin to center of circle
    }

    void update(float dt) {
        velocity.y += gravity * dt; // Apply gravity
        move(velocity * dt); // Update position based on velocity
    }

    void invertVelocityIfColliding(const std::vector<Particle>& particles, size_t index) {
        for (size_t i = 0; i < particles.size(); ++i) {
            if (i != index && checkCollision(particles[i])) {
                velocity = -velocity;
                break;
            }
        }
    }

private:
    bool checkCollision(const Particle& other) const {
        float dx = getPosition().x - other.getPosition().x;
        float dy = getPosition().y - other.getPosition().y;
        float distance = std::sqrt(dx * dx + dy * dy);
        return distance <= (getRadius() + other.getRadius());
    }

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

private:
    sf::CircleShape shape{ 10.f };
};

int main() {
    unsigned int width = dimRatio[0];
    unsigned int height = dimRatio[1];
    auto window = sf::RenderWindow{
        { width, height },
        "Particle Simulation",
        sf::Style::Titlebar | sf::Style::Close,
        sf::ContextSettings{ 32, 8, 16 }
    };

    std::vector<Particle> particles;
    std::mutex particlesMutex; // Mutex for synchronizing access to particles vector

    MousePointer mousePointer(sf::Color(151, 167, 198), sf::Color::White);

    // Function to update particle positions and handle collisions
    auto updateParticles = [&particles, &particlesMutex] {
        while (true) {
            for (auto& particle : particles) {
                particle.update(0.1f);
            }

            for (size_t i = 0; i < particles.size(); ++i) {
                particles[i].invertVelocityIfColliding(particles, i);
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(10)); // Adjust sleep duration as needed
        }
    };

    // Start the thread for updating particles
    std::thread updateThread(updateParticles);

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
            }
        }

        if (sf::Mouse::isButtonPressed(sf::Mouse::Left)) {
            sf::Vector2i mousePos = sf::Mouse::getPosition(window);
            Particle particle(5.f, sf::Vector2f(mousePos), sf::Vector2f(0.f, 0.f));

            // Lock access to particles vector before modification
            std::lock_guard<std::mutex> lock(particlesMutex);
            particles.push_back(particle);
        }

        window.clear(sf::Color(35, 39, 46));

        // Draw particles in the main thread
        {
            std::lock_guard<std::mutex> lock(particlesMutex); // Lock access to particles vector
            for (auto& particle : particles) {
                window.draw(particle);
            }
        }

        mousePointer.updateMousePosition(sf::Mouse::getPosition(window));
        mousePointer.drawTo(window);

        window.display();
    }

    // Join the thread before exiting
    updateThread.join();

    return 0;
}
