#include <SFML/Graphics.hpp>
#include <optional>
#include <iostream>
#include <algorithm>

// Funzione speciale per "spezzare" un double a 64 bit in due float a 32 bit
void spezzaDouble(double valore, float& hi, float& lo) {
    hi = static_cast<float>(valore);
    lo = static_cast<float>(valore - static_cast<double>(hi));
}

int main() {
    // Rileviamo la risoluzione del monitor
    sf::VideoMode desktop = sf::VideoMode::getDesktopMode();
    const unsigned int LARGHEZZA = desktop.size.x;
    const unsigned int ALTEZZA = desktop.size.y;

    // Finestra a schermo intero
    sf::RenderWindow window(sf::VideoMode({LARGHEZZA, ALTEZZA}), "Mandelbrot GPU 64-Bit Extreme", sf::State::Fullscreen);
    window.setFramerateLimit(144); 

    sf::Shader shader;
    if (!shader.loadFromFile("mandelbrot.frag", sf::Shader::Type::Fragment)) {
        std::cerr << "Errore critico: Impossibile caricare mandelbrot.frag!\n";
        return -1;
    }

    sf::Texture texture;
    texture.resize({LARGHEZZA, ALTEZZA});
    sf::Sprite sprite(texture);

    // PARAMETRI INIZIALI IN DOUBLE (64-bit veri)
    double centroX = -0.75;
    double centroY = 0.0;
    double ampiezza = 2.7;
    int max_iterazioni = 500;

    while (window.isOpen()) {
        while (const std::optional event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) {
                window.close();
            }
            if (const auto* keyPressed = event->getIf<sf::Event::KeyPressed>()) {
                if (keyPressed->code == sf::Keyboard::Key::Escape) {
                    window.close();
                }
            }
        }

        // --- GESTIONE INPUT (Asse Y corretto per lo standard OpenGL) ---
        double spostamento = ampiezza * 0.02;

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Up))    centroY += spostamento; // Ora va SU
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Down))  centroY -= spostamento; // Ora va GIÙ
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left))  centroX -= spostamento;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right)) centroX += spostamento;
        
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W))     ampiezza *= 0.95; // Zoom IN
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S))     ampiezza *= 1.05; // Zoom OUT
        
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A))     max_iterazioni += 10;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D))     max_iterazioni = std::max(10, max_iterazioni - 10);

        // --- SCOMPOSIZIONE DEI DOUBLE PER IL VIAGGIO VERSO LA GPU ---
        float cx_hi, cx_lo;
        float cy_hi, cy_lo;
        float amp_hi, amp_lo;

        spezzaDouble(centroX, cx_hi, cx_lo);
        spezzaDouble(centroY, cy_hi, cy_lo);
        spezzaDouble(ampiezza, amp_hi, amp_lo);

        // --- INVIO PARAMETRI ---
        shader.setUniform("u_resolution", sf::Vector2f(static_cast<float>(LARGHEZZA), static_cast<float>(ALTEZZA)));
        shader.setUniform("u_max_iterazioni", max_iterazioni);
        
        // Passiamo i float accoppiati a SFML
        shader.setUniform("u_centroX_hi", cx_hi);
        shader.setUniform("u_centroX_lo", cx_lo);
        shader.setUniform("u_centroY_hi", cy_hi);
        shader.setUniform("u_centroY_lo", cy_lo);
        shader.setUniform("u_ampiezza_hi", amp_hi);
        shader.setUniform("u_ampiezza_lo", amp_lo);

        // --- RENDERING ---
        window.clear();
        window.draw(sprite, &shader);
        window.display();
    }

    return 0;
}