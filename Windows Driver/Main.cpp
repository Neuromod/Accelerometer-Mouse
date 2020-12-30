#include <cstdlib>

#include <iostream>
#include <iomanip>
#include <queue>

#include "windows.h"


constexpr char port[] = "COM11";

constexpr DWORD baudRate = 115200;
constexpr BYTE  byteSize = 8;
constexpr BYTE  stopBits = ONESTOPBIT;
constexpr BYTE  parity   = NOPARITY;


HANDLE openSerial(std::string port, DWORD baudRate, BYTE byteSize, BYTE stopBits, BYTE parity);
void readSerial(HANDLE handle, std::queue<unsigned char>* queue);
void updateMouse(int dx, int dy, bool left, bool middle, bool right);



int main()
{

    std::cout << std::fixed << std::setprecision(3) << std::showpos;

    HANDLE handle = openSerial(port, baudRate, byteSize, stopBits, parity);

    std::queue<unsigned char> buffer;

    double sumX = .0;
    double sumY = .0;
    double sumZ = .0;

    while (true)
    {
        readSerial(handle, &buffer);

        if (buffer.size() >= 10)
        {
            unsigned long data[10];
            
            if (buffer.front() & 0x80)
            {
                for (int i = 0; i < 10; i++)
                {
                    data[i] = buffer.front();
                    buffer.pop();
                }
                
                double x = (double(data[1] | (data[2] << 7) | (data[3] & 0x3F) << 14) / double(1 << 16)) * (data[3] & 0x40 ? -1.0 : 1.0);
                double y = (double(data[4] | (data[5] << 7) | (data[6] & 0x3F) << 14) / double(1 << 16)) * (data[6] & 0x40 ? -1.0 : 1.0);
                double z = (double(data[7] | (data[8] << 7) | (data[9] & 0x3F) << 14) / double(1 << 16)) * (data[9] & 0x40 ? -1.0 : 1.0);

                double r = std::sqrt(x * x + y * y + z * z);

                x /= r;
                y /= r;
                z /= r;

                sumX += (std::exp(std::fabs(x)) - 1.) * (x > 0. ? 1 : -1) * 1.5;
                sumY += (std::exp(std::fabs(y)) - 1.) * (y > 0. ? 1 : -1) * 2.;
                sumZ += (std::exp(std::fabs(z)) - 1.) * (z > 0. ? 1 : -1) * 2.;

                bool b0 =  data[0]       & 0x01;
                bool b1 = (data[0] >> 1) & 0x01;
                bool b2 = (data[0] >> 2) & 0x01;
                bool b3 = (data[0] >> 3) & 0x01;

                updateMouse(int(sumX), int(sumY) , b0, b2, b1);

                sumX -= int(sumX);
                sumY -= int(sumY);
                sumZ -= int(sumZ);

                // ANSI Escape sequence to move to the home position
                //std::cout << "\x1B" "\x5B" ";H";        
                
                //std::cout << (data[0] & 0x01) << (data[0] >> 1 & 0x01) << (data[0] >> 2 & 0x01) << (data[0] >> 3 & 0x01) << ", " << xNew << ", " << yNew << ", " << zNew << '\n';

            }
            else
                buffer.pop();
        }
    }
}



HANDLE openSerial(std::string port, DWORD baudRate, BYTE byteSize, BYTE stopBits, BYTE parity)
{
    HANDLE handle;

    handle = CreateFileA((std::string("\\\\.\\") + port).c_str(), GENERIC_READ | GENERIC_WRITE, 0, 0, OPEN_EXISTING, 0, 0);

    if (handle == INVALID_HANDLE_VALUE)
    {
        std::cout << "Could not open port\n";
        exit(1);
    }

    DCB dcb;
    dcb.DCBlength = sizeof(dcb);

    if (!GetCommState(handle, &dcb))
    {
        std::cout << "Could not read port parameters\n";
        exit(1);
    }

    dcb.BaudRate = baudRate;
    dcb.ByteSize = byteSize;
    dcb.StopBits = stopBits;
    dcb.Parity = parity;

    if (!SetCommState(handle, &dcb))
    {
        std::cout << "Could not read port parameters\n";
        exit(1);
    }

    COMMTIMEOUTS timeouts;

    timeouts.ReadIntervalTimeout = MAXDWORD;
    timeouts.ReadTotalTimeoutConstant = 0;
    timeouts.ReadTotalTimeoutMultiplier = 0;
    timeouts.WriteTotalTimeoutConstant = 0;
    timeouts.WriteTotalTimeoutMultiplier = 0;

    if (!SetCommTimeouts(handle, &timeouts))
    {
        std::cout << "Could not set port timeouts\n";
        exit(1);
    }

    return handle;
}


void readSerial(HANDLE handle, std::queue<unsigned  char>* buffer)
{
    unsigned char data;
    DWORD readBytes;

    while (true)
    {
        if (!ReadFile(handle, &data, sizeof(data), &readBytes, 0))
        {
            std::cout << "Could not read data\n";
            exit(-1);
        }

        if (readBytes)
            buffer->push(data);
        else
            break;
    }
}


void updateMouse(int dx, int dy, bool left, bool middle, bool right)
{
    static bool currentLeft   = false;
    static bool currentMiddle = false;
    static bool currentRight  = false;

    INPUT input;

    input.type = INPUT_MOUSE;
    input.mi.dx = dx;
    input.mi.dy = dy;
    input.mi.mouseData = 0;
    input.mi.dwFlags = MOUSEEVENTF_MOVE | MOUSEEVENTF_VIRTUALDESK;
    input.mi.time = 0;
    input.mi.dwExtraInfo = 0;

    if (currentLeft != left)
    {
        if (left)
            input.mi.dwFlags |= MOUSEEVENTF_LEFTDOWN;
        else
            input.mi.dwFlags |= MOUSEEVENTF_LEFTUP;

        currentLeft = left;
    }
        
    if (currentMiddle != middle)
    {
        if (middle)
            input.mi.dwFlags |= MOUSEEVENTF_MIDDLEDOWN;
        else
            input.mi.dwFlags |= MOUSEEVENTF_MIDDLEUP;

        currentMiddle = middle;
    }
            
    if (currentRight != right)
    {
        if (right)
            input.mi.dwFlags |= MOUSEEVENTF_RIGHTDOWN;
        else
            input.mi.dwFlags |= MOUSEEVENTF_RIGHTUP;

        currentRight = right;
    }

    SendInput(1, &input, sizeof(input));
}