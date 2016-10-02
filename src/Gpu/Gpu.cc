//
// Created by Jeremy Lugand on 05/06/2016.
//

#include <thread>
#include "Gpu.hh"
#include "../Cpu/Cpu.hh"

int Gpu::mode = 0;
int Gpu::modeclock = 0;
int Gpu::line = 0;
int Gpu::scx = 0;
int Gpu::scy = 0;

int Gpu::switchbg = 0;
int Gpu::bgmap = 0;
int Gpu::bgtile = 0;
int Gpu::switchlcd = 0;

sf::RenderWindow* Gpu::window;
sf::Event Gpu::event;
sf::Image Gpu::ImagePlan;
sf::Texture Gpu::text;
sf::Sprite Gpu::sprite;
float Gpu::scale = 5;

void Gpu::VramInfos(Mmu mmu, Cpu cpu)
{
    sf::RenderWindow window(sf::VideoMode(32 * 8, 32 * 8), "Vram infos");
    sf::Image ImagePlan;
    sf::Texture text;
    sf::Sprite sprite;

    ImagePlan.create(32 * 8, 32 * 8, sf::Color::White);

    int tile[32][32][64] = {0};
    for (int k = 0; k < 32 * 32; ++k) {
        for (int i = 16 * k; i < 16 * k + 16; i += 2) {
            uint8_t tmp = mmu.vram[i];
            for (int j = 0; j < 8; ++j) {
                if ((tmp >> 7) == 0)
                    tile[k % 32][k / 32][((i / 2) % 8) * 8 + j] = 0;
                else
                    tile[k % 32][k / 32][((i / 2) % 8) * 8 + j] = 1;
                tmp <<= 1;
            }
        }
    }

    for (int l = 0; l < 32 * 32; ++l) {
        for (int i = 0; i < 64; ++i) {
            if (tile[l % 32][l / 32][i] == 1) {
                ImagePlan.setPixel((l % 32) * 8 + i % 8, i / 8 + (l / 32) * 8, sf::Color::Black);
            }
        }
    }

    text.create(32 * 8, 32 * 8);
    text.update(ImagePlan);
    sprite.setTexture(text);


    window.clear(sf::Color::White);
    window.draw(sprite);
    window.display();

    while (window.isOpen()) {
        // Process events
        sf::Event event;
        while (window.pollEvent(event)) {
            // Close window: exit
            if (event.type == sf::Event::Closed)
                window.close();
        }
    }
}

void Gpu::updateScreen(Mmu* mmu) {
    ImagePlan.create(160, 144, sf::Color::White);

    //Fill tiles
    int tile[32][32][64] = {0};
    for (int k = 0; k < 32 * 32; ++k) {
        for (int i = 16 * k; i < 16 * k + 16; i += 2) {
            uint8_t tmp = mmu->vram[i];
            for (int j = 0; j < 8; ++j) {
                if ((tmp >> 7) == 0)
                    tile[k % 32][k / 32][((i / 2) % 8) * 8 + j] = 0;
                else
                    tile[k % 32][k / 32][((i / 2) % 8) * 8 + j] = 1;
                tmp <<= 1;
            }
        }
    }


    int screen[32][32][64] = {0};
    //cpu.printRegisters();
    //mmu.printVram(0x9800 - 0x8000, 0x9bff - 0x8000);

    for (int m = 0; m < 32; ++m) {
        for (int i = 0; i < 32; ++i) {
            if (mmu->vram[(0x9800 - 0x8000) + i + m * 32] != 0) {
                //std::cout << mmu.vram[(0x9800 - 0x8000) + i + m * 32] << std::endl;
                for (int j = 0; j < 8; ++j) {
                    for (int k = 0; k < 8; ++k) {
                        int nbTile = mmu->vram[(0x9800 - 0x8000) + i + m * 32];
                        screen[i][m][j * 8 + k] = tile[nbTile % 32][nbTile / 32][j * 8 + k];
                    }
                }
            }
        }
    }

    for (int l = 0; l < 32 * 32; ++l) {
        for (int i = 0; i < 64; ++i) {
            if (screen[l % 32][l / 32][i] == 1
                    && (l % 32) * 8 + i % 8 - Gpu::scx >= 0
                    && i / 8 + (l / 32) * 8 - Gpu::scy >= 0) {
                ImagePlan.setPixel((l % 32) * 8 + i % 8 - Gpu::scx, i / 8 + (l / 32) * 8 - Gpu::scy, sf::Color::Black);
            }
        }
    }

    text.update(ImagePlan);
    sprite.setTexture(text);

    window->clear(sf::Color::White);
    window->draw(sprite);

}

void Gpu::initScreen(Mmu mmu, Cpu cpu) {
    Gpu::window = new sf::RenderWindow(sf::VideoMode(160 * scale, 144 * scale), "Screen viewer");

    ImagePlan.create(160, 144, sf::Color::White);

    //Fill tiles
    int tile[32][32][64] = {0};
    for (int k = 0; k < 32 * 32; ++k) {
        for (int i = 16 * k; i < 16 * k + 16; i += 2) {
            uint8_t tmp = mmu.vram[i];
            for (int j = 0; j < 8; ++j) {
                if ((tmp >> 7) == 0)
                    tile[k % 32][k / 32][((i / 2) % 8) * 8 + j] = 0;
                else
                    tile[k % 32][k / 32][((i / 2) % 8) * 8 + j] = 1;
                tmp <<= 1;
            }
        }
    }


    int screen[32][32][64] = {0};
    //cpu.printRegisters();
    //mmu.printVram(0x9800 - 0x8000, 0x9bff - 0x8000);

    for (int m = 0; m < 32; ++m) {
        for (int i = 0; i < 32; ++i) {
            if (mmu.vram[(0x9800 - 0x8000) + i + m * 32] != 0) {
                std::cout << mmu.vram[(0x9800 - 0x8000) + i + m * 32] << std::endl;
                for (int j = 0; j < 8; ++j) {
                    for (int k = 0; k < 8; ++k) {
                        int nbTile = mmu.vram[(0x9800 - 0x8000) + i + m * 32];
                        screen[i][m][j * 8 + k] = tile[nbTile % 32][nbTile / 32][j * 8 + k];
                    }
                }
            }
        }
    }

    for (int l = 0; l < 32 * 32; ++l) {
        for (int i = 0; i < 64; ++i) {
            if (screen[l % 32][l / 32][i] == 1
                && (l % 32) * 8 + i % 8 - Gpu::scx >= 0
                && i / 8 + (l / 32) * 8 - Gpu::scy >= 0) {
                ImagePlan.setPixel((l % 32) * 8 + i % 8 - Gpu::scx, i / 8 + (l / 32) * 8 - Gpu::scy, sf::Color::Black);
            }
        }
    }

    text.create(160, 144);
    text.update(ImagePlan);

    sprite.setTexture(text);
    sprite.scale(sf::Vector2f(scale, scale));

    while (Gpu::window->pollEvent(event)) {
        // Close window: exit
        if (event.type == sf::Event::Closed)
            Gpu::window->close();
    }

    window->clear(sf::Color::White);
    window->draw(sprite);
    window->display();
}

void Gpu::step(int t, Mmu* mmu, Cpu* cpu)
{
    Gpu::modeclock += t;
    switch (mode)
    {
        case 2:
            if( Gpu::modeclock >= 80)
            {
                // Enter scanline mode 3
                Gpu::modeclock = 0;
                Gpu::mode = 3;
            }
            break;

            // VRAM read mode, scanline active
            // Treat end of mode 3 as end of scanline
        case 3:
            if( Gpu::modeclock >= 172)
            {
                // Enter hblank
                Gpu::modeclock = 0;
                Gpu::mode = 0;

                // Write a scanline to the framebuffer
                // renderscan();
            }
            break;

            // Hblank
            // After the last hblank, push the screen data to canvas
        case 0:
            if( Gpu::modeclock >= 204)
            {
                Gpu::modeclock = 0;
                Gpu::line++;

                if( Gpu::line == 143)
                {
                    // Enter vblank
                    Gpu::mode = 1;
                    //Gpu::updateScreen(mmu, cpu);
                    //std::cout << "update screen";

                    Gpu::updateScreen(mmu);
                    Gpu::window->display();
                    //std::this_thread::sleep_for(std::chrono::milliseconds(10));

                    //canvas.putImageData(GPU._scrn, 0, 0);
                }
                else
                {
                    Gpu::mode = 2;
                }
            }
            break;

            // Vblank (10 lines)
        case 1:
            if( Gpu::modeclock >= 456)
            {
                Gpu::modeclock = 0;
                Gpu::line++;

                if( Gpu::line > 153)
                {
                    // Restart scanning modes
                    Gpu::mode = 2;
                    Gpu::line = 0;
                }
            }
            break;
    }
}