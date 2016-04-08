driver: driver.c
	gcc -g -o driver -lpthread driver.c
install_driver: dmx_usb.ko
	sudo rmmod ftdi_sio || true
	sudo rmmod dmx_usb || sudo insmod dmx_usb.ko
dmx_usb.ko:
	git clone https://github.com/lowlander/dmx_usb_module.git
	cd dmx_usb_module && make
	cp dmx_usb_module/dmx_usb.ko dmx_usb.ko
visualizer:
	find . -name '*.anim' | xargs -n1 -- cat | shuf -r | ./driver
clean:
	rm -rf dmx_usb_module dmx_usb.ko driver
