#include "pemsa/graphics/pemsa_graphics_module.hpp"
#include "pemsa/pemsa_emulator.hpp"

#include <cmath>
#include <cstring>

/*
 * I'm sorry to myself and whoever is reading this, but this is literally the only way
 * to do this. I can't cast lambdas with captured emulator variable to luaC_Function nor can I capture a method.
 * So this is my last resort - global variable, I'm sorry.
 */

static PemsaEmulator* emulator;

#define DRAW_PIXEL(x, y, c) if (drawStateModule->getFillPatternBit(x, y) == 0) { \
		memoryModule->setPixel(x, y, drawStateModule->getDrawColor(c), PEMSA_RAM_SCREEN); \
  } else if (!transparent) { \
		memoryModule->setPixel(x, y, drawStateModule->getDrawColor(c >> 4), PEMSA_RAM_SCREEN); \
  }

static int read_color(lua_State* state, int slot) {
	PemsaDrawStateModule* drawStateModule = emulator->getDrawStateModule();

	if (lua_isnumber(state, slot)) {
		int color = lua_tonumber(state, slot);
		drawStateModule->setColor(color);

		return color;
	}

	return drawStateModule->getColor();
}

static int flip(lua_State* state) {
	PemsaCartridgeModule* cartridgeModule = emulator->getCartridgeModule();

	std::unique_lock<std::mutex> uniqueLock(*cartridgeModule->getMutex());
	cartridgeModule->getLock()->wait(uniqueLock);

	return 0;
}

static int cls(lua_State* state) {
	int color = luaL_optnumber(state, 1, 0);

	memset(emulator->getMemoryModule()->ram + PEMSA_RAM_SCREEN, (color << 4) + color, 0x2000);
	return 0;
}

static int pset(lua_State* state) {
	int x = round(luaL_checknumber(state, 1));
	int y = round(luaL_checknumber(state, 2));

	int c = read_color(state, 3);
	// todo: take camera into account
	
	PemsaDrawStateModule* drawStateModule = emulator->getDrawStateModule();
	PemsaMemoryModule* memoryModule = emulator->getMemoryModule();
	
	bool transparent = drawStateModule->isFillPatternTransparent();
	
	DRAW_PIXEL(x, y, c)
	return 0;
}

static int pget(lua_State* state) {
	int x = round(luaL_checknumber(state, 1));
	int y = round(luaL_checknumber(state, 2));

	// TODO: take camera into account
	lua_pushnumber(state, emulator->getMemoryModule()->getPixel(x, y, PEMSA_RAM_SCREEN));
	return 1;
}

static int sset(lua_State* state) {
	int x = round(luaL_checknumber(state, 1));
	int y = round(luaL_checknumber(state, 2));

	if (!(x < 0 || y < 0 || x > 127 || y > 127)) {
		int c = read_color(state, 3);
		emulator->getMemoryModule()->setPixel(x, y, c % 16, PEMSA_RAM_GFX);
	}

	return 0;
}

static int sget(lua_State* state) {
	int x = round(luaL_checknumber(state, 1));
	int y = round(luaL_checknumber(state, 2));

	lua_pushnumber(state, emulator->getMemoryModule()->getPixel(x, y, PEMSA_RAM_GFX));
	return 1;
}

static inline void swap(int* a, int* b) {
	int tmp = *a;
	*a = *b;
	*b = tmp;
}

static void plot_line(int x0, int y0, int x1, int y1, int c) {
	PemsaDrawStateModule* drawStateModule = emulator->getDrawStateModule();
	PemsaMemoryModule* memoryModule = emulator->getMemoryModule();

	bool transparent = drawStateModule->isFillPatternTransparent();
	
	bool steep = false;

	if (abs(x1 - x0) < abs(y1 - y0)) {
		swap(&x0, &y0);
		swap(&x1, &y1);
		steep = true;
	}

	if (x0 > x1) {
		swap(&x0, &x1);
		swap(&y0, &y1);
	}

	int dx = x1 - x0;
	int dy = y1 - y0;
	int d_err = 2 * abs(dy);
	int err = 0;
	int y = y0;

	for (int x = x0; x <= x1; x++) {
		if (steep) {
			DRAW_PIXEL(x, y, c)
		} else {
			DRAW_PIXEL(x, y, c)
		}

		err += d_err;

		if (err > dx) {
			y += y1 > y0 ? 1 : -1;
			err -= dx * 2;
		}
	}
}

static int line(lua_State* state) {
	int x0 = round(luaL_checknumber(state, 1));
	int y0 = round(luaL_checknumber(state, 2));
	int x1 = round(luaL_checknumber(state, 3));
	int y1 = round(luaL_checknumber(state, 4));

	int c =  read_color(state, 5);
	plot_line(x0, y0, x1, y1, c);

	return 0;
}

static int rect(lua_State* state) {
	int x0 = round(luaL_checknumber(state, 1));
	int y0 = round(luaL_checknumber(state, 2));
	int x1 = round(luaL_checknumber(state, 3));
	int y1 = round(luaL_checknumber(state, 3));

	int c = read_color(state, 5);

	plot_line(x0, y0, x1, y0, c);
	plot_line(x0, y1, x1, y1, c);
	plot_line(x0, y0, x0, y1, c);
	plot_line(x1, y0, x1, y1, c);

	return 0;
}

static int rectfill(lua_State* state) {
	int x0 = round(luaL_checknumber(state, 1));
	int y0 = round(luaL_checknumber(state, 2));
	int x1 = round(luaL_checknumber(state, 3));
	int y1 = round(luaL_checknumber(state, 3));

	if (x0 > x1) {
		swap(&x0, &x1);
	}

	if (y0 > y1) {
		swap(&y0, &y1);
	}

	int c = read_color(state, 5);

	PemsaDrawStateModule* drawStateModule = emulator->getDrawStateModule();
	PemsaMemoryModule* memoryModule = emulator->getMemoryModule();

	bool transparent = drawStateModule->isFillPatternTransparent();

	for (int y = y0; y <= y1; y++) {
		for (int x = x0; x <= x1; x++) {
			DRAW_PIXEL(x, y, c)
		}
	}

	return 0;
}

static int circ(lua_State* state) {
	int ox = round(luaL_checknumber(state, 1));
	int oy = round(luaL_checknumber(state, 2));
	int r = round(luaL_optnumber(state, 3, 1));

	int c = read_color(state, 4);

	int x = r;
	int y = 0;
	int decisionOver2 = 1 - x;

	PemsaDrawStateModule* drawStateModule = emulator->getDrawStateModule();
	PemsaMemoryModule* memoryModule = emulator->getMemoryModule();

	bool transparent = drawStateModule->isFillPatternTransparent();

	while (y <= x) {
		DRAW_PIXEL(ox + x, oy + y, c)
		DRAW_PIXEL(ox + y, oy + x, c)
		DRAW_PIXEL(ox - x, oy + y, c)
		DRAW_PIXEL(ox - y, oy + x, c)

		DRAW_PIXEL(ox - x, oy - y, c)
		DRAW_PIXEL(ox - y, oy - x, c)
		DRAW_PIXEL(ox + x, oy - y, c)
		DRAW_PIXEL(ox + y, oy - x, c)
		
		y++;

		if (decisionOver2 < 0) {
			decisionOver2 += 2 * y + 1;
		} else {
			x--;
			decisionOver2 += 2 * (y - x) + 1;
		}
	}

	return 0;
}

static inline void horizontalLine(int x0, int y, int x1, int c) {
	PemsaDrawStateModule* drawStateModule = emulator->getDrawStateModule();
	PemsaMemoryModule* memoryModule = emulator->getMemoryModule();

	bool transparent = drawStateModule->isFillPatternTransparent();

	for (int x = x0; x <= x1; x++) {
		DRAW_PIXEL(x, y, c)
	}
}

static inline void plot(int cx, int cy, int x, int y, int c) {
	horizontalLine(cx - x, cy + y, cx + x, c);

	if (y != 0) {
		horizontalLine(cx - x, cy - y, cx + x, c);
	}
}

static int circfill(lua_State* state) {
	int ox = round(luaL_checknumber(state, 1));
	int oy = round(luaL_checknumber(state, 2));
	int r = round(luaL_optnumber(state, 3, 1));

	int c = read_color(state, 4);

	int x = r;
	int y = 0;
	int error = 1 - r;

	while (y <= x) {
		plot(ox, oy, x, y, c);

		if (error < 0) {
			error += 2 * y + 3;
		} else {
			if (x != y) {
				plot(ox, oy, y, x, c);
			}

			x--;
			error += 2 * (y - x) + 3;
		}

		y++;
	}

	return 0;
}

// spr( n, x, y, [w,] [h,] [flip_x,] [flip_y] )
static int spr(lua_State* state) {
	int n = luaL_checknumber(state, 1);

	if (n < 0 || n > 255) {
		return 0;
	}

	int x = round(luaL_optnumber(state, 2, 0));
	int y = round(luaL_optnumber(state, 3, 0));

	// TODO: subtract camera position from x & y

	int sprX = (n & 0x0f) << 3;
	int sprY = (n >> 4) << 3;
	int width = luaL_optnumber(state, 4, 1);
	int height = luaL_optnumber(state, 5, 1);

	bool flipX = pemsa_optional_bool(state, 6, false);
	bool flipY = pemsa_optional_bool(state, 7, false);

	PemsaMemoryModule* memoryModule = emulator->getMemoryModule();
	PemsaDrawStateModule* drawStateModule = emulator->getDrawStateModule();

	for (int i = 0; i < 8 * width; i++) {
		for (int j = 0; j < 8 * height; j++) {
			int color = memoryModule->getPixel(i + sprX, j + sprY, PEMSA_RAM_GFX) & 0x0f;

			if (drawStateModule->isTransparent(color)) {
				continue;
			}

			memoryModule->setPixel((int) x + (flipX ? 8 * width - i : i), (int) y + (flipY ? 8 * height - j : j), drawStateModule->getDrawColor(color), PEMSA_RAM_SCREEN);
		}
	}

	return 0;
}

void pemsa_open_graphics_api(PemsaEmulator* machine, lua_State* state) {
	emulator = machine;

	lua_register(state, "flip", flip);
	lua_register(state, "cls", cls);
	lua_register(state, "pset", pset);
	lua_register(state, "pget", pget);
	lua_register(state, "sset", sset);
	lua_register(state, "sget", sget);
	lua_register(state, "line", line);
	lua_register(state, "rect", rect);
	lua_register(state, "rectfill", rectfill);
	lua_register(state, "circ", circ);
	lua_register(state, "circfill", circfill);
	lua_register(state, "spr", spr);
}