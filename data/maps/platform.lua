demolish_house = false
time_to_demolish = 10
time = 0

function make_passage()
	demolish_house = true
end

function on_tick(dt)
	if demolish_house then
		time = time + dt
		if time > time_to_demolish then 
			damage_map(547,1820,100,64)
			demolish_house = false
		end
	end
end
		