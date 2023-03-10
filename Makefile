default: play

play:
	gcc play.c -ldl -lm -lpthread -o play
clean:
	rm -f play