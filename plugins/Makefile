CC:=clang

%.o: %.c
	$(CC) -fPIC -c $(CFLAGS) $(CPPFLAGS) $< -o $@

lib%.so: %.o
	$(CC) -shared -Wl,-soname,$@.1 -o $@.1.0.1  $<
