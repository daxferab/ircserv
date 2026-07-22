MAKEFLAGS := --no-print-directory
.DEFAULT_GOAL := all

.PHONY: all bonus clean fclean re

all:
	@$(MAKE) -C standard

bonus:
	@$(MAKE) -C bonus

clean:
	@$(MAKE) -C standard clean
	@$(MAKE) -C bonus clean

fclean:
	@$(MAKE) -C standard fclean
	@$(MAKE) -C bonus fclean

re: fclean all
