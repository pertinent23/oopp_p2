CXX = g++
CC = gcc
SCANNER = wayland-scanner

# Les flags du professeur (-std=c++20, -pthread) + nos flags de warning
INC_DIRS = $(shell find src -type d -name include)
INCLUDES = $(addprefix -I, $(INC_DIRS)) -I./build -I./src
CXXFLAGS = -std=c++20 -Wall -Wextra -O2 $(INCLUDES) -pthread
CFLAGS = -O2 -I. -I./build
LDFLAGS = -pthread -lwayland-client -lxkbcommon

TARGET = game

# Chemins Wayland (du Makefile officiel)
XDG_XML = /usr/share/wayland-protocols/stable/xdg-shell/xdg-shell.xml
XDG_HDR = build/xdg-shell-client-protocol.h
XDG_SRC = build/xdg-shell-protocol.c
XDG_OBJ = build/xdg-shell-protocol.o

# Tous nos fichiers source
SRCS = $(shell find src -name '*.cpp')
OBJS = $(patsubst src/%.cpp, build/%.o, $(SRCS))
DEPS = $(OBJS:.o=.d)

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(XDG_OBJ) $(OBJS)
	@echo "🚀 Édition des liens pour $@..."
	@$(CXX) $(OBJS) $(XDG_OBJ) -o $(TARGET) $(LDFLAGS)
	@echo "✅ Compilation terminée avec succès. Lancez le jeu avec ./$(TARGET)"

# Forcer la création du header généré AVANT de compiler le C++
$(OBJS): | $(XDG_HDR)

build/%.o: src/%.cpp
	@mkdir -p $(dir $@)
	@echo "🔨 Compilation de $<..."
	@$(CXX) $(CXXFLAGS) -MMD -MP -c $< -o $@

$(XDG_OBJ): $(XDG_SRC)
	@mkdir -p $(dir $@)
	@$(CC) $(CFLAGS) -c $< -o $@

$(XDG_HDR): $(XDG_XML)
	@mkdir -p $(dir $@)
	@echo "⚙️ Génération du header Wayland..."
	@$(SCANNER) client-header $< $@

$(XDG_SRC): $(XDG_XML)
	@mkdir -p $(dir $@)
	@$(SCANNER) private-code $< $@

-include $(DEPS)

clean:
	@echo "🧹 Nettoyage..."
	@rm -rf build $(TARGET)
