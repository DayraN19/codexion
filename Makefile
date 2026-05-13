# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: bastiangranier <bastiangranier@student.    +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2025/12/05 13:00:48 by bgranier          #+#    #+#              #
#    Updated: 2026/05/13 14:28:47 by bastiangran      ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

NAME = codexion
CC = gcc
CFLAGS = -Wall -Wextra -Werror -pthread

SRC = main.c \
		heap.c \
		init.c \
		monitor.c \
		time.c \
		routine.c \
		utils.c \
		init.c \

OBJ = $(SRC:.c=.o)

RM = rm -f

all: $(NAME)

$(NAME): $(OBJ)
	$(CC) $(CFLAGS) $(OBJ) -o $(NAME)

%.o: %.c codexion.h
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	$(RM) $(OBJ)

fclean: clean
	$(RM) $(NAME)

re: fclean all

.PHONY: all clean fclean re