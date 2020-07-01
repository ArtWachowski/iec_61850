EXAMPLE_DIRS = client
EXAMPLE_DIRS += mms_utility
EXAMPLE_DIRS += server_app

MODEL_DIRS += server_app

all:	examples

model:
	-for d in $(MODEL_DIRS); do(cd $$d && $(MAKE) model); done

examples:
	-for d in $(EXAMPLE_DIRS); do(cd $$d && $(MAKE)); done

clean:
	-for d in $(EXAMPLE_DIRS); do(cd $$d && $(MAKE) clean); done
	
