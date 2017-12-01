default: lab3b

lab3b: src/lab3b/Lab3b.java src/lab3b/infoobject/*.java
	javac -d src/ $^
	echo -e '#!/bin/bash\njava -cp src/ lab3b.Lab3b $$@' > lab3b
	chmod +x lab3b

.PHONY: default clean dist

clean:
	rm -f lab3b src/lab3b/Lab3b.class src/lab3b/infoobject/*.class *.tar.gz

dist: src/lab3b/Lab3b.java src/lab3b/infoobject/*.java Makefile README
	tar -czf lab3b-404751542.tar.gz $^
