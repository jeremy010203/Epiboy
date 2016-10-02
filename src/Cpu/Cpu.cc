//
// Created by Jeremy Lugand on 04/06/2016.
//

#include <cassert>
#include "Cpu.hh"
#include "../Gpu/Gpu.hh"

void Cpu::runRom(const char* path)
{
    bool debug = false;

    mmu.loadRom(path);
    //mmu.printRom();

    Gpu::initScreen(mmu, *this);

    //int i = 0;
    while (Gpu::window->isOpen()) {


        /*
        if (!mmu.biosMode && pc == 0x69f0)
        {
            std::cout << std::hex << (int)mmu.readByte(sp + 1, pc) << std::endl;

            debug = true;
            printRegisters();
            Gpu::VramInfos(mmu, *this);
            //mmu.printVram(0, 0x8000);

            std::string str;
            std::cin >> str;
        }
        */




        if (debug && !mmu.biosMode) {

            printRegisters();

            std::string str;
            std::cin >> str;
        }

        char op = mmu.readByte(pc, pc);
        pc++;

        executeOp(op);



        /*
        if (B == 0xFF) {

            std::cout << "Execute op: " << std::hex << (int)op << std::endl;

            printRegisters();


            std::string rep;
            std::cin >> rep;
            if (rep != "n")
                break;
        }
        */


        clockT += t;
        clockM += m;
        Gpu::step(t, &mmu, this);

        t = 0;
        m = 0;

        if(ime && mmu._ie && mmu._if)
        {
            // Mask off ints that aren't enabled
            uint8_t ifired = mmu._ie & mmu._if;

            if(ifired & 0x01)
            {
                mmu._if &= (255 - 0x01);
                rst40();
            }
        }

        clockT += t;
        clockM += m;
    }
}

void Cpu::loadSPd16()
{
    sp = mmu.readWord(pc, pc);
    pc += 2;

    m = 3;
    t = 12;
}

void Cpu::loadCd8()
{
    C = mmu.readByte(pc, pc);
    pc++;

    m = 2;
    t = 8;
}

void Cpu::xorVar(uint8_t var)
{
    A = (A ^ var);

    F = 0x0;
    if (A == 0)
        F |= 0b10000000;

    m = 1;
    t = 4;
}

void Cpu::loadBCd16()
{
    C = mmu.readByte(pc, pc);
    pc++;
    B = mmu.readByte(pc, pc);
    pc++;

    m = 3;
    t = 12;
}

void Cpu::loadHLd16()
{
    L = mmu.readByte(pc, pc);
    pc++;
    H = mmu.readByte(pc, pc);
    pc++;

    m = 3;
    t = 12;
}

void Cpu::loadDEd16()
{
    E = mmu.readByte(pc, pc);
    pc++;
    D = mmu.readByte(pc, pc);
    pc++;

    m = 3;
    t = 12;
}

void Cpu::loadHLmA()
{
    uint16_t HL = H;
    HL = (HL << 8);
    HL += L;

    mmu.writeByte(HL, A);
    HL--;

    L = (uint8_t)(HL & 0x00FF);
    H = (uint8_t)(HL >> 8);

    m = 1;
    t = 8;
}

void Cpu::loadHLd8()
{
    uint16_t HL = H;
    HL = (HL << 8);
    HL += L;

    mmu.writeByte(HL, mmu.readByte(pc, pc));
    pc++;

    L = (uint8_t)(HL & 0x00FF);
    H = (uint8_t)(HL >> 8);

    m = 2;
    t = 12;
}

void Cpu::loadHLpA()
{
    uint16_t HL = H;
    HL = (HL << 8);
    HL += L;

    mmu.writeByte(HL, A);
    HL++;

    L = (uint8_t )(HL & 0x00FF);
    H = (uint8_t )(HL >> 8);

    m = 1;
    t = 8;
}

void Cpu::incHL()
{
    uint16_t HL = H;
    HL = (HL << 8);
    HL += L;

    HL++;

    L = (uint8_t )(HL & 0x00FF);
    H = (uint8_t )(HL >> 8);

    m = 1;
    t = 8;
}

void Cpu::incHLpar()
{
    uint16_t HL = H;
    HL = (HL << 8);
    HL += L;

    uint8_t val = mmu.readByte(HL, pc);
    val++;
    mmu.writeByte(HL, val);

    F &= 0b00011111;
    if (val == 0)
        F |= 0b10000000;
    if ((val & 0x000f) % 16 == 0)
        F |= 0b00100000;

    m = 1;
    t = 12;
}

void Cpu::incDE()
{
    uint16_t DE = D;
    DE = (DE << 8);
    DE += E;

    DE++;

    E = (uint8_t )(DE & 0x00FF);
    D = (uint8_t )(DE >> 8);

    m = 1;
    t = 8;
}

void Cpu::loadHLA()
{
    uint16_t HL = H;
    HL = (HL << 8);
    HL += L;

    mmu.writeByte(HL, A);

    L = (uint8_t )(HL & 0x00FF);
    H = (uint8_t )(HL >> 8);

    m = 1;
    t = 8;
}

void Cpu::loadCA()
{
    mmu.writeByte(0xFF00 + C, A);

    m = 2;
    t = 8;
}

void Cpu::loadVar(uint8_t* var1, uint8_t* var2)
{
    *var1 = *var2;

    m = 1;
    t = 4;
}

void Cpu::prefixcb()
{
    uint8_t code = mmu.readByte(pc, pc);
    pc++;
    switch (code)
    {
        //BIT 7, H
        case 0x7C:
        {
            if ((H & 0b10000000) == 0x0)
                //bit 7 equal to 0 set
                F |= 0b10100000;
            else {
                F &= 0b00111111;
                F |= 0b00100000;
                //bit 7 equal to 1 cleared
            }

            m = 2;
            t = 8;

            break;
        }
        //RL C
        case 0x11:
        {
            uint16_t tmp = 0;
            tmp += (uint16_t )(F & 0b00010000);
            tmp = tmp << 4;
            tmp += C;

            tmp = (uint16_t)(((tmp << 1)|(tmp >> 8)) & 0x1FF);

            C = (uint8_t)(tmp & 0xFF);
            F = (uint8_t)((tmp & 0xFF00) >> 4);

            m = 2;
            t = 8;

            break;
        }
        //SWAP A
        case 0x37:
        {
            A = ((A << 4) + (A >> 4));

            F = 0;
            if (A == 0)
                F |= 0b10000000;

            m = 2;
            t = 8;
            break;
        }
        //RES 0, A
        case 0x87:
        {
            A &= ~(0x1 << 0);
            break;
        }
        default:
            mmu.printVram(0, 4* 256);
            printRegisters();
            std::cout << "Unknown prefix CB instr: " << std::hex << (int)code << std::endl;
            assert(false);
    }
}

void Cpu::incH()
{
    H++;

    F &= 0b10111111;
    if ((H & 0x000f) == 0x0)
        F |= 0b10000000;
    else
        F |= ~0b10000000;

    if ((H & 0x000f) % 16 == 0)
        F |= 0b00100000;
    else
        F |= ~0b00100000;

    m = 1;
    t = 4;
}

void Cpu::incE()
{
    E++;

    F &= 0b10111111;
    if ((E & 0x000f) == 0x0)
        F |= 0b10000000;
    else
        F |= ~0b10000000;

    if ((E & 0x000f) % 16 == 0)
        F |= 0b00100000;
    else
        F |= ~0b00100000;

    m = 1;
    t = 4;
}

void Cpu::incC()
{
    C++;

    F &= 0b10111111;
    if ((C & 0x000f) == 0x0)
        F |= 0b10000000;
    else
        F |= ~0b10000000;

    if ((C & 0x000f) % 16 == 0)
        F |= 0b00100000;
    else
        F |= ~0b00100000;

    m = 1;
    t = 4;
}

void Cpu::incB()
{
    B++;

    F &= 0b10111111;
    if ((B & 0x000f) == 0x0)
        F |= 0b10000000;
    else
        F |= ~0b10000000;

    if ((B & 0x000f) % 16 == 0)
        F |= 0b00100000;
    else
        F |= ~0b00100000;

    m = 1;
    t = 4;
}

void Cpu::incA()
{
    A++;

    F &= 0b10111111;
    if ((A & 0x000f) == 0x0)
        F |= 0b10000000;
    else
        F |= ~0b10000000;

    if ((A & 0x000f) % 16 == 0)
        F |= 0b00100000;
    else
        F |= ~0b00100000;

    m = 1;
    t = 4;
}

void Cpu::jrnz()
{
    //not zero condition met -> let's jump
    if ((F & 0b10000000) == 0x0)
    {
        int8_t tmp = mmu.readByte(pc, pc);
        pc++;
        pc += tmp;
        m = 2;
        t = 12;
    }
    else
    {
        pc++;
        m = 2;
        t = 8;
    }
}

void Cpu::jrz()
{
    //zero condition met -> let's jump
    if ((F & 0b10000000) != 0x0)
    {
        int8_t tmp = mmu.readByte(pc, pc);
        pc++;
        pc += tmp;
        m = 2;
        t = 12;
    }
    else
    {
        pc++;
        m = 2;
        t = 8;
    }
}

void Cpu::loadADE()
{
    uint16_t DE = D;
    DE = (DE << 8);
    DE += E;

    A = mmu.readByte(DE, pc);

    m = 1;
    t = 8;
}

void Cpu::ldha8A()
{
    mmu.writeByte(0xFF00 + mmu.readByte(pc, pc), A);
    pc++;

    m = 2;
    t = 12;
}

void Cpu::callnn()
{
    uint16_t word = mmu.readWord(pc, pc);
    pc += 2;

    mmu.writeWord(sp - 2, pc);
    sp -= 2;

    pc = word;

    m = 5;
    t = 20;
}

void Cpu::printRegisters()
{
    std::cout << "<---- Registers ---->" << std::endl;
    std::cout << std::hex << "A: " << (int)A << std::endl;
    std::cout << std::hex << "B: " << (int)B << std::endl;
    std::cout << std::hex << "C: " << (int)C << std::endl;
    std::cout << std::hex << "D: " << (int)D << std::endl;
    std::cout << std::hex << "E: " << (int)E << std::endl;
    std::cout << std::hex << "H: " << (int)H << std::endl;
    std::cout << std::hex << "L: " << (int)L << std::endl;
    std::cout << std::hex << "F: " << (int)F << std::endl;
    std::cout << std::hex << "Sp: " << sp << std::endl;
    std::cout << std::hex << "Pc: " << pc << std::endl;
    std::cout << "<---- /Registers ---->" << std::endl;
}

void Cpu::loadAinC()
{
    C = A;

    m = 1;
    t = 4;
}

void Cpu::loadBd8()
{
    B = mmu.readByte(pc, pc);
    pc++;

    m = 2;
    t = 8;
}

void Cpu::loadEd8()
{
    E = mmu.readByte(pc, pc);
    pc++;

    m = 2;
    t = 8;
}

void Cpu::pushAF()
{
    sp--;
    mmu.writeByte(sp, A);
    sp--;
    mmu.writeByte(sp, F);

    m = 1;
    t = 16;
}

void Cpu::pushDE()
{
    sp--;
    mmu.writeByte(sp, D);
    sp--;
    mmu.writeByte(sp, E);

    m = 1;
    t = 16;
}

void Cpu::pushHL()
{
    sp--;
    mmu.writeByte(sp, H);
    sp--;
    mmu.writeByte(sp, L);

    m = 1;
    t = 16;
}

void Cpu::pushBC()
{
    sp--;
    mmu.writeByte(sp, B);
    sp--;
    mmu.writeByte(sp, C);

    m = 1;
    t = 16;
}

void Cpu::popBC()
{
    C = mmu.readByte(sp, pc);
    sp++;
    B = mmu.readByte(sp, pc);
    sp++;

    m = 1;
    t = 12;
}

void Cpu::popHL()
{
    L = mmu.readByte(sp, pc);
    sp++;
    H = mmu.readByte(sp, pc);
    sp++;

    m = 1;
    t = 12;
}

void Cpu::popDE()
{
    E = mmu.readByte(sp, pc);
    sp++;
    D = mmu.readByte(sp, pc);
    sp++;

    m = 1;
    t = 12;
}

void Cpu::popAF()
{
    F = mmu.readByte(sp, pc);
    sp++;
    A = mmu.readByte(sp, pc);
    sp++;

    m = 1;
    t = 12;
}

void Cpu::decE()
{
    E--;

    F |= 0b01000000;
    if (E == 0x0)
        F |= 0b10000000;
    else
        F &= 0b01111111;
    if (E % 16 == 15)
        F |= 0b00100000;
    else
        F &= 0b11011111;

    m = 1;
    t = 4;
}

void Cpu::decD()
{
    D--;

    F |= 0b01000000;
    if (D == 0x0)
        F |= 0b10000000;
    else
        F &= 0b01111111;
    if (D % 16 == 15)
        F |= 0b00100000;
    else
        F &= 0b11011111;

    m = 1;
    t = 4;
}

void Cpu::decC()
{
    C--;

    F |= 0b01000000;
    if (C == 0x0)
        F |= 0b10000000;
    else
        F &= 0b01111111;
    if (C % 16 == 15)
        F |= 0b00100000;
    else
        F &= 0b11011111;

    m = 1;
    t = 4;
}

void Cpu::decB()
{
    B--;

    F |= 0b01000000;
    if (B == 0x0)
        F |= 0b10000000;
    else
        F &= 0b01111111;
    if (B % 16 == 15)
        F |= 0b00100000;
    else
        F &= 0b11011111;

    m = 1;
    t = 4;
}

void Cpu::decA()
{
    A--;

    F |= 0b01000000;
    if (A == 0x0)
        F |= 0b10000000;
    else
        F &= 0b01111111;
    if (A % 16 == 15)
        F |= 0b00100000;
    else
        F &= 0b11011111;

    m = 1;
    t = 4;
}

void Cpu::ret()
{
    pc = mmu.readWord(sp, pc);
    sp += 2;

    m = 1;
    t = 16;
}

void Cpu::reti()
{
    pc = mmu.readWord(sp, pc);
    sp += 2;

    ime = 1;

    m = 1;
    t = 16;
}

void Cpu::retnz()
{
    if ((F & 0b10000000) == 0)
    {
        pc = mmu.readWord(sp, pc);
        sp += 2;

        m = 1;
        t = 20;
        return;
    }

    m = 1;
    t = 8;
}

void Cpu::retz()
{
    if ((F & 0b10000000) != 0)
    {
        pc = mmu.readWord(sp, pc);
        sp += 2;

        m = 1;
        t = 20;
        return;
    }

    m = 1;
    t = 8;
}

void Cpu::loadAE()
{
    A = E;

    m = 1;
    t = 4;
}

void Cpu::loadBA()
{
    B = A;

    m = 1;
    t = 4;
}

void Cpu::loadEA()
{
    E = A;

    m = 1;
    t = 4;
}

void Cpu::loadAL()
{
    A = L;

    m = 1;
    t = 4;
}

void Cpu::loadAB()
{
    A = B;

    m = 1;
    t = 4;
}

void Cpu::loadHA()
{
    H = A;

    m = 1;
    t = 4;
}

void Cpu::loadDA()
{
    D = A;

    m = 1;
    t = 4;
}

void Cpu::loadAH()
{
    A = H;

    m = 1;
    t = 4;
}

void Cpu::cpd8()
{
    uint8_t tmp = mmu.readByte(pc, pc);
    pc++;

    F = 0b01000000;
    if (A == tmp)
        F |= 0b10000000;

    if ((((A & 0x000f) - (tmp & 0x000f)) & 0x10) == 0x10)
        F |= 0b00100000;

    if (tmp > A)
        F |= 0b00010000;

    m = 1;
    t = 4;
}

void Cpu::cphl()
{
    uint16_t HL = H;
    HL = (HL << 8);
    HL += L;

    uint8_t tmp = mmu.readByte(HL, pc);

    F = 0b01000000;
    if (A == tmp)
        F |= 0b10000000;

    if ((((A & 0x000f) - (tmp & 0x000f)) & 0x10) == 0x10)
        F |= 0b00100000;

    if (tmp > A)
        F |= 0b00010000;

    m = 1;
    t = 4;
}

void Cpu::loadAa16()
{
    A = mmu.readByte(mmu.readWord(pc, pc), pc);
    pc += 2;

    m = 3;
    t = 16;
}

void Cpu::loada16A()
{
    uint16_t addr = mmu.readWord(pc, pc);
    pc += 2;

    mmu.writeByte(addr, A);

    m = 3;
    t = 16;
}

void Cpu::loadLd8()
{
    L = mmu.readByte(pc, pc);
    pc++;

    m = 2;
    t = 8;
}

void Cpu::jrr8()
{
    int8_t tmp = mmu.readByte(pc, pc);
    pc++;
    pc += tmp;

    m = 2;
    t = 12;
}

void Cpu::ldhAa8()
{

    uint8_t tmp = mmu.readByte(pc, pc);
    uint8_t tmp2 = mmu.readByte(0xff00 + tmp, pc);
    pc++;

    A = tmp2;

    m = 2;
    t = 12;
}

void Cpu::subB()
{
    F = 0b01000000;
    if (A == B)
        F |= 0b10000000;

    if ((((A & 0x000f) - (B & 0x000f)) & 0x10) == 0x10)
        F |= 0b00100000;

    if (B > A)
        F |= 0b00010000;

    A -= B;

    m = 1;
    t = 4;
}

void Cpu::subd8()
{
    uint8_t d8 = mmu.readByte(pc, pc);
    pc++;

    F = 0b01000000;
    if (A == d8)
        F |= 0b10000000;

    if ((((A & 0x000f) - (d8 & 0x000f)) & 0x10) == 0x10)
        F |= 0b00100000;

    if (d8 > A)
        F |= 0b00010000;

    A -= d8;

    m = 2;
    t = 8;
}

void Cpu::loadAC()
{
    A = mmu.readByte(0xff00 + C, pc);

    m = 2;
    t = 8;
}

void Cpu::loadAd8()
{
    A = mmu.readByte(pc, pc);
    pc++;

    m = 2;
    t = 8;
}

void Cpu::loadDd8()
{
    D = mmu.readByte(pc, pc);
    pc++;

    m = 2;
    t = 8;
}

void Cpu::addAHL()
{
    uint16_t HL = H;
    HL = (HL << 8);
    HL += L;

    uint8_t val = mmu.readByte(HL, pc);

    F = 0;
    if ((((A & 0x000f) + (val & 0x000f)) & 0x10) == 0x10)
        F |= 0b00100000;
    if ((int)A + (int)val > 255)
        F |= 0b00010000;

    A += val;

    if (A == 0x0)
        F|= 0b10000000;

    m = 1;
    t = 8;
}

void Cpu::addAA()
{

    F = 0;
    if ((((A & 0x000f) + (A & 0x000f)) & 0x10) == 0x10)
        F |= 0b00100000;
    if ((int)A + (int)A > 255)
        F |= 0b00010000;

    A += A;

    F = 0;
    if (A == 0x0)
        F|= 0b10000000;

    m = 1;
    t = 4;
}

void Cpu::noOp()
{
    m = 1;
    t = 4;
}

void Cpu::jpa16()
{
    pc = mmu.readWord(pc, pc);

    m = 3;
    t = 12;
}

void Cpu::jpza16()
{
    if ((F & 0b10000000) != 0)
    {
        pc = mmu.readWord(pc, pc);

        m = 3;
        t = 16;
        return;
    }

    pc += 2;

    m = 3;
    t = 12;
}

void Cpu::cpl()
{
    A = (uint8_t )(A ^ 0xFF);
    F |= 0b01100000;

    m = 1;
    t = 4;
}

void Cpu::loadBCA()
{
    uint16_t BC = B;
    BC = BC << 8;
    BC += C;

    mmu.writeByte(BC, A);

    m = 1;
    t = 8;
}

void Cpu::loadDEA()
{
    uint16_t DE = D;
    DE = DE << 8;
    DE += E;

    mmu.writeByte(DE, A);

    m = 1;
    t = 8;
}

void Cpu::addHLDE()
{
    uint16_t HL = H;
    HL = HL << 8;
    HL += L;

    uint16_t DE = D;
    DE = DE << 8;
    DE += E;

    F &= 0b10111111;
    if ((((HL & 0x000f) + (DE & 0x000f)) & 0x10) == 0x10)
        F |= 0b00100000;
    if ((int)HL + (int)DE > 255)
        F |= 0b00010000;

    HL += DE;

    L = (uint8_t )(HL & 0x00FF);
    H = (uint8_t )(HL >> 8);

    m = 1;
    t = 8;

}

void Cpu::rst28()
{

    sp -= 2;
    mmu.writeWord(sp, pc);

    pc = 0x28;

    m = 1;
    t = 16;
}

void Cpu::rst38()
{

    sp -= 2;
    mmu.writeWord(sp, pc);

    pc = 0x38;

    m = 1;
    t = 16;
}

void Cpu::rst40()
{

    sp -= 2;
    mmu.writeWord(sp, pc);

    pc = 0x40;

    m = 1;
    t = 16;
}

void Cpu::di()
{
    ime = 0;
    m = 1;
    t = 4;
}

void Cpu::ei()
{
    ime = 1;
    m = 1;
    t = 4;
}

void Cpu::loadAHLp()
{
    uint16_t HL = H;
    HL = HL << 8;
    HL += L;

    A = mmu.readByte(HL, pc);
    HL++;

    L = (uint8_t )(HL & 0x00FF);
    H = (uint8_t )(HL >> 8);

    m = 1;
    t = 8;
}

void Cpu::loadAHL()
{
    uint16_t HL = H;
    HL = HL << 8;
    HL += L;

    A = mmu.readByte(HL, pc);

    m = 1;
    t = 8;
}

void Cpu::loadAHLm()
{
    uint16_t HL = H;
    HL = HL << 8;
    HL += L;

    A = mmu.readByte(HL, pc);
    HL--;

    L = (uint8_t )(HL & 0x00FF);
    H = (uint8_t )(HL >> 8);

    m = 1;
    t = 8;
}

void Cpu::decBC()
{
    uint16_t BC = B;
    BC = (BC << 8);
    BC += C;

    BC--;

    C = (uint8_t )(BC & 0x00FF);
    B = (uint8_t )(BC >> 8);

    m = 1;
    t = 8;
}

void Cpu::orVar(uint8_t var)
{
    A |= var;

    F = 0;
    if (A == 0)
        F |= 0b10000000;

    m = 1;
    t = 4;
}

void Cpu::andVar(uint8_t var)
{
    A &= var;

    F = 0b00100000;
    if (A == 0)
        F |= 0b10000000;

    m = 1;
    t = 4;
}

void Cpu::loadEHL()
{
    uint16_t HL = H;
    HL = HL << 8;
    HL += L;

    E = mmu.readByte(HL, pc);

    m = 1;
    t = 8;
}

void Cpu::loadDHL()
{
    uint16_t HL = H;
    HL = HL << 8;
    HL += L;

    D = mmu.readByte(HL, pc);

    m = 1;
    t = 8;
}

void Cpu::jpHL()
{
    uint16_t HL = H;
    HL = HL << 8;
    HL += L;

    pc = HL;

    m = 1;
    t = 4;
}
void Cpu::executeOp(uint8_t op)
{
    switch (op)
    {
        case 0x0:
            noOp();
            break;
        case 0x01:
            loadBCd16();
            break;
        case 0x31:
            loadSPd16();
            break;
        case 0xaf:
            xorVar(A);
            break;
        case 0xad:
            xorVar(L);
            break;
        case 0xac:
            xorVar(H);
            break;
        case 0xab:
            xorVar(E);
            break;
        case 0xaa:
            xorVar(D);
            break;
        case 0xa9:
            xorVar(C);
            break;
        case 0xa8:
            xorVar(B);
            break;
        case 0x21:
            loadHLd16();
            break;
        case 0x11:
            loadDEd16();
            break;
        case 0x32:
            loadHLmA();
            break;
        case 0xcb:
            prefixcb();
            break;
        case 0x20:
            jrnz();
            break;
        case 0x28:
            jrz();
            break;
        case 0x0E:
            loadCd8();
            break;
        case 0x3E:
            loadAd8();
            break;
        case 0xe2:
            loadCA();
            break;
        case 0x04:
            incB();
            break;
        case 0x0C:
            incC();
            break;
        case 0x77:
            loadHLA();
            break;
        case 0xe0:
            ldha8A();
            break;
        case 0x1a:
            loadADE();
            break;
        case 0xcd:
            callnn();
            break;
        case 0x4f:
            loadAinC();
            break;
        case 0x06:
            loadBd8();
            break;
        case 0xc5:
            pushBC();
            break;
        case 0xc1:
            popBC();
            break;
        case 0xe1:
            popHL();
            break;
        case 0xd1:
            popDE();
            break;
        case 0xf1:
            popAF();
            break;
        case 0x24:
            incH();
            break;
        case 0x1d:
            decE();
            break;
        case 0x15:
            decD();
            break;
        case 0x0d:
            decC();
            break;
        case 0x05:
            decB();
            break;
        case 0x3d:
            decA();
            break;
        case 0x22:
            loadHLpA();
            break;
        case 0x23:
            incHL();
            break;
        case 0x13:
            incDE();
            break;
        case 0xc9:
            ret();
            break;
        case 0x7c:
            loadAH();
            break;
        case 0x7b:
            loadAE();
            break;
        case 0x7d:
            loadAL();
            break;
        case 0x78:
            loadAB();
            break;
        case 0xfe:
            cpd8();
            break;
        case 0xbe:
            cphl();
            break;
        case 0xea:
            loada16A();
            break;
        case 0x1e:
            loadEd8();
            break;
        case 0x2e:
            loadLd8();
            break;
        case 0x18:
            jrr8();
            break;
        case 0x67:
            loadHA();
            break;
        case 0x57:
            loadDA();
            break;
        case 0xf0:
            ldhAa8();
            break;
        case 0x90:
            subB();
            break;
        case 0xf2:
            loadAC();
            break;
        case 0x16:
            loadDd8();
            break;
        case 0x86:
            addAHL();
            break;
        case 0xc3:
            jpa16();
            break;
        case 0x02:
            loadBCA();
            break;
        case 0xd6:
            subd8();
            break;
        case 0x5f:
            loadEA();
            break;
        case 0x19:
            addHLDE();
            break;
        case 0xef:
            rst28();
            break;
        case 0xff:
            rst38();
            break;
        case 0xf3:
            di();
            break;
        case 0xfb:
            ei();
            break;
        case 0x36:
            loadHLd8();
            break;
        case 0x2a:
            loadAHLp();
            break;
        case 0x3a:
            loadAHLm();
            break;
        case 0x0b:
            decBC();
            break;
        case 0xb7:
            orVar(A);
            break;
        case 0xb0:
            orVar(B);
            break;
        case 0xb1:
            orVar(C);
            break;
        case 0xb2:
            orVar(D);
            break;
        case 0xb3:
            orVar(E);
            break;
        case 0xb4:
            orVar(H);
            break;
        case 0xb5:
            orVar(L);
            break;
        case 0xf5:
            pushAF();
            break;
        case 0xe5:
            pushHL();
            break;
        case 0xd5:
            pushDE();
            break;
        case 0xa7:
            andVar(A);
            break;
        case 0xa5:
            andVar(L);
            break;
        case 0xa4:
            andVar(H);
            break;
        case 0xa3:
            andVar(E);
            break;
        case 0xa2:
            andVar(D);
            break;
        case 0xa1:
            andVar(C);
            break;
        case 0xa0:
            andVar(B);
            break;
        case 0xe6:
            andVar(mmu.readByte(pc, pc));
            pc++;
            break;
        case 0xc0:
            retnz();
            break;
        case 0xc8:
            retz();
            break;
        case 0xd9:
            reti();
            break;
        case 0xfa:
            loadAa16();
            break;
        case 0x34:
            incHLpar();
            break;
        case 0x3c:
            incA();
            break;
        case 0x2f:
            cpl();
            break;
        case 0x47:
            loadBA();
            break;
        case 0x79:
            loadVar(&A, &C);
            break;
        case 0x87:
            addAA();
            break;
        case 0x5e:
            loadEHL();
            break;
        case 0x56:
            loadDHL();
            break;
        case 0xe9:
            jpHL();
            break;
        case 0x12:
            loadDEA();
            break;
        case 0x1c:
            incE();
            break;
        case 0xca:
            jpza16();
            break;
        case 0x7e:
            loadAHL();
            break;
        case 0x17: {

            uint16_t tmp = 0;
            tmp += (F & 0b00010000);
            tmp = tmp << 4;
            tmp += A;

            tmp = (uint16_t) (((tmp << 1) | (tmp >> 8)) & 0x1FF);

            A = (uint8_t) (tmp & 0x00FF);
            F = (uint8_t) ((tmp & 0xFF00) >> 4);

            m = 1;
            t = 4;
            break;
        }
        default:
            //mmu.printVram(0, 4* 256);
            //printRegisters();
            std::cout << "Unknown op hex: " << std::hex << (int)(unsigned char)op;
            std::cout << "( dec:" << std::dec << (int)op << ") pc: " << std::hex << (int)pc << std::endl;
            assert(false);
    }
}