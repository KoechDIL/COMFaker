#include <iostream>
#include <fstream>
#include <string>
#include <chrono>
#include <thread>

#ifdef _WIN32
#include <windows.h>
#else
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#endif

void print_message(const std::string& message)
{
	std::cout << "Received message: " << message << std::endl;
}

int main()
{
#ifdef _WIN32
	// Windows implementation using the Win32 API

	HANDLE hSerial;
	DWORD bytesRead;
	char buffer[1024] = { 0 };

	std::wstring wideFilename;
	int length = MultiByteToWideChar(CP_UTF8, 0, "COM4", -1, NULL, 0);
	if (length > 0)
	{
		wideFilename.resize(length);
		MultiByteToWideChar(CP_UTF8, 0, "COM4", -1, &wideFilename[0], length);
	}

	// Open the serial port for reading
	hSerial = CreateFile(wideFilename.c_str(), GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hSerial == INVALID_HANDLE_VALUE)
	{
		std::cerr << "Failed to open serial port" << std::endl;
		return 1;
	}

	// Set the serial port parameters
	DCB dcbSerialParams = { 0 };
	dcbSerialParams.DCBlength = sizeof(dcbSerialParams);
	if (!GetCommState(hSerial, &dcbSerialParams))
	{
		std::cerr << "Failed to get serial port state" << std::endl;
		CloseHandle(hSerial);
		return 1;
	}
	dcbSerialParams.BaudRate = CBR_9600;
	dcbSerialParams.ByteSize = 8;
	dcbSerialParams.StopBits = ONESTOPBIT;
	dcbSerialParams.Parity = NOPARITY;
	if (!SetCommState(hSerial, &dcbSerialParams))
	{
		std::cerr << "Failed to set serial port state" << std::endl;
		CloseHandle(hSerial);
		return 1;
	}

	while (true)
	{
		// Read data from the serial port
		if (ReadFile(hSerial, buffer, sizeof(buffer), &bytesRead, NULL))
		{
			std::string message(buffer, bytesRead);
			print_message(message);
		}
		else
		{
			std::cerr << "Failed to read from serial port" << std::endl;
		}

		// Wait for a short period before reading again
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}

	// Close the serial port
	CloseHandle(hSerial);
#else
	// Linux implementation using standard POSIX functions

	int fd;
	ssize_t bytesRead;
	char buffer[1024] = { 0 };

	// Open the serial port for reading
	fd = open("/dev/ttyUSB0", O_RDONLY | O_NOCTTY);
	if (fd == -1)
	{
		std::cerr << "Failed to open serial port" << std::endl;
		return 1;
	}

	// Set the serial port parameters
	struct termios tty;
	tcgetattr(fd, &tty);
	cfsetospeed(&tty, B9600);
	cfmakeraw(&tty);
	tcsetattr(fd, TCSANOW, &tty);

	while (true)
	{
		// Read data from the serial port
		bytesRead = read(fd, buffer, sizeof(buffer));
		if (bytesRead > 0)
		{
			std::string message(buffer, bytesRead);
			print_message(message);
		}
		else
		{
			std::cerr << "Failed to read from serial port" << std::endl;
		}

		// Wait for a short period before reading again
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}

	// Close the serial port
	close(fd);
#endif

	return 0;
}
