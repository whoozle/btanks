
current_stage = nil
current_stage_idx = 0
current_message = nil
message_timer = 0

check_timer = 0
check_interval = 0.5

function on_load()
end

items = {
	nuke_missiles_item=150, 
	guided_missiles_item=10, 
	dumb_missiles_item=10, 
	smoke_missiles_item=30, 
	mines_item=20, 
	ricochet_bullets_item=20, 
	dispersion_bullets_item=50, 
	machinegunner_item=30, 
	thrower_item=20, 
	boomerang_missiles_item = 40, 
	stun_missiles_item = 20, 
	heal = 20, 
	megaheal = 40,
	};

items_id = {}

monsters = {zombie= 10, slime= 20};
troopers = {kamikaze= 5, machinegunner= 10, thrower= 5, cannon= 10, tank= 100, launcher= 100, shilka= 100, mortar= 75, watchtower_with_machinegunner = 60, watchtower_with_thrower = 30, heli_bomb = 200 };
scores = {
	troopers,	
	troopers,	
	troopers,	
	monsters, 	
	troopers,	
	troopers,	
}

animations = {
	launcher={"red-launcher", "green-launcher", "cyan-launcher", "yellow-launcher", },
	tank={"red-tank", "green-tank", "cyan-tank", "yellow-tank", },
	shilka={"red-shilka", "green-shilka", "cyan-shilka", "yellow-shilka", },
	mortar={"red-mortar", "green-mortar", "cyan-mortar", "yellow-mortar", }, 
	watchtower_with_machinegunner = {"watchtower"},
	watchtower_with_thrower = {"watchtower"},
	heli_bomb = {"helicopter"},
}

function array_keys(array)
	local keys = {};
	local k;
	for k in pairs(array) do 
		keys[#keys + 1] = k;
	end
	return keys;
end


function generate_stage(idx) 
	local score = idx * 100;
	local item_score = score;
	local stage = {};

	print("score "..score);

	local i, stage;
	stage = {};
	local row = random(table.getn(scores)) + 1;
	local objects = scores[row];
	local keys = array_keys(objects);

	local heli = false

	repeat 
		local object = keys[1 + random(#keys)];
		local animation = object;
		if animations[object] ~= nil then 
			animation = animations[object][random(#animations[object]) + 1];
		end

		if (score > objects[object] / 2 ) and ((object ~= "heli_bomb") or not heli) then
			score = score - objects[object];
			print("i've chosen: "..object..":"..animation);
			object = string.gsub(object, "_", "-");
			animation = string.gsub(animation, "_", "-");
			if (object == "heli-bomb") then 
				object = "raider-helicopter"; 
				heli = true; 
			end;
			stage[#stage + 1] = spawn_random(object, animation);
		end
	until score <= 0;
	set_specials(stage); --add marks on the minimap

	for i = 1, #items_id do 
		kill_object(items_id[i], true);
	end

	items_id = {}
	keys = array_keys(items);

	repeat 
		local object = keys[1 + random(#keys)];
		item_score = item_score - items[object];

		object = string.gsub(object, "_", "-");
		items_id[#items_id + 1] = spawn_random(object, object);
	until item_score <= 0;

	return stage;
end

function on_tick(dt) 
	if (message_timer > 0) then
		message_timer = message_timer - dt;
		return;
	end
	
	check_timer = check_timer + dt;
	if check_timer > check_interval then
		check_timer = 0;
		-- print "test";
		if current_stage == nil then
			if current_message == nil then 
				if current_stage_idx >= 10 then
					game_over('messages', 'mission-accomplished', 3, true);
					message_timer = 1000;
					return;
				end

				current_message = 'ready'
				message_timer = 2
				display_message('messages', current_message, message_timer, true)
			elseif current_message == 'ready' then
				current_message = 'go'
				message_timer = 1
				display_message('messages', current_message, message_timer, true)				
			else 
				current_message = nil;
				current_stage_idx = current_stage_idx + 1
				current_stage = generate_stage(current_stage_idx);
			end
		else 
			-- current_stage not nil
			for i = 1,#current_stage do 
				if object_exists(current_stage[i]) then return end
			end
			current_stage = nil;
		end
	end
end