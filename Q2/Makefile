all: ipcrm reset train manager

train:
	gcc train.c -o t

ipcrm:
	ipcrm -a

reset:
	reset

manager:
	gcc manager.c -o m

clean: clean_manager clean_train

clean_manager:
	rm m

clean_train:
	rm t
