//
// Created by Jeremy Lugand on 04/06/2016.
//

#ifndef LUGAND_J_EPIBOY_CPU_HH
#define LUGAND_J_EPIBOY_CPU_HH


#include <string>
#include <iostream>
#include <fstream>
#include <cstring>
#include <cstddef>
#include <vector>
#include "../Mmu/Mmu.hh"

class Cpu {
public:
    void runRom(const char* path);
    void executeOp(uint8_t op);

    void printRegisters();

    //instructions
    void noOp();
    void loadSPd16();
    void loadHLd16();
    void loadDEd16();
    void xorVar(uint8_t var);
    void loadHLmA();
    void loadHLpA();
    void loadHLA();
    void prefixcb();
    void jrnz();
    void jrz();
    void loadEd8();
    void loadCd8();
    void loadAd8();
    void loadCA();
    void loadAL();
    void loadAB();
    void loadEA();
    void incA();
    void incB();
    void incC();
    void incE();
    void incH();
    void ldha8A();
    void loadADE();
    void callnn();
    void loadAinC();
    void loadBd8();
    void pushBC();
    void pushAF();
    void pushDE();
    void pushHL();
    void popBC();
    void popHL();
    void popDE();
    void popAF();
    void decE();
    void decD();
    void decC();
    void decB();
    void decA();
    void incHL();
    void incDE();
    void ret();
    void reti();
    void loadAE();
    void loadBA();
    void cpd8();
    void cphl();
    void loadAa16();
    void loada16A();
    void loadLd8();
    void jrr8();
    void loadHA();
    void loadAH();
    void loadDA();
    void ldhAa8();
    void subB();
    void subd8();
    void loadAC();
    void loadDd8();
    void addAHL();
    void jpa16();
    void loadBCA();
    void loadDEA();
    void addHLDE();
    void rst28();
    void rst38();
    void rst40();
    void ei();
    void di();
    void loadHLd8();
    void loadAHLp();
    void loadAHLm();
    void loadBCd16();
    void decBC();
    void orVar(uint8_t var);
    void andVar(uint8_t var);
    void retnz();
    void retz();
    void incHLpar();
    void cpl();
    void loadVar(uint8_t* var1, uint8_t* var2);
    void addAA();
    void loadEHL();
    void loadDHL();
    void jpHL();
    void jpza16();
    void loadAHL();
private:

    //Registers
    //8-bit registers
    uint8_t A = 0x0;
    uint8_t B = 0x0;
    uint8_t C = 0x0;
    uint8_t D = 0x0;
    uint8_t E = 0x0;
    uint8_t H = 0x0;
    uint8_t L = 0x0;
    uint8_t F = 0x0;

    //16-bit registers
    uint16_t pc = 0x0;
    uint16_t sp = 0x0;

    int m = 0;
    int t = 0;

    int ime = 0;

    //Clock
    int clockM = 0;
    int clockT = 0;

    //memory unit
    Mmu mmu = Mmu();

};


#endif //LUGAND_J_EPIBOY_CPU_HH
