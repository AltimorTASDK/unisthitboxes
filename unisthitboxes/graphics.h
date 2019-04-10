#pragma once

#include <d3d9.h>
#include <functional>

namespace graphics
{
	// Restores device state to parameters saved in render_start.
	extern std::function<void(IDirect3DDevice9*)> render_end;

	// Set up the device for rendering.
	void render_start(IDirect3DDevice9 *device);

	// Draw a box using worldspace coordinates.
	void draw_box(
		IDirect3DDevice9 *device,
		int x1,
		int y1,
		int x2,
		int y2,
		D3DCOLOR inner_color,
		D3DCOLOR outer_color);
}
