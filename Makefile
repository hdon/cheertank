all : cheertank

cheertank : build/main.o
	$(CC) $(CFLAGS) $^ $(LDFLAGS) -o $@ 

build/%.o : src/%.c
	$(CC) $(CFLAGS) -o $@ -c $<

# Local system configuration - not used right now
include configuration

clean ::
	@-rm -f deps/* build/* cheertank

