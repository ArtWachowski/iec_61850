
EXAMPLE_DIRS = iec61850_client_example_control
EXAMPLE_DIRS += server_example_control
EXAMPLE_DIRS += server_example_config_file
EXAMPLE_DIRS += mms_utility
EXAMPLE_DIRS += server_example_write_handler
EXAMPLE_DIRS += server_app

MODEL_DIRS += server_example_control
MODEL_DIRS += server_example_config_file
MODEL_DIRS += server_example_logging

all:	examples

model:
	-for d in $(MODEL_DIRS); do(cd $$d && $(MAKE) model); done

examples:
	-for d in $(EXAMPLE_DIRS); do(cd $$d && $(MAKE)); done

clean:
	-for d in $(EXAMPLE_DIRS); do(cd $$d && $(MAKE) clean); done
	
