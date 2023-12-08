#include <SFML/Graphics.hpp>
#include <iostream>
#include <vector>
#include <thread>

const int dimRatio[] = { 1920, 1080 };

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

private:
    sf::CircleShape shape{ 10.f };
};

bool checkCollision(const Particle& p1, const Particle& p2) {
    float dx = p1.getPosition().x - p2.getPosition().x;
    float dy = p1.getPosition().y - p2.getPosition().y;
    float distance = std::sqrt(dx * dx + dy * dy);
    return distance <= (p1.getRadius() + p2.getRadius());
}


int main() {
    unsigned int width = dimRatio[0] / 2;
    unsigned int height = dimRatio[1] / 2;
    auto window = sf::RenderWindow{
        { width, height },
        "SFML test",
        sf::Style::Titlebar | sf::Style::Close,
        sf::ContextSettings{ 32, 8, 16 }
    };

    std::vector<Particle> particles;
    MousePointer mousePointer(sf::Color(151, 167, 198), sf::Color::White);

    window.setVerticalSyncEnabled(true);
    window.setMouseCursorVisible(false);

    auto updateParticles = [&particles] {
        while (true) {
            for (auto& particle : particles) {
                particle.update(0.1f);
            }
            // Adjust the sleep duration to control the update rate
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    };

//        Start the thread for updating particles
    std::thread updateThread(updateParticles);

    while (window.isOpen()) {
        for (auto event = sf::Event{}; window.pollEvent(event);) {
            if (event.type == sf::Event::Closed) {
                window.close();
            }

            if (event.type == sf::Event::MouseButtonPressed) {
                if (event.mouseButton.button == sf::Mouse::Right) {
                    particles.clear();
                }
            }
        }

        window.clear(sf::Color(35, 39, 46));

        if (sf::Mouse::isButtonPressed(sf::Mouse::Left)) {
            sf::Vector2i mousePos = sf::Mouse::getPosition(window);
            Particle particle(5.f, sf::Vector2f(mousePos), sf::Vector2f(0.f, 0.f));
            particles.push_back(particle);
            std::cout << "Particle created at: " << mousePos.x << ", " << mousePos.y << std::endl;
            std::cout << "Size: " << particles.size() << std::endl;
        }

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

        mousePointer.updateMousePosition(sf::Mouse::getPosition(window));
        mousePointer.drawTo(window);
        window.display();
    }

        updateThread.join();

    return 0;
}
