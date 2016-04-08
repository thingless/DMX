install_driver: dmx_usb.ko
	sudo insmod dmx_usb.ko
dmx_usb.ko:
	git clone https://github.com/lowlander/dmx_usb_module.git
	cd dmx_usb_module && make
	cp dmx_usb_module/dmx_usb.ko dmx_usb.ko
clean:
	rm -rf dmx_usb_module dmx_usb.ko
