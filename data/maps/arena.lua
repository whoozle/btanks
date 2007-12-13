
current_stage = nil
current_message = nil
message_timer = 0

check_timer = 0
check_interval = 0.5

function on_load()
	stages = {
		{'object:helicopter-with-kamikazes:chinook:1', 'object:raider-helicopter:helicopter:1', 'object:raider-helicopter:helicopter:2'}, 
	};
	table.foreachi(stages, hide_stage);
end

function hide_stage_item(idx, value) hide_item(value) end

function hide_stage(idx, items)
	table.foreachi(items, hide_stage_item);
end

function on_tick(dt) 
	if (message_timer > 0) then
		message_timer = message_timer - dt
		return
	end
	
	check_timer = check_timer + dt;
	if check_timer > check_interval then
		check_timer = 0;
		-- print "test";
		if current_stage == nil then
			if table.getn(stages) == 0 then
				game_over('messages', 'mission-accomplished', 3, true);
				message_timer = 1000;
				return;
			end
			
			if current_message == nil then 
				current_message = 'ready'
				message_timer = 1
				display_message('messages', current_message, message_timer, true)
			elseif current_message == 'ready' then
				current_message = 'go'
				message_timer = 1
				display_message('messages', current_message, message_timer, true)				
			else 
				current_message = nil
				local n = table.getn(stages);
				local idx = random(n);
				print("searching for the new stage #"..idx.." from the "..n.." entries");
				current_stage = table.remove(stages, idx + 1)
				for i = 1,#current_stage do 
					show_item(current_stage[i]);
				end

			end
		else 
			-- current_stage not nil
			for i = 1,#current_stage do 
				if item_exists(current_stage[i]) then return end
			end
			current_stage = nil;
		end
	end
end