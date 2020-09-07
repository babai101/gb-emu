#include "include/cpu.hpp"
#include "include/ppu.hpp"
#include <array>
#include <cstdint>
#include <iostream>
#include <fstream>

namespace CPU
{
    u8 memory[65536];
    u8 A, B, C, D, E, F, H, L, opcode;
    bool IME = false;
    u8 *a = &A;
    u8 *b = &B;
    u8 *c = &C;
    u8 *d = &D;
    u8 *e = &E;
    u8 *h = &H;
    u8 *l = &L;
    std::string gb_type = "DMG";
    u16 PC, SP;
    std::array<u8, 8> rst_addrs = {
        {0x00, 0x08, 0x10, 0x18, 0x20, 0x28, 0x30, 0x38}};
    int cycles = 0;
    int cycles_this_frame = 0;
    const int CYCLES_PER_FRAME = 70224;
    bool error_occurred = false;
    void reset()
    {
        for (std::size_t i = 0; i < 65536; i++)
            memory[i] = 0;
        seed();
    }
    void seed()
    {
        PC = 0x100;
        A = 0x00;

        if (gb_type == "DMG")
        {
            A = 0x01;
        }
        else if (gb_type == "CGB")
        {
            A = 0x11;
        }
        else if (gb_type == "GBP")
        {
            A = 0xFF;
        }

        // A = 0x11; //debug

        F = 0xB0;
        B = 0x00;
        C = 0x13;
        D = 0x00;
        E = 0xD8;
        H = 0x01;
        L = 0x4D;
        SP = 0xFFFE;

        memory[0xFF05] = 0x00; // TIMA
        memory[0xFF06] = 0x00; // TMA
        memory[0xFF07] = 0x00; // TAC
        memory[0xFF10] = 0x80; // NR10
        memory[0xFF11] = 0xBF; // NR11
        memory[0xFF12] = 0xF3; // NR12
        memory[0xFF14] = 0xBF; // NR14
        memory[0xFF16] = 0x3F; // NR21
        memory[0xFF17] = 0x00; // NR22
        memory[0xFF19] = 0xBF; // NR24
        memory[0xFF1A] = 0x7F; // NR30
        memory[0xFF1B] = 0xFF; // NR31
        memory[0xFF1C] = 0x9F; // NR32
        memory[0xFF1E] = 0xBF; // NR33
        memory[0xFF20] = 0xFF; // NR41
        memory[0xFF21] = 0x00; // NR42
        memory[0xFF22] = 0x00; // NR43
        memory[0xFF23] = 0xBF; // NR30
        memory[0xFF24] = 0x77; // NR50
        memory[0xFF25] = 0xF3; // NR51
        if (gb_type == "DMG")
        {
            memory[0xFF26] = 0xF1; // NR52
        }
        else if (gb_type == "CGB")
        {
            memory[0xFF26] = 0xF0; // NR52
        }
        memory[0xFF40] = 0x91; // LCDC
        memory[0xFF42] = 0x00; // SCY
        memory[0xFF43] = 0x00; // SCX
        memory[0xFF45] = 0x00; // LYC
        memory[0xFF47] = 0xFC; // BGP
        memory[0xFF48] = 0xFF; // OBP0
        memory[0xFF49] = 0xFF; // OBP1
        memory[0xFF4A] = 0x00; // WY
        memory[0xFF4B] = 0x00; // WX
        memory[0xFFFF] = 0x00; // IE
    }
    u8 read_memory(u16 addr)
    {
        // TODO: Mirroring logic and events
        return memory[addr];
    }
    void write_memory(u16 addr, u8 val)
    {
        // if (addr >= 0x8000 && addr <= 0x9FFF && (memory[LCDC] & 0x80) == 0x80)
        // {
        // std::cout << "debug" << std::endl;
        // return;
        // }
        // if (addr == 0xd803)
        // {
        //     std::cout << "debug" << std::endl;
        // }
        // TODO: Mirroring logic and events
        memory[addr] = val;
    }
    u16 to_u16(u8 lsb, u8 msb)
    {
        u16 temp = (msb << 8) | lsb;
        return temp;
    }
    u8 msb(u16 val) { return val >> 8; }
    u8 lsb(u16 val) { return val & 0xFF; }
    u8 *get_reg(u8 val)
    {
        switch (val)
        {
        case 0b111:
            return a;
        case 0b000:
            return b;
        case 0b001:
            return c;
        case 0b010:
            return d;
        case 0b011:
            return e;
        case 0b100:
            return h;
        case 0b101:
            return l;
        default:
            std::cout << "Error! Incorrect register to fetch!: " << val
                      << std::endl;
            return a;
        }
    }
    bool check_flag(enum flags f)
    {
        switch (f)
        {
        case zero:
            if ((F >> 7) == 0x01)
                return true;
            else
                return false;
        case subtract:
            if (((F >> 6) & 0x01) == 0x01)
                return true;
            else
                return false;
        case half_carry:
            if (((F >> 5) & 0x01) == 0x01)
                return true;
            else
                return false;
        case carry:
            if (((F >> 4) & 0x01) == 0x01)
                return true;
            else
                return false;
        }
        return true; // stub
    }
    void set_flag(enum flags f)
    {
        switch (f)
        {
        case zero:
            F = F & 0b01111111;
            F = F | 0b10000000;
            break;
        case subtract:
            F = F & 0b10111111;
            F = F | 0b01000000;
            break;
        case half_carry:
            F = F & 0b11011111;
            F = F | 0b00100000;
            break;
        case carry:
            F = F & 0b11101111;
            F = F | 0b00010000;
            break;
        }
    }
    void reset_flag(enum flags f)
    {
        switch (f)
        {
        case zero:
            F = F & 0b01111111;
            break;
        case subtract:
            F = F & 0b10111111;
            break;
        case half_carry:
            F = F & 0b11011111;
            break;
        case carry:
            F = F & 0b11101111;
            break;
        }
    }
    void set_reset_half_carry8(u8 a, u8 b)
    {
        u8 sum = (a & 0x0F) + (b & 0x0F);
        if ((sum & 0x10) == 0x10)
            set_flag(half_carry);
        else
            reset_flag(half_carry);
    }
    void set_reset_half_carry16(u16 a, u16 b)
    {
        u16 sum = (a & 0x0FFF) + (b & 0x0FFF);
        if ((sum & 0X1000) == 0x1000)
            set_flag(half_carry);
        else
            reset_flag(half_carry);
    }
    void set_reset_half_borrow8(u8 a, u8 b)
    {
        a = (a & 0x0F) | 0x10;
        u8 diff = a - (b & 0x0F);
        if ((diff & 0x10) != 0x10)
            set_flag(half_carry);
        else
            reset_flag(half_carry);
    }
    void set_reset_zero(u8 a)
    {
        if (a == 0)
            set_flag(zero);
        else
            reset_flag(zero);
    }
    void set_reset_carry8(u8 a, u8 b)
    {
        if ((a + b) > 0xFF)
            set_flag(carry);
        else
            reset_flag(carry);
    }
    void set_reset_carry16(u16 a, u16 b)
    {
        if ((a + b) > 0xFFFF)
            set_flag(carry);
        else
            reset_flag(carry);
    }
    void set_reset_borrow8(u8 a, u8 b)
    {
        if (b > a)
            set_flag(carry);
        else
            reset_flag(carry);
    }
    void add(u8 *dest, u8 src)
    {
        set_reset_half_carry8(*dest, src);
        set_reset_carry8(*dest, src);
        *a += src;
        set_reset_zero(A);
    }
    u16 add16(u16 dest, u16 src)
    {
        set_reset_half_carry16(dest, src);
        set_reset_carry16(dest, src);
        reset_flag(subtract);
        dest += src;
        return dest;
    }
    void addc(u8 *dest, u8 src)
    {
        u8 temp = 0;
        if (check_flag(carry))
            temp = 1;
        else
            temp = 0;
        set_reset_half_carry8(*dest, src + temp);
        set_reset_carry8(*dest, src + temp);
        *a += (src + temp);
        set_reset_zero(A);
    }
    void sub(u8 *dest, u8 src)
    {
        set_reset_half_borrow8(*dest, src);
        set_reset_borrow8(*dest, src);
        *a -= src;
        set_reset_zero(A);
    }
    void subc(u8 *dest, u8 src)
    {
        u8 temp = 0;
        if (check_flag(carry))
            temp = 1;
        else
            temp = 0;
        set_reset_half_borrow8(*dest, src - temp);
        set_reset_borrow8(*dest, src - temp);
        *a -= (src - temp);
        set_reset_zero(A);
    }
    void and8(u8 operand)
    {
        reset_flag(subtract);
        reset_flag(carry);
        set_flag(half_carry);
        A &= operand;
        set_reset_zero(A);
    }
    void or8(u8 operand)
    {
        reset_flag(subtract);
        reset_flag(half_carry);
        reset_flag(carry);
        A |= operand;
        set_reset_zero(A);
    }
    void xor8(u8 operand)
    {
        reset_flag(subtract);
        reset_flag(half_carry);
        reset_flag(carry);
        A |= operand;
        set_reset_zero(A);
    }
    void cp8(u8 operand)
    {
        set_flag(subtract);
        set_reset_half_borrow8(A, operand);
        set_reset_borrow8(A, operand);
        if (A == operand)
            set_reset_zero(0);
        // set_reset_zero(A);
    }
    u8 inc8(u8 val)
    {
        reset_flag(subtract);
        set_reset_half_carry8(val, 1);
        set_reset_zero(val + 1);
        return val + 1;
    }
    u8 dec8(u8 val)
    {
        set_flag(subtract);
        set_reset_half_borrow8(val, 1);
        set_reset_zero(val - 1);
        return val - 1;
    }
    u8 rotate_left_carry(u8 reg)
    {
        u8 output;
        if ((reg & 0x80) == 0x80)
        {
            output = ((reg << 1) & 0xFF) | 0x01;
            set_flag(carry);
        }
        else
        {
            output = (reg << 1) & 0xFF;
            reset_flag(carry);
        }
        reset_flag(half_carry);
        reset_flag(subtract);
        reset_flag(zero);
        return output;
    }
    u8 rotate_right_carry(u8 reg)
    {
        u8 output;
        if ((reg & 0x01) == 0x01)
        {
            output = (reg >> 1) | 0x80;
            set_flag(carry);
        }
        else
        {
            output = reg >> 1;
            reset_flag(carry);
        }
        reset_flag(half_carry);
        reset_flag(subtract);
        reset_flag(zero);
        return output;
    }
    u8 rotate_left(u8 reg)
    {
        u8 output;
        bool old_carry = check_flag(carry);
        if ((reg & 0x80) == 0x80)
            set_flag(carry);
        else
            reset_flag(carry);
        output = (reg << 1) & 0xFF;
        if (old_carry)
            output = output | 0x01;
        reset_flag(half_carry);
        reset_flag(subtract);
        reset_flag(zero);
        return output;
    }
    u8 rotate_right(u8 reg)
    {
        u8 output;
        bool old_carry = check_flag(carry);
        if ((reg & 0x01) == 0x01)
            set_flag(carry);
        else
            reset_flag(carry);
        output = reg >> 1;
        if (old_carry)
            output = output | 0x80;
        reset_flag(half_carry);
        reset_flag(subtract);
        reset_flag(zero);
        return output;
    }
    u8 shift_left(u8 reg)
    {
        u8 output;
        if ((reg & 0x80) == 0x80)
            set_flag(carry);
        else
            reset_flag(carry);
        output = (reg << 1) & 0xFF;
        reset_flag(subtract);
        reset_flag(half_carry);
        if (output == 0)
            set_flag(zero);
        else
            reset_flag(zero);
        return output;
    }
    u8 shift_right(u8 reg, bool change_msg)
    {
        u8 output;
        u8 old_msb = reg & 0x80;
        if ((reg & 0x01) == 0x01)
            set_flag(carry);
        else
            reset_flag(carry);
        output = reg >> 1;
        reset_flag(subtract);
        reset_flag(half_carry);
        if (!change_msg)
            output = output | old_msb;
        if (output == 0)
            set_flag(zero);
        else
            reset_flag(zero);
        return output;
    }
    u8 swap(u8 reg)
    {
        u8 lsbit = reg & 0x0F;
        u8 msbit = reg & 0xF0;
        u8 output = ((lsbit << 4) | (msbit >> 4)) & 0xFF;
        reset_flag(carry);
        reset_flag(half_carry);
        reset_flag(subtract);
        if (output == 0)
            set_flag(zero);
        else
            reset_flag(zero);
        return output;
    }
    void bit_test(u8 reg, u8 bit)
    {
        if (((reg >> bit) & 0x01) == 0x01)
            set_flag(zero);
        else
            reset_flag(zero);
        set_flag(half_carry);
        reset_flag(subtract);
    }
    u8 bit_set(u8 reg, u8 bit)
    {
        // u8 mask = 0x01 << bit;
        // if (((reg >> bit) & 0x01) == 0x00)
        //     reg = reg | mask;
        switch (bit)
        {
        case 0:
            reg &= 0xFE;
            reg |= 0x01;
            break;
        case 1:
            reg &= 0xFD;
            reg |= 0x02;
            break;
        case 2:
            reg &= 0xFB;
            reg |= 0x04;
            break;
        case 3:
            reg &= 0xF7;
            reg |= 0x08;
            break;
        case 4:
            reg &= 0xEF;
            reg |= 0x10;
            break;
        case 5:
            reg &= 0xDF;
            reg |= 0x20;
            break;
        case 6:
            reg &= 0xBF;
            reg |= 0x40;
            break;
        case 7:
            reg &= 0x7F;
            reg |= 0x80;
        default:
            break;
        }
        return reg;
    }
    u8 bit_reset(u8 reg, u8 bit)
    {
        // for (int i = 0; i <= bit; i++)
        // {
        //     reg = reg << 1;
        //     if (i > 0)
        //         reg = reg | 0x01;
        // }
        switch (bit)
        {
        case 0:
            reg &= 0xFE;
            break;
        case 1:
            reg &= 0xFD;
            break;
        case 2:
            reg &= 0xFB;
            break;
        case 3:
            reg &= 0xF7;
            break;
        case 4:
            reg &= 0xEF;
            break;
        case 5:
            reg &= 0xDF;
            break;
        case 6:
            reg &= 0xBF;
            break;
        case 7:
            reg &= 0x7F;
        default:
            break;
        }

        return reg;
    }
    void jp_nn()
    {
        u8 lsb = read_memory(PC++);
        u8 msb = read_memory(PC++);
        PC = to_u16(lsb, msb);
        cycles = 4;
    }
    void jp_hl()
    {
        PC = to_u16(L, H);
        cycles = 1;
    }
    void jp_cc_nn()
    {
        u8 lsb = read_memory(PC++);
        u8 msb = read_memory(PC++);
        bool result = false;
        switch (opcode)
        {
        case 0xC2:
            result = !check_flag(zero);
            break;
        case 0xCA:
            result = check_flag(zero);
            break;
        case 0xD2:
            result = !check_flag(carry);
            break;
        case 0xDA:
            result = check_flag(carry);
            break;
        }
        cycles = 3;
        if (result)
        {
            PC = to_u16(lsb, msb);
            cycles++;
        }
    }
    void jr_r()
    {
        s8 r = read_memory(PC++);
        PC += r;
        cycles = 3;
    }
    void jr_cc_r()
    {
        s8 r = read_memory(PC++);
        bool result = false;
        ;
        switch (opcode)
        {
        case 0x20:
            result = !check_flag(zero);
            break;
        case 0x28:
            result = check_flag(zero);
            break;
        case 0x30:
            result = !check_flag(carry);
            break;
        case 0x38:
            result = check_flag(carry);
            break;
        }
        cycles = 2;
        if (result)
        {
            PC += r;
            cycles++;
        }
    }
    void call_nn()
    {
        u8 addr_lsb = read_memory(PC++);
        u8 addr_msb = read_memory(PC++);
        u16 nn = to_u16(addr_lsb, addr_msb);
        write_memory(--SP, msb(PC));
        write_memory(--SP, lsb(PC));
        PC = nn;
        cycles = 6;
    }
    void call_cc_nn()
    {
        u8 addr_lsb = read_memory(PC++);
        u8 addr_msb = read_memory(PC++);
        u16 nn = to_u16(addr_lsb, addr_msb);
        bool result = false;
        switch (opcode)
        {
        case 0xC4:
            result = !check_flag(zero);
            break;
        case 0xCC:
            result = check_flag(zero);
            break;
        case 0xD4:
            result = !check_flag(carry);
            break;
        case 0xDC:
            result = check_flag(carry);
            break;
        }
        cycles = 3;
        if (result)
        {
            write_memory(--SP, msb(PC));
            write_memory(--SP, lsb(PC));
            PC = nn;
            cycles += 3;
        }
    }
    void ret()
    {
        u8 lsb = read_memory(SP++);
        u8 msb = read_memory(SP++);
        PC = to_u16(lsb, msb);
        cycles = 4;
    }
    void ret_cc()
    {
        bool result = false;
        switch (opcode)
        {
        case 0xC0:
            result = !check_flag(zero);
            break;
        case 0xC8:
            result = check_flag(zero);
            break;
        case 0xD0:
            result = !check_flag(carry);
            break;
        case 0xD8:
            result = check_flag(carry);
            break;
        }
        cycles = 2;
        if (result)
        {
            u8 ret_lsb = read_memory(SP++);
            u8 ret_msb = read_memory(SP++);
            PC = to_u16(ret_lsb, ret_msb);
            cycles += 3;
        }
    }
    void reti()
    {
        u8 ret_lsb = read_memory(SP++);
        u8 ret_msb = read_memory(SP++);
        PC = to_u16(ret_lsb, ret_msb);
        IME = true;
        write_memory(IE, 0x1F);
        cycles = 4;
    }
    void rst_n()
    {
        u8 rst_addr = (opcode >> 3) & 0x07;
        rst_addr = rst_addrs[rst_addr];
        write_memory(--SP, msb(PC));
        write_memory(--SP, lsb(PC));
        PC = to_u16(rst_addr, 0x00);
        cycles = 4;
    }
    void ld_r_r()
    {
        u8 *r1 = get_reg((opcode >> 3) & 0x07);
        u8 *r2 = get_reg(opcode & 0x07);
        *r1 = *r2;
        cycles = 1;
    }
    void ld_r_n()
    {
        u8 *r = get_reg((opcode >> 3) & 0x07);
        *r = read_memory(PC++);
        cycles = 2;
    }
    void ld_r_hl()
    {
        // if (PC == 0xc8d0)
        // {
        //     std::cout << "Debug" << std::endl;
        // }
        u8 *r = get_reg((opcode >> 3) & 0x07);
        *r = read_memory(to_u16(L, H));
        cycles = 2;
    }
    void ld_hl_r()
    {
        // if (opcode == 0x77 && H == 0xd8)
        // {
        //     std::cout << "Debug" << std::endl;
        // }
        u8 *r = get_reg(opcode & 0x07);
        write_memory(to_u16(L, H), *r);
        cycles = 2;
    }
    void ld_hl_n()
    {
        u8 n = read_memory(PC++);
        write_memory(to_u16(L, H), n);
        cycles = 3;
    }
    void ld_a_bc()
    {
        A = read_memory(to_u16(C, B));
        cycles = 2;
    }
    void ld_a_de()
    {
        A = read_memory(to_u16(E, D));
        cycles = 2;
    }
    void ld_a_c()
    {
        A = read_memory(0xFF00 + read_memory(C));
        cycles = 2;
    }
    void ld_c_a()
    {
        write_memory((0xFF00 + C), A);
        cycles = 2;
    }
    void ld_a_n()
    {
        A = read_memory(0xFF00 + read_memory(PC++));
        cycles = 3;
    }
    void ld_n_a()
    {
        write_memory(0xFF00 + read_memory(PC++), A);
        cycles = 3;
    }
    void ld_a_nn()
    {
        u8 lsb = read_memory(PC++);
        u8 msb = read_memory(PC++);
        A = read_memory(to_u16(lsb, msb));
        cycles = 4;
    }
    void ld_nn_a()
    {
        u8 lsb = read_memory(PC++);
        u8 msb = read_memory(PC++);
        // if ((msb == 216) && (lsb == 3))
        // {
        //     std::cout << "debug" << std::endl;
        // }
        write_memory(to_u16(lsb, msb), A);
        cycles = 4;
    }
    void ld_a_hli()
    {
        u16 addr = to_u16(L, H);
        A = read_memory(addr++);
        L = lsb(addr);
        H = msb(addr);
        cycles = 2;
    }
    void ld_a_hld()
    {
        u16 addr = to_u16(L, H);
        A = read_memory(addr--);
        L = lsb(addr);
        H = msb(addr);
        cycles = 2;
    }
    void ld_bc_a()
    {
        write_memory(to_u16(C, B), A);
        cycles = 2;
    }
    void ld_de_a()
    {
        write_memory(to_u16(E, D), A);
        cycles = 2;
    }
    void ld_hli_a()
    {
        u16 addr = to_u16(L, H);
        write_memory(addr++, A);
        L = lsb(addr);
        H = msb(addr);
        cycles = 2;
    }
    void ld_hld_a()
    {
        u16 addr = to_u16(L, H);
        write_memory(addr--, A);
        L = lsb(addr);
        H = msb(addr);
        cycles = 2;
    }
    void ld_dd_nn()
    {
        u8 lsb = read_memory(PC++);
        u8 msb = read_memory(PC++);
        switch (opcode)
        {
        case 0x01:
            B = msb, C = lsb;
            break;
        case 0x11:
            D = msb, E = lsb;
            break;
        case 0x21:
            H = msb, L = lsb;
            break;
        case 0x31:
            SP = to_u16(lsb, msb);
            break;
        }
        cycles = 3;
    }
    void ld_sp_hl()
    {
        SP = to_u16(L, H);
        cycles = 2;
    }
    void push_qq()
    {
        u8 lsb, msb;
        switch (opcode)
        {
        case 0xF5:
            lsb = F, msb = A;
            break;
        case 0xC5:
            lsb = C, msb = B;
            break;
        case 0xD5:
            lsb = E, msb = D;
            break;
        case 0xE5:
            lsb = L, msb = H;
            break;
        }
        write_memory(--SP, msb);
        write_memory(--SP, lsb);
        cycles = 4;
    }
    void pop_qq()
    {
        u8 lsb = read_memory(SP++);
        u8 msb = read_memory(SP++);
        switch (opcode)
        {
        case 0xF1:
            A = msb;
            F &= 0x0F;
            lsb &= 0xF0;
            F |= lsb;
            break;
        case 0xC1:
            B = msb, C = lsb;
            break;
        case 0xD1:
            D = msb, E = lsb;
            break;
        case 0xE1:
            H = msb, L = lsb;
            break;
        }
        cycles = 3;
    }
    void ldhl_sp_e()
    {
        s8 e = read_memory(PC++);
        H = msb(SP + e);
        L = lsb(SP + e);
        reset_flag(zero);
        reset_flag(subtract);
        if ((SP + e) > 0xFFFF)
            set_flag(carry);
        else
            reset_flag(carry);
        set_reset_half_carry16(SP, e);
        cycles = 3;
    }
    void ld_nn_sp()
    {
        u8 lsb_addr = read_memory(PC++);
        u8 msb_addr = read_memory(PC++);
        u16 addr = to_u16(lsb_addr, msb_addr);
        write_memory(addr, lsb(SP));
        write_memory(addr + 1, msb(SP));
        cycles = 5;
    }
    void add_a_r()
    {
        u8 *r = get_reg(opcode & 0x07);
        add(a, *r);
        reset_flag(subtract);
        cycles = 1;
    }
    void add_a_n()
    {
        u8 n = read_memory(PC++);
        add(a, n);
        reset_flag(subtract);
        cycles = 2;
    }
    void add_a_hl()
    {
        add(a, read_memory(to_u16(L, H)));
        reset_flag(subtract);
        cycles = 2;
    }
    void adc_a_r()
    {
        u8 *r = get_reg(opcode & 0x07);
        addc(a, *r);
        reset_flag(subtract);
        cycles = 1;
    }
    void adc_a_n()
    {
        addc(a, read_memory(PC++));
        reset_flag(subtract);
        cycles = 2;
    }
    void adc_a_hl()
    {
        addc(a, read_memory(to_u16(L, H)));
        reset_flag(subtract);
        cycles = 2;
    }
    void sub_r()
    {
        u8 *r = get_reg(opcode & 0x07);
        sub(a, *r);
        set_flag(subtract);
        cycles = 1;
    }
    void sub_n()
    {
        sub(a, read_memory(PC++));
        set_flag(subtract);
        cycles = 2;
    }
    void sub_hl()
    {
        sub(a, read_memory(to_u16(L, H)));
        set_flag(subtract);
        cycles = 2;
    }
    void sbc_a_r()
    {
        u8 *r = get_reg(opcode & 0x07);
        subc(a, *r);
        set_flag(subtract);
        cycles = 1;
    }
    void sbc_a_n()
    {
        subc(a, read_memory(PC++));
        set_flag(subtract);
        cycles = 2;
    }
    void sbc_a_hl()
    {
        subc(a, read_memory(to_u16(L, H)));
        set_flag(subtract);
        cycles = 2;
    }
    void and_r()
    {
        u8 *r = get_reg(opcode & 0x07);
        and8(*r);
        cycles = 1;
    }
    void and_n()
    {
        and8(read_memory(PC++));
        cycles = 2;
    }
    void and_hl()
    {
        and8(read_memory(to_u16(L, H)));
        cycles = 2;
    }
    void or_r()
    {
        u8 *r = get_reg(opcode & 0x07);
        or8(*r);
        cycles = 1;
    }
    void or_n()
    {
        or8(read_memory(PC++));
        cycles = 2;
    }
    void or_hl()
    {
        or8(read_memory(to_u16(L, H)));
        cycles = 2;
    }
    void xor_r()
    {
        u8 *r = get_reg(opcode & 0x07);
        xor8(*r);
        cycles = 1;
    }
    void xor_n()
    {
        xor8(read_memory(PC++));
        cycles = 2;
    }
    void xor_hl()
    {
        xor8(read_memory(to_u16(L, H)));
        cycles = 2;
    }
    void cp_r()
    {
        u8 *r = get_reg(opcode & 0x07);
        cp8(*r);
        cycles = 1;
    }
    void cp_n()
    {
        cp8(read_memory(PC++));
        cycles = 2;
    }
    void cp_hl()
    {
        cp8(read_memory(to_u16(L, H)));
        cycles = 2;
    }
    void inc_r()
    {
        u8 *r = get_reg((opcode >> 3) & 0x07);
        *r = inc8(*r);
        cycles = 1;
    }
    void inc_hl()
    {
        write_memory(to_u16(L, H), inc8(read_memory(to_u16(L, H))));
        cycles = 3;
    }
    void dec_r()
    {
        u8 *r = get_reg((opcode >> 3) & 0x07);
        *r = dec8(*r);
        cycles = 1;
    }
    void dec_hl()
    {
        write_memory(to_u16(L, H), dec8(read_memory(to_u16(L, H))));
        cycles = 3;
    }
    void add_hl_ss()
    {
        u8 msb_reg, lsb_reg;
        switch (opcode)
        {
        case 0x09:
            msb_reg = B, lsb_reg = C;
            break;
        case 0x19:
            msb_reg = D, lsb_reg = E;
            break;
        case 0x29:
            msb_reg = H, lsb_reg = L;
            break;
        case 0x39:
            msb_reg = msb(SP), lsb_reg = lsb(SP);
            break;
        }
        u16 temp = add16(to_u16(L, H), to_u16(lsb_reg, msb_reg));
        H = msb(temp);
        L = lsb(temp);
        cycles = 2;
    }
    void add_sp_e()
    {
        SP = add16(SP, to_u16(read_memory(PC++), 0x00));
        reset_flag(zero);
        cycles = 4;
    }
    void inc_ss()
    {
        u8 msb_reg, lsb_reg;
        switch (opcode)
        {
        case 0x03:
            msb_reg = B, lsb_reg = C;
            break;
        case 0x13:
            msb_reg = D, lsb_reg = E;
            break;
        case 0x23:
            msb_reg = H, lsb_reg = L;
            break;
        case 0x33:
            msb_reg = msb(SP), lsb_reg = lsb(SP);
            break;
        }
        u16 temp = to_u16(lsb_reg, msb_reg) + 1;
        msb_reg = msb(temp), lsb_reg = lsb(temp);
        switch (opcode)
        {
        case 0x03:
            B = msb_reg, C = lsb_reg;
            break;
        case 0x13:
            D = msb_reg, E = lsb_reg;
            break;
        case 0x23:
            H = msb_reg, L = lsb_reg;
            break;
        case 0x33:
            SP = temp;
            break;
        }
        cycles = 2;
    }
    void dec_ss()
    {
        u8 msb_reg, lsb_reg;
        switch (opcode)
        {
        case 0x0B:
            msb_reg = B, lsb_reg = C;
            break;
        case 0x1B:
            msb_reg = D, lsb_reg = E;
            break;
        case 0x2B:
            msb_reg = H, lsb_reg = L;
            break;
        case 0x3B:
            msb_reg = msb(SP), lsb_reg = lsb(SP);
            break;
        }
        u16 temp = to_u16(lsb_reg, msb_reg) - 1;
        msb_reg = msb(temp), lsb_reg = lsb(temp);
        switch (opcode)
        {
        case 0x0B:
            B = msb_reg, C = lsb_reg;
            break;
        case 0x1B:
            D = msb_reg, E = lsb_reg;
            break;
        case 0x2B:
            H = msb_reg, L = lsb_reg;
            break;
        case 0x3B:
            SP = temp;
            break;
        }
        cycles = 2;
    }
    void rlca()
    {
        A = rotate_left_carry(A);
        cycles = 1;
    }
    void rla()
    {
        A = rotate_left(A);
        cycles = 1;
    }
    void rrca()
    {
        A = rotate_right_carry(A);
        cycles = 1;
    }
    void rra()
    {
        A = rotate_right(A);
        cycles = 1;
    }
    void rlc_r()
    {
        u8 *r = get_reg(opcode & 0x07);
        *r = rotate_left_carry(*r);
        cycles = 2;
    }
    void rlc_hl()
    {
        u16 addr = to_u16(L, H);
        write_memory(addr, rotate_left_carry(read_memory(addr)));
        cycles = 4;
    }
    void rl_r()
    {
        u8 *r = get_reg(opcode & 0x07);
        *r = rotate_left(*r);
        cycles = 2;
    }
    void rl_hl()
    {
        u16 addr = to_u16(L, H);
        write_memory(addr, rotate_left(read_memory(addr)));
        cycles = 4;
    }
    void rrc_r()
    {
        u8 *r = get_reg(opcode & 0x07);
        *r = rotate_right_carry(*r);
        cycles = 2;
    }
    void rrc_hl()
    {
        u16 addr = to_u16(L, H);
        write_memory(addr, rotate_right_carry(read_memory(addr)));
        cycles = 4;
    }
    void rr_r()
    {
        u8 *r = get_reg(opcode & 0x07);
        *r = rotate_right(*r);
        cycles = 2;
    }
    void rr_hl()
    {
        u16 addr = to_u16(L, H);
        write_memory(addr, rotate_right(read_memory(addr)));
        cycles = 4;
    }
    void sla_r()
    {
        u8 *r = get_reg(opcode & 0x07);
        *r = shift_left(*r);
        cycles = 2;
    }
    void sla_hl()
    {
        u16 addr = to_u16(L, H);
        write_memory(addr, shift_left(read_memory(addr)));
        cycles = 4;
    }
    void sra_r()
    {
        u8 *r = get_reg(opcode & 0x07);
        *r = shift_right(*r, false);
        cycles = 2;
    }
    void sra_hl()
    {
        u16 addr = to_u16(L, H);
        write_memory(addr, shift_right(read_memory(addr), false));
        cycles = 4;
    }
    void srl_r()
    {
        u8 *r = get_reg(opcode & 0x07);
        *r = shift_right(*r, true);
        cycles = 2;
    }
    void srl_hl()
    {
        u16 addr = to_u16(L, H);
        write_memory(addr, shift_right(read_memory(addr), true));
        cycles = 4;
    }
    void swap_r()
    {
        u8 *r = get_reg(opcode & 0x07);
        *r = swap(*r);
        cycles = 2;
    }
    void swap_hl()
    {
        u16 addr = to_u16(L, H);
        write_memory(addr, swap(read_memory(addr)));
        cycles = 4;
    }
    void bit_b_r()
    {
        u8 *r = get_reg(opcode & 0x07);
        bit_test(*r, (opcode >> 3) & 0x07);
        cycles = 2;
    }
    void bit_b_hl()
    {
        u16 addr = to_u16(L, H);
        bit_test(read_memory(addr), (opcode >> 3) & 0x07);
        cycles = 3;
    }
    void set_b_r()
    {
        u8 *r = get_reg(opcode & 0x07);
        *r = bit_set(*r, (opcode >> 3) & 0x07);
        cycles = 2;
    }
    void set_b_hl()
    {
        u16 addr = to_u16(L, H);
        write_memory(addr, bit_set(read_memory(addr), (opcode >> 3) & 0x07));
        cycles = 3;
    }
    void res_b_r()
    {
        u8 *r = get_reg(opcode & 0x07);
        *r = bit_reset(*r, (opcode >> 3) & 0x07);
        cycles = 2;
    }
    void res_b_hl()
    {
        u16 addr = to_u16(L, H);
        write_memory(addr, bit_reset(read_memory(addr), (opcode >> 3) & 0x07));
        cycles = 4;
    }
    void daa()
    {
        u8 lsbit = A & 0x0F;
        if ((lsbit > 0x09) || check_flag(half_carry))
        {
            A += 0x06;
        }
        u8 msbit = (A & 0xF0) >> 4;
        if ((msbit > 0x09) || check_flag(carry))
        {
            A += 0x60;
            set_flag(carry);
        }
        else
            reset_flag(carry);
        if (A == 0)
            set_flag(zero);
        else
            reset_flag(zero);
        reset_flag(half_carry);
        cycles = 1;
    }
    void cpl()
    {
        A = ~A;
        set_flag(half_carry);
        set_flag(subtract);
        cycles = 1;
    }
    void nop() { cycles++; }
    void ccf()
    {
        if (check_flag(carry))
            reset_flag(carry);
        else
            set_flag(carry);
        reset_flag(half_carry);
        reset_flag(subtract);
        cycles = 1;
    }
    void scf()
    {
        set_flag(carry);
        reset_flag(half_carry);
        reset_flag(subtract);
        cycles = 1;
    }
    void di()
    {
        IME = false;
        //disable all interrupt flags
        write_memory(IE, 0);
        cycles = 1;
    }
    void ei()
    {
        // std::cout << "IME enabled" << std::endl;
        IME = true;
        //enable all interrupt flags
        write_memory(IE, 0x1F);
        cycles = 1;
    }
    void halt()
    {
        // TODO
    }
    void stop()
    {
        // TODO
    }
    void fetch_opcode() { opcode = read_memory(PC++); }
    void decode_opcode()
    {
        switch (opcode)
        {
        // 8-bit Transfer IN/OUTs
        // LD to A
        case 0x7F:
        case 0x78:
        case 0x79:
        case 0x7A:
        case 0x7B:
        case 0x7C:
        case 0x7D:
        // LD to B
        case 0x40:
        case 0x41:
        case 0x42:
        case 0x43:
        case 0x44:
        case 0x45:
        case 0x47:
        // LD to C
        case 0x48:
        case 0x49:
        case 0x4A:
        case 0x4B:
        case 0x4C:
        case 0x4D:
        case 0x4F:
        // LD to D
        case 0x50:
        case 0x51:
        case 0x52:
        case 0x53:
        case 0x54:
        case 0x55:
        // LD to E
        case 0x58:
        case 0x59:
        case 0x5A:
        case 0x5B:
        case 0x5C:
        case 0x5D:
        case 0x5F:
        case 0x57:
        // LD to H
        case 0x60:
        case 0x61:
        case 0x62:
        case 0x63:
        case 0x64:
        case 0x65:
        case 0x67:
        // LD to L
        case 0x68:
        case 0x69:
        case 0x6A:
        case 0x6B:
        case 0x6C:
        case 0x6D:
        case 0x6F:
            ld_r_r();
            break;
        case 0x06:
        case 0x0E:
        case 0x16:
        case 0x1E:
        case 0x26:
        case 0x2E:
        case 0x3E:
            ld_r_n();
            break;
        case 0x7E:
        case 0x46:
        case 0x4E:
        case 0x56:
        case 0x5E:
        case 0x66:
        case 0x6E:
            ld_r_hl();
            break;
        case 0x70:
        case 0x71:
        case 0x72:
        case 0x73:
        case 0x74:
        case 0x75:
        case 0x77:
            ld_hl_r();
            break;
        case 0x36:
            ld_hl_n();
            break;
        case 0x0A:
            ld_a_bc();
            break;
        case 0x1A:
            ld_a_de();
            break;
        case 0xF2:
            ld_a_c();
            break;
        case 0xE2:
            ld_c_a();
            break;
        case 0xF0:
            ld_a_n();
            break;
        case 0xE0:
            ld_n_a();
            break;
        case 0xFA:
            ld_a_nn();
            break;
        case 0xEA:
            ld_nn_a();
            break;
        case 0x2A:
            ld_a_hli();
            break;
        case 0x3A:
            ld_a_hld();
            break;
        case 0x02:
            ld_bc_a();
            break;
        case 0x12:
            ld_de_a();
            break;
        case 0x22:
            ld_hli_a();
            break;
        case 0x32:
            ld_hld_a();
            break;
        // 16-bit Transfer Instruction
        case 0x01:
        case 0x11:
        case 0x21:
        case 0x31:
            ld_dd_nn();
            break;
        case 0xF9:
            ld_sp_hl();
            break;
        case 0xF5:
        case 0xC5:
        case 0xD5:
        case 0xE5:
            push_qq();
            break;
        case 0xF1:
        case 0xC1:
        case 0xD1:
        case 0xE1:
            pop_qq();
            break;
        case 0xF8:
            ldhl_sp_e();
            break;
        case 0x08:
            ld_nn_sp();
            break;
        // 8-bit ALU
        case 0x87:
        case 0x80:
        case 0x81:
        case 0x82:
        case 0x83:
        case 0x84:
        case 0x85:
            add_a_r();
            break;
        case 0xC6:
            add_a_n();
            break;
        case 0x86:
            add_a_hl();
            break;
        case 0x8F:
        case 0x88:
        case 0x89:
        case 0x8A:
        case 0x8B:
        case 0x8C:
        case 0x8D:
            adc_a_r();
            break;
        case 0xCE:
            adc_a_n();
            break;
        case 0x8E:
            adc_a_hl();
            break;
        case 0x97:
        case 0x90:
        case 0x91:
        case 0x92:
        case 0x93:
        case 0x94:
        case 0x95:
            sub_r();
            break;
        case 0xD6:
            sub_n();
            break;
        case 0x96:
            sub_hl();
            break;
        case 0x9F:
        case 0x98:
        case 0x99:
        case 0x9A:
        case 0x9B:
        case 0x9C:
        case 0x9D:
            sbc_a_r();
            break;
        case 0xDE:
            sbc_a_n();
            break;
        case 0x9E:
            sbc_a_hl();
            break;
        case 0xA7:
        case 0xA0:
        case 0xA1:
        case 0xA2:
        case 0xA3:
        case 0xA4:
        case 0xA5:
            and_r();
            break;
        case 0xE6:
            and_n();
            break;
        case 0xA6:
            and_hl();
            break;
        case 0xB7:
        case 0xB0:
        case 0xB1:
        case 0xB2:
        case 0xB3:
        case 0xB4:
        case 0xB5:
            or_r();
            break;
        case 0xF6:
            or_n();
            break;
        case 0xB6:
            or_hl();
            break;
        case 0xAF:
        case 0xA8:
        case 0xA9:
        case 0xAA:
        case 0xAB:
        case 0xAC:
        case 0xAD:
            xor_r();
            break;
        case 0xEE:
            xor_n();
            break;
        case 0xAE:
            xor_hl();
            break;
        case 0xBF:
        case 0xB8:
        case 0xB9:
        case 0xBA:
        case 0xBB:
        case 0xBC:
        case 0xBD:
            cp_r();
            break;
        case 0xFE:
            cp_n();
            break;
        case 0xBE:
            cp_hl();
            break;
        case 0x3C:
        case 0x04:
        case 0x0C:
        case 0x14:
        case 0x1C:
        case 0x24:
        case 0x2C:
            inc_r();
            break;
        case 0x34:
            inc_hl();
            break;
        case 0x3D:
        case 0x05:
        case 0x0D:
        case 0x15:
        case 0x1D:
        case 0x25:
        case 0x2D:
            dec_r();
            break;
        case 0x35:
            dec_hl();
            break;
        // 16-bit ALU
        case 0x09:
        case 0x19:
        case 0x29:
        case 0x39:
            add_hl_ss();
            break;
        case 0xE8:
            add_sp_e();
            break;
        case 0x03:
        case 0x13:
        case 0x23:
        case 0x33:
            inc_ss();
            break;
        case 0x0B:
        case 0x1B:
        case 0x2B:
        case 0x3B:
            dec_ss();
            break;
        // Rotate shift Instructions
        case 0x07:
            rlca();
            break;
        case 0x17:
            rla();
            break;
        case 0x0F:
            rrca();
            break;
        case 0x1F:
            rra();
            break;
        case 0xCB:
            opcode = read_memory(PC++);
            switch (opcode & 0xC7)
            {
            case 0x47:
            case 0x40:
            case 0x41:
            case 0x42:
            case 0x43:
            case 0x44:
            case 0x45:
                bit_b_r();
                return;
            case 0x46:
                bit_b_hl();
                return;
            case 0xC7:
            case 0xC0:
            case 0xC1:
            case 0xC2:
            case 0xC3:
            case 0xC4:
            case 0xC5:
                set_b_r();
                return;
            case 0xC6:
                set_b_hl();
                return;
            case 0x87:
            case 0x80:
            case 0x81:
            case 0x82:
            case 0x83:
            case 0x84:
            case 0x85:
                res_b_r();
                return;
            case 0x86:
                res_b_hl();
                return;
            }
            switch (opcode)
            {
            case 0x07:
            case 0x00:
            case 0x01:
            case 0x02:
            case 0x03:
            case 0x04:
            case 0x05:
                rlc_r();
                break;
            case 0x06:
                rlc_hl();
                break;
            case 0x17:
            case 0x10:
            case 0x11:
            case 0x12:
            case 0x13:
            case 0x14:
            case 0x15:
                rl_r();
                break;
            case 0x16:
                rl_hl();
                break;
            case 0x0F:
            case 0x08:
            case 0x09:
            case 0x0A:
            case 0x0B:
            case 0x0C:
            case 0x0D:
                rrc_r();
                break;
            case 0x0E:
                rrc_hl();
                break;
            case 0x1F:
            case 0x18:
            case 0x19:
            case 0x1A:
            case 0x1B:
            case 0x1C:
            case 0x1D:
                rr_r();
                break;
            case 0x1E:
                rr_hl();
                break;
            case 0x27:
            case 0x20:
            case 0x21:
            case 0x22:
            case 0x23:
            case 0x24:
            case 0x25:
                sla_r();
                break;
            case 0x26:
                sla_hl();
                break;
            case 0x2F:
            case 0x28:
            case 0x29:
            case 0x2A:
            case 0x2B:
            case 0x2C:
            case 0x2D:
                sra_r();
                break;
            case 0x2E:
                sra_hl();
                break;
            case 0x3F:
            case 0x38:
            case 0x39:
            case 0x3A:
            case 0x3B:
            case 0x3C:
            case 0x3D:
                srl_r();
                break;
            case 0x3E:
                srl_hl();
                break;
            case 0x37:
            case 0x30:
            case 0x31:
            case 0x32:
            case 0x33:
            case 0x34:
            case 0x35:
                swap_r();
                break;
            case 0x36:
                swap_hl();
                break;
            default:
                std::cout << "Unknown opcode: CB,  " << std::hex << opcode
                          << std::endl;
                break;
                exit(3);
            }
            break;
        // General-Purpose Arithmetic Operations and CPU Control Instructions
        case 0x27:
            daa();
            break;
        case 0x2F:
            cpl();
            break;
        case 0x00:
            nop();
            break;
        case 0x3F:
            ccf();
            break;
        case 0x37:
            scf();
            break;
        case 0xF3:
            di();
            break;
        case 0xFB:
            ei();
            break;
        case 0x76:
            halt();
            break;
        case 0x10:
            opcode = read_memory(PC++);
            if (opcode == 0x00)
            {
                stop();
                return;
            }
            break;
        // JUMPs
        case 0xC3:
            jp_nn();
            break;
        case 0xE9:
            jp_hl();
            break;
        case 0xC2:
        case 0xD2:
        case 0xCA:
        case 0xDA:
            jp_cc_nn();
            break;
        case 0x18:
            jr_r();
            break;
        case 0x20:
        case 0x30:
        case 0x28:
        case 0x38:
            jr_cc_r();
            break;
        // CALLs
        case 0xCD:
            call_nn();
            break;
        case 0xC4:
        case 0xD4:
        case 0xCC:
        case 0xDC:
            call_cc_nn();
            break;
        // RETURNs
        case 0xC9:
            ret();
            break;
        case 0xC0:
        case 0xD0:
        case 0xC8:
        case 0xD8:
            ret_cc();
            break;
        case 0xD9:
            reti();
            break;
        // RESTARTs
        case 0xC7:
        case 0xD7:
        case 0xE7:
        case 0xF7:
        case 0xCF:
        case 0xDF:
        case 0xEF:
        case 0xFF:
            rst_n();
            break;
        default:
            std::cout << "Unknown opcode: " << std::hex << opcode << std::endl;
            // exit(1);
            error_occurred = true;
            break;
        }
    }
    int run()
    {
        // std::ofstream myfile("log.txt", std::ofstream::app);
        // if (myfile.is_open())
        // {
        while (true)
        {
            fetch_opcode();
            //debug
            // myfile << "PC: " << std::hex << int(PC - 1)
            //        << " opcode: " << std::hex << int(opcode) << " A:" << std::hex << int(A) << " B:" << std::hex << int(B) << " C:" << std::hex << int(C) << " D:" << std::hex << int(D) << " E:" << std::hex << int(E) << " F:" << std::hex << int(F) << " H:" << std::hex << int(H) << " L:" << std::hex << int(L) << std::endl;

            // //debug
            // std::cout << "PC: " << std::hex << int(PC - 1)
            //           << " opcode: " << std::hex << int(opcode) << " A:" << std::hex << int(A) << " B:" << std::hex << int(B) << " C:" << std::hex << int(C) << " D:" << std::hex << int(D) << " E:" << std::hex << int(E) << " F:" << std::hex << int(F) << " H:" << std::hex << int(H) << " L:" << std::hex << int(L) << std::endl;
            decode_opcode();
            if (error_occurred)
                break;
            for (u8 i = 0; i < cycles; i++)
            {
                PPU::tick();
            }

            //check for IRQs
            if (read_memory(IF) > 0) //an interrupt is there
            {
                if (IME)
                {
                    if ((read_memory(IE) & 0x01) == 0x01)
                    {
                        //VBlank
                        //disable interrupt
                        if ((read_memory(IF) & 0x01) == 0x01)
                        {
                            write_memory(IF, read_memory(IF) & 0xFE);
                            IME = false;
                            serve_isr(VSYNCVEC);
                        }
                    }
                    if ((read_memory(IE) & 0x04) == 0x04)
                    {
                        if ((read_memory(IF) & 0x04) == 0x04)
                        {
                            //TIMER
                            //disable interrupt
                            write_memory(IF, read_memory(IF) & 0xFB);
                            IME = false;
                            serve_isr(TIMER);
                        }
                    }
                }
            }

            cycles_this_frame += cycles;
            if (cycles_this_frame >= CYCLES_PER_FRAME)
            {
                cycles_this_frame -= CYCLES_PER_FRAME;
                break;
            }
        }
        //     myfile.close();
        // }
        // std::cout << "Ran a frame" << std::endl;
        return 0;
    }

    void serve_isr(u16 vector)
    {
        write_memory(--SP, msb(PC));
        write_memory(--SP, lsb(PC));
        PC = vector;
        cycles = 5;
    }
} // namespace CPU