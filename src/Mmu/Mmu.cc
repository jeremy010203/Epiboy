//
// Created by Jeremy Lugand on 04/06/2016.
//

#include <cassert>
#include "Mmu.hh"
#include "../Gpu/Gpu.hh"

void Mmu::loadRom(char const* filename)
{
    std::ifstream file (filename, std::ios::in | std::ios::binary | std::ios::ate);
    std::ifstream::pos_type fileSize;
    if(file.is_open())
    {
        std::cout << "OPEN!!" << std::endl;
        file.seekg(0, std::ios::end); // set the pointer to the end
        fileSize = file.tellg() ; // get the length of the file
        file.seekg(0, std::ios::beg); // set the pointer to the beginning

        len_rom = fileSize;
        rom = new uint8_t[len_rom]; //  for the '\0'
        file.read( (char*)rom, len_rom );

    }else
        std::cout << "Error opening file" << std::endl;
}

void Mmu::printRom()
{
    if (rom)
    {
        std::cout << len_rom << std::endl;
        for (int i = 0; i < len_rom; ++i) {
            if (i % 16 == 15)
                std::cout << std::hex << (int)(unsigned char)rom[i] << std::endl;
            else
                std::cout << std::hex << (int)(unsigned char)rom[i] << "::";
        }
    }else
        std::cout << "No rom loaded..." << std::endl;
}

uint16_t Mmu::readWord(int pos, int pc)
{
    return (unsigned char)readByte(pos, pc) + (((unsigned char)readByte(pos + 1, pc)) << 8);
}

uint8_t Mmu::readByte(int pos, int pc) {
    uint16_t addr = (uint16_t )pos >> 12;
    switch (addr) {
        case 0x0:
            if (biosMode) {
                if (pos < 0x0100)
                    return bios[pos];
                else if (pc == 0x100) {
                    biosMode = false;
                }
            }
            return rom[pos];
        case 0x1:
            return rom[pos];
        case 0x2:
            return rom[pos];
        case 0x3:
            return rom[pos];
        case 0x4:
            return rom[pos];
        case 0x5:
            return rom[pos];
        case 0x6:
            return rom[pos];
        case 0x7:
            return rom[pos];
        case 0xa:
        case 0xb:
            return eram[pos - 0xa000];
        case 0xc:
        case 0xd:
        case 0xe:
            return wram[pos - 0xc000];
        case 0xf: {
            if (pos == 0xffff)
                return _ie;
            if (pos == 0xff0f)
                return _if;

            if (pos <= 0xfdff)
                return wram[pos - 0xc000];

            if (pos == 0xff40)
                return (Gpu::switchbg  ? 0x01 : 0x00) |
                       (Gpu::bgmap     ? 0x08 : 0x00) |
                       (Gpu::bgtile    ? 0x10 : 0x00) |
                       (Gpu::switchlcd ? 0x80 : 0x00);

            if (pos == 0xff42)
                return (uint8_t)Gpu::scy;

            if (pos == 0xff43)
                return (uint8_t)Gpu::scx;

            if (pos == 0xff44)
                return (uint8_t)Gpu::line;

            uint16_t tmp = pos - 0xfe00;
            //Sprite table
            if (tmp <= 0x9f)
                return spritetable[tmp];

            tmp = pos - 0xff80;
            if (tmp <= 0x7e)
                return zeroram[tmp];

            tmp = pos - 0xff00;
            //io map
            if (tmp <= 0x7f)
                return iomap[tmp];
        }
        default:
            std::cout << "out of bound read..." << std::hex << (int)pos << std::endl;
            assert(false);
    }
}

void Mmu::writeByte(int pos, uint8_t b)
{
    uint16_t addr = (uint16_t )pos >> 12;
    //std::cout << std::hex << pos << std::endl;
    switch (addr) {
        case 0x0:
            break;
        case 0x1:
            break;
        case 0x2:
            break;
        case 0x3:
            break;
        case 0x4:
            break;
        case 0x5:
            break;
        case 0x6:
            break;
        case 0x7:
            break;
        case 0x8:
            vram[pos - 0x8000] = b;
            break;
        case 0x9:
            vram[pos - 0x8000] = b;
            break;
        case 0xa:
        case 0xb:
            eram[pos - 0xa000] = b;
            break;
        case 0xc:
        case 0xd:
        case 0xe:
            wram[pos - 0xc000] = b;
            break;
        case 0xf: {
            if (pos == 0xffff) {
                _ie = b;
                break;
            }
            if (pos == 0xff0f) {
                _if = b;
                break;
            }

            if (pos <= 0xfdff)
            {
                wram[pos - 0xc000] = b;
                break;
            }

            if (pos == 0xFF40) {
                Gpu::switchbg = (b & 0x01) ? 1 : 0;
                Gpu::bgmap = (b & 0x08) ? 1 : 0;
                Gpu::bgtile = (b & 0x10) ? 1 : 0;
                Gpu::switchlcd = (b & 0x80) ? 1 : 0;
                break;
            }

            // Scroll Y
            if (pos == 0xFF42)
            {
                Gpu::scy = b;
                break;
            }

            // Scroll X
            if (pos == 0xFF43)
            {
                Gpu::scx = b;
                break;
            }

            uint16_t tmp = pos - 0xfe00;
            //std::cout << std::hex << pos << std::endl;
            //Sprite table
            if (pos >= 0xfe00 && pos <= 0xfeff) {
                if (pos < 0xfea0)
                    spritetable[tmp] = b;
                //gpu update oam
                break;
            }

            tmp = pos - 0xff80;
            if (pos >= 0xff80 && pos <= 0xffff) {
                zeroram[tmp] = b;
                break;
            }

            tmp = pos - 0xff00;
            //std::cout << std::hex << tmp << std::endl;
            //io map
            if (tmp <= 0x7f) {
                iomap[tmp] = b;
                break;
            }
        }
        default:
            std::cout << "Out of bound write byte...." << std::hex << pos << std::endl;
            assert(false);

    }
}

void Mmu::writeWord(int pos, uint16_t b)
{
    writeByte(pos, (uint8_t )(b & 0xFF));
    writeByte(pos + 1, (uint8_t )((b & 0xFF00) >> 8));
}

void Mmu::printVram(int a, int b)
{
    for (int i = a; i < b; ++i) {
        std::cout << std::hex << (int)vram[i] << " ";
        if (i % 16 == 15)
            std::cout << std::endl;
    }
    std::cout << std::endl;
}