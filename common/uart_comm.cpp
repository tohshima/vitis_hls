#include <windows.h>
#include "uart_comm.hpp"

uart_comm::uart_comm(const char* portName) {
    hSerial = CreateFileA(portName,
        GENERIC_READ | GENERIC_WRITE,
        0,
        0,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        0);

    if (hSerial == INVALID_HANDLE_VALUE) {
        if (GetLastError() == ERROR_FILE_NOT_FOUND) {
            std::cerr << "Serial port does not exist.\n";
        }
        else {
            std::cerr << "Some other error occurred.\n";
        }
    }

    DCB dcbSerialParams = { 0 };
    dcbSerialParams.DCBlength = sizeof(dcbSerialParams);

    if (!GetCommState(hSerial, &dcbSerialParams)) {
        std::cerr << "Error getting device state\n";
        CloseHandle(hSerial);
        return;
    }

    dcbSerialParams.BaudRate = CBR_256000; //CBR_115200;
    dcbSerialParams.ByteSize = 8;
    dcbSerialParams.StopBits = ONESTOPBIT;
    dcbSerialParams.Parity = NOPARITY;

    if (!SetCommState(hSerial, &dcbSerialParams)) {
        std::cerr << "Error setting device parameters\n";
        CloseHandle(hSerial);
        return;
    }

    COMMTIMEOUTS timeouts = { 0 };
    timeouts.ReadIntervalTimeout = 50;
    timeouts.ReadTotalTimeoutConstant = 50;
    timeouts.ReadTotalTimeoutMultiplier = 10;
    timeouts.WriteTotalTimeoutConstant = 50;
    timeouts.WriteTotalTimeoutMultiplier = 10;

    if (!SetCommTimeouts(hSerial, &timeouts)) {
        std::cerr << "Error setting timeouts\n";
        CloseHandle(hSerial);
        return;
    }
}

uart_comm::~uart_comm() {
    if (hSerial != INVALID_HANDLE_VALUE) {
        CloseHandle(hSerial);
    }
}

bool uart_comm::write_data(const char* data, size_t length) {
    DWORD bytesWritten;
    if (!WriteFile(hSerial, data, length, &bytesWritten, NULL)) {
        std::cerr << "Error writing to serial port\n";
        return false;
    }
    return true;
}

bool uart_comm::read_data(char* buffer, size_t buffer_size, DWORD& bytes_read) {
    if (!ReadFile(hSerial, buffer, buffer_size, &bytes_read, NULL)) {
        std::cerr << "Error reading from serial port\n";
        return false;
    }
    return true;
}
