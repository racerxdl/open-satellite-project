

all: SatHelper decoder

clean:
	@echo -e '\033[0;32mCleaning target SatHelper\033[0m'
	@echo -e '\033[0;34m'
	$(MAKE) -C SatHelper clean
	@echo -e '\033[0m'
	@echo -e '\033[0;32mFinished cleaning SatHelper\033[0m'
	@echo -e '\033[0;32mCleaning target GOES Decoder\033[0m'
	@echo -e '\033[0;34m'
	$(MAKE) -C GOES/decoder clean
	@echo -e '\033[0m'
	@echo -e '\033[0;32mFinished cleaning GOES Decoder\033[0m'

SatHelper: FORCE
	@echo -e '\033[0;32mBuilding target: $@\033[0m'
	@echo -e '\033[0;34m'
	$(MAKE) -C SatHelper
	@echo -e '\033[0m'
	@echo -e '\033[0;32mFinished building target: $@\033[0m'
	@echo ' '

decoder: SatHelper
	@echo -e '\033[0;32mBuilding target: $@\033[0m'
	@echo -e '\033[0;34m'
	$(MAKE) -C GOES/decoder
	@echo -e '\033[0m'
	@echo -e '\033[0;32mFinished building target: $@\033[0m'
	@echo ' '

test: SatHelper decoder
	@echo -e '\033[0;32mTesting SatHelper\033[0m'
	@echo -e '\033[0;34m'
	$(MAKE) -C SatHelper test
	@echo -e '\033[0m'
	@echo -e '\033[0;32mFinished testing SatHelper\033[0m'
	@echo ' '
	@echo -e '\033[0;32mTesting GOES Decoder\033[0m'
	@echo -e '\033[0;34m'
	$(MAKE) -C GOES/decoder test
	@echo -e '\033[0m'
	@echo -e '\033[0;32mFinished testing GOES Decoder\033[0m'
	@echo ' '

libvolk: FORCE
	@echo -e '\033[0;32mBuilding target: $@\033[0m'
	@echo -e '\033[0;34m'
	@git clone https://github.com/gnuradio/volk
	@mkdir volk/build -p
	@cd volk/build && cmake ..
	$(MAKE) -C volk/build
	@echo -e '\033[0m'
	@echo -e '\033[0;32mFinished building target: $@\033[0m'

libvolk-install: FORCE
	@echo -e '\033[0;32mInstalling target: $@\033[0m'
	@echo -e '\033[0;34m'
	$(MAKE) -C volk/build install
	@echo -e '\033[0m'
	@echo -e '\033[0;32mFinished installing target: $@\033[0m'
	@echo ' '

libcorrect: FORCE
	@echo -e '\033[0;32mBuilding target: $@\033[0m'
	@echo -e '\033[0;34m'
	@git clone https://github.com/quiet/libcorrect/
	@mkdir libcorrect/build -p
	@cd libcorrect/build && cmake ..
	$(MAKE) -C libcorrect/build
	@echo -e '\033[0m'
	@echo -e '\033[0;32mFinished building target: $@\033[0m'
	@echo ' '

libcorrect-install: FORCE
	@echo -e '\033[0;32mInstalling target: $@\033[0m'
	@echo -e '\033[0;34m'
	$(MAKE) -C libcorrect/build install
	@echo -e '\033[0m'
	@echo -e '\033[0;32mFinished installing target: $@\033[0m'
	@echo ' '

FORCE:
