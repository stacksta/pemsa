#include "pemsa/graphics/pemsa_graphics_module.hpp"
#include "pemsa/util/pemsa_font.hpp"
#include "pemsa/pemsa_emulator.hpp"

#include <cmath>
#include <cstring>
#include <iostream>

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

	if (lua_gettop(state) >= slot && lua_isnumber(state, slot)) {
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

	PemsaDrawStateModule* drawStateModule = emulator->getDrawStateModule();
	PemsaMemoryModule* memoryModule = emulator->getMemoryModule();

	bool transparent = drawStateModule->isFillPatternTransparent();

	DRAW_PIXEL(x - drawStateModule->getCameraX(), y - drawStateModule->getCameraY(), c)

	return 0;
}

static int pget(lua_State* state) {
	int x = round(luaL_checknumber(state, 1));
	int y = round(luaL_checknumber(state, 2));

	PemsaDrawStateModule* drawStateModule = emulator->getDrawStateModule();
	lua_pushnumber(state, emulator->getMemoryModule()->getPixel(x - drawStateModule->getCameraX(), y - drawStateModule->getCameraY(), PEMSA_RAM_SCREEN));

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
	int dError = 2 * abs(dy);
	int err = 0;
	int y = y0;

	int cx = drawStateModule->getCameraX();
	int cy = drawStateModule->getCameraY();

	for (int x = x0; x <= x1; x++) {
		if (steep) {
			DRAW_PIXEL(y - cx, x - cy, c)
		} else {
			DRAW_PIXEL(x - cx, y - cy, c)
		}

		err += dError;

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
	int y1 = round(luaL_checknumber(state, 4));

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
	int y1 = round(luaL_checknumber(state, 4));

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

static int oval(lua_State* state) {
	int x0 = round(luaL_checknumber(state, 1));
	int y0 = round(luaL_checknumber(state, 2));
	int x1 = round(luaL_checknumber(state, 3));
	int y1 = round(luaL_checknumber(state, 4));
	int c = read_color(state, 5);

	int width = abs(round((x0 - x1) / 2));
	int height = abs(round((y0 - y1) / 2));
	int ox = round(fmin(x0, x1) + width);
	int oy = round(fmin(y0, y1) + height);

	PemsaDrawStateModule *drawStateModule = emulator->getDrawStateModule();
	PemsaMemoryModule *memoryModule = emulator->getMemoryModule();

	bool transparent = drawStateModule->isFillPatternTransparent();

#define incx() x++, dxt += d2xt, t += dxt
#define incy() y--, dyt += d2yt, t += dyt

	int x = 0;
	int y = height;
	long a2 = (long) width * width;
	long b2 = (long) height * height;
	long crit1 = -(a2 / 4 + width % 2 + b2);
	long crit2 = -(b2 / 4 + height % 2 + a2);
	long crit3 = -(b2 / 4 + height % 2);
	long t = -a2 * y;
	long dxt = 2 * b2 * x, dyt = -2 * a2 * y;
	long d2xt = 2 * b2, d2yt = 2 * a2;

	while (y >= 0 && x <= width) {
		DRAW_PIXEL(ox + x, oy + y, c)

		if (x != 0 || y != 0) {
			DRAW_PIXEL(ox - x, oy - y, c)

			if (x != 0 && y != 0) {
				DRAW_PIXEL(ox + x, oy - y, c)
				DRAW_PIXEL(ox - x, oy + y, c)
			}
		}

		if (t + b2 * x <= crit1 || t + a2 * y <= crit3) {
			incx();
		} else if (t - a2 * y > crit2) {
			incy();
		} else {
			incx();
			incy();
		}
	}

#undef incx
#undef incy

	return 0;
}

static int ovalfill(lua_State* state) {
	double x0 = luaL_checknumber(state, 1);
	double y0 = luaL_checknumber(state, 2);
	double x1 = luaL_checknumber(state, 3);
	double y1 = luaL_checknumber(state, 4);
	int c = read_color(state, 5);

	int width = abs(round((x0 - x1) / 2));
	int height = abs(round((y0 - y1) / 2));
	int ox = round(fmin(x0, x1) + width);
	int oy = round(fmin(y0, y1) + height);

	PemsaDrawStateModule *drawStateModule = emulator->getDrawStateModule();
	PemsaMemoryModule *memoryModule = emulator->getMemoryModule();

	bool transparent = drawStateModule->isFillPatternTransparent();

	int hh = height * height;
	int ww = width * width;
	int hhww = (hh + 1) * (ww + 1);
	x0 = width;
	int dx = 0;

	for (int x = -width; x <= width; x++) {
		DRAW_PIXEL(ox + x, oy, c)
	}

	for (int y = 1; y <= height; y++) {
		x1 = x0 - (dx - 1);

		for (; x1 > 0; x1--) {
			if (x1 * x1 * hh + y * y * ww <= hhww) {
				break;
			}
		}

		dx = x0 - x1;
		x0 = x1;

		for (int x = -x0; x <= x0; x++) {
			DRAW_PIXEL(ox + x, oy - y, c)
			DRAW_PIXEL(ox + x, oy + y, c)
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

	PemsaDrawStateModule* drawStateModule = emulator->getDrawStateModule();

	ox -= drawStateModule->getCameraX();
	oy -= drawStateModule->getCameraY();

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

static void plot_sprite(int n, int x, int y, int width, int height, bool flipX, bool flipY) {
	int sprX = (n & 0x0f) << 3;
	int sprY = (n >> 4) << 3;

	PemsaMemoryModule* memoryModule = emulator->getMemoryModule();
	PemsaDrawStateModule* drawStateModule = emulator->getDrawStateModule();

	x -= drawStateModule->getCameraX();
	y -= drawStateModule->getCameraY();

	for (int i = 0; i < 8 * width; i++) {
		for (int j = 0; j < 8 * height; j++) {
			int color = memoryModule->getPixel(i + sprX, j + sprY, PEMSA_RAM_GFX) & 0x0f;

			if (drawStateModule->isTransparent(color)) {
				continue;
			}

			memoryModule->setPixel((int) x + (flipX ? 8 * width - i : i), (int) y + (flipY ? 8 * height - j : j), drawStateModule->getDrawColor(color), PEMSA_RAM_SCREEN);
		}
	}
}

static int spr(lua_State* state) {
	int n = luaL_checknumber(state, 1);

	if (n < 0 || n > 255) {
		return 0;
	}

	int x = round(luaL_optnumber(state, 2, 0));
	int y = round(luaL_optnumber(state, 3, 0));
	int width = luaL_optnumber(state, 4, 1);
	int height = luaL_optnumber(state, 5, 1);

	bool flipX = pemsa_optional_bool(state, 6, false);
	bool flipY = pemsa_optional_bool(state, 7, false);

	plot_sprite(n, x, y, width, height, flipX, flipY);
	return 0;
}

// sspr( sx, sy, sw, sh, dx, dy, [dw,] [dh,] [flip_x,] [flip_y] )
static int sspr(lua_State* state) {
	int sx = round(luaL_checknumber(state, 1));
	int sy = round(luaL_checknumber(state, 2));
	int sw = round(luaL_checknumber(state, 3));
	int sh = round(luaL_checknumber(state, 4));
	int dx = round(luaL_checknumber(state, 5));
	int dy = round(luaL_checknumber(state, 6));
	int dw = round(luaL_optnumber(state, 7, sw));
	int dh = round(luaL_optnumber(state, 8, sh));

	bool flipX = pemsa_optional_bool(state, 9, false);
	bool flipY = pemsa_optional_bool(state, 10, false);

	if (sw < 0) {
		sw *= -1;
		sx -= sw;
		flipX = !flipX;
	}

	if (sh < 0) {
		sh *= -1;
		sy -= sh;
		flipY = !flipY;
	}

	if (dw < 0) {
		dw *= -1;
		dx -= dw;
		flipX = !flipX;
	}

	if (dh < 0) {
		dh *= -1;
		dy -= dh;
		flipY = !flipY;
	}

	float ratioX = sw / (float) dw;
	float ratioY = sh / (float) dh;
	float x = sx;
	double screenX = dx;
	float y;
	double screenY;

	PemsaMemoryModule* memoryModule = emulator->getMemoryModule();
	PemsaDrawStateModule* drawStateModule = emulator->getDrawStateModule();

	dx -= drawStateModule->getCameraX();
	dy -= drawStateModule->getCameraY();

	while (x < sx + sw && screenX < dx + dw) {
		y = sy;
		screenY = dy;

		while (y < sy + sh && screenY < dy + dh) {
			int color = memoryModule->getPixel(x, y, PEMSA_RAM_GFX) & 0x0f;

			if (!drawStateModule->isTransparent(color)) {
				memoryModule->setPixel((int) (flipX ? dx + dw - ((int) screenX - dx) : (int) screenX), (int) (flipY ? dy + dh - ((int) screenY - dy) : (int) screenY), drawStateModule->getDrawColor(color), PEMSA_RAM_SCREEN);
			}

			y += ratioY;
			screenY += 1;
		}

		x += ratioX;
		screenX += 1;
	}

	return 0;
}

static int map(lua_State* state) {
	int mx = luaL_optint(state, 1, 0);
	int my = luaL_optint(state, 2, 0);
	int x = luaL_optint(state, 3, 0);
	int y = luaL_optint(state, 4, 0);
	int mw = luaL_optint(state, 5, 16);
	int mh = luaL_optint(state, 6, 16);
	int layer = luaL_optinteger(state, 7, 0);

	PemsaMemoryModule* memoryModule = emulator->getMemoryModule();
	uint8_t* ram = memoryModule->ram;

	for (int cy = fmax(0, my); cy < fmin(64, my + mh); cy++) {
		for (int cx = fmax(0, mx); cx < fmin(128, mx + mw); cx++) {
			int tile = ram[(cy > 31 ? PEMSA_RAM_GFX : PEMSA_RAM_MAP) + cx + cy * 128];

			if (tile == 0) {
				continue;
			} else if (layer != 0) {
				int flagMask = ram[PEMSA_RAM_GFX_PROPS + tile];
				bool found = false;

				for (int i = 0; i < 8; i++) {
					if ((layer & (1 << i)) == 1 && (flagMask & (1 << i)) == 0) {
						found = true;
						break;
					}
				}

				if (found) {
					continue;
				}
			}

			plot_sprite(tile, (cx - mx) * 8 + x, (cy - my) * 8 + y, 1, 1, false, false);
		}
	}

	return 0;
}

static int print(lua_State* state) {
	if (lua_gettop(state) == 0) {
		return 0;
	}

	if (lua_isnil(state, 1)) {
		return 0;
	}

	const char* text = pemsa_to_string(state, 1);

	PemsaDrawStateModule* drawStateModule = emulator->getDrawStateModule();
	PemsaMemoryModule* memoryModule = emulator->getMemoryModule();

	int x;
	int y;
	int c;
	bool givenCoords = lua_gettop(state) > 1;

	if (givenCoords) {
		x = round(luaL_checknumber(state, 2)) - drawStateModule->getCameraX();
		y = round(luaL_checknumber(state, 3)) - drawStateModule->getCameraY();
		c = read_color(state, 4);
	} else {
		x = drawStateModule->getCursorX();
		y = drawStateModule->getCursorY();
		c = drawStateModule->getColor();

		if (y + 18 >= 127) {
			memcpy(memoryModule->ram + PEMSA_RAM_SCREEN, memoryModule->ram + PEMSA_RAM_SCREEN + 0x180, 0x2000 - 0x180);
			memset(memoryModule->ram + PEMSA_RAM_SCREEN + 0x1e80, 0, 0x180);
			drawStateModule->setCursorY(y);
		} else {
			drawStateModule->setCursorY(y + 6);
		}

	}

	int index = 0;

	bool transparent = false;
	int offsetX = 0;
	int offsetY = 0;

	while (*text != '\0') {
		char cr = *text++;

		if (cr == '\n') {
			offsetX = 0;
			offsetY += 6;

			continue;
		} else if (cr >= 'A' && cr <= 'z') {
			if (cr >= 'a') {
				cr = toupper(cr);
			} else {
				cr = tolower(cr);
			}
		}

		const char** letter = pemsa_get_letter(cr);

		if (letter != nullptr) {
			int w = cr < '!' ? 7 : 3;

			for (int ly = 0; ly < 5; ly++) {
				for (int lx = 0; lx < w; lx++) {
					if (letter[ly][lx] == 'x') {
						DRAW_PIXEL(offsetX + x + lx, offsetY + y + ly, c)
					}
				}
			}

			offsetX += w + 1;
		} else {
			offsetX += 4;
		}

		index++;
	}

	if (givenCoords) {
		drawStateModule->setCursorX(x);
		drawStateModule->setCursorY(y);
	}

	return 0;
}

static int tline(lua_State* state) {
	int x0 = round(luaL_checknumber(state, 1));
	int y0 = round(luaL_checknumber(state, 2));
	int x1 = round(luaL_checknumber(state, 3));
	int y1 = round(luaL_checknumber(state, 4));

	double mx = luaL_optnumber(state, 5, 0);
	double my = luaL_optnumber(state, 6, 0);
	double mdx = luaL_optnumber(state, 7, 0.125);
	double mdy = luaL_optnumber(state, 8, 0);

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
	int dError = 2 * abs(dy);
	int err = 0;
	int y = y0;

	PemsaDrawStateModule* drawStateModule = emulator->getDrawStateModule();
	PemsaMemoryModule* memoryModule = emulator->getMemoryModule();

	int cx = drawStateModule->getCameraX();
	int cy = drawStateModule->getCameraY();

	for (int x = x0; x <= x1; x++) {
		int c = 0;

		int imx = floor(mx);
		int imy = floor(my);

		if (imx >= 0 && imy >= 0 && imx < 128 && imy < 64) {
			int tile = memoryModule->ram[(imy > 31 ? PEMSA_RAM_GFX : PEMSA_RAM_MAP) + imx + imy * 128];

			int sx = ((tile & 0x0f) << 3) + (fmod(mx, 1) * 8);
			int sy = ((tile >> 4) << 3) + (fmod(my, 1) * 8);

			if (sx >= 0 && sy >= 0 && sx < 128 && sy < 128) {
				c = memoryModule->getPixel(sx, sy, PEMSA_RAM_GFX);
			}
		}

		if (!drawStateModule->isTransparent(c)) {
			if (steep) {
				memoryModule->setPixel(y - cx, x - cy, c, PEMSA_RAM_SCREEN);
			} else {
				memoryModule->setPixel(x - cx, y - cy, c, PEMSA_RAM_SCREEN);
			}
		}

		err += dError;

		if (err > dx) {
			y += y1 > y0 ? 1 : -1;
			err -= dx * 2;
		}

		mx += mdx;
		my += mdy;
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
	lua_register(state, "oval", oval);
	lua_register(state, "ovalfill", ovalfill);
	lua_register(state, "spr", spr);
	lua_register(state, "sspr", sspr);
	lua_register(state, "map", map);
	lua_register(state, "print", print);
	lua_register(state, "tline", tline);
}