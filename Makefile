##################################################################
# TARGET = ${notdir $(CURDIR)}
# TARGET = v4l2grab
# TARGET = test
TARGET = python_extension

OBJ_DIR = build

INCLUDES = \
-I/usr/include/ \
-I/usr/include/opencv4 \
-I/usr/include/python3.9 \
-I/usr/local/include/


CC = g++
C_FLAGS = -g -O0 -Wall
LD = $(CC)
LD_FLAGS = -lpthread `pkg-config --cflags --libs opencv4`
INCLUDE_SRCS = $(TARGET).cpp v4l2.cpp
LD_LIBS = -lv4l2

ifeq ($(CC), g++)
	TYPE = cpp
else
	TYPE = c
endif

ifeq ($(TARGET), python_extension)
	LD_FLAGS += -shared
	INCLUDE_SRCS := pyv4l2.cpp
endif

# SRCS = $(filter-out $(EXCLUDE_SRCS),$(INCLUDE_SRCS))
SRCS = $(TARGET).$(TYPE)
OBJS += ${foreach src, $(notdir $(SRCS)), ${patsubst %.$(TYPE), $(OBJ_DIR)/%.o, $(src)}}
vpath %.$(TYPE) $(sort $(dir $(SRCS)))

ifneq ($(TARGET),$(subst python_extension,,$(TARGET)))
all:
	python3 setup.py build_ext --inplace
	cp v4l2lib.*.so python
	@echo "Compile finished\n"
else
all : $(TARGET)
	@echo "Builded target:" $^
	@echo "Done"
	cp $(TARGET) exec
	@echo "Compile finished\n"

$(TARGET) : $(OBJS)
	@mkdir -p $(@D)
	@echo "Linking" $@ "from" $^ "..."
	$(LD) -o $@ $^ $(LD_FLAGS) $(LD_LIBS)
	@echo "Link finished\n"

$(OBJS) : $(OBJ_DIR)/%.o:%.$(TYPE)
	@mkdir -p $(@D)
	@echo "Compiling" $@ "from" $< "..."
	$(CC) -c -o $@ $< $(C_FLAGS) $(INCLUDES)
endif


.PHONY : clean cleanobj
clean : cleanobj
	@echo "Remove all executable files"
	rm -f $(TARGET)
cleanobj :
	@echo "Remove object files"
	rm -rf $(OBJ_DIR)/*.o
    ifeq ($(TARGET),$(subst python_extension,,$(TARGET)))
		rm $(TARGET)
    else
		rm *.so
    endif
	@# rm -rf build/*
