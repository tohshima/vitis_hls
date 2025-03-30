#include <string>
#include <bitset>
#include <sstream>

std::string disassemble(uint16_t instruction) {
    std::bitset<16> bits(instruction);
    std::stringstream result;

    // A-instruction
    if (!bits[15]) {
        return "@" + std::to_string(instruction);
    }

    // C-instruction
    else {
        // Destination
        if (bits[5]) result << "A";
        if (bits[4]) result << "D";
        if (bits[3]) result << "M";
        if (bits[3] || bits[4] || bits[5]) result << "=";

        // Computation
        std::string comp;
        if (!bits[12]) { // a-bit is 0
            switch ((bits[11] << 5) | (bits[10] << 4) | (bits[9] << 3) | (bits[8] << 2) | (bits[7] << 1) | bits[6]) {
                case 0b101010: comp = "0"; break;
                case 0b111111: comp = "1"; break;
                case 0b111010: comp = "-1"; break;
                case 0b001100: comp = "D"; break;
                case 0b110000: comp = "A"; break;
                case 0b001101: comp = "-D"; break;
                case 0b110001: comp = "!A"; break;
                case 0b001111: comp = "D+1"; break;
                case 0b110111: comp = "A+1"; break;
                case 0b001110: comp = "D-1"; break;
                case 0b110010: comp = "A-1"; break;
                case 0b000010: comp = "D+A"; break;
                case 0b010011: comp = "D-A"; break;
                case 0b000111: comp = "A-D"; break;
                case 0b000000: comp = "D&A"; break;
                case 0b010101: comp = "D|A"; break;
            }
        } else { // a-bit is 1
            switch ((bits[11] << 5) | (bits[10] << 4) | (bits[9] << 3) | (bits[8] << 2) | (bits[7] << 1) | bits[6]) {
                case 0b110000: comp = "M"; break;
                case 0b110001: comp = "!M"; break;
                case 0b110011: comp = "-M"; break;
                case 0b110111: comp = "M+1"; break;
                case 0b110010: comp = "M-1"; break;
                case 0b000010: comp = "D+M"; break;
                case 0b010011: comp = "D-M"; break;
                case 0b000111: comp = "M-D"; break;
                case 0b000000: comp = "D&M"; break;
                case 0b010101: comp = "D|M"; break;
            }
        }
        result << comp;

        // Jump
        if (bits[2] || bits[1] || bits[0]) {
            result << ";";
            if (bits[2] && bits[1] && bits[0]) result << "JMP";
            else if (bits[2] && bits[1] && !bits[0]) result << "JGE";
            else if (bits[2] && !bits[1] && bits[0]) result << "JNE";
            else if (bits[2] && !bits[1] && !bits[0]) result << "JGT";
            else if (!bits[2] && bits[1] && bits[0]) result << "JLE";
            else if (!bits[2] && bits[1] && !bits[0]) result << "JEQ";
            else if (!bits[2] && !bits[1] && bits[0]) result << "JLT";
        }

        return result.str();
    }
}
