#include "graphics.h"
#include "game.h"
#include <functional>
#include <cmath>

namespace graphics
{
	struct vertex
	{
		float x, y, z, rhw;
		DWORD color;
	};

	// Resolution size scale relative to 1280x720.
	float size_factor_x;
	float size_factor_y;

	// Convert worldspace coordinates to screenspace pixel coordinates.
	float world_to_screen_x(const int x)
	{
		return floorf((640.F + (float)(x - game_data.camera->final.pos_x) * game_data.camera->final.zoom_factor / game::COORD_TO_PIXEL) * size_factor_x + .5F) + .5F;
	}

	float world_to_screen_y(const int y)
	{
		return floorf((360.F + game::CAMERA_HEIGHT + (float)(y - game_data.camera->final.pos_y) * game_data.camera->final.zoom_factor / game::COORD_TO_PIXEL) * size_factor_y + .5F) + .5F;
	}

	// Restores device state to parameters saved in render_start.
	std::function<void(IDirect3DDevice9*)> render_end;

	// Set up the device for rendering.
	void render_start(IDirect3DDevice9 *device)
	{
		// Get the resolution scale.
		D3DVIEWPORT9 viewport;
		device->GetViewport(&viewport);
		size_factor_x = (float)(viewport.Width) / 1280.F;
		size_factor_y = (float)(viewport.Height) / 720.F;

		// Back up device state.
		DWORD alpha_blend_enable, dest_blend, src_blend, dest_blend_alpha, src_blend_alpha, fvf;
		IDirect3DPixelShader9 *pixel_shader;
		IDirect3DBaseTexture9 *texture;
		device->GetRenderState(D3DRS_ALPHABLENDENABLE, &alpha_blend_enable);
		device->GetRenderState(D3DRS_DESTBLEND, &dest_blend);
		device->GetRenderState(D3DRS_SRCBLEND, &src_blend);
		device->GetRenderState(D3DRS_DESTBLENDALPHA, &dest_blend_alpha);
		device->GetRenderState(D3DRS_SRCBLENDALPHA, &src_blend_alpha);
		device->GetPixelShader(&pixel_shader);
		device->GetFVF(&fvf);
		device->GetTexture(0, &texture);

		device->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
		device->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
		device->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
		device->SetRenderState(D3DRS_DESTBLENDALPHA, D3DBLEND_ONE);
		device->SetRenderState(D3DRS_SRCBLENDALPHA, D3DBLEND_ZERO);
		device->SetPixelShader(nullptr);
		device->SetFVF(D3DFVF_XYZRHW | D3DFVF_DIFFUSE);
		device->SetTexture(0, nullptr);

		render_end = [=](IDirect3DDevice9 *device)
		{
			// Restore device state.
			device->SetRenderState(D3DRS_ALPHABLENDENABLE, alpha_blend_enable);
			device->SetRenderState(D3DRS_DESTBLEND, dest_blend);
			device->SetRenderState(D3DRS_SRCBLEND, src_blend);
			device->SetRenderState(D3DRS_DESTBLENDALPHA, dest_blend_alpha);
			device->SetRenderState(D3DRS_SRCBLENDALPHA, src_blend_alpha);
			device->SetPixelShader(pixel_shader);
			device->SetFVF(fvf);
			device->SetTexture(0, texture);

			if (texture != nullptr)
				texture->Release();
		};
	}

	// Draw a box using worldspace coordinates.
	void draw_box(
		IDirect3DDevice9 *device,
		const int x1,
		const int y1,
		const int x2,
		const int y2,
		D3DCOLOR inner_color,
		D3DCOLOR outer_color)
	{
		// Convert to screenspace coordinates.
		float screen_x1 = world_to_screen_x(x1 > x2 ? x2 : x1);
		float screen_y1 = world_to_screen_y(y1);
		float screen_x2 = world_to_screen_x(x1 > x2 ? x1 : x2);
		float screen_y2 = world_to_screen_y(y2);


		vertex vertices[] =
		{
			{ screen_x1, screen_y1, 0.F, 0.F, inner_color },
			{ screen_x1, screen_y2, 0.F, 0.F, inner_color },
			{ screen_x2, screen_y1, 0.F, 0.F, inner_color },
			{ screen_x2, screen_y2, 0.F, 0.F, inner_color }
		};

		device->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, vertices, sizeof(vertex));

		vertex outline[] =
		{
			{ screen_x1, screen_y1 - 1.F, 0.F, 0.F, outer_color },
			{ screen_x1, screen_y2, 0.F, 0.F, outer_color },
			{ screen_x2, screen_y2, 0.F, 0.F, outer_color },
			{ screen_x2, screen_y1, 0.F, 0.F, outer_color },
			{ screen_x1, screen_y1, 0.F, 0.F, outer_color }
		};

		device->DrawPrimitiveUP(D3DPT_LINESTRIP, 4, outline, sizeof(vertex));
	}
}
