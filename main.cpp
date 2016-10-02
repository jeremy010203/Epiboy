#include <iostream>
#include "src/Cpu/Cpu.hh"

int main() {

    Cpu cpu = Cpu();
    cpu.runRom("/Users/jeremy/Documents/Epita/lugand_j-Epiboy/rom/tetris.gb");

    return 0;
}