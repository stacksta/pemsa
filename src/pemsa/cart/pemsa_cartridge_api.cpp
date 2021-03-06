#include "pemsa/pemsa_emulator.hpp"

static PemsaEmulator* emulator;

static int dget(lua_State* state) {
	int index = luaL_checknumber(state, 1);

	if (index >= 0 && index < PEMSA_CART_DATA_SIZE) {
		lua_pushnumber(state, emulator->getCartridgeModule()->getCart()->cartData[index]);
	} else {
		lua_pushnumber(state, 0);
	}

	return 1;
}

static int dset(lua_State* state) {
	int index = luaL_checknumber(state, 1);

	if (index >= 0 && index < PEMSA_CART_DATA_SIZE) {
		PemsaCartridgeModule* cartridgeModule = emulator->getCartridgeModule();

		cartridgeModule->getCart()->cartData[index] = luaL_checknumber(state, 2);
		cartridgeModule->saveData();
	}

	return 0;
}

static int cartdata(lua_State* state) {
	const char* name = luaL_checkstring(state, 1);
	emulator->getCartridgeModule()->loadData(name);

	return 0;
}

void pemsa_open_cartridge_api(PemsaEmulator* machine, lua_State* state) {
	emulator = machine;

	lua_register(state, "dget", dget);
	lua_register(state, "dset", dset);
	lua_register(state, "cartdata", cartdata);
}