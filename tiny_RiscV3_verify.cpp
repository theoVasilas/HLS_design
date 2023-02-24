#include <stdlib.h>
#include <iostream>
#include <cmath>
#include <bitset>

#include <ac_int.h>
#include <ac_math.h>

#include <stdio.h>

#include "mc_scverify.h"

ac_int<32, true> signed_numA, signed_numB;  //for signed extentions

ac_int<32, true> mem[0xfffff];              // 1MB virtual memory address space from 0x00000000 to 0x000fffff

ac_int<32, true> PC = 0;
ac_int<32, false> instr[1024], ir;          // instraction memory   ,instraction counter

ac_int<32, false> R[32];                    //registers  file
ac_int<12, false> CSR[32];                  //control/status register


struct output
{
    ac_int<3, false> flag = true;
    ac_int<32, true> address = true;
    ac_int<32, true> value = true;
};


output  CCS_BLOCK(R_type) (ac_int<7, false> opcode, ac_int<5, false> rd, ac_int<3, false> func3, ac_int<5, false> rs1, ac_int<5, false> rs2, ac_int<7, false> func7) {

    ac_int<5, false> imm;                   //only need unsigned 
    ac_int<32, true> signed_num;

    //std::cout << "func7 " << func7.to_string(AC_BIN, false, true) << //std::endl;

    output out;       //creat output flag - address - value

    switch (func3) {

    case 0b000:
        if (func7 == 0) {                    //ADD
            out.value = R[rs1] + R[rs2];

            //std::cout << "ADD x" << rd << " ,x" << rs1 << " ,x" << rs2 << //std::endl;
        }
        else if (func7 == 0x20) {             //SUB
            out.value = R[rs1] - R[rs2];

            //std::cout << "SUB x" << rd << " ,x" << rs1 << " ,x" << rs2 << //std::endl;
        }
        else if (func7 == 1) {                //MUL
            out.value = R[rs1] * R[rs2];

            //std::cout << "MUL x" << rd << " ,x" << rs1 << " ,x" << rs2 << //std::endl;
        }
        else
            //std::cout << "Unknown R instraction" << //std::endl;
        break;

    case 0b001:                             //SLL - SLLI
        if (opcode == 0b0010011) {          //SLLI
            imm = rs2;                      //rs2 is immediate value
            out.value = R[rs1] << imm;

            //std::cout << "SLLI x" << rd << " ,x" << rs1 << " ," << imm << //std::endl;
        }
        else if (opcode == 0b0110011) {     //SLL
            imm = R[rs2] & 0b11111;         //only use the bottom five bits of R[rs2] when performing the shift.
            out.value = R[rs1] << imm;

            //std::cout << "SLL x" << rd << " ,x" << rs1 << " ,x" << rs2 << "=" << imm << //std::endl;
        }
        else {
            //std::cout << "Unknown R instraction" << //std::endl;
        }
        break;

    case 0b010:                             //SLT  signed less-than comparison
        signed_numA = R[rs1];               //sined casting 
        signed_numB = R[rs2];
        out.value = signed_numA < signed_numB;

        //std::cout << "SLT x" << rd << " ,x" << rs1 << " ,x" << rs2 << //std::endl;
        break;

    case 0b011:                             //SLTU
        out.value = R[rs1] < R[rs2];

        //std::cout << "SLTU x" << rd << " ,x" << rs1 << " ,x" << rs2 << //std::endl;
        break;

    case 0b100:                             //XOR
        out.value = R[rs1] ^ R[rs2];
        //std::cout << "XOR x" << rd << " ,x" << rs1 << " ,x" << rs2 << //std::endl;
        break;

    case 0b101:                             //SRA-SRL - SRAI - SRLI
        if (func7 == 0b0100000) {                //shift right ARITHMETIC 

            if (opcode == 0b0110011) {      //SRA
                imm = R[rs2] & 0b11111;     //only use thebottom five bits of R[rs2] when performing the shift
                signed_numA = R[rs1];       //sign casting
                out.value = signed_numA >> imm; //sign - bit of R[rs1] is extended

                //std::cout << "SRA x" << rd << " ,x" << rs1 << " ,x" << rs2 << "=" << imm << //std::endl;

            }
            else if (opcode == 0b0010011) {  //SRAI
                imm = rs2;                  //rs2 is immediate value
                signed_numA = R[rs1];       //sign casting
                out.value = R[rs1] >> imm;      //sign - bit of R[rs1] is extended

                //std::cout << "SRAI x" << rd << " ,x" << rs1 << " ," << imm << //std::endl;

            }
            else {
                //std::cout << "Unknown R instraction" << //std::endl;
            }
        }
        else if (func7 == 0) {              //SR LOGICAL (no need for sign casting )

            if (opcode == 0b0110011) {      //SRL
                imm = R[rs2] & 0b11111;     //only use thebottom five bits of R[rs2] when performing the shift
                out.value = R[rs1] >> R[rs2];   //sign - bit of R[rs1] is extended

                //std::cout << "SRL x" << rd << " ,x" << rs1 << " ,x" << rs2 << //std::endl;

            }
            else if (opcode == 0b0010011) {  //SRLI
                imm = rs2;                  //rs2 is immediate value
                out.value = R[rs1] >> imm;      //sign - bit of R[rs1] is extended

                //std::cout << "SRLI x" << rd << " ,x" << rs1 << " ," << imm << //std::endl;

            }
            else {
                //std::cout << "Unknown R instraction" << //std::endl;
            }
        }
        break;
    case 0b110:                             //OR
        out.value = R[rs1] | R[rs2];

        //std::cout << "OR x" << rd << " ,x" << rs1 << " ,x" << rs2 << //std::endl;

        break;

    case 0b111:                             //AND
        out.value = R[rs1] & R[rs2];

        //std::cout << "AND x" << rd << " ,x" << rs1 << " ,x" << rs2 << //std::endl;

        break;

    default:                                // Handle unknown 
        //std::cout << "Unknown R instraction" << //std::endl;
    }

    out.address = rd;
    out.flag = 1;                         //all registers writes (code 01)
                                            //there is no PC change 
    return out;
}


output  CCS_BLOCK(S_type) (ac_int<7, false> opcode, ac_int<3, false> func3, ac_int<5, false> rs1, ac_int<5, false> rs2, ac_int<12, false> imm12) {

    output out;       //creat output flag - address - value
    switch (func3)
    {
    case 0b000:                         //BEQ	
        //if (imm12 & 0b00) {           //bottom two bits of every effective address will always be zero.
                                        // Sign-extend the immediate value
        if (imm12 & 0x800) {            // If the 12th bit of the immediate is set (i.e., the value is negative)
            imm12 = imm12 | 0xFFFFF000; // Set all the bits to the left of the 12th bit to 1
        }
        out.value = (R[rs1] == R[rs2]) ? (PC + imm12) : (PC + 1);

        //std::cout << "BEQ x" << rs1 << " ,x" << rs1 << " ," << imm12 << //std::endl;

        //}
        out.flag = 2;                     //PC change code
        break;

    case 0b001:                             //BNE	
        //if (imm12 & 0b00) {                 //bottom two bits of every effective address will always be zero.
                                            // Sign-extend the immediate value
        if (imm12 & 0x800) {            // If the 12th bit of the immediate is set (i.e., the value is negative)
            imm12 = imm12 | 0xFFFFF000; // Set all the bits to the left of the 12th bit to 1
        }
        out.value = (R[rs1] != R[rs2]) ? (PC + imm12) : (PC + 1);

        //std::cout << "BNE x" << rs1 << " ,x" << rs1 << " ," << imm12 << //std::endl;

        //}
        out.flag = 2;                     //PC change code
        break;

    case 0b010:                             //SW	M_4B[ R[rs1] + sext(imm) ] = R[rs2]
        //if (imm12 & 0b00) {                 //bottom two bits of every effective address will always be zero.
                                            // Sign-extend the immediate value
        if (imm12 & 0x800) {            // If the 12th bit of the immediate is set (i.e., the value is negative)
            imm12 = imm12 | 0xFFFFF000; // Set all the bits to the left of the 12th bit to 1
        }
        out.value = R[rs2];
        out.address = R[rs1] + imm12;

        //std::cout << "mem[" << out.address << "] <- R[" << rs2 << "] = " << out.value << //std::endl;
        //}
        out.flag = 3;                           //mem write code
        break;

    case 0b100:                             //BLT	
                                            //for sext(imm)
        if (imm12 & 0x800) {                // If the 12th bit of the immediate is set (i.e., the value is negative)
            imm12 = imm12 | 0xFFFFF000;     // Set all the bits to the left of the 12th bit to 1
        }
        signed_numA = R[rs1];               //sined casting 
        signed_numB = R[rs2];
        out.value = (signed_numA < signed_numB) ? PC + imm12 : PC + 1;

        //std::cout << "BLT x" << rs1 << " ,x" << rs2 << " ," << imm12 << //std::endl;

        out.flag = 2;                           //PC change code
        break;

    case 0b101:                             //BGE	
        if (imm12 & 0x800) {                // If the 12th bit of the immediate is set (i.e., the value is negative)
            imm12 = imm12 | 0xFFFFF000;     // Set all the bits to the left of the 12th bit to 1
        }
        signed_numA = R[rs1];               //sined casting 
        signed_numB = R[rs2];
        out.value = (signed_numA >= signed_numB) ? PC + imm12 : PC + 1;

        //std::cout << "BGE x" << rs1 << " ,x" << rs2 << " ," << imm12 << //std::endl;

        out.flag = 2;                           //PC change code
        break;

    case 0b110:                             //BLTU	
        if (imm12 & 0x800) {                // If the 12th bit of the immediate is set (i.e., the value is negative)
            imm12 = imm12 | 0xFFFFF000;     // Set all the bits to the left of the 12th bit to 1
        }
        out.value = (R[rs1] < R[rs2]) ? PC + imm12 : PC + 1;

        //std::cout << "BLTU x" << rs1 << " ,x" << rs2 << " ," << imm12 << //std::endl;

        out.flag = 2;                           //PC change code
        break;

    case 0b111:                             // BGEU	PC = (R[rs1] >= u R[rs2]) ? PC + sext(imm) : PC + 1
        if (imm12 & 0x800) {                // If the 12th bit of the immediate is set (i.e., the value is negative)
            imm12 = imm12 | 0xFFFFF000;     // Set all the bits to the left of the 12th bit to 1
        }
        out.value = (R[rs1] >= R[rs2]) ? PC + imm12 : PC + 1;

        //std::cout << "BGEU x" << rs1 << " ,x" << rs2 << " ," << imm12 << //std::endl;

        out.flag = 2;                           //PC change code
        break;

    default:
        //std::cout << "Unknown S instraction" << //std::endl;
        break;
    }

    return out;
}


output  CCS_BLOCK(I_type) (ac_int<7, false> opcode, ac_int<5, false> rd, ac_int<3, false> func3, ac_int<5, false> rs1, ac_int<12, false> imm12) {

    ////std::cout << "imm12: " << imm12 << //std::endl;
    output out;       //creat output flag - address - value

    switch (func3)
    {
    case 0b010:                             //CSRR
        if (opcode == 0b1110011) {
            out.value = CSR[imm12];         //csr = immediate

            //std::cout << "CSRR " << rd << " ," << imm12 << //std::endl;

        }
        else if (opcode == 0b0000011) {     //LW	
            if (imm12 & 0x800) {            // If the 12th bit of the immediate is set (i.e., the value is negative)
                imm12 = imm12 | 0xFFFFF000; // Set all the bits to the left of the 12th bit to 1
            }
            if ((R[rs1] + imm12) & 0b00) {  //checking four-byte aligned
                out.value = mem[R[rs1] + imm12];
            }

            //std::cout << "LW x" << rs1 << " ," << imm12 << "(x" << rs1 << ")" << //std::endl;

        }
        else if (opcode == 0010011) {       //SLTI	
            if (imm12 & 0x800) {            // If the 12th bit of the immediate is set (i.e., the value is negative)
                imm12 = imm12 | 0xFFFFF000; // Set all the bits to the left of the 12th bit to 1
            }
            signed_numA = R[rs1];           //sign cast
            out.value = (R[rs1] < imm12);

            //std::cout << "SLTI x" << rd << " ,x" << rs1 << " ," << imm12 << //std::endl;
        }
        else {
            //std::cout << "Unknown instraction" << //std::endl;
        }

        out.address = rd;
        out.flag = 1;                       //regidter write 
        break;

    case 0b001:                             //CSRW		
        out.value = R[rs1];                 //csr = immediate
        out.address = imm12;

        //std::cout << "CSRW " << imm12 << " ,x" << rs1 << //std::endl;

        out.flag = 5;                       //CSR code 
        break;

    case 0b111:                             //ANDI	
        if (imm12 & 0x800) {                // If the 12th bit of the immediate is set (i.e., the value is negative)
            imm12 = imm12 | 0xFFFFF000;     // Set all the bits to the left of the 12th bit to 1
        }
        out.value = R[rs1] & imm12;
        out.address = rd;

        //std::cout << "ANDI x" << rd << " ,x" << rs1 << " ," << imm12 << //std::endl;

        out.flag = 1;                           //regidter write 
        break;

    case 0b110:                             //ORI	
        if (imm12 & 0x800) {                // If the 12th bit of the immediate is set (i.e., the value is negative)
            imm12 = imm12 | 0xFFFFF000;     // Set all the bits to the left of the 12th bit to 1
        }
        out.value = R[rs1] | imm12;
        out.address = rd;

        //std::cout << "ORI x" << rd << " ,x" << rs1 << " ," << imm12 << //std::endl;

        out.flag = 1;                           //regidter write
        break;

    case 0b100:                             //XORI	
        if (imm12 & 0x800) {                // If the 12th bit of the immediate is set (i.e., the value is negative)
            imm12 = imm12 | 0xFFFFF000;     // Set all the bits to the left of the 12th bit to 1
        }
        out.value = R[rs1] ^ imm12;
        out.address = rd;

        //std::cout << "XORI x" << rd << " ,x" << rs1 << " ," << imm12 << //std::endl;

        out.flag = 1;                           //regidter write
        break;

    case 0b011:                             //SLTIU
        if (imm12 & 0x800) {                // If the 12th bit of the immediate is set (i.e., the value is negative)
            imm12 = imm12 | 0xFFFFF000;     // Set all the bits to the left of the 12th bit to 1
        }
        out.value = R[rs1] < imm12;
        out.address = rd;

        //std::cout << "SLTIU x" << rd << " ,x" << rs1 << " ," << imm12 << //std::endl;

        out.flag = 1;                               //regidter write
        break;

    case 0b000:
        if (opcode == 0010011) {                //ADDI	
            if (imm12 & 0x800) {                // If the 12th bit of the immediate is set (i.e., the value is negative)
                imm12 = imm12 | 0xFFFFF000;     // Set all the bits to the left of the 12th bit to 1
            }
            out.value = R[rs1] + imm12;
            out.address = rd;

            //std::cout << "ADDI x" << rd << " ,x" << rs1 << " ," << imm12 << //std::endl;

            out.flag = 1;                           //reg write
        }

        else if (opcode == 1100111) {
            if ((rd == 0) && (imm12 == 0)) {        //JR	
                out.value = R[rs1];

                //std::cout << "JR x" << rs1 << //std::endl;

                out.flag = 2;                       //pc write
            }
            else {                                  //JALR	
                if (imm12 & 0x800) {                // If the 12th bit of the immediate is set (i.e., the value is negative)
                    imm12 = imm12 | 0xFFFFF000;     // Set all the bits to the left of the 12th bit to 1
                }
                //R[rd] = PC + 1;                   //its implimented in loop
                out.address = rd;

                out.value = (R[rs1] + imm12) & 0xfffffffe;

                //std::cout << "JALR x" << rd << " ,x" << rs1 << " ," << imm12 << //std::endl;

                out.flag = 4;                       //PC write
            }
        }

        break;

    default:
        //std::cout << "Unknown I instraction" << //std::endl;
        out.flag = 0;
        break;
    }

    return out;
}


output  CCS_BLOCK(U_type) (ac_int<7, false> opcode, ac_int<5, false> rd, ac_int<20, false> imm20) {

    //std::cout << "imm20: " << imm20.to_string(AC_BIN, false, true) << //std::endl;

    output out;       //creat output flag - address - value

    switch (opcode)
    {
    case 0b0110111:                     //LUI
        out.value = imm20 << 12;
        out.address = rd;

        //std::cout << "LUI x" << rd << " ," << imm20 << //std::endl;

        out.flag = 1;                   //reg write
        break;

    case 0b0010111:                     //AUIPC
        out.value = PC + (imm20 << 12);
        out.address = rd;

        //std::cout << "AUIPC x" << out.address << " ," << imm20 << //std::endl;

        out.flag = 1;                   //reg write
        break;

    case 0b1101111:                     //JAL
        //R[rd] = PC + 1;

        if (imm20 & 0x80000) {          // If the 20th bit of the immediate is set (i.e., the value is negative)
            imm20 = imm20 | 0xFFF00000; // Set all the bits to the left of the 20th bit to 1
        }
        out.address = rd;
        out.value = PC + imm20;

        //std::cout << "JAL x" << rd << " ," << imm20 << //std::endl;

        out.flag = 4;                   //PC write
        break;

    default:
        //std::cout << "Unknown U instraction" << //std::endl;
        break;
    }
    return out;
}


output  CCS_BLOCK(Decoder) (ac_int<32, false> instr) {

    ac_int<7, false> opcode;
    ac_int<5, false> rd, rs1, rs2;
    ac_int<3, false> func3;
    ac_int<7, false> func7;
    ac_int<32, true> imm12, imm20;

    //seperate the instaction
    opcode = instr & 0x7F;
    rd = (instr >> 7) & 0x1F;
    func3 = (instr >> 12) & 0b111;
    rs1 = (instr >> 15) & 0x1F;
    rs2 = (instr >> 20) & 0x1F;
    func7 = (instr >> 25) & 0x7f;

    //std::cout << "\n" << PC << " instr: " << instr.to_string(AC_BIN, false, true)
 //       << " " << instr.to_string(AC_HEX, false, true)
 //       << " opcode: " << opcode.to_string(AC_BIN, false, true)
 //       << " func3: " << func3.to_string(AC_BIN, false, true) << //std::endl;

    output out;       //creat output flag - address - value

    switch (opcode) {

    case 0b0110011: // R-type

        out = R_type(opcode, rd, func3, rs1, rs2, func7);

        //std::cout << "R-type" << //std::endl;
        break;

    case 0b0010011: // I-type R-type
        if ((func3 == 0b001) || (func3 == 0b101)) {

            out = R_type(opcode, rd, func3, rs1, rs2, func7);

            //std::cout << "R-type" << //std::endl;
            break;
        }
    case 0b1100111:// I-type
    case 0b0000011:// I-type
    case 0b1110011:// I-type

        imm12 = (instr >> 20) & 0xFFF;  // extract 12 bit immediate
        out = I_type(opcode, rd, func3, rs1, imm12);

        //std::cout << "I-type" << " immm12:" << imm12.to_string(AC_BIN, false, true) << //std::endl; // true, the string captures the full length of the type 
        break;

    case 0b1100011: // S-type
    case 0b0100011:

        imm12 = (rd & 0x1F) | ((func7 & 0x7F) << 5); // combine rd and func7 
        out = S_type(opcode, func3, rs1, rs2, imm12);

        //std::cout << "S-type" << " immm12:" << imm12.to_string(AC_BIN, false, true) << //std::endl;
        break;

    case 0b1101111: // U-type
    case 0b0010111:
    case 0b0110111:


        imm20 = instr >> 12;
        out = U_type(opcode, rd, imm20);

        //std::cout << "U-type" << //std::endl;
        break;

    default: // Handle unknown opcode
        //std::cout << "Unknown opcode " << opcode.to_string(AC_BIN, false, true) << //std::endl;
        out.flag = 0b00;   //stop the execution
        break;
    }

    return out;
}

#pragma hls_design top
bool CCS_BLOCK(LOOP)() {

    ac::init_array<AC_VAL_0>(mem, 1024);   //clear all values

    //import instrations 

    output out;             //creat output flag - address - value

    while (out.flag)
    {
        //FETCHE
        ir = instr[PC];

        //Decode 
        out = Decoder(ir); //decode -> ALU (R,I,S,U-type) -> output

        //std::cout << "flage:" << out.flag << " ardess:" << out.address << " value:" << out.value << //std::endl;

        //Memory Register CSR WRITE 
        switch (out.flag)
        {
        case 1:
            R[out.address] = out.value;
            PC += 1;
            break;

        case 2:
            PC = out.value;
            break;

        case 3:
            mem[out.address] = out.value;
            PC += 1;
            break;

        case 4:
            R[out.address] = PC + 1;
            PC = out.value;
            break;

        case 5:
            CSR[out.address] = out.value;
            PC += 1;
            break;

        default:
            break;
        }


    }
    return 0;
}


CCS_MAIN(int argc, char** argv) {
    //============================== U-TYPE
    instr[0] = 0x00002237;//LUI	R[4] = 2 << 12
    instr[1] = 0x00002217;//AUIPC  rd, imm
    instr[2] = 0x0000116F;//JAL    imm 
    //============================== I-TYPE
    instr[3] = 0xFFFF2F73;//CSRR
    instr[4] = 0x00041273;//CSRW
    instr[5] = 0x00040213;//ADDI
    instr[6] = 0x00047213;//ANDI
    instr[7] = 0x00046213;//ORI
    instr[8] = 0x00044213;//XORI
    instr[9] = 0x00042213;//SLTI
    instr[10] = 0x00043213;//SLTIU
    instr[11] = 0x00040203;//LW
    instr[12] = 0x00040267;//JR         
    instr[13] = 0x00440267;//JALR       
    //============================== S-TYPE
    instr[14] = 0x01122223;//SW
    instr[15] = 0x000000E3;//BEQ       
    instr[16] = 0x000010E3;//BNE
    instr[17] = 0x01124263;//BLT
    instr[18] = 0x000050E3;//BGE
    instr[19] = 0x01126263;//BLTU
    instr[20] = 0x001270E3;//BGEU
    //============================== R-TYPE
    instr[21] = 0x00230233;//ADD
    instr[22] = 0x40230233;//SUB
    instr[23] = 0x02230233;//MUL
    instr[24] = 0x00231293;//SLLI       
    instr[25] = 0x00231233;//SLL
    instr[26] = 0x00232233;//SLT
    instr[27] = 0x00233233;//SLTU
    instr[28] = 0x00234233;//XOR
    instr[29] = 0x40235233;//SRA
    instr[30] = 0x4008D113;//SRAI       
    instr[31] = 0x00235233;//SRL
    instr[32] = 0x00235213;//SRLI       
    instr[33] = 0x00236233;//OR         
    instr[34] = 0x00237233;//AND        
    instr[35] = 0;


    bool zero;
    zero = LOOP();

    CCS_RETURN(0);

}
