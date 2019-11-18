firmware:
	source .env && mix firmware

burn:
	source .env && mix firmware.burn

deploy: firmware burn

air_deploy: firmware
	source .env && ./upload.sh
