all: reset producer manager consumer

producer:
	cc producer.c -o p

reset:
	reset

manager:
	cc manager.c -o m

consumer:
	cc consumer.c -o c

clean: clean_consumer clean_manager clean_producer

clean_manager:
	rm m

clean_consumer:
	rm c

clean_producer:
	rm p
