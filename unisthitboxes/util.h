#pragma once

#include <cstdint>

namespace util
{
	// Get the start and end address of a loaded module. Returns success.
	bool get_module_bounds(const char *name, uintptr_t *start, uintptr_t *end);

	// Scan for a byte pattern with a mask in the form of "xxx???xxx".
	uintptr_t sigscan(const char *name, const char *sig, const char *mask);

	// Replace a function in a virtual function table. Returns the original function.
	const void *hook_vtable(const void **vtable, int index, const void *new_function);
}