#GENERAL VARIABLES

MAKEFLAGS := --no-print-directory
.DEFAULT_GOAL := all

RM := rm -rf
MKDIR := mkdir -p

NAME := ircserv

CXX= c++
CXXFLAGS = -Wall -Wextra -Werror -std=c++98 -fsanitize=address -g3

#FILES

SRC_DIR := src/
OBJ_DIR := obj/

VPATH := $(SRC_DIR) $(addprefix $(SRC_DIR), \
 				server\
     			utils\
			)

SOURCES :=	\
			main.cpp\

SOURCES +=	\
			Channel.cpp\
			Client.cpp\
			Message.cpp\
			Server.cpp\

OBJECTS := $(addprefix $(OBJ_DIR), $(SOURCES:.cpp=.o))

#COLOURS

RED := \033[31m
GREEN := \033[32m
YELLOW := \033[33m
BLUE := \033[34m

PDEL-OLM := \033[38;5;207m
DAXFERNA := \033[38;5;76m
DARMARTI := \033[38;5;55m
CROWN := \033[38;5;220m

RESET := \033[0m

#EXTRA VARIABLES

VALGRIND_DIR := .valgrind/

#RULES

$(NAME): $(OBJECTS)
	$(CXX) $(CXXFLAGS) $(OBJECTS) -o $(NAME)
	@$(MAKE) msg_credits

$(OBJ_DIR)%.o: %.cpp $(HEADER) | $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJ_DIR):
	$(MKDIR) $(OBJ_DIR)
	
.PHONY: a all
a: all
all: $(NAME)

.PHONY: c clean
c: clean
clean: msg_clean_start
	$(RM) $(OBJ_DIR)
	@$(MAKE) msg_clean_end

.PHONY: f fclean
f: fclean
fclean: msg_fclean_start
	$(RM) $(OBJ_DIR) $(NAME)
	@$(MAKE) msg_fclean_end

.PHONY: r re
r: re
re: fclean all

#EXTRA RULES

.PHONY: e exec
e: exec
exec: all
	-./$(NAME) 6667 pass

.PHONY: v valgrind valgrind_no_flags
v: valgrind
valgrind:
	@$(MAKE) CXXFLAGS= valgrind_no_flags
valgrind_no_flags: clean $(OBJECTS) | $(VALGRIND_DIR)
	@$(CXX) $(CXXFLAGS) $(OBJECTS) -o $(NAME)
	@-valgrind --leak-check=full --show-leak-kinds=all --log-file=$(VALGRIND_DIR)$$(date +"%y%m%d%H%M%S").txt ././$(NAME) scenes/minimalist.rt
	@echo "$(BLUE)$$(grep -e "ERROR SUMMARY" -e "lost:" -e "reachable:" $(VALGRIND_DIR)/$$(ls $(VALGRIND_DIR) | tail -1))"
	@$(MAKE) clean

$(VALGRIND_DIR):
	$(MKDIR) $(VALGRIND_DIR)

.PHONY: last_valgrind
last_valgrind:
	@cat $(VALGRIND_DIR)$$(ls $(VALGRIND_DIR) | tail -1)

.PHONY: clean_valgrind
clean_valgrind:
	$(RM) $(VALGRIND_DIR)

#MESSAGES

.PHONY: msg_clean_start msg_clean_end msg_fclean_start msg_fclean_end msg_credits

msg_clean_start:
	@echo "$(YELLOW)Cleaning $(NAME) objects$(RED)"

msg_clean_end:
	@echo "$(GREEN)$(NAME) objects cleaned$(RESET)"

msg_fclean_start:
	@echo "$(YELLOW)Cleaning $(NAME)$(RED)"

msg_fclean_end:
	@echo "$(GREEN)$(NAME) cleaned$(RESET)"

msg_credits:
	@echo "$(CROWN)\n\
            ▟╗▟▙╖▙╖\n\
            ██████║\n\$(PDEL-OLM)\
██████╗ ████▇▇╗ ▇▇▇████╗██╗            ██████╗ ██╗     ███╗   ███╗\n\
██╔══██╗██╔══██╗██╔════╝██║           ██╔═══██╗██║     ████╗ ████║\n\
██████╔╝██║  ██║█████╗  ██║    █████╗ ██║   ██║██║     ██╔████╔██║\n\
██╔═══╝ ██║  ██║██╔══╝  ██║    ╚════╝ ██║   ██║██║     ██║╚██╔╝██║\n\
██║     ██████╔╝███████╗███████╗      ╚██████╔╝███████╗██║ ╚═╝ ██║\n\
╚═╝     ╚═════╝ ╚══════╝╚══════╝       ╚═════╝ ╚══════╝╚═╝     ╚═╝\n\
$(CROWN)\
                                           ▟╗▟▙╖▙╖\n\
                                           ██████║\n$(DAXFERNA)\
██████╗  █████╗ ██╗  ██╗███████╗███████╗███▇▇▇╗ ▇▇█╗   ██╗ █████╗ \n\
██╔══██╗██╔══██╗╚██╗██╔╝██╔════╝██╔════╝██╔══██╗████╗  ██║██╔══██╗\n\
██║  ██║███████║ ╚███╔╝ █████╗  █████╗  ██████╔╝██╔██╗ ██║███████║\n\
██║  ██║██╔══██║ ██╔██╗ ██╔══╝  ██╔══╝  ██╔══██╗██║╚██╗██║██╔══██║\n\
██████╔╝██║  ██║██╔╝╚██╗██║     ███████╗██║  ██║██║ ╚████║██║  ██║\n\
╚═════╝ ╚═╝  ╚═╝╚═╝  ╚═╝╚═╝     ╚══════╝╚═╝  ╚═╝╚═╝  ╚═══╝╚═╝  ╚═╝\n\
$(CROWN)\
                   ▟╗▟▙╖▙╖\n\
                   ██████║\n$(DARMARTI)\
██████╗  █████╗ ███▇▇▇╗ ▇▇█╗   ███╗ █████╗ ██████╗ ████████╗██╗\n\
██╔══██╗██╔══██╗██╔══██╗████╗ ████║██╔══██╗██╔══██╗╚══██╔══╝██║\n\
██║  ██║███████║██████╔╝██╔████╔██║███████║██████╔╝   ██║   ██║\n\
██║  ██║██╔══██║██╔══██╗██║╚██╔╝██║██╔══██║██╔══██╗   ██║   ██║\n\
██████╔╝██║  ██║██║  ██║██║ ╚═╝ ██║██║  ██║██║  ██║   ██║   ██║\n\
╚═════╝ ╚═╝  ╚═╝╚═╝  ╚═╝╚═╝     ╚═╝╚═╝  ╚═╝╚═╝  ╚═╝   ╚═╝   ╚═╝\n\
$(RESET)"
