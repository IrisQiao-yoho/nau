#
# make file 
#

CXX		= g++
CXXFLAGS= -std=c++17 -Wall -g -O2 -I./include/
LDFLAGS = -lpthread -lstdc++fs -lsqlite3
SOURCES	= nau_item.cpp nau_json.cpp nau_arp.cpp nau_http.cpp nau_db.cpp nau_main.cpp nau_iface.cpp main.cpp
TARGET	= nau
OBJECTS = $(SOURCES:.cpp=.o)

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS)

clean:
	rm -f $(TARGET) $(OBJECTS)

