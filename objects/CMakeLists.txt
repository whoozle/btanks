include_directories(../engine ../engine/src)

set(bt_objects_SOURCES
	bullet.cpp explosion.cpp single_pose.cpp
	tank.cpp shilka.cpp launcher.cpp ai_tank.cpp
	ai_launcher.cpp ai_shilka.cpp ai_trooper.cpp ai_machinegunner_player.cpp
	ai_heli.cpp ai_mortar.cpp raider_heli.cpp
	traffic_lights.cpp missiles_in_vehicle.cpp missile.cpp
	corpse.cpp item.cpp mine.cpp dirt.cpp
	damage.cpp helicopter.cpp paratrooper.cpp kamikaze.cpp
	machinegunner_on_launcher.cpp destructable_object.cpp submarine.cpp train.cpp
	trooper.cpp fakemod.cpp car.cpp tooltip.cpp
	barrack.cpp watchtower.cpp slime.cpp
	cannon.cpp boat.cpp poison.cpp old_school_destructable.cpp
	zombie.cpp civilian.cpp teleport.cpp cow.cpp
	heli.cpp bomb.cpp explosive.cpp mortar.cpp
	sandworm.cpp barrier.cpp pillbox.cpp buggy.cpp turrel.cpp
	ballistic_missile.cpp mortar_bullet.cpp ctf_base.cpp ctf_flag.cpp
	shilka_turret.cpp gta_car.cpp
)

add_library(bt_objects SHARED ${bt_objects_SOURCES})
target_link_libraries(bt_objects btanks_engine sdlx mrt)
add_custom_command(TARGET bt_objects POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy ARGS $<TARGET_FILE:bt_objects> ${CMAKE_BINARY_DIR})