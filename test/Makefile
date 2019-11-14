program_NAME := tcc
CC := /home/mam1/gcc-linaro-7.4.1-2019.02-x86_64_arm-linux-gnueabihf/bin/arm-linux-gnueabihf-

.PHONY: all clean distclean

all: $(program_NAME)

$(program_NAME): $(program_OBJS)

	$(CC)gcc tcc.c -o tcc -Wall -fno-exceptions

# # clean up
# 	@- $(RM) -v $(program_NAME)
# 	@- $(RM) -v $(program_OBJS)

clean:
	@- $(RM) -v $(program_NAME)
	@- $(RM) -v $(program_OBJS)

dump:
	@echo program_NAME $(program_NAME)
	@echo program_OBJS $((program_OBJS)
	@echo LDFLAGS $(LDFLAGS)
	@echo CPPFLAGS $(CPPFLAGS)
	@echo CFLAGS $(CFLAGS)
	@echo LINK.cc $(LINK.cc)
	@echo LOGAL_CGI_DIT $(LOCAL_CGI_DIR)

distclean: clean
