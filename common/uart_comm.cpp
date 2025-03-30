#if 1 // linux
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
#include <iostream>
#include <cstring>
#include <errno.h>
#include "uart_comm.hpp"

uart_comm::uart_comm(const char* portName) {
    // O_RDWR: 読み書き両方、O_NOCTTY: 制御端末にしない、O_NDELAY: ブロッキングしない
    fd = open(portName, O_RDWR | O_NOCTTY | O_NDELAY);
    
    if (fd == -1) {
        std::cerr << "Error opening serial port: " << strerror(errno) << std::endl;
        return;
    }
    
    // ブロッキングモードに設定
    fcntl(fd, F_SETFL, 0);
    
    struct termios options;
    // 現在の設定を取得
    if (tcgetattr(fd, &options) != 0) {
        std::cerr << "Error getting device attributes: " << strerror(errno) << std::endl;
        close(fd);
        fd = -1;
        return;
    }
    
    // ボーレートを設定
    speed_t baud = B4000000; 
    cfsetispeed(&options, baud);
    cfsetospeed(&options, baud);
    
    // 8N1設定 (8ビット、パリティなし、1ストップビット)
    options.c_cflag &= ~PARENB; // パリティなし
    options.c_cflag &= ~CSTOPB; // 1ストップビット
    options.c_cflag &= ~CSIZE;  // マスク文字サイズビット
    options.c_cflag |= CS8;     // 8ビット
    options.c_cflag |= CREAD | CLOCAL; // 受信を有効にし、モデム制御線を無視
    
    // ローカルオプションを設定
    options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG); // 非カノニカルモード
    
    // 入力オプションを設定
    options.c_iflag &= ~(IXON | IXOFF | IXANY); // ソフトウェアフロー制御を無効化
    options.c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL); // 入力処理を無効化
    
    // 出力オプションを設定
    options.c_oflag &= ~OPOST; // 出力処理を無効化
    
    // タイムアウトを設定
    options.c_cc[VMIN] = 0;  // 最小読み取りバイト数
    options.c_cc[VTIME] = 5; // 0.5秒のタイムアウト (単位は1/10秒)
    
    // 設定を適用
    if (tcsetattr(fd, TCSANOW, &options) != 0) {
        std::cerr << "Error setting device attributes: " << strerror(errno) << std::endl;
        close(fd);
        fd = -1;
        return;
    }
}

uart_comm::~uart_comm() {
    if (fd != -1) {
        close(fd);
    }
}

bool uart_comm::write_data(const char* data, size_t length) {
    ssize_t bytesWritten = write(fd, data, length);
    if (bytesWritten < 0) {
        std::cerr << "Error writing to serial port: " << strerror(errno) << std::endl;
        return false;
    }
    
    // すべてのバイトが書き込まれたか確認
    if (static_cast<size_t>(bytesWritten) != length) {
        std::cerr << "Warning: Not all bytes were written to the serial port" << std::endl;
    }
    
    return true;
}

bool uart_comm::read_data(char* buffer, size_t buffer_size, size_t& bytes_read) {
    ssize_t result = read(fd, buffer, buffer_size);
    
    if (result < 0) {
        std::cerr << "Error reading from serial port: " << strerror(errno) << std::endl;
        bytes_read = 0;
        return false;
    }
    
    bytes_read = static_cast<size_t>(result);
    return true;
}
#else // windows
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

    dcbSerialParams.BaudRate = CBR_1152000; //CBR_115200;
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
#endif
