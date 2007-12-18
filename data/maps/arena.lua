
current_stage = nil
current_stage_idx = 0
current_message = nil
message_timer = 0

check_timer = 0
check_interval = 0.5

function on_load()
end

function generate_stage(idx) 
	return { spawn('kamikaze', 'kamikaze', 256, 256) };
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
			if current_message == nil then 
				current_message = 'ready'
				message_timer = 1
				display_message('messages', current_message, message_timer, true)
			elseif current_message == 'ready' then
				current_message = 'go'
				message_timer = 1
				display_message('messages', current_message, message_timer, true)				
			else 
				current_message = nil;
				current_stage_idx = current_stage_idx + 1
				current_stage = generate_stage(current_stage_idx);
				if current_stage == nil then
					game_over('messages', 'mission-accomplished', 3, true);
					message_timer = 1000;
					return;
				end
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