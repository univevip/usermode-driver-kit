#include <Windows.h>
#include <iostream>
#include <TlHelp32.h>

#include "driver.h"

int main()
{
	DWORD process_id = get_process_id("explorer.exe");


	if (open_driver_handle("explorer.exe"))
	{
		uintptr_t base = get_process_base();

		std::cout << process_id << "\n";
		std::cout << std::hex << base << std::dec << "\n";
		
		while(true) { }
	}
}