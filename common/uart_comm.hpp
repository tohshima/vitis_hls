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
    uart_comm uart_comm("\\\\.\\COM3");  // COM3ポートを開く

    // データ送信
    const char* message = "Hello, Serial Port!";
    if (uart_comm.write_data(message, strlen(message))) {
        std::cout << "Data sent successfully\n";
    }

    // データ受信
    char buffer[256];
    DWORD bytesRead;
    if (uart_comm.read_data(buffer, sizeof(buffer), bytesRead)) {
        buffer[bytesRead] = '\0';  // Null-terminate the string
        std::cout << "Received: " << buffer << std::endl;
    }

    return 0;
}
#endif
