#include "util.h"
#include <stdexcept>
#include <Windows.h>
#include <Psapi.h>

namespace util
{
	// Get the start and end address of a loaded module. Returns success.
	bool get_module_bounds(const char *name, uintptr_t *start, uintptr_t *end)
	{
		const auto module = GetModuleHandle(name);
		if (module == nullptr)
			return false;

		MODULEINFO info;
		GetModuleInformation(GetCurrentProcess(), module, &info, sizeof(info));
		*start = (uintptr_t)(info.lpBaseOfDll);
		*end = *start + info.SizeOfImage;
		return true;
	}

	// Scan for a byte pattern with a mask in the form of "xxx???xxx".
	uintptr_t sigscan(const char *name, const char *sig, const char *mask)
	{
		uintptr_t start, end;
		if (!get_module_bounds(name, &start, &end))
			throw std::runtime_error("Module not loaded");

		const auto last_scan = end - strlen(mask) + 1;

		for (auto addr = start; addr < last_scan; addr++) {
			for (size_t i = 0;; i++) {
				if (mask[i] == '\0')
					return addr;
				if (mask[i] != '?' && sig[i] != *(char*)(addr + i))
					break;
			}
		}

		throw std::runtime_error("Sigscan failed");
	}

	// Replace a function in a virtual function table. Returns the original function.
	const void *hook_vtable(const void **vtable, const int index, const void *new_function)
	{
		DWORD old_protect;
		VirtualProtect(&vtable[index], sizeof(void*), PAGE_READWRITE, &old_protect);
		const auto *orig = vtable[index];
		vtable[index] = new_function;
		VirtualProtect(&vtable[index], sizeof(void*), old_protect, &old_protect);

		return orig;
	}
}