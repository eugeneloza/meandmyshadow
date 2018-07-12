/*
 * Copyright (C) 2012-2013 Me and My Shadow
 *
 * This file is part of Me and My Shadow.
 *
 * Me and My Shadow is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Me and My Shadow is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Me and My Shadow.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "ScriptAPI.h"
#include "ScriptExecutor.h"
#include "SoundManager.h"
#include "Functions.h"
#include "Game.h"
#include "MusicManager.h"
#include <iostream>
using namespace std;

/////////////////////////// HELPER MACRO ///////////////////////////

#define HELPER_GET_AND_CHECK_ARGS(ARGS) \
	int args = lua_gettop(state); \
	if(args != ARGS) { \
		lua_pushstring(state, "Incorrect number of arguments for " __FUNCTION__ ", expected " #ARGS "."); \
		return lua_error(state); \
	}

#define HELPER_GET_AND_CHECK_ARGS_RANGE(ARGS1, ARGS2) \
	int args = lua_gettop(state); \
	if(args < ARGS1 || args > ARGS2) { \
		lua_pushstring(state, "Incorrect number of arguments for " __FUNCTION__ ", expected " #ARGS1 "-" #ARGS2 "."); \
		return lua_error(state); \
	}

#define HELPER_GET_AND_CHECK_ARGS_2(ARGS1, ARGS2) \
	int args = lua_gettop(state); \
	if(args != ARGS1 && args != ARGS2) { \
		lua_pushstring(state, "Incorrect number of arguments for " __FUNCTION__ ", expected " #ARGS1 " or " #ARGS2 "."); \
		return lua_error(state); \
	}

#define HELPER_GET_AND_CHECK_ARGS_AT_LEAST(ARGS) \
	int args = lua_gettop(state); \
	if(args < ARGS) { \
		lua_pushstring(state, "Incorrect number of arguments for " __FUNCTION__ ", expected at least " #ARGS "."); \
		return lua_error(state); \
	}

#define HELPER_GET_AND_CHECK_ARGS_AT_MOST(ARGS) \
	int args = lua_gettop(state); \
	if(args > ARGS) { \
		lua_pushstring(state, "Incorrect number of arguments for " __FUNCTION__ ", expected at most " #ARGS "."); \
		return lua_error(state); \
	}

//================================================================

#define HELPER_CHECK_ARGS_TYPE(INDEX, TYPE) \
	if(!lua_is##TYPE(state,INDEX)) { \
		lua_pushstring(state,"Invalid type for argument " #INDEX " of " __FUNCTION__ ", should be " #TYPE "."); \
		return lua_error(state); \
	}

#define HELPER_CHECK_ARGS_TYPE_NO_HINT(INDEX, TYPE) \
	if(!lua_is##TYPE(state,INDEX)) { \
		lua_pushstring(state,"Invalid type for argument " #INDEX " of " __FUNCTION__ "."); \
		return lua_error(state); \
	}

#define HELPER_CHECK_ARGS_TYPE_2(INDEX, TYPE1, TYPE2) \
	if(!lua_is##TYPE1(state,INDEX) && !lua_is##TYPE2(state,INDEX)) { \
		lua_pushstring(state,"Invalid type for argument " #INDEX " of " __FUNCTION__ ", should be " #TYPE1 " or " #TYPE2 "."); \
		return lua_error(state); \
	}

#define HELPER_CHECK_ARGS_TYPE_2_NO_HINT(INDEX, TYPE1, TYPE2) \
	if(!lua_is##TYPE1(state,INDEX) && !lua_is##TYPE2(state,INDEX)) { \
		lua_pushstring(state,"Invalid type for argument " #INDEX " of " __FUNCTION__ "."); \
		return lua_error(state); \
	}

#define HELPER_CHECK_ARGS_TYPE_OR_NIL(INDEX, TYPE) \
	HELPER_CHECK_ARGS_TYPE_2(INDEX, TYPE, nil)

#define HELPER_CHECK_ARGS_TYPE_OR_NIL_NO_HINT(INDEX, TYPE) \
	HELPER_CHECK_ARGS_TYPE_2_NO_HINT(INDEX, TYPE, nil)

//================================================================

#define HELPER_CHECK_OPTIONAL_ARGS_TYPE(INDEX, TYPE) \
	if(args>=INDEX && !lua_is##TYPE(state,INDEX)) { \
		lua_pushstring(state,"Invalid type for argument " #INDEX " of " __FUNCTION__ ", should be " #TYPE "."); \
		return lua_error(state); \
	}

#define HELPER_CHECK_OPTIONAL_ARGS_TYPE_NO_HINT(INDEX, TYPE) \
	if(args>=INDEX && !lua_is##TYPE(state,INDEX)) { \
		lua_pushstring(state,"Invalid type for argument " #INDEX " of " __FUNCTION__ "."); \
		return lua_error(state); \
	}

#define HELPER_CHECK_OPTIONAL_ARGS_TYPE_2(INDEX, TYPE1, TYPE2) \
	if(args>=INDEX && !lua_is##TYPE1(state,INDEX) && !lua_is##TYPE2(state,INDEX)) { \
		lua_pushstring(state,"Invalid type for argument " #INDEX " of " __FUNCTION__ ", should be " #TYPE1 " or " #TYPE2 "."); \
		return lua_error(state); \
	}

#define HELPER_CHECK_OPTIONAL_ARGS_TYPE_2_NO_HINT(INDEX, TYPE1, TYPE2) \
	if(args>=INDEX && !lua_is##TYPE1(state,INDEX) && !lua_is##TYPE2(state,INDEX)) { \
		lua_pushstring(state,"Invalid type for argument " #INDEX " of " __FUNCTION__ "."); \
		return lua_error(state); \
	}

#define HELPER_CHECK_OPTIONAL_ARGS_TYPE_OR_NIL(INDEX, TYPE) \
	HELPER_CHECK_OPTIONAL_ARGS_TYPE_2(INDEX, TYPE, nil)

#define HELPER_CHECK_OPTIONAL_ARGS_TYPE_OR_NIL_NO_HINT(INDEX, TYPE) \
	HELPER_CHECK_OPTIONAL_ARGS_TYPE_2_NO_HINT(INDEX, TYPE, nil)

//================================================================

#define _F(FUNC) \
	{ #FUNC, _L::FUNC }

///////////////////////////BLOCK SPECIFIC///////////////////////////

namespace block {

	int getBlockById(lua_State* state){
		//Get the number of args, this MUST be one.
		HELPER_GET_AND_CHECK_ARGS(1);

		//Make sure the given argument is an id (string).
		HELPER_CHECK_ARGS_TYPE(1, string);

		//Check if the currentState is the game state.
		Game* game = dynamic_cast<Game*>(currentState);
		if (game == NULL) return 0;

		//Get the actual game object.
		string id = lua_tostring(state, 1);
		std::vector<Block*>& levelObjects = game->levelObjects;
		Block* object = NULL;
		for (unsigned int i = 0; i < levelObjects.size(); i++){
			if (levelObjects[i]->getEditorProperty("id") == id){
				object = levelObjects[i];
				break;
			}
		}
		if (object == NULL){
			//Unable to find the requested object.
			//Return nothing, will result in a nil in the script. 
			return 0;
		}

		//Create the userdatum.
		object->createUserData(state, "block");

		//We return one object, the userdatum.
		return 1;
	}

	int getBlocksById(lua_State* state){
		//Get the number of args, this MUST be one.
		HELPER_GET_AND_CHECK_ARGS(1);

		//Make sure the given argument is an id (string).
		HELPER_CHECK_ARGS_TYPE(1, string);

		//Check if the currentState is the game state.
		Game* game = dynamic_cast<Game*>(currentState);
		if (game == NULL) return 0;

		//Get the actual game object.
		string id = lua_tostring(state, 1);
		std::vector<Block*>& levelObjects = game->levelObjects;
		std::vector<Block*> result;
		for (unsigned int i = 0; i < levelObjects.size(); i++){
			if (levelObjects[i]->getEditorProperty("id") == id){
				result.push_back(levelObjects[i]);
			}
		}

		//Create the table that will hold the result.
		lua_createtable(state, result.size(), 0);

		//Loop through the results.
		for (unsigned int i = 0; i < result.size(); i++){
			//Create the userdatum.
			result[i]->createUserData(state, "block");
			//And set the table.
			lua_rawseti(state, -2, i + 1);
		}

		//We return one object, the userdatum.
		return 1;
	}

	int moveTo(lua_State* state){
		//Check the number of arguments.
		HELPER_GET_AND_CHECK_ARGS(3);

		//Check if the arguments are of the right type.
		HELPER_CHECK_ARGS_TYPE_NO_HINT(1, userdata);
		HELPER_CHECK_ARGS_TYPE(2, number); // integer
		HELPER_CHECK_ARGS_TYPE(3, number); // integer

		//Now get the pointer to the object.
		Block* object = Block::getObjectFromUserData(state, 1);
		if (object == NULL) return 0;

		int x = lua_tonumber(state, 2);
		int y = lua_tonumber(state, 3);
		object->moveTo(x, y);

		return 0;
	}

	int getLocation(lua_State* state){
		//Make sure there's only one argument and that argument is an userdatum.
		HELPER_GET_AND_CHECK_ARGS(1);

		HELPER_CHECK_ARGS_TYPE_NO_HINT(1, userdata);

		Block* object = Block::getObjectFromUserData(state, 1);
		if (object == NULL) return 0;

		//Get the object.
		lua_pushnumber(state, object->getBox().x);
		lua_pushnumber(state, object->getBox().y);
		return 2;
	}

	int setLocation(lua_State* state){
		//Check the number of arguments.
		HELPER_GET_AND_CHECK_ARGS(3);

		//Check if the arguments are of the right type.
		HELPER_CHECK_ARGS_TYPE_NO_HINT(1, userdata);
		HELPER_CHECK_ARGS_TYPE(2, number); // integer
		HELPER_CHECK_ARGS_TYPE(3, number); // integer

		//Now get the pointer to the object.
		Block* object = Block::getObjectFromUserData(state, 1);
		if (object == NULL) return 0;

		int x = lua_tonumber(state, 2);
		int y = lua_tonumber(state, 3);
		object->setLocation(x, y);

		return 0;
	}

	int growTo(lua_State* state){
		//Check the number of arguments.
		HELPER_GET_AND_CHECK_ARGS(3);

		//Check if the arguments are of the right type.
		HELPER_CHECK_ARGS_TYPE_NO_HINT(1, userdata);
		HELPER_CHECK_ARGS_TYPE(2, number); // integer
		HELPER_CHECK_ARGS_TYPE(3, number); // integer

		//Now get the pointer to the object.
		Block* object = Block::getObjectFromUserData(state, 1);
		if (object == NULL) return 0;

		int w = lua_tonumber(state, 2);
		int h = lua_tonumber(state, 3);
		object->growTo(w, h);

		return 0;
	}

	int getSize(lua_State* state){
		//Check the number of arguments.
		HELPER_GET_AND_CHECK_ARGS(1);

		//Check if the arguments are of the right type.
		HELPER_CHECK_ARGS_TYPE_NO_HINT(1, userdata);

		Block* object = Block::getObjectFromUserData(state, 1);
		if (object == NULL) return 0;

		//Get the object.
		lua_pushnumber(state, object->getBox().w);
		lua_pushnumber(state, object->getBox().h);
		return 2;
	}

	int setSize(lua_State* state){
		//Check the number of arguments.
		HELPER_GET_AND_CHECK_ARGS(3);

		//Check if the arguments are of the right type.
		HELPER_CHECK_ARGS_TYPE_NO_HINT(1, userdata);
		HELPER_CHECK_ARGS_TYPE(2, number); // integer
		HELPER_CHECK_ARGS_TYPE(3, number); // integer

		//Now get the pointer to the object.
		Block* object = Block::getObjectFromUserData(state, 1);
		if (object == NULL) return 0;

		int w = lua_tonumber(state, 2);
		int h = lua_tonumber(state, 3);
		object->setSize(w, h);

		return 0;
	}

	int getType(lua_State* state){
		//Check the number of arguments.
		HELPER_GET_AND_CHECK_ARGS(1);

		//Check if the arguments are of the right type.
		HELPER_CHECK_ARGS_TYPE_NO_HINT(1, userdata);

		Block* object = Block::getObjectFromUserData(state, 1);
		if (object == NULL || object->type < 0 || object->type >= TYPE_MAX) return 0;

		lua_pushstring(state, Game::blockName[object->type]);
		return 1;
	}

	int changeThemeState(lua_State* state){
		//Check the number of arguments.
		HELPER_GET_AND_CHECK_ARGS(2);

		//Check if the arguments are of the right type.
		HELPER_CHECK_ARGS_TYPE_NO_HINT(1, userdata);
		HELPER_CHECK_ARGS_TYPE(2, string);

		Block* object = Block::getObjectFromUserData(state, 1);
		object->appearance.changeState(lua_tostring(state, 2));

		return 0;
	}

	int setVisible(lua_State* state){
		//Check the number of arguments.
		HELPER_GET_AND_CHECK_ARGS(2);

		//Check if the arguments are of the right type.
		HELPER_CHECK_ARGS_TYPE_NO_HINT(1, userdata);
		HELPER_CHECK_ARGS_TYPE(2, boolean);

		Block* object = Block::getObjectFromUserData(state, 1);
		if (object == NULL)
			return 0;

		bool visible = lua_toboolean(state, 2);
		object->visible = visible;

		return 0;
	}

	int isVisible(lua_State* state){
		//Check the number of arguments.
		HELPER_GET_AND_CHECK_ARGS(1);

		//Check if the arguments are of the right type.
		HELPER_CHECK_ARGS_TYPE_NO_HINT(1, userdata);

		Block* object = Block::getObjectFromUserData(state, 1);
		if (object == NULL)
			return 0;

		lua_pushboolean(state, object->visible);
		return 1;
	}

	int getEventHandler(lua_State* state){
		//Check the number of arguments.
		HELPER_GET_AND_CHECK_ARGS(2);

		//Check if the arguments are of the right type.
		HELPER_CHECK_ARGS_TYPE_NO_HINT(1, userdata);
		HELPER_CHECK_ARGS_TYPE(2, string);

		Block* object = Block::getObjectFromUserData(state, 1);
		if (object == NULL) return 0;

		//Check event type
		string eventType = lua_tostring(state, 2);
		map<string, int>::iterator it = Game::gameObjectEventNameMap.find(eventType);
		if (it == Game::gameObjectEventNameMap.end()) return 0;

		//Check compiled script
		map<int, int>::iterator script = object->compiledScripts.find(it->second);
		if (script == object->compiledScripts.end()) return 0;

		//Get event handler
		lua_rawgeti(state, LUA_REGISTRYINDEX, script->second);
		return 1;
	}

	//It will return old event handler.
	int setEventHandler(lua_State* state){
		//Check the number of arguments.
		HELPER_GET_AND_CHECK_ARGS(3);

		//Check if the arguments are of the right type.
		HELPER_CHECK_ARGS_TYPE_NO_HINT(1, userdata);
		HELPER_CHECK_ARGS_TYPE(2, string);
		HELPER_CHECK_ARGS_TYPE_OR_NIL(3, function);

		Block* object = Block::getObjectFromUserData(state, 1);
		if (object == NULL) return 0;

		//Check event type
		string eventType = lua_tostring(state, 2);
		map<string, int>::const_iterator it = Game::gameObjectEventNameMap.find(eventType);
		if (it == Game::gameObjectEventNameMap.end()){
			lua_pushfstring(state, "Unknown block event type: '%s'.", eventType.c_str());
			return lua_error(state);
		}

		//Check compiled script
		int scriptIndex = LUA_REFNIL;
		{
			map<int, int>::iterator script = object->compiledScripts.find(it->second);
			if (script != object->compiledScripts.end()) scriptIndex = script->second;
		}

		//Set new event handler
		object->compiledScripts[it->second] = luaL_ref(state, LUA_REGISTRYINDEX);

		//Get old event handler and unreference it
		lua_rawgeti(state, LUA_REGISTRYINDEX, scriptIndex);
		luaL_unref(state, LUA_REGISTRYINDEX, scriptIndex);
		return 1;
	}

}

#define _L block
//Array with the methods for the block library.
static const struct luaL_Reg blocklib_m[]={
	_F(getBlockById),
	_F(getBlocksById),
	_F(moveTo),
	_F(getLocation),
	_F(setLocation),
	_F(growTo),
	_F(getSize),
	_F(setSize),
	_F(getType),
	_F(changeThemeState),
	_F(setVisible),
	_F(isVisible),
	_F(getEventHandler),
	_F(setEventHandler),
	{NULL,NULL}
};
#undef _L

int luaopen_block(lua_State* state){
	luaL_newlib(state,blocklib_m);
	
	//Create the metatable for the block userdata.
	luaL_newmetatable(state,"block");

	lua_pushstring(state,"__index");
	lua_pushvalue(state,-2);
	lua_settable(state,-3);

	Block::registerMetatableFunctions(state,-3);

	//Register the functions and methods.
	luaL_setfuncs(state,blocklib_m,0);
	return 1;
}

//////////////////////////PLAYER SPECIFIC///////////////////////////

struct PlayerUserDatum{
	char sig1,sig2,sig3,sig4;
};

Player* getPlayerFromUserData(lua_State* state,int idx){
	PlayerUserDatum* ud=(PlayerUserDatum*)lua_touserdata(state,1);
	//Make sure the user datum isn't null.
	if(!ud) return NULL;

	//Get the game state.
	Game* game=dynamic_cast<Game*>(currentState);
	if(game==NULL) return NULL;

	Player* player=NULL;

	//Check the signature to see if it's the player or the shadow.
	if(ud->sig1=='P' && ud->sig2=='L' && ud->sig3=='Y' && ud->sig4=='R')
		player=&game->player;
	else if(ud->sig1=='S' && ud->sig2=='H' && ud->sig3=='D' && ud->sig4=='W')
		player=&game->shadow;
	
	return player;
}

namespace playershadow {

	int getLocation(lua_State* state){
		//Check the number of arguments.
		HELPER_GET_AND_CHECK_ARGS(1);

		//Check if the arguments are of the right type.
		HELPER_CHECK_ARGS_TYPE_NO_HINT(1, userdata);

		Player* player = getPlayerFromUserData(state, 1);
		if (player == NULL) return 0;

		//Get the object.
		lua_pushnumber(state, player->getBox().x);
		lua_pushnumber(state, player->getBox().y);
		return 2;
	}

	int setLocation(lua_State* state){
		//Check the number of arguments.
		HELPER_GET_AND_CHECK_ARGS(3);

		//Check if the arguments are of the right type.
		HELPER_CHECK_ARGS_TYPE_NO_HINT(1, userdata);
		HELPER_CHECK_ARGS_TYPE(2, number); // integer
		HELPER_CHECK_ARGS_TYPE(3, number); // integer

		//Get the player.
		Player* player = getPlayerFromUserData(state, 1);
		if (player == NULL) return 0;

		//Get the new location.
		int x = lua_tonumber(state, 2);
		int y = lua_tonumber(state, 3);
		player->setLocation(x, y);

		return 0;
	}

	int jump(lua_State* state){
		//Check the number of arguments.
		HELPER_GET_AND_CHECK_ARGS_2(1, 2);

		//Check if the arguments are of the right type.
		HELPER_CHECK_ARGS_TYPE_NO_HINT(1, userdata);
		HELPER_CHECK_OPTIONAL_ARGS_TYPE(2, number); // integer

		//Get the player.
		Player* player = getPlayerFromUserData(state, 1);
		if (player == NULL) return 0;

		//Get the new location.
		if (args == 2){
			int yVel = lua_tonumber(state, 2);
			player->jump(yVel);
		} else{
			//Use default jump strength.
			player->jump();
		}

		return 0;
	}

	int isShadow(lua_State* state){
		//Check the number of arguments.
		HELPER_GET_AND_CHECK_ARGS(1);

		//Check if the arguments are of the right type.
		HELPER_CHECK_ARGS_TYPE_NO_HINT(1, userdata);

		Player* player = getPlayerFromUserData(state, 1);
		if (player == NULL) return 0;

		lua_pushboolean(state, player->isShadow());
		return 1;
	}

	int getCurrentStand(lua_State* state){
		//Check the number of arguments.
		HELPER_GET_AND_CHECK_ARGS(1);

		//Check if the arguments are of the right type.
		HELPER_CHECK_ARGS_TYPE_NO_HINT(1, userdata);

		Player* player = getPlayerFromUserData(state, 1);
		if (player == NULL) return 0;

		//Get the actual game object.
		Block* object = player->getObjCurrentStand();
		if (object == NULL){
			return 0;
		}

		//Create the userdatum.
		object->createUserData(state, "block");

		//We return one object, the userdatum.
		return 1;
	}

}

#define _L playershadow
//Array with the methods for the player and shadow library.
static const struct luaL_Reg playerlib_m[]={
	_F(getLocation),
	_F(setLocation),
	_F(jump),
	_F(isShadow),
	_F(getCurrentStand),
	{NULL,NULL}
};
#undef _L

int luaopen_player(lua_State* state){
	luaL_newlib(state,playerlib_m);

	//Create the metatable for the player userdata.
	luaL_newmetatable(state,"player");

	lua_pushstring(state,"__index");
	lua_pushvalue(state,-2);
	lua_settable(state,-3);

	//Now create two default player user data, one for the player and one for the shadow.
	PlayerUserDatum* ud=(PlayerUserDatum*)lua_newuserdata(state,sizeof(PlayerUserDatum));
	ud->sig1='P';ud->sig2='L';ud->sig3='Y';ud->sig4='R';
	luaL_getmetatable(state,"player");
	lua_setmetatable(state,-2);
	lua_setglobal(state,"player");

	ud=(PlayerUserDatum*)lua_newuserdata(state,sizeof(PlayerUserDatum));
	ud->sig1='S';ud->sig2='H';ud->sig3='D';ud->sig4='W';
	luaL_getmetatable(state,"player");
	lua_setmetatable(state,-2);
	lua_setglobal(state,"shadow");

	//Register the functions and methods.
	luaL_setfuncs(state,playerlib_m,0);
	return 1;
}

//////////////////////////LEVEL SPECIFIC///////////////////////////

namespace level {

	int getSize(lua_State* state){
		//NOTE: this function accepts 0 arguments, but we ignore the argument count.

		//Returns level size.
		lua_pushinteger(state, LEVEL_WIDTH);
		lua_pushinteger(state, LEVEL_HEIGHT);
		return 2;
	}

	int getWidth(lua_State* state){
		//NOTE: this function accepts 0 arguments, but we ignore the argument count.

		//Returns level size.
		lua_pushinteger(state, LEVEL_WIDTH);
		return 1;
	}

	int getHeight(lua_State* state){
		//NOTE: this function accepts 0 arguments, but we ignore the argument count.

		//Returns level size.
		lua_pushinteger(state, LEVEL_HEIGHT);
		return 1;
	}

	int getName(lua_State* state){
		//NOTE: this function accepts 0 arguments, but we ignore the argument count.

		//Check if the currentState is the game state.
		Game* game = dynamic_cast<Game*>(currentState);
		if (game == NULL) return 0;

		//Returns level name.
		lua_pushstring(state, game->getLevelName().c_str());
		return 1;
	}

	int getEventHandler(lua_State* state){
		//Check the number of arguments.
		HELPER_GET_AND_CHECK_ARGS(1);

		//Check if the arguments are of the right type.
		HELPER_CHECK_ARGS_TYPE(1, string);

		//Check if the currentState is the game state.
		Game* game = dynamic_cast<Game*>(currentState);
		if (game == NULL) return 0;

		//Check event type
		string eventType = lua_tostring(state, 1);
		map<string, int>::iterator it = Game::levelEventNameMap.find(eventType);
		if (it == Game::levelEventNameMap.end()) return 0;

		//Check compiled script
		map<int, int>::iterator script = game->compiledScripts.find(it->second);
		if (script == game->compiledScripts.end()) return 0;

		//Get event handler
		lua_rawgeti(state, LUA_REGISTRYINDEX, script->second);
		return 1;
	}

	//It will return old event handler.
	int setEventHandler(lua_State* state){
		//Check the number of arguments.
		HELPER_GET_AND_CHECK_ARGS(2);

		//Check if the arguments are of the right type.
		HELPER_CHECK_ARGS_TYPE(1, string);
		HELPER_CHECK_ARGS_TYPE_OR_NIL(2, function);

		//Check if the currentState is the game state.
		Game* game = dynamic_cast<Game*>(currentState);
		if (game == NULL) return 0;

		//Check event type
		string eventType = lua_tostring(state, 1);
		map<string, int>::const_iterator it = Game::levelEventNameMap.find(eventType);
		if (it == Game::levelEventNameMap.end()){
			lua_pushfstring(state, "Unknown level event type: '%s'.", eventType.c_str());
			return lua_error(state);
		}

		//Check compiled script
		int scriptIndex = LUA_REFNIL;
		{
			map<int, int>::iterator script = game->compiledScripts.find(it->second);
			if (script != game->compiledScripts.end()) scriptIndex = script->second;
		}

		//Set new event handler
		game->compiledScripts[it->second] = luaL_ref(state, LUA_REGISTRYINDEX);

		//Get old event handler and unreference it
		lua_rawgeti(state, LUA_REGISTRYINDEX, scriptIndex);
		luaL_unref(state, LUA_REGISTRYINDEX, scriptIndex);
		return 1;
	}

	int win(lua_State* state){
		//NOTE: this function accepts 0 arguments, but we ignore the argument count.

		//Check if the currentState is the game state.
		if (stateID == STATE_LEVEL_EDITOR)
			return 0;
		Game* game = dynamic_cast<Game*>(currentState);
		if (game == NULL) return 0;

		game->won = true;
		return 0;
	}

	int getTime(lua_State* state){
		//NOTE: this function accepts 0 arguments, but we ignore the argument count.

		//Check if the currentState is the game state.
		Game* game = dynamic_cast<Game*>(currentState);
		if (game == NULL) return 0;

		//Returns level size.
		lua_pushinteger(state, game->time);
		return 1;
	}

	int getRecordings(lua_State* state){
		//NOTE: this function accepts 0 arguments, but we ignore the argument count.

		//Check if the currentState is the game state.
		Game* game = dynamic_cast<Game*>(currentState);
		if (game == NULL) return 0;

		//Returns level size.
		lua_pushinteger(state, game->recordings);
		return 1;
	}

}

#define _L level
//Array with the methods for the level library.
static const struct luaL_Reg levellib_m[]={
	_F(getSize),
	_F(getWidth),
	_F(getHeight),
	_F(getName),
	_F(getEventHandler),
	_F(setEventHandler),
	_F(win),
	_F(getTime),
	_F(getRecordings),
	{NULL,NULL}
};
#undef _L

int luaopen_level(lua_State* state){
	luaL_newlib(state,levellib_m);
	
	//Register the functions and methods.
	luaL_setfuncs(state,levellib_m,0);
	return 1;
}

/////////////////////////CAMERA SPECIFIC///////////////////////////

//FIXME: I can't define namespace camera since there is already a global variable named camera.
//Therefore I use struct camera for a workaround.

struct camera {

	static int setMode(lua_State* state){
		//Check the number of arguments.
		HELPER_GET_AND_CHECK_ARGS(1);

		//Check if the arguments are of the right type.
		HELPER_CHECK_ARGS_TYPE(1, string);

		string mode = lua_tostring(state, 1);

		//Get the game for setting the camera.
		Game* game = dynamic_cast<Game*>(currentState);
		if (game == NULL) return 0;
		//Check which mode.
		if (mode == "player"){
			game->cameraMode = Game::CAMERA_PLAYER;
		} else if (mode == "shadow"){
			game->cameraMode = Game::CAMERA_SHADOW;
		} else{
			//Unkown OR invalid camera mode.
			lua_pushfstring(state, "Unkown or invalid camera mode for " __FUNCTION__ ": '%s'.", mode.c_str());
			return lua_error(state);
		}

		//Returns nothing.
		return 0;
	}

	static int lookAt(lua_State* state){
		//Check the number of arguments.
		HELPER_GET_AND_CHECK_ARGS(2);

		//Check if the arguments are of the right type.
		HELPER_CHECK_ARGS_TYPE(1, number); // integer
		HELPER_CHECK_ARGS_TYPE(2, number); // integer

		//Get the point.
		int x = lua_tonumber(state, 1);
		int y = lua_tonumber(state, 2);

		//Get the game for setting the camera.
		Game* game = dynamic_cast<Game*>(currentState);
		if (game == NULL) return 0;
		game->cameraMode = Game::CAMERA_CUSTOM;
		game->cameraTarget.x = x;
		game->cameraTarget.y = y;

		return 0;
	}

};

#define _L camera
//Array with the methods for the camera library.
static const struct luaL_Reg cameralib_m[]={
	_F(setMode),
	_F(lookAt),
	{NULL,NULL}
};
#undef _L

int luaopen_camera(lua_State* state){
	luaL_newlib(state,cameralib_m);

	//Register the functions and methods.
	luaL_setfuncs(state,cameralib_m,0);
	return 1;
}

/////////////////////////AUDIO SPECIFIC///////////////////////////

namespace audio {

	int playSound(lua_State* state){
		//Get the number of args, this can be anything from one to three.
		HELPER_GET_AND_CHECK_ARGS_RANGE(1, 3);

		//Check if the arguments are of the right type.
		HELPER_CHECK_ARGS_TYPE(1, string);
		HELPER_CHECK_OPTIONAL_ARGS_TYPE(2, number); // integer
		HELPER_CHECK_OPTIONAL_ARGS_TYPE(3, boolean);

		//Default values for concurrent and force.
		//See SoundManager.h
		int concurrent = 1;
		bool force = false;

		//If there's a second one it should be an integer.
		if (args > 1){
			concurrent = lua_tonumber(state, 2);
		}
		//If there's a third one it should be a boolean.
		if (args > 2){
			force = lua_toboolean(state, 3);
		}

		//Get the name of the sound.
		string sound = lua_tostring(state, 1);
		//Try to play the sound.
		getSoundManager()->playSound(sound, concurrent, force);

		//Returns nothing.
		return 0;
	}

	int playMusic(lua_State* state){
		//Get the number of args, this can be either one or two.
		HELPER_GET_AND_CHECK_ARGS_2(1, 2);

		//Make sure the first argument is a string.
		HELPER_CHECK_ARGS_TYPE(1, string);
		HELPER_CHECK_OPTIONAL_ARGS_TYPE(2, boolean);

		//Default value of fade for playMusic.
		//See MusicManager.h.
		bool fade = true;

		//If there's a second one it should be a boolean.
		if (args > 1){
			fade = lua_toboolean(state, 2);
		}

		//Get the name of the music.
		string music = lua_tostring(state, 1);
		//Try to switch to the new music.
		getMusicManager()->playMusic(music, fade);

		//Returns nothing.
		return 0;
	}

	int pickMusic(lua_State* state){
		//NOTE: this function accepts 0 arguments, but we ignore the argument count.

		//Let the music manager pick a song from the current music list.
		getMusicManager()->pickMusic();
		return 0;
	}

	int setMusicList(lua_State* state){
		//Get the number of args, this MUST be one.
		HELPER_GET_AND_CHECK_ARGS(1);

		//Make sure the given argument is a string.
		HELPER_CHECK_ARGS_TYPE(1, string);

		//And set the music list in the music manager.
		string list = lua_tostring(state, 1);
		getMusicManager()->setMusicList(list);
		return 0;
	}

	int getMusicList(lua_State* state){
		//NOTE: this function accepts 0 arguments, but we ignore the argument count.

		//Return the name of the song (contains list prefix).
		lua_pushstring(state, getMusicManager()->getCurrentMusicList().c_str());
		return 1;
	}


	int currentMusic(lua_State* state){
		//NOTE: this function accepts 0 arguments, but we ignore the argument count.

		//Return the name of the song (contains list prefix).
		lua_pushstring(state, getMusicManager()->getCurrentMusic().c_str());
		return 1;
	}

}

#define _L audio
//Array with the methods for the audio library.
static const struct luaL_Reg audiolib_m[]={
	_F(playSound),
	_F(playMusic),
	_F(pickMusic),
	_F(setMusicList),
	_F(getMusicList),
	_F(currentMusic),
	{NULL,NULL}
};
#undef _L

int luaopen_audio(lua_State* state){
	luaL_newlib(state,audiolib_m);

	//Register the functions and methods.
	luaL_setfuncs(state,audiolib_m,0);
	return 1;
}
