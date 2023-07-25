.PHONY:

init:
	
	# Board
	arduino-cli config init &2>/dev/null
	sleep 1
	
	# Core
	arduino-cli config add board_manager.additional_urls https://raw.githubusercontent.com/RAKWireless/RAKwireless-Arduino-BSP-Index/main/package_rakwireless.com_rui_index.json
	arduino-cli update
	arduino-cli core install rak_rui:nrf52
	
	# Libraries
	arduino-cli config set library.enable_unsafe_install true
	arduino-cli lib install --zip-path ../Lib_Used/*.zip 

	# hack for linux/mac machines
	[ ! -d src ] && ln -s Src src

build:
	arduino-cli compile --fqbn rak_rui:nrf52:WisCoreRAK4631Board .

upload:
	arduino-cli upload -p /dev/ttyACM0 --fqbn rak_rui:nrf52:WisCoreRAK4631Board .

