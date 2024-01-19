#include <GarrysMod/Lua/Interface.h>
#include <GarrysMod/FactoryLoader.hpp>
#include <GarrysMod/InterfacePointers.hpp>
#include <MinHook.h>
#include <iclientrenderable.h>
#include <GarrysMod/Lua/LuaShared.h>

#define LUASHAREDBULLSHIT "LUASHARED003"
#define VENGINE_HUDMODEL_INTERFACE_VERSION "VEngineModel016"

template<typename Fn>
inline Fn getvfunc(const void* v, int i)
{
	return (Fn) * (*(const void***)v + i);
}

enum
{
	LUA_CLIENT = 0,
	LUA_SERVER,
	LUA_MENU
};

GarrysMod::Lua::ILuaShared* pLuaShared;
int _G_hook_Call = NULL;

typedef void(__thiscall* fnDrawModelExecute)(void*, DrawModelState_t&, ModelRenderInfo_t&, matrix3x4_t*);
fnDrawModelExecute ofnDrawModelExecute;

void __fastcall hkDrawModelExecute(void* pthis, DrawModelState_t& state, ModelRenderInfo_t& pInfo, matrix3x4_t* pCustomBoneToWorld = NULL)
{
	// Death and despair
	GarrysMod::Lua::ILuaBase* LUA_CL = (GarrysMod::Lua::ILuaBase*)pLuaShared->GetLuaInterface(LUA_CLIENT);
	if (!LUA_CL)
		return ofnDrawModelExecute(pthis, state, pInfo, pCustomBoneToWorld);
	
	LUA_CL->ReferencePush(_G_hook_Call);
	{
		LUA_CL->PushString("PreDrawModelExecute");
		LUA_CL->PushNil();
		LUA_CL->PushNumber(pInfo.entity_index);
	}
	LUA_CL->Call(3, 0);

	ofnDrawModelExecute(pthis, state, pInfo, pCustomBoneToWorld);

	LUA_CL->ReferencePush(_G_hook_Call);
	{
		LUA_CL->PushString("PostDrawModelExecute");
		LUA_CL->PushNil();
		LUA_CL->PushNumber(pInfo.entity_index);
	}
	LUA_CL->Call(3, 0);
}

GMOD_MODULE_OPEN()
{
	// Get hook.Call so we can make it actually useful
	LUA->PushSpecial(GarrysMod::Lua::SPECIAL_GLOB);
	{
		LUA->PushString("hook");
		LUA->RawGet(-2);
		if (LUA->IsType(-1, GarrysMod::Lua::Type::Table))
		{
			LUA->PushString("Call");
			LUA->RawGet(-2);
			if (LUA->IsType(-1, GarrysMod::Lua::Type::Function))
			{
				_G_hook_Call = LUA->ReferenceCreate();
			}
			LUA->Pop();
		}
		LUA->Pop();
	}
	LUA->Pop();

	if (!_G_hook_Call)
	{
		LUA->ThrowError("_G.hook.Call not found!");
		return 0;
	}

	// Get Lua shared so we can use the hook.Call we just got
	SourceSDK::FactoryLoader factory_LuaShared("lua_shared");
	pLuaShared = factory_LuaShared.GetInterface<GarrysMod::Lua::ILuaShared>(LUASHAREDBULLSHIT);
	if (!pLuaShared)
	{
		LUA->ThrowError("NO LUA SHARED RETARD");
		return 0;
	}

	// Get the actual thing to make this work
	SourceSDK::FactoryLoader factory_Engine("engine");
	IVEngineClient* iModelRender = factory_Engine.GetInterface<IVEngineClient>(VENGINE_HUDMODEL_INTERFACE_VERSION);
	if (!iModelRender)
	{
		LUA->ThrowError("NO MODEL RENDER RETARD");
		return 0;
	}

	// Haxor time
	MH_Initialize();
	{
		void* DrawModelExecute = getvfunc<void*>(iModelRender, 20);
		MH_CreateHook(DrawModelExecute, hkDrawModelExecute, (void**)&ofnDrawModelExecute);
	}
	MH_EnableHook(MH_ALL_HOOKS);

	return 0;
}

GMOD_MODULE_CLOSE()
{
	MH_DisableHook(MH_ALL_HOOKS);
	MH_Uninitialize();

	return 0;
}