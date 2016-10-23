driver: driver.c
	gcc -g -o driver -lpthread driver.c
visualizer: driver
	find examples -name '*.anim' | xargs -n1 -- cat | shuf -r | ./driver
install_driver: dmx_usb.ko
	sudo rmmod ftdi_sio || true
	sudo rmmod dmx_usb || sudo insmod dmx_usb.ko
dmx_usb.ko:
	rm -r dmx_usb_module || true
	git clone https://github.com/lowlander/dmx_usb_module.git
	cd dmx_usb_module && make
	cp dmx_usb_module/dmx_usb.ko dmx_usb.ko
clean:
	rm -rf dmx_usb_module dmx_usb.ko driver
