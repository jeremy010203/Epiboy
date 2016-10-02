//
// Created by Jeremy Lugand on 05/06/2016.
//

#ifndef LUGAND_J_EPIBOY_GPU_HH
#define LUGAND_J_EPIBOY_GPU_HH


#include <SFML/Graphics.hpp>
#include "../Cpu/Cpu.hh"

class Gpu {
public:
    static void VramInfos(Mmu mmu, Cpu cpu);
    static void initScreen(Mmu mmu, Cpu cpu);
    static void updateScreen(Mmu* mmu);
   static void step(int t, Mmu* mmu, Cpu* cpu);
    static int mode;
    static int modeclock;
    static int line;
    static int scx, scy;
    static int switchbg, bgmap ,bgtile ,switchlcd;
    static sf::RenderWindow* window;
    static sf::Event event;
    static sf::Image ImagePlan;
    static sf::Texture text;
    static sf::Sprite sprite;
    static float scale;
};


#endif //LUGAND_J_EPIBOY_GPU_HH
