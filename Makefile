# Meta-Makefile for Context Switcher. Builds both versions of the
# context switcher.
#
# Jeremy Hunt and Christopher Buck
# ELEC 424 Lab 5

all:
	@echo "Building both SysTick and PendSV Switchers"
	make -C pendsv/
	make -C systick/
	@echo "Output files put in pendsv/bin/ and systick/bin/"

clean:
	@echo "Cleaning all"
	make -C pendsv/ clean
	make -C systick/ clean

flash:
	@echo "Please run from within the inner folder"

debug:
	@echo "Please run from within the inner folder"	


