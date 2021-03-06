#include "pemsa/memory/pemsa_memory_module.hpp"
#include "pemsa/pemsa_emulator.hpp"

#include <iostream>

static PemsaEmulator* emulator;

static int fget(lua_State* state) {
	int tile = luaL_checknumber(state, 1);

	if (tile < 0 || tile > 255) {
		lua_pushboolean(state, false);
		return 1;
	}

	int mask = emulator->getMemoryModule()->ram[PEMSA_RAM_GFX_PROPS + tile];

	if (lua_gettop(state) == 2) {
		int bit = luaL_checkinteger(state, 2);
		lua_pushboolean(state, (mask & (1 << bit)) != 0);
	} else {
		lua_pushnumber(state, mask);
	}

	return 1;
}

static int fset(lua_State* state) {
	int tile = luaL_checknumber(state, 1);

	if (tile < 0 || tile > 255) {
		return 0;
	}

	uint8_t* ram = emulator->getMemoryModule()->ram + PEMSA_RAM_GFX_PROPS + tile;

	int bit = luaL_checknumber(state, 2);
	bool on = pemsa_optional_bool(state, 3, false);

	*ram = SET_BIT(*ram, bit, on)
	return 0;
}

static int mget(lua_State* state) {
	int x = luaL_checknumber(state, 1);
	int y = luaL_checknumber(state, 2);

	if (x < 0 || y < 0 || x > 127 || y > 63) {
		lua_pushnumber(state, 0);
	} else {
		lua_pushnumber(state, emulator->getMemoryModule()->ram[(y > 31 ? PEMSA_RAM_GFX : PEMSA_RAM_MAP) + x + y * 128]);
	}

	return 1;
}

static int mset(lua_State* state) {
	int x = luaL_checknumber(state, 1);
	int y = luaL_checknumber(state, 2);
	int tile = luaL_checknumber(state, 3);

	if (!(x < 0 || y < 0 || x > 127 || y > 63)) {
		emulator->getMemoryModule()->ram[(y > 31 ? PEMSA_RAM_GFX : PEMSA_RAM_MAP) + x + y * 128] = tile % 256;
	}

	return 0;
}

static int cstore(lua_State* state) {
	std::cerr << "Warning: menuitem() is not currently implemented\n";
	return 0;
}

static int memcpy(lua_State* state) {
	int to = luaL_checknumber(state, 1);
	int from = luaL_checknumber(state, 2);
	int amount = luaL_checknumber(state, 3);

	if (to < 0 || from < 0 || to + amount > PEMSA_RAM_END || from + amount > PEMSA_RAM_END) {
		return 0;
	}

	uint8_t* ram = emulator->getMemoryModule()->ram;

	for (int i = 0; i < amount; i++) {
		ram[to + i] = ram[from + i];
	}

	return 0;
}

static int memset(lua_State* state) {
	int to = luaL_checknumber(state, 1);
	int value = luaL_checknumber(state, 2);
	int amount = luaL_checknumber(state, 3);

	if (to < 0 || to + amount >= PEMSA_RAM_END) {
		return 0;
	}

	uint8_t* ram = emulator->getMemoryModule()->ram;

	for (int i = 0; i < amount; i++) {
		ram[to + i] = value;
	}

	return 0;
}

static int peek(lua_State* state) {
	int index = luaL_checknumber(state, 1);

	if (index >= 0 && index < PEMSA_RAM_END) {
		lua_pushnumber(state, emulator->getMemoryModule()->ram[index]);
	} else {
		lua_pushnumber(state, 0);
	}

	return 1;
}

static int poke(lua_State* state) {
	int index = luaL_checknumber(state, 1);

	if (index >= 0 && index < PEMSA_RAM_END) {
		emulator->getMemoryModule()->ram[index] = luaL_checknumber(state, 2);
	}

	return 0;
}

static int reload(lua_State* state) {
	int to = 0;
	int from = 0;
	int amount;

	if (lua_gettop(state) == 0) {
		amount = 0x4300;
	} else {
		to = luaL_checknumber(state, 1);
		from = luaL_checknumber(state, 2);
		amount = luaL_checknumber(state, 3);
	}

	if (to < 0 || from < 0 || to + amount > PEMSA_RAM_END || from + amount > PEMSA_ROM_END) {
		return 0;
	}

	uint8_t* ram = emulator->getMemoryModule()->ram;
	uint8_t* rom = emulator->getCartridgeModule()->getCart()->rom;

	for (int i = 0; i < amount; i++) {
		ram[to + i] = rom[from + i];
	}

	return 0;
}

void pemsa_open_memory_api(PemsaEmulator* machine, lua_State* state) {
	emulator = machine;

	lua_register(state, "fget", fget);
	lua_register(state, "fset", fset);
	lua_register(state, "mget", mget);
	lua_register(state, "mset", mset);

	lua_register(state, "cstore", cstore);
	lua_register(state, "memcpy", memcpy);
	lua_register(state, "memset", memset);
	lua_register(state, "peek", peek);
	lua_register(state, "poke", poke);
	lua_register(state, "reload", reload);
}