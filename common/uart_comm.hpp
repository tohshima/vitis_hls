#ifndef UART_COMM_HPP
#define UART_COMM_HPP

#if 1 // linux

#include <string>

#define UART_COMM_EOF (0x1A)

class uart_comm {
private:
    int fd; // ファイルディスクリプタ

public:
    uart_comm(const char* portName);
    ~uart_comm();
    bool write_data(const char* data, size_t length);
    bool read_data(char* buffer, size_t buffer_size, size_t& bytes_read);
};

#ifdef UART_COMM_TEST
#include <iostream>
#if 0
int main() {
    uart_comm uart("/dev/ttyUSB0"); // Linuxではデバイスパスが異なります
    char buffer[256];
    size_t bytesRead;
    
    if (uart.read_data(buffer, sizeof(buffer), bytesRead)) {
        buffer[bytesRead] = '\0';  // Null-terminate the string
        std::cout << "Received: " << buffer << std::endl;
    }
    return 0;
}
#endif
#endif

#else // windows
#include <windows.h>
#include <iostream>
#include <string>

#define UART_COMM_EOF (0x1A)

class uart_comm {
private:
    HANDLE hSerial;

public:
    uart_comm(const char* portName);
    ~uart_comm();

    bool write_data(const char* data, size_t length);
    bool read_data(char* buffer, size_t buffer_size, DWORD& bytes_read);
};

#if 0
int main() {
    uart_comm uart_comm("\\\\.\\COM3");  // COM3�|�[�g���J��

    // �f�[�^���M
    const char* message = "Hello, Serial Port!";
    if (uart_comm.write_data(message, strlen(message))) {
        std::cout << "Data sent successfully\n";
    }

    // �f�[�^��M
    char buffer[256];
    DWORD bytesRead;
    if (uart_comm.read_data(buffer, sizeof(buffer), bytesRead)) {
        buffer[bytesRead] = '\0';  // Null-terminate the string
        std::cout << "Received: " << buffer << std::endl;
    }

    return 0;
}
#endif
#endif
#endif // UART_COMM_HPP
