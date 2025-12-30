#
# make file 
#

CXX		= g++
CXXFLAGS= -std=c++17 -Wall -g -O2 -I./include/
LDFLAGS = -lpthread -lstdc++fs -lsqlite3
SOURCES	= nau_item.cpp nau_json.cpp nau_arp.cpp nau_http.cpp nau_db.cpp nau_main.cpp nau_iface.cpp main.cpp
GRE_SOURCES = nau_item.cpp nau_json.cpp nau_arp.cpp nau_http.cpp nau_db.cpp nau_main.cpp nau_iface.cpp gre_admin.cpp  # 额外的源文件，用于 gre_admin 可执行文件

TARGET	= nau
GRE_TARGET = gre_admin  # 新的可执行文件目标
OBJECTS = $(SOURCES:.cpp=.o)
GRE_OBJECTS = $(GRE_SOURCES:.cpp=.o)

# 默认目标，生成 nau
all: $(TARGET) $(GRE_TARGET)

$(TARGET): $(OBJECTS)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS)

$(GRE_TARGET): $(GRE_OBJECTS)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS)

clean:
	rm -f $(TARGET) $(GRE_TARGET) $(OBJECTS) $(GRE_OBJECTS)
