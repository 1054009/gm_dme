require("dme") -- Load the module

local pLocalPlayer = LocalPlayer() -- Store a reference to the local player

local matVisible = Material("models/debug/debugwhite")
local matOccluded = CreateMaterial("debugwhite_occluded", "VertexLitGeneric", {
	["$basetexture"] = matVisible:GetName(),
	["$model"] = 1,
	["$ignorez"] = 1
})

local bCalled = false -- Prevent an infinite loop

hook.Add("PreDrawModelExecute", "chams", function(entIndex)
	if bCalled then return end

	local pEntity = Entity(entIndex)
	if not IsValid(pEntity) or not pEntity:IsPlayer() then return end -- Due to this player check, these hooks work the same as PrePlayerDraw and PostPlayerDraw. But, you can remove this check in order to use it for any entity
	if pEntity == pLocalPlayer then return end -- Don't modify ourselves

	-- Visible chams
	render.MaterialOverride(matVisible)
	render.SetColorModulation(1, 0, 0)

	bCalled = true -- Prevent an infinite loop
		pEntity:DrawModel() -- Draw the visible chams
	bCalled = false

	-- Let the engine render the occluded chams
	render.MaterialOverride(matOccluded)
	render.SetColorModulation(0, 1, 0)
end)

hook.Add("PostDrawModelExecute", "chams", function(entIndex)
	-- Reset everything so the game can render normally
	render.MaterialOverride()
	render.SetColorModulation(1, 1, 1)
end)