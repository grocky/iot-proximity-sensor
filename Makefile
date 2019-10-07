build:
	platformio run
upload:
	platformio run -t upload
monitor:
	platformio serialports monitor
