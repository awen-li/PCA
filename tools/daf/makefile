
CC = g++ 
SOURCE_PATH  = source
INCLUDE_PATH = include
OBJ_PATH     = obj
LLAF_PATH    = /home/liwen/llaf
CFLAGS       = -g -std=c++11 -I$(INCLUDE_PATH) -I$(LLAF_PATH)/include

SOURCE_FILES = $(wildcard $(SOURCE_PATH)/*.cpp)
OBJ_FILES    = $(addprefix $(OBJ_PATH)/, $(addsuffix .o,$(notdir $(basename $(SOURCE_FILES)))))

TARGET = analyze

.PHONY:all clean

all: $(TARGET)

$(TARGET): $(OBJ_FILES)
	$(CC) -lpthread -o $@ $^
	
$(OBJ_PATH)/%.o: $(SOURCE_PATH)/%.cpp
	@if [ ! -d $(OBJ_PATH) ];then mkdir $(OBJ_PATH); fi
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -rf $(OBJ_PATH) $(TARGET)
