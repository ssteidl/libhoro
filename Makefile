all:
	make -C src

nevercreated:

tags: nevercreated
	find . -name "*.[ch]" -exec etags -a {} \;

clean:
	make -C src clean
	rm -vf *~
