demolish_house = false
time_to_demolish = 2
time = 0

function make_passage()
	demolish_house = true
end

function on_tick(dt)
	if demolish_house then
		time = time + dt
		if time > time_to_demolish then 
			damage_map(576, 1856, 100)
			demolish_house = false
		end
	end
end
		

disable_ai('tank')