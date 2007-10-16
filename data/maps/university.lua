-- sample lua test file

ai_stage = 0
ai_ids = {}
throttle = 0

function on_tick(dt)
	if ai_stage > 3 then game_over('messages', 'mission-accomplished', 4, true) return end

	throttle = throttle + dt
	if throttle < 5 then return end
	throttle = 0

	if #ai_ids == 0 then 
		print(ai_stage)
		
		ai_stage = ai_stage + 1
		if ai_stage > 3 then return end
		
		local i = 1
		while i <= ai_stage do
			ai_ids[i] = show_item('object:helicopter:helicopter:'..(ai_stage+i-1))
			print('spawned heli: '..ai_ids[i])
			i = i + 1
		end
	else 
		--checking ai units
		local i = 1
		while i <= #ai_ids do 
			if object_exists(ai_ids[i]) then return end -- live ai player
			i = i + 1
		end
		ai_ids = {}
	end
end

hide_item('object:helicopter:helicopter:1')
hide_item('object:helicopter:helicopter:2')
hide_item('object:helicopter:helicopter:3')
hide_item('object:helicopter:helicopter:4')
hide_item('object:helicopter:helicopter:5')
hide_item('object:helicopter:helicopter:6')
